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
   Common menu interface functions, also known as "ZX Vision"
*/

//
// Archivo para funciones auxiliares de soporte de menu, excluyendo entradas de menu
// Las entradas de menu estan en menu_items.c, menu_debug_cpu.c, menu_items_settings.c, menu_filesel.c, menu_fileviewer.c
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




int zxvision_switch_to_window_on_open_menu=0;
char zxvision_switch_to_window_on_open_menu_name[MAX_NAME_WINDOW_GEOMETRY];


//si se pulsa tecla mientras se lee el menu
int menu_speech_tecla_pulsada=0;

//indica que hay funcion activa de overlay o no
int menu_overlay_activo=0;

//indica si el menu hace zoom. valores validos: 1 en adelante
int menu_gui_zoom=1;

//Si se dibuja papelera abierta
int zxvision_draw_icon_papelera_abierta=0;

//Ancho de caracter de menu
int menu_char_width=8;

//Alto de caracter de menu. De momento siempre fijo a 8 pero es conveniente que las rutinas siempre usen esta variable en vez de suponer
//siempre 8, por si en un futuro cambia
int menu_char_height=8;

int menu_last_cpu_use=0;

defined_f_function defined_direct_functions_array[MAX_F_FUNCTIONS]={
    {"Default",F_FUNCION_DEFAULT,bitmap_button_ext_desktop_userdefined,""},  //no mover nunca de sitio el default para que sea siempre la posicion 0
    {"Nothing",F_FUNCION_NOTHING,bitmap_button_ext_desktop_nothing,""},
    {"OpenMenu",F_FUNCION_OPENMENU,zesarux_ascii_logo,""},

    //Actuar sobre la cpu y velocidad del emulador
    {"Reset",F_FUNCION_RESET,bitmap_button_ext_desktop_reset,""},
    {"HardReset",F_FUNCION_HARDRESET,bitmap_button_ext_desktop_hardreset,""},
    {"NMI",F_FUNCION_NMI,bitmap_button_ext_desktop_nmi,""},
    {"DebugCPU",F_FUNCION_DEBUGCPU,bitmap_button_ext_desktop_debugcpu,"debugcpu"},
    {"DebugCPUBreakpoints",F_FUNCION_DEBUGCPU_BREAKPOINTS,bitmap_button_ext_desktop_breakpoints,""},
    {"DebugCPUViewAdv",F_FUNCION_DEBUGCPU_VIEW_ADVENTURE,bitmap_button_ext_desktop_debugcpu_view_adventure,""},
    {"Poke",F_FUNCION_POKE,bitmap_button_ext_desktop_poke,""},
    {"TextAdventureMap",F_FUNCION_TEXT_ADVENTURE_MAP,bitmap_button_ext_desktop_text_adventure_map,"textadvmap"},
    {"HexEditor",F_FUNCION_HEX_EDITOR,bitmap_button_ext_desktop_hexeditor,"hexeditor"},
    {"ViewSprites",F_FUNCION_VIEW_SPRITES,bitmap_button_ext_desktop_viewsprites,"sprites"},
    {"FileUtils",F_FUNCION_FILE_UTILITIES,bitmap_button_ext_desktop_fileutils,""},
    {"Pause",F_FUNCION_PAUSE,bitmap_button_ext_desktop_pause,""},
    {"TopSpeed",F_FUNCION_TOPSPEED,bitmap_button_ext_desktop_topspeed,""},

    //Snaps
    {"SnapInRAMRewind",F_FUNCION_REWIND,bitmap_button_ext_desktop_snapinramrewind,""},
    {"SnapInRAMFFW",F_FUNCION_FFW,bitmap_button_ext_desktop_snapinramffw,""},

    //ventanas directas a menus
    {"SmartLoad",F_FUNCION_SMARTLOAD,bitmap_button_ext_desktop_smartload,""},
    {"Quickload",F_FUNCION_QUICKLOAD,bitmap_button_ext_desktop_quickload,""},
    {"Quicksave",F_FUNCION_QUICKSAVE,bitmap_button_ext_desktop_quicksave,""},
    {"QuicksaveScreen",F_FUNCION_QUICKSAVE_SCREEN,bitmap_button_ext_desktop_quicksave_scr,""},
    {"LoadBinary",F_FUNCION_LOADBINARY,bitmap_button_ext_desktop_loadbinary,""},
    {"SaveBinary",F_FUNCION_SAVEBINARY,bitmap_button_ext_desktop_savebinary,""},
    {"Settings",F_FUNCION_SETTINGS,bitmap_button_ext_desktop_settings,""},
    //menu audio
    {"AudioChipRegisters",F_FUNCION_AUDIO_REGISTERS,bitmap_button_ext_desktop_audioregisters,"ayregisters"},
    {"AudioChipSheet",F_FUNCION_AUDIO_SHEET,bitmap_button_ext_desktop_audiosheet,"aysheet"},
    {"AudioChipPiano",F_FUNCION_AUDIO_PIANO,bitmap_button_ext_desktop_audiopiano,"aypiano"},
    {"WavePiano",F_FUNCION_WAVE_PIANO,bitmap_button_ext_desktop_wavepiano,"wavepiano"},
    {"Waveform",F_FUNCION_WAVEFORM,bitmap_button_ext_desktop_waveform,"waveform"},
    {"AYMixer",F_FUNCION_AYMIXER,bitmap_button_ext_desktop_aymixer,""},
    {"AYPlayer",F_FUNCION_AYPLAYER,bitmap_button_ext_desktop_ayplayer,"ayplayer"},
    {"AYPlayerNextTrack",F_FUNCION_AYPLAYER_NEXT_TRACK,bitmap_button_ext_desktop_ayplayer_next_track,""},
    {"AYPlayerPrevTrack",F_FUNCION_AYPLAYER_PREVIOUS_TRACK,bitmap_button_ext_desktop_ayplayer_previous_track,""},
    {"AYPlayerNextFile",F_FUNCION_AYPLAYER_NEXT_FILE,bitmap_button_ext_desktop_ayplayer_next_file,""},
    {"AYPlayerPrevFile",F_FUNCION_AYPLAYER_PREVIOUS_FILE,bitmap_button_ext_desktop_ayplayer_previous_file,""},
    {"AYPlayerPause",F_FUNCION_AYPLAYER_PAUSE,bitmap_button_ext_desktop_ayplayer_pause,""},
    {"VolumeDecrement",F_FUNCION_VOLUME_DECREMENT,bitmap_button_ext_desktop_volume_decrement,""},
    {"VolumeIncrement",F_FUNCION_VOLUME_INCREMENT,bitmap_button_ext_desktop_volume_increment,""},
    {"VolumeSilence",F_FUNCION_VOLUME_SILENCE,bitmap_button_ext_desktop_volume_silence,""},
    //menu display
    {"ColourPalettes",F_FUNCION_COLOUR_PALETTES,bitmap_button_ext_desktop_colour_palettes,"displaypalettes"},
    {"OSDKeyboard",F_FUNCION_OSDKEYBOARD,bitmap_button_ext_desktop_osdkeyboard,""},
    {"OSDTextKeyboard",F_FUNCION_OSDTEXTKEYBOARD,bitmap_button_ext_desktop_osdadvkeyboard,""},

    //Realmente las dos siguientes actuan igual, solo les diferencia el bitmap,
    //y que cuando se crea SetMachine desde "New Icon" se asigna SetMachine con la maquina actual
    {"MachineSelection",F_FUNCION_MACHINE_SELECTION,bitmap_button_ext_desktop_machine,""},
    {"SetMachine",F_FUNCION_SET_MACHINE,bitmap_button_ext_desktop_set_machine,""},

    //joystick
    {"JoyLeftRight",F_FUNCION_LEFTRIGHT_JOY,bitmap_button_ext_desktop_joyleftright,""},

    //Switch de cosas
    {"SwitchBorder",F_FUNCION_SWITCHBORDER,bitmap_button_ext_desktop_switchborder,""},
    {"SwitchFullScr",F_FUNCION_SWITCHFULLSCREEN,bitmap_button_ext_desktop_fullscreen,""},
    {"SwitchFooter",F_FUNCION_SWITCHFOOTER,bitmap_button_ext_desktop_switchfooter,""},

    //Actuar sobre storage
    {"ReloadMMC",F_FUNCION_RELOADMMC,bitmap_button_ext_desktop_reloadmmc,""},
    {"ReinsertStdTape",F_FUNCION_REINSERTSTDTAPE,bitmap_button_ext_desktop_reinserttape,""},
    {"PauseUnpauseRealTape",F_FUNCION_PAUSEUNPAUSEREALTAPE,bitmap_button_ext_desktop_pauseunpausetape,""},
    {"ReinsertRealTape",F_FUNCION_REINSERTREALTAPE,bitmap_button_ext_desktop_reinsertrealtape,""},
    {"RewindRealTape",F_FUNCION_REWINDREALTAPE,bitmap_button_ext_desktop_rewindtape,""},
    {"FFWDRealTape",F_FUNCION_FFWDREALTAPE,bitmap_button_ext_desktop_ffwdtape,""},
    {"VisualRealTape",F_FUNCION_VISUALREALTAPE,bitmap_button_ext_desktop_visualrealtape,"visualrealtape"},

    //Actuar sobre menus y ventanas
    {"CloseAllMenus",F_FUNCION_CLOSE_ALL_MENUS,bitmap_button_ext_desktop_close_all_menus,""},
    {"ExitEmulator",F_FUNCION_EXITEMULATOR,bitmap_button_ext_desktop_exit,""},
    {"BackgroundWindow",F_FUNCION_BACKGROUND_WINDOW,bitmap_button_ext_desktop_backgroundwindow,""},
    //Para el usuario, mejor esta descripcion de ShowBackgroundWindows en vez de overlay_windows
    {"ShowBackgroundWindows",F_FUNCION_OVERLAY_WINDOWS,bitmap_button_ext_desktop_showbackgroundwindows,""},
    //Abrir una ventana cualquiera. Necesita nombre de la ventana en la extra info del icono
    {"OpenWindow",F_FUNCION_OPEN_WINDOW,bitmap_button_ext_desktop_openwindow,""},

    //Misc
    {"ZengMessage",F_FUNCION_ZENG_SENDMESSAGE,bitmap_button_ext_desktop_zengmessage,""},
    {"ZengOnline",F_FUNCION_ZENG_ONLINE,bitmap_button_ext_desktop_zengonline,""},
    {"ZengOnlineMessage",F_FUNCION_ZENG_ONLINE_SENDMESSAGE,bitmap_button_ext_desktop_zengonlinemessage,""},
    {"SpeccyOnline",F_FUNCION_ONLINE_SPECCY,bitmap_button_ext_desktop_speccy_online,""},
    {"ZX81Online",F_FUNCION_ONLINE_ZX81,bitmap_button_ext_desktop_zx81_online,""},
    {"OCR",F_FUNCION_OCR,bitmap_button_ext_desktop_ocr,""},
    {"ZXUnoPrismSwitch",F_FUNCION_ZXUNO_PRISM,bitmap_button_ext_desktop_zxunoprismswitch,""},
    {"TrashCan",F_FUNCION_DESKTOP_TRASH,bitmap_button_ext_desktop_trash,""},

    //Estos solo tiene sentido cuando lleva asociado la ruta en la extra info del icono
    //Para un snapshot
    {"LinkToSnapshot",F_FUNCION_DESKTOP_SNAPSHOT,bitmap_button_ext_desktop_file_snapshot,""},
    //Para una cinta
    {"LinkToTape",F_FUNCION_DESKTOP_TAPE,bitmap_button_ext_desktop_file_tape,""},

    //Para el resto
    {"LinkToDefault",F_FUNCION_DESKTOP_GENERIC_SMARTLOAD,bitmap_button_ext_desktop_file_generic_smartload,""},

    {"MyMachine",F_FUNCION_DESKTOP_MY_MACHINE,bitmap_button_ext_desktop_my_machine_generic,""}
};

//Retorna accion asociada a una posicion dentro de defined_direct_functions_array
enum defined_f_function_ids menu_da_accion_direct_functions_indice(int indice)
{
    return defined_direct_functions_array[indice].id_funcion;
}

//Funciones de teclas F mapeadas. Desde F1 hasta F15
//apuntan a indices sobre la tabla defined_direct_functions_array
int defined_f_functions_keys_array[MAX_F_FUNCTIONS_KEYS]={
    0,
    0,
    0,
    0,
    0, //F5
    0,
    0,
    0,
    0,
    0, //F10
    0,
    0,
    0,
    0,
    0 //F15
};

//Extra info de las Funciones de teclas F mapeadas. Desde F1 hasta F15
//Esto de momento solo se usa al asignar tecla F a accion Openwindow, y el nombre de la ventana se asigna aqui
char defined_f_functions_keys_array_parameters[MAX_F_FUNCTIONS_KEYS][PATH_MAX]={
    "",
    "",
    "",
    "",
    "", //F5
    "",
    "",
    "",
    "",
    "", //F10
    "",
    "",
    "",
    "",
    "" //F15
};


//Botones a teclas F mapeadas. 11 botones. Desde smartload hasta help
//apuntan a indices sobre la tabla defined_direct_functions_array
int defined_buttons_functions_array[MAX_USERDEF_BUTTONS]={
    0,
    0,
    0,
    0,
    0, //F5
    0,
    0,
    0,
    0,
    0, //F10
    0
};


//Extra info de los botones a acciones mapeadas
char defined_buttons_functions_array_parameters[MAX_USERDEF_BUTTONS][PATH_MAX]={
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    ""
};


//Retorna el indice a la tabla para una accion concreta
int zxvision_get_id_direct_funcion_index(enum defined_f_function_ids id_funcion)
{


    int i;

    for (i=0;i<MAX_F_FUNCTIONS;i++) {
        if (defined_direct_functions_array[i].id_funcion==id_funcion) {
            return i;
        }
    }

    return -1;
}

//Iconos configurables por el usuario presentes en el zx desktop
zxdesktop_configurable_icon zxdesktop_configurable_icons_list[MAX_ZXDESKTOP_CONFIGURABLE_ICONS];

//Id de icono que se esta ejecutando la accion ahora mismo
int zxdesktop_configurable_icons_current_executing=-1;

//Dice si una posicion para un icono es valida, evitando salirse de ventana, en zonas de botones, o en zona de pantalla emulada
//x,y en coordenadas de iconos (o sea sin zoom)
//Retorna no 0 si es valido
int zxvision_if_configurable_icon_on_valid_position(int x,int y)
{
    //Ver si posicion no se sale del rango de ventana total (sin footer)
    int total_width=screen_get_total_width_window_plus_zxdesktop_no_zoom()-ZESARUX_ASCII_LOGO_ANCHO;
    int total_height=screen_get_total_height_window_no_footer_plus_zxdesktop_no_zoom()-ZESARUX_ASCII_LOGO_ANCHO;

    //Consideramos el tamanyo del icono (ZESARUX_ASCII_LOGO_ANCHO) para que no se pueda ubicar medio icono fuera de rango por ejemplo

    if (x<0 || y<0 || x>total_width || y>total_height) {
        //printf("Check icon position: %d,%d out of range (%d,%d)\n",x,y,total_width,total_height);
        return 0;
    }

    int xinicio_botones,xfinal_botones,yinicio_botones,alto_boton;

    //Ver si en posicion de iconos superiores
    menu_ext_desktop_buttons_get_geometry(NULL,&alto_boton,NULL,&xinicio_botones,&xfinal_botones);
    //Posiciones menos el zoom
    xinicio_botones /=zoom_x;
    xfinal_botones /=zoom_x;
    alto_boton /=zoom_y;

    //Consideramos el tamanyo del icono (ZESARUX_ASCII_LOGO_ANCHO) para que no se pueda ubicar medio icono fuera de rango por ejemplo
    xinicio_botones -=ZESARUX_ASCII_LOGO_ANCHO;


    if (y<alto_boton && x>=xinicio_botones && x<xfinal_botones) {
        //printf("Check icon position: %d,%d on upper buttons position (y<%d)\n",x,y,alto_boton);
        return 0;
    }

    //Ver si en posicion de iconos inferiores

    menu_ext_desktop_lower_icons_get_geometry(NULL,NULL,NULL,&xinicio_botones,&xfinal_botones,&yinicio_botones);
    //Posiciones menos el zoom
    xinicio_botones /=zoom_x;
    xfinal_botones /=zoom_x;
    yinicio_botones /=zoom_y;

    //Consideramos el tamanyo del icono (ZESARUX_ASCII_LOGO_ANCHO) para que no se pueda ubicar medio icono fuera de rango por ejemplo
    xinicio_botones -=ZESARUX_ASCII_LOGO_ANCHO;
    yinicio_botones -=ZESARUX_ASCII_LOGO_ANCHO;

    if (y>=yinicio_botones && x>=xinicio_botones && x<=xfinal_botones) {
        //printf("Check icon position: %d,%d on lower device icons position\n",x,y);
        return 0;
    }


    //Ver si icono en posicion de pantalla emulada
    int ancho_maquina=screen_get_emulated_display_width_no_zoom_border_en();
    int alto_maquina=screen_get_emulated_display_height_no_zoom_border_en();

    if (x<ancho_maquina && y<alto_maquina) {
        //printf("Check icon position: %d,%d on emulated machine display\n",x,y);
        return 0;
    }

    return 1;

}

//Posicion sin considerar zoom
void zxvision_set_configurable_icon_position(int icon,int x,int y)
{

    if (!zxvision_if_configurable_icon_on_valid_position(x,y)) {
        //printf("Trying to set icon %d to invalid position %d,%d\n",icon,x,y);
        return;
    }


    zxdesktop_configurable_icons_list[icon].pos_x=x;
    zxdesktop_configurable_icons_list[icon].pos_y=y;
}

void zxvision_set_configurable_icon_text(int indice_icono,char *texto)
{
    strcpy(zxdesktop_configurable_icons_list[indice_icono].text_icon,texto);
}

void zxvision_set_configurable_icon_extra_info(int indice_icono,char *extra_info)
{
    strcpy(zxdesktop_configurable_icons_list[indice_icono].extra_info,extra_info);
}

//Ver cuantos iconos hay cerca para saber si se puede posicionar uno o no
int zxvision_si_icono_cerca(int x,int y)
{
    int xminimo=x-ZESARUX_ASCII_LOGO_ANCHO;
    int xmaximo=x+ZESARUX_ASCII_LOGO_ANCHO;

    int yminimo=y-ZESARUX_ASCII_LOGO_ANCHO;
    int ymaximo=y+ZESARUX_ASCII_LOGO_ANCHO;

    int i;

    int iconos_cerca=0;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_EXISTS) {
            int icon_x=zxdesktop_configurable_icons_list[i].pos_x;
            int icon_y=zxdesktop_configurable_icons_list[i].pos_y;

            //Si hay uno cerca de ahi, volver con 1
            if (icon_x>=xminimo && icon_x<xmaximo && icon_y>=yminimo && icon_y<ymaximo) iconos_cerca++;

        }
    }

    return iconos_cerca;

}

void zxvision_get_start_valid_positions_icons(int *p_xinicial,int *p_xfinal,int *p_yinicial,int *p_yfinal)
{
    /*

    Zona de forma cuadrada delimitada por:
    x: desde derecha pantalla emulada hasta final x
    y: desde debajo botones superiores hasta por encima botones inferiores

    */


    int inicio_x_zxdesktop=screen_get_emulated_display_width_no_zoom_border_en();


    //Empezar a ubicarlos con algo de margen
    int xinicial=inicio_x_zxdesktop+24;

    int xfinal=screen_get_total_width_window_plus_zxdesktop_no_zoom()-ZESARUX_ASCII_LOGO_ANCHO;

    //yinicial debajo de botones superiores
    int alto_boton;
    menu_ext_desktop_buttons_get_geometry(NULL,&alto_boton,NULL,NULL,NULL);
    alto_boton /=zoom_y;


    int yinicial=alto_boton+16;



    //Hasta llegar a los iconos de dispositivos inferiores
    int yfinal;
    menu_ext_desktop_lower_icons_get_geometry(NULL,NULL,NULL,NULL,NULL,&yfinal);
    //Posiciones menos el zoom
    yfinal /=zoom_y;
    //Consideramos el tamanyo del icono (ZESARUX_ASCII_LOGO_ANCHO) para que no se pueda ubicar medio icono fuera de rango por ejemplo
    yfinal -=ZESARUX_ASCII_LOGO_ANCHO;

    *p_xinicial=xinicial;
    *p_xfinal=xfinal;
    *p_yinicial=yinicial;
    *p_yfinal=yfinal;

}

#define ZXVISION_SEPARACION_ICONOS_AL_ORDENAR (ZESARUX_ASCII_LOGO_ANCHO*2*menu_gui_zoom)
#define ZXVISION_OFFSET_ICONOS_REPETIR_POSICION 2

void zxvision_get_next_free_icon_position(int *p_x,int *p_y)
{
    /*


    Si hay uno cerca, mover a derecha. Si no hay en en toda esa línea, incrementar Y y seguir
    Si finalmente no se encuentra hueco, ponerlo en posición inicial

    Las funciones zxvision_get_next_free_icon_position y zxvision_get_next_free_icon_position utilizan
    la misma separación de posicionamiento de iconos,
    solo que zxvision_get_next_free_icon_position busca el siguiente hueco libre mientras que zxvision_reorder_configurable_icons
    reordena uno a uno cada icono
    */


/*

    int inicio_x_zxdesktop=screen_get_emulated_display_width_no_zoom_border_en();


    //Empezar a ubicarlos con algo de margen
    int xinicial=inicio_x_zxdesktop+24;

    int xfinal=screen_get_total_width_window_plus_zxdesktop_no_zoom()-ZESARUX_ASCII_LOGO_ANCHO;

    //yinicial debajo de botones superiores
    int alto_boton;
    menu_ext_desktop_buttons_get_geometry(NULL,&alto_boton,NULL,NULL,NULL);
    alto_boton /=zoom_y;


    int yinicial=alto_boton+16;



    //Hasta llegar a los iconos de dispositivos inferiores
    int yfinal;
    menu_ext_desktop_lower_icons_get_geometry(NULL,NULL,NULL,NULL,NULL,&yfinal);
    //Posiciones menos el zoom
    yfinal /=zoom_y;
    //Consideramos el tamanyo del icono (ZESARUX_ASCII_LOGO_ANCHO) para que no se pueda ubicar medio icono fuera de rango por ejemplo
    yfinal -=ZESARUX_ASCII_LOGO_ANCHO;

    */

    int xinicial,xfinal,yinicial,yfinal;


    zxvision_get_start_valid_positions_icons(&xinicial,&xfinal,&yinicial,&yfinal);


    int x,y;

    //empezar buscando huecos donde haya 0 iconos cerca. Si no hay, buscar con 1, con 2... hasta 5
    int buscar_cerca;

    for (buscar_cerca=0;buscar_cerca<5;buscar_cerca++) {

        //printf("buscar_cerca %d\n",buscar_cerca);
        //Desplazarlos un poco cada vez
        for (y=yinicial+ZXVISION_OFFSET_ICONOS_REPETIR_POSICION*buscar_cerca;y<yfinal;y+=ZXVISION_SEPARACION_ICONOS_AL_ORDENAR) {
            for (x=xinicial+ZXVISION_OFFSET_ICONOS_REPETIR_POSICION*buscar_cerca;x<xfinal;x+=ZXVISION_SEPARACION_ICONOS_AL_ORDENAR) {
                int total_iconos_cerca=zxvision_si_icono_cerca(x,y);
                //printf("%d %d iconos cerca %d\n",x,y,total_iconos_cerca);

                //Si hay menos iconos cerca de los que esperamos y si esa posicion es valida
                if (total_iconos_cerca<=buscar_cerca && zxvision_if_configurable_icon_on_valid_position(x,y) ) {
                    *p_x=x;
                    *p_y=y;
                    return;
                }
            }
        }

    }


    //Si no hay hueco, mala suerte... lo ponemos en posicion inicial
    debug_printf(VERBOSE_DEBUG,"zxvision_get_next_free_icon_position: No free space. Fallback to initial position %d %d",xinicial,yinicial);
    *p_x=xinicial;
    *p_y=yinicial;
}

void zxvision_reorder_configurable_icons(void)
{

    /*
    Se reubican iconos empezando en una zona de forma cuadrada delimitada por:
    x: desde derecha pantalla emulada hasta final x
    y: desde debajo botones superiores hasta por encima botones inferiores
    Las funciones zxvision_get_next_free_icon_position y zxvision_get_next_free_icon_position utilizan
    la misma separación de posicionamiento de iconos,
    solo que zxvision_get_next_free_icon_position busca el siguiente hueco libre mientras que zxvision_reorder_configurable_icons
    reordena uno a uno cada icono
    */


/*
    int inicio_x_zxdesktop=screen_get_emulated_display_width_no_zoom_border_en();

    //Empezar a ubicarlos con algo de margen
    int xinicial=inicio_x_zxdesktop+24;

    int xfinal=screen_get_total_width_window_plus_zxdesktop_no_zoom()-ZESARUX_ASCII_LOGO_ANCHO;

    //yinicial debajo de botones superiores
    int alto_boton;

    menu_ext_desktop_buttons_get_geometry(NULL,&alto_boton,NULL,NULL,NULL);
    alto_boton /=zoom_y;


    int yinicial=alto_boton+16;



    //Hasta llegar a los iconos de dispositivos inferiores
    int yfinal;
    menu_ext_desktop_lower_icons_get_geometry(NULL,NULL,NULL,NULL,NULL,&yfinal);
    //Posiciones menos el zoom
    yfinal /=zoom_y;
    //Consideramos el tamanyo del icono (ZESARUX_ASCII_LOGO_ANCHO) para que no se pueda ubicar medio icono fuera de rango por ejemplo
    yfinal -=ZESARUX_ASCII_LOGO_ANCHO;

*/
    int xinicial,xfinal,yinicial,yfinal;


    zxvision_get_start_valid_positions_icons(&xinicial,&xfinal,&yinicial,&yfinal);




    int i;

    int x=xinicial;
    int y=yinicial;

    int separacion_iconos=ZXVISION_SEPARACION_ICONOS_AL_ORDENAR;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_EXISTS) {
            zxvision_set_configurable_icon_position(i,x,y);
            x +=separacion_iconos;
            if (x>=xfinal) {
                x=xinicial;
                y+=separacion_iconos;

                //Final de pantalla. Ubicamos al principio algo desplazados
                //TODO: puede que xinicial o yinicial acaben saliendose de rango,
                //aunque esto no es un problema al final, al llamarse a zxvision_set_configurable_icon_position no se permite
                //establecer una posicion invalida; en ese caso, se quedaria la misma posicion que tenia el icono

                if (y>=yfinal) {
                    xinicial+=ZXVISION_OFFSET_ICONOS_REPETIR_POSICION;
                    x=xinicial;

                    yinicial+=ZXVISION_OFFSET_ICONOS_REPETIR_POSICION;
                    y=yinicial;
                }
            }
        }
    }
}


void zxvision_reorder_configurable_icons_if_auto(void)
{
    //Si desactivado zx desktop
    if (!zxdesktop_configurable_icons_enabled_and_visible()) return;

    if (zxdesktop_automatic_reorder_icons.v) {
        zxvision_reorder_configurable_icons();
    }
}

//Comprobar y reajustar posiciones (si conviene) en caso de por ejemplo cambios de maquina (de spectrum a tbblue por ejemplo)
//donde iconos pueden ir a parar a debajo de pantalla emulada
void zxvision_check_all_configurable_icons_positions(void)
{

    //Si desactivado
    if (!zxdesktop_configurable_icons_enabled_and_visible()) return;

    //printf("zxvision_check_all_configurable_icons_positions\n");

    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_EXISTS) {
            int x=zxdesktop_configurable_icons_list[i].pos_x;
            int y=zxdesktop_configurable_icons_list[i].pos_y;

            if (!zxvision_if_configurable_icon_on_valid_position(x,y)) {
                debug_printf(VERBOSE_DEBUG,"Relocate icon %d (%s) because it is on an invalid position %d,%d",i,zxdesktop_configurable_icons_list[i].text_icon,x,y);
                zxvision_get_next_free_icon_position(&x,&y);
                debug_printf(VERBOSE_DEBUG,"Relocate icon %d to %d,%d",i,x,y);
                zxvision_set_configurable_icon_position(i,x,y);
            }
        }
    }

    zxvision_reorder_configurable_icons_if_auto();
}

//Agregar nuevo icono indicandole indice a tabla de acciones
int zxvision_add_configurable_icon(int indice_funcion)
{

    //Si desactivado
    if (zxdesktop_configurable_icons_enabled.v==0) return -1;

    //buscar el primero disponible
    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_NOT_EXISTS) {
            zxdesktop_configurable_icons_list[i].indice_funcion=indice_funcion;
            zxdesktop_configurable_icons_list[i].status=ZXDESKTOP_CUSTOM_ICON_EXISTS;

            //Texto de momento el de la accion asociada
            strcpy(zxdesktop_configurable_icons_list[i].text_icon,defined_direct_functions_array[indice_funcion].texto_funcion);


            int icon_x,icon_y;

            zxvision_get_next_free_icon_position(&icon_x,&icon_y);

            zxvision_set_configurable_icon_position(i,icon_x,icon_y);

            zxvision_reorder_configurable_icons_if_auto();

            return i;
        }
    }

    //no hay sitio. error
    debug_printf(VERBOSE_ERR,"Can not add more icons, probably limit reached: %d",MAX_ZXDESKTOP_CONFIGURABLE_ICONS);
    return -1;


}

//Agregar nuevo icono indicandole indice a tabla de acciones, pero sin asignar x,y automatica, util para llamarse desde
//parseo de parametros de configuracion
int zxvision_add_configurable_icon_no_add_position(int indice_funcion)
{

    //Si desactivado
    if (zxdesktop_configurable_icons_enabled.v==0) return -1;

    //buscar el primero disponible
    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_NOT_EXISTS) {
            zxdesktop_configurable_icons_list[i].indice_funcion=indice_funcion;
            zxdesktop_configurable_icons_list[i].status=ZXDESKTOP_CUSTOM_ICON_EXISTS;

            //Texto de momento el de la accion asociada
            strcpy(zxdesktop_configurable_icons_list[i].text_icon,defined_direct_functions_array[indice_funcion].texto_funcion);

            return i;
        }
    }

    //no hay sitio. error
    debug_printf(VERBOSE_ERR,"Can not add more icons, probably limit reached: %d",MAX_ZXDESKTOP_CONFIGURABLE_ICONS);
    return -1;


}

//Función auxiliar para asignar parametros adicionales a iconos, botones a acciones, o f funciones,
//Dependiendo del tipo de accion, si es por ejemplo SET_MACHINE, indicar parametro de maquina actual
//Parecido a zxdesktop_add_extra_parameters_element_action pero aqui es todo automatico y no se pide nada al usuario
void zxvision_add_extra_parameters_element_action(enum defined_f_function_ids accion, char *parametros, char *nombre_icono)
{
    if (accion==F_FUNCION_SET_MACHINE) {
        char buffer_maquina[100];

        get_machine_config_name_by_number(buffer_maquina,current_machine_type);
        if (buffer_maquina[0]!=0) {
            strcpy(parametros,buffer_maquina);

            if (nombre_icono!=NULL) {
                //Cambiar nombre icono con la maquina en cuestión
                sprintf(nombre_icono,"Set %s",get_machine_name(current_machine_type));
            }

        }
    }


    //Cualquier otra acción, dejar los parámetros en blanco para no arrastrar parámetros
    //de acciones anteriores que si que tenian parámetros
    //Esto podria haber ocasionado que si primero configuramos accion set_machine, y luego la cambiamos
    //a machine selection, ese machine selection no pide parámetros pero como arrastramos un parámetro
    //anterior, al disparar machine selection, como la gestión de la acción es la misma que set_machine, se le pasaría
    //ese parámetro "heredado" y no preguntaria la maquina
    else {
        //printf("Limpiar parametros\n");
        parametros[0]=0;
    }
}

//Agregar nuevo icono indicandole id de accion
int zxvision_add_configurable_icon_by_id_action(enum defined_f_function_ids id_funcion)
{
    //Crear un icono
    int indice_accion=zxvision_get_id_direct_funcion_index(id_funcion);
    return zxvision_add_configurable_icon(indice_accion);
}

//Agregar nuevo icono indicandole id de accion y detectando parametros extra segun tipo de icono
void zxvision_add_configurable_icon_by_id_action_and_auto_extra(enum defined_f_function_ids id_funcion)
{
    int indice_icono=zxvision_add_configurable_icon_by_id_action(id_funcion);


    if (indice_icono>=0) {
        //Asignar parametros extra segun el tipo de accion

        zxvision_add_extra_parameters_element_action(id_funcion,
            zxdesktop_configurable_icons_list[indice_icono].extra_info,
            zxdesktop_configurable_icons_list[indice_icono].text_icon);

    }
}


//Crear un icono con parametros "nombre" y con extra_info
void zxvision_create_configurable_icon(enum defined_f_function_ids id_funcion,char *nombre,char *extra_info)
{
    if (if_zxdesktop_enabled_and_driver_allows() ) {
        int indice_icono=zxvision_add_configurable_icon_by_id_action(id_funcion);

        if (indice_icono>=0) {

            strcpy(zxdesktop_configurable_icons_list[indice_icono].extra_info,extra_info);
            //Agregarle texto

            strcpy(zxdesktop_configurable_icons_list[indice_icono].text_icon,nombre);
        }
    }
}

//Crear un icono con parametros "nombre" y de nombre del icono obtener ese nombre sin path
//usado especialmente en creacion de iconos de tipo file link
void zxvision_create_configurable_icon_file_type(enum defined_f_function_ids id_funcion,char *nombre)
{

    char name_no_dir[PATH_MAX];
    util_get_file_no_directory(nombre,name_no_dir);

    zxvision_create_configurable_icon(id_funcion,name_no_dir,nombre);
}

//Crear un icono con parametros "nombre" y de nombre del icono obtener ese nombre sin path
//usado especialmente en creacion de iconos de tipo file link
void old_zxvision_create_configurable_icon_file_type(enum defined_f_function_ids id_funcion,char *nombre)
{
    if (if_zxdesktop_enabled_and_driver_allows() ) {
        int indice_icono=zxvision_add_configurable_icon_by_id_action(id_funcion);

        if (indice_icono>=0) {
            //Indicarle la ruta al snapshot
            strcpy(zxdesktop_configurable_icons_list[indice_icono].extra_info,nombre);
            //Agregarle texto
            char name_no_dir[PATH_MAX];
            util_get_file_no_directory(nombre,name_no_dir);

            strcpy(zxdesktop_configurable_icons_list[indice_icono].text_icon,name_no_dir);
        }
    }
}

//Crear icono en el escritorio que abre ventana indicada
void zxvision_create_link_desktop_from_window(zxvision_window *w)
{

    zxvision_create_configurable_icon(F_FUNCION_OPEN_WINDOW,w->window_title,w->geometry_name);
    menu_draw_ext_desktop();

}

//Indicar todos los iconos como no presentes
//Se inicializan incluso aunque no tengamos habilitado iconos en el zx desktop
void init_zxdesktop_configurable_icons(void)
{
    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        zxdesktop_configurable_icons_list[i].status=ZXDESKTOP_CUSTOM_ICON_NOT_EXISTS;

        //extra info en blanco
        zxdesktop_configurable_icons_list[i].extra_info[0]=0;

        //text icon en blanco
        zxdesktop_configurable_icons_list[i].text_icon[0]=0;
    }
}

//Si estan habilitados los iconos y ademas esta zx desktop activo (y por tanto se ven)
int zxdesktop_configurable_icons_enabled_and_visible(void)
{
    if (zxdesktop_configurable_icons_enabled.v==0) return 0;

    //Fixed: TODO: realmente deberia llamar a if_zxdesktop_enabled_and_driver_allows() pero esa llamada depende del driver de video
    //y al iniciar el emulador hay algun punto en que aun no hay driver de video y esta llamada petaria
    //por tanto esto podria dar un falso positivo: el zx desktop esta habilitado aunque el driver no lo permite,
    //por ejemplo si lo tenemos habilitado con driver Xwindow o cocoa, pero luego arrancamos curses (donde no se permite zx desktop)
    //Tampoco es un gran problema porque al no estar zx desktop permitido en curses, no se intentarian dibujar los iconos, y aunque se intentasen
    //dibujar, las funciones de putpixel no hacen nada. Lo unico que podria intentarse gestionar los eventos de raton si se elige algun icono,
    //pero tampoco tampoco se gestionarian al no poderse usar el raton en driver curses
    //if (!screen_ext_desktop_enabled) return 0;


    if (!if_zxdesktop_enabled_and_driver_allows()) return 0;


    return 1;
}

//Crear algunos iconos por defecto
void create_default_zxdesktop_configurable_icons(void)
{

    if (!zxdesktop_configurable_icons_enabled_and_visible()) {
        debug_printf(VERBOSE_INFO,"Do not create default icons because icons on zx desktop are disabled or zx desktop disabled");
        return;
    }

    //solo asignar iconos si no hay ninguno cargado de configuracion
    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status!=ZXDESKTOP_CUSTOM_ICON_NOT_EXISTS) {
            debug_printf(VERBOSE_INFO,"Do not create default icons as there are already at least one created");
            return;
        }
    }

    debug_printf(VERBOSE_INFO,"Creating default icons");

    int indice_icono;



    //My Machine
    int indice_my_machine=zxvision_add_configurable_icon_by_id_action(F_FUNCION_DESKTOP_MY_MACHINE);
    //Quicksave
    zxvision_add_configurable_icon_by_id_action(F_FUNCION_QUICKSAVE);


    //Add Trash
    indice_icono=zxvision_add_configurable_icon_by_id_action(F_FUNCION_DESKTOP_TRASH);

    if (indice_icono>=0) {
        strcpy(zxdesktop_configurable_icons_list[indice_icono].text_icon,"Trash Can");

        //Solo alterar y
        //La x pillarla del primer icono
        int x;
        if (indice_my_machine>=0) {
            x=zxdesktop_configurable_icons_list[indice_my_machine].pos_x;
        }
        else {
            //Cosa extraña. primer icono no valido. pues dejar la x tal cual
            x=zxdesktop_configurable_icons_list[indice_icono].pos_x;
        }

        //La papelera queda a la posicion y algo por encima de los lower device icons
        int yinicio_botones;
        menu_ext_desktop_lower_icons_get_geometry(NULL,NULL,NULL,NULL,NULL,&yinicio_botones);
        yinicio_botones /=zoom_y;

        int separacion_y_iconos=ZESARUX_ASCII_LOGO_ANCHO*2*menu_gui_zoom;


        //printf("Y trash: %d\n",yinicio_botones);
        zxvision_set_configurable_icon_position(indice_icono,x,yinicio_botones-separacion_y_iconos);
    }


}





//Dice si algun icono custom en el escritorio es la papelera
//-1 si no
int zxvision_search_trash_configurable_icon(void)
{
    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_EXISTS) {
            //id de la tabla de acciones
            int id_tabla=zxdesktop_configurable_icons_list[i].indice_funcion;

            enum defined_f_function_ids id_funcion=defined_direct_functions_array[id_tabla].id_funcion;

            if (id_funcion==F_FUNCION_DESKTOP_TRASH) return i;
        }
    }

    return -1;
}

int if_zxdesktop_trash_not_empty(void)
{
    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_DELETED) {
            return 1;
        }
    }

    return 0;
}

void zxvision_move_configurable_icon_to_trash(int indice_icono)
{
    zxdesktop_configurable_icons_list[indice_icono].status=ZXDESKTOP_CUSTOM_ICON_DELETED;

    zxvision_reorder_configurable_icons_if_auto();
}

void zxvision_recover_configurable_icon_from_trash(int indice_icono)
{
    zxdesktop_configurable_icons_list[indice_icono].status=ZXDESKTOP_CUSTOM_ICON_EXISTS;
}

void zxvision_empty_trash(void)
{

    debug_printf(VERBOSE_DEBUG,"Emptying trash");

    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_DELETED) {
            zxdesktop_configurable_icons_list[i].status=ZXDESKTOP_CUSTOM_ICON_NOT_EXISTS;
        }
    }
}

void menu_speech_reset_tecla_pulsada(void)
{
    menu_speech_tecla_pulsada=0;
    //printf("Reset tecla pulsada desde:\n");
    //debug_exec_show_backtrace();
}


void menu_speech_set_tecla_pulsada(void)
{

    //printf("Set tecla pulsada desde:\n");
    //debug_exec_show_backtrace();
    menu_speech_tecla_pulsada=1;

}

//Si el abrir menu (tipica F5 o tecla joystick) esta limitado. De tal manera que para poderlo abrir habra que pulsar 3 veces seguidas en menos de 1 segundo
z80_bit menu_limit_menu_open={0};



//OSD teclado aventura
/*
//numero maximo de entradas
#define MAX_OSD_ADV_KEYB_WORDS 40
//longitud maximo de cada entrada
#define MAX_OSD_ADV_KEYB_TEXT_LENGTH 20
*/


//algunas entradas definidas de ejemplo
int osd_adv_kbd_defined=9;
char osd_adv_kbd_list[MAX_OSD_ADV_KEYB_WORDS][MAX_OSD_ADV_KEYB_TEXT_LENGTH]={
    "~~north",
    "~~west",
    "~~east",
    "~~south",
    "loo~~k",  //5
    "e~~xamine",
    "~~help",
    "~~talk",
    "ex~~it"
};

//Retorna indice a funcion segun su nombre en texto. -1 si no encontrado
int get_defined_direct_functions(char *funcion)
{
    int i;

    for (i=0;i<MAX_F_FUNCTIONS;i++) {
        if (!strcasecmp(funcion,defined_direct_functions_array[i].texto_funcion)) {
            //enum defined_f_function_ids id=defined_direct_functions_array[i].id_funcion;
            return i;
        }
    }

    return -1;
}

//Definir una tecla a una funcion
//Entrada: tecla: 1...15 F1...15   funcion: string correspondiente a defined_f_functions_array
//Devuelve 0 si ok
int menu_define_key_function(int tecla,char *funcion)
{
    if (tecla<1 || tecla>MAX_F_FUNCTIONS_KEYS) return 1;

    //Buscar en todos los strings de funciones cual es

    //int i;

    int indice=get_defined_direct_functions(funcion);
    if (indice<0) return 1;

    defined_f_functions_keys_array[tecla-1]=indice;

    return 0;
}

//Definir una tecla a una extra info de una funcion
//Entrada: tecla: 1...15 F1...15
//Devuelve 0 si ok
int menu_define_key_function_extra_info(int tecla,char *extra_info)
{
    if (tecla<1 || tecla>MAX_F_FUNCTIONS_KEYS) return 1;



    strcpy(defined_f_functions_keys_array_parameters[tecla-1],extra_info);

    return 0;
}

//Definir una boton a una funcion
//Entrada: buton 0...   funcion: string correspondiente a defined_f_functions_array
//Devuelve 0 si ok
int menu_define_button_function(int boton,char *funcion)
{
    if (boton<0 || boton>=MAX_USERDEF_BUTTONS) return 1;

    //Buscar en todos los strings de funciones cual es

    int i;

    for (i=0;i<MAX_F_FUNCTIONS;i++) {
        if (!strcasecmp(funcion,defined_direct_functions_array[i].texto_funcion)) {
            //enum defined_f_function_ids id=defined_direct_functions_array[i].id_funcion;
            defined_buttons_functions_array[boton]=i;
            return 0;
        }
    }

    return 1;
}

//funcion activa de overlay
void (*menu_overlay_function)(void);

//buffer de escritura por pantalla
overlay_screen overlay_screen_array[OVERLAY_SCREEN_MAX_WIDTH*OVERLAY_SCREEN_MAX_HEIGTH];

//buffer de escritura de footer
overlay_screen footer_screen_array[WINDOW_FOOTER_COLUMNS*WINDOW_FOOTER_LINES];


//buffer de texto usado
//int overlay_usado_screen_array[OVERLAY_SCREEN_MAX_WIDTH*OVERLAY_SCREEN_MAX_HEIGTH];

//Indica que hay una segunda capa de texto por encima de menu y por encima del juego incluso
//util para mostrar indicadores de carga de cinta, por ejemplo
//int menu_second_layer=0;

//Footer activado por defecto
int menu_footer=1;

//se activa second layer solo para un tiempo limitado
//int menu_second_layer_counter=0;

//buffer de escritura de segunda capa
//overlay_screen second_overlay_screen_array[32*24];



//Si el menu esta desactivado completamente. Si es asi, cualquier evento que abra el menu, no hará nada
z80_bit menu_desactivado={0};

//Si el menu esta desactivado completamente. Si es asi, cualquier evento que abra el menu, provocará la salida del emulador
z80_bit menu_desactivado_andexit={0};

//Si el menu de file utilities esta deshabilitado
z80_bit menu_desactivado_file_utilities={0};

//indica que el menu aparece en modo multitarea - mientras ejecuta codigo de emulacion de cpu
int menu_multitarea=1;

//indica que aunque no hay multitarea se leen los timers
//int menu_enable_timers_without_multitask=0;

//emulacion en menu esta pausada
int menu_emulation_paused_on_menu=0;

//emulacion en menu esta pausada debido a que Debug cpu se ha puesto en step mode
int menu_emulation_paused_on_menu_by_debug_step_mode=0;


//Si se oculta la barra vertical en la zona de porcentaje de ventanas de texto o selector de archivos
z80_bit menu_hide_vertical_percentaje_bar={0};

//Si se oculta el indicador de submenu ">"
z80_bit menu_hide_submenu_indicator={0};

//Si se oculta boton de minimizar ventana
z80_bit menu_hide_minimize_button={0};

//Si se oculta boton de maximizar ventana
z80_bit menu_hide_maximize_button={0};

//Si se oculta boton de cerrar ventana
z80_bit menu_hide_close_button={0};

//Si se ocultan ventanas minimizadas
z80_bit zxvision_hide_minimized_windows={0};

//Si se permite salir a las ventanas de la zona del zx desktop visible
z80_bit zxvision_allow_windows_beyond_limit={0};

//Si se oculta boton de background ventana en ventana no activa (cuando deberia parpadear)
//Quiero que por defecto esté oculto
z80_bit menu_hide_background_button_on_inactive={1};

//Si se cambia color marco ventana al pasar raton por zona de redimensionado
z80_bit menu_change_frame_when_resize_zone={1};

//Si se invierte sentido movimiento scroll raton
z80_bit menu_invert_mouse_scroll={0};

//Boton derecho no hace ESC
z80_bit menu_mouse_right_send_esc={0};

//Viejo comportamiento de tecla ESC en menu y de cierre ventanas:
// Tecla ESC siempre lleva a menu anterior
// Si se cierra una ventana que sale de un menu, se vuelve al menu que lo ha creado
// Flecha izquierda o derecha no hace nada (en menus no tabulados)
//El nuevo comportamiento (por defecto en ZEsarUX X):
// Tecla ESC cierra todos menus
// Si se cierra una ventana que sale de un menu, se cerraran todos menus
// Flecha izquierda lleva al menu anterior, flecha derecha es como enter (esto en menus no tabulados)
z80_bit menu_old_behaviour_close_menus={0};

//Indica que al abrir submenus, se ven los menus anteriores
z80_bit menu_show_submenus_tree={1};

//indica que se ha pulsado ESC y por tanto debe aparecer el menu, o gestion de breakpoints, osd, etc
//y tambien, la lectura de puertos de teclado (254) no devuelve nada
int menu_abierto=0;

//Si se tiene overlay menu aunque este con menu cerrado. De momento solo accesible mediante tecla F
//de F_FUNCION_OVERLAY_WINDOWS
//Nota: logicamente cualquier mensaje de splash (por ejemplo cambio de modo ulaplus)
//ara saltar el mensaje splash y al cabo de unos segundos, desactivara overlay y se ocultara todo el overlay
//Esto NO abre el menu, solo deja el overlay de menu activo
int overlay_visible_when_menu_closed=0;

//Hacer autoframeskip al moverse ventanas
//Esto permite, si se desactiva, ver las ventanas siempre que se muevan, pero a costa de usar mas cpu y puede que ralentizar la emulacion al moverse
//las ventanas si la cpu ya va saturada
z80_bit auto_frameskip_even_when_movin_windows={1};

//Hacer autoframeskip al dibujar fondo zx deskop
z80_bit frameskip_draw_zxdesktop_background={1};

//Decir que el siguiente refresco de zx desktop no se hara frameskip
z80_bit no_next_frameskip_draw_zxdesktop_background={0};

//Aunque este siempre se asignara primero, pero por si acaso le damos valor por defecto
z80_bit autoframeskip_setting_before_moving_windows={1};

//Si realmente aparecera el menu
z80_bit menu_event_open_menu={0};

//Si el menu se ha abierto con boton izquierdo
z80_bit menu_was_open_by_left_mouse_button={0};

//Si el menu se ha abierto con boton derecho
z80_bit menu_was_open_by_right_mouse_button={0};

//indica si hay pendiente un mensaje de error por mostrar
int if_pending_error_message=0;

//mensaje de error pendiente a mostrar
char pending_error_message[1024];

//Indica que hay que salir de todos los menus. Esto sucede, por ejemplo, al cargar snapshot
int salir_todos_menus;

//char *welcome_message_key=" Press ESC for menu ";

char esc_key_message[10]="ESC";
char *openmenu_key_message="F5/Button";

//Si desde el gestor de ventanas, una ventana se ha ido a background
//Eso sucede al pulsar F6, boton de background, o conmutar a otra ventana
int menu_window_manager_window_went_background=0;

//Gestionar pulsaciones directas de teclado o joystick
//para quickload
z80_bit menu_button_smartload={0};
//para on screen keyboard
z80_bit menu_button_osdkeyboard={0};
z80_bit menu_button_osdkeyboard_return={0};

//Retorno de envio de una tecla
z80_bit menu_button_osd_adv_keyboard_return={0};
//Abrir el menu de Adventure text
z80_bit menu_button_osd_adv_keyboard_openmenu={0};

//Comun para zx8081 y spectrum
//z80_bit menu_button_osdkeyboard_caps={0};
//Solo para spectrum
//z80_bit menu_button_osdkeyboard_symbol={0};
//Solo para zx81
//z80_bit menu_button_osdkeyboard_enter={0};

z80_bit menu_button_exit_emulator={0};

z80_bit menu_event_drag_drop={0};

z80_bit menu_event_pending_zmenu_file_menu_open={0};
z80_bit menu_event_open_zmenu_file={0};
//Ruta al archivo de menu que se va a cargar
char menu_open_zmenu_file_path[PATH_MAX]="";

z80_bit menu_event_pending_drag_drop_menu_open={0};

z80_bit menu_event_new_version_show_changes={0};

z80_bit menu_event_new_update={0};

z80_bit menu_button_f_function={0};

//tecla f pulsada
int menu_button_f_function_index;
//accion de tecla f simulada desde joystick
int menu_button_f_function_action=0;

//char menu_event_drag_drop_file[PATH_MAX];

//Para evento de entrada en paso a paso desde remote protocol
z80_bit menu_event_remote_protocol_enterstep={0};

//mostrando easter egg desde ZRCP
z80_bit zrcp_easter_egg_running={0};

//Si menus de confirmacion asumen siempre yes y no preguntan nunca
z80_bit force_confirm_yes={0};

//Si raton no tiene accion sobre el menu
z80_bit mouse_menu_disabled={0};

//Si pulsar un boton de mouse no activa el menu
z80_bit mouse_menu_ignore_click_open={0};


//Se ha pulsado tecla de menu cuando menu esta abierto
z80_bit menu_pressed_open_menu_while_in_menu={0};

//Si se pulsa tecla que simula F5 + ESC (y por tanto cierra todos menus)
z80_bit menu_pressed_close_all_menus={0};

//En que boton se ha pulsado del menu
int menu_pressed_zxdesktop_button_which=-1;
//Que se ha pulsado con boton derecho
int menu_pressed_zxdesktop_button_which_right_button=0;
//En que lower icon se ha pulsado del menu
int menu_pressed_zxdesktop_lower_icon_which=-1;
//Que se ha pulsado con boton derecho
int menu_pressed_zxdesktop_lower_icon_which_right_button=0;
//En que icono configurable se ha pulsado del menu
int menu_pressed_zxdesktop_configurable_icon_which=-1;
//Se ha pulsado en boton derecho sobre el desktop
int menu_pressed_zxdesktop_right_button_background=-1;

//Si se ha pulsado boton derecho en un icono
int menu_pressed_zxdesktop_configurable_icon_right_button=0;

//Posicion donde se ha empezado a pulsar icono configurable. para saber si se arrastra
int menu_pressed_zxdesktop_configurable_icon_where_x=99999;
int menu_pressed_zxdesktop_configurable_icon_where_y=99999;

//Que el siguiente menu se ha abierto desde boton y por tanto hay que ajustar coordenada x,y
z80_bit force_next_menu_position={0};
//int force_next_menu_position_which=0;
int force_next_menu_position_x=0;
int force_next_menu_position_y=0;

//z80_bit menu_zxdesktop_buttons_enabled={1};
z80_bit menu_zxdesktop_upper_buttons_enabled={1};
z80_bit menu_zxdesktop_lower_buttons_enabled={1};

z80_bit zxdesktop_switch_button_enabled={1};


z80_bit no_close_menu_after_smartload={0};

//Modo navidad: cambios en los colores del logo (tanto footer como splash), mensaje bienvenida, copos de nieve en logo del footer
z80_bit christmas_mode={0};

//Evitar modo navidad, pensado para máquinas sin reloj RTC y que siempre están en 1 de Enero
z80_bit avoid_christmas_mode={0};


void menu_dibuja_cuadrado(int x1,int y1,int x2,int y2,int color,int color_marca_redimensionado);
void menu_desactiva_cuadrado(void);
void menu_establece_cuadrado(int x1,int y1,int x2,int y2,int color);


void menu_util_cut_line_at_spaces(int posicion_corte, char *texto,char *linea1, char *linea2);


void menu_espera_tecla_timeout_tooltip(void);
z80_byte menu_da_todas_teclas(void);
void menu_espera_tecla_cualquiera(void);

void zxvision_helper_menu_shortcut_print(int tecla);
void zxvision_helper_menu_shortcut_delete_last(void);
void zxvision_helper_menu_shortcut_init(void);

void zxvision_set_all_flag_dirty_must_draw_contents(void);


//Gestión de conmutar entre ventanas mediante shift+left/right
/*
Es una funcionalidad que no me acaba de gustar por varias razones:
1) Tal y como se conmuta entre una ventana a la otra, hace difícil mostrarlas por orden cuando se cambia de una a la otra
2) Se simula evento de pulsado de botón de mouse cuando se intenta conmutar desde una ventana que no permite background,
esto hace un tanto complicado liberar esa pulsación de botón que igual en un futuro puede dar problemas
*/
int menu_pressed_shift_left=0;
int menu_pressed_shift_right=0;
int menu_pressed_shift_cursor_window_doesnot_allow=0;
//Contador que indica cuantas veces hay que conmutar de ventana al pulsar shift+left/right
//En zxvision_handle_mouse_events está explicada la problemática con esto
int menu_contador_conmutar_ventanas=0;




int menu_simple_two_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2);
void zxvision_handle_mouse_ev_switch_back_wind(zxvision_window *ventana_pulsada);





void menu_process_f_functions_by_action_index(int accion,int si_pulsado_boton_redefinido,int boton);

z80_byte menu_retorna_caracter_background(void);

//si hay recuadro activo, y cuales son sus coordenadas y color

int cuadrado_activo=0;
int cuadrado_x1,cuadrado_y1,cuadrado_x2,cuadrado_y2,cuadrado_color;

//Y si dicho recuadro tiene marca de redimensionado posible para zxvision
int cuadrado_activo_resize=0;

//Indica que ventana activa tiene la marca de redimensionado con aviso
//Esto sucede cuando se ha escrito mas alla del tamanyo de la ventana
int cuadrado_activo_marca_redimensionado_aviso=0;

//Indica que la ventana activa tiene boton de minimizar
int ventana_activa_puede_minimizar=0;
//int ventana_activa_tipo_zxvision=0;

//Si estamos dibujando las ventanas de debajo de la del frente, y por tanto no muestra boton de cerrar por ejemplo
int ventana_es_background=0;

int draw_bateria_contador=0;
int draw_cpu_use=0;
int draw_cpu_temp=0;
int draw_fps=0;

//Portapapeles del menu
z80_byte *menu_clipboard_pointer=NULL;

//tamanyo del portapapeles
int menu_clipboard_size=0;

//Si driver de video soporta lectura de teclas F
int f_functions;

//Si hay que escribir las letras de atajos de menu en inverso. Esto solo sucede cuando salta el tooltip o cuando se pulsa una tecla
//que no es atajo
//NO se debe cambiar aqui directamente; la ventana que quiera desactivar esto, debe recurrir al parametro
//writing_inverse_color de la ventana
z80_bit menu_writing_inverse_color={0};

//Si forzar letras en color inverso siempre
z80_bit menu_force_writing_inverse_color={0};

//siempre empieza con 1 espacio de separacion en menu_escribe_linea_opcion excepto algunas ventanas que requieren mas ancho, como hexdump
int menu_escribe_linea_startx=1;


//Si se desactiva parseo caracteres especiales como ~~ o ^^ etc
//NO se debe cambiar aqui directamente; la ventana que quiera desactivar esto, debe recurrir al parametro
//disable_special_chars de la ventana
z80_bit menu_disable_special_chars={0};

//Colores franja speccy
int colores_franja_speccy_brillo[]={2+8,6+8,4+8,5+8};
int colores_franja_speccy_oscuro[]={2,6,4,5};

//Colores franja cpc. Ultima amarillo, porque son 3 barras y queremos que se confunda con el fondo
int colores_franja_cpc_brillo[]={2+8,4+8,1+8,6+8};
int colores_franja_cpc_oscuro[]={2,4,1,6+8};


int estilo_gui_activo=0;

estilos_gui definiciones_estilos_gui[ESTILOS_GUI]={
    //Este es una mezcla de Amiga, OS/2 y Atari
    {1,"ZEsarUX Plus",AMIGAOS_COLOUR_blue,ZESARUX_PLUS_COLOUR_WHITE,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        5+8,0, 		//Colores para opcion seleccionada
        AMIGAOS_COLOUR_blue,AMIGAOS_COLOUR_red,5+8,AMIGAOS_COLOUR_red, 	//Colores para opcion no disponible

        0,ZESARUX_PLUS_COLOUR_WHITE,        	//Colores para el titulo ventana
        0,              //Color recuadro
        ZESARUX_PLUS_COLOUR_WHITE,OSDOS_COLOUR_GRAY_INACTIVE,        	//Colores para el titulo ventana inactiva

        4,4+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        2,ZESARUX_PLUS_COLOUR_WHITE,		//Color para opcion marcada

        ZESARUX_PLUS_COLOUR_FILESELECTOR,0, //Colores fileselector, zona seleccion archivos

        154, //caracter de cerrar ventana
        171, //caracter de minimizar ventana
        172, //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        149, //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2+8, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        5, //-1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_ataritos, //char_set_dos,
        153 //caracter franja cambiado
    },


    {0,"ZEsarUX",7+8,0,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        5+8,0, 		//Colores para opcion seleccionada
        7+8,2,7,2, 	//Colores para opcion no disponible

        0,7+8,        	//Colores para el titulo ventana
        0,              //Color recuadro
        7+8,0,        	//Colores para el titulo ventana inactiva

        1,1+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    //Tema igual que ZEsarUX pero sin brillo. Y con color de visualmem cambiado
    {0,"ZEsarUX Matte",7,0,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        5,0, 		//Colores para opcion seleccionada
        7,2,5,2, 	//Colores para opcion no disponible

        0,7,        	//Colores para el titulo ventana
        0,              //Color recuadro
        7,0,        	//Colores para el titulo ventana inactiva

        1,1+8,		//Colores waveform
        4,		//Color para zona no usada en visualmem
        4,      //color block visualtape
        2,7,		//Color para opcion marcada

        6,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        //colores de franjas invertidas, por defecto la oscura
        colores_franja_speccy_oscuro,colores_franja_speccy_brillo,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },



    {0,"ZXSpectr",1,6,
        1,1,0,0,		//Mostrar cursor >, mostrar recuadro, no mostrar rainbow
        1+8,6,		//Colores para opcion seleccionada
        1,6,1,6,	//Colores para opcion no disponible, iguales que para opcion disponible

        6,1,		//Colores para el titulo ventana
        6,              //Color recuadro
        1,6,		//Colores para el titulo ventana inactiva

        4,4+8,		//Colores waveform
        7,               //Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    {0,"ManSoftware",7+8,0,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        5+8,0, 		//Colores para opcion seleccionada
        7+8,3,7,3, 	//Colores para opcion no disponible

        0,7+8,        	//Colores para el titulo ventana
        0,              //Color recuadro
        7+8,0,        	//Colores para el titulo ventana inactiva

        1,1+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        3,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '#',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        3+8, //color de aviso, en este tema, magenta con brillo
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_mansoftware,
        0 //caracter franja normal
    },


    {0,"CPC",1,6+8,
        0,1,1,0,          //No mostrar cursor,mostrar recuadro,mostrar rainbow
        6+8,1,            //Colores para opcion seleccionada
        1,2,6+8,2,        //Colores para opcion no disponible

        6+8,1,            //Colores para el titulo ventana
        6+8,              //Color recuadro
        1,6+8,            //Colores para el titulo ventana inactiva

        4,4+8,              //Colores waveform
        7,               //Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_cpc_brillo,colores_franja_cpc_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_cpc,
        0 //caracter franja normal
    },

        //Solo vale en video driver completo por los colores usados (primer valor de la estructura)
    {1,"MSX",VDP_9918_INDEX_FIRST_COLOR+4,VDP_9918_INDEX_FIRST_COLOR+15,
        0,1,0,0,          //No mostrar cursor,mostrar recuadro,no mostrar rainbow
        VDP_9918_INDEX_FIRST_COLOR+15,VDP_9918_INDEX_FIRST_COLOR+4,            //Colores para opcion seleccionada
        VDP_9918_INDEX_FIRST_COLOR+4,VDP_9918_INDEX_FIRST_COLOR+6,VDP_9918_INDEX_FIRST_COLOR+15,VDP_9918_INDEX_FIRST_COLOR+6,        //Colores para opcion no disponible

        VDP_9918_INDEX_FIRST_COLOR+15,VDP_9918_INDEX_FIRST_COLOR+4,            //Colores para el titulo ventana
        VDP_9918_INDEX_FIRST_COLOR+15,              //Color recuadro
        VDP_9918_INDEX_FIRST_COLOR+4,VDP_9918_INDEX_FIRST_COLOR+15,            //Colores para el titulo ventana inactiva

        VDP_9918_INDEX_FIRST_COLOR+2,VDP_9918_INDEX_FIRST_COLOR+3,              //Colores waveform
        7,               //Color para zona no usada en visualmem
        7,      //color block visualtape
        VDP_9918_INDEX_FIRST_COLOR+6,VDP_9918_INDEX_FIRST_COLOR+15,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '.',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso. Seria VDP_9918_INDEX_FIRST_COLOR+8 pero las franjas de volumen usan un formato $$ que solo permite color de 1 digito
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_msx,
        153 //caracter franja cambiado
    },

    {0,"QL",7+8,0,
        0,1,0,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        4+8,0, 		//Colores para opcion seleccionada
        7+8,2,7,2, 	//Colores para opcion no disponible

        2,7+8,        	//Colores para el titulo ventana
        2,              //Color recuadro
        7+8,2,        	//Colores para el titulo ventana inactiva

        4,4+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_ql,
        0 //caracter franja normal
    },

    {0,"Sam",7+8,0,
        0,1,1,0,                //No mostrar cursor,mostrar recuadro,mostrar rainbow
        5+8,0,          //Colores para opcion seleccionada
        7+8,2,7,2,      //Colores para opcion no disponible

        0,7+8,          //Colores para el titulo ventana
        0,              //Color recuadro
        7+8,0,          //Colores para el titulo ventana inactiva

        1,1+8,              //Colores waveform
        7,               //Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '#',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_sam,
        0 //caracter franja normal
    },

    {1,"Z88",Z88_PXCOLOFF,Z88_PXCOLON,
        0,1,0,0,                //No mostrar cursor,mostrar recuadro,no mostrar rainbow
        Z88_PXCOLON,Z88_PXCOLOFF,          //Colores para opcion seleccionada
        Z88_PXCOLOFF,Z88_PXCOLGREY,Z88_PXCOLON,Z88_PXCOLGREY,      //Colores para opcion no disponible

        Z88_PXCOLON,Z88_PXCOLOFF,          //Colores para el titulo ventana
        Z88_PXCOLON,              //Color recuadro
        Z88_PXCOLGREY,Z88_PXCOLOFF,          //Colores para el titulo ventana inactiva

        4,4+8,              //Colores waveform
        4,               //Color para zona no usada en visualmem
        6,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_z88,
        0 //caracter franja normal
    },

    {0,"ZX80/81",7+8,0,
        1,1,0,1,          //Mostrar cursor >, mostrar recuadro, no mostrar rainbow, solo mayusculas
        0,7+8,          //Colores para opcion seleccionada
        7+8,0,0,7+8,      //Colores para opcion no disponible

        0,7+8,          //Colores para el titulo ventana
        0,              //Color recuadro
        7+8,0,          //Colores para el titulo ventana inactiva

        4,4+8,              //Colores waveform
        7,               //Color para zona no usada en visualmem
        7,      //color block visualtape
        7,0,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '.',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        149, //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    {1,"AmigaOS",AMIGAOS_COLOUR_blue,7+8,
        0,1,0,0, 		//No mostrar cursor,mostrar recuadro,no mostrar rainbow
        0,AMIGAOS_COLOUR_orange, 		//Colores para opcion seleccionada
        AMIGAOS_COLOUR_blue,AMIGAOS_COLOUR_red,0,AMIGAOS_COLOUR_red, 	//Colores para opcion no disponible

        7+8,AMIGAOS_COLOUR_blue,        	//Colores para el titulo ventana
        7+8,              //Color recuadro
        7+8,AMIGAOS_COLOUR_inactive_title_ink,        	//Colores para el titulo ventana inactiva

        AMIGAOS_COLOUR_orange_dark,AMIGAOS_COLOUR_orange,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        AMIGAOS_COLOUR_red,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        141, //boton cerrar
        150, //caracter de minimizar ventana
        152, //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        149, //caracter de background ventana
        148, //caracter de fondo de titulo
        2+8, //color de aviso. Seria AMIGAOS_COLOUR_red pero las franjas de volumen usan un formato $$ que solo permite color entre 0-15
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_amigaos,
        153 //caracter franja cambiado
    },

    {1,"AtariTOS",ATARITOS_COLOUR_white,0,
        0,1,0,0, 		//No mostrar cursor,mostrar recuadro,no mostrar rainbow
        0,ATARITOS_COLOUR_white, 		//Colores para opcion seleccionada
        ATARITOS_COLOUR_white,2,0,2, 	//Colores para opcion no disponible

        ATARITOS_COLOUR_white,0,        	//Colores para el titulo ventana
        0,              //Color recuadro
        ATARITOS_COLOUR_white,0,        	//Colores para el titulo ventana inactiva

        ATARITOS_COLOUR_green_dark,ATARITOS_COLOUR_green,		//Colores waveform
        ATARITOS_COLOUR_green,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        2,ATARITOS_COLOUR_white,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        140, //boton cerrar
        '-', //caracter de minimizar ventana
        152, //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        149, //caracter de background ventana
        148, //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_ataritos,
        153 //caracter franja cambiado
    },

    {1,"BeOS",BEOS_COLOUR_grey_menu,0,
        0,1,0,0, 		//No mostrar cursor,mostrar recuadro,no mostrar rainbow,no mayusculas
        BEOS_COLOUR_grey_selection,0, 		//Colores para opcion seleccionada
        BEOS_COLOUR_grey_menu,2,BEOS_COLOUR_grey_selection,2, 	//Colores para opcion no disponible

        BEOS_COLOUR_yellow,0,        	//Colores para el titulo ventana
        BEOS_COLOUR_grey_box,              //Color recuadro
        BEOS_COLOUR_grey_inactive_title,0,        	//Colores para el titulo ventana inactiva

        1,1+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        139, //boton cerrar especial
        150, //caracter de minimizar ventana
        152, //caracter de maximizar ventana
        151, //caracter de restaurar ventana
        149, //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        BEOS_COLOUR_blue_hotkey, //si texto inverso solo cambia color tinta
        1, //NO rellenar titulo
        char_set_beos,
        0 //caracter franja normal
    },


    {1,"OS/2",OSDOS_COLOUR_GRAY,0,
        0,1,0,0, 		//No mostrar cursor,mostrar recuadro,no mostrar rainbow
        OSDOS_COLOUR_BLUE,7+8, 		//Colores para opcion seleccionada
        OSDOS_COLOUR_GRAY,OSDOS_COLOUR_GRAY_INACTIVE,OSDOS_COLOUR_BLUE,OSDOS_COLOUR_GRAY_INACTIVE, 	//Colores para opcion no disponible

        OSDOS_COLOUR_BLUE,7+8,        	//Colores para el titulo ventana
        0,              //Color recuadro
        OSDOS_COLOUR_GRAY_INACTIVE,OSDOS_COLOUR_GRAY,        	//Colores para el titulo ventana inactiva

        OSDOS_COLOUR_BLUE_DARK,OSDOS_COLOUR_BLUE,		//Colores waveform
        OSDOS_COLOUR_GRAY_INACTIVE,		//Color para zona no usada en visualmem
        OSDOS_COLOUR_GRAY_INACTIVE,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        140, //caracter de cerrar ventana
        150, //caracter de minimizar ventana
        152, //caracter de maximizar ventana
        151, //caracter de restaurar ventana
        149, //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_dos,
        153 //caracter franja cambiado
    },

    {1,"QNX",QNX_COLOUR_GRAY,0,
        0,1,0,0, 		//No mostrar cursor,mostrar recuadro,no mostrar rainbow
        QNX_COLOUR_BLUE,7+8, 		//Colores para opcion seleccionada
        QNX_COLOUR_GRAY,QNX_COLOUR_GRAY_INACTIVE,QNX_COLOUR_BLUE,QNX_COLOUR_GRAY_INACTIVE, 	//Colores para opcion no disponible

        QNX_COLOUR_GREEN,0,        	//Colores para el titulo ventana
        QNX_COLOUR_GRAY_INACTIVE,              //Color recuadro
        QNX_COLOUR_GRAY_INACTIVE,0,        	//Colores para el titulo ventana inactiva

        QNX_COLOUR_BLUE_DARK,QNX_COLOUR_BLUE,		//Colores waveform
        QNX_COLOUR_GRAY_INACTIVE,		//Color para zona no usada en visualmem
        QNX_COLOUR_GRAY_INACTIVE,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        170, //caracter de cerrar ventana
        169, //caracter de minimizar ventana
        139, //caracter de maximizar ventana
        152, //caracter de restaurar ventana
        149, //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        QNX_COLOUR_GREEN, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_beos,
        153 //caracter franja cambiado
    },

    {1,"RetroMac",RETROMAC_COLOUR_paper,0,
        0,1,0,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        RETROMAC_COLOUR_selected_paper,7+8, 		//Colores para opcion seleccionada
        RETROMAC_COLOUR_paper,RETROMAC_COLOUR_unavailable_ink,RETROMAC_COLOUR_selected_paper,RETROMAC_COLOUR_unavailable_ink, 	//Colores para opcion no disponible

        RETROMAC_COLOUR_active_title,0,        	//Colores para el titulo ventana
        RETROMAC_COLOUR_window_box,              //Color recuadro
        RETROMAC_COLOUR_paper,RETROMAC_COLOUR_unavailable_ink,    	//Colores para el titulo ventana inactiva

        4,4+8,		//Colores waveform
        5,		//Color para zona no usada en visualmem
        5+8,      //color block visualtape
        2,7,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        141, //boton cerrar
        150, //caracter de minimizar ventana
        152, //caracter de maximizar ventana
        151, //caracter de restaurar ventana
        149, //caracter de background ventana
        148, //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        5, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_retromac,
        153 //caracter franja cambiado
    },

    //Risc Os V.1 ("Arthur")
    {1,"RiscOS",7+8,RISCOS_COLOUR_DARKBLUE,
        0,1,0,0, 		//No mostrar cursor,mostrar recuadro,no mostrar rainbow
        RISCOS_COLOUR_DARKBLUE,7+8, 		//Colores para opcion seleccionada
        7+8,RISCOS_COLOUR_DARKBLUE,RISCOS_COLOUR_DARKBLUE,RISCOS_COLOUR_RED, 	//Colores para opcion no disponible

        RISCOS_COLOUR_RED,RISCOS_COLOUR_LIGHTBLUE,        	//Colores para el titulo ventana
        RISCOS_COLOUR_LIGHTBLUE,              //Color recuadro
        RISCOS_COLOUR_ORANGE,RISCOS_COLOUR_LIGHTBLUE,        	//Colores para el titulo ventana inactiva

        4,4+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        RISCOS_COLOUR_RED,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        155, //caracter de cerrar ventana
        '-', //caracter de minimizar ventana

        //En RiscOS originalmente tiene la misma apariencia el botón de maximizar y el de restaurar
        //aquí, para que no sea confuso, utilizo dos diferentes
        152, //caracter de maximizar ventana
        157, //caracter de restaurar ventana

        156, //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2+8, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //-1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_dos,
        153 //caracter franja cambiado
    },

    {0,"TempleOS",7+8,1,
        0,1,0,0, 		//No mostrar cursor,mostrar recuadro,no mostrar rainbow
        6+8,1, 		//Colores para opcion seleccionada
        7+8,2,7,2, 	//Colores para opcion no disponible

        7+8,1,        	//Colores para el titulo ventana
        1,              //Color recuadro
        7+8,1,        	//Colores para el titulo ventana inactiva

        4,4+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        'X',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        148, //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_templeos,
        0 //caracter franja normal
    },

    {0,"Borland",1,7+8,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        4,1, 		//Colores para opcion seleccionada
        1,7,7,1, 	//Colores para opcion no disponible

        7+8,0,        	//Colores para el titulo ventana
        7+8,              //Color recuadro
        7,0,        	//Colores para el titulo ventana inactiva

        4,4+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        2+8, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_dos,
        153 //caracter franja cambiado
    },

    {1,"TurboVision",TURBOVISION_COLOUR_white,TURBOVISION_COLOUR_black,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        TURBOVISION_COLOUR_green,TURBOVISION_COLOUR_black, 		//Colores para opcion seleccionada
        TURBOVISION_COLOUR_white,TURBOVISION_COLOUR_grey,TURBOVISION_COLOUR_green,TURBOVISION_COLOUR_grey, 	//Colores para opcion no disponible

        TURBOVISION_COLOUR_lightwhite,TURBOVISION_COLOUR_black,        	//Colores para el titulo ventana
        TURBOVISION_COLOUR_lightwhite,              //Color recuadro
        TURBOVISION_COLOUR_white,TURBOVISION_COLOUR_black,        	//Colores para el titulo ventana inactiva

        TURBOVISION_COLOUR_blue_dark,TURBOVISION_COLOUR_blue,		//Colores waveform
        TURBOVISION_COLOUR_cyan,		//Color para zona no usada en visualmem
        TURBOVISION_COLOUR_cyan,       //color block visualtape
        TURBOVISION_COLOUR_red,TURBOVISION_COLOUR_lightwhite,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        141,
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        149, //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso. Seria TURBOVISION_COLOUR_red pero las franjas de volumen usan un formato $$ que solo permite color de 1 digito
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        TURBOVISION_COLOUR_red, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_dos,
        153 //caracter franja cambiado
    },

    {0,"Bloody",2,7,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        1,7, 		//Colores para opcion seleccionada
        2,6,1,6, 	//Colores para opcion no disponible

        2+8,7+8,        	//Colores para el titulo ventana
        2+8,              //Color recuadro
        2,0,        	//Colores para el titulo ventana inactiva

        1,1+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        6, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    {0,"Grass",4,0,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        1,7, 		//Colores para opcion seleccionada
        4,6,1,6, 	//Colores para opcion no disponible

        4+8,0,        	//Colores para el titulo ventana
        4+8,              //Color recuadro
        4,7,        	//Colores para el titulo ventana inactiva

        1,1+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        4,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        6, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    {0,"Ocean",1,7,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        4,0, 		//Colores para opcion seleccionada
        1,2,5+8,2, 	//Colores para opcion no disponible

        1+8,7+8,        	//Colores para el titulo ventana
        1+8,              //Color recuadro
        1,7,        	//Colores para el titulo ventana inactiva

        4,4+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        1,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        6, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    {0,"Panther",3,7,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        1,7, 		//Colores para opcion seleccionada
        3,6,1,6, 	//Colores para opcion no disponible

        3+8,7+8,        	//Colores para el titulo ventana
        3+8,              //Color recuadro
        3,0,        	//Colores para el titulo ventana inactiva

        1,1+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        3,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        6, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    {0,"Sky",5,0,
        0,5,5,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        4,0, 		//Colores para opcion seleccionada
        5,2,4,2, 	//Colores para opcion no disponible

        5+8,0,        	//Colores para el titulo ventana
        5+8,              //Color recuadro
        5,0,        	//Colores para el titulo ventana inactiva

        1,1+8,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        5,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        1, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    {0,"Sunny",6,0,
        0,6,6,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
        4,0, 		//Colores para opcion seleccionada
        6,2,4,2, 	//Colores para opcion no disponible

        6+8,0,        	//Colores para el titulo ventana
        6+8,              //Color recuadro
        6,0,        	//Colores para el titulo ventana inactiva

        4,4+8,		//Colores waveform
        5,		//Color para zona no usada en visualmem
        5,      //color block visualtape
        6,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        1, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    {0,"Clean",7,0,
        0,1,0,0,          //No Mostrar cursor >, mostrar recuadro, no mostrar rainbow
        0,7,          //Colores para opcion seleccionada
        7,2,0,2, 	//Colores para opcion no disponible

        0,7,          //Colores para el titulo ventana
        0,              //Color recuadro
        7,0,          //Colores para el titulo ventana inactiva

        1,1+8,              //Colores waveform
        7+8,               //Color para zona no usada en visualmem
        7+8,      //color block visualtape
        7+8,0,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        'X',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    {0,"CleanInverse",0,7,
        0,1,0,0,          //No Mostrar cursor >, mostrar recuadro, no mostrar rainbow
        7,0,          //Colores para opcion seleccionada
        0,2,7,2, 	//Colores para opcion no disponible

        7,0,          //Colores para el titulo ventana
        7,              //Color recuadro
        0,7,          //Colores para el titulo ventana inactiva

        5,5+8,              //Colores waveform
        7,               //Color para zona no usada en visualmem
        7,      //color block visualtape
        0,7+8,		//Color para opcion marcada

        0,7, //Colores fileselector, zona seleccion archivos

        'X',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        0 //caracter franja normal
    },

    // https://ethanschoonover.com/solarized/. Solo vale en video driver completo por los colores usados (primer valor de la estructura)
    {1,"Solarized Dark",SOLARIZED_COLOUR_base03,SOLARIZED_COLOUR_base0,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow

        SOLARIZED_COLOUR_base02,SOLARIZED_COLOUR_base0, 		//Colores para opcion seleccionada
        SOLARIZED_COLOUR_base03,SOLARIZED_COLOUR_red,SOLARIZED_COLOUR_base02,SOLARIZED_COLOUR_red, 	//Colores para opcion no disponible

        SOLARIZED_COLOUR_base0,SOLARIZED_COLOUR_base03,        	//Colores para el titulo ventana
        SOLARIZED_COLOUR_base0,              //Color recuadro
        SOLARIZED_COLOUR_base03,SOLARIZED_COLOUR_base0,        	//Colores para el titulo ventana inactiva

        SOLARIZED_COLOUR_blue,SOLARIZED_COLOUR_cyan,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso. Seria SOLARIZED_COLOUR_red pero las franjas de volumen usan un formato $$ que solo permite color de 1 digito
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        153 //caracter franja cambiado
    },

    //Solo vale en video driver completo por los colores usados (primer valor de la estructura)
    {1,"Solarized Light",SOLARIZED_COLOUR_base3,SOLARIZED_COLOUR_base00,
        0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow

        SOLARIZED_COLOUR_base2,SOLARIZED_COLOUR_base00, 		//Colores para opcion seleccionada
        SOLARIZED_COLOUR_base3,SOLARIZED_COLOUR_red,SOLARIZED_COLOUR_base2,SOLARIZED_COLOUR_red, 	//Colores para opcion no disponible

        SOLARIZED_COLOUR_base00,SOLARIZED_COLOUR_base3,        	//Colores para el titulo ventana
        SOLARIZED_COLOUR_base00,              //Color recuadro
        SOLARIZED_COLOUR_base3,SOLARIZED_COLOUR_base00,        	//Colores para el titulo ventana inactiva

        SOLARIZED_COLOUR_blue,SOLARIZED_COLOUR_cyan,		//Colores waveform
        7,		//Color para zona no usada en visualmem
        7,      //color block visualtape
        2,7+8,		//Color para opcion marcada

        7,0, //Colores fileselector, zona seleccion archivos

        '*',
        '-', //caracter de minimizar ventana
        '+', //caracter de maximizar ventana
        '=', //caracter de restaurar ventana
        '!', //caracter de background ventana
        ' ', //caracter de fondo de titulo
        2, //color de aviso. Seria SOLARIZED_COLOUR_red pero las franjas de volumen usan un formato $$ que solo permite color de 1 digito
        colores_franja_speccy_brillo,colores_franja_speccy_oscuro,
        -1, //si texto inverso solo cambia color tinta
        0, //rellenar titulo
        char_set_spectrum,
        153 //caracter franja cambiado
    }


};



//valores de la ventana mostrada
//current_win_minimize_button_position en teoria solo usados para temas como Beos en que el boton no esta pegado al ancho de ventana
int current_win_x,current_win_y,current_win_ancho,current_win_alto,current_win_minimize_button_position;

//tipo ventana. normalmente activa. se pone tipo inactiva desde zxvision al pulsar fuera de la ventana
//Indica que la ventana a dibujar es la activa y por ejemplo el titulo tiene las franjas de colores y el texto del titulo con color destacado
int ventana_tipo_activa=1;



int menu_tooltip_counter;



int menu_window_splash_counter;
int menu_window_splash_counter_ms;

z80_bit tooltip_enabled;

//La primera vez que arranca, dispara evento de startup aid. Se inicializa desde cpu.c
int menu_first_aid_startup=0;


int menu_first_aid_must_show_startup=0;




//El texto a disparar al startup
char *string_config_key_aid_startup=NULL;


int realjoystick_detected_startup=0;


//Si se refresca en color gris cuando menu abierto y multitask es off
//z80_bit screen_bw_no_multitask_menu={1};









//Indica que esta el splash activo o cualquier otro texto de splash, como el de cambio de modo de video
z80_bit menu_splash_text_active;

//segundos que le faltan para desactivarse
int menu_splash_segundos=0;





char menu_buffer_textspeech_filter_program[PATH_MAX];
char menu_buffer_textspeech_stop_filter_program[PATH_MAX];

//cinta real seleccionada. realtape_name apuntara aqui
char menu_realtape_name[PATH_MAX];




//snapshot load. snapfile apuntara aqui
char snapshot_load_file[PATH_MAX];
char snapshot_save_file[PATH_MAX]="";

char binary_file_load[PATH_MAX]="";
char binary_file_save[PATH_MAX];

//char file_viewer_file_name[PATH_MAX]="";

char file_utils_file_name[PATH_MAX]="";

char *quickfile=NULL;
//quickload seleccionada. quickfile apuntara aqui
char quickload_file[PATH_MAX];

//Ultimos archivos cargados desde smartload
char last_files_used_array[MAX_LAST_FILESUSED][PATH_MAX];



void menu_debug_hexdump_with_ascii(char *dumpmemoria,menu_z80_moto_int dir_leida,int bytes_por_linea,z80_byte valor_xor);



//Interrumpe el core y le dice que hay que abrir el menu
void menu_fire_event_open_menu(void)
{
    //printf ("Ejecutar menu_fire_event_open_menu\n");
    menu_abierto=1;
    menu_event_open_menu.v=1;
}



//Cambia a directorio donde estan los archivos de instalacion (en share o en ..Resources)

void menu_chdir_sharedfiles(void)
{

    //cambia a los dos directorios. se quedara en el ultimo que exista
    debug_printf(VERBOSE_INFO,"Trying ../Resources");
    zvfs_chdir("../Resources");

    char installshare[PATH_MAX];
    sprintf (installshare,"%s/%s",INSTALL_PREFIX,"/share/zesarux/");
    debug_printf(VERBOSE_INFO,"Trying %s",installshare);
    zvfs_chdir(installshare);


}


//retorna dentro de un array de N teclas, la tecla pulsada
char menu_get_key_array_n_teclas(z80_byte valor_puerto,char *array_teclas,int teclas)
{

        int i;
        for (i=0;i<teclas;i++) {
                if ((valor_puerto&1)==0) return *array_teclas;
                valor_puerto=valor_puerto >> 1;
                array_teclas++;
        }

        return 0;

}



//retorna dentro de un array de 5 teclas, la tecla pulsada
char menu_get_key_array(z80_byte valor_puerto,char *array_teclas)
{

    return	menu_get_key_array_n_teclas(valor_puerto,array_teclas,5);

}

//funcion que retorna la tecla pulsada, solo tener en cuenta caracteres y numeros, sin modificador (mayus, etc)
//y por tanto solo 1 tecla a la vez

/*
z80_byte puerto_65278=255; //    db    		 255  ; V    C    X    Z    Sh    ;0
z80_byte puerto_65022=255; //    db    		 255  ; G    F    D    S    A     ;1
z80_byte puerto_64510=255; //    db              255  ; T    R    E    W    Q     ;2
z80_byte puerto_63486=255; //    db              255  ; 5    4    3    2    1     ;3
z80_byte puerto_61438=255; //    db              255  ; 6    7    8    9    0     ;4
z80_byte puerto_57342=255; //    db              255  ; Y    U    I    O    P     ;5
z80_byte puerto_49150=255; //    db              255  ; H    J    K    L    Enter ;6
z80_byte puerto_32766=255; //    db              255  ; B    N    M    Simb Space ;7

//puertos especiales no presentes en spectrum
z80_byte puerto_especial1=255; //   Home  End  PgDn  PgUp ESC
z80_byte puerto_especial2=255; //   F5 F4 F3 F2 F1
z80_byte puerto_especial3=255; //  F10 F9 F8 F7 F6
z80_byte puerto_especial4=255; //  F15 F14 F13 F12 F11
*/

static char menu_array_keys_65022[]="asdfg";
static char menu_array_keys_64510[]="qwert";
static char menu_array_keys_63486[]="12345";
static char menu_array_keys_61438[]="09876";
static char menu_array_keys_57342[]="poiuy";
static char menu_array_keys_49150[]="\x0dlkjh";

//arrays especiales
static char menu_array_keys_65278[]="zxcv";
static char menu_array_keys_32766[]="mnb";


/*
valores de teclas especiales:
2  ESC
3  Tecla de background
4  shift+cursor left
5  shift+cursor right
6  shift+cursor up
8  cursor left
9  cursor right
10 cursor down
11 cursor up
12 Delete o joystick left
13 Enter o joystick fire
15 SYM+MAY(TAB)
21 F1
24 PgUp
25 PgDn
26 Home
27 End

Joystick izquierda funcionara como Delete, no como cursor left. Resto de direcciones de joystick (up, down, right) se mapean como cursores

*/

/*
Teclas que tienen que retornar estas funciones para todas las maquinas posibles: spectrum, zx80/81, z88, cpc, sam, etc:

Letras y numeros

.  , : / - + < > = ' ( ) "


Hay drivers que retornan otros simbolos adicionales, por ejemplo en Z88 el ;. Esto es porque debe retornar ":" y estos : se obtienen mediante
mayusculas + tecla ";"

*/



//z80_byte puerto_especial2=255; //   F5 F4 F3 F2 F1
//z80_byte puerto_especial3=255; //  F10 F9 F8 F7 F6

//tecla F desde 1 hasta 10

/*
z80_byte *menu_get_port_puerto_especial(int tecla_f)
{
    if (tecla_f>=1 && tecla_f<=5) return &puerto_especial2;
    else return &puerto_especial3;
}

//tecla F desde 1 hasta 10
int menu_get_mask_puerto_especial(int tecla_f)
{

    tecla_f--; //Para ponernos en offset 0..4 y 5...9
    tecla_f =tecla_f % 5; //desde 0 a 4

    int mascara=1;

    if (tecla_f>0) mascara=mascara << tecla_f;

    return mascara;
}





z80_byte menu_get_port_value_background_key(void)
{
    z80_byte *puntero;


    int tecla_f=menu_get_defined_f_key_background();

    puntero=menu_get_port_puerto_especial(tecla_f);

    return *puntero;
}

int menu_get_mask_value_background_key(void)
{

    int tecla_f=menu_get_defined_f_key_background();

    return menu_get_mask_puerto_especial(tecla_f);
}

int menu_pressed_background_key(void)
{
    if ((menu_get_port_value_background_key() & menu_get_mask_value_background_key() )==0) return 1;
    else return 0;
}
*/

int menu_if_pressed_background_button(void)
{
    //Si pulsada tecla background

    //Si se pulsa tecla F que no es default
    if (menu_button_f_function.v && menu_button_f_function_index>=0) {

        //printf ("Pulsada alguna tecla de funcion\n");

        //Estas variables solo se activan cuando   //Abrir menu si funcion no es defecto y no es background window
          //if (accion!=F_FUNCION_DEFAULT && accion!=F_FUNCION_BACKGROUND_WINDOW) {

        int indice=menu_button_f_function_index;

        //Si accion es backgroundwindow
        int indice_tabla=defined_f_functions_keys_array[indice];
        enum defined_f_function_ids accion=menu_da_accion_direct_functions_indice(indice_tabla);
        if (accion==F_FUNCION_BACKGROUND_WINDOW) {
            //liberamos indicador de tecla de funcion
            menu_button_f_function.v=0;
            //printf ("Pulsada tecla F background\n");
            //sleep(1);
            return 1;
        }


        else {

            //Si tecla F6, es default, retornar ok si es Default
            if (indice==6-1 && accion==F_FUNCION_DEFAULT) {
                //liberamos indicador de tecla de funcion
                menu_button_f_function.v=0;
                //printf ("Es F6 por defecto\n");
                return 1;
            }

            //liberamos indicador de tecla de funcion si funcion es nothing
            if (accion==F_FUNCION_NOTHING) menu_button_f_function.v=0;

            return 0;
        }

    }

    //Si es F6 por default
    if ((puerto_especial3&1)==0) {
        //printf ("Pulsada F6\n");
        //sleep(1);

        //Ver si funcion F6 no esta asignada
        int indice=6-1;
        int indice_tabla=defined_f_functions_keys_array[indice];
        enum defined_f_function_ids accion=menu_da_accion_direct_functions_indice(indice_tabla);

        if (accion==F_FUNCION_DEFAULT) {
                //liberamos indicador de tecla de funcion
                menu_button_f_function.v=0;
                //printf ("Es F6 por defecto\n");
                return 1;
        }

        return 0;
    }


    return 0;
}


int menu_if_pressed_menu_button(void)
{
    //Si pulsada tecla menu

    //Si se pulsa tecla F que no es default
    if (menu_button_f_function.v && menu_button_f_function_index>=0) {

        //Estas variables solo se activan cuando   //Abrir menu si funcion no es defecto y no es background window
          //if (accion!=F_FUNCION_DEFAULT && accion!=F_FUNCION_BACKGROUND_WINDOW) {

        int indice=menu_button_f_function_index;

        //Si accion es openmenu
        int indice_tabla=defined_f_functions_keys_array[indice];
        enum defined_f_function_ids accion=menu_da_accion_direct_functions_indice(indice_tabla);
        if (accion==F_FUNCION_OPENMENU) {
            //liberamos esa tecla
            menu_button_f_function.v=0;
            //printf ("Pulsada tecla F abrir menu\n");
            //sleep(1);
            return 1;
        }


        else return 0;

    }

    //Sera tecla F5 por defecto, ya que no se ha pulsado tecla con no default
    if ((puerto_especial2&16)==0) {
        //printf ("Pulsada F5 por defecto\n");
        //sleep(1);
        return 1;
    }


    return 0;
}



int menu_if_pressed_close_all_menus_button(void)
{
    //Si pulsada tecla que simula F5 + ESC (y por tanto cierra todas ventanas de menu)

    //Si se pulsa tecla F que no es default
    if (menu_button_f_function.v && menu_button_f_function_index>=0) {

        //Estas variables solo se activan cuando   //Abrir menu si funcion no es defecto y no es background window
          //if (accion!=F_FUNCION_DEFAULT && accion!=F_FUNCION_BACKGROUND_WINDOW) {

        int indice=menu_button_f_function_index;

        //printf("Index: %d\n",menu_button_f_function_index);

        //Si accion es openmenu
        int indice_tabla=defined_f_functions_keys_array[indice];
        enum defined_f_function_ids accion=menu_da_accion_direct_functions_indice(indice_tabla);
        if (accion==F_FUNCION_CLOSE_ALL_MENUS) {
            //liberamos esa tecla
            menu_button_f_function.v=0;
            //printf ("Pulsada tecla F abrir menu\n");
            //sleep(1);
            return 1;
        }


        else return 0;

    }



    return 0;
}


z80_byte menu_get_pressed_key_no_modifier(void)
{
    z80_byte tecla;


    //printf("menu_get_pressed_key_no_modifier\n");

    //ESC significa Shift+Space en ZX-Uno y tambien ESC puerto_especial para menu.
    //Por tanto si se pulsa ESC, hay que leer como tal ESC antes que el resto de teclas (Espacio o Shift)
    if ((puerto_especial1&1)==0) return 2;

    //Si movimiento en menu con 5678, caps+space es como ESC
    if (zxvision_setting_use_speccy_keys.v && (puerto_65278 & 1)==0 && (puerto_32766 & 1)==0) return 2;
    //Y caps+symbol es TAB
    if (zxvision_setting_use_speccy_keys.v && (puerto_65278 & 1)==0 && (puerto_32766 & 2)==0) return 15;

    //if (menu_pressed_background_key() && menu_allow_background_windows) return 3; //Tecla background F6
    if (menu_if_pressed_background_button() && menu_allow_background_windows) {
        //printf("Retornamos tecla 3 desde menu_get_pressed_key_no_modifier\n");
        return 3; //Tecla background F6
    }

    int pulsada_tecla_cerrar_todos_menus=0;
    if (menu_if_pressed_close_all_menus_button()) {
        pulsada_tecla_cerrar_todos_menus=1;
    }

    //Si comando ZRCP de cerrar todos menus
    // de momento sin uso esto
    if (zrcp_command_close_all_menus) {
        zrcp_command_close_all_menus=0;
        pulsada_tecla_cerrar_todos_menus=1;
    }



    //Si menu esta abierto y pulsamos de nuevo la tecla de menu, cerrar todas ventanas y reabrir menu
    //O si se pulsa tecla que cierra todos menus (simula F5+ESC), y por tanto tiene que hacer la parte de F5 igual
    //No acabo de tener claro que este sea el mejor sitio para comprobar esto... o si?
    if (menu_if_pressed_menu_button() || pulsada_tecla_cerrar_todos_menus ) {
    //if ((puerto_especial2&16)==0) {
        //printf ("Pulsada tecla abrir menu\n");
        //sleep(1);
        menu_pressed_open_menu_while_in_menu.v=1;


        if (pulsada_tecla_cerrar_todos_menus) {
            debug_printf(VERBOSE_INFO,"Pressed key close all menus");
            menu_pressed_close_all_menus.v=1;
        }

        /*
        -si no se permite background, cerrar todos menus abiertos y volver a abrir el menu principal
-si se permite background:
—si ventana activa se puede enviar a background, enviarla a background
—si ventana activa no permite enviar a background, cerrarla
y luego en cualquiera de los dos casos, abrir el menu principal

las condiciones de "ventana activa se puede enviar a background o no" son comunes de cuando se pulsa en otra ventana. hacer función común??

    Estas decisiones son parecidas en casos:
    pulsar tecla menu cuando menu activo (menu_if_pressed_menu_button en menu_get_pressed_key_no_modifier), conmutar ventana, pulsar logo ZEsarUX en ext desktop
        */
    salir_todos_menus=1;

        if (!menu_allow_background_windows) {
            //Temp retornar escape
            return 2; //Escape
        }

        else {
            if (zxvision_current_window!=NULL) {

                //printf("menu_get_pressed_key_no_modifier. Retorno tecla 3 con zxvision current window no es null\n");
                                            //Si la ventana activa permite ir a background, mandarla a background
                                if (zxvision_current_window->can_be_backgrounded) {
                                        return 3; //Tecla background F6
                                }

                                //Si la ventana activa no permite ir a background, cerrarla
                                else {
                                        return 2; //Escape
                                }
            }
        }
    }

    tecla=menu_get_key_array(puerto_65022,menu_array_keys_65022);
    if (tecla) return tecla;

    tecla=menu_get_key_array(puerto_64510,menu_array_keys_64510);
    if (tecla) return tecla;

    tecla=menu_get_key_array(puerto_63486,menu_array_keys_63486);
    if (tecla) return tecla;

    tecla=menu_get_key_array(puerto_61438,menu_array_keys_61438);
    if (tecla) return tecla;

    tecla=menu_get_key_array(puerto_57342,menu_array_keys_57342);
    if (tecla) return tecla;

    tecla=menu_get_key_array(puerto_49150,menu_array_keys_49150);
    if (tecla) return tecla;

        tecla=menu_get_key_array_n_teclas(puerto_65278>>1,menu_array_keys_65278,4);
        if (tecla) return tecla;

        tecla=menu_get_key_array_n_teclas(puerto_32766>>2,menu_array_keys_32766,3);
        if (tecla) return tecla;

    //Y espacio
    if ((puerto_32766&1)==0) return ' ';

    //PgUp
    if ((puerto_especial1&2)==0) return 24;

    //PgDn
    if ((puerto_especial1&4)==0) return 25;

    //Home
    if ((puerto_especial1&16)==0) return 26;

    //End
    if ((puerto_especial1&8)==0) return 27;


    //F1
    if ((puerto_especial2&1)==0) return MENU_TECLA_AYUDA;



    return 0;
}




z80_bit menu_symshift={0};
z80_bit menu_capshift={0};
z80_bit menu_backspace={0};
z80_bit menu_tab={0};

//devuelve tecla pulsada teniendo en cuenta mayus, sym shift
z80_byte menu_get_pressed_key(void)
{
    //printf("menu_get_pressed_key\n");
    //Ver tambien eventos de mouse de zxvision
    //int pulsado_boton_cerrar=
    zxvision_handle_mouse_events(zxvision_current_window);

    //Boton shift+cursor en ventana que no permite background, cerrarla
    if (menu_pressed_shift_cursor_window_doesnot_allow) {
        debug_printf(VERBOSE_DEBUG,"Return ESC because window does not allow background after pressing shift+cursor");
        return 2; //Como ESC
    }

    if (mouse_pressed_close_window) {
        return 2; //Como ESC
    }

    if (mouse_pressed_background_window) {
        //printf ("pulsado background en menu_get_pressed_key\n");
        //sleep(5);
        //printf("Retornamos tecla 3 desde menu_get_pressed_key\n");
        return 3; //Como F6 background
    }

    if (mouse_pressed_hotkey_window) {
        mouse_pressed_hotkey_window=0;
        //printf ("Retornamos hoykey %c desde menu_get_pressed_key\n",mouse_pressed_hotkey_window_key);
        return mouse_pressed_hotkey_window_key;
    }

    z80_byte tecla;

    //primero joystick
    if (puerto_especial_joystick) {
        //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
        if ((puerto_especial_joystick&1)) {
            //Cursor Right
            if (menu_allow_background_windows && (puerto_65278&1)==0) {
                //printf("Pulsada shift+cursor right\n");
                return 5;
            }
            return 9;
        }

        if ((puerto_especial_joystick&2)) {
            //Cursor Left

            //De momento no permitimos shift+left. solo shift+right
            /*
            if (menu_allow_background_windows && (puerto_65278&1)==0) {
                //printf("Pulsada shift+cursor left\n");
                return 4;
            }
            */

            return 8;
        }

        //left joystick hace delete en menu.NO
        //if ((puerto_especial_joystick&2)) return 12;

        if ((puerto_especial_joystick&4)) return 10;
        if ((puerto_especial_joystick&8)) {
            if ((puerto_65278 & 1)==0) {
                //printf("Pulsada shift+cursor up\n");
                return 6;
            }
            return 11;
        }
//8  cursor left
//9  cursor right
//10 cursor down
//11 cursor up

        //Fire igual que enter
        if ((puerto_especial_joystick&16)) return 13;
    }



    if (menu_tab.v) {
        //printf ("Pulsado TAB\n");
        return 15;
    }

    if (menu_backspace.v) return 12;

    tecla=menu_get_pressed_key_no_modifier();


    if (tecla==0) return 0;


    //ver si hay algun modificador

    //mayus

//z80_byte puerto_65278=255; //    db              255  ; V    C    X    Z    Sh    ;0
    //if ( (puerto_65278&1)==0) {
    if (menu_capshift.v) {



        //si son letras, ponerlas en mayusculas
        if (tecla>='a' && tecla<='z') {
            tecla=tecla-('a'-'A');
            return tecla;
        }
        //seran numeros


        switch (tecla) {
            case '0':
                //delete
                return 12;
            break;


        }

    }

    //sym shift
    //else if ( (puerto_32766&2)==0) {
    else if (menu_symshift.v) {
        //ver casos concretos
        switch (tecla) {

            case 'z':
                return ':';
            break;

            case 'x':
                return 96; //Libra
            break;

            case 'c':
                return '?';
            break;

            case 'v':
                return '/';
            break;

            case 'b':
                return '*';
            break;

            case 'n':
                return ',';
            break;

            case 'm':
                return '.';
            break;

            case 'a':
                return '~'; //Aunque esta sale con ext+symbol
            break;

            case 's':
                return '|'; //Aunque esta sale con ext+symbol
            break;

            case 'd':
                return '\\'; //Aunque esta sale con ext+symbol
            break;

            case 'f':
                return '{'; //Aunque esta sale con ext+symbol
            break;

            case 'g':
                return '}'; //Aunque esta sale con ext+symbol
            break;

            case 'h':
                return 94; //Símbolo exponente/circunflejo
            break;

            case 'j':
                return '-';
            break;

            case 'k':
                return '+';
            break;

            case 'l':
                return '=';
            break;



            case 'r':
                return '<';
            break;

            case 't':
                return '>';
            break;

            case 'y':
                return '[';
            break;

            case 'u':
                return ']';
            break;

            //Faltaria el (C) del ext+sym+p. Se podria mapear a sym+I, pero esto genera el codigo 127,
            //Y dicho código en ascii no es imprimible y puede dar problemas en drivers texto, como curses

            case 'o':
                return ';';
            break;

            case 'p':
                return '"';
            break;



            case '1':
                return '!';
            break;

            case '2':
                return '@';
            break;

            case '3':
                return '#';
            break;

            case '4':
                return '$';
            break;

            case '5':
                return '%';
            break;

            case '6':
                return '&';
            break;

            case '7':
                return '\'';
            break;

            case '8':
                return '(';
            break;

            case '9':
                return ')';
            break;

            case '0':
                return '_';
            break;




            //no hace falta esta tecla. asi tambien evitamos que alguien la use en nombre de
            //archivo pensando que se puede introducir un filtro tipo *.tap, etc.
            //case 'b':
            //	return '*';
            //break;

        }
    }

    //printf("menu_get_pressed_key. Retorno 2 tecla: %d\n",tecla);
    return tecla;

}



//funcion que guarda el contenido del texto del menu. Usado por ejemplo en scanf cuando se usa teclado en pantalla
void menu_save_overlay_text_contents(overlay_screen *destination,int size)
{
    //int size=sizeof(overlay_screen_array);
    debug_printf(VERBOSE_DEBUG,"Saving overlay text contents. Size=%d bytes",size);

    memcpy(destination,overlay_screen_array,size);
}

//funcion que restaura el contenido del texto del menu. Usado por ejemplo en scanf cuando se usa teclado en pantalla
void menu_restore_overlay_text_contents(overlay_screen *origin,int size)
{
    //int size=sizeof(overlay_screen_array);
    debug_printf(VERBOSE_DEBUG,"Restoring overlay text contents. Size=%d bytes",size);

    memcpy(overlay_screen_array,origin,size);
}


//Llamar al teclado en pantalla pero desde algun menu ya, sobreimprimiéndolo
void menu_call_onscreen_keyboard_from_menu(void)
{


    menu_espera_no_tecla();
    menu_button_osdkeyboard.v=0; //Decir que no tecla osd pulsada, por si acaso
    menu_button_f_function.v=0;

    //overlay_screen copia_overlay[OVERLAY_SCREEN_MAX_WIDTH*OVERLAY_SCREEN_MAX_HEIGTH];
    overlay_screen *copia_overlay;

    int size=sizeof(struct s_overlay_screen)*OVERLAY_SCREEN_MAX_WIDTH*OVERLAY_SCREEN_MAX_HEIGTH;

    copia_overlay=malloc(size);

    if (copia_overlay==NULL) cpu_panic("Can not allocate memory for OSD");

    //Guardamos contenido de la pantalla
    menu_save_overlay_text_contents(copia_overlay,size);
    //Guardamos linea cuadrado ventana
    int antes_cuadrado_activo=0;
    int antes_cuadrado_activo_resize=0;
    int antes_ventana_activa_puede_minimizar=0;
    int antes_cuadrado_x1,antes_cuadrado_y1,antes_cuadrado_x2,antes_cuadrado_y2,antes_cuadrado_color;

    antes_cuadrado_activo=cuadrado_activo;
    antes_cuadrado_activo_resize=cuadrado_activo_resize;
    antes_ventana_activa_puede_minimizar=ventana_activa_puede_minimizar;
    antes_cuadrado_x1=cuadrado_x1;
    antes_cuadrado_y1=cuadrado_y1;
    antes_cuadrado_x2=cuadrado_x2;
    antes_cuadrado_y2=cuadrado_y2;
    antes_cuadrado_color=cuadrado_color;

    //Guardamos tamanyo ventana
    z80_byte antes_ventana_x,antes_ventana_y,antes_ventana_ancho,antes_ventana_alto;
    antes_ventana_x=current_win_x;
    antes_ventana_y=current_win_y;
    antes_ventana_ancho=current_win_ancho;
    antes_ventana_alto=current_win_alto;

    //Comportamiento de 1 caracter de margen a la izquierda en ventana (lo altera hexdump)
    int antes_menu_escribe_linea_startx=menu_escribe_linea_startx;

    menu_escribe_linea_startx=1;


    //Conservar setting salir_todos_menus, que lo cambia el osd
    int antes_salir_todos_menus=salir_todos_menus;


    //Cambiar funcion de overlay por la normal


    //Guardar funcion de texto overlay activo, para menus como el de visual memory por ejemplo, para desactivar  temporalmente
    void (*previous_function)(void);

    previous_function=menu_overlay_function;

    //restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);




    menu_onscreen_keyboard(0);


    //Restauramos funcion anterior de overlay
    set_menu_overlay_function(previous_function);




    salir_todos_menus=antes_salir_todos_menus;

    menu_escribe_linea_startx=antes_menu_escribe_linea_startx;

    //Restaurar texto ventana
    menu_restore_overlay_text_contents(copia_overlay,size);
    free(copia_overlay);

    //Restaurar linea cuadrado ventana
    cuadrado_activo=antes_cuadrado_activo;
    cuadrado_activo_resize=antes_cuadrado_activo_resize;
    ventana_activa_puede_minimizar=antes_ventana_activa_puede_minimizar;
    cuadrado_x1=antes_cuadrado_x1;
    cuadrado_y1=antes_cuadrado_y1;
    cuadrado_x2=antes_cuadrado_x2;
    cuadrado_y2=antes_cuadrado_y2;
    cuadrado_color=antes_cuadrado_color;

    //Restaurar tamanyo ventana
    current_win_x=antes_ventana_x;
    current_win_y=antes_ventana_y;
    current_win_ancho=antes_ventana_ancho;
    current_win_alto=antes_ventana_alto;

    menu_refresca_pantalla();

    //printf("Redraw todo\n");
    zxvision_redraw_all_windows();



}

//Si se ha pulsado tecla (o boton) asignado a osd
int menu_si_pulsada_tecla_osd(void)
{
    if (menu_button_osdkeyboard.v) {
        //debug_printf(VERBOSE_DEBUG,"Pressed OSD default key");

        //Pero siempre que esa funcion no este asignada a otra cosa
        //Esto es necesario en F8 pues dicha funcion es accesible desde menu, a diferencia de las demas,
        //que solo actuan con menu cerrado


        int indice=8-1; //tecla F8
        int indice_tabla=defined_f_functions_keys_array[indice];
        enum defined_f_function_ids accion=menu_da_accion_direct_functions_indice(indice_tabla);
        if (accion!=F_FUNCION_OSDKEYBOARD && accion!=F_FUNCION_DEFAULT) {

            //probablemente aqui solo entra cuando F8 se asigna a backgroundwindow, pues esa funcion no abre el menu
            //printf ("Asignada F8 a otra cosa\n");
            //sleep(5);
            menu_button_osdkeyboard.v=0;
            return 0;
        }

        else return 1;
    }

    if (menu_button_f_function.v==0) return 0;

    //debug_printf(VERBOSE_DEBUG,"Pressed F key");

    //Tecla F pulsada, ver si es la asignada a osd
        int indice=menu_button_f_function_index;

        int indice_tabla=defined_f_functions_keys_array[indice];
        enum defined_f_function_ids accion=menu_da_accion_direct_functions_indice(indice_tabla);
    if (accion==F_FUNCION_OSDKEYBOARD) {
        //debug_printf(VERBOSE_DEBUG,"Pressed F key mapped to OSD");
        return 1;
    }

    return 0;


}

//devuelve cadena de texto desde teclado
//max_length contando caracter 0 del final, es decir, para un texto de 4 caracteres, debemos especificar max_length=5
//ejemplo, si el array es de 50, se le debe pasar max_length a 50
/*
int menu_scanf(char *string,unsigned int max_length,int max_length_shown,int x,int y)
{

    //Enviar a speech
    char buf_speech[MAX_BUFFER_SPEECH+1];
    sprintf (buf_speech,"Edit box: %s",string);
    menu_textspeech_send_text(buf_speech);


    z80_byte tecla;

    //ajustar offset sobre la cadena de texto visible en pantalla
    int offset_string;

    int j;
    j=strlen(string);
    if (j>max_length_shown-1) offset_string=j-max_length_shown+1;
    else offset_string=0;


    //max_length ancho maximo del texto, sin contar caracter 0
    //por tanto si el array es de 50, se le debe pasar max_length a 50

    max_length--;

    //cursor siempre al final del texto

    do {
        menu_scanf_print_string(string,offset_string,max_length_shown,x,y);

        if (menu_multitarea==0) menu_refresca_pantalla();

        menu_espera_tecla();
        //printf ("Despues de espera tecla\n");
        tecla=menu_get_pressed_key();
        //printf ("tecla leida=%d\n",tecla);
        menu_espera_no_tecla();



        //si tecla normal, agregar:
        if (tecla>31 && tecla<128) {
            if (strlen(string)<max_length) {
                int i;
                i=strlen(string);
                string[i++]=tecla;
                string[i]=0;

                //Enviar a speech letra pulsada
                menu_speech_reset_tecla_pulsada();
                    sprintf (buf_speech,"%c",tecla);
                    menu_textspeech_send_text(buf_speech);


                if (i>=max_length_shown) offset_string++;

            }
        }

        //tecla borrar o tecla izquierda
        if (tecla==12 || tecla==8) {
            if (strlen(string)>0) {
                                int i;
                                i=strlen(string)-1;

                                //Enviar a speech letra borrada

                menu_speech_reset_tecla_pulsada();
                                sprintf (buf_speech,"%c",string[i]);
                                menu_textspeech_send_text(buf_speech);


                                string[i]=0;
                if (offset_string>0) {
                    offset_string--;
                    //printf ("offset string: %d\n",offset_string);
                }
            }

        }


    } while (tecla!=13 && tecla!=15 && tecla!=2);

    //if (tecla==13) printf ("salimos con enter\n");
    //if (tecla==15) printf ("salimos con tab\n");

    menu_reset_counters_tecla_repeticion();
    return tecla;

//papel=7+8;
//tinta=0;

}
*/


//funcion para asignar funcion de overlay
void set_menu_overlay_function(void (*funcion)(void) )
{

    menu_overlay_function=funcion;

    //para que al oscurecer la pantalla tambien refresque el border
    modificado_border.v=1;
    menu_overlay_activo=1;

    //Necesario para que al poner la capa de menu, se repinte todo
    clear_putpixel_cache();

    //Y por si acaso, aunque ya deberia haber buffer de capas activo, asignarlo
    scr_init_layers_menu();
}


//funcion para desactivar funcion de overlay
void reset_menu_overlay_function(void)
{
    //para que al oscurecer la pantalla tambien refresque el border
    modificado_border.v=1;



    menu_overlay_activo=0;

    scr_clear_layer_menu();


}

/*funcion para escribir un caracter en el buffer de overlay
//tinta y/o papel pueden tener brillo (color+8)

//Nota: funciones putchar_menu_overlay* vienen heredadas del anterior entorno sin zxvision
//aunque ahora solo se deberian usar para escribir caracteres que van al titulo de la ventana.
//Para escribir dentro de la ventana (no en el titulo) se deben usar funciones de zxvision_*

Hay dos optimizaciones de cache:

1) Campo .modificado dice que se ha escrito ese caracter y al refrescar desde nomal_overlay_texto_menu debe renderizarlo. Desde alli,
al renderizar, se pondra ese campo .modificado a 0. Asi en ventanas como help->readme, si no se mueve no hace scroll ni nada,
solo se renderiza una vez, en cada refresco de pantalla no se renderiza de nuevo, ahorrando monton de ciclos de cpu

2) Si el caracter a escribir es el mismo, con mismos atributos, y parametro use_cache_mismo_caracter=1, no modificamos ese caracter, por tanto
no se renderizara desde nomal_overlay_texto_menu. Dado que es mismo caracter, ahorraremos ciclos de cpu

//El parametro cache_mismo_caracter le dice si usamos cache al escribir caracter. Esta cache no se usa por ejemplo en ventanas que
//tienen mucho putpixel con fondo en blanco (caracter " ") que quieren que se escriba el fondo al refrescar ventana
//Podria dar problemas con alguna otra llamada a putchar_menu_overlay o putchar_menu_overlay_parpadeo  (donde se usa cache)
//pero lo que hemos hecho es que en putchar_menu_overlay y putchar_menu_overlay_parpadeo, siempre llama sin cache
*/
void putchar_menu_overlay_parpadeo_cache_or_not(int x,int y,z80_byte caracter,int tinta,int papel,int parpadeo,int use_cache_mismo_caracter)
{

    //int xusado=x;

    //if (menu_char_width!=8) {
        //xusado=(x*menu_char_width)/8;
    //}

    //int xfinal=((x*menu_char_width)+menu_char_width-1)/8;

    //Controlar limite
    if (x<0 || y<0 || x>=scr_get_menu_width() || y>=scr_get_menu_height() ) {
        //printf ("Out of range. X: %d Y: %d Character: %c\n",x,y,caracter);
        return;
    }

    int pos_array=y*scr_get_menu_width()+x;
    if (ESTILO_GUI_SOLO_MAYUSCULAS) caracter=letra_mayuscula(caracter);

/*
Otra optimizacion mas. Si se escribe el mismo caracter con iguales atributos, decimos que esta en cache
Con esto se pasa, por ejemplo, enseñando el Sonic, con varias ventanas de menu abiertas, de usar
88 % de cpu
a usar
45% de cpu

Esto requiere por otra parte que, ventanas que dibujan con putpixel (como waveform, audio chip sheet etc)
esperan que siempre se borre con espacios la ventana y luego ellos escriben encima sus pixeles con el fondo "ya limpio"
Esto ya no sucede mas, pues el fondo limpio con espacios, al no alterarse, no se redibuja limpiando los pixeles anteriores
Requiere entonces que llamen a una función que limpia la ventana e indicando parametro de .modificado
*/
//#ifdef ZXVISION_USE_CACHE_OVERLAY_TEXT
    //Cualquier atributo alterado del caracter, o el propio caracter, decimos a la cache que se ha alterado y hay que redibujar
    if (
        use_cache_mismo_caracter==0 ||
        overlay_screen_array[pos_array].tinta!=tinta ||
        overlay_screen_array[pos_array].papel!=papel ||
        overlay_screen_array[pos_array].parpadeo!=parpadeo ||
        overlay_screen_array[pos_array].caracter!=caracter
    ) {
        overlay_screen_array[pos_array].modificado=1;
    }
//#endif

    overlay_screen_array[pos_array].tinta=tinta;
    overlay_screen_array[pos_array].papel=papel;
    overlay_screen_array[pos_array].parpadeo=parpadeo;
    overlay_screen_array[pos_array].caracter=caracter;


    //overlay_usado_screen_array[y*scr_get_menu_width()+xusado]=1;

}


void putchar_menu_overlay_parpadeo(int x,int y,z80_byte caracter,int tinta,int papel,int parpadeo)
{
    putchar_menu_overlay_parpadeo_cache_or_not(x,y,caracter,tinta,papel,parpadeo,0);
}

//funcion para escribir un caracter en el buffer de overlay
//tinta y/o papel pueden tener brillo (color+8)
void putchar_menu_overlay(int x,int y,z80_byte caracter,int tinta,int papel)
{
    putchar_menu_overlay_parpadeo(x,y,caracter,tinta,papel,0); //sin parpadeo
}

//Funcion para borrar caracter del buffer de overlay
void deletechar_menu_overlay(int x,int y)
{
    putchar_menu_overlay_parpadeo(x,y,0,0,0,0);
}



void menu_scr_putpixel(int x,int y,int color)
{

    //int margenx_izq,margeny_arr;
    //scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);

    x *=menu_gui_zoom;
    y *=menu_gui_zoom;

    //Esto ya no hace falta desde el uso de dos layers de menu y maquina
    /*if (rainbow_enabled.v) {
        x+=margenx_izq;
        y+=margeny_arr;
    }*/



    scr_putpixel_gui_zoom(x,y,color,menu_gui_zoom);
}


//sin el zoom de ventana, solo el posible de menu. usado en keyboard help
void menu_scr_putpixel_no_zoom(int x,int y,int color)
{

    //int margenx_izq,margeny_arr;
    //scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);

    x *=menu_gui_zoom;
    y *=menu_gui_zoom;

    //Esto ya no hace falta desde el uso de dos layers de menu y maquina
    /*if (rainbow_enabled.v) {
        x+=margenx_izq;
        y+=margeny_arr;
    }*/



    scr_putpixel_gui_no_zoom(x,y,color,menu_gui_zoom);
}

/*
//Hacer un putpixel en la coordenada indicada pero haciendo tan gordo el pixel como diga zoom_level
void scr_putpixel_gui_zoom(int x,int y,int color,int zoom_level)
{
    //Hacer zoom de ese pixel si conviene
    int incx,incy;
    for (incy=0;incy<zoom_level;incy++) {
        for (incx=0;incx<zoom_level;incx++) {
            //printf("putpixel %d,%d\n",x+incx,y+incy);
            if (rainbow_enabled.v==1) scr_putpixel_zoom_rainbow(x+incx,y+incy,color);

            else scr_putpixel_zoom(x+incx,y+incy,color);
        }
    }
}
*/

void new_menu_putchar_footer(int x,int y,z80_byte caracter,int tinta,int papel)
{

    putchar_footer_array(x,y,caracter,tinta,papel,0);


}

int previous_switchzxdesktop_timer_event_mouse_x=0;
int previous_switchzxdesktop_timer_event_mouse_y=0;
//si estaba visible o no
z80_bit switchzxdesktop_button_visible={0};

//temporizador desde que empieza a no moverse
int switchzxdesktop_button_visible_timer=0;


int zxvision_if_lower_button_switch_zxdesktop_enabled(void)
{

    //con emulacion de kempston mouse, no se dispara evento de abrir menu al pulsar con raton, por tanto,
    //no se puede gestionar pulsaciones sobre el boton de switch

    if (si_complete_video_driver() && scr_driver_can_ext_desktop() &&
        menu_footer && zxdesktop_switch_button_enabled.v && !ventana_fullscreen &&
        mouse_menu_disabled.v==0 && kempston_mouse_emulation.v==0) return 1;
    else return 0;
}

//Boton habilitado y ademas visible (visibilidad determinada por movimiento de raton)
int zxvision_if_lower_button_switch_zxdesktop_visible(void)
{
    if (zxvision_if_lower_button_switch_zxdesktop_enabled() && switchzxdesktop_button_visible.v) return 1;
    else return 0;
}


unsigned char zxvision_retorna_caracter_flecha_izquierda(void)
{
    //Si driver video permite caracteres redefinidos, poner los cursores "bonitos"
    if (si_complete_video_driver()) {
        return 166;
    }
    else return '<';
}

unsigned char zxvision_retorna_caracter_flecha_derecha(void)
{
    //Si driver video permite caracteres redefinidos, poner los cursores "bonitos"
    if (si_complete_video_driver()) {
        return 165;
    }
    else return '>';
}

unsigned char zxvision_retorna_caracter_flecha_arriba(void)
{
    //Si driver video permite caracteres redefinidos, poner los cursores "bonitos"
    if (si_complete_video_driver()) {
        return 164;
    }
    else return '^';
}

unsigned char zxvision_retorna_caracter_flecha_abajo(void)
{
    //Si driver video permite caracteres redefinidos, poner los cursores "bonitos"
    if (si_complete_video_driver()) {
        return 163;
    }
    else return 'v';
}

void menu_put_switch_zxdesktop_footer(void)
{

    //TODO: Estas posiciones donde estan los botones, se obtienen de manera distinta en las funciones:
    //menu_put_switch_zxdesktop_footer
    //zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_common
    //Aunque se obtienen de diferentes maneras pero el resultado final (en teoria) es el mismo

    //printf("Visibilidad boton: %d\n",switchzxdesktop_button_visible.v);

    //si menu abierto no mostrar el boton
    if (menu_abierto) return;


    if (zxvision_if_lower_button_switch_zxdesktop_visible() ) {


        //Poner boton de +/- de zx desktop a la derecha del todo del footer
        //printf("Escribir boton\n");

        //ancho total del footer
        int xancho_total_footer=screen_get_window_size_width_no_zoom_border_en()/8;

        //printf("xancho_total_footer: %d\n",xancho_total_footer);
        int xorigen=xancho_total_footer;


        //restarle margen izquierdo. Porque al escribir el footer la posicion 0 ya tiene el margen del border sumado
        int margenx_izq;
        int margeny_arr;


        scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);

        margenx_izq /=8;

        xorigen -=margenx_izq;
        //xorigen contiene el origen donde van los botones de ampliar/reducir ancho

        unsigned char caracter_flecha_izquierda=zxvision_retorna_caracter_flecha_izquierda();
        unsigned char caracter_flecha_derecha=zxvision_retorna_caracter_flecha_derecha();
        unsigned char caracter_flecha_arriba=zxvision_retorna_caracter_flecha_arriba();
        unsigned char caracter_flecha_abajo=zxvision_retorna_caracter_flecha_abajo();


        z80_byte caracter_ampliar_ancho,caracter_reducir_ancho;
        z80_byte caracter_ampliar_alto,caracter_reducir_alto;
        if (screen_ext_desktop_enabled) {
            caracter_ampliar_ancho=caracter_flecha_derecha;
            caracter_reducir_ancho=caracter_flecha_izquierda;
            caracter_ampliar_alto=caracter_flecha_abajo;
            caracter_reducir_alto=caracter_flecha_arriba;
        }
        else {
            caracter_ampliar_ancho='+';
            caracter_reducir_ancho=' ';  //Esta deshabilitado. Mostrar espacio (no boton)
            caracter_ampliar_alto=' ';
            caracter_reducir_alto=' ';
        }

        if (zxdesktop_width>=ZXDESKTOP_MAXIMUM_WIDTH_BY_BUTTON) {
            caracter_ampliar_ancho=' ';
        }

        if (zxdesktop_height>=ZXDESKTOP_MAXIMUM_HEIGHT_BY_BUTTON) {
            caracter_ampliar_alto=' ';
        }

        //int yorigen=screen_get_emulated_display_height_no_zoom_bottomborder_en()/8;

        int yorigen=0;

        //printf("yorigen: %d\n",yorigen);

        //justo 2 caracter a la izquierda del tope de la derecha
        xorigen-=2;

        //printf("xorigen: %d (*8=%d) yorigen: %d\n",xorigen,xorigen*8,yorigen);

        debug_printf(VERBOSE_PARANOID,"Drawing ZX Desktop switch button");


        if (caracter_ampliar_ancho!=' ') scr_putchar_footer_comun_zoom(caracter_ampliar_ancho,xorigen,yorigen,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);

        if (caracter_reducir_ancho!=' ') scr_putchar_footer_comun_zoom(caracter_reducir_ancho,xorigen,yorigen+1,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);

        if (caracter_ampliar_alto!=' ') scr_putchar_footer_comun_zoom(caracter_ampliar_alto,xorigen-1,yorigen+1,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);

        if (caracter_reducir_alto!=' ') scr_putchar_footer_comun_zoom(caracter_reducir_alto,xorigen-1,yorigen,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);


    }

}

void zxdesktop_make_switchbutton_visible(void)
{
    debug_printf(VERBOSE_INFO,"Make zxdesktop switch button visible");
    switchzxdesktop_button_visible.v=1;


    //TODO: no se porque aqui es necesario el clear_putpixe_cache, pero si lo quito,
    //la segunda vez que debe aparecer el boton, no aparece
    //habria que debugarlo mas, es extraño
    //quiza el putpixel cache afecta de manera extraña al footer
    //La putpixel cache no ha hecho que darme mas que problemas en toda la historia de ZEsarUX,
    //se gana rendimiento teniendola pero provoca efectos muy extraños, probablemente porque
    //no se refresca siempre donde deberia
    clear_putpixel_cache();

    menu_init_footer();
    redraw_footer();
}

#define MAX_SWITCH_DESKTOP_VISIBLE_TIMER 100

void zxdesktop_make_switchbutton_invisible(void)
{
    //Esto puede ser redundante desde abajo donde se llama pero a esta funcion se llama desde otros sitios
    //e interesa establecer el timer como conviene
    switchzxdesktop_button_visible_timer=MAX_SWITCH_DESKTOP_VISIBLE_TIMER;

    debug_printf(VERBOSE_INFO,"Make zxdesktop switch button hidden");
    switchzxdesktop_button_visible.v=0;
    menu_init_footer();
    redraw_footer();

    //menu_put_switch_zxdesktop_footer();

}

//Ocultar o mostrar boton de switch zx desktop
void zxdesktop_switchdesktop_timer_event(void)
{

    if (!zxvision_if_lower_button_switch_zxdesktop_enabled()) return;

    //Condicion de menu abierto no se controla desde zxvision_if_lower_button_switch_zxdesktop_enabled
    //pues en esa funcion interesa que no se compruebe eso pues al pulsar ese boton se abre el menu (aunque sin verse)
    //Luego por ejemplo en menu_put_switch_zxdesktop_footer se comprueba tambien aparte que si el menu esta abierto, no escribir el boton
    if (menu_abierto) return;

    int movido=0;

    if (previous_switchzxdesktop_timer_event_mouse_x!=mouse_x || previous_switchzxdesktop_timer_event_mouse_y!=mouse_y)
    {
        movido=1;
    }

    previous_switchzxdesktop_timer_event_mouse_x=mouse_x;
    previous_switchzxdesktop_timer_event_mouse_y=mouse_y;

    /*if (movido) {
        printf("mouse movido\n");
    }*/

    //No estaba visible
    if (switchzxdesktop_button_visible.v==0) {
        if (movido) {
            zxdesktop_make_switchbutton_visible();
        }
    }

    //Estaba visible
    else {
        if (movido) {
            switchzxdesktop_button_visible_timer=0;
        }

        else {
            switchzxdesktop_button_visible_timer++;

            //en 2 segundos (50*2 frames) desaparece
            if (switchzxdesktop_button_visible_timer==MAX_SWITCH_DESKTOP_VISIBLE_TIMER) {
                zxdesktop_make_switchbutton_invisible();
            }
        }

    }
}




void menu_putstring_footer(int x,int y,char *texto,int tinta,int papel)
{

    //if (y==0) {
    //    printf("print a footer: %s\n",texto);
    //}

    while ( (*texto)!=0) {
        new_menu_putchar_footer(x++,y,*texto,tinta,papel);
        texto++;
    }

    //Solo en putstring actualizamos el footer. En putchar, no
    redraw_footer();

    //Cuando escribimos en primera linea de footer, volver a escribir el boton de switch ZX Desktop
    /*if (y==0) {
        printf("poner boton conmutar zx desktop\n");
        menu_put_switch_zxdesktop_footer();
    }
    */
}


void menu_footer_activity(char *texto)
{

    char buffer_texto[32];
    //Agregar espacio delante y detras
    sprintf (buffer_texto," %s ",texto);

    int inicio_x=32-strlen(buffer_texto);

    menu_putstring_footer(inicio_x,1,buffer_texto,WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);

}

void menu_delete_footer_activity(void)
{
                              //345678901
    menu_putstring_footer(23,1,"         ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
}

//Escribir info tarjetas memoria Z88
void menu_footer_z88(void)
{

    if (!MACHINE_IS_Z88) return;

    //printf("draw footer\n");

    char nombre_tarjeta[20];
    int x=0;

    //menu_putstring_footer(0,0,get_machine_name(current_machine_type),WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

    //borramos esa zona primero

    menu_footer_clear_bottom_line();


    int i;
    for (i=1;i<=3;i++) {
        if (z88_memory_slots[i].size==0) sprintf (nombre_tarjeta,"[Empty]");
        else sprintf (nombre_tarjeta,"[%s]",z88_memory_types[z88_memory_slots[i].type]);

        //Si ocupa el texto mas de 10, cortar texto
        if (strlen(nombre_tarjeta)>10) {
            nombre_tarjeta[9]=']';
            nombre_tarjeta[10]=0;
        }

        //Si tapa abierta, escribir en minusculas... TODO: hacer mejor manera de enseñar esto
        if (z88_flap_is_open()) string_a_minusculas(nombre_tarjeta,nombre_tarjeta);

        int tinta=WINDOW_FOOTER_INK;
        int papel=WINDOW_FOOTER_PAPER;

        //Si tiene que hacer flush, es que acaba de escribir, indicarlo
        if (i==3 && z88_slot3_activity_indicator) {
            //intercambiar colores
            int temp_tinta=tinta;
            tinta=papel;
            papel=temp_tinta;
            strcpy(nombre_tarjeta,"[WRITE]");

        }

        menu_putstring_footer(x,2,nombre_tarjeta,tinta,papel);
        x +=10;
    }
}

int menu_si_mostrar_footer_f5_menu(void)
{
    if (!MACHINE_IS_Z88)  {
        //Y si no hay texto por encima de cinta autodetectada
        if (tape_options_set_first_message_counter==0 && tape_options_set_second_message_counter==0) {
            return 1;
        }
    }

    return 0;
}

//Durante 60 segundos aparece
int menu_footer_f5_menu_counter=60;


void menu_footer_f5_menu_timer(void)
{
    if (menu_footer_f5_menu_counter>0) {
        menu_footer_f5_menu_counter--;

        //Si llega a 0, actualizarlo con footer definitivo
        if (menu_footer_f5_menu_counter==0) {
            menu_footer_clear_bottom_line();
            menu_footer_bottom_line();
        }
    }
}

void menu_putstring_footer_spaces(int y)
{
    menu_putstring_footer(0,y,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
}

void menu_footer_f5_menu(void)
{

    //Decir F5 menu en linea de tarjetas de memoria de z88

    //Borrar antes con espacios si hay algo               //01234567890123456789012345678901
    //Sucede que al cargar emulador con un tap, se pone abajo primero el nombre de emulador y version,
    //y cuando se quita el splash, se pone este texto. Si no pongo espacios, se mezcla parte del texto de F5 menu etc con la version del emulador

    //menu_putstring_footer(0,WINDOW_FOOTER_ELEMENT_Y_F5MENU,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
    menu_putstring_footer_spaces(WINDOW_FOOTER_ELEMENT_Y_F5MENU);
    char texto_f_menu[32];
    sprintf(texto_f_menu,"%s Menu",openmenu_key_message);
    menu_putstring_footer(0,WINDOW_FOOTER_ELEMENT_Y_F5MENU,texto_f_menu,WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);



}

void menu_footer_zesarux_emulator(void)
{

    if (menu_si_mostrar_footer_f5_menu() ) {
        debug_printf (VERBOSE_DEBUG,"Showing ZEsarUX footer message");

        if (menu_footer_f5_menu_counter) {
            menu_footer_f5_menu();
        }

        else {

            //Si esta zeng online conectado, alterar algo el mensaje

            //01234567890123456789012345678901
            //ZEsarUX 11 - ONLINE
            //ZEsarUX 11-RC3 - ONLINE
            //Aunque ocupe algo mas, por alguna version beta, no deberia pasar nada pues el putchar en el footer controla margenes
            char buffer_footer[100];

            if (zeng_online_connected.v) {
                //printf("Mostrando footer con estado conexion ZENG online\n");
                sprintf(buffer_footer,"ZEsarUX "EMULATOR_VERSION " %s",zoc_return_connected_status() );
            }

            else {
                //printf("Mostrando footer SIN estado conexion ZENG online\n");
                sprintf(buffer_footer,"ZEsarUX "EMULATOR_VERSION);
            }

            menu_putstring_footer(0,WINDOW_FOOTER_ELEMENT_Y_ZESARUX_EMULATOR,buffer_footer,WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

        }

    }

}

void footer_logo_putpixel(z80_int *destino GCC_UNUSED,int x,int y,int ancho GCC_UNUSED,int color)
{
        scr_putpixel(x,y,color);
}


//Este es el logo que se mostrara en el footer, con el color de marco cambiado segun el color del footer
//aqui el array de punteros de cada linea, necesario para renderizar el bitmap
char *zesarux_ascii_logo_footer[ZESARUX_ASCII_LOGO_ALTO];
//memoria asignada para contener el logo del footer
char *zesarux_ascii_logo_footer_mem=NULL;


//copiar logo original a destino
//el color_marco podria ser cualquier letra de color valida, aunque en la funcion que llama
//para escribir el footer solo le indico o bien blanco (w) o bien transparente (espacio)
//tambien cambiamos los colores si estamos en modo navidad
void menu_footer_logo_copy_final(char color_marco)
{

    if (zesarux_ascii_logo_footer_mem==NULL) {
        //ancho+1 porque son strings con 0 al final de cada linea del bitmap
        zesarux_ascii_logo_footer_mem=malloc((ZESARUX_ASCII_LOGO_ANCHO+1)*ZESARUX_ASCII_LOGO_ALTO);
        if (zesarux_ascii_logo_footer_mem==NULL) {
            cpu_panic("Can not allocate memory for footer logo");
        }
    }

    int y;

    for (y=0;y<ZESARUX_ASCII_LOGO_ALTO;y++) {
        int i;
        //poner cada puntero de lineas de logo a la memoria asignada
        zesarux_ascii_logo_footer[y]=&zesarux_ascii_logo_footer_mem[(ZESARUX_ASCII_LOGO_ANCHO+1)*y];
        for (i=0;get_zesarux_ascii_logo_whitebright()[y][i];i++) {

            char c=get_zesarux_ascii_logo_whitebright()[y][i];
            //printf("y %d i %d c %c\n",y,i,c);

            //cambiamos el color del marco (blanco brillante en el original) por el color indicado en el parametro
            if (c=='W') c=color_marco;

            else {
                if (christmas_mode.v) {
                    switch (c) {
                        case 'r':
                        case 'g':
                            c='R';
                        break;


                        case 'y':
                        case 'c':
                            c='G';
                        break;
                    }

                    if (c=='x') {
                        //En logo X anniversary y en navidad, no alteramos color negro
                        if (xanniversary_logo.v==0) c='R';
                    }
                }
            }

            //Y algunos puntos blancos, simulando copos de nieve, en varias posiciones
            if (christmas_mode.v) {
                if (y==3 && i==5) c='W';
                if (y==2 && i==20) c='W';
                if (y==24 && i==13) c='W';
            }

            //printf("(%c)\n",zesarux_ascii_logo_footer[y][i]);
            zesarux_ascii_logo_footer[y][i]=c;
            //printf("(%c)\n",zesarux_ascii_logo_footer[y][i]);
        }
        zesarux_ascii_logo_footer[y][i]=0;
    }
}

void menu_clear_footer(void)
{
    if (!menu_footer) return;


    debug_printf (VERBOSE_DEBUG,"Clearing Footer");

    //Borrar footer
    if (si_complete_video_driver() ) {

        int alto=WINDOW_FOOTER_SIZE;
        int ancho=screen_get_window_size_width_no_zoom_border_en();

        int yinicial=screen_get_window_size_height_no_zoom_border_en()+screen_get_ext_desktop_height_no_zoom()-alto;

        int x,y;

        //Para no andar con problemas de putpixel en el caso de realvideo desactivado,
        //usamos putpixel tal cual y calculamos zoom nosotros manualmente

        alto *=zoom_y;
        ancho *=zoom_x;

        yinicial *=zoom_y;

        int color=WINDOW_FOOTER_PAPER;

        for (y=yinicial;y<yinicial+alto;y++) {
                //printf ("%d ",y);
                for (x=0;x<ancho;x++) {
                        scr_putpixel(x,y,color);
                }
        }

        //Agregar logo en footer. siempre que haya margen en el footer por la izquierda.

        int margenx_izq;
        int margeny_arr;

        scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);

        //asumimos que si es mayor que 0, habra espacio
        //0 sera el caso de border deshabilitado o tambien de Z88

        int xlogo=0;

        if (MACHINE_IS_Z88) {
            //Pegado a la derecha casi, dejando espacio a la derecha del todo para los botones de switch zxdesktop
            xlogo=zoom_x*(SCREEN_Z88_WIDTH-ZESARUX_ASCII_LOGO_ANCHO-8*3);

            //engañamos para decir que si que dibuje el logo
            margenx_izq=1;
        }

        if (MACHINE_IS_TSCONF) {
            //Pegado a la derecha casi, dejando espacio a la derecha del todo para los botones de switch zxdesktop
            xlogo=zoom_x*(TSCONF_DISPLAY_WIDTH-ZESARUX_ASCII_LOGO_ANCHO-8*3);

            //engañamos para decir que si que dibuje el logo
            margenx_izq=1;
        }

        if (MACHINE_IS_QL) {
            //Pegado a la derecha casi, dejando espacio a la derecha del todo para los botones de switch zxdesktop
            xlogo=zoom_x*(QL_DISPLAY_WIDTH+(QL_LEFT_BORDER_NO_ZOOM*2)*border_enabled.v-ZESARUX_ASCII_LOGO_ANCHO-8*3);

            //engañamos para decir que si que dibuje el logo
            margenx_izq=1;


            //return +(QL_LEFT_BORDER_NO_ZOOM*2)*border_enabled.v;
        }

        /*if (MACHINE_IS_PCW) {
            //Pegado a la derecha casi, dejando espacio a la derecha del todo para los botones de switch zxdesktop
            xlogo=zoom_x*(PCW_DISPLAY_WIDTH+(PCW_LEFT_BORDER_NO_ZOOM*2)*border_enabled.v-ZESARUX_ASCII_LOGO_ANCHO-8*3);

            //engañamos para decir que si que dibuje el logo
            margenx_izq=1;


            //return +(QL_LEFT_BORDER_NO_ZOOM*2)*border_enabled.v;
        } */

        //Contemplar si hay espacio a la izquierda. Esto es por si no hay border
        if (margenx_izq>0) {
            //sacar el zoom mas pequeño
            int zoom_logo;
            if (zoom_x<zoom_y) zoom_logo=zoom_x;
            else zoom_logo=zoom_y;

            //saltar la primera linea del logo
            //alto ocupa 26, pero footer es de 24, por tanto dibujamos solo 24 quitando la primera y ultima lineas

            //Lo siguiente es una pijada pero queda bien
            //Hacer transparente el color del marco del logo, excepto cuando el footer es un color oscuro
            //Si no, poner marco del logo en Blanco
            //Esto hace que el logo siempre se vea bien independientemente del color del footer (que varia con el GUI style),
            //pues no se fundira con el footer.
            //Asi por ejemplo, footer amarillo, rojo ..., tendra logo sin marco, porque es mayormente negro el logo y no se confunde con el footer
            //pero si el footer es negro u oscuro (ejemplo del tema Solarized Dark), el logo tendra un marco de color blanco,
            //para que no se confunda el negro del logo con el footer
            int color_footer=WINDOW_FOOTER_PAPER;
            char color_marco=' '; //asumimos transparente

            //ver si color se acerca a negro
            int color_fondo=spectrum_colortable_normal[color_footer];
            int r=(color_fondo>>16) & 0xFF;
            int g=(color_fondo>>8 ) & 0xFF;
            int b=(color_fondo    ) & 0xFF;
            int gris=rgb_to_grey(r,g,b);

            //printf("gris: %d\n",gris);

            //valores de menos o igual de 33%, hay que poner marco
            //nota: tema solarized dark, tiene 33%
            //TODO: con el logo de xanniversary fuerzo siempre marco aunque esto no es del todo correcto,
            //pues los colores expuestos a izquierda y derecha son cyan y amarillo, a diferencia del negro del logo normal
            int umbral_marco=33;
            if (gris<=umbral_marco) {
                debug_printf(VERBOSE_DEBUG,"Drawing frame around footer logo because background colour intensity is less than %d%%",umbral_marco);
                color_marco='w';
            }

            if (xanniversary_logo.v) color_marco='w';

            //copiamos el logo a bitmap de destino cambiando el color del marco
            menu_footer_logo_copy_final(color_marco);

            screen_put_asciibitmap_generic(&zesarux_ascii_logo_footer[1],NULL,xlogo,yinicial,ZESARUX_ASCII_LOGO_ANCHO,24, 0,footer_logo_putpixel,zoom_logo,0);
        }


    }

}

void menu_footer_bottom_line(void)
{
    menu_footer_z88();
    menu_footer_zesarux_emulator();
}

void menu_footer_clear_bottom_line(void)
{

    //                         01234567890123456789012345678901
    menu_putstring_footer(0,2,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

}

//Escribir textos iniciales en el footer
void menu_init_footer(void)
{
    if (!menu_footer) return;


        //int margeny_arr=screen_borde_superior*border_enabled.v;

        if (MACHINE_IS_Z88) {
                //no hay border. estas variables se leen en modo rainbow
                //margeny_arr=0;
        }

    //Si no hay driver video
    if (scr_putpixel==NULL || scr_putpixel_zoom==NULL) return;

    debug_printf (VERBOSE_INFO,"init_footer");

    //Al iniciar emulador, si aun no hay definidas funciones putpixel, volver


    //Borrar footer con pixeles blancos
    menu_clear_footer();

    //Inicializar array footer
    cls_footer();


    //Borrar zona con espacios
    //Tantos espacios como el nombre mas largo de maquina (Microdigital TK90X (Spanish))
    menu_putstring_footer(0,0,"                            ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

    //Obtener maquina activa
    menu_putstring_footer(0,0,get_machine_name(current_machine_type),WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

    autoselect_options_put_footer();

    menu_footer_bottom_line();

    //Si hay lectura de flash activa en ZX-Uno
    //Esto lo hago asi porque al iniciar ZX-Uno, se ha activado el contador de texto "FLASH",
    //y en driver xwindows suele generar un evento ConfigureNotify, que vuelve a llamar a init_footer y borraria dicho texto FLASH
    //y por lo tanto no se veria el texto "FLASH" al arrancar la maquina
    //en otros drivers de video en teoria no haria falta
    //if (MACHINE_IS_ZXUNO) zxuno_footer_print_flash_operating();


}



//funcion para limpiar el buffer de overlay y si hay cuadrado activo
void cls_menu_overlay(void)
{
    int i;

    //Borrar solo el tamanyo de menu activo
    for (i=0;i<scr_get_menu_width()*scr_get_menu_height();i++) {
        overlay_screen_array[i].caracter=0;
        //overlay_usado_screen_array[i]=0;
//#ifdef ZXVISION_USE_CACHE_OVERLAY_TEXT
        overlay_screen_array[i].modificado=1;
//#endif
    }
    menu_desactiva_cuadrado();

        //si hay segunda capa, escribir la segunda capa en esta primera
    //copy_second_first_overlay();




    //Si es CPC, entonces aqui el border es variable y por tanto tenemos que redibujarlo, pues quiza el menu esta dentro de zona de border
    modificado_border.v=1;

    scr_clear_layer_menu();

    //Si en Z88 y driver grafico, redibujamos la zona inferior
    //Despues de scr_clear_layer_menu, porque si lo hacemos antes, se habria dibujado en layer menu,
    //se borra layer menu y entonces los pixeles que hemos dibujado antes se perderian...
    //Bueno esto es mas o menos lo que he deducido aunque no tiene porque ser 100% asi. Lo que si que es cierto
    //es que si esto se pone antes de scr_clear_layer_menu, teniendo real video off y maquina z88, no borra los menus
    //Tiene que ver tambien con que este screen_z88_draw_lower_screen esta usando putpixel normal de maquina,
    //pero deberia usar scr_putpixel_layer_menu/scr_putpixel_gui_zoom

    if (MACHINE_IS_Z88) {
        screen_z88_draw_lower_screen();
    }

    menu_draw_ext_desktop();

    zxvision_set_all_flag_dirty_must_draw_contents();

    //printf("cls menu overlay\n");

}



//funcion para escribir un caracter en el buffer de footer
//tinta y/o papel pueden tener brillo (color+8)
void putchar_footer_array(int x,int y,z80_byte caracter,int tinta,int papel,int parpadeo)
{

    if (!menu_footer) return;

    //int xfinal=((x*menu_char_width)+menu_char_width-1)/8;

    //Controlar limite
    if (x<0 || y<0 || x>=WINDOW_FOOTER_COLUMNS || y>=WINDOW_FOOTER_LINES) {
        //printf ("Out of range. X: %d Y: %d Character: %c\n",x,y,caracter);
        return;
    }

    if (ESTILO_GUI_SOLO_MAYUSCULAS) caracter=letra_mayuscula(caracter);

    int pos_array=y*WINDOW_FOOTER_COLUMNS+x;
    footer_screen_array[pos_array].tinta=tinta;
    footer_screen_array[pos_array].papel=papel;
    footer_screen_array[pos_array].parpadeo=parpadeo;
    footer_screen_array[pos_array].caracter=caracter;


}


void cls_footer(void)
{
    if (!menu_footer) return;

    int x,y;
    for (y=0;y<WINDOW_FOOTER_LINES;y++) {
        for (x=0;x<WINDOW_FOOTER_COLUMNS;x++) {
            putchar_footer_array(x,y,' ',WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER,0);
        }
    }

}

void redraw_footer_continue(void)
{
    if (!menu_footer) return;

    //printf ("redraw footer\n");

    int x,y;
    int tinta,papel,caracter,parpadeo;
    int pos_array=0;
    for (y=0;y<WINDOW_FOOTER_LINES;y++) {
        for (x=0;x<WINDOW_FOOTER_COLUMNS;x++,pos_array++) {

            caracter=footer_screen_array[pos_array].caracter;

            tinta=footer_screen_array[pos_array].tinta;
            papel=footer_screen_array[pos_array].papel;
            parpadeo=footer_screen_array[pos_array].parpadeo;

            //Si esta multitask, si es caracter con parpadeo y si el estado del contador del parpadeo indica parpadear
            if (menu_multitarea && parpadeo && estado_parpadeo.v) caracter=' '; //si hay parpadeo y toca, meter espacio tal cual (se oculta)

            scr_putchar_footer(x,y,caracter,tinta,papel);

        }
    }

    //agregar boton switch zx desktop
    menu_put_switch_zxdesktop_footer();

}

void redraw_footer(void)

{


    if (!menu_footer) return;

    //Sin interlaced
    if (video_interlaced_mode.v==0) {
        redraw_footer_continue();
        return;
    }


    //Con interlaced
    //Queremos que el footer se vea bien, no haga interlaced y no haga scanlines
    //Cuando se activa interlaced se cambia la funcion de putpixel, por tanto,
    //desactivando aqui interlaced no seria suficiente para que el putpixel saliese bien


    //No queremos que le afecte el scanlines
    z80_bit antes_scanlines;
    antes_scanlines.v=video_interlaced_scanlines.v;
    video_interlaced_scanlines.v=0;

    //Escribe texto pero como hay interlaced, lo hará en una linea de cada 2
    redraw_footer_continue();

    //Dado que hay interlaced, simulamos que estamos en siguiente frame de pantalla para que dibuje la linea par/impar siguiente
    interlaced_numero_frame++;
    redraw_footer_continue();
    interlaced_numero_frame--;

    //restaurar scanlines
    video_interlaced_scanlines.v=antes_scanlines.v;

}

//Esta funcion antes se usaba para poner color oscuro o no al abrir el menu
//Actualmente solo cambia el valor de menu_abierto
void menu_set_menu_abierto(int valor)
{

        menu_abierto=valor;
}

//Para meter el logo u otros botones en zona de extended desktop
void menu_draw_ext_desktop_putpixel_bitmap(z80_int *destino GCC_UNUSED,int x,int y,int ancho GCC_UNUSED,int color)
{
    //Controlar si se sale de rango
    int total_width=screen_get_total_width_window_plus_zxdesktop();
    int total_height=screen_get_total_height_window_no_footer_plus_zxdesktop();

    if (x<0 || x>=total_width || y<0 || y>=total_height) return;

    //Ver si icono en posicion de pantalla emulada
    int ancho_maquina=screen_get_emulated_display_width_zoom_border_en();
    int alto_maquina=screen_get_emulated_display_height_zoom_border_en();

    if (x<ancho_maquina && y<alto_maquina) {
        //printf("Retorno %d %d\n",x,y);
        return;
    }

    scr_putpixel(x,y,color);
}

//Para el texto de los iconos
void menu_draw_ext_desktop_putpixel_bitmap_icon_text(z80_int *destino GCC_UNUSED,int x,int y,int ancho GCC_UNUSED,int color GCC_UNUSED)
{
    //Controlar si se sale de rango
    int total_width=screen_get_total_width_window_plus_zxdesktop();
    int total_height=screen_get_total_height_window_no_footer_plus_zxdesktop();

    if (x<0 || x>=total_width || y<0 || y>=total_height) return;

    //Ver si icono en posicion de pantalla emulada
    int ancho_maquina=screen_get_emulated_display_width_zoom_border_en();
    int alto_maquina=screen_get_emulated_display_height_zoom_border_en();

    if (x<ancho_maquina && y<alto_maquina) {
        //printf("Retorno %d %d\n",x,y);
        return;
    }

    //El color es el del estilo
    scr_putpixel(x,y,ESTILO_GUI_TINTA_NORMAL);
}



//Retorna geometria de los botones, si punteros no son null
//ancho, alto boton
//xfinal_botones: posicion X mas a la derecha del ultimo boton
void menu_ext_desktop_buttons_get_geometry(int *p_ancho_boton,int *p_alto_boton,int *p_total_botones,int *p_inicio_botones,int *p_xfinal_botones)
{
    int total_botones=EXT_DESKTOP_TOTAL_BUTTONS;

    int ancho_zx_desktop=screen_get_ext_desktop_width_zoom();
    int xinicio=screen_get_ext_desktop_start_x();


    int ancho_boton=ancho_zx_desktop/total_botones;

    //Minimo 32 pixeles
    if (ancho_boton<EXT_DESKTOP_BUTTONS_TOTAL_SIZE) ancho_boton=EXT_DESKTOP_BUTTONS_TOTAL_SIZE;

    //Maximo 64 pixeles
    if (ancho_boton>EXT_DESKTOP_BUTTONS_TOTAL_SIZE*2) ancho_boton=EXT_DESKTOP_BUTTONS_TOTAL_SIZE*2;


    int alto_boton=ancho_boton;

    int xfinal_ventana=xinicio+ancho_zx_desktop;

    int xfinal_botones=xinicio+total_botones*ancho_boton;

    //no caben todos los botones
    if (xfinal_botones>xfinal_ventana) {
        total_botones=ancho_zx_desktop/ancho_boton;
        xfinal_botones=xinicio+total_botones*ancho_boton;
    }

    if (p_ancho_boton!=NULL) *p_ancho_boton=ancho_boton;
    if (p_alto_boton!=NULL) *p_alto_boton=alto_boton;
    if (p_total_botones!=NULL) *p_total_botones=total_botones;
    if (p_inicio_botones!=NULL) *p_inicio_botones=xinicio;
    if (p_xfinal_botones!=NULL) *p_xfinal_botones=xfinal_botones;

}


//Retorna geometria de los lower icons, si punteros no son null
//ancho, alto boton
//xfinal_botones: posicion X mas a la derecha del ultimo boton
void menu_ext_desktop_lower_icons_get_geometry(int *p_ancho_boton,int *p_alto_boton,int *p_total_botones,int *p_xinicio_botones,int *p_xfinal_botones,int *p_yinicio_botones)
{

    //int total_botones=TOTAL_ZXDESKTOP_MAX_LOWER_ICONS;

    //Considerar los botones que estan visibles solamente para la geometria, no todos los posibles


    int total_botones=0;


    int i;


    for (i=0;i<TOTAL_ZXDESKTOP_MAX_LOWER_ICONS;i++) {
        int (*funcion_is_visible)(void);
        funcion_is_visible=zdesktop_lowericons_array[i].is_visible;

        int visible=funcion_is_visible();

//Para poder forzar visibilidad de iconos para debug
#ifdef FORCE_VISIBLE_ALL_LOWER_ICONS
    visible=1;
#endif
        if (visible) {

            total_botones++;
        }
    }

    //printf ("total iconos visibles: %d\n",total_botones);

    int ancho_zx_desktop=screen_get_ext_desktop_width_zoom();
    int xinicio=screen_get_ext_desktop_start_x();


    int ancho_boton;

    if (total_botones==0) {
        //evitar división por cero
        ancho_boton=32;
    }
    else {
        ancho_boton=ancho_zx_desktop/total_botones;
    }

    //Minimo 32 pixeles
    if (ancho_boton<EXT_DESKTOP_BUTTONS_TOTAL_SIZE) ancho_boton=EXT_DESKTOP_BUTTONS_TOTAL_SIZE;

    //Maximo 64 pixeles
    if (ancho_boton>EXT_DESKTOP_BUTTONS_TOTAL_SIZE*2) ancho_boton=EXT_DESKTOP_BUTTONS_TOTAL_SIZE*2;


    int alto_boton=ancho_boton;

    int xfinal_ventana=xinicio+ancho_zx_desktop;

    int xfinal_botones=xinicio+total_botones*ancho_boton;

    //no caben todos los botones
    if (xfinal_botones>xfinal_ventana) {
        total_botones=ancho_zx_desktop/ancho_boton;
        xfinal_botones=xinicio+total_botones*ancho_boton;
    }


/*

    int xinicio=screen_get_ext_desktop_start_x();


    int ancho=screen_get_ext_desktop_width_zoom();


    int xfinal;



    int nivel_zoom=1;

    //Si hay espacio para meter iconos con zoom 2
    //6 pixeles de margen
    if (ancho_boton>=(6+EXT_DESKTOP_BUTTONS_ANCHO*2)) nivel_zoom=2;




*/
    int alto_zx_desktop=screen_get_total_alto_ventana_zoom();
    int yinicio=alto_zx_desktop-alto_boton;

    //printf ("alto_boton: %d alto_zx_desktop: %d yinicio: %d\n",alto_boton,alto_zx_desktop,yinicio);

    if (p_ancho_boton!=NULL) *p_ancho_boton=ancho_boton;
    if (p_alto_boton!=NULL) *p_alto_boton=alto_boton;
    if (p_total_botones!=NULL) *p_total_botones=total_botones;
    if (p_xinicio_botones!=NULL) *p_xinicio_botones=xinicio;
    if (p_xfinal_botones!=NULL) *p_xfinal_botones=xfinal_botones;
    if (p_yinicio_botones!=NULL) *p_yinicio_botones=yinicio;

}

//Si se muestra el boton de cerrar todos menus.
//Nota: aunque no se muestre, el evento de cerrar los menus se produce aunque el menu este cerrado,
//pero como cierra los menus, se abre y se cierra y no se nota nada
z80_bit menu_mostrar_boton_close_all_menus={0};

//Dice si hay que dibujar ese boton. Normalmente todos se ven, excepto el de cerrar menus cuando menu esta cerrado
int menu_si_dibujar_boton(int numero_boton)
{
    //Si no es el boton de cerrar todo y menu esta cerrado y por tanto no se ve
    int mostrar=1;

    if (menu_mostrar_boton_close_all_menus.v==0 && numero_boton==EXT_DESKTOP_BUTTON_CLOSE_ALL_ID) mostrar=0;
    //if (!menu_abierto && numero_boton==EXT_DESKTOP_BUTTON_CLOSE_ALL_ID) mostrar=0;

    //if (numero_boton==EXT_DESKTOP_BUTTON_CLOSE_ALL_ID) printf("menu_si_dibujar_boton. mostrar: %d menu_mostrar_boton_close_all_menus %d menu_abierto: %d\n",
    //    mostrar,menu_mostrar_boton_close_all_menus.v,menu_abierto);

    return mostrar;
}

void menu_draw_ext_desktop_recuadro_button(int xinicio,int yinicio,int ancho_boton,int alto_boton,int color_recuadro)
{

    int x,y;

    //Horizontal
    for (x=xinicio+1;x<xinicio+ancho_boton-1;x++) {
        scr_putpixel(x,yinicio+1,color_recuadro);
        scr_putpixel(x,yinicio+alto_boton-2,color_recuadro);
    }

    //Vertical
    for (y=yinicio+1;y<yinicio+alto_boton-1;y++) {
        scr_putpixel(xinicio+1,y,color_recuadro);
        scr_putpixel(xinicio+ancho_boton-2,y,color_recuadro);
    }
}

void menu_draw_ext_desktop_one_button_background(int contador_boton,int pulsado)
{

    if (!menu_si_dibujar_boton(contador_boton)) return;

    int ancho_boton;
    int alto_boton;





    menu_ext_desktop_buttons_get_geometry(&ancho_boton,&alto_boton,NULL,NULL,NULL);


    //printf ("upper icons background. ancho boton: %d alto boton: %d\n",ancho_boton,alto_boton);


    //int nivel_zoom=1;

    //Si hay espacio para meter iconos con zoom 2
    //6 pixeles de margen
    //if (ancho_boton>=(6+EXT_DESKTOP_BUTTONS_ANCHO*2)) nivel_zoom=2;


        int xinicio=screen_get_ext_desktop_start_x();

        xinicio +=contador_boton*ancho_boton;
        int yinicio=0;

    int x,y;

    //Rectangulo alrededor. Dejando margen de 1 pixel alrededor sin tocar


    int color_recuadro=0;
    int color_relleno=7;

    if (pulsado) color_recuadro=7;

    if (menu_ext_desktop_disable_box_upper_icons.v==0) {
        menu_draw_ext_desktop_recuadro_button(xinicio,yinicio,ancho_boton,alto_boton,color_recuadro);
    }



    //Se rellena solo cuando se pulsa el botón o cuando no hay transparencia
    if (pulsado || menu_ext_desktop_transparent_upper_icons.v==0) {
        for (y=yinicio+2;y<yinicio+alto_boton-2;y++) {
            for (x=xinicio+2;x<xinicio+ancho_boton-2;x++) {
                scr_putpixel(x,y,color_relleno);
            }
        }
    }



}


void menu_draw_ext_desktop_one_lower_icon_background(int contador_boton,int pulsado)
{

    int ancho_boton;
    int alto_boton;


    //int xfinal;

    int yinicio; //=0;


    menu_ext_desktop_lower_icons_get_geometry(&ancho_boton,&alto_boton,NULL,NULL,NULL,&yinicio);


    //printf ("lower icons background. ancho boton: %d alto boton: %d\n",ancho_boton,alto_boton);


    //int nivel_zoom=1;

    //Si hay espacio para meter iconos con zoom 2
    //6 pixeles de margen
    //if (ancho_boton>=(6+EXT_DESKTOP_BUTTONS_ANCHO*2)) nivel_zoom=2;

    //printf ("zoom lower icons background: %d\n",nivel_zoom);

        int xinicio=screen_get_ext_desktop_start_x();

        xinicio +=contador_boton*ancho_boton;


    int x,y;

    //Rectangulo alrededor. Dejando margen de 1 pixel alrededor sin tocar


    int color_recuadro=0;
    int color_relleno=7;

    if (pulsado) color_recuadro=7;

    if (menu_ext_desktop_disable_box_lower_icons.v==0) {
        menu_draw_ext_desktop_recuadro_button(xinicio,yinicio,ancho_boton,alto_boton,color_recuadro);
    }




    //Se rellena solo cuando se pulsa el botón o cuando no hay transparencia
    if (pulsado || menu_ext_desktop_transparent_lower_icons.v==0) {
        for (y=yinicio+2;y<yinicio+alto_boton-2;y++) {
            for (x=xinicio+2;x<xinicio+ancho_boton-2;x++) {
                scr_putpixel(x,y,color_relleno);
            }
        }
    }




}


char **menu_get_extdesktop_button_bitmap(int numero_boton,int *es_set_machine)
{
    char **puntero_bitmap;

    *es_set_machine=0;

    //por defecto
    puntero_bitmap=zxdesktop_buttons_bitmaps[numero_boton];
    puntero_bitmap=alter_zesarux_ascii_logo(puntero_bitmap);

    int boton_id=numero_boton-1;

    //El 0 no esta permitido
    if (boton_id>=0 && boton_id<MAX_USERDEF_BUTTONS) {

        enum defined_f_function_ids accion;

        int indice_tabla=defined_buttons_functions_array[boton_id];
        accion=menu_da_accion_direct_functions_indice(indice_tabla);


        if (accion!=F_FUNCION_DEFAULT) {
            puntero_bitmap=get_direct_function_icon_bitmap_final(indice_tabla);


            //Si icono es OPEN_WINDOW, adoptar icono de la ventana que se va a abrir
            //printf("Accion open window\n");
            if (accion==F_FUNCION_OPEN_WINDOW) {
                char *geometry_name;
                geometry_name=defined_buttons_functions_array_parameters[boton_id];
                char **possible_bitmap=zxvision_find_icon_for_known_window(geometry_name);
                if (possible_bitmap!=NULL) puntero_bitmap=possible_bitmap;
            }



            //Si icono es F_FUNCION_SET_MACHINE y tiene parametro de set machine, dibujamos el icono de la maquina y luego la "flechita"
            //De tal manera que estamos dibujando un icono sobre el otro. Este es el unico caso de momento que hago eso
            if (accion==F_FUNCION_SET_MACHINE) {


                char *machine_name;
                machine_name=defined_buttons_functions_array_parameters[boton_id];

                if (machine_name[0]) {

                    //Obtener bitmap en base al parametro
                    puntero_bitmap=get_machine_icon_by_name(machine_name);

                    *es_set_machine=1;
                }

            }


        }



    }


    return puntero_bitmap;
}


//Dibujar un boton con su bitmap, con efecto pulsado si/no
void menu_draw_ext_desktop_one_button_bitmap(int numero_boton,int pulsado,int yinicio)
{

    if (!menu_si_dibujar_boton(numero_boton)) return;

    int total_botones;


    //Tamanyo fijo


    //Tamanyo variable segun tamanyo ZX Desktop. Iconos con contenido 26x26.
    //Hay que dejar margen de 6 por cada lado (3 izquierdo, 3 derecho, 3 alto, 3 alto)
    //Cada 3 pixeles de margen son: fondo-negro(rectangulo)-gris(de dentro boton)
    //total maximo 32x32
    //Ejemplo:
    /*

char *zesarux_ascii_logo[ZESARUX_ASCII_LOGO_ALTO]={
  ................................
  ################################
  --------------------------------
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",     //0
      "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WWWWWWWWWWWWWWWWWXXXXWWWWW",
    "                WXXXXW   W",
    "                WXXXXW  RW",
    "             WWWWXXXXW RRW",
    "            WXXXXWWWW RRRW",
    "            WXXXXW   RRRRW",	//10
    "            WXXXXW  RRRRYW",
    "         WWWWXXXXW RRRRYYW",
    "        WXXXXWWWW RRRRYYYW",
    "        WXXXXW   RRRRYYYYW",
    "        WXXXXW  RRRRYYYYGW",
    "     WWWWXXXXW RRRRYYYYGGW",
    "    WXXXXWWWW RRRRYYYYGGGW",
    "    WXXXXW   RRRRYYYYGGGGW",
    "    WXXXXW  RRRRYYYYGGGGCW",
    "WWWWWXXXXW RRRRYYYYGGGGCCW",    //20
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW" 		//25
};
    */

    int ancho_boton;
    int alto_boton;

        int xinicio=screen_get_ext_desktop_start_x();


    int xfinal;

    menu_ext_desktop_buttons_get_geometry(&ancho_boton,&alto_boton,&total_botones,NULL,&xfinal);

    //printf("draw bitmap ancho_boton %d alto_boton %d total_botones %d xfinal %d\n",
    //ancho_boton,alto_boton,total_botones,xfinal);


    int nivel_zoom=1;

    //Si hay espacio para meter iconos con zoom 2
    //6 pixeles de margen
    if (ancho_boton>=(6+EXT_DESKTOP_BUTTONS_ANCHO*2)) nivel_zoom=2;


    //int x;
    //int contador_boton=0;





    //Dibujar un boton


    if (numero_boton<total_botones) {
    //for (numero_boton=0;numero_boton<total_botones;numero_boton++) {


        int medio_boton_x=ancho_boton/2;
        int medio_boton_y=alto_boton/2;

        int destino_x=xinicio+ancho_boton*numero_boton;
        destino_x +=medio_boton_x-(EXT_DESKTOP_BUTTONS_ANCHO*nivel_zoom)/2;

        int destino_y=yinicio;
        destino_y +=medio_boton_y-(EXT_DESKTOP_BUTTONS_ALTO*nivel_zoom)/2;



        char **puntero_bitmap;

        int es_set_machine;

        puntero_bitmap=menu_get_extdesktop_button_bitmap(numero_boton,&es_set_machine);


        if (pulsado) {
            destino_x+=2;
            destino_y+=2;
        }

        if (es_set_machine) {
            screen_put_asciibitmap_generic(puntero_bitmap,NULL,destino_x,destino_y,ZESARUX_ASCII_LOGO_ANCHO,ZESARUX_ASCII_LOGO_ALTO, 0,menu_draw_ext_desktop_putpixel_bitmap,nivel_zoom,0);

            //Y continuamos hacia abajo diciendo que dibuje la flecha
            puntero_bitmap=bitmap_button_ext_desktop_set_machine_only_arrow;

        }

        screen_put_asciibitmap_generic(puntero_bitmap,NULL,destino_x,destino_y,ZESARUX_ASCII_LOGO_ANCHO,ZESARUX_ASCII_LOGO_ALTO, 0,menu_draw_ext_desktop_putpixel_bitmap,nivel_zoom,0);
    }
}





//Busca el N elemento dentro del array y que este visible
//Por ejemplo si los primeros 3 elementos están visibles/no:
//visible-novisible-visible
//Al ir a buscar el segundo elemento (parametro=1) se saltara el novisible y retornara el tercero visible
//esto es para evitar huecos al dibujarlos en pantalla
//Retorna <0 si no lo encuentra
int zxdesktop_lowericon_find_index(int icono)
{
    //int total_botones;
    int total_botones=TOTAL_ZXDESKTOP_MAX_LOWER_ICONS;

    if (icono>=total_botones || icono<0) return -1;

    int i;

    int i_enabled=0;

    for (i=0;i<total_botones;i++) {
        int (*funcion_is_visible)(void);
        funcion_is_visible=zdesktop_lowericons_array[i].is_visible;

        int visible=funcion_is_visible();

//Para poder forzar visibilidad de iconos para debug
#ifdef FORCE_VISIBLE_ALL_LOWER_ICONS
    visible=1;
#endif

        if (visible) {
            if (i_enabled==icono) {
                //printf("buscando %d encontrado indice %d\n",icono,i);
                return i;
            }

            i_enabled++;
        }
    }

    return -1;

}

int lowericon_realtape_frame=0;
int lowericon_cf2_floppy_frame=0;

void menu_ext_desktop_draw_lower_icon(int numero_boton,int pulsado)
{



    int total_botones;

    int ancho_boton;
    int alto_boton;

    int yinicio;

    menu_ext_desktop_lower_icons_get_geometry(&ancho_boton,&alto_boton,&total_botones,NULL,NULL,&yinicio);

    //printf("yinicio: %d\n",)

    //printf ("menu_ext_desktop_draw_lower_icon. numero_boton %d total_botones %d\n",numero_boton,total_botones);
    if (numero_boton>=total_botones) {
        //printf ("no dibujar\n");
        return;
    }


    //Buscar el indice dentro del array de botones
    int indice_array;

    indice_array=zxdesktop_lowericon_find_index(numero_boton);

    if (indice_array<0) return;



    //Nota: en los botones superiores el background se dibuja al principio para todos
    //mientras que en estos lower icons se dibuja cada background con su icono asociado
    //Quiza habria que estandarizar esto
    menu_draw_ext_desktop_one_lower_icon_background(numero_boton,pulsado);

    int xinicio=screen_get_ext_desktop_start_x();




    int nivel_zoom=1;

    //Si hay espacio para meter iconos con zoom 2
    //6 pixeles de margen
    if (ancho_boton>=(6+EXT_DESKTOP_BUTTONS_ANCHO*2)) nivel_zoom=2;

    //printf ("zoom lower icons: %d\n",nivel_zoom);



    //Dibujar un boton



    int medio_boton_x=ancho_boton/2;
    int medio_boton_y=alto_boton/2;

    int destino_x=xinicio+ancho_boton*numero_boton;
    destino_x +=medio_boton_x-(EXT_DESKTOP_BUTTONS_ANCHO*nivel_zoom)/2;

    int destino_y=yinicio; //+3*nivel_zoom; //3 de margen
    destino_y +=medio_boton_y-(EXT_DESKTOP_BUTTONS_ALTO*nivel_zoom)/2;


    char **puntero_bitmap;



    //Funcion is visible
    int (*funcion_is_enabled)(void);


    funcion_is_enabled=zdesktop_lowericons_array[indice_array].is_active;

    int is_enabled=funcion_is_enabled();

#ifdef FORCE_DISABLED_ALL_LOWER_ICONS
    is_enabled=0;
#endif

#ifdef FORCE_ENABLED_ALL_LOWER_ICONS
    is_enabled=1;
#endif

    //Ver si icono se dibuja en color inverso solo cuando esta el icono activo
    int inverso=0;


    if (is_enabled) {
        puntero_bitmap=zdesktop_lowericons_array[indice_array].bitmap_active;

        int *puntero_a_inverso;

        puntero_a_inverso=zdesktop_lowericons_array[indice_array].icon_is_inverse;
        if (*puntero_a_inverso) {
            inverso=1;
        }
    }

    else  {
        puntero_bitmap=zdesktop_lowericons_array[indice_array].bitmap_inactive;

        //Se deben mostrar como inversos los iconos con actividad, aunque no esten habilitados?
        int *puntero_a_inverso;

        puntero_a_inverso=zdesktop_lowericons_array[indice_array].icon_is_inverse;
        if (*puntero_a_inverso) {
            inverso=1;
        }

    }


    //Caso especial para realtape moviendose
    //printf("tape 00\n");
    if (puntero_bitmap==bitmap_lowericon_ext_desktop_cassette_active/* && inverso*/) {
        //printf("tape 0\n");
        if (realtape_inserted.v) {

            //printf("tape\n");

            if (lowericon_realtape_frame==1) puntero_bitmap=bitmap_lowericon_ext_desktop_cassette_active_frametwo;
            if (lowericon_realtape_frame==2) puntero_bitmap=bitmap_lowericon_ext_desktop_cassette_active_framethree;
            if (lowericon_realtape_frame==3) puntero_bitmap=bitmap_lowericon_ext_desktop_cassette_active_framefour;

            inverso=0;
        }
    }

    //Caso especial para disco moviendose

    if (puntero_bitmap==bitmap_lowericon_ext_desktop_plus3_flp_active) {

        if (dskplusthree_emulation.v && pd765_motor_status) {

            //Iconos cuando esta grabando (write, format...)
            if (cf2_floppy_icon_is_saving) {
                if (lowericon_cf2_floppy_frame==0) puntero_bitmap=bitmap_lowericon_ext_desktop_plus3_flp_active_save_framezero;
                if (lowericon_cf2_floppy_frame==1) puntero_bitmap=bitmap_lowericon_ext_desktop_plus3_flp_active_save_frameone;
                if (lowericon_cf2_floppy_frame==2) puntero_bitmap=bitmap_lowericon_ext_desktop_plus3_flp_active_save_frametwo;
                if (lowericon_cf2_floppy_frame==3) puntero_bitmap=bitmap_lowericon_ext_desktop_plus3_flp_active_save_framethree;
            }

            else {
                if (lowericon_cf2_floppy_frame==0) puntero_bitmap=bitmap_lowericon_ext_desktop_plus3_flp_active_framezero;
                if (lowericon_cf2_floppy_frame==1) puntero_bitmap=bitmap_lowericon_ext_desktop_plus3_flp_active_frameone;
                if (lowericon_cf2_floppy_frame==2) puntero_bitmap=bitmap_lowericon_ext_desktop_plus3_flp_active_frametwo;
                if (lowericon_cf2_floppy_frame==3) puntero_bitmap=bitmap_lowericon_ext_desktop_plus3_flp_active_framethree;
            }
            inverso=0;
        }
    }

    //Caso especial para slots z88 con tapa abierta. Al abrir la tapa
    //no se ve el numero
    if (MACHINE_IS_Z88 && z88_flap_is_open() ) {
        //con tarjeta insertada
        if (
            puntero_bitmap==bitmap_lowericon_ext_desktop_z88_slot_one_active ||
            puntero_bitmap==bitmap_lowericon_ext_desktop_z88_slot_two_active ||
            puntero_bitmap==bitmap_lowericon_ext_desktop_z88_slot_three_active
        ) {

            puntero_bitmap=bitmap_lowericon_ext_desktop_z88_slot_nonumber_active;
        }

        //sin tarjeta insertada
        if (
            puntero_bitmap==bitmap_lowericon_ext_desktop_z88_slot_one_inactive ||
            puntero_bitmap==bitmap_lowericon_ext_desktop_z88_slot_two_inactive ||
            puntero_bitmap==bitmap_lowericon_ext_desktop_z88_slot_three_inactive
        ) {

            puntero_bitmap=bitmap_lowericon_ext_desktop_z88_slot_nonumber_inactive;
        }
    }


    if (pulsado) {
        //desplazado 2 pixel cuando se pulsa
        destino_x+=2;
        destino_y+=2;
    }


    screen_put_asciibitmap_generic(puntero_bitmap,NULL,destino_x,destino_y,ZESARUX_ASCII_LOGO_ANCHO,ZESARUX_ASCII_LOGO_ALTO, 0,menu_draw_ext_desktop_putpixel_bitmap,nivel_zoom,inverso);



}


void menu_draw_ext_desktop_lower_icons(void)
{

    if (menu_zxdesktop_lower_buttons_enabled.v==0) return;

    int total_iconos=TOTAL_ZXDESKTOP_MAX_LOWER_ICONS;


    int i;

    for (i=0;i<total_iconos;i++) {
        menu_ext_desktop_draw_lower_icon(i,0);

    }
}

void menu_draw_ext_desktop_dibujar_boton_pulsado(int boton)
{
    menu_draw_ext_desktop_one_button_background(boton,1);
    menu_draw_ext_desktop_one_button_bitmap(boton,1,0);
}

void menu_draw_ext_desktop_upper_icons(int xinicio)
{

    /*
    0           1          2         3        4           5        6         7       8         9         10        11     12
    ZEsarUX | Smartload | Machine | Storage | Snapshot | Audio | Network | Debug | Display | Windows | Settings | Help | Exit
    */
    int total_botones;

    //total_botones=EXT_DESKTOP_TOTAL_BUTTONS;

    //Tamanyo fijo


    //Tamanyo variable segun tamanyo ZX Desktop. Iconos con contenido 26x26.
    //Hay que dejar margen de 6 por cada lado (3 izquierdo, 3 derecho, 3 alto, 3 alto)
    //Cada 3 pixeles de margen son: fondo-negro(rectangulo)-gris(de dentro boton)
    //total maximo 32x32
    //Ejemplo:
    /*

char *zesarux_ascii_logo[ZESARUX_ASCII_LOGO_ALTO]={
................................
################################
--------------------------------
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",     //0
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WWWWWWWWWWWWWWWWWXXXXWWWWW",
    "                WXXXXW   W",
    "                WXXXXW  RW",
    "             WWWWXXXXW RRW",
    "            WXXXXWWWW RRRW",
    "            WXXXXW   RRRRW",	//10
    "            WXXXXW  RRRRYW",
    "         WWWWXXXXW RRRRYYW",
    "        WXXXXWWWW RRRRYYYW",
    "        WXXXXW   RRRRYYYYW",
    "        WXXXXW  RRRRYYYYGW",
    "     WWWWXXXXW RRRRYYYYGGW",
    "    WXXXXWWWW RRRRYYYYGGGW",
    "    WXXXXW   RRRRYYYYGGGGW",
    "    WXXXXW  RRRRYYYYGGGGCW",
    "WWWWWXXXXW RRRRYYYYGGGGCCW",    //20
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WXXXXXXXXXXXXXXXXXXXXXXXXW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW" 		//25
};
    */

    int ancho_boton;
    int alto_boton;


    int xfinal;


    menu_ext_desktop_buttons_get_geometry(&ancho_boton,&alto_boton,&total_botones,NULL,&xfinal);

    //printf("draw buttons ancho_boton %d alto_boton %d total_botones %d xfinal %d\n",
    //ancho_boton,alto_boton,total_botones,xfinal);


    //int nivel_zoom=1;

    //Si hay espacio para meter iconos con zoom 2
    //6 pixeles de margen
    //if (ancho_boton>=(6+EXT_DESKTOP_BUTTONS_ANCHO*2)) nivel_zoom=2;


    int x;
    int contador_boton=0;

    for (x=xinicio;contador_boton<total_botones;x+=ancho_boton,contador_boton++) {
        menu_draw_ext_desktop_one_button_background(contador_boton,0);
    }


    //Boton ZEsarUX en primer botón. Centrado
    /*
    screen_put_watermark_generic(NULL,xinicio+ancho_boton/2-ZESARUX_ASCII_LOGO_ANCHO/2,yinicio+alto_boton/2-ZESARUX_ASCII_LOGO_ALTO/2
        ,0, menu_draw_ext_desktop_putpixel_bitmap);

*/


    //Dibujar botones
    int numero_boton;
    for (numero_boton=0;numero_boton<total_botones;numero_boton++) {

        menu_draw_ext_desktop_one_button_bitmap(numero_boton,0,0);


    }

}

void menu_draw_ext_desktop_buttons(int xinicio)
{

    if (menu_zxdesktop_upper_buttons_enabled.v && zxvision_topbar_menu_enabled.v==0) {
        menu_draw_ext_desktop_upper_icons(xinicio);
    }


    if (menu_zxdesktop_lower_buttons_enabled.v) {
        menu_draw_ext_desktop_lower_icons();
    }


    //mostrar todos los iconos de la parte superior, pero solo para modo debug
#ifdef FORCE_VISIBLE_ALL_UPPER_ICONS

    for (numero_boton=0;numero_boton<MAX_F_FUNCTIONS;numero_boton++) {

        //int yinicio;


        int total_en_fila=12;

        //saltar cada 12 iconos
        int yinicio=((numero_boton/12)*70)+70;


        int pulsado=0;





        int ancho_boton;
        int alto_boton;

        int xinicio=screen_get_ext_desktop_start_x();


        int xfinal;

        menu_ext_desktop_buttons_get_geometry(&ancho_boton,&alto_boton,&total_botones,NULL,&xfinal);




        int nivel_zoom=1;

        //Si hay espacio para meter iconos con zoom 2
        //6 pixeles de margen
        if (ancho_boton>=(6+EXT_DESKTOP_BUTTONS_ANCHO*2)) nivel_zoom=2;



    //for (numero_boton=0;numero_boton<total_botones;numero_boton++) {


        int medio_boton_x=ancho_boton/2;
        int medio_boton_y=alto_boton/2;

        int destino_x=xinicio+ancho_boton*(numero_boton % total_en_fila);
        destino_x +=medio_boton_x-(EXT_DESKTOP_BUTTONS_ANCHO*nivel_zoom)/2;

        int destino_y=yinicio;
        destino_y +=medio_boton_y-(EXT_DESKTOP_BUTTONS_ALTO*nivel_zoom)/2;




//fondo
    int color_recuadro=0;
    int color_relleno=7;

    if (pulsado) color_recuadro=7;

    if (menu_ext_desktop_disable_box_upper_icons.v==0) {
        menu_draw_ext_desktop_recuadro_button(destino_x-6,destino_y-6,ancho_boton,alto_boton,color_recuadro);
    }



    //Se rellena solo cuando se pulsa el botón o cuando no hay transparencia
    int y;
    if (pulsado || menu_ext_desktop_transparent_upper_icons.v==0) {
        for (y=destino_y-4;y<destino_y+alto_boton-8;y++) {
            for (x=destino_x-4;x<destino_x+ancho_boton-8;x++) {
                scr_putpixel(x,y,color_relleno);
            }
        }
    }





        char **puntero_bitmap;


        //puntero_bitmap=menu_get_extdesktop_button_bitmap(numero_boton);

        //forzamos siempre custom buttons
        puntero_bitmap=defined_direct_functions_array[numero_boton].bitmap_button;



        screen_put_asciibitmap_generic(puntero_bitmap,NULL,destino_x,destino_y,ZESARUX_ASCII_LOGO_ANCHO,ZESARUX_ASCII_LOGO_ALTO, 0,menu_draw_ext_desktop_putpixel_bitmap,nivel_zoom,0);






    }

#endif


}

int menu_get_ext_desktop_icons_zoom(void)
{
    int zoom;

    //darle el nivel de zoom x o y, el que sea mayor. normalmente los dos niveles de zoom son iguales, pero por si acaso
    if (zoom_x>zoom_y) zoom=zoom_x;
    else zoom=zoom_y;

    //Tamaño iconos multiplicado por el zoom de la interfaz
    zoom *=menu_gui_zoom;

    return zoom;
}

void menu_draw_ext_desktop_one_icon(int x,int y,char **puntero_bitmap)
{



    int zoom=menu_get_ext_desktop_icons_zoom();

    //Aplicar mascara blanca debajo
    //screen_put_mask_asciibitmap_generic_offset_inicio(puntero_bitmap,NULL,x,y,ZESARUX_ASCII_LOGO_ANCHO,ZESARUX_ASCII_LOGO_ALTO, 0,
    //    menu_draw_ext_desktop_putpixel_bitmap,zoom,0,7);

    //Y dibujar bitmap
    screen_put_asciibitmap_generic(puntero_bitmap,NULL,x,y,ZESARUX_ASCII_LOGO_ANCHO,ZESARUX_ASCII_LOGO_ALTO, 0,
        menu_draw_ext_desktop_putpixel_bitmap,zoom,0);

}


//Escribir texto del icono
void menu_draw_ext_desktop_one_icon_text(int x,int y,char *texto)
{


    int zoom;

    //darle el nivel de zoom x o y, el que sea menor
    if (zoom_x<zoom_y) zoom=zoom_x;
    else zoom=zoom_y;

    zoom *=menu_gui_zoom;

    int ancho_caracter=CHARSET_ICONS_ANCHO;
    int alto_caracter=CHARSET_ICONS_ALTO;


    while (*texto) {
        unsigned char c=*texto;
        texto++;

        if (ESTILO_GUI_SOLO_MAYUSCULAS) c=letra_mayuscula(c);

        if (c<32 || c>126) c='?';

        int offset=(c-32)*alto_caracter;
        //printf("c %d offset %d\n",c,offset);


        //TODO: esta funcion no permite zoom X y zoom Y diferentes, solo permite uno
        //en el caso del texto le enviamos el zoom que sea mas pequeño
        screen_put_asciibitmap_generic_offset_inicio(charset_icons_text,NULL,x,y,ancho_caracter,alto_caracter, 0,
            menu_draw_ext_desktop_putpixel_bitmap_icon_text,zoom,0,offset);


        x +=(ancho_caracter+1)*zoom_x*menu_gui_zoom;     //El ancho de caracter +1 para que no queden pegados

    }


}

int menu_get_ext_desktop_icons_size(void)
{
    return ZESARUX_ASCII_LOGO_ANCHO*menu_get_ext_desktop_icons_zoom();
}

void menu_get_ext_desktop_icons_position(int index,int *p_x,int *p_y)
{
    int x=zxdesktop_configurable_icons_list[index].pos_x;
    int y=zxdesktop_configurable_icons_list[index].pos_y;

    //Posicion considerando zoom
    x *=zoom_x;
    y *=zoom_y;

    *p_x=x;
    *p_y=y;
}




void menu_draw_ext_desktop_one_configurable_icon_background(int xinicio,int yinicio,int ancho_boton,int alto_boton,int color_relleno)
{

    //Controlar si se sale de rango
    int total_width=screen_get_total_width_window_plus_zxdesktop();
    int total_height=screen_get_total_height_window_no_footer_plus_zxdesktop();

    int ancho_maquina=screen_get_emulated_display_width_zoom_border_en();
    int alto_maquina=screen_get_emulated_display_height_zoom_border_en();

    int x,y;

    for (y=yinicio;y<yinicio+alto_boton+2;y++) {
        for (x=xinicio;x<xinicio+ancho_boton+2;x++) {
            if (x>=0 && x<total_width && y>=0 && y<total_height && !(x<ancho_maquina && y<alto_maquina) ) {
                scr_putpixel(x,y,color_relleno);
            }
        }
    }

}

char **menu_ext_desktop_draw_configurable_icon_return_machine_icon(void)
{
    char buffer_name[255];

    get_machine_config_name_by_number(buffer_name,current_machine_type);

    return get_machine_icon_by_name(buffer_name);

}


/*
char **old_menu_ext_desktop_draw_configurable_icon_return_machine_icon(void)
{

    char **bitmap;

    //Por defecto
    bitmap=bitmap_button_ext_desktop_my_machine_generic;

    //Para un gomas
    if (MACHINE_IS_SPECTRUM_16 || MACHINE_IS_SPECTRUM_48) {
        bitmap=bitmap_button_ext_desktop_my_machine_gomas;
    }
    else if (MACHINE_IS_ZX81) {
        bitmap=bitmap_button_ext_desktop_my_machine_zx81;
    }
    else if (MACHINE_IS_TIMEX_TS1000) {
        bitmap=bitmap_button_ext_desktop_my_machine_ts1000;
    }
    else if (MACHINE_IS_TIMEX_TS1500) {
        bitmap=bitmap_button_ext_desktop_my_machine_ts1500;
    }
    //TK82C igual al TK82 en apariencia pero sin la C (y teclado diferente tambien)
    else if (MACHINE_IS_MICRODIGITAL_TK82) {
        bitmap=bitmap_button_ext_desktop_my_machine_tk82c;
    }
    else if (MACHINE_IS_MICRODIGITAL_TK82C) {
        bitmap=bitmap_button_ext_desktop_my_machine_tk82c;
    }
    else if (MACHINE_IS_MICRODIGITAL_TK83) {
        bitmap=bitmap_button_ext_desktop_my_machine_tk83;
    }
    else if (MACHINE_IS_MICRODIGITAL_TK85) {
        bitmap=bitmap_button_ext_desktop_my_machine_tk85;
    }
    else if (MACHINE_IS_ZX80) {
        bitmap=bitmap_button_ext_desktop_my_machine_zx80;
    }

    //Aunque el teclado sea el de un zx80, la imagen exterior es casi como un zx81??
    else if (MACHINE_IS_MICRODIGITAL_TK80) {
        bitmap=bitmap_button_ext_desktop_my_machine_tk80;
    }

    //Aunque el teclado sea el de un zx80, la imagen exterior es casi como un zx81??
    else if (MACHINE_IS_MICRODIGITAL_TK82) {
        bitmap=bitmap_button_ext_desktop_my_machine_tk82;
    }


    else if (MACHINE_IS_ACE) {
        bitmap=bitmap_button_ext_desktop_my_machine_ace;
    }

    else if (MACHINE_IS_MK14_STANDARD) {
        bitmap=bitmap_button_ext_desktop_my_machine_mk14;
    }

    else if (MACHINE_IS_QL) {
        bitmap=bitmap_button_ext_desktop_my_machine_ql;
    }

    else if (MACHINE_IS_SPECTRUM_128_SPA) {
        bitmap=bitmap_button_ext_desktop_my_machine_spectrum_128_spa;
    }

    else if (MACHINE_IS_SPECTRUM_128) {
        bitmap=bitmap_button_ext_desktop_my_machine_spectrum_128_eng;
    }

    else if (MACHINE_IS_SPECTRUM_P2) {
        bitmap=bitmap_button_ext_desktop_my_machine_spectrum_p2;
    }

    else if (MACHINE_IS_SPECTRUM_P2A) {
        bitmap=bitmap_button_ext_desktop_my_machine_spectrum_p2a;
    }

    else if (MACHINE_IS_SPECTRUM_P3) {
        bitmap=bitmap_button_ext_desktop_my_machine_spectrum_p3;
    }

    else if (MACHINE_IS_SPECTRUM_48_PLUS_SPA) {
        bitmap=bitmap_button_ext_desktop_my_machine_spectrum_48_spa;
    }

    else if (MACHINE_IS_SPECTRUM_48_PLUS_ENG) {
        bitmap=bitmap_button_ext_desktop_my_machine_spectrum_48_spa;
    }

    else if (MACHINE_IS_PENTAGON) {
        bitmap=bitmap_button_ext_desktop_my_machine_pentagon;
    }

    else if (MACHINE_IS_INVES) {
        bitmap=bitmap_button_ext_desktop_my_machine_inves;
    }

    else if (MACHINE_IS_CPC_464 || MACHINE_IS_CPC_4128) {
        bitmap=bitmap_button_ext_desktop_my_machine_cpc_464;
    }

    else if (MACHINE_IS_CPC_664) {
        bitmap=bitmap_button_ext_desktop_my_machine_cpc_664;
    }

    else if (MACHINE_IS_CPC_6128) {
        bitmap=bitmap_button_ext_desktop_my_machine_cpc_6128;
    }

    else if (MACHINE_IS_PCW) {
        bitmap=bitmap_button_ext_desktop_my_machine_pcw_8256;
    }


    else if (MACHINE_IS_SMS) {
        bitmap=bitmap_button_ext_desktop_my_machine_sms;
    }

    else if (MACHINE_IS_SG1000) {
        bitmap=bitmap_button_ext_desktop_my_machine_sg1000;
    }

    else if (MACHINE_IS_COLECO) {
        bitmap=bitmap_button_ext_desktop_my_machine_coleco;
    }

    else if (MACHINE_IS_MICRODIGITAL_TK90X || MACHINE_IS_MICRODIGITAL_TK90X_SPA) {
        bitmap=bitmap_button_ext_desktop_my_machine_tk90x;
    }

    else if (MACHINE_IS_MICRODIGITAL_TK95 || MACHINE_IS_MICRODIGITAL_TK95_SPA) {
        bitmap=bitmap_button_ext_desktop_my_machine_tk95;
    }


    else if (MACHINE_IS_MSX) {
        bitmap=bitmap_button_ext_desktop_my_machine_msx;
    }

    else if (MACHINE_IS_SVI_318) {
        bitmap=bitmap_button_ext_desktop_my_machine_svi318;
    }

    else if (MACHINE_IS_SVI_328) {
        bitmap=bitmap_button_ext_desktop_my_machine_svi328;
    }

    else if (MACHINE_IS_TIMEX_TS_TC_2068) {
        bitmap=bitmap_button_ext_desktop_my_machine_timex_ts2068;
    }

    else if (MACHINE_IS_TIMEX_TC2048) {
        bitmap=bitmap_button_ext_desktop_my_machine_timex_tc2048;
    }

    else if (MACHINE_IS_Z88) {
        bitmap=bitmap_button_ext_desktop_my_machine_z88;
    }

    else if (MACHINE_IS_ZXUNO) {
        bitmap=bitmap_button_ext_desktop_my_machine_zxuno;
    }

    else if (MACHINE_IS_TBBLUE) {
        bitmap=bitmap_button_ext_desktop_my_machine_spectrum_next;
    }

    else if (MACHINE_IS_TSCONF) {
        bitmap=bitmap_button_ext_desktop_my_machine_tsconf;
    }

    else if (MACHINE_IS_BASECONF) {
        bitmap=bitmap_button_ext_desktop_my_machine_baseconf;
    }

    else if (MACHINE_IS_SAM) {
        bitmap=bitmap_button_ext_desktop_my_machine_sam;
    }

    return bitmap;
}

*/

//Retorna bitmap de una accion
/*
char **get_direct_function_icon_bitmap(int id_accion)
{

    return defined_direct_functions_array[id_accion].bitmap_button;
}
*/

//Retorna bitmap de una accion, considerando tambien iconos dinamicos (papelera, my machine...)
char **get_direct_function_icon_bitmap_final(int id_accion)
{
    char **bitmap=defined_direct_functions_array[id_accion].bitmap_button;
    bitmap=alter_zesarux_ascii_logo(bitmap);

    //Si icono es papelera, decir que cambiamos si la papelera no esta vacia

    enum defined_f_function_ids id_funcion=defined_direct_functions_array[id_accion].id_funcion;

    //Comportamiento icono diferente para trash
    if (id_funcion==F_FUNCION_DESKTOP_TRASH) {
        //Ver si papelera no esta vacia
        if (if_zxdesktop_trash_not_empty()) {
            bitmap=bitmap_button_ext_desktop_trash_not_empty;

            //Y si papelera abierta, porque se esta arrastrando algun icono cerca
            if (zxvision_draw_icon_papelera_abierta) {
              bitmap=bitmap_button_ext_desktop_trash_open_not_empty;
            }
        }

        else {
            //Papelera vacia

            //Y si papelera abierta, porque se esta arrastrando algun icono cerca
            if (zxvision_draw_icon_papelera_abierta) {
                bitmap=bitmap_button_ext_desktop_trash_open;
            }
        }
    }

    //Comportamiento icono diferente para My Machine
    if (id_funcion==F_FUNCION_DESKTOP_MY_MACHINE) {
        bitmap=menu_ext_desktop_draw_configurable_icon_return_machine_icon();
    }

    //logo x anniversary
    //if (id_funcion==F_FUNCION_OPENMENU) {
    //    bitmap=get_zesarux_ascii_logo();
    //}

    return bitmap;
}

void menu_ext_desktop_draw_configurable_icon(int index_icon,int pulsado)
{
    int x,y;
    menu_get_ext_desktop_icons_position(index_icon,&x,&y);

    int id_accion=zxdesktop_configurable_icons_list[index_icon].indice_funcion;

    char **bitmap=get_direct_function_icon_bitmap_final(id_accion);

    if (pulsado || menu_ext_desktop_transparent_configurable_icons.v==0 || menu_pressed_zxdesktop_configurable_icon_which==index_icon) {
        //Aplicar un background si se pulsa o si hay setting de no fondo transparente
        //Color 7 blanco siempre fijo, asumimos que los iconos no usan color blanco que se puedan "fusionar" con este fondo
        int color=7;

        //Pero si se ha hecho boton derecho sobre este icono, alterar color
        if (menu_pressed_zxdesktop_configurable_icon_which==index_icon) color=ESTILO_GUI_PAPEL_SELECCIONADO;

        menu_draw_ext_desktop_one_configurable_icon_background(x,y,menu_get_ext_desktop_icons_size(),menu_get_ext_desktop_icons_size(),color);
    }

    if (pulsado) {
        //desplazado 2 pixel cuando se pulsa
        x+=2;
        y+=2;
    }

    //Dibujar el icono
    //Si icono es OPEN_WINDOW, adoptar icono de la ventana que se va a abrir
    if (defined_direct_functions_array[id_accion].id_funcion==F_FUNCION_OPEN_WINDOW) {
        char *geometry_name;
        geometry_name=zxdesktop_configurable_icons_list[index_icon].extra_info;
        char **possible_bitmap=zxvision_find_icon_for_known_window(geometry_name);
        if (possible_bitmap!=NULL) bitmap=possible_bitmap;
    }

    //Si icono es F_FUNCION_SET_MACHINE y tiene parametro de set machine, dibujamos el icono de la maquina y luego la "flechita"
    //De tal manera que estamos dibujando un icono sobre el otro. Este es el unico caso de momento que hago eso
    if (defined_direct_functions_array[id_accion].id_funcion==F_FUNCION_SET_MACHINE) {


        char *machine_name;
        machine_name=zxdesktop_configurable_icons_list[index_icon].extra_info;

        if (machine_name[0]) {

            //Obtener bitmap en base al parametro
            bitmap=get_machine_icon_by_name(machine_name);

            menu_draw_ext_desktop_one_icon(x,y,bitmap);

            //Y continuamos hacia abajo diciendo que dibuje la flecha
            bitmap=bitmap_button_ext_desktop_set_machine_only_arrow;
        }

    }


    menu_draw_ext_desktop_one_icon(x,y,bitmap);


    //Si se indica que la app está abierta

    if (zxdesktop_icon_show_app_open.v) {
        char *geometry_name;

        //Caso especial icono que dice de abrir ventana generica
        if (defined_direct_functions_array[id_accion].id_funcion==F_FUNCION_OPEN_WINDOW) {
            geometry_name=zxdesktop_configurable_icons_list[index_icon].extra_info;
        }

        else {
            geometry_name=defined_direct_functions_array[id_accion].geometry_name;
        }

        if (geometry_name[0]) {
            zxvision_window *buscar_ventana_abierta;
            buscar_ventana_abierta=zxvision_find_window_in_background(geometry_name);


            //buscar si esta abierta
            if (buscar_ventana_abierta!=NULL) {
                int y_app_abierta=y+menu_get_ext_desktop_icons_size();

                //Una linea por debajo
                //TODO: la linea se sale dos pixeles a la derecha, porque asi esta definido para dibujar el background de un icono,
                //pero en la linea quiza no nos interesa. Eso no se aprecia en ninguna app excepto la de shortcutshelper, en que
                //el icono llega a la derecha del todo y la linea de apertura se va mas 2 pixeles mas a la derecha
                menu_draw_ext_desktop_one_configurable_icon_background(x,y_app_abierta,menu_get_ext_desktop_icons_size(),
                    1*zoom_y,ESTILO_GUI_COLOR_AVISO);

                //Si esta minimizada, mismo puntito que sale en el process switcher con ventanas minimizadas
                if (buscar_ventana_abierta->is_minimized) {
                    menu_draw_ext_desktop_one_configurable_icon_background(x+menu_get_ext_desktop_icons_size()-3*zoom_x,y_app_abierta-3*zoom_y,
                        zoom_x*2,zoom_y*2,ESTILO_GUI_TINTA_NO_DISPONIBLE);
                }
            }
        }
    }



    //Escribir texto del icono
    int separacion_texto=2*menu_get_ext_desktop_icons_zoom();
    int y_texto_icono=y+menu_get_ext_desktop_icons_size()+separacion_texto;

    char texto_mostrado[MAX_LENGTH_TEXT_ICON];
    strcpy(texto_mostrado,zxdesktop_configurable_icons_list[index_icon].text_icon);


    if (menu_ext_desktop_configurable_icons_short_text.v) {
        //Si excede, acortar
        int l=strlen(texto_mostrado);

        if (l>MAX_LENGTH_TEXT_SHORT_ICON) {
            texto_mostrado[MAX_LENGTH_TEXT_SHORT_ICON]=0;
            texto_mostrado[MAX_LENGTH_TEXT_SHORT_ICON-1]='.';
            //texto_mostrado[MAX_LENGTH_TEXT_SHORT_ICON-2]='.';
            //texto_mostrado[MAX_LENGTH_TEXT_SHORT_ICON-3]='.';
        }
    }

    //Si background para el texto
    if (menu_ext_desktop_configurable_icons_text_background.v) {
        int longitud_texto=strlen(texto_mostrado);

        //int zoom_iconos=menu_get_ext_desktop_icons_zoom();

        menu_draw_ext_desktop_one_configurable_icon_background(x,y_texto_icono,(CHARSET_ICONS_ANCHO+1)*longitud_texto*zoom_x*menu_gui_zoom,
            CHARSET_ICONS_ALTO*zoom_y*menu_gui_zoom,ESTILO_GUI_PAPEL_NORMAL);
    }


    menu_draw_ext_desktop_one_icon_text(x,y_texto_icono,texto_mostrado);
}

//Dibujar los iconos configurables por el usuario
void menu_draw_ext_desktop_configurable_icons(void)
{

    if (!zxdesktop_configurable_icons_enabled_and_visible()) return;

    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_EXISTS) {
            //printf("Dibujando icono %d\n",i);
            menu_ext_desktop_draw_configurable_icon(i,0);
        }
    }
}



//Dice si posicion x,y esta dentro del icono
int if_position_in_desktop_icons(int posicion_x,int posicion_y)
{
    int tamanyo=menu_get_ext_desktop_icons_size();

    //printf("tamanyo icono: %d\n",tamanyo);

    int i;
    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_EXISTS) {
            int x,y;
            menu_get_ext_desktop_icons_position(i,&x,&y);

            if (posicion_x>=x && posicion_x<x+tamanyo &&
                posicion_y>=y && posicion_y<y+tamanyo) return i;
        }
    }

    return -1;
}


void menu_draw_ext_desktop_dibujar_boton_or_lower_icon_pulsado(void)
{
    if (menu_pressed_zxdesktop_button_which>=0) menu_draw_ext_desktop_dibujar_boton_pulsado(menu_pressed_zxdesktop_button_which);
    if (menu_pressed_zxdesktop_lower_icon_which>=0) menu_ext_desktop_draw_lower_icon(menu_pressed_zxdesktop_lower_icon_which,1);
    if (menu_pressed_zxdesktop_configurable_icon_which>=0) {
        menu_ext_desktop_draw_configurable_icon(menu_pressed_zxdesktop_configurable_icon_which,1);
    }
    if (menu_pressed_zxdesktop_right_button_background>=0) {

    }
}

//Retorna posicion del logo de ZEsarUX en el extended desktop
/*
void menu_ext_desktop_get_logo_coords(int *x,int *y)
{

    int xinicio=screen_get_ext_desktop_start_x();
    int ancho=screen_get_ext_desktop_width_zoom();
    int alto=screen_get_ext_desktop_height_zoom();

    //Agregamos logo ZEsarUX en esquina inferior derecha, con margen
    int xfinal=xinicio+ancho-ZESARUX_ASCII_LOGO_ANCHO-ZESARUX_WATERMARK_LOGO_MARGIN;
    int yfinal=alto-ZESARUX_ASCII_LOGO_ALTO-ZESARUX_WATERMARK_LOGO_MARGIN;


    *x=xfinal;
    *y=yfinal;
}
*/


//Tipo de rellenado de extended desktop:
//0=color solido
//1=barras diagonales de colores
//2=barras diagonales de colores que se mueven
//3=punteado blanco/negro
//4=ajedrez
//5=Grid
//6=Random
//7=Degraded
int menu_ext_desktop_fill=0;
int menu_ext_desktop_fill_first_color=5;
int menu_ext_desktop_fill_second_color=13;

z80_bit menu_ext_desktop_degraded_inverted={0};


z80_bit menu_ext_desktop_transparent_upper_icons={0};
z80_bit menu_ext_desktop_transparent_lower_icons={0};

z80_bit menu_ext_desktop_disable_box_upper_icons={0};
z80_bit menu_ext_desktop_disable_box_lower_icons={0};

z80_bit menu_ext_desktop_transparent_configurable_icons={1};

//Si se escribe el texto de los iconos con texto de fondo
z80_bit menu_ext_desktop_configurable_icons_text_background={1};

//Si mostrar el texto de los iconos mas corto para que no exceda en ancho
z80_bit menu_ext_desktop_configurable_icons_short_text={0};

int menu_ext_desktop_fill_rainbow_counter;


void menu_draw_ext_desktop_footer(void)
{
    if (!menu_footer) return;


    debug_printf(VERBOSE_DEBUG,"Drawing zxdesktop footer");

    // De momento solo dibujarlo en color de fondo y ya
    int x,y;
    int xinicio=screen_get_ext_desktop_start_x();
    int yinicio=screen_get_emulated_display_height_zoom_border_en()+screen_get_ext_desktop_height_zoom();

    int ancho=screen_get_ext_desktop_width_zoom();
    int alto=WINDOW_FOOTER_SIZE*zoom_y;

        int color=WINDOW_FOOTER_PAPER;

        for (y=yinicio;y<yinicio+alto;y++) {
            for (x=xinicio;x<xinicio+ancho;x++) {
                scr_putpixel(x,y,color);

            }
        }






}

/*
void old_menu_draw_ext_desktop(void)
{

    //Si no escritorio extendido, salir
    if (!screen_ext_desktop_enabled || !scr_driver_can_ext_desktop() ) return;


        //Los putpixel que hacemos aqui son sin zoom. Se podrian hacer con zoom, pero habria que
        //usar scr_putpixel_zoom_rainbow y scr_putpixel_zoom dependiendo del caso, y sumar margenes en el caso de rainbow,
        //pero no vale la pena, con una sola funcion scr_putpixel vale para todos los casos
        //Con zoom se habria hecho asi:
        //
        //	int margenx_izq;
        //	int margeny_arr;
        //	scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);
        //	if (rainbow_enabled.v==1) scr_putpixel_zoom_rainbow(x+margenx_izq,y+margenx_der,color);
        //	else scr_putpixel_zoom(x,y,color);
        //
        //	Y considerando el espacio de coordenadas x e y con zoom


        int xinicio=screen_get_ext_desktop_start_x();
        int yinicio=0;

        int ancho=screen_get_ext_desktop_width_zoom();
        int alto=screen_get_emulated_display_height_zoom_border_en();

        int x,y;



        //Color solido
        if (menu_ext_desktop_fill==0) {

            int color=menu_ext_desktop_fill_first_color;

            for (y=yinicio;y<yinicio+alto;y++) {
                for (x=xinicio;x<xinicio+ancho;x++) {
                    scr_putpixel(x,y,color);
                }
            }

        }

        //Rayas diagonales de colores, fijas o movibles
        if (menu_ext_desktop_fill==1 || menu_ext_desktop_fill==2) {

            int grueso_lineas=8*zoom_x*menu_gui_zoom; //Para que coincida el color con rainbow de titulo de ventanas
             int total_colores=5;

            int contador_color;

            //En el caso de barras fijas, offset es 0
            if (menu_ext_desktop_fill==1) menu_ext_desktop_fill_rainbow_counter=0;

            for (y=yinicio;y<yinicio+alto;y++) {
                contador_color=y; //Para dar un aspecto de rayado

                for (x=xinicio;x<xinicio+ancho;x++) {
                    int indice_color=((contador_color/grueso_lineas)+menu_ext_desktop_fill_rainbow_counter) % total_colores;
                    int color=screen_colores_rainbow_nobrillo[indice_color];
                    scr_putpixel(x,y,color);

                    contador_color++;
                }
            }

            menu_ext_desktop_fill_rainbow_counter++;

        }

        //punteado
        if (menu_ext_desktop_fill==3) {

            int color;

            for (y=yinicio;y<yinicio+alto;y++) {
                for (x=xinicio;x<xinicio+ancho;x++) {

                    int suma=x+y;
                    color=(suma & 1 ? menu_ext_desktop_fill_first_color : menu_ext_desktop_fill_second_color);

                    scr_putpixel(x,y,color);
                }
            }

        }

        //ajedrez
        if (menu_ext_desktop_fill==4) {

            int color;

            for (y=yinicio;y<yinicio+alto;y++) {
                for (x=xinicio;x<xinicio+ancho;x++) {

                    //Tamaño de 32x32 cada cuadrado
                    int suma=(x/32)+(y/32);

                    //Blanco 7 para que no sea tan brillante
                    color=(suma & 1 ? menu_ext_desktop_fill_first_color : menu_ext_desktop_fill_second_color);

                    scr_putpixel(x,y,color);
                }
            }

        }

        //grid
        if (menu_ext_desktop_fill==5) {

            int color;

            for (y=yinicio;y<yinicio+alto;y++) {
                for (x=xinicio;x<xinicio+ancho;x++) {

                    //Tamaño de 32x32 cada cuadrado
                    int suma=(x/32)*(y/32);

                    //Blanco 7 para que no sea tan brillante
                    color=(suma & 1 ? menu_ext_desktop_fill_first_color : menu_ext_desktop_fill_second_color);

                    scr_putpixel(x,y,color);
                }
            }

        }

        //Random
        if (menu_ext_desktop_fill==6) {

            for (y=yinicio;y<yinicio+alto;y++) {


                for (x=xinicio;x<xinicio+ancho;x++) {

                    ay_randomize(0);

                    //randomize_noise es valor de 16 bits. sacar uno de 8 bits
                    int color=value_16_to_8h(randomize_noise[0]) % EMULATOR_TOTAL_PALETTE_COLOURS;

                    scr_putpixel(x,y,color);

                }
            }

        }

        //Degraded
        if (menu_ext_desktop_fill==7) {

            for (y=yinicio;y<yinicio+alto;y++) {


                //usamos esa paleta de colores de 5 bits por componente, por tanto, 32 colores maximo por componente
                int offset_y=y-yinicio;
                //dividir alto total en 32 segmentos
                int divisor=alto/32;

                int posicion_color=offset_y/divisor; //ahi tenemos un valor entre 0 y 31


                //multiplicar por el color deseado. pillar color primario del setting menu_ext_desktop_fill_first_color
                //de la lista de 8 colores del spectrum (paleta grb), ignorando brillos

                int color_basico=menu_ext_desktop_fill_first_color & 7;

                //en el caso particular del color 0 negro, hacemos que se comporte como 7 blanco... si no, que degradado habria de negro??
                if (color_basico==0) color_basico=7;

                int componente_r=(color_basico>>1)&1;
                int componente_g=(color_basico>>2)&1;
                int componente_b=color_basico&1;

                componente_r *=posicion_color;
                componente_g *=posicion_color;
                componente_b *=posicion_color;

                //cada componente maximo 5 bits (valor 31). sucede que con el calculo anterior, puede llegar a ser mayor que 31,
                //por ejemplo con maquina jupiter ace (debido al tamaño de ventana de dicha máquina)
                if (componente_r>31) componente_r=31;
                if (componente_g>31) componente_g=31;
                if (componente_b>31) componente_b=31;

                //poner cada componente en su posicion final
                int color_tsconf=(componente_b)|(componente_g<<5)|(componente_r<<10);

                int color=TSCONF_INDEX_FIRST_COLOR+color_tsconf;

                for (x=xinicio;x<xinicio+ancho;x++) {

                    scr_putpixel(x,y,color);

                }
            }

        }


    //Agregamos logo ZEsarUX en esquina inferior derecha, con margen, solo si menu esta abierto

    //if (menu_abierto) {
    //	int xfinal;
    //	int yfinal;
    //	//xfinal=xinicio+ancho-ZESARUX_ASCII_LOGO_ANCHO-ZESARUX_WATERMARK_LOGO_MARGIN;
    //	//yfinal=alto-ZESARUX_ASCII_LOGO_ALTO-ZESARUX_WATERMARK_LOGO_MARGIN;
    //
    //	menu_ext_desktop_get_logo_coords(&xfinal,&yfinal);
    //
    //	//El ancho y el puntero dan igual, no los vamos a usar
    //	screen_put_watermark_generic(NULL,xfinal,yfinal,0, menu_draw_ext_desktop_putpixel_bitmap);
    //}


    //Dibujar botones si están activados (por defecto)
    if (menu_zxdesktop_buttons_enabled.v) {
        menu_draw_ext_desktop_buttons(xinicio);
    }

    //Dibujar footer del zxdesktop
    menu_draw_ext_desktop_footer();

}

*/

//si habilitado fondo scr
int zxdesktop_draw_scrfile_enabled=0;

//nombre del archivo SCR
char zxdesktop_draw_scrfile_name[PATH_MAX]="";

//si centrado
int zxdesktop_draw_scrfile_centered=0;
//si escala el maximo que quepa
int zxdesktop_draw_scrfile_fill_scale=0;

//factor de escalado si no esta zxdesktop_draw_scrfile_fill_scale activado
int zxdesktop_draw_scrfile_scale_factor=2;

//Si desactivar flash en scr file
int zxdesktop_draw_scrfile_disable_flash=0;

//Si mezclar scr file con el fondo
int zxdesktop_draw_scrfile_mix_background=0;

z80_byte *zxdesktop_draw_scrfile_pointer=NULL;

//cache para el caso de parpadeo normal
z80_byte *zxdesktop_cache_scrfile=NULL;

//cache para el caso de parpadeo invertido
z80_byte *zxdesktop_cache_scrfile_invertflash=NULL;


//cargar archivo scr en pantalla si es que esta habilitado el scrfile en fondo
void zxdesktop_draw_scrfile_load(void)
{

    if (!if_zxdesktop_enabled_and_driver_allows() ) return;

    if (!zxdesktop_draw_scrfile_enabled) return;

    debug_printf(VERBOSE_DEBUG,"Loading ZX Desktop background SCR file %s",zxdesktop_draw_scrfile_name);

    //Si se extrae de un tap, snapshot o similar, indicar aqui siempre que scr archivo es realmente
    char final_scrfile_name[PATH_MAX]="";

    if (!util_compare_file_extension(zxdesktop_draw_scrfile_name,"scr")) {
        strcpy(final_scrfile_name,zxdesktop_draw_scrfile_name);
    }
    else {

        //Extraer scr de esa cinta, snapshot, etc

        char tmpdir[PATH_MAX];
        sprintf (tmpdir,"%s/zxdesktop_scr",get_tmpdir_base());
        menu_filesel_mkdir(tmpdir);


        char buf_file_no_dir[PATH_MAX];
        util_get_file_no_directory(zxdesktop_draw_scrfile_name,buf_file_no_dir);

        char tempscr[PATH_MAX];
        sprintf(tempscr,"%s/%s.scr",tmpdir,buf_file_no_dir);

        int retorno=util_convert_any_to_scr(zxdesktop_draw_scrfile_name,tempscr);

        if (!retorno) {

            strcpy(final_scrfile_name,tempscr);
            //printf("Leyendo archivo scr %s\n",final_scrfile_name);
        }

        else {
            //TODO: no hay miniatura, que hacer??
            debug_printf(VERBOSE_ERR,"File has no SCR screen");
        }

    }


    //asignar memoria si conviene
    if (zxdesktop_draw_scrfile_pointer==NULL) {
        zxdesktop_draw_scrfile_pointer=malloc(6912);
        if (zxdesktop_draw_scrfile_pointer==NULL) cpu_panic("Can not allocate memory for scrfile on zxdesktop");

        //Inicializar este espacio con 0, por si el archivo no existe, que aparezca en negro
        int i;
        for (i=0;i<6912;i++) {
            zxdesktop_draw_scrfile_pointer[i]=0;
        }
    }

    //Y generamos la cache de x,y a colores para no tener que recalcular cada pixel cada vez
    //Importante hacer esto antes que el return que viene despues, por si el archivo no existe,
    //se intentará igualmente renderizar desde esta cache de scr vacía
    if (zxdesktop_cache_scrfile==NULL) {
        //Posiciones x,y retorna color
        int total_cache=256*192*sizeof(int);

        zxdesktop_cache_scrfile=malloc(total_cache);
        zxdesktop_cache_scrfile_invertflash=malloc(total_cache);
        if (zxdesktop_cache_scrfile==NULL || zxdesktop_cache_scrfile_invertflash==NULL) cpu_panic("Can not allocate cache memory for scr file");
    }

    if (!si_existe_archivo(final_scrfile_name)) {
        debug_printf(VERBOSE_ERR,"Can not load ZX Desktop background SCR file %s",zxdesktop_draw_scrfile_name);
        //Aunque el archivo no exista, igualmente leer a cache (aun con datos random de zxdesktop_draw_scrfile_pointer),
        //asi la cache estará inicializada con datos de colores dentro de rango
        //Si no, correriamos el riesgo de tener en cache valores de colores fuera de rango y generar un segfault
    }

    else {
        lee_archivo(final_scrfile_name,(char *)zxdesktop_draw_scrfile_pointer,6912);
    }


    int i;
    for (i=0;i<768;i++) {
        if (zxdesktop_draw_scrfile_disable_flash) zxdesktop_draw_scrfile_pointer[6144+i] &=(255-128);
    }

    int x,y;

    int offset_cache=0;

    //Conservar estado parpadeo
    int antes_estado_parpadeo=estado_parpadeo.v;

    for (y=0;y<192;y++) {
        for (x=0;x<256;x++) {
            int color_final;

            //Guardar en cache, estado parpadeo normal
            estado_parpadeo.v=0;
            color_final=util_get_pixel_color_scr(zxdesktop_draw_scrfile_pointer,x,y);
            zxdesktop_cache_scrfile[offset_cache]=color_final;

            //Guardar en cache pero parpadeo invertido
            estado_parpadeo.v=1;
            color_final=util_get_pixel_color_scr(zxdesktop_draw_scrfile_pointer,x,y);
            zxdesktop_cache_scrfile_invertflash[offset_cache]=color_final;

            offset_cache++;
        }
    }

    //Restaurar estado parpadeo
    estado_parpadeo.v=antes_estado_parpadeo;

}




//si conviene dibujar el background de un scr file
//retorna -1 si no
//o el color si hay que mostrarlo
int menu_draw_ext_desktop_si_scrfile(int x,int y,int ancho,int alto)
{
    if (!zxdesktop_draw_scrfile_enabled) return -1;

    int scale_x=1;
    int scale_y=1;


    if (zxdesktop_draw_scrfile_fill_scale) {
        scale_x=ancho/256;
        scale_y=alto/192;
    }

    else {
        scale_x=scale_y=zxdesktop_draw_scrfile_scale_factor;
    }

    //Y evitar escala 0 cuando zx desktop es menos ancho que 256
    if (scale_x==0) scale_x=1;
    if (scale_y==0) scale_y=1;

    //quedarnos con la escala mas pequeña
    if (scale_x>scale_y) scale_x=scale_y;
    else scale_y=scale_x;


    int total_size_x=256*scale_x;
    int total_size_y=192*scale_y;

    int margen_min_x=0;
    int margen_max_x=total_size_x-1;

    int margen_min_y=0;
    int margen_max_y=total_size_y-1;

    if (zxdesktop_draw_scrfile_centered) {
        margen_min_x=(ancho-total_size_x)/2;
        margen_min_y=(alto-total_size_y)/2;
        margen_max_x=margen_min_x+total_size_x-1;
        margen_max_y=margen_min_y+total_size_y-1;
    }

    if (x<margen_min_x || x>margen_max_x || y<margen_min_y || y>margen_max_y) return -1;

    //Por si acaso
    if (zxdesktop_draw_scrfile_pointer==NULL) return -1;

    //return util_get_pixel_color_scr(zxdesktop_draw_scrfile_pointer,(x-margen_min_x)/scale_x,(y-margen_min_y)/scale_y);

    int xfinal=(x-margen_min_x)/scale_x;
    int yfinal=(y-margen_min_y)/scale_y;

    //Por si acaso
    if (xfinal<0 || xfinal>255 || yfinal<0 || yfinal>191) return 0;

    //Retornamos el color de pixel de lo que tenemos en cache
    int offset_cache=(yfinal*256)+xfinal;

    if (estado_parpadeo.v) {
        //printf("Retornando con parpadeo %d,%d\n",x,y);
        return zxdesktop_cache_scrfile_invertflash[offset_cache];
    }
    else {
        return zxdesktop_cache_scrfile[offset_cache];
    }

}



void menu_draw_ext_desktop_border_emulated_machine(int ancho_no_zxdesktop,int alto_zxdesktop,int alto_no_zxdesktop)
{

    int x,y;

    //Recuadro que envuelve maquina emulada. Solo si hay zxdesktop vertical y tiene minimo de 16
    if (!zxdesktop_disable_show_frame_around_display && alto_zxdesktop>=ZXDESKTOP_MINIMUM_HEIGHT_SHOW_FRAME) {
        int grueso_recuadro=4;

        int color=ESTILO_GUI_PAPEL_TITULO;

        if (zxvision_key_not_sent_emulated_mach() ) color=ESTILO_GUI_PAPEL_TITULO_INACTIVA;

        //linea vertical de abajo
        for (y=alto_no_zxdesktop;y<alto_no_zxdesktop+grueso_recuadro;y++) {

            //Se le suma a x el grueso, para que coincida con linea vertical
            for (x=0;x<ancho_no_zxdesktop+grueso_recuadro;x++) {
                scr_putpixel(x,y,color);
            }
        }

        //linea vertical de derecha
        for (x=ancho_no_zxdesktop;x<ancho_no_zxdesktop+grueso_recuadro;x++) {
            for (y=0;y<alto_no_zxdesktop;y++) {
                scr_putpixel(x,y,color);
            }
        }

    }
}

void menu_draw_ext_desktop_background(int xstart_zxdesktop)
{


    //Los putpixel que hacemos aqui son sin zoom. Se podrian hacer con zoom, pero habria que
    //usar scr_putpixel_zoom_rainbow y scr_putpixel_zoom dependiendo del caso, y sumar margenes en el caso de rainbow,
    //pero no vale la pena, con una sola funcion scr_putpixel vale para todos los casos
    //Con zoom se habria hecho asi:
    /*
        int margenx_izq;
        int margeny_arr;
        scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);
        if (rainbow_enabled.v==1) scr_putpixel_zoom_rainbow(x+margenx_izq,y+margenx_der,color);
        else scr_putpixel_zoom(x,y,color);

        Y considerando el espacio de coordenadas x e y con zoom
    */


    //int xinicio=0;
    int yinicio=0;

    //int ancho=screen_get_emulated_display_width_zoom_border_en()+screen_get_ext_desktop_width_zoom();

    int ancho_zxdesktop=screen_get_ext_desktop_width_zoom();
    int ancho_no_zxdesktop=screen_get_emulated_display_width_zoom_border_en();
    int xfinal=ancho_no_zxdesktop+ancho_zxdesktop;
    //int alto=screen_get_ext_desktop_height_zoom(); //screen_get_emulated_display_height_zoom_border_en();

    int alto_zxdesktop=screen_get_ext_desktop_height_zoom();
    int alto_no_zxdesktop=screen_get_emulated_display_height_zoom_border_en();

    int alto=alto_no_zxdesktop+alto_zxdesktop;

    int x,y;

    int color;

    int grueso_lineas_rainbow=8*zoom_x*menu_gui_zoom; //Para que coincida el color con rainbow de titulo de ventanas
    int total_colores_rainbow=5;

    int contador_color_rainbow;

    int indice_color;

    int suma;

    //En el caso de barras fijas, offset es 0
    if (menu_ext_desktop_fill==1) menu_ext_desktop_fill_rainbow_counter=0;

    for (y=yinicio;y<yinicio+alto;y++) {

        //Este bloque solo para degraded, se calcula en cada posicion y
        if (menu_ext_desktop_fill==7) {

            //usamos esa paleta de colores de 5 bits por componente, por tanto, 32 colores maximo por componente
            int offset_y=y-yinicio;

            if (menu_ext_desktop_degraded_inverted.v) offset_y=alto-offset_y-1;

            //dividir alto total en 32 segmentos
            int divisor=alto/32;

            int posicion_color=offset_y/divisor; //ahi tenemos un valor entre 0 y 31


            //multiplicar por el color deseado. pillar color primario del setting menu_ext_desktop_fill_first_color
            //de la lista de 8 colores del spectrum (paleta grb), ignorando brillos

            int color_basico=menu_ext_desktop_fill_first_color & 7;

            //en el caso particular del color 0 negro, hacemos que se comporte como 7 blanco... si no, que degradado habria de negro??
            if (color_basico==0) color_basico=7;

            int componente_r=(color_basico>>1)&1;
            int componente_g=(color_basico>>2)&1;
            int componente_b=color_basico&1;

            componente_r *=posicion_color;
            componente_g *=posicion_color;
            componente_b *=posicion_color;

            //cada componente maximo 5 bits (valor 31). sucede que con el calculo anterior, puede llegar a ser mayor que 31,
            //por ejemplo con maquina jupiter ace (debido al tamaño de ventana de dicha máquina)
            if (componente_r>31) componente_r=31;
            if (componente_g>31) componente_g=31;
            if (componente_b>31) componente_b=31;

            //poner cada componente en su posicion final
            int color_tsconf=(componente_b)|(componente_g<<5)|(componente_r<<10);

            color=TSCONF_INDEX_FIRST_COLOR+color_tsconf;

        }

        contador_color_rainbow=y; //Para dar un aspecto de rayado en tipos rainbow

        //Si estamos en zona por arriba de donde empieza el zxdesktop vertical, "saltar" toda esa zona, para que
        //el contador parezca que ha recorrido esa zona. Esto da continuidad en las franjas en cuanto vamos a la zona de zxdesktop vertical

        if (y<alto_no_zxdesktop) {
            contador_color_rainbow +=ancho_no_zxdesktop;

            //Y el modo random igual, esa zona de la izquierda generamos color random tantas veces como ancho tenga
            if (menu_ext_desktop_fill==6) {
                int j;

                for (j=0;j<ancho_no_zxdesktop;j++) ay_randomize(0);
            }
        }

        /*

        Ventana:


        Pantalla emulada
        ||||||||||||||||
        vvvvvvvvvvvvvvvv
        xxxxxxxxxxxxxxxxxxxxxxxx
        xxxxxxxxxxxxxxxxxxxxxxxx    <-- ZX Desktop horizontal ->
        xxxxxxxxxxxxxxxxxxxxxxxx
        xxxxxxxxxxxxxxxxxxxxxxxx

        <-- ZX Desktop vertical --> <-- ZX Desktop horizontal ->

        */

        //Si estamos en la zona de arriba (aun no llega a zxdesktop vertical) saltar posicion x para no dibujar encima de la pantalla emulada
        int xinicio;
        if (y<alto_no_zxdesktop) {
            xinicio=xstart_zxdesktop;
        }
        else {
            xinicio=0;
        }

        for (x=xinicio;x<xfinal;x++) {

            //Si mostrar en esa posicion un scrfile
            int xrelative=x-xstart_zxdesktop;
            int yrelative=y-yinicio;

            int mostrar_scrfile=0;

            int color_scrfile;

            if (xrelative>=0) {
                color_scrfile=menu_draw_ext_desktop_si_scrfile(xrelative,yrelative,ancho_zxdesktop,alto);


                //temporal game of life
                //color_scrfile=menu_draw_ext_desktop_si_gamelife(xrelative,yrelative,ancho_zxdesktop,alto);

                if (color_scrfile>=0) {
                    mostrar_scrfile=1;
                }
            }

            if (mostrar_scrfile && zxdesktop_draw_scrfile_mix_background) {
                //aplicar tramado con el fondo
                int desactivado=(x+y)%2;

                if (desactivado) mostrar_scrfile=0;
            }


            if (mostrar_scrfile) {
                scr_putpixel(x,y,color_scrfile);
            }

            else {

                switch (menu_ext_desktop_fill) {
                    //Color solido
                    case 0:
                        scr_putpixel(x,y,menu_ext_desktop_fill_first_color);
                    break;

                    //Rayas diagonales de colores, fijas o movibles
                    case 1:
                    case 2:
                        indice_color=((contador_color_rainbow/grueso_lineas_rainbow)+menu_ext_desktop_fill_rainbow_counter) % total_colores_rainbow;
                        color=screen_colores_rainbow_nobrillo[indice_color];
                        scr_putpixel(x,y,color);


                    break;

                    //Punteado
                    case 3:
                        suma=x+y;
                        color=(suma & 1 ? menu_ext_desktop_fill_first_color : menu_ext_desktop_fill_second_color);

                        scr_putpixel(x,y,color);
                    break;

                    //Ajedrez
                    case 4:
                        //Tamaño de 32x32 cada cuadrado
                        suma=(x/32)+(y/32);

                        //Blanco 7 para que no sea tan brillante
                        color=(suma & 1 ? menu_ext_desktop_fill_first_color : menu_ext_desktop_fill_second_color);

                        scr_putpixel(x,y,color);
                    break;

                    //Grid
                    case 5:
                        //Tamaño de 32x32 cada cuadrado
                        suma=(x/32)*(y/32);

                        //Blanco 7 para que no sea tan brillante
                        color=(suma & 1 ? menu_ext_desktop_fill_first_color : menu_ext_desktop_fill_second_color);

                        scr_putpixel(x,y,color);
                    break;

                    //Random
                    case 6:
                        ay_randomize(0);

                        //randomize_noise es valor de 16 bits. sacar uno de 8 bits
                        color=value_16_to_8h(randomize_noise[0]) % EMULATOR_TOTAL_PALETTE_COLOURS;

                        scr_putpixel(x,y,color);
                    break;

                    //Degraded
                    case 7:
                        scr_putpixel(x,y,color);
                    break;
                }

            }

            //para los tipos 1 y 2
            contador_color_rainbow++;
        }
    }

    menu_ext_desktop_fill_rainbow_counter++;

    menu_draw_ext_desktop_border_emulated_machine(ancho_no_zxdesktop,alto_zxdesktop,alto_no_zxdesktop);


}



void menu_draw_ext_desktop(void)
{

    //Si no escritorio extendido, salir
    if (!screen_ext_desktop_enabled || !scr_driver_can_ext_desktop() ) return;

    //esto peta desde end_emulator si se sale con ctrl+c con menu principal abierto
    //Es porque el thread thread_main_loop se ha terminado
    //if (ending_emulator_flag) return;

    int xstart_zxdesktop=screen_get_ext_desktop_start_x();

    int dibujar_zxdesktop_background=1;

    //if (frameskip_draw_zxdesktop_background.v && no_next_frameskip_draw_zxdesktop_background.v) {
    //	printf("No se hace frameskip del siguiente refresco de zx desktop\n");
    //}

    //Si frameskip al zx desktop, pero a no ser que se haya hecho un destroy window y por tanto no hay frameskip
    if (frameskip_draw_zxdesktop_background.v && no_next_frameskip_draw_zxdesktop_background.v==0) {
        if (!screen_if_refresh()) dibujar_zxdesktop_background=0;
    }

    no_next_frameskip_draw_zxdesktop_background.v=0;

    //No hago frameskip de ZX Desktop si la emulacion esta detenida en el menu, pues en ese caso
    //no hay "nadie" ejecutando el core de cpu que diga que hay que descartar frames o no
    if (!menu_multitarea || menu_emulation_paused_on_menu) dibujar_zxdesktop_background=1;

    if (dibujar_zxdesktop_background) menu_draw_ext_desktop_background(xstart_zxdesktop);


    //Dibujar botones
    //if (menu_zxdesktop_buttons_enabled.v) {
    menu_draw_ext_desktop_buttons(xstart_zxdesktop);
    //}

    //Dibujar iconos
    menu_draw_ext_desktop_configurable_icons();

    //Dibujar footer del zxdesktop
    menu_draw_ext_desktop_footer();

}


//refresco de pantalla, avisando cambio de border,
void menu_refresca_pantalla(void)
{

    modificado_border.v=1;
    all_interlace_scr_refresca_pantalla();

    //necesario si hay efectos de darken o grayscale
    //menu_clear_footer();

    //y redibujar todo footer
    redraw_footer();

    //Y redibujar zx desktop
    menu_draw_ext_desktop();

}

//Borra la pantalla del menu, refresca la pantalla de spectrum
void menu_cls_refresh_emulated_screen()
{

                cls_menu_overlay();

                menu_refresca_pantalla();

}

void enable_footer(void)
{

        menu_footer=1;

        //forzar redibujar algunos contadores
        draw_bateria_contador=0;

}

void disable_footer(void)
{

        menu_footer=0;



}



//retornar puntero a campo desde texto, separado por espacios. se permiten multiples espacios entre campos
char *menu_get_cpu_use_idle_value(char *m,int campo)
{

    char c;

    while (campo>1) {
        c=*m;

        if (c==' ') {
            while (*m==' ') m++;
            campo--;
        }

        else m++;
    }

    return m;
}


long menu_cpu_use_seconds_antes=0;
long menu_cpu_use_seconds_ahora=0;
long menu_cpu_use_idle_antes=0;
long menu_cpu_use_idle_ahora=0;

int menu_cpu_use_num_cpus=1;

//devuelve valor idle desde /proc/stat de cpu
//devuelve <0 si error

//long temp_idle;

long menu_get_cpu_use_idle(void)
{

    //printf ("llamando a menu_get_cpu_use_idle\n");

//En Mac OS X, obtenemos consumo cpu de este proceso
#if defined(__APPLE__)

    struct rusage r_usage;

    if (getrusage(RUSAGE_SELF, &r_usage)) {
        return -1;
        /* ... error handling ... */
    }

    //printf("Total User CPU = %ld.%d\n",        r_usage.ru_utime.tv_sec,        r_usage.ru_utime.tv_usec);

    long cpu_use_mac=r_usage.ru_utime.tv_sec*100+(r_usage.ru_utime.tv_usec/10000);   //el 10000 sale de /1000000*100

    //printf ("Valor retorno: %ld\n",cpu_use_mac);

    return cpu_use_mac;

#endif

//En Linux en cambio obtenemos uso de cpu de todo el sistema
//cat /proc/stat
//cpu  2383406 37572370 7299316 91227807 7207258 18372 473173 0 0 0
//     user    nice    system   idle

    //int max_leer=DEBUG_MAX_MESSAGE_LENGTH-200;

    #define MAX_LEER (DEBUG_MAX_MESSAGE_LENGTH-200)

    //dado que hacemos un debug_printf con este texto,
    //el maximo del debug_printf es DEBUG_MAX_MESSAGE_LENGTH. Quitamos 200 que da margen para poder escribir sin
    //hacer segmentation fault

    //metemos +1 para poder poner el 0 del final
    char procstat_buffer[MAX_LEER+1];

    //char buffer_nulo[100];
    //char buffer_idle[100];
    char *buffer_idle;
    long cpu_use_idle=0;

    char *archivo_cpuuse="/proc/stat";

    if (si_existe_archivo(archivo_cpuuse) ) {
        int leidos=lee_archivo(archivo_cpuuse,procstat_buffer,MAX_LEER);

            if (leidos<1) {
                debug_printf (VERBOSE_DEBUG,"Error reading cpu status on %s",archivo_cpuuse);
                            return -1;
                    }

            //leidos es >=1

            //temp
            //printf ("leidos: %d DEBUG_MAX_MESSAGE_LENGTH: %d sizeof: %d\n",leidos,DEBUG_MAX_MESSAGE_LENGTH,sizeof(procstat_buffer) );

            //establecemos final de cadena
            procstat_buffer[leidos]=0;

            debug_printf (VERBOSE_PARANOID,"procstat_buffer: %s",procstat_buffer);

            //miramos numero cpus
            menu_cpu_use_num_cpus=0;

            char *p;
            p=procstat_buffer;

            while (p!=NULL) {
                p=strstr(p,"cpu");
                if (p!=NULL) {
                    p++;
                    menu_cpu_use_num_cpus++;
                }
            }

            if (menu_cpu_use_num_cpus==0) {
                //como minimo habra 1
                menu_cpu_use_num_cpus=1;
            }

            else {
                //se encuentra cabecera con "cpu" y luego "cpu0, cpu1", etc, por tanto,restar 1
                menu_cpu_use_num_cpus--;
            }

            debug_printf (VERBOSE_DEBUG,"cpu number: %d",menu_cpu_use_num_cpus);

            //parsear valores, usamos scanf
            //fscanf(ptr_procstat,"%s %s %s %s %s",buffer_nulo,buffer_nulo,buffer_nulo,buffer_nulo,buffer_idle);

            //parsear valores, usamos funcion propia
            buffer_idle=menu_get_cpu_use_idle_value(procstat_buffer,5);



            if (buffer_idle!=NULL) {
                //ponemos 0 al final
                int i=0;
                while (buffer_idle[i]!=' ') {
                    i++;
                }

                buffer_idle[i]=0;


                debug_printf (VERBOSE_DEBUG,"idle value: %s",buffer_idle);

                cpu_use_idle=atoi(buffer_idle);
            }
    }

    else {
        cpu_use_idle=-1;
    }

    return cpu_use_idle;

}

void menu_get_cpu_use_perc(void)
{

    int usocpu=0;

    struct timeval menu_cpu_use_time;

    gettimeofday(&menu_cpu_use_time, NULL);
    menu_cpu_use_seconds_ahora=menu_cpu_use_time.tv_sec;

    menu_cpu_use_idle_ahora=menu_get_cpu_use_idle();

    if (menu_cpu_use_idle_ahora<0) {
        menu_last_cpu_use=-1;
        return;
    }

    if (menu_cpu_use_seconds_antes!=0) {
        long dif_segundos=menu_cpu_use_seconds_ahora-menu_cpu_use_seconds_antes;
        long dif_cpu_idle=menu_cpu_use_idle_ahora-menu_cpu_use_idle_antes;

        debug_printf (VERBOSE_PARANOID,"sec now: %ld before: %ld cpu now: %ld before: %ld",menu_cpu_use_seconds_ahora,menu_cpu_use_seconds_antes,
            menu_cpu_use_idle_ahora,menu_cpu_use_idle_antes);

        long uso_cpu_idle;

        //proteger division por cero
        if (dif_segundos==0) uso_cpu_idle=100;
        else uso_cpu_idle=dif_cpu_idle/dif_segundos/menu_cpu_use_num_cpus;

#if defined(__APPLE__)
        debug_printf (VERBOSE_PARANOID,"cpu use: %ld",uso_cpu_idle);
        usocpu=uso_cpu_idle;
#else
        debug_printf (VERBOSE_PARANOID,"cpu idle: %ld",uso_cpu_idle);
        //pasamos a int
        usocpu=100-uso_cpu_idle;
#endif
    }

    menu_cpu_use_seconds_antes=menu_cpu_use_seconds_ahora;
    menu_cpu_use_idle_antes=menu_cpu_use_idle_ahora;

    menu_last_cpu_use=usocpu;
}

int cpu_use_total_acumulado=0;
int cpu_use_total_acumulado_medidas=0;

int footer_last_cpu_use=0;

void menu_draw_cpu_use_last(void)
{

    int cpu_use=menu_last_cpu_use;
    debug_printf (VERBOSE_PARANOID,"cpu: %d",cpu_use );

    //error
    if (cpu_use<0) return;

    //control de rango
    if (cpu_use>100) cpu_use=100;
    if (cpu_use<0) cpu_use=0;

    //temp
    //cpu_use=100;

    //printf ("mostrando cpu use\n");

    char buffer_perc[9];
    sprintf (buffer_perc,"%3d%% CPU",cpu_use);

    footer_last_cpu_use=cpu_use;

    int x;

    x=WINDOW_FOOTER_ELEMENT_X_CPU_USE;

    int color_tinta=WINDOW_FOOTER_INK;

    //Color en rojo si uso cpu sube
    if (cpu_use>=85) color_tinta=ESTILO_GUI_COLOR_AVISO;

    menu_putstring_footer(x,WINDOW_FOOTER_ELEMENT_Y_CPU_USE,buffer_perc,color_tinta,WINDOW_FOOTER_PAPER);

}


void timer_get_cpu_use(void)
{
    menu_get_cpu_use_perc();

    //Y calcular el acumulado
    int cpu_use=menu_last_cpu_use;

    //error
    if (cpu_use<0) return;

    //control de rango
    if (cpu_use>100) cpu_use=100;
    if (cpu_use<0) cpu_use=0;


    cpu_use_total_acumulado +=cpu_use;
    cpu_use_total_acumulado_medidas++;

}

void menu_draw_cpu_use(void)
{


    if (top_speed_timer.v) {
        debug_printf (VERBOSE_DEBUG,"Refreshing footer cpu topspeed");
        menu_putstring_footer(WINDOW_FOOTER_ELEMENT_X_CPU_USE,WINDOW_FOOTER_ELEMENT_Y_CPU_USE,"TOPSPEED",ESTILO_GUI_COLOR_AVISO,WINDOW_FOOTER_PAPER);
        return;
    }

        //solo redibujarla de vez en cuando
        if (draw_cpu_use!=0) {
                draw_cpu_use--;
                return;
        }

        //cada 5 segundos
        draw_cpu_use=50*5;

    //menu_get_cpu_use_perc();

    /*
    int cpu_use=menu_last_cpu_use;
    debug_printf (VERBOSE_PARANOID,"cpu: %d",cpu_use );

    //error
    if (cpu_use<0) return;

    //control de rango
    if (cpu_use>100) cpu_use=100;
    if (cpu_use<0) cpu_use=0;


    cpu_use_total_acumulado +=cpu_use;
    cpu_use_total_acumulado_medidas++;
    */

    menu_draw_cpu_use_last();

}



//Retorna -1 si hay algun error
int menu_get_cpu_temp(void)
{

    char procstat_buffer[10];

    //sensor generico
    char *posible_archivo_cputemp1="/sys/class/thermal/thermal_zone0/temp";

    //sensor especifico para mi pc linux
    char *posible_archivo_cputemp2="/sys/devices/platform/smsc47b397.1152/hwmon/hwmon0/temp1_input";

    char *archivo_cputemp;

    if (si_existe_archivo(posible_archivo_cputemp1) ) {
        archivo_cputemp=posible_archivo_cputemp1;
    }

    else if (si_existe_archivo(posible_archivo_cputemp2) ) {
        archivo_cputemp=posible_archivo_cputemp2;
    }

    else return -1;


    int leidos=lee_archivo(archivo_cputemp,procstat_buffer,9);

    if (leidos<1) {
        debug_printf (VERBOSE_DEBUG,"Error reading cpu status on %s",archivo_cputemp);
        return -1;
    }

    //establecemos final de cadena
    procstat_buffer[leidos]=0;


    return atoi(procstat_buffer);

}

void menu_draw_cpu_temp(void)
{
        //solo redibujarla de vez en cuando
        if (draw_cpu_temp!=0) {
                draw_cpu_temp--;
                return;
        }

        //cada 5 segundos
        draw_cpu_temp=50*5;

        int cpu_temp=menu_get_cpu_temp();
        debug_printf (VERBOSE_DEBUG,"CPU temp: %d",cpu_temp );

    //algun error al leer temperatura
    if (cpu_temp<0) return;

        //control de rango
        if (cpu_temp>99999) cpu_temp=99999;


        //temp forzar
        //cpu_temp=10000;

        char buffer_temp[6];

        int grados_entero=cpu_temp/1000; //2 cifras
        int grados_decimal=(cpu_temp%1000)/100; //1 cifra

        sprintf (buffer_temp,"%2d.%dC",grados_entero,grados_decimal );

        //primero liberar esas zonas
        int x;

    int color_tinta=WINDOW_FOOTER_INK;

    //Color en rojo si temperatura alta
    if (grados_entero>=80) color_tinta=ESTILO_GUI_COLOR_AVISO;


        //luego escribimos el texto
        x=WINDOW_FOOTER_ELEMENT_X_CPU_TEMP;


    menu_putstring_footer(x,WINDOW_FOOTER_ELEMENT_Y_CPU_TEMP,buffer_temp,color_tinta,WINDOW_FOOTER_PAPER);
}

void menu_draw_last_fps(void)
{


        //int fps=ultimo_fps;
        int fps=sensor_get_value("fps");
        debug_printf (VERBOSE_PARANOID,"FPS: %d",fps);

        //algun error al leer fps
        if (fps<0) return;

        //control de rango
        if (fps>50) fps=50;

    const int ancho_maximo=6;

            //printf ("mostrando fps\n");

        char buffer_fps[ancho_maximo+1];
        sprintf (buffer_fps,"%02d FPS",fps);

        //primero liberar esas zonas
        int x;


        //luego escribimos el texto
        x=WINDOW_FOOTER_ELEMENT_X_FPS;


        int color_tinta=WINDOW_FOOTER_INK;

    //Color en rojo si uso fps bajo sube
    if (fps<10) color_tinta=ESTILO_GUI_COLOR_AVISO;


    menu_putstring_footer(x,WINDOW_FOOTER_ELEMENT_Y_FPS,buffer_fps,color_tinta,WINDOW_FOOTER_PAPER);

}

void menu_draw_fps(void)
{

            //solo redibujarla de vez en cuando
        if (draw_fps!=0) {
                draw_fps--;
                return;
        }

        //printf("draw fps\n");

        //cada 1 segundo
        draw_fps=50*1;

        menu_draw_last_fps();

}



int menu_get_bateria_perc(void)
{
        //temp forzar
        return 25;

}






//Aqui se llama desde cada driver de video al refrescar la pantalla
//Importante que lo que se muestre en footer se haga cada cierto tiempo y no siempre, sino saturaria la cpu probablemente
void draw_middle_footer(void)
{

    if (menu_footer==0) return;

    //temp forzado
    //menu_draw_cpu_temp();

//Temperatura mostrarla en raspberry y en general en Linux
//#ifdef EMULATE_RASPBERRY
#ifdef __linux__
    if (screen_show_cpu_temp.v) {
        menu_draw_cpu_temp();
    }
#endif

    if (screen_show_cpu_usage.v) {
        menu_draw_cpu_use();
    }

    if (screen_show_fps.v) {
        menu_draw_fps();
    }




//01234567890123456789012345678901
//50 FPS 100% CPU 99.9C TEMP

}


//0 si no valido
//1 si valido
int si_valid_char(z80_byte caracter)
{
    if (si_complete_video_driver() ) {
        if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) return 0;
    }

    else {
        if (caracter<32 || caracter>127) return 0;
    }

    return 1;
}

void menu_draw_background_windows_overlay_after_normal(void)
{

    zxvision_window *ventana;
    ventana=zxvision_current_window;
    zxvision_draw_overlays_below_windows(ventana);
    //printf ("overlay funcion desde menu_draw_background_windows_overlay\n");
}


void normal_overlay_texto_menu_final(void)
{

    if (cuadrado_activo && ventana_tipo_activa) {
        int color_marca_redimensionado=cuadrado_color;

        if (cuadrado_activo_marca_redimensionado_aviso) {
            //darle parpadeo a la marca
            if ( (contador_segundo_infinito % 1000) >500) {
                color_marca_redimensionado=ESTILO_GUI_COLOR_AVISO;
            }
        }

        menu_dibuja_cuadrado(cuadrado_x1,cuadrado_y1,cuadrado_x2,cuadrado_y2,cuadrado_color,color_marca_redimensionado);

    }

    //Dibujar ventanas en background pero solo si menu está abierto, esto evita que aparezcan las ventanas cuando hay un
    //mensaje de splash y el menú está cerrado
    if (/*menu_allow_background_windows && */
      (menu_abierto || overlay_visible_when_menu_closed)
    ) {
        //printf("redrawing windows on normal_overlay\n");
        //Conservar estado de tecla pulsada o no para el speech
        int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
        menu_draw_background_windows_overlay_after_normal();
        //if (menu_speech_tecla_pulsada && !antes_menu_speech_tecla_pulsada) {
        //    printf("Pasar de 1 a 0 desde \n");
        //    debug_exec_show_backtrace();
        //}
        menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;
    }

    //Dibujar topbar
    topbar_text_overlay();


}

/*
void old_normal_overlay_texto_menu_final(void)
{

    if (cuadrado_activo && ventana_tipo_activa) {
        menu_dibuja_cuadrado(cuadrado_x1,cuadrado_y1,cuadrado_x2,cuadrado_y2,cuadrado_color);

    }

    //Dibujar ventanas en background pero solo si menu está abierto, esto evita que aparezcan las ventanas cuando hay un
    //mensaje de splash y el menú está cerrado
    if (menu_allow_background_windows &&
      (menu_abierto || overlay_visible_when_menu_closed)
    ) {
        //printf("redrawing windows on normal_overlay\n");
        //Conservar estado de tecla pulsada o no para el speech
        int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
        menu_draw_background_windows_overlay_after_normal();
        menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;
    }


}
*/

#ifdef DEBUG_ZXVISION_USE_CACHE_OVERLAY_TEXT
z80_byte debug_zxvision_cache_overlay_caracter=33;
#endif


//Para estadisticas. Total de caracteres visibles en el overlay
int stats_normal_overlay_menu_total_chars=0;
//Total de caracteres que se han redibujado en normal_overlay_texto_menu, o sea, que no estaban en cache
int stats_normal_overlay_menu_drawn_chars=0;

long normal_overlay_time_total_drawing_overlay=0;

//funcion normal de impresion de overlay de buffer de texto y cuadrado de lineas usado en los menus
//En drivers no graficos, cuando renderizan la maquina emulada, siempre escriben encima de cualquier cosa, aunque haya menu abierto
//luego es cuando se redibuja la capa de menu. Pero claro, si la capa de menu tiene que .modificado es 0, no volvera a escribir el menu
//encima despues de que la maquina emulada haya borrado dicho menu
//A diferencia de drivers graficos, en que la maquina emulada, si hay menu abierto, no escriben encima, sino que hacen mix (over o segun el modo transparencia)
//con el menu
//Es por esto que en drivers no graficos, NO hacemos caso de la cache putchar
void normal_overlay_texto_menu(void)
{

    //Calcular tiempo total haciendo este normal overlay
    struct timeval zxvision_time_total_antes,zxvision_time_total_despues;

    timer_stats_current_time(&zxvision_time_total_antes);

    stats_normal_overlay_menu_total_chars=0;
    stats_normal_overlay_menu_drawn_chars=0;

    //printf ("inicio normal_overlay_texto_menu\n");
#ifdef DEBUG_ZXVISION_USE_CACHE_OVERLAY_TEXT
    debug_zxvision_cache_overlay_caracter++;
    if (debug_zxvision_cache_overlay_caracter>126) debug_zxvision_cache_overlay_caracter=33;
#endif

    int x,y;
    int tinta,papel,parpadeo;

    z80_byte caracter;
    int pos_array=0;

    int nocache=0;
    if (!si_complete_video_driver() ) nocache=1;


    //printf ("normal_overlay_texto_menu\n");
    for (y=0;y<scr_get_menu_height();y++) {
        for (x=0;x<scr_get_menu_width();x++,pos_array++) {
            caracter=overlay_screen_array[pos_array].caracter;

            //sacamos el papel antes para poder alterarlo cuando se habilita DEBUG_ZXVISION_USE_CACHE_OVERLAY_TEXT
            papel=overlay_screen_array[pos_array].papel;
            //si caracter es 0, no mostrar

            if (caracter) {
                stats_normal_overlay_menu_total_chars++;

                if (overlay_screen_array[pos_array].modificado || nocache || overlay_screen_array[pos_array].parpadeo) {
                    //Siempre que sea caracter!=0, y si se ha modificado la  cache
                    //caracter con parpadeo se redibuja siempre

                    stats_normal_overlay_menu_drawn_chars++;

                    //Indicar que el caracter ya se ha dibujado en pantalla, para que en el siguiente refresco se muestre, si conviene
                    overlay_screen_array[pos_array].modificado=0;

    #ifdef DEBUG_ZXVISION_USE_CACHE_OVERLAY_TEXT
                    //Para que de alguna manera se vea facilmente las zonas que no estan cacheandose
                    papel += (debug_zxvision_cache_overlay_caracter&7);
    #endif



                    //128 y 129 corresponden a franja de menu y a letra enye minuscula
                    if (si_valid_char(caracter) ) {
                        tinta=overlay_screen_array[pos_array].tinta;
                        parpadeo=overlay_screen_array[pos_array].parpadeo;

                        //Si esta multitask, si es caracter con parpadeo y si el estado del contador del parpadeo indica parpadear
                        if (menu_multitarea && parpadeo && estado_parpadeo.v) caracter=' '; //si hay parpadeo y toca, meter espacio tal cual (se oculta)

                        scr_putchar_menu(x,y,caracter,tinta,papel);
                    }

                    else if (caracter==255) {
                        //Significa no mostrar caracter. Usado en pantalla panic
                    }

                    //Si caracter no valido, mostrar ?
                    else {
                        tinta=overlay_screen_array[pos_array].tinta;
                        papel=overlay_screen_array[pos_array].papel;
                        scr_putchar_menu(x,y,'?',tinta,papel);
                    }
                }
            }
        }
    }

    normal_overlay_time_total_drawing_overlay=timer_stats_diference_time(&zxvision_time_total_antes,&zxvision_time_total_despues);

    normal_overlay_texto_menu_final();


}

/*
Antigua funcion con los "if" de ZXVISION_USE_CACHE_OVERLAY_TEXT
void old_delete_normal_overlay_texto_menu(void)
{


    //printf ("inicio normal_overlay_texto_menu\n");


    int x,y;
    int tinta,papel,parpadeo;

    z80_byte caracter;
    int pos_array=0;

    int nocache=0;
    if (!si_complete_video_driver() ) nocache=1;


    //printf ("normal_overlay_texto_menu\n");
    for (y=0;y<scr_get_menu_height();y++) {
        for (x=0;x<scr_get_menu_width();x++,pos_array++) {
            caracter=overlay_screen_array[pos_array].caracter;

            //sacamos el papel antes para poder alterarlo cuando se habilita DEBUG_ZXVISION_USE_CACHE_OVERLAY_TEXT
            papel=overlay_screen_array[pos_array].papel;
            //si caracter es 0, no mostrar
#ifdef ZXVISION_USE_CACHE_OVERLAY_TEXT
            if (
                (caracter && (overlay_screen_array[pos_array].modificado || nocache) ) ||  //Siempre que sea caracter!=0, y si se ha modificado la  cache
                overlay_screen_array[pos_array].parpadeo   //caracter con parpadeo se redibuja siempre
            ) {

                //Indicar que el caracter ya se ha dibujado en pantalla, para que en el siguiente refresco se muestre, si conviene
                overlay_screen_array[pos_array].modificado=0;



#else
            if (caracter) {
#endif
                //128 y 129 corresponden a franja de menu y a letra enye minuscula
                if (si_valid_char(caracter) ) {
                    tinta=overlay_screen_array[pos_array].tinta;
                    parpadeo=overlay_screen_array[pos_array].parpadeo;

                    //Si esta multitask, si es caracter con parpadeo y si el estado del contador del parpadeo indica parpadear
                    if (menu_multitarea && parpadeo && estado_parpadeo.v) caracter=' '; //si hay parpadeo y toca, meter espacio tal cual (se oculta)

                    scr_putchar_menu(x,y,caracter,tinta,papel);
                }

                else if (caracter==255) {
                    //Significa no mostrar caracter. Usado en pantalla panic
                }

                //Si caracter no valido, mostrar ?
                else {
                    tinta=overlay_screen_array[pos_array].tinta;
                    papel=overlay_screen_array[pos_array].papel;
                    scr_putchar_menu(x,y,'?',tinta,papel);
                }
            }
        }
    }

    normal_overlay_texto_menu_final();


}
*/


//establece cuadrado activo usado en los menus para xwindows y fbdev
void menu_establece_cuadrado(int x1,int y1,int x2,int y2,int color)
{

    cuadrado_x1=x1;
    cuadrado_y1=y1;
    cuadrado_x2=x2;
    cuadrado_y2=y2;
    cuadrado_color=color;
    cuadrado_activo=1;

    //Por defecto no se ve marca de resize, para compatibilidad con ventanas no zxvision
    cuadrado_activo_resize=0;
    cuadrado_activo_marca_redimensionado_aviso=0;
    ventana_activa_puede_minimizar=0;
    //ventana_activa_tipo_zxvision=0;

}

//desactiva cuadrado  usado en los menus para xwindows y fbdev
void menu_desactiva_cuadrado(void)
{
    cuadrado_activo=0;
    cuadrado_activo_resize=0;
    cuadrado_activo_marca_redimensionado_aviso=0;
    ventana_activa_puede_minimizar=0;
    //ventana_activa_tipo_zxvision=0;
}

//Devuelve 1 si hay dos ~~ seguidas en la posicion del indice o ~^ o ~ !
//~~: color hotkey, bajando letra a minusculas
//~^: color hotkey, sin bajar letra a minusculas
//~!: inverso siempre, cambiando tinta por papel, independientemente del estilo de GUI. Usado en lector de audio a zx81
//Sino, 0
int menu_escribe_texto_si_inverso(char *texto, int indice)
{

    if (menu_disable_special_chars.v) return 0;

    if (texto[indice++]!='~') return 0;
    if (texto[indice]!='~' && texto[indice]!='^' && texto[indice]!='!') {
        return 0;
    }

    indice++;

    //Y siguiente caracter no es final de texto
    if (texto[indice]==0) return 0;

    return 1;
}

//Devuelve 1 si hay dos ^^ seguidas en la posicion del indice
//Sino, 0
int menu_escribe_texto_si_parpadeo(char *texto, int indice)
{

    if (menu_disable_special_chars.v) return 0;

    if (texto[indice++]!='^') return 0;
    if (texto[indice++]!='^') return 0;

    //Y siguiente caracter no es final de texto
    if (texto[indice]==0) return 0;

    return 1;
}


int menu_escribe_texto_si_cambio_tinta(char *texto,int indice)
{
    if (menu_disable_special_chars.v) return 0;

    if (texto[indice++]!='$') return 0;
    if (texto[indice++]!='$') return 0;
    if (texto[indice]<'0' || texto[indice]>'7'+8) return 0; //Soportar colores con brillo
    indice++;

    //Y siguiente caracter no es final de texto
    if (texto[indice]==0) return 0;

    return 1;

}

//Quita simbolos ^^y ~~ y $$X de un texto. Puede que esta funcion este repetida en algun otro sitio
void menu_convierte_texto_sin_modificadores(char *texto,char *texto_destino)
{
    int origen,destino;

    char c;

    for (origen=0,destino=0;texto[origen];origen++,destino++) {
        //printf ("origen: %d destino: %d\n",origen,destino);
        if (menu_escribe_texto_si_inverso(texto,origen) || menu_escribe_texto_si_parpadeo(texto,origen) ) {
            origen +=2;
        }

        else if (menu_escribe_texto_si_cambio_tinta(texto,origen)) {
            origen +=3;
        }

        //else {
            c=texto[origen];
            texto_destino[destino]=c;
        //}
        //printf ("origen: %d destino: %d\n",origen,destino);
    }

    texto_destino[destino]=0;

}

int menu_es_prefijo_utf(z80_byte caracter)
{
    if (caracter==0xD0 || caracter==0xD1 || caracter==0xC2 || caracter==0xC3 || caracter==0xC9 || caracter==0xCE) return 1;
    else return 0;
}

//Si decimos que no queremos mostrar caracteres extendidos para parseo de utf
//si no muestra, por ejemplo una a acentuada se mostrara como a
int forzar_no_mostrar_caracteres_extendidos=0;

int si_menu_muestra_caracteres_extendidos(void)
{
    if (forzar_no_mostrar_caracteres_extendidos) return 0;

    else return si_complete_video_driver();
}

unsigned char menu_escribe_texto_convert_utf(unsigned char prefijo_utf,unsigned char caracter)
{


    if (prefijo_utf==0xC2) {

        if (caracter==0xAB) {
            //«
            //no hay caracter raro, siempre es "
            return '"';
        }

        if (caracter==0xBB) {
            //»
            //no hay caracter raro, siempre es "
            return '"';
        }
    }

    if (prefijo_utf==0xC3) {

        if (caracter==0x81) {
            //Á Acentuada mayuscula
            if (si_menu_muestra_caracteres_extendidos()) {
                return 168;
            }
            else {
                return 'A';
            }
        }

        if (caracter==0x91) {
            //Eñe mayuscula
            if (si_menu_muestra_caracteres_extendidos()) {
                return 147; //Eñe mayuscula
            }
            else {
                return 'N';
            }
        }

        if (caracter==0xB1) {
            //Eñe
            if (si_menu_muestra_caracteres_extendidos()) {
                return 129; //Eñe
            }
            else {
                return 'n';
            }
        }


        if (caracter==0xA0) {
            //à a acentuada abierta
            if (si_menu_muestra_caracteres_extendidos()) {
                return 159;
            }
            else {
                return 'a';
            }
        }

        if (caracter==0xA1) {
            //á a acentuada
            if (si_menu_muestra_caracteres_extendidos()) {
                return 142;
            }
            else {
                return 'a';
            }
        }

        if (caracter==0xA7) {
            //ç cedilla
            if (si_menu_muestra_caracteres_extendidos()) {
                return 158;
            }
            else {
                return 'c';
            }
        }

        if (caracter==0xA8) {
            //è e acentuada abierta
            if (si_menu_muestra_caracteres_extendidos()) {
                return 160;
            }
            else {
                return 'e';
            }
        }

        if (caracter==0xA9) {
            //é e acentuada
            if (si_menu_muestra_caracteres_extendidos()) {
                return 143;
            }
            else {
                return 'e';
            }
        }

        if (caracter==0xAD) {
            //í i acentuada
            if (si_menu_muestra_caracteres_extendidos()) {
                return 144;
            }
            else {
                return 'i';
            }
        }

        if (caracter==0xAF) {
            //ï i con dieresis
            if (si_menu_muestra_caracteres_extendidos()) {
                return 167;
            }
            else {
                return 'i';
            }
        }

        if (caracter==0xB2) {
            //ò o acentuada abierta
            if (si_menu_muestra_caracteres_extendidos()) {
                return 161;
            }
            else {
                return 'o';
            }
        }

        if (caracter==0xB3) {
            //ó o acentuada
            if (si_menu_muestra_caracteres_extendidos()) {
                return 145;
            }
            else {
                return 'o';
            }
        }

        if (caracter==0xBA) {
            //ú u acentuada
            if (si_menu_muestra_caracteres_extendidos()) {
                return 146;
            }
            else {
                return 'u';
            }
        }

        if (caracter==0xBC) {
            //ü u con dieresis
            if (si_menu_muestra_caracteres_extendidos()) {
                return 162;
            }
            else {
                return 'u';
            }
        }

    }

    if (prefijo_utf==0xC9) {
        if (caracter==0xBE) {
            //r minuscula con anzuelo. Sale en el FAQ, en la pronunciacion de ZEsarUX
            if (si_menu_muestra_caracteres_extendidos()) {
                                return 137;
                        }
                        else {
                                return 'r';
                        }
                }

    }

    if (prefijo_utf==0xCE) {
        if (caracter==0xB8) {
            //Greek Small Letter Theta θ - 138. Sale en el FAQ, en la pronunciacion de ZEsarUX
            if (si_menu_muestra_caracteres_extendidos()) {
                                return 138;
                        }
                        else {
                                return 'z'; //no se si es lo mas parecido a θ pero en cuanto a sonido es la Z en ZEsarUX
                        }
                }

    }

    if (prefijo_utf==0xD0) {
        if (caracter==0x90) return 'A';
        if (caracter==0x92) return 'B';
        if (caracter==0x9C) return 'M'; //cyrillic capital letter em (U+041C)
        if (caracter==0xA1) return 'C';
        if (caracter==0xA8) { //Ш
            if (si_menu_muestra_caracteres_extendidos()) {
                return 131;
            }
            else {
                return 'W';
            }
        }
        if (caracter==0xB0) return 'a';
        if (caracter==0xB2) return 'B';



        if (caracter==0xB3) {
            if (si_menu_muestra_caracteres_extendidos()) {
                return 133; //г
            }
            else {
                return 'g';
            }
        }

        if (caracter==0xB4) {
            if (si_menu_muestra_caracteres_extendidos()) {
                return 135; //д
            }
            else {
                return 'D';
            }
        }

        if (caracter==0xB5) return 'e';
        if (caracter==0xB8) {
            if (si_menu_muestra_caracteres_extendidos()) {
                return 130; //CYRILLIC SMALL LETTER I и
            }
            else {
                return 'i';
            }
        }
        if (caracter==0xBA) return 'k';
        if (caracter==0xBB) { //л
            if (si_menu_muestra_caracteres_extendidos()) {
                return 132;
            }
            else {
                return 'l';
            }
        }
        if (caracter==0xBC) return 'M';
        if (caracter==0xBD) return 'H';
        if (caracter==0xBE) return 'o';
    }

    if (prefijo_utf==0xD1) {
                if (caracter==0x80) return 'p';
                if (caracter==0x81) return 'c';
                if (caracter==0x82) return 'T';
                if (caracter==0x83) return 'y';
                if (caracter==0x85) return 'x';



                if (caracter==0x87) {
                    if (si_menu_muestra_caracteres_extendidos()) {
                        return 134; //ч
                    }
                    else {
                        return 'y';
                    }
                }

                //я
                if (caracter==0x8F) {
                    if (si_menu_muestra_caracteres_extendidos()) {
                        return 136; //я
                    }
                    else {
                        return 'a'; //no es lo mismo, sonaria como una "ja" en dutch, pero bueno
                    }
                }

        }

    return '?';


    //Nota: caracteres que generan texto fuera de la tabla normal, considerar si es un driver de texto o grafico, con if (si_menu_muestra_caracteres_extendidos() ) {
}


//escribe una linea de texto
//coordenadas relativas al interior de la pantalla de spectrum (0,0=inicio pantalla)

//Si codigo de color inverso, invertir una letra
//Codigo de color inverso: dos ~ seguidas
//Funcion CASI no usada, solo unas pocas funciones la usan. Era mas usada antiguamente
//pero con el cambio a zxvision se pasa a escribir en contexto de ventana en la mayoria de los casos
void menu_escribe_texto(int x,int y,int tinta,int papel,char *texto)
{
        unsigned int i;
    z80_byte letra;

    int parpadeo=0;

    int era_utf=0;

    //y luego el texto
    for (i=0;i<strlen(texto);i++) {
        letra=texto[i];

        //Si dos ^ seguidas, invertir estado parpadeo
        if (menu_escribe_texto_si_parpadeo(texto,i)) {
            parpadeo ^=1;
            //y saltamos esos codigos de negado
                        i +=2;
                        letra=texto[i];
        }

        //codigo control color tinta
        if (menu_escribe_texto_si_cambio_tinta(texto,i)) {
            tinta=texto[i+2]-'0';
            i+=3;
            letra=texto[i];
        }

        //ver si dos ~~ seguidas y cuidado al comparar que no nos vayamos mas alla del codigo 0 final
        if (menu_escribe_texto_si_inverso(texto,i)) {
            //y saltamos esos codigos de negado
            i +=2;
            letra=texto[i];

            if (menu_writing_inverse_color.v) putchar_menu_overlay_parpadeo(x,y,letra,papel,tinta,parpadeo);
            else putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
        }

        else {

            //Si estaba prefijo utf activo

            if (era_utf) {
                letra=menu_escribe_texto_convert_utf(era_utf,letra);
                era_utf=0;

                //Caracter final utf
                putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
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
                    putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
                }
            }


        }

        //if (x>=32) {
        //	printf ("Escribiendo caracter [%c] en x: %d\n",letra,x);
        //}


        if (!era_utf) x++;
    }

}





int menu_if_speech_enabled(void)
{
        if (textspeech_filter_program==NULL) return 0;
        if (textspeech_also_send_menu.v==0) return 0;
        if (menu_speech_tecla_pulsada) return 0;

    return 1;
}

void menu_textspeech_filter_corchetes(char *texto_orig,char *texto)
{
    char texto_active_item[32]=""; //Inicializado a vacio de momento
    int inicio_corchete=0;

    //Buscar si empieza con "Selected item: "
    char *encontrado;

    char *cadena_buscar="Selected item: ";

    encontrado=strstr(texto_orig,cadena_buscar);
    if (encontrado==texto_orig) {
        //Avanzamos el indice a inicio a buscar
        inicio_corchete=strlen(cadena_buscar);
        //Y metemos cadena "prefijo"
        strcpy(texto_active_item,cadena_buscar);

        //printf ("Encontrado texto Selected item en %s\n",texto_orig);
    }

    //char texto[MAX_BUFFER_SPEECH+1];

    //buscar primero si hay [ ] al principio

    int cambiado=0;
    //printf ("texto: %s. inicio corchete %d\n",texto_orig,inicio_corchete);
    if (texto_orig[inicio_corchete]=='[') {
        //posible
        int i;
        for (i=inicio_corchete;texto_orig[i]!=0 && !cambiado;i++) {
            //printf ("%d\n",i);
            if (texto_orig[i]==']') {
                //Hay inicio con [..]. Ponerlo al final en nueva string
                char buf_opcion[MAX_BUFFER_SPEECH+1];
                strcpy(buf_opcion,&texto_orig[inicio_corchete]);

                int longitud_opcion=(i+1)-inicio_corchete;

                //buf_opcion[i+1]=0;  //buf_opcion contiene solo los corchetes y lo de dentro de corchetes
                buf_opcion[longitud_opcion]=0;  //buf_opcion contiene solo los corchetes y lo de dentro de corchetes

                //Y ahora ademas, si la opcion es [ ] dice disabled. Si es [x] dice enabled
                //TODO: solo estamos detectando esto a principio de linea. Creo que no hay ningun menu en que diga [ ] o [X] en otro
                //sitio que no sea principio de linea. Si estuviera en otro sitio, no funcionaria
                if (!strcmp(buf_opcion,"[ ]")) strcpy(buf_opcion,"Disabled");
                else if (!strcmp(buf_opcion,"[X]")) strcpy(buf_opcion,"Enabled");

                sprintf(texto,"%s%s. %s",texto_active_item,&texto_orig[i+1],buf_opcion);
                //printf ("Detected setting at the beginning of the line. Changing speech to menu item and setting: %s\n",texto);
                cambiado=1;
            }
        }
    }

    if (!cambiado) strcpy(texto,texto_orig);


}

void menu_textspeech_send_text(char *texto_orig)
{

    if (!menu_if_speech_enabled() ) return;
    //printf("menu_speech_tecla_pulsada en menu_textspeech_send_text: %d\n",menu_speech_tecla_pulsada);

    //if (!menu_speech_tecla_pulsada) debug_exec_show_backtrace();

    //printf ("Send text to speech: %s\n",texto_orig);
    //printf ("Send text to speech from: \n");
    //debug_exec_show_backtrace();

    debug_printf (VERBOSE_DEBUG,"Send text to speech: %s",texto_orig);



    //-si item empieza por [, buscar hasta cierre ]. Y eso se reproduce al final de linea
    //-Si [X], se dice "enabled". Si [ ], se dice "disabled"
    //TODO: aqui se llama tambien al decir directorios por ejemplo. Si hay directorio con [ ] (cosa rara) se interpretaria como una opcion
    //y se diria al final

    //Detectar tambien si al principio se dice "Selected item: "
    //Esto por tanto solo servira cuando el [] esta a principio de linea o bien despues de "Selected item: "
    //Se podria extender a localizar los [] en cualquier sitio pero entonces el problema podria venir por alguna linea
    //que tuviera [] en medio y no fuera de menu, y la moviese al final

    char texto[MAX_BUFFER_SPEECH+1];
    menu_textspeech_filter_corchetes(texto_orig,texto);




    //Eliminamos las ~~ o ^^ del texto. Realmente eliminamos cualquier ~ aunque solo haya una
    //Si hay dos ~~, decir que atajo es al final del texto
    int orig,dest;
    orig=dest=0;
    char buf_speech[MAX_BUFFER_SPEECH+1];

    char letra_atajo=0;

    //printf ("texto: puntero: %d strlen: %d : %s\n",texto,strlen(texto),texto);

    for (;texto[orig]!=0;orig++) {

        if (texto[orig]=='~' && texto[orig+1]=='~') {
            letra_atajo=texto[orig+2];
        }

        //printf ("texto orig : %d\n",texto[orig]);
        //printf ("texto orig char: %c\n",texto[orig]);
        //TODO: saltar cuando hay cambio de color de tinta %%X
        //Si no es ~ ni ^, copiar e incrementar destino
        if (texto[orig]!='~' && texto[orig]!='^') {
            buf_speech[dest]=texto[orig];
            dest++;
        }
    }


    //Si se ha encontrado letra atajo
    if (letra_atajo!=0) {
        buf_speech[dest++]='.';
        buf_speech[dest++]=' ';
        //Parece que en los sistemas de speech la letra mayuscula se lee con mas pausa (al menos testado con festival)
        buf_speech[dest++]=letra_mayuscula(letra_atajo);
        buf_speech[dest++]='.';
    }

    buf_speech[dest]=0;


    //printf ("directorio antes: [%s]\n",buf_speech);

    //Si hay texto <dir> cambiar por directory
    char *p;
    p=strstr(buf_speech,"<dir>");
    if (p) {
        //suponemos que esto va a final de texto
        sprintf (p,"%s","directory");
    }

    //Si directorio es ".."
    if (!strcmp(buf_speech,".. directory") || !strcmp(buf_speech,"..                     directory") ) {
        strcpy(buf_speech,"dot dot directory");
    }

    //Si directorio es ".."
    if (!strcmp(buf_speech,"Selected item: .. directory")) {
        strcpy(buf_speech,"Selected item: dot dot directory");
    }


    //Si es todo espacios sin ningun caracter, no enviar
    int vacio=1;
    for (orig=0;buf_speech[orig]!=0;orig++) {
        if (buf_speech[orig]!=' ') {
            vacio=0;
            break;
        }
    }

    if (vacio==1) {
        debug_printf (VERBOSE_DEBUG,"Empty line, do not send to speech");
        return;
    }





    debug_printf (VERBOSE_DEBUG,"Final sent text to speech after processing filters: %s",buf_speech);
    textspeech_print_speech(buf_speech);
    //printf ("textspeech_print_speech: %s\n",buf_speech);

    //hacemos que el timeout de tooltip se reinicie porque sino cuando se haya leido el menu, acabara saltando el timeout
    //menu_tooltip_counter=0;

    z80_byte acumulado;


    /*
    int contador_refresco=0;
    int max_contador_refresco;

    //Algo mas de un frame de pantalla
    if (menu_multitarea==1) max_contador_refresco=100000;

    //Cada 20 ms
    //Nota: Ver funcion  menu_cpu_core_loop(void) , donde hay usleep(500) en casos de menu_multitarea 0
    else max_contador_refresco=40;
    */

        //Parece que esto en maquinas lentas (especialmente en mi Windows virtual). Bueno creo que realmente no es un problema de Windows,
        //si no de que la maquina es muy lenta y tarda en refrescar la pantalla mientras esta
        //esperando una tecla y reproduciendo speech. Si quito esto, sucede que
        //si se pulsa el cursor rapido y hay speech, dicho cursor va "con retraso" una posicion
    menu_refresca_pantalla();

        do {
                if (textspeech_finalizado_hijo_speech() ) scrtextspeech_filter_run_pending();

                menu_cpu_core_loop();
                acumulado=menu_da_todas_teclas();


                        //Hay tecla pulsada
                        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA ) {
                //printf ("pulsada tecla\n");
                                //int tecla=menu_get_pressed_key();

                //de momento cualquier tecla anula speech
                textspeech_empty_speech_fifo();

                menu_speech_set_tecla_pulsada();

                        }

            //no hay tecla pulsada
            else {
                //Decir que no repeticion de tecla. Si no pusiesemos esto aqui,
                //pasaria que si entramos con repeticion activa, y
                //mientras esperamos a que acabe proceso hijo, no pulsamos una tecla,
                //la repeticion seguiria activa
                menu_reset_counters_tecla_repeticion();
            }

        //Parece que esto en maquinas lentas (especialmente en mi Windows virtual). Bueno creo que realmente no es un problema de Windows,
        //si no de que la maquina es muy lenta y tarda en refrescar la pantalla mientras esta
        //esperando una tecla y reproduciendo speech. Si quito esto, sucede que
        //si se pulsa el cursor rapido y hay speech, dicho cursor va "con retraso" una posicion
        /*contador_refresco++;
        if (contador_refresco==max_contador_refresco) {
            printf ("refrescar\n");
            contador_refresco=0;
            menu_refresca_pantalla();
        }
        */


        } while (!textspeech_finalizado_hijo_speech() && menu_speech_tecla_pulsada==0);
    //hacemos que el timeout de tooltip se reinicie porque sino cuando se haya leido el menu, acabara saltando el timeout
    menu_tooltip_counter=0;

}

void menu_retorna_colores_linea_opcion(int indice,int opcion_actual,int opcion_activada,int *papel_orig,int *tinta_orig,int opcion_marcada,int opcion_campo_seleccionable)
{
    int papel,tinta;

    /*
    5 combinaciones:
    opcion seleccionada, disponible (activada)
    opcion seleccionada, no disponible
    opcion no seleccionada, disponible
    opcion no seleccionada, disponible, marcada
    opcion no seleccionada, no disponible
    */

        //Opcion seleccionada
        if (opcion_actual==indice) {
                if (opcion_activada==1) {
                        papel=ESTILO_GUI_PAPEL_SELECCIONADO;
                        tinta=ESTILO_GUI_TINTA_SELECCIONADO;
                }
                else {
                        papel=ESTILO_GUI_PAPEL_SEL_NO_DISPONIBLE;
                        tinta=ESTILO_GUI_TINTA_SEL_NO_DISPONIBLE;
                }
        }

        //Opcion no seleccionada
        else {
                if (opcion_activada==1) {
                    if (opcion_marcada) {
                        papel=ESTILO_GUI_PAPEL_OPCION_MARCADA;
                        tinta=ESTILO_GUI_TINTA_OPCION_MARCADA;
                    }
                    else if (opcion_campo_seleccionable) {
                        papel=ESTILO_GUI_PAPEL_CAMPO_SELECCIONABLE;
                        tinta=ESTILO_GUI_TINTA_CAMPO_SELECCIONABLE;
                    }
                    else {
                        papel=ESTILO_GUI_PAPEL_NORMAL;
                        tinta=ESTILO_GUI_TINTA_NORMAL;
                    }
                }
                else {
                        papel=ESTILO_GUI_PAPEL_NO_DISPONIBLE;
                        tinta=ESTILO_GUI_TINTA_NO_DISPONIBLE;
                }
        }

    *papel_orig=papel;
    *tinta_orig=tinta;

}

//Dice si el menu es de tipo stdout
int menu_es_stdout(void)
{
    if (!strcmp(scr_new_driver_name,"stdout")) {
        return 1;
    }

    else {
        return 0;
    }
}

//escribe opcion de linea de texto
//coordenadas "indice" relativa al interior de la ventana (0=inicio)
//opcion_actual indica que numero de linea es la seleccionada
//opcion activada indica a 1 que esa opcion es seleccionable
void menu_escribe_linea_opcion_zxvision(zxvision_window *ventana,int indice,int opcion_actual,int opcion_activada,char *texto_entrada,
    int tiene_submenu,int opcion_marcada,int genera_ventana,int opcion_campo_seleccionable)
{

    char texto[MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH+1];
    //Le doy 1 byte mas. Por si acaso alguien llama aqui sin contar el byte 0 del final y la lia...

    if (menu_es_stdout()) {
        printf ("%s\n",texto_entrada);
        scrstdout_menu_print_speech_macro (texto_entrada);
        return;
    }

    //printf("menu_escribe_linea_opcion_zxvision. indice %d opcion_actual %d texto %s\n",indice,opcion_actual,texto_entrada);

    int papel,tinta;
    int i;

    //tinta=0;


    menu_retorna_colores_linea_opcion(indice,opcion_actual,opcion_activada,&papel,&tinta,opcion_marcada,opcion_campo_seleccionable);


    //Obtenemos colores de una opcion sin seleccion y activada, para poder tener texto en ventana con linea en dos colores
    int papel_normal,tinta_normal;
    menu_retorna_colores_linea_opcion(0,-1,1,&papel_normal,&tinta_normal,0,opcion_campo_seleccionable);

    //Buscamos a ver si en el texto hay el caracter "||" y en ese caso lo eliminamos del texto final
    int encontrado=-1;
    int destino=0;
    for (i=0;texto_entrada[i];i++) {
        if (menu_disable_special_chars.v==0 && texto_entrada[i]=='|' && texto_entrada[i+1]=='|') {
            encontrado=i;
            i ++;
        }
        else {
            texto[destino++]=texto_entrada[i];
        }
    }

    //Si genera ventana mostrar ...
    if (genera_ventana) {
        texto[destino++]='.';
        texto[destino++]='.';
        texto[destino++]='.';
    }

    texto[destino]=0;


    //linea entera con espacios
    for (i=0;i<current_win_ancho;i++) {
        zxvision_print_string(ventana,i,indice,0,papel,0," ");
    }

    //y texto propiamente
    int startx=menu_escribe_linea_startx;
    zxvision_print_string(ventana,startx,indice,tinta,papel,0,texto);

    //Si tiene submenu, mostrar caracter >
    if (tiene_submenu && menu_hide_submenu_indicator.v==0) {
        int ancho=ventana->visible_width;
        zxvision_print_string(ventana,ancho-1,indice,tinta,papel,0,">");
    }

    //Si tiene dos colores
    if (encontrado>=0) {
        zxvision_print_string(ventana,startx+encontrado,indice,tinta_normal,papel_normal,0,&texto[encontrado]);
    }

    //si el driver de video no tiene colores o si el estilo de gui lo indica, indicamos opcion activa con un cursor
    if (!scr_tiene_colores || ESTILO_GUI_MUESTRA_CURSOR) {
        if (opcion_actual==indice) {
            if (opcion_activada==1) {
                zxvision_print_string(ventana,0,indice,tinta,papel,0,">");
            }
            else {
                zxvision_print_string(ventana,0,indice,tinta,papel,0,"x");
            }
        }
    }
    if (menu_if_speech_enabled() ) {
        //printf ("redibujar ventana\n");
        zxvision_draw_window_contents_no_speech(ventana);
        //menu_refresca_pantalla();
    }

    menu_textspeech_send_text(texto);




}






//escribe opcion de texto tabulado
//coordenadas "indice" relativa al interior de la ventana (0=inicio)
//opcion_actual indica que numero de linea es la seleccionada
//opcion activada indica a 1 que esa opcion es seleccionable
void menu_escribe_linea_opcion_tabulado_zxvision(zxvision_window *ventana,int indice,int opcion_actual,int opcion_activada,char *texto,
    int x,int y,int opcion_marcada,int opcion_campo_seleccionable)
{

        if (menu_es_stdout()) {
                printf ("%s\n",texto);
                scrstdout_menu_print_speech_macro (texto);
                return;
        }


        int papel,tinta;


        menu_retorna_colores_linea_opcion(indice,opcion_actual,opcion_activada,&papel,&tinta,opcion_marcada,opcion_campo_seleccionable);


        zxvision_print_string(ventana,x,y,tinta,papel,0,texto);
        //printf ("Escribiendo texto tabulado %s en %d,%d\n",texto,x,y);

        menu_textspeech_send_text(texto);

}

void menu_retorna_margenes_border(int *miz, int *mar)
{
    //margenes de zona interior de pantalla. para modo rainbow
    int margenx_izq=screen_total_borde_izquierdo*border_enabled.v;
    int margeny_arr=screen_borde_superior*border_enabled.v;

if (MACHINE_IS_Z88) {
//margenes para realvideo
margenx_izq=margeny_arr=0;
}


    else if (MACHINE_IS_CPC) {
//margenes para realvideo
margenx_izq=CPC_LEFT_BORDER_NO_ZOOM*border_enabled.v;
                    margeny_arr=CPC_TOP_BORDER_NO_ZOOM*border_enabled.v;
    }

    else if (MACHINE_IS_PCW) {
//margenes para realvideo
margenx_izq=PCW_LEFT_BORDER_NO_ZOOM*border_enabled.v;
                    margeny_arr=PCW_TOP_BORDER_NO_ZOOM*border_enabled.v;
    }

    else if (MACHINE_IS_PRISM) {
//margenes para realvideo
margenx_izq=PRISM_LEFT_BORDER_NO_ZOOM*border_enabled.v;
                    margeny_arr=PRISM_TOP_BORDER_NO_ZOOM*border_enabled.v;
    }

    else if (MACHINE_IS_TBBLUE) {
//margenes para realvideo
margenx_izq=TBBLUE_LEFT_BORDER_NO_ZOOM*border_enabled.v;
                    margeny_arr=TBBLUE_TOP_BORDER_NO_ZOOM*border_enabled.v;
    }

    else if (MACHINE_IS_SAM) {
                    //margenes para realvideo
                    margenx_izq=SAM_LEFT_BORDER_NO_ZOOM*border_enabled.v;
                    margeny_arr=SAM_TOP_BORDER_NO_ZOOM*border_enabled.v;
    }

    else if (MACHINE_IS_QL) {
                    //margenes para realvideo
                    margenx_izq=QL_LEFT_BORDER_NO_ZOOM*border_enabled.v;
                    margeny_arr=QL_TOP_BORDER_NO_ZOOM*border_enabled.v;
    }

    else if (MACHINE_IS_TSCONF) {
        margenx_izq=margeny_arr=0;
    }

    *miz=margenx_izq;
    *mar=margeny_arr;

}

//Indica que el raton esta encima de la zona de redimensionado y por tanto se dibuja diferente
int ventana_marca_redimensionado_raton_encima=0;

//dibuja cuadrado (4 lineas) usado en los menus para xwindows y fbdev
//Entrada: x1,y1 punto superior izquierda,x2,y2 punto inferior derecha en resolucion de zx spectrum. Color
//nota: realmente no es un cuadrado porque el titulo ya hace de franja superior
void menu_dibuja_cuadrado(int x1,int y1,int x2,int y2,int color,int color_marca_redimensionado)
{

    if (!ESTILO_GUI_MUESTRA_RECUADRO) return;


    int x,y;

    //Si ratón está encima de la zona de redimensionado
    if (ventana_marca_redimensionado_raton_encima && menu_change_frame_when_resize_zone.v) {
        color=ESTILO_GUI_COLOR_AVISO;
        color_marca_redimensionado=ESTILO_GUI_COLOR_AVISO;
    }

    //Para poner una marca en la ventana indicando si es de tipo zxvision
    //int centro_marca_zxvison_x=x2-3-6;
    //int centro_marca_zxvison_y=y1+3+2;

    //int longitud_marca_zxvision=3;
    //int mitad_long_marca_zxvision=longitud_marca_zxvision/2;
    //int color_marca_zxvision=ESTILO_GUI_PAPEL_NORMAL;


    //printf ("Cuadrado %d,%d - %d,%d\n",x1,y1,x2,y2);


    //solo hacerlo en el caso de drivers completos
    if (si_complete_video_driver() ) {


        //parte inferior
        for (x=x1;x<=x2;x++) {
            if (mouse_is_dragging && (x%2)==0) continue; //punteado cuando se mueve o redimensiona
            scr_putpixel_gui_zoom(x*menu_gui_zoom,y2*menu_gui_zoom,color,menu_gui_zoom);
        }


        //izquierda
        for (y=y1;y<=y2;y++) {
            if (mouse_is_dragging && (y%2)==0) continue; //punteado cuando se mueve o redimensiona
            scr_putpixel_gui_zoom(x1*menu_gui_zoom,y*menu_gui_zoom,color,menu_gui_zoom);
        }



        //derecha
        for (y=y1;y<=y2;y++) {
            if (mouse_is_dragging && (y%2)==0) continue; //punteado cuando se mueve o redimensiona
            scr_putpixel_gui_zoom(x2*menu_gui_zoom,y*menu_gui_zoom,color,menu_gui_zoom);
        }




        //Marca redimensionado
        if (cuadrado_activo_resize) {
            //marca de redimensionado
            //		  *
            //		 **
            //		***
            //     ****

            //Arriba del todo
            scr_putpixel_gui_zoom((x2-1)*menu_gui_zoom,(y2-4)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);

            //Medio
            scr_putpixel_gui_zoom((x2-1)*menu_gui_zoom,(y2-3)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            scr_putpixel_gui_zoom((x2-2)*menu_gui_zoom,(y2-3)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);

            //Abajo
            scr_putpixel_gui_zoom((x2-1)*menu_gui_zoom,(y2-2)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            scr_putpixel_gui_zoom((x2-2)*menu_gui_zoom,(y2-2)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            scr_putpixel_gui_zoom((x2-3)*menu_gui_zoom,(y2-2)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);

            //Abajo del todo
            scr_putpixel_gui_zoom((x2-1)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            scr_putpixel_gui_zoom((x2-2)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            scr_putpixel_gui_zoom((x2-3)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            scr_putpixel_gui_zoom((x2-4)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);

        }

        /*
        if (ventana_activa_tipo_zxvision) {
            //Poner un pixel avisando que ventana no es zxvision
            scr_putpixel_gui_zoom((centro_marca_zxvison_x)*menu_gui_zoom,(centro_marca_zxvison_y)*menu_gui_zoom,color_marca_zxvision,menu_gui_zoom);
        }
        */




    }


}

void menu_muestra_pending_error_message(void)
{
    if (if_pending_error_message) {
        if_pending_error_message=0;
        debug_printf (VERBOSE_INFO,"Showing pending error message on menu");
        //menu_generic_message("ERROR",pending_error_message);
        menu_error_message(pending_error_message);
    }
}


//x,y origen ventana, ancho ventana
void menu_dibuja_ventana_franja_arcoiris_oscuro(int x, int y, int ancho,int indice)
{

    if (!ventana_tipo_activa) return;

    //int cr[]={2,6,4,5};

    int cr[4];
    //Copiar del estilo actual aqui, pues internamente lo modificamos
    int i;
    int *temp_ptr;
    temp_ptr=ESTILO_GUI_FRANJAS_OSCURAS;
    for (i=0;i<4;i++) {
        cr[i]=temp_ptr[i];
    }
    //int *cr;
    //cr=ESTILO_GUI_FRANJAS_OSCURAS;

    //int indice=4-franjas;

    if (indice>=0 && indice<=3) {
        //cr[indice]+=8;
        //Coger color de las normales brillantes
        int *temp_ptr_brillo;
        temp_ptr_brillo=ESTILO_GUI_FRANJAS_NORMALES;
        cr[indice]=temp_ptr_brillo[indice];
    }

    int restar=0;

    if (zxvision_window_can_be_backgrounded(zxvision_current_window)) restar++;

    x-=restar;

    //Esto va considerando que se tienen 3 botones a la derecha de background, minimizar y maximizar
    //total debe ser 7 para 3 botones
    int margen=ZXVISION_WIDTH_RAINBOW_TITLE+ZXVISION_TOTAL_BUTTONS_RIGHT-1;
    int indice_inicio_franjas=x+ancho-margen;

    z80_byte caracter_franja=128;

    //Quiero que salgan las franjas aunque el estilo no lo permita en ventanas de error y splash message
    //Excepto BEOS
    if (!ESTILO_GUI_NO_RELLENAR_TITULO) {
    //if (1/*ESTILO_GUI_MUESTRA_RAINBOW*/) {

        if (si_complete_video_driver() ) {

            if (ESTILO_GUI_CARACTER_FRANJA!=0) {
                caracter_franja=ESTILO_GUI_CARACTER_FRANJA;

                for (i=0;i<4;i++) {
                    //Empieza desde el final al principio, pues si hay menos franjas
                    int posicion_x=indice_inicio_franjas+4-i;
                    int indice_color=3-i;
                    //printf("Pos: %d indice: %d\n",posicion_x,indice_color);

                     putchar_menu_overlay(posicion_x,y,caracter_franja,cr[indice_color],ESTILO_GUI_PAPEL_TITULO);

                }


            }

            else {


                putchar_menu_overlay(indice_inicio_franjas,y,caracter_franja,cr[0],ESTILO_GUI_PAPEL_TITULO);
                putchar_menu_overlay(indice_inicio_franjas+1,y,caracter_franja,cr[1],cr[0]);
                putchar_menu_overlay(indice_inicio_franjas+2,y,caracter_franja,cr[2],cr[1]);
                putchar_menu_overlay(indice_inicio_franjas+3,y,caracter_franja,cr[3],cr[2]);
                putchar_menu_overlay(indice_inicio_franjas+4,y,caracter_franja,ESTILO_GUI_PAPEL_TITULO,cr[3]);
            }
        }

        //en caso de curses o caca, hacerlo con lineas de colores
        if (!strcmp(scr_new_driver_name,"curses") || !strcmp(scr_new_driver_name,"caca") ) {


            putchar_menu_overlay(indice_inicio_franjas+1,y,'/',cr[0],ESTILO_GUI_PAPEL_TITULO);
            putchar_menu_overlay(indice_inicio_franjas+2,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
            putchar_menu_overlay(indice_inicio_franjas+3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
            putchar_menu_overlay(indice_inicio_franjas+4,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
        }

    }
}

//x,y origen ventana, ancho ventana
void menu_dibuja_ventana_franja_arcoiris_trozo(int x, int y, int ancho,int franjas)
{

    if (!ventana_tipo_activa) return;

    //int cr[]={2+8,6+8,4+8,5+8};
    int *cr;
    cr=ESTILO_GUI_FRANJAS_NORMALES;

    int restar=0;

    if (zxvision_window_can_be_backgrounded(zxvision_current_window)) restar++;

    x-=restar;

    //Esto va considerando que se tienen 3 botones a la derecha de background, minimizar y maximizar
    //total debe ser 7 para 3 botones
    int margen=ZXVISION_WIDTH_RAINBOW_TITLE+ZXVISION_TOTAL_BUTTONS_RIGHT-1;
    int indice_inicio_franjas=x+ancho-margen;

    z80_byte caracter_franja=128;

    //Quiero que salgan las franjas aunque el estilo no lo permita en ventanas de error y splash message
    //Excepto BEOS
    if (!ESTILO_GUI_NO_RELLENAR_TITULO) {
    //if (1/*ESTILO_GUI_MUESTRA_RAINBOW*/) {
        //en el caso de drivers completos, hacerlo real
        if (si_complete_video_driver() ) {
            //5 espacios negro primero
            int i;
            for (i=margen;i>=margen-4;i--) putchar_menu_overlay(x+ancho-i,y,menu_retorna_caracter_espacio_titulo(),ESTILO_GUI_TINTA_TITULO,ESTILO_GUI_PAPEL_TITULO);

            if (ESTILO_GUI_CARACTER_FRANJA!=0) {
                caracter_franja=ESTILO_GUI_CARACTER_FRANJA;

                for (i=0;i<franjas;i++) {
                    //Empieza desde el final al principio, pues si hay menos franjas
                    int posicion_x=indice_inicio_franjas+4-i;
                    int indice_color=3-i;
                    //printf("Pos: %d indice: %d\n",posicion_x,indice_color);

                    //por si acaso
                    if (indice_color>=0) {
                        putchar_menu_overlay(posicion_x,y,caracter_franja,cr[indice_color],ESTILO_GUI_PAPEL_TITULO);
                    }
                }


            }

            else {

                    if (franjas==4) {
                        putchar_menu_overlay(indice_inicio_franjas,y,caracter_franja,cr[0],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+1,y,caracter_franja,cr[1],cr[0]);
                        putchar_menu_overlay(indice_inicio_franjas+2,y,caracter_franja,cr[2],cr[1]);
                        putchar_menu_overlay(indice_inicio_franjas+3,y,caracter_franja,cr[3],cr[2]);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,caracter_franja,ESTILO_GUI_PAPEL_TITULO,cr[3]);
                    }

                     if (franjas==3) {
                        putchar_menu_overlay(indice_inicio_franjas+1,y,caracter_franja,cr[1],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+2,y,caracter_franja,cr[2],cr[1]);
                        putchar_menu_overlay(indice_inicio_franjas+3,y,caracter_franja,cr[3],cr[2]);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,caracter_franja,ESTILO_GUI_PAPEL_TITULO,cr[3]);
                    }


                    if (franjas==2) {
                        putchar_menu_overlay(indice_inicio_franjas+2,y,caracter_franja,cr[2],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+3,y,caracter_franja,cr[3],cr[2]);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,caracter_franja,ESTILO_GUI_PAPEL_TITULO,cr[3]);
                    }

                    if (franjas==1) {
                        putchar_menu_overlay(indice_inicio_franjas+3,y,caracter_franja,cr[3],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,caracter_franja,ESTILO_GUI_PAPEL_TITULO,cr[3]);
                    }
            }


        }

        //en caso de curses o caca, hacerlo con lineas de colores
            if (!strcmp(scr_new_driver_name,"curses") || !strcmp(scr_new_driver_name,"caca") ) {
                    //putchar_menu_overlay(x+ancho-6,y,'/',cr[0],ESTILO_GUI_PAPEL_TITULO);
                    //putchar_menu_overlay(x+ancho-5,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
                    //putchar_menu_overlay(x+ancho-4,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
                    //putchar_menu_overlay(x+ancho-3,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);


                    //5 espacios negro primero
                    int i;
                    for (i=margen;i>=margen-4;i--) putchar_menu_overlay(x+ancho-i,y,' ',ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_PAPEL_TITULO);
                    if (franjas==4) {
                        putchar_menu_overlay(indice_inicio_franjas+1,y,'/',cr[0],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+2,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
                    }

                     if (franjas==3) {
                        putchar_menu_overlay(indice_inicio_franjas+2,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
                    }


                    if (franjas==2) {
                        putchar_menu_overlay(indice_inicio_franjas+3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
                    }

                    if (franjas==1) {
                        putchar_menu_overlay(indice_inicio_franjas+4,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
                    }
            }
    }
}


void menu_dibuja_ventana_franja_arcoiris(int x, int y, int ancho)
{
    menu_dibuja_ventana_franja_arcoiris_trozo(x,y,ancho,4);
}

void menu_dibuja_ventana_franja_arcoiris_trozo_current(int trozos)
{

    menu_dibuja_ventana_franja_arcoiris_trozo(current_win_x,current_win_y,current_win_ancho,trozos);
}

void menu_dibuja_ventana_franja_arcoiris_oscuro_current(int indice)
{

    menu_dibuja_ventana_franja_arcoiris_oscuro(current_win_x,current_win_y,current_win_ancho,indice);
}

//Da ancho titulo de ventana segun el texto titulo, boton cerrado si/no, y franjas de color
int menu_da_ancho_titulo(char *titulo)
{
        int ancho_boton_cerrar=2;

        if (menu_hide_close_button.v) ancho_boton_cerrar=0;

        int ancho_franjas_color=MENU_ANCHO_FRANJAS_TITULO;

        if (!ESTILO_GUI_MUESTRA_RAINBOW) ancho_franjas_color=0;

        int margen_adicional=ZXVISION_TOTAL_BUTTONS_RIGHT; //1 para que no se pegue el titulo a la derecha, otro mas para el caracter de minimizar, otro mas para caracter maximizar

        int ancho_total=strlen(titulo)+ancho_boton_cerrar+ancho_franjas_color+margen_adicional; //+1 de margen, para que no se pegue el titulo

        if (zxvision_window_can_be_backgrounded(zxvision_current_window)) {
            //printf ("Sumamos 1\n");
            //sleep(5);
            ancho_total++;
        }

        return ancho_total;
}



int menu_dibuja_ventana_ret_ancho_titulo(int ancho,char *titulo)
{
    int ancho_mostrar_titulo=menu_da_ancho_titulo(titulo);

    int ancho_disponible_titulo=ancho;

    if (ancho_disponible_titulo<ancho_mostrar_titulo) ancho_mostrar_titulo=ancho_disponible_titulo;

    return ancho_mostrar_titulo;
}

z80_byte menu_retorna_caracter_minimizar(zxvision_window *w)
{

    z80_byte caracter_mostrar=ESTILO_GUI_BOTON_MINIMIZAR;

    //Si caracter es un udg especial y no es driver video completo, retornar por defecto
    if (caracter_mostrar>126 && !si_complete_video_driver()) caracter_mostrar='-';


    if (w->is_minimized) {
        caracter_mostrar=ESTILO_GUI_BOTON_RESTAURAR;

        //Si caracter es un udg especial y no es driver video completo, retornar por defecto
        if (caracter_mostrar>126 && !si_complete_video_driver()) caracter_mostrar='=';
    }

    return caracter_mostrar;
}

z80_byte menu_retorna_caracter_maximizar(zxvision_window *w)
{
    //z80_byte caracter_mostrar='+';

    z80_byte caracter_mostrar=ESTILO_GUI_BOTON_MAXIMIZAR;

    //Si caracter es un udg especial y no es driver video completo, retornar por defecto
    if (caracter_mostrar>126 && !si_complete_video_driver()) caracter_mostrar='+';

    if (w->is_maximized) {
        caracter_mostrar=ESTILO_GUI_BOTON_RESTAURAR;

        //Si caracter es un udg especial y no es driver video completo, retornar por defecto
        if (caracter_mostrar>126 && !si_complete_video_driver()) caracter_mostrar='=';
    }

    return caracter_mostrar;
}

int zxvision_window_can_be_backgrounded(zxvision_window *w)
{
    if (w==NULL) return 0;

    if (menu_allow_background_windows && w->can_be_backgrounded) return 1;
    else return 0;
}

//Retorna el caracter que indica que una ventana esta en background
char zxvision_get_character_backgrounded_window(void)
{

    //Si estamos redibujando con menu cerrado, para indicar que no se interactua directamente con esa ventana
    /*if (overlay_visible_when_menu_closed) return '/';

    else return menu_retorna_caracter_background();*/

    //En cualquiera de los dos casos (menu abierto o cerrado) mostrar mismo caracter
    return menu_retorna_caracter_background();
}

int zxvision_return_minimize_button_position(int ancho)
{
    //Para beos
    if (ESTILO_GUI_NO_RELLENAR_TITULO) {

        int position=current_win_minimize_button_position-1;

        //si dejar espacio para boton de background
        if (zxvision_current_window!=NULL) {   //por si acaso comprobar NULL
            if (zxvision_window_can_be_backgrounded(zxvision_current_window)) position++;
        }

        return position;
    }

    else return ancho-2;
}

int zxvision_return_maximize_button_position(int ancho)
{
    return zxvision_return_minimize_button_position(ancho)+1;
}

int zxvision_return_background_button_position(int ancho)
{
    //Para beos
    if (ESTILO_GUI_NO_RELLENAR_TITULO) return current_win_minimize_button_position-1;

    else return ancho-3;
}

//Dice si la ventana es la activa y se dibuja como tal, o sea,
//esta activa y esta en el menu abierto
//De momento solo se usa en Process Management para dibujar la linea de cursor desde overlay
int zxvision_window_is_active_drawn(zxvision_window *w)
{
    if (w==zxvision_current_window && ventana_tipo_activa) return 1;
    else return 0;
}

void menu_dibuja_ventana_boton_background(int x,int y,int ancho,zxvision_window *w)
{


            //Boton de background
            if (zxvision_window_can_be_backgrounded(w)) {
                if (ventana_tipo_activa) {
                    //Boton de background, con ventana activa
                    putchar_menu_overlay(x+zxvision_return_background_button_position(ancho),y,zxvision_get_character_backgrounded_window(),ESTILO_GUI_TINTA_TITULO,ESTILO_GUI_PAPEL_TITULO);
                }

                else {
                    //ventana inactiva. mostrar "!" con parpadeo
                    if (menu_hide_background_button_on_inactive.v==0) {
                        //printf ("ventana inactiva\n");
                        if (w->overlay_function!=NULL) {
                            //printf ("boton background\n");
                            //zxvision_print_char_simple(zxvision_current_window,ancho-2,0,ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_TINTA_TITULO,1,'!');
                            putchar_menu_overlay_parpadeo(x+zxvision_return_background_button_position(ancho),y,zxvision_get_character_backgrounded_window(),ESTILO_GUI_TINTA_TITULO_INACTIVA,ESTILO_GUI_PAPEL_TITULO_INACTIVA,1);
                        }
                    }
                }

            }
}



void menu_dibuja_ventana_botones(void)
{

    int x=current_win_x;
    int y=current_win_y;
    int ancho=current_win_ancho;
    //int alto=ventana_alto;


        //if (ventana_activa_tipo_zxvision) {

            //Boton de minimizar y maximizar
            if (ventana_tipo_activa) {
                if (cuadrado_activo_resize) {

                    //Maximizar
                    z80_byte caracter_maximizar=menu_retorna_caracter_maximizar(zxvision_current_window);
                    if (menu_hide_maximize_button.v) caracter_maximizar=menu_retorna_caracter_espacio_titulo();
                    putchar_menu_overlay(x+zxvision_return_maximize_button_position(ancho),y,caracter_maximizar,ESTILO_GUI_TINTA_TITULO,ESTILO_GUI_PAPEL_TITULO);

                }

                //if (ventana_activa_puede_minimizar) {
                    //Minimizar
                    z80_byte caracter_minimizar=menu_retorna_caracter_minimizar(zxvision_current_window);
                    if (menu_hide_minimize_button.v || !ventana_activa_puede_minimizar) caracter_minimizar=menu_retorna_caracter_espacio_titulo();
                    //Si no mostrar, meter solo espacio. es importante esto, si no hay boton, y no escribieramos espacio,
                    //se veria el texto de titulo en caso de que ancho de ventana la hagamos pequeña
                    //Tambien, necesario en Beos, porque si no, las ventanas (y menu) que no permiten minimizar, tendrian
                    //un hueco entre el titulo y el boton de maximizar de la derecha

                    putchar_menu_overlay(x+zxvision_return_minimize_button_position(ancho),y,caracter_minimizar,ESTILO_GUI_TINTA_TITULO,ESTILO_GUI_PAPEL_TITULO);

                //}

            }

            menu_dibuja_ventana_boton_background(x,y,ancho,zxvision_current_window);


        //}


        //putchar_menu_overlay(x+ancho-1,y,'-',ESTILO_GUI_TINTA_TITULO,ESTILO_GUI_PAPEL_TITULO);
}

//si no mostramos mensajes de error pendientes
int no_dibuja_ventana_muestra_pending_error_message=0;

//Retorna caracter cerrar
z80_byte menu_retorna_caracter_cerrar(void)
{

    z80_byte caracter=ESTILO_GUI_BOTON_CERRAR;

    //Si caracter es un udg especial y no es driver video completo, retornar por defecto
    if (caracter>126 && !si_complete_video_driver()) return '*';

    else return caracter;
}


//Retorna caracter de espacio de titulo
z80_byte menu_retorna_caracter_espacio_titulo(void)
{

    z80_byte caracter=ESTILO_GUI_CARACTER_ESPACIO_TITULO;

    //Si caracter es un udg especial y no es driver video completo, retornar por defecto
    if (caracter>126 && !si_complete_video_driver()) return ' ';

    else return caracter;
}

//Retorna caracter background
z80_byte menu_retorna_caracter_background(void)
{

    z80_byte caracter=ESTILO_GUI_BOTON_BACKGROUND;

    //Si caracter es un udg especial y no es driver video completo, retornar por defecto
    if (caracter>126 && !si_complete_video_driver()) return '!';

    else return caracter;
}



//dibuja ventana de menu, con:
//titulo
//contenido blanco
//recuadro de lineas
//Entrada: x,y posicion inicial. ancho, alto. Todo coordenadas en caracteres 0..31 y 0..23
void menu_dibuja_ventana(int x,int y,int ancho,int alto,char *titulo_original_utf)
{

    //Para draw below windows, no mostrar error pendiente cuando esta dibujando ventanas de debajo
    //Ni cuando estamos restaurando ventanas en startup (si no se hiciera y hay error pendiente al restaurar ventanas,
    //provoca que se quede el emulador aqui medio colgado y no habilita zxdesktop ni funciona correctamente)
    if (!no_dibuja_ventana_muestra_pending_error_message && !zxvision_currently_restoring_windows_on_start) menu_muestra_pending_error_message();

    //En el caso de stdout, solo escribimos el texto
        if (menu_es_stdout()) {
                printf ("%s\n",titulo_original_utf);
        scrstdout_menu_print_speech_macro(titulo_original_utf);
        printf ("------------------------\n\n");
        //paso de curses a stdout deja stdout que no hace flush nunca. forzar
        fflush(stdout);
                return;
        }

    //Convertir de cadena utf con posibles acentos a caracteres internos
    char titulo_original[ZXVISION_MAX_WINDOW_TITLE];
    util_convert_utf_charset(titulo_original_utf,(z80_byte *)titulo_original,strlen(titulo_original_utf));


    //Pasar titulo a string temporal. Agregamos un espacio al final en estilos que no rellenan toda la barra de titulo (como BeOS)
    char titulo[ZXVISION_MAX_WINDOW_TITLE];

    sprintf(titulo,"%s%s",titulo_original,(ESTILO_GUI_NO_RELLENAR_TITULO ? " " : "")  );

    //printf ("valor menu_speech_tecla_pulsada: %d\n",menu_speech_tecla_pulsada);

    //valores en pixeles
    int xpixel,ypixel,anchopixel,altopixel;
    int i,j;

    //guardamos valores globales de la ventana mostrada
    current_win_x=x;
    current_win_y=y;
    current_win_ancho=ancho;
    current_win_alto=alto;

    xpixel=x*menu_char_width;
    ypixel=(y+1)*menu_char_height; //La barra de titulo no tendra linea como tal
    anchopixel=ancho*menu_char_width;
    altopixel=alto*menu_char_height;

    int xderecha=xpixel+anchopixel-1;
    //printf ("x derecha: %d\n",xderecha);

    //if (menu_char_width!=8) xderecha++; //?????

    //contenido en blanco normalmente en estilo ZEsarUX
    //Sin usar cache
    //Para evitar por ejemplo que ventanas como daad graphics, que aparecen encima de otra ventana tipo visualmem o audio chip piano con pixeles,
    //que esos pixeles no se metan "dentro" de la ventana de daad graphics
    for (i=0;i<alto-1;i++) {
        for (j=0;j<ancho;j++) {
            putchar_menu_overlay_parpadeo_cache_or_not(x+j,y+i+1,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,0);
        }
    }

    menu_establece_cuadrado(xpixel,ypixel,xderecha,ypixel+altopixel-1-menu_char_height,ESTILO_GUI_COLOR_RECUADRO);


    int color_tinta_titulo;
    int color_papel_titulo;


    if (ventana_tipo_activa) {
        color_tinta_titulo=ESTILO_GUI_TINTA_TITULO;
        color_papel_titulo=ESTILO_GUI_PAPEL_TITULO;
    }

    else {
        color_tinta_titulo=ESTILO_GUI_TINTA_TITULO_INACTIVA;
        color_papel_titulo=ESTILO_GUI_PAPEL_TITULO_INACTIVA;
    }

    z80_byte caracter_espacio_titulo=menu_retorna_caracter_espacio_titulo();

    //si ventana es background o no es ventana activa, caracter fondo titulo es siempre espacio
    if (ventana_es_background || !ventana_tipo_activa) caracter_espacio_titulo=' ';

    //titulo
    //primero franja toda negra normalmente en estilo ZEsarUX
    for (i=0;i<ancho;i++) {
        if (ESTILO_GUI_NO_RELLENAR_TITULO) {
            //Caso del estilo BeOS, si estan los botones de antes, al irse la ventana a background,
            //hay que borrar botones
            deletechar_menu_overlay(x+i,y);
        }
        else {
            putchar_menu_overlay(x+i,y,caracter_espacio_titulo,color_tinta_titulo,color_papel_titulo);
        }
    }


    int ancho_mostrar_titulo=menu_dibuja_ventana_ret_ancho_titulo(ancho,titulo);

    char titulo_mostrar[ZXVISION_MAX_WINDOW_TITLE];
    z80_byte caracter_cerrar=menu_retorna_caracter_cerrar();



    if (menu_hide_close_button.v || ventana_es_background || !ventana_tipo_activa) {
        //strcpy(titulo_mostrar,titulo);
        //Ancho del titulo sera igual, aun sin el boton de cerrar
        sprintf (titulo_mostrar,"%c%c%s",caracter_espacio_titulo,caracter_espacio_titulo,titulo);
    }
    else {
        sprintf (titulo_mostrar,"%c%c%s",caracter_cerrar,caracter_espacio_titulo,titulo);
    }


    //y luego el texto. titulo mostrar solo lo que cabe de ancho


    //Boton de cerrado



        for (i=0;i<ancho_mostrar_titulo && titulo_mostrar[i];i++) {
            char caracter_mostrar=titulo_mostrar[i];

            putchar_menu_overlay(x+i,y,caracter_mostrar,color_tinta_titulo,color_papel_titulo);
        }

        //Indicar posicion del boton minimizar
        //A la derecha de este hay otro mas (el de maximizar)
        current_win_minimize_button_position=i+1;

        if (current_win_minimize_button_position+1>=ancho) current_win_minimize_button_position=ancho-2;



        //y las franjas de color
    if (ESTILO_GUI_MUESTRA_RAINBOW && ventana_tipo_activa) {
        //en el caso de drivers completos, hacerlo real
        menu_dibuja_ventana_franja_arcoiris(x,y,ancho);

    }

        menu_dibuja_ventana_botones();

        //zxvision_sound_event_new_window();

        char buffer_titulo[100];
        sprintf (buffer_titulo,"Window: %s",titulo);
        menu_textspeech_send_text(buffer_titulo);

        //Forzar que siempre suene
        //menu_speech_reset_tecla_pulsada();



}

int last_mouse_x,last_mouse_y;
int mouse_movido=0;

//Coordenadas dentro de la ventana activa, en caracter
int menu_mouse_x=0;
int menu_mouse_y=0;

//Coordenadas dentro de toda la ventana de ZEsarUX, en caracter
int zxvision_mouse_x=0;
int zxvision_mouse_y=0;

zxvision_window *zxvision_current_window=NULL;


//Para cargar las ventanas al inicio, las va rellenando al leer los parámetros en arranque
char restore_window_array[MAX_RESTORE_WINDOWS_START][MAX_NAME_WINDOW_GEOMETRY];

int total_restore_window_array_elements=0;

//Ventanas conocidas y sus funciones que las inicializan y sus iconos. Usado al restaurar ventanas al inicio
//Y tambien al reabrir todas ventanas en el cambio de estilo de GUI style
//La ultima siempre finaliza con funcion NULL
//Conviene tener la lista ordenada alfabeticamente por el nombre corto asi al listarlo desde new icon->open window,saldra ordenado
//El nombre largo ("nombre") no conviene cambiarlo porque si en la config hay guardada geometria de esa ventana y/o restaurar esa ventana,
//no la reconocera con el nombre anterior
//El nombre corto ("nombre_corto") se puede cambiar sin afectar a nada
zxvision_known_window_names zxvision_known_window_names_array[]={
    {"textadvlocimage",     "adv locimg",   menu_textadv_loc_image,bitmap_button_ext_desktop_textadvlocimage},
    {"asciitable",          "asciitable",   menu_ascii_table,bitmap_button_ext_desktop_asciitable},
    {"aypiano",             "audiopiano",menu_ay_pianokeyboard,bitmap_button_ext_desktop_audiopiano},
    {"ayregisters",         "audio regs",menu_ay_registers,bitmap_button_ext_desktop_audioregisters},
    {"aysheet",             "audiosheet",menu_ay_partitura,bitmap_button_ext_desktop_audiosheet},
    {"ayplayer",            "ay player",menu_audio_new_ayplayer,bitmap_button_ext_desktop_ayplayer},
    {"convertaudiotozx81",  "cnv aud 81",   menu_convert_audio_to_zx81,bitmap_lowericon_ext_desktop_audio_to_zx81_convert},
    {"corestatistics",      "core stats",   menu_about_core_statistics,bitmap_button_ext_desktop_corestatistics},
#ifdef EMULATE_CPU_STATS
    {"cpucompactstatistics","cpucmpstat",menu_debug_cpu_resumen_stats,bitmap_button_ext_desktop_cpustatistics},
#endif
    {"debugconsole",        "debug cons",   menu_debug_unnamed_console,bitmap_button_ext_desktop_debugconsole},
    {"debugcpu",            "debug cpu",menu_debug_registers,bitmap_button_ext_desktop_debugcpu},
    {"debugdma",            "debug dma",menu_debug_dma,bitmap_button_ext_desktop_debugdma},
    {"debugioports",        "debug i/o",    menu_debug_ioports,bitmap_button_ext_desktop_ioports},
    {"displaypalettes",     "dis palett",menu_display_total_palette,bitmap_button_ext_desktop_colour_palettes},
    {"externalaudiosource", "ext audio",    menu_realtape_record_input,bitmap_button_ext_desktop_record_input},
    {"audiogensound",       "gen sound",    menu_audio_general_sound,bitmap_button_ext_desktop_geneneralsoundregisters},
    {"helpshowkeyboard",    "helpkeybrd",   menu_help_show_keyboard,bitmap_button_ext_desktop_helpkeyboard},
    {"hexeditor",           "hex editor",   menu_debug_hexdump,bitmap_button_ext_desktop_hexeditor},
    {"hilowconvertaudio",   "hlw cnv au",   menu_hilow_convert_audio,bitmap_lowericon_ext_desktop_hilow_convert},
    {"hilowvisualdatadrive",     "vsdatadrv",    menu_hilow_visual_datadrive,bitmap_button_ext_desktop_hilow_visual_datadrive},
    {"memorycheat",         "memcheat",     menu_memory_cheat,bitmap_button_ext_desktop_memorycheat},
    {"microdriverawmap",    "mdv rawmap",  menu_microdrive_raw_map,bitmap_button_ext_desktop_mdvrawmap},
    {"networktraffic",      "nettraffic",   menu_network_traffic,bitmap_button_ext_desktop_networktrafiic},
    {"processmanagement",   "prc mngmnt",   menu_display_window_list,bitmap_button_ext_desktop_processmanagement},
    {"processswitcher",     "proc swtch",   menu_process_switcher,bitmap_button_ext_desktop_processswitcher},
    {"viewsensors",         "sensors",      menu_debug_view_sensors,bitmap_button_ext_desktop_view_sensors},
    {"shortcutshelper",     "shrtcuthlp",   menu_shortcuts_window,bitmap_button_ext_desktop_shortcutshelper},
    {"inspectrumanalyzer",  "spec anlys",   menu_input_spectrum_analyzer,bitmap_button_ext_desktop_inspectrumanalyzer},
    {"specnextaudiodac",    "next DAC",menu_specnext_audio_dac,bitmap_button_ext_desktop_nextdac},
    {"sprites",             "sprites",menu_debug_view_sprites,bitmap_button_ext_desktop_viewsprites},
    {"tsconftbbluespritenav","sprite nav",menu_debug_tsconf_tbblue_msx_spritenav,bitmap_button_ext_desktop_tsconftbbluespritenav},
    {"textadvmap",          "textadvmap",   menu_debug_textadventure_map_connections,bitmap_button_ext_desktop_text_adventure_map},
    {"tsconftbbluetilenav", "tile nav",menu_debug_tsconf_tbblue_msx_tilenav,bitmap_button_ext_desktop_tsconftbbluetilenav},
    {"videoinfo",           "videoinfo",menu_debug_tsconf_tbblue_msx_videoregisters,bitmap_button_ext_desktop_videoinfo},
    {"videolayers",         "videolayer",   menu_video_layers,bitmap_button_ext_desktop_videolayers},
    {"videooutput",         "video outp",   menu_video_output,bitmap_button_ext_desktop_videooutput},
    {"viewbasiclisting",    "view basic",   menu_view_basic_listing,bitmap_button_ext_desktop_view_basic},
    {"viewbasicvariables",  "view varia",   menu_view_basic_variables,bitmap_button_ext_desktop_view_basic_variables},
    {"viewgosubstack",      "view gosub",   menu_view_gosub_stack,bitmap_button_ext_desktop_view_gosub_stack},
    {"visualcassettetape",  "viscastap",    menu_visual_cassette_tape,bitmap_button_ext_desktop_visual_cassete_tape},
    {"visualfloppy",        "visual flp",   menu_visual_floppy,bitmap_button_ext_desktop_visualfloppy},
#ifdef EMULATE_VISUALMEM
    {"visualmem",           "visualmem",menu_debug_new_visualmem,bitmap_button_ext_desktop_visualmem},
#endif
    {"visualmicrodrive",    "vsmicrodrive",   menu_visual_microdrive,bitmap_button_ext_desktop_visualmicrodrive},
    {"visualrealtape",      "vsrealtape",   menu_visual_realtape,bitmap_button_ext_desktop_visualrealtape},
    {"watches",             "watches",menu_watches,bitmap_button_ext_desktop_watches},
    {"waveform",            "waveform",menu_audio_new_waveform,bitmap_button_ext_desktop_waveform},
    {"wavepiano",           "wave piano",menu_beeper_pianokeyboard,bitmap_button_ext_desktop_wavepiano},
    {"toyzxeyes",           "zx eyes",      menu_toy_follow_mouse,bitmap_button_ext_desktop_zxeyes},
    {"zengonlinestatus",    "zengonlinest",      menu_zeng_online_status_window,bitmap_button_ext_desktop_zengonline_status},
    {"zxlife",              "zx life",      menu_toys_zxlife,bitmap_button_ext_desktop_zxlife},

    {"","",NULL,bitmap_button_ext_desktop_userdefined} //NO BORRAR ESTA!!
};



//Decir que con una ventana zxvision visible, las pulsaciones de teclas no se envian a maquina emulada
int zxvision_keys_event_not_send_to_machine=1;

//indicar que estamos restaurando ventanas y por tanto las funciones que las crean tienen que volver nada mas entrar
int zxvision_currently_restoring_windows_on_start=0;

//Retorna posicion a indice de zxvision_known_window_names_array si se encuentra
//-1 si no
int zxvision_find_known_window(char *nombre)
{
    int i;


    for (i=0;zxvision_known_window_names_array[i].start!=NULL;i++) {

         if (!strcasecmp(zxvision_known_window_names_array[i].nombre,nombre)) return i;

    }
    return -1;
}

//Retorna posicion a indice de zxvision_known_window_names_array si es valido
int zxvision_known_window_is_valid_by_index(int indice)
{
    int i;

    //Nos posicionamos en el indicado
    for (i=0;i<indice && zxvision_known_window_names_array[i].start!=NULL;i++) {
    }

    if (zxvision_known_window_names_array[i].start!=NULL) {
        return i;
    }

    return -1;
}

//Retorna bitmap de zxvision_known_window_names_array si se encuentra
//NULL si no
char **zxvision_find_icon_for_known_window(char *nombre)
{
    int i;


    for (i=0;zxvision_known_window_names_array[i].start!=NULL;i++) {

         if (!strcasecmp(zxvision_known_window_names_array[i].nombre,nombre)) {
            char **p=zxvision_known_window_names_array[i].bitmap_button;
            p=alter_zesarux_ascii_logo(p);

            return p;
         }

    }
    return NULL;
}

//Funcion en casos de debug. Agregar todas las ventanas para restaurar
void zxvision_add_all_windows_to_restore(void)
{
    int i;


    for (i=0;zxvision_known_window_names_array[i].start!=NULL;i++) {

        add_window_to_restore(zxvision_known_window_names_array[i].nombre);

    }

}


//Decir que hay que abrir menu con una ventana concreta
void zxvision_open_menu_with_window(char *geometry_name)
{
    zxvision_switch_to_window_on_open_menu=1;
    strcpy(zxvision_switch_to_window_on_open_menu_name,geometry_name);
    menu_event_open_menu.v=1;
    menu_abierto=1;
}

void zxvision_restore_windows_on_startup_aux_launch_window(int indice)
{
    //Lanzar funcion que la crea
    zxvision_known_window_names_array[indice].start(0);

    //Antes de restaurar funcion overlay, guardarla en estructura ventana, por si nos vamos a background,
    //siempre que no sea la de normal overlay o null
    //zxvision_set_window_overlay_from_current(zxvision_current_window);

    //restauramos modo normal de texto de menu
    //set_menu_overlay_function(normal_overlay_texto_menu);

    /*
    Esa ventana ya viene de background por tanto no hay que guardar nada en la ventana.,
    es más, si estamos aquí es que se ha salido de la ventana con escape (y la current window ya no estará)
    o con f6. Total que no hay que guardar nada.
    Pero si que conviene dejar el overlay como estaba antes
    */
}

//Restaurar una sola ventana. Funcion utilizada en otros sitios y no desde la restauracion en startup
//De momento no usada


void zxvision_restore_one_window(char *ventana_a_restaurar)
{
    if (!menu_allow_background_windows) return;

    //Si no hay multitask, no restaurar, porque esto puede implicar que se abran ventanas que necesitan multitask,
    //y se quejen con "This window needs multitask enabled", y ese mensaje no se ve el error, y espera una tecla
    if (!menu_multitarea) return;

    //indicar que estamos restaurando ventanas y por tanto las funciones que las crean tienen que volver nada mas entrar
    zxvision_currently_restoring_windows_on_start=1;


    menu_speech_set_tecla_pulsada(); //Si no, envia continuamente los textos de las ventanas a speech


    //Guardar valores funciones anteriores
    int antes_menu_overlay_activo=menu_overlay_activo;


    //printf ("Restoring window %s\n",ventana_a_restaurar);

    int indice=zxvision_find_known_window(ventana_a_restaurar);

    if (indice<0) {
        //Si hay error con alguna ventana desconocida, nos guardamos el error para el final
        //si no, en medio de la restauracion salta esta ventana y ademas no se redimensiona a zxdesktop
        //error_restoring_window=1;
        //error_restoring_window_index=i;
        //debug_printf (VERBOSE_ERR,"Unknown window to restore: %s",restore_window_array[i]);
    }

    else {
        //Lanzar funcion que la crea
        zxvision_restore_windows_on_startup_aux_launch_window(indice);
    }



    zxvision_currently_restoring_windows_on_start=0;

    //Si antes no estaba activo, ponerlo a 0. El cambio a normal_overlay_texto_menu que se hace en el bucle
    //no lo desactiva
    //Si no hicieramos esto, al restaurar ventanas y, no siempre, se quedan las ventanas abiertas,
    //sin dibujar el contenido, pero con los marcos y titulo visible, aunque el menu está cerrado
    //quiza sucede cuando la maquina al arrancar es tsconf o cualquier otra que no tiene el tamaño de ventana standard de spectrum
    if (!antes_menu_overlay_activo) {
        menu_overlay_activo=0;
    }

}

//abrir tal cual la ventana identificada por el nombre
//no hace mas magia, ni abre menu, ni nada... solo busca la ventana y la abre
void zxvision_open_window_by_name(char *nombre)
{
    int indice=zxvision_find_known_window(nombre);

    if (indice>=0) {
        zxvision_known_window_names_array[indice].start(0);
    }
}

void zxvision_restore_windows_on_startup(void)
{
    if (!menu_allow_background_windows) return;

    if (menu_reopen_background_windows_on_start.v==0) return;

    //Si no hay multitask, no restaurar, porque esto puede implicar que se abran ventanas que necesitan multitask,
    //y se quejen con "This window needs multitask enabled", y ese mensaje no se ve el error, y espera una tecla
    if (!menu_multitarea) return;

    //indicar que estamos restaurando ventanas y por tanto las funciones que las crean tienen que volver nada mas entrar
    zxvision_currently_restoring_windows_on_start=1;

    //Iterar sobre todas
    int i;

    menu_speech_set_tecla_pulsada(); //Si no, envia continuamente los textos de las ventanas a speech

    //Si ha habido un error y mostramos al final
    int error_restoring_window=0;
    int error_restoring_window_index;

    //Guardar valores funciones anteriores
    int antes_menu_overlay_activo=menu_overlay_activo;

    //printf("zxvision_restore_windows_on_startup: menu_overlay_activo: %d\n",menu_overlay_activo);


    for (i=0;i<total_restore_window_array_elements;i++) {
        //printf ("Restoring window %s\n",restore_window_array[i]);

        int indice=zxvision_find_known_window(restore_window_array[i]);

        if (indice<0) {
            //Si hay error con alguna ventana desconocida, nos guardamos el error para el final
            //si no, en medio de la restauracion salta esta ventana y ademas no se redimensiona a zxdesktop
            error_restoring_window=1;
            error_restoring_window_index=i;
            //debug_printf (VERBOSE_ERR,"Unknown window to restore: %s",restore_window_array[i]);
        }

        else {
            //Lanzar funcion que la crea
            zxvision_restore_windows_on_startup_aux_launch_window(indice);
        }

    }

    zxvision_currently_restoring_windows_on_start=0;

    if (error_restoring_window) {
        debug_printf (VERBOSE_ERR,"Unknown window to restore: %s",restore_window_array[error_restoring_window_index]);
    }

    //printf ("End restoring windows\n");

    //Si antes no estaba activo, ponerlo a 0. El cambio a normal_overlay_texto_menu que se hace en el bucle
    //no lo desactiva
    //Si no hicieramos esto, al restaurar ventanas y, no siempre, se quedan las ventanas abiertas,
    //sin dibujar el contenido, pero con los marcos y titulo visible, aunque el menu está cerrado
    //quiza sucede cuando la maquina al arrancar es tsconf o cualquier otra que no tiene el tamaño de ventana standard de spectrum
    if (!antes_menu_overlay_activo) {
        menu_overlay_activo=0;
    }

    //Lo siguiente ya se enviara a speech
    menu_speech_reset_tecla_pulsada();

}

void zxvision_restart_all_background_windows(void)
{
    if (!menu_allow_background_windows) return;


    //Si no hay multitask, no restaurar, porque esto puede implicar que se abran ventanas que necesitan multitask,
    //y se quejen con "This window needs multitask enabled", y ese mensaje no se ve el error, y espera una tecla
    if (!menu_multitarea) return;

    if (zxvision_current_window==NULL) return;

    //indicar que estamos restaurando ventanas y por tanto las funciones que las crean tienen que volver nada mas entrar
    zxvision_currently_restoring_windows_on_start=1;



    menu_speech_set_tecla_pulsada(); //Si no, envia continuamente los textos de las ventanas a speech


    //Guardar valores funciones anteriores
    int antes_menu_overlay_activo=menu_overlay_activo;


    //Primero ir a buscar la de abajo del todo
    zxvision_window *pointer_window;

    pointer_window=zxvision_find_first_window_below_this(zxvision_current_window);

    zxvision_window *initial_current_window=zxvision_current_window;

    //Y ahora de ahi hacia arriba
    int salir=0;
    do {

        //Hay que ir con ojo con los punteros a ventana. Dado que lo que haremos sera reseguir las ventanas,
        //empezando desde la de mas abajo, antes de reabrir una, guardamos el puntero a la siguiente,
        //reabrimos y vamos a la siguiente

        //Y finalizamos cuando la que hemos redibujado era la misma que la inicial
        /*
        Ejemplo tenemos ventanas:
        A
        B
        C
        D
        Donde la actual es la A

        La primera que encontramos abajo es la D, Al reabrirla tendremos:

        D
        A
        B
        C

        La siguiente la C

        C
        D
        A
        B

        La siguiente la B
        B
        C
        D
        A

        Y la siguiente la A
        A
        B
        C
        D

        Y entonces es cuando verá que la última que hemos reabierto, la A, era la inicial A, y finaliza

        */

        //printf("Puntero ventana: %p\n",pointer_window);
        if (pointer_window==NULL) {
            debug_printf(VERBOSE_DEBUG,"Window is null. Exiting");
            salir=1;
        }
        else {
            //printf("ventana: %s\n",pointer_window->geometry_name);

            //Obtenemos la siguiente ventana antes de borrar la actual
            zxvision_window *next_window=pointer_window->next_window;

            if (pointer_window->can_be_backgrounded) {
                //Mirar su nombre de geometria
                char *nombre;

                nombre=pointer_window->geometry_name;
                if (nombre[0]!=0) {
                    //debug_printf(VERBOSE_DEBUG,"Closing and opening window %s",nombre);

                    int indice=zxvision_find_known_window(nombre);

                    if (indice>=0) {
                        //Lanzar funcion que la crea
                        //printf("Relanzando ventana %s (indice %d)\n",nombre,indice);
                        debug_printf(VERBOSE_DEBUG,"Closing and reopening window %s",nombre);

                        //Hay que borrar la ventana de esta manera para que se recree de nuevo con el color de fondo correspondiente
                        //Nota: no se usa destroy_window porque esta designada para destruir la ventana current
                        //en cambio, zxvision_delete_window_if_exists, borra la ventana independientemente de si es la current, la del medio, la de abajo...
                        zxvision_delete_window_if_exists(pointer_window);
                        zxvision_known_window_names_array[indice].start(0);

                    }
                    else {
                        //printf("Window %s not found\n",nombre);
                    }
                }
            }

            //Si la que hemos redibujado era la inicial, salimos
            if (pointer_window==initial_current_window) {
                debug_printf(VERBOSE_DEBUG,"Redrawn window was the last. Exiting");
                salir=1;
            }

            else {
                pointer_window=next_window;
            }

        }


    } while(!salir);



    zxvision_currently_restoring_windows_on_start=0;



    //printf ("End restoring windows\n");

    //Si antes no estaba activo, ponerlo a 0. El cambio a normal_overlay_texto_menu que se hace en el bucle
    //no lo desactiva
    //Si no hicieramos esto, al restaurar ventanas y, no siempre, se quedan las ventanas abiertas,
    //sin dibujar el contenido, pero con los marcos y titulo visible, aunque el menu está cerrado
    //quiza sucede cuando la maquina al arrancar es tsconf o cualquier otra que no tiene el tamaño de ventana standard de spectrum
    if (!antes_menu_overlay_activo) {
        menu_overlay_activo=0;
    }

}

//Cambia el color del caracter espacio en una ventana dada
void zxvision_change_space_colour(zxvision_window *w,int papel)
{

    int total_width=w->total_width;
    int total_height=w->total_height;

    int buffer_size=total_width*total_height;


    int i;
    overlay_screen *p;
    p=w->memory;

    for (i=0;i<buffer_size;i++) {
        if (p->caracter==' ') {

            p->papel=papel;

        }

        p++;
    }
}



//Funcion para reaplicar colores en las ventanas existentes
//De momento solo se usa por la funcion de cambio de estilo de gui, mientras se mueve el cursor, y sin tener
//que recrear todas las ventanas, se cambian los colores
void zxvision_reapply_style_colours_all_windows(void)
{

    if (zxvision_current_window==NULL) return;

    //Primero ir a buscar la de abajo del todo
    zxvision_window *pointer_window;


    pointer_window=zxvision_find_first_window_below_this(zxvision_current_window);


    while (pointer_window!=zxvision_current_window && pointer_window!=NULL) {


        pointer_window->dirty_must_draw_contents=1;
        pointer_window->dirty_user_must_draw_contents=1;
        pointer_window->default_paper=ESTILO_GUI_PAPEL_NORMAL;

        //Cambiar color caracter espacio
        zxvision_change_space_colour(pointer_window,ESTILO_GUI_PAPEL_NORMAL);

        //El resto de colores se refrescaran por si solos cuando el overlay de cada ventana diga que hay que refrescar

        pointer_window=pointer_window->next_window;
    }


}




void zxvision_set_draw_window_parameters(zxvision_window *w)
{
    //ventana_activa_tipo_zxvision=1;

    cuadrado_activo_resize=w->can_be_resized;
    ventana_activa_puede_minimizar=w->can_be_minimized;

}

void zxvision_draw_below_windows_nospeech(zxvision_window *w)
{
    //Redibujar las de debajo
    //printf ("antes draw below\n");

    int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
    //No enviar a speech las ventanas por debajo
    menu_speech_set_tecla_pulsada(); //Si no, envia continuamente todo ese texto a speech

    zxvision_draw_below_windows(w);

        /*if (menu_speech_tecla_pulsada && !antes_menu_speech_tecla_pulsada) {
            printf("Pasar de 1 a 0 desde \n");
            debug_exec_show_backtrace();
        }*/

    menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;
    //printf ("despues draw below\n");
}

//Ver si posicion y tamaño ventana admitidos, en x
//0: no admitido
//no 0: admitido
int zxvision_new_window_check_range_aux_x(int x,int visible_width)
{

    if (zxvision_allow_windows_beyond_limit.v) {
        //Permitir ventanas que salgan del ZX Desktop
        //Minimo 2x2
        visible_width=2;
    }


    if (

     (x<0               || x>ZXVISION_MAX_X_VENTANA) ||


     //Rangos estaticos de ventana
     (visible_width<=0) ||


    //Rangos de final de ventana.
     (x+visible_width>ZXVISION_MAX_X_VENTANA+1)


    )
    {
        return 0;

    }

    else return 1;
}

//Ver si posicion y tamaño ventana admitidos, en y
//0: no admitido
//no 0: admitido
int zxvision_new_window_check_range_aux_y(int y,int visible_height)
{

    if (zxvision_allow_windows_beyond_limit.v) {
        //Permitir ventanas que salgan del ZX Desktop
        //Minimo 2x2
        visible_height=2;
    }


    if (


     (y<0               || y>ZXVISION_MAX_Y_VENTANA) ||

     //Rangos estaticos de ventana

     (visible_height<=0) ||

    //Rangos de final de ventana.

     (y+visible_height>ZXVISION_MAX_Y_VENTANA+1)

    )
    {
        return 0;

    }

    else return 1;
}

//Controlar rangos excepto tamaño ventana en estatico
void zxvision_new_window_check_range(int *x,int *y,int *visible_width,int *visible_height)
{

    //Controlar rangos. Cualquier valor que se salga de rango, hacemos ventana maximo 32x24

    //Rango xy es el total de ventana. Rango ancho y alto es 32x24, aunque luego se pueda hacer mas grande

    //Controlamos cada parametro por separado, para cambiar lo minimo
    //TODO: esto se podria hacer aun mejor, usando dos funciones aux para x, y otras dos para y, y mirando si
    //no es valida la funcion solo por posicion x (y por tanto cambiariamos la x) o si tambien es culpa de que la x + ancho
    //esta fuera de pantalla (con lo que habria que cambiar x primero, y luego ancho si conviene). Y lo mismo para y, alto
    if (!zxvision_new_window_check_range_aux_x(*x,*visible_width)) {

        debug_printf (VERBOSE_INFO,"zxvision_new_window: window out of range: %d,%d %dx%d. Adjusting x position",*x,*y,*visible_width,*visible_height);
        *x=0;
    }

    if (!zxvision_new_window_check_range_aux_x(*x,*visible_width)) {

        debug_printf (VERBOSE_INFO,"zxvision_new_window: window out of range: %d,%d %dx%d. Adjusting visible width",*x,*y,*visible_width,*visible_height);
        *visible_width=ZXVISION_MAX_ANCHO_VENTANA;

    }

    if (!zxvision_new_window_check_range_aux_y(*y,*visible_height)) {

        debug_printf (VERBOSE_INFO,"zxvision_new_window: window out of range: %d,%d %dx%d. Adjusting y position",*x,*y,*visible_width,*visible_height);
        *y=0;
    }


    if (!zxvision_new_window_check_range_aux_y(*y,*visible_height)) {

        debug_printf (VERBOSE_INFO,"zxvision_new_window: window out of range: %d,%d %dx%d. Adjusting visible height",*x,*y,*visible_width,*visible_height);
        *visible_height=ZXVISION_MAX_ALTO_VENTANA;

    }
}

//Comprobar que el alto y ancho no pase de un fijo estatico
//para tener ventanas que normalmente no excedan ese limite al crearse
//Nota: menu_filesel no hace este check
void zxvision_new_window_check_static_size_range(int *x,int *y,int *visible_width,int *visible_height)
{
    //printf("sizes %d %d - %d %d\n",*visible_width,*visible_height,ZXVISION_MAX_ANCHO_VENTANA,ZXVISION_MAX_ALTO_VENTANA);

    if (


     //Rangos estaticos de ancho ventana
     (*visible_width>ZXVISION_MAX_ANCHO_VENTANA) ||
     (*visible_height>ZXVISION_MAX_ALTO_VENTANA)


    )
        {
                debug_printf (VERBOSE_INFO,"zxvision_new_window: window out of range: %d,%d %dx%d",*x,*y,*visible_width,*visible_height);
                //printf ("zxvision_new_window: window out of range: %d,%d %dx%d\n",*x,*y,*visible_width,*visible_height);
                *x=0;
                *y=0;
                *visible_width=ZXVISION_MAX_ANCHO_VENTANA;
                *visible_height=ZXVISION_MAX_ALTO_VENTANA;

        }
}


//Buscar la ventana mas antigua debajo de esta en cascada
zxvision_window *zxvision_find_first_window_below_this(zxvision_window *w)
{
        //Primero ir a buscar la de abajo del todo
    zxvision_window *pointer_window;

    //printf ("original window: %p\n",w);
    //printf ("title pointer: %p\n",w->window_title);
    //printf ("title contents: %02XH\n",w->window_title[0]);
    //printf ("Title: %s\n",w->window_title);

    //printf ("after original window\n");



    pointer_window=w;

    while (pointer_window->previous_window!=NULL) {
        //printf ("zxvision_find_first_window_below_this. current window %p below window: %p title below: %s\n",pointer_window,pointer_window->previous_window,pointer_window->previous_window->window_title);
        pointer_window=pointer_window->previous_window;
    }

    return pointer_window;
}



//decir si ventana ya existe, en base a su puntero a estructura
int zxvision_if_window_already_exists(zxvision_window *w)
{
    if (zxvision_current_window==NULL) return 0;

    //Primero buscar la inicial

    zxvision_window *pointer_window;

    pointer_window=zxvision_find_first_window_below_this(zxvision_current_window);





    while (pointer_window!=NULL) {
        //while (pointer_window!=w) {
        //printf ("window from bottom to top %p. next: %p nombre: %s\n",pointer_window,pointer_window->next_window,pointer_window->window_title);

        if (pointer_window==w) {
            //printf ("Window already exists!!\n");
            return 1;
        }


        pointer_window=pointer_window->next_window;
    }

    return 0;


}

//Borrar ventana si ya existe
//Esto se suele hacer cuando se conmuta entre ventanas en background
void zxvision_delete_window_if_exists(zxvision_window *ventana)
{
    //IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
    //si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
    //la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
    if (zxvision_if_window_already_exists(ventana)) {
        //printf ("Window already exists! We are possibly running on background. Make this the top window\n");

        //menu_generic_message_splash("Background task","OK. Window removed from background");

        debug_printf (VERBOSE_DEBUG,"Window removed from background");

        menu_speech_set_tecla_pulsada(); //Para que no envie a speech la ventana que esta por debajo de la que borramos

        //Al borrar la ventana se leeria la ventana que hay justo debajo
        zxvision_window_delete_this_window(ventana);

        //Forzar a leer la siguiente ventana que se abra (o sea a la que conmutamos)
        menu_speech_reset_tecla_pulsada();

    }
}

void zxvision_cls(zxvision_window *w)
{

    int total_width=w->total_width;
    int total_height=w->total_height;

    int buffer_size=total_width*total_height;


    //Inicializarlo todo con texto blanco

    int i;
    overlay_screen *p;
    p=w->memory;

    for (i=0;i<buffer_size;i++) {
        p->tinta=ESTILO_GUI_TINTA_NORMAL;
        p->papel=w->default_paper;
        p->parpadeo=0;
        p->caracter=' ';

        p++;
    }
}

int zxvision_get_size_text_buffer(int total_width,int total_height)
{
    int buffer_size=total_width*total_height;
    buffer_size *= sizeof(overlay_screen);

    return buffer_size;
}

void zxvision_alloc_memory(zxvision_window *w,int total_width,int total_height)
{
    //printf("zxvision_alloc_memory. width: %d height: %d\n",total_width,total_height);

    //int buffer_size=total_width*total_height;
    //w->memory=malloc(buffer_size*sizeof(overlay_screen));

    int memory_required=zxvision_get_size_text_buffer(total_width,total_height);
    w->memory=malloc(memory_required);

    //printf("ALLOC %lu %d\n",buffer_size*sizeof(overlay_screen),memory_required);

    if (w->memory==NULL) cpu_panic ("Can not allocate memory for window");

    //Inicializarlo todo con texto blanco
    //putchar_menu_overlay(x+j,y+i+1,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);

    zxvision_cls(w);

    /*
    int i;
    overlay_screen *p;
    p=w->memory;

    for (i=0;i<buffer_size;i++) {
        p->tinta=ESTILO_GUI_TINTA_NORMAL;
        p->papel=ESTILO_GUI_PAPEL_NORMAL;
        p->parpadeo=0;
        p->caracter=' ';

        p++;
    }
    */
}

void zxvision_reset_flag_dirty_must_draw_contents(zxvision_window *w)
{
    w->dirty_must_draw_contents=0;
}

void zxvision_set_flag_dirty_must_draw_contents(zxvision_window *w)
{
    w->dirty_must_draw_contents=1;
    //para el usuario
    w->dirty_user_must_draw_contents=1;
}

void zxvision_set_all_flag_dirty_must_draw_contents(void)
{

    //esto peta desde end_emulator si se sale con ctrl+c con menu principal abierto
    //Es porque el thread thread_main_loop se ha terminado
    //if (ending_emulator_flag) return;

    //Podemos empezar desde la de arriba por ejemplo, da igual
    zxvision_window *ventana;

    ventana=zxvision_current_window;


    while (ventana!=NULL) {
        //printf("ventana : %p\n",ventana);
        //printf("ventana titulo: %s\n",ventana->window_title);

        zxvision_set_flag_dirty_must_draw_contents(ventana);

        ventana=ventana->previous_window;
    }


}

//Siguiente pid a usar
unsigned int zxvision_last_pid=0;


//Buscar si el pid está asignada a alguna ventana
zxvision_window *zxvision_search_window_by_pid(unsigned int pid_to_search)
{
    //printf("Buscar si existe pid %u\n",pid_to_search);

    zxvision_window *w;
    w=zxvision_current_window;

    //Empezamos de arriba hacia abajo

    while (w!=NULL) {

        if (w->pid==pid_to_search) {
            //printf("Existe pid %u en ventana %s\n",pid_to_search,w->window_title);
            return w;
        }

        zxvision_window *lower_window;

        lower_window=w->previous_window;

        w=lower_window;

    }

    return NULL;

}

//Buscar siguiente pid para una ventana
//Nos basamos en zxvision_last_pid pero descartando que ese no exista ya
//Eso puede suceder cuando el contador de pid "da la vuelta", cosa altamente improbable, siendo un contador de 32 bits,
//pero podria pasar
unsigned int zxvision_get_next_pid(void)
{
    //TODO: este while se quedaria sin salir cuando hubiera tantas ventanas como permite el contador de pid = 2^32 = 4294967296
    //cosa completamente absurda, pero que sepamos que si se llena todo eso de ventanas, el bucle no acabara nunca
    //pues no encontrará ningun pid libre
    //En otros entornos con menos "cantidad" de pid libre se deberia tener un escape del bucle con un error indicando
    //que no hay pids libres y por tanto no se podria crear la ventana
    while (zxvision_search_window_by_pid(zxvision_last_pid)!=NULL) {
        zxvision_last_pid++;
    }

    //printf("Proximo pid: %u\n",zxvision_last_pid);

    return zxvision_last_pid;
}

void zxvision_set_ventana_tipo_activa(void)
{
    ventana_tipo_activa=1;
}

void zxvision_reset_ventana_tipo_activa(void)
{
    ventana_tipo_activa=0;
}


void zxvision_new_window_no_check_range(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title)
{

    //Alto visible se reduce en 1 - por el titulo de ventana
    //printf("--Crear ventana %s\n",title);

    w->x=x;
    w->y=y;
    w->visible_width=visible_width;
    w->visible_height=visible_height;

    w->total_width=total_width;
    w->total_height=total_height;

    w->offset_x=0;
    w->offset_y=0;

    //Asignar pid al principio, antes de meterla en la lista de ventanas activas
    w->pid=zxvision_get_next_pid();

    w->default_paper=ESTILO_GUI_PAPEL_NORMAL;

    //Establecer titulo ventana
    strcpy(w->window_title,title);

    zxvision_alloc_memory(w,total_width,total_height);


    //Ventana anterior
    w->previous_window=zxvision_current_window;
    //printf ("Previous window: %p\n",w->previous_window);
    w->next_window=NULL;

    //Ventana siguiente, decir a la anterior, es la actual
    if (zxvision_current_window!=NULL) {
        //printf ("Decimos que siguiente ventana es: %p\n",zxvision_current_window);
        zxvision_current_window->next_window=w;
    }

    //Ventana actual
    zxvision_current_window=w;


    //Decir que al abrir la ventana, las pulsaciones de teclas no se envian por defecto a maquina emulada
    zxvision_keys_event_not_send_to_machine=1;

    zxvision_set_ventana_tipo_activa();



    //Decimos que se puede redimensionar
    w->can_be_resized=1;
    //Decimos que se puede minimizar
    w->can_be_minimized=1;
    //Decimos que se puede cambiar scroll
    w->can_be_scrolled=1;


    w->disable_special_chars=0;
    w->writing_inverse_color=0;


    //Decimos que su contenido se puede redimensionar si se aumenta
    w->contents_can_be_enlarged=1;

    w->can_be_backgrounded=0;
    w->is_minimized=0;
    w->is_maximized=0;
    //printf("--zxvision_new_window_no_check_range. setting before min: %d X %d\n",visible_width,visible_height);
    w->height_before_max_min_imize=visible_height;
    w->width_before_max_min_imize=visible_width;


    //printf("zxvision_new_window_no_check_range. before min: %d X %d\n",w->width_before_max_min_imize,w->height_before_max_min_imize);

    w->x_before_max_min_imize=x;
    w->y_before_max_min_imize=y;

    w->beyond_x=0;
    w->beyond_y=0;
    w->beyond_width=0;
    w->beyond_height=0;
    w->beyond_color=0;

    w->can_use_all_width=0;
    //w->applied_can_use_all_width=0;

    w->must_clear_cache_on_draw=0;

    w->must_clear_cache_on_draw_once=0;

    zxvision_set_all_flag_dirty_must_draw_contents();

    w->has_been_drawn_contents=0;

    w->always_visible=0;

    w->not_altered_by_massive_changes=0;

    w->last_spent_time_overlay=0;

    w->can_mouse_send_hotkeys=0;

    w->no_refresh_change_offset=0;

    w->tried_write_beyond_size=0;

    w->do_not_warn_tried_write_beyond_size=0;

    w->visible_cursor=0;
    w->cursor_line=0;
    w->acortar_cursor=0;

    w->upper_margin=0;
    w->lower_margin=0;

    //Por defecto no tiene nombre de guardado de geometria
    w->geometry_name[0]=0;

    //Y textos margen nulos
    //w->text_margin[0]=NULL;


    //Funcion de overlay inicializada a NULL
    w->overlay_function=NULL;


    zxvision_set_draw_window_parameters(w);

    //Redibujar las de debajo
    zxvision_draw_below_windows_nospeech(w);


}

//La quita de la lista pero no libera su memoria usada
void zxvision_window_delete_this_window_no_free_mem(zxvision_window *ventana)
{
        //hacer esta la ventana activa
        /*
        Ventana activa: zxvision_current_window. Si no hay ventanas, vale NULL
        Por debajo: se enlaza con previous_window. Hacia arriba se enlaza con next_window

        Para hacer esta ventana la activa

        NULL *prev* <- A  *prev* <-  -> *next* ventana *prev* <-  -> *next* C <-> D <-> E <-> zxvision_current_window -> NULL


        NULL *prev* <- A  *prev* <-  -> *next*                       *next* C <-> D <-> E <->  <-> ventana -> NULL
        */

        zxvision_window *next_to_ventana=ventana->next_window;
        zxvision_window *prev_to_ventana=ventana->previous_window;
        //zxvision_window *prev_to_current_window=zxvision_current_window->previous_window;

        //Primero cambiar la anterior a esta, diciendo que nos saltamos "ventana"
        if (prev_to_ventana!=NULL) {
            //La siguiente a esta, sera la siguiente a la ventana actual
            prev_to_ventana->next_window=next_to_ventana;
        }

        //Luego la que era la siguiente a esta "ventana", decir que su anterior es la anterior a "ventana"
        if (next_to_ventana!=NULL) {
            next_to_ventana->previous_window=prev_to_ventana;
        }

        //Si era la de arriba del todo, hacer que apunte a la anterior. Esto tambien cumple el caso de ser la unica ventana
        if (zxvision_current_window==ventana) {
            zxvision_current_window=prev_to_ventana;
            //printf ("Somos la de arriba\n");
            //printf ("Current: %p\n",zxvision_current_window);
            //sleep(5);
        }


}

//Elimina la ventana de la lista y libera su memoria usada
void zxvision_window_delete_this_window(zxvision_window *ventana)
{

    zxvision_window_delete_this_window_no_free_mem(ventana);

    if (ventana->memory!=NULL) free(ventana->memory);


    zxvision_redraw_all_windows();

}


//Elimina todas las ventanas
void zxvision_window_delete_all_windows(void)
{

    //No hacerlo si no hay ventanas
    if (zxvision_current_window==NULL) return;

    zxvision_window *ventana;

    ventana=zxvision_current_window;

    do {
        //Se cierran todas, incluso las que tienen el flag immutable (como process switcher)
        //porque esta función de cerrado se usa, entre otros sitios, al cerrar todas las ventanas
        //al pasar a pantalla completa, y no queremos dejarnos ninguna abierta en ese caso
        zxvision_window *previa;

        //Antes de cerrarlas, desminimizar las que estén minimizadas. No quiero que tengan su geometria como minimizadas
        //si ya no están abiertas. Por que si no, se guardarian como minimizadas, quedarian cerradas,
        //y al arrancarlas de nuevo no se verian porque arrancan minimizadas. Y habria que arrancarlas dos veces para que la
        //segunda vez ya se desminimicen

        if (ventana->is_minimized) {

            debug_printf(VERBOSE_INFO,"Unminimize window [%s] before closing it",ventana->window_title);

            //Luego simulamos accion de pulsar boton de minimizar ventana
            //zxvision_handle_click_minimize(ventana);

            //Hacemos esto otro que va mas rapido. No restaura tamaños y demas pero es que simular el click va mucho mas lento
            ventana->is_minimized=0;


            //Y guardar la geometria
            //printf("antes grabar geometria\n");
            util_add_window_geometry_compact(ventana);
            //printf("despues grabar geometria\n");

        }

        previa=ventana->previous_window;
        zxvision_window_delete_this_window(ventana);


        //Saltamos a la ventana anterior
        ventana=previa;


    } while (ventana!=NULL);

    //Gestion de los submenus
    //liberar memoria de todos los submenus, ya se han cerrado desde el bucle previo
    menu_dibuja_submenu_free_all();


}


//Elimina todas las ventanas y borra geometria ventanas
void zxvision_window_delete_all_windows_and_clear_geometry(void)
{

    if (menu_allow_background_windows) zxvision_window_delete_all_windows();

    //Olvidar geometria ventanas
    util_clear_all_windows_geometry();

}

void zxvision_window_move_this_window_on_top(zxvision_window *ventana)
{
        //hacer esta la ventana activa
        /*
        Ventana activa: zxvision_current_window. Si no hay ventanas, vale NULL
        Por debajo: se enlaza con previous_window. Hacia arriba se enlaza con next_window

        Para hacer esta ventana la activa

        NULL *prev* <- A  *prev* <-  -> *next* ventana *prev* <-  -> *next* C <-> D <-> E <-> zxvision_current_window -> NULL


        NULL *prev* <- A  *prev* <-  -> *next*                       *next* C <-> D <-> E <->  <-> ventana -> NULL
        */

        zxvision_window_delete_this_window_no_free_mem(ventana);



        //Hasta aqui lo que hemos hecho ha sido quitar nuestra ventana

        //printf("zxvision_current_window: %p\n",zxvision_current_window);

        //Ahora la subimos arriba del todo
        if (zxvision_current_window!=NULL) {
            zxvision_current_window->next_window=ventana;
        }

        //Y mi actual ahora es la actual que habia de current
        ventana->previous_window=zxvision_current_window;

        //Y no tenemos siguiente, o sea NULL
        ventana->next_window=NULL;

        //Y la actual somos nosotros

        zxvision_current_window=ventana;

        zxvision_redraw_all_windows();
}

void zxvision_activate_this_window(zxvision_window *ventana)
{

    //Si esta minimizada y el setting de ocultarla, desminimizarla
    if (ventana->is_minimized && zxvision_hide_minimized_windows.v) {
        zxvision_toggle_minimize_window(ventana);
    }

    zxvision_window_move_this_window_on_top(ventana);
    zxvision_keys_event_not_send_to_machine=1;

    zxvision_set_ventana_tipo_activa();

}

void zxvision_window_move_this_window_to_bottom(zxvision_window *ventana)
{
        //hacer esta la ventana activa
        /*
        Ventana activa: zxvision_current_window. Si no hay ventanas, vale NULL
        Por debajo: se enlaza con previous_window. Hacia arriba se enlaza con next_window

        Para hacer esta ventana la activa

                   NULL *prev* <- A  *prev* <-  -> *next* ventana *prev* <-  -> *next* C <-> D <-> E <-> zxvision_current_window -> NULL


        NULL *prev* <- ventana <- A  *prev* <-  -> *next*                       *next* C <-> D <-> E <->  zxvision_current_window -> NULL
        */



        //buscamos la de abajo del todo
        zxvision_window *lower_window=zxvision_find_first_window_below_this(zxvision_current_window);

        //Si hay mas de 1
        if (lower_window!=ventana) {
            zxvision_window_delete_this_window_no_free_mem(ventana);

            //Hasta aqui lo que hemos hecho ha sido quitar nuestra ventana

            //Indicamos la nueva de abajo del todo
            lower_window->previous_window=ventana;

            //Y de la nuestra, su siguiente
            ventana->next_window=lower_window;

            //Y de la nuestra, la de abajo (ninguna)
            ventana->previous_window=NULL;
        }


        zxvision_redraw_all_windows();
}



//Retorna ventana empezando por la 0 desde arriba hasta abajo
//NULL si no existe
zxvision_window *zxvision_return_n_window_from_top(int indice)
{
    zxvision_window *ventana=zxvision_current_window;

    //printf ("zxvision_return_n_window_from_top. indice: %d\n",indice);

    int i;


    for (i=0;i<indice && ventana!=NULL;i++) {
        //printf ("ventana: %p indice: %d\n",ventana,indice);
        ventana=ventana->previous_window;
    }

    return ventana;


}

void zxvision_new_window(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title)
{

    zxvision_new_window_check_range(&x,&y,&visible_width,&visible_height);
    zxvision_new_window_check_static_size_range(&x,&y,&visible_width,&visible_height);
    zxvision_new_window_no_check_range(w,x,y,visible_width,visible_height,total_width,total_height,title);
}

void zxvision_new_window_nocheck_staticsize(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title)
{

    zxvision_new_window_check_range(&x,&y,&visible_width,&visible_height);
    zxvision_new_window_no_check_range(w,x,y,visible_width,visible_height,total_width,total_height,title);
}

//Crear ventana, asignando Geometry Name (gm) y comprobando si esta minimizada (Check If Minimized)
//Y asignando tamaños de antes minimizado

//Si la ventana se crea minimizada, se asigna total width y height a valores grandes -> ESTO YA NO, PUES AL REDIMENSIONAR SE RECREA
void zxvision_new_window_gn_cim(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,
    char *title,char *geometry_name,int is_minimized,int is_maximized,int width_before_max_min_imize,int height_before_max_min_imize)
{

    //Si ventana se crea minimizada, establecemos total_width y total_height a un valor grande,
    //para que se tenga contenido de esa ventana al cambiarle el tamaño: pues en ese caso, al cambiar tamaño,
    //la ventana no se recrea y el tamaño del texto contenido dentro es siempre total_width X total_height
    //Si no se hiciera esto, lo que sucede es que al redimensionar una ventana minimizada, no se ve su contenido
    //pues tiene alto total=0
    //No en todas las ventanas hace falta esto, pues algunas detectan cuando se redimensionan y se crean de nuevo
    //(y por tanto al crearlas se asigna nuevo total width y height)
    //O porque son ventanas basadas en pixel y ahi siempre se muestra (como wave piano)
    if (is_minimized) {
        //printf("ancho %d alto %d\n",total_width,total_height);
        //printf("Changing total width and height as the window is created minimized\n");
        //le doy ancho y alto mas que suficiente

        //Esto ya no hace falta pues al redimensionar se recrea
        //total_width=ZXVISION_MAX_ANCHO_VENTANA-10;
        //total_height=ZXVISION_MAX_ALTO_VENTANA-10;

        //printf("despues ancho %d alto %d\n",total_width,total_height);

    }

    zxvision_new_window(w,x,y,visible_width,visible_height,total_width,total_height,title);

    if (is_minimized || is_maximized) {
        //printf("--zxvision_new_window_gn_cim. setting before min: %d X %d\n",width_before_max_min_imize,height_before_max_min_imize);
        w->width_before_max_min_imize=width_before_max_min_imize;
        w->height_before_max_min_imize=height_before_max_min_imize;
        //printf("zxvision_new_window_gn_cim. window is minimized\n");
    }

    //printf("zxvision_new_window_gn_cim. before minimize ancho %d alto %d\n",width_before_max_min_imize,height_before_max_min_imize);


    //indicar nombre del grabado de geometria
    strcpy(w->geometry_name,geometry_name);
    //restaurar estado minimizado de ventana
    w->is_minimized=is_minimized;
    w->is_maximized=is_maximized;


}

/*
void legacy_zxvision_new_window_gn_cim(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,
    char *title,char *geometry_name,int is_minimized,int width_before_max_min_imize,int height_before_max_min_imize)
{
    zxvision_new_window_gn_cim(w,x,y,visible_width,visible_height,total_width,total_height,title,geometry_name,
        is_minimized,0,width_before_max_min_imize,height_before_max_min_imize);

}
*/

//Borrar contenido ventana con espacios
void zxvision_clear_window_contents(zxvision_window *w)
{

    int y;

    for (y=0;y<w->total_height;y++) {
        zxvision_fill_width_spaces(w,y);
    }

}

void zxvision_fill_window_transparent(zxvision_window *w)
{
        //Metemos todo el contenido de la ventana con caracter transparente, para que no haya parpadeo
        //en caso de drivers xwindows por ejemplo, pues continuamente redibuja el texto (espacios) y encima el overlay
        //Al meter caracter transparente, el normal_overlay lo ignora y no dibuja ese caracter
        int x,y;

        for (y=0;y<w->total_height;y++) {
            for (x=0;x<w->total_width;x++) {
                zxvision_print_string_defaults(w,x,y,"\xff");
            }
        }
}

//Indica si en el siguiente evento de cierre de ventana no reproduce sonido de cierre de ventana
int zxvision_next_do_not_play_close_window_sound=0;

void zxvision_destroy_window_if_redraw(zxvision_window *w,int redraw)
{
    zxvision_current_window=w->previous_window;


    //printf ("Setting current window to %p\n",zxvision_current_window);

    //printf ("Next window was %p\n",w->next_window);
    //printf("Destroying window %p\n",w);

    zxvision_set_ventana_tipo_activa();
    zxvision_keys_event_not_send_to_machine=1;

    int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;

    menu_speech_set_tecla_pulsada(); //Para no leer las ventanas de detrás al cerrar la actual

    //Siguiente refresco de zxdesktop no hay framedrop, para forzar que se vea el cambio de cerrar ventana
    no_next_frameskip_draw_zxdesktop_background.v=1;

    if (zxvision_current_window!=NULL) {
        //Dibujar las de detras
        //printf ("Dibujando ventanas por detras\n");

        //printf ("proxima ventana antes de dibujar: %p\n",w->next_window);


        if (redraw) {
            zxvision_draw_below_windows_nospeech(w);
            zxvision_set_draw_window_parameters(zxvision_current_window);

            //Dibujar ventana que habia debajo
            zxvision_draw_window(zxvision_current_window);
            zxvision_draw_window_contents(zxvision_current_window);
            //printf ("Dibujando ventana de debajo que ahora es de frente\n");
        }
    }

        /*if (menu_speech_tecla_pulsada && !antes_menu_speech_tecla_pulsada) {
            printf("Pasar de 1 a 0 desde \n");
            debug_exec_show_backtrace();
        }*/

    menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;

    //Liberar memoria cuando ya no se use para nada
    free(w->memory);

    //Decir que esta ventana no tiene siguiente.
    //TODO: solo podemos hacer destroy de la ultima ventana creada,
    //habria que tener metodo para poder destruir ventana de en medio
    //TODO2: si hacemos esto justo despues de zxvision_current_window=w->previous_window; acaba provocando segfault al redibujar. por que?
    if (zxvision_current_window!=NULL) zxvision_current_window->next_window=NULL;

    //para poder hacer destroy de ventana de en medio seria tan simple como hacer que zxvision_current_window->next_window= fuera el next que habia al principio

    zxvision_set_all_flag_dirty_must_draw_contents();

    if (redraw) {
        cls_menu_overlay();

        zxvision_redraw_all_windows();
    }

    //printf("zxvision_destroy_window_if_redraw. zxvision_next_do_not_play_close_window_sound=%d\n",zxvision_next_do_not_play_close_window_sound);
    if (!zxvision_next_do_not_play_close_window_sound) zxvision_sound_event_close_window();

    //printf("Se pone zxvision_next_do_not_play_close_window_sound a 0 al cerrar ventana\n");
    zxvision_next_do_not_play_close_window_sound=0;

}

void zxvision_destroy_window(zxvision_window *w)
{
    zxvision_destroy_window_if_redraw(w,1);
}

void zxvision_speech_read_current_window(void)
{
    //Esto no deberia pasar, pero por si acaso
    if (zxvision_current_window==NULL) return;

    menu_espera_no_tecla();

    //Decir que no hay tecla forzada, para releer el menu
    menu_speech_reset_tecla_pulsada();
    //Y simplemente dibujamos la ventana y su contenido, y eso hará releerla
    menu_textspeech_send_text("Reading the window contents");

    //Guardamos valor overlay anterior. Ventanas que cambian overlay, como AY Registers, dicen que hay tecla
    //pulsada y no salte el speech, sino estarian enviando a cada refresco de pantalla
    //Nos aseguramos que esto no se hace temporalmente y asi se redibuja y se lee la ventana

    void (*previous_function)(void);

    previous_function=menu_overlay_function;

    //restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);


    menu_speech_reset_tecla_pulsada();
    zxvision_draw_window(zxvision_current_window);

    menu_speech_reset_tecla_pulsada();
    zxvision_draw_window_contents(zxvision_current_window);


    //Restauramos funcion anterior de overlay
    set_menu_overlay_function(previous_function);
}


z80_byte zxvision_read_keyboard(void)
{

    //Decimos que de momento no se pulsa shift+left/right
    menu_pressed_shift_left=menu_pressed_shift_right=0;

    //A 0 por defecto aunque la mayoria de casos estan gestionados, pero por ejemplo si mouse_pressed_hotkey_window
    //Se iria hasta casi el final de la funcion donde hay el if (mouse_pressed_hotkey_window) pasando por algun (if tecla==4)
    //donde tecla si no pusieramos a 0 podria tener un valor indeterminado
    z80_byte tecla=0;

    if (!mouse_pressed_close_window && !mouse_pressed_background_window && !mouse_pressed_hotkey_window) {
        tecla=menu_get_pressed_key();


        //Si ventana inactiva y se ha pulsado tecla, excepto ESC y excepto background, no leer dicha tecla
        if (tecla!=0 && tecla!=2 && tecla!=3 && zxvision_keys_event_not_send_to_machine==0) {
            //printf ("no leemos tecla en ventana pues esta inactiva\n");
            tecla=0;
        }
    }

    //Si pulsado boton cerrar ventana, enviar ESC
    if (mouse_pressed_close_window) {
        //printf("Retornar ESC pues hay mouse_pressed_close_window\n");
        return 2;
    }

    //Si pulsado boton background ventana, enviar tecla background
    if (mouse_pressed_background_window) {
        //printf ("pulsado background en zxvision_read_keyboard\n");
        //sleep(5);
        //printf("Retornamos tecla 3 desde zxvision_read_keyboard\n");
        return 3;
    }

    //Pulsadas teclas de shift+left/right
    if (menu_allow_background_windows && (tecla==4 || tecla==5)) {
        if (tecla==4) menu_pressed_shift_left=1;
        if (tecla==5) menu_pressed_shift_right=1;
        debug_printf(VERBOSE_DEBUG,"Pressed switch window keys (left or right)");
        //printf("menu_pressed_shift_cursor_window_doesnot_allow: %d\n",menu_pressed_shift_cursor_window_doesnot_allow);


        //Si ventana no tiene previo, decir no tecla
        if (zxvision_current_window!=NULL) {
            if (zxvision_current_window->previous_window==NULL) {
                debug_printf(VERBOSE_DEBUG,"Just one window");
                return 0;
            }
        }


        //devolver como ESC cuando ventana no permite background
        if (zxvision_current_window!=NULL) {
            if (!(zxvision_current_window->can_be_backgrounded)) {
                debug_printf(VERBOSE_DEBUG,"Return ESC cause window does not allow background");
                salir_todos_menus=1;
                //hay que hacer que se activase una ventana de las de background
                zxvision_window *previous_window=zxvision_current_window->previous_window;
                if (previous_window!=NULL) {
                    zxvision_handle_mouse_ev_switch_back_wind(previous_window);
                }
                return 2;
            }
        }
        //Devolver como background
        return 3;
    }


    if (mouse_pressed_hotkey_window) {
        mouse_pressed_hotkey_window=0;
        //printf ("Retornamos hoykey %c desde zxvision_read_keyboard\n",mouse_pressed_hotkey_window_key);
        return mouse_pressed_hotkey_window_key;
    }

    //Si se ha pulsado F4, leer ventana
    //z80_byte puerto_especial2=255; //   F5 F4 F3 F2 F1
    if ((puerto_especial2 & 8)==0) {
        //printf ("leer ventana de menu\n");
        zxvision_speech_read_current_window();
    }

    //printf("zxvision_read_keyboard. Retorno tecla: %d\n",tecla);

    return tecla;
}

int zxvision_wait_until_esc(zxvision_window *w)
{
    z80_byte tecla;

    do {
        tecla=zxvision_common_getkey_refresh();
        zxvision_handle_cursors_pgupdn(w,tecla);
    } while (tecla!=2 && tecla!=3);

    return tecla;

}


//escribe la cadena de texto
void zxvision_scanf_print_string(zxvision_window *ventana,char *string,int offset_string,int max_length_shown,int x,int y,int pos_cursor_x)
{

    char cadena_buf[2];

    string=&string[offset_string];


    int rel_x=0;

    //y si offset>0, primer caracter sera '<'
    if (offset_string) {
        zxvision_print_string_defaults(ventana,x,y,"<");
        max_length_shown--;
        x++;
        rel_x++;
        string++;
    }

    for (;max_length_shown;max_length_shown--) {

        if (rel_x==pos_cursor_x) {
            //printf ("Escribir cursor en medio o final en %d %d\n",x,y);
            zxvision_print_string(ventana,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,1,"_");
        }

        else {
            if ( (*string) !=0) {
                cadena_buf[0]=*string;
                cadena_buf[1]=0;

                zxvision_print_string_defaults(ventana,x,y,cadena_buf);
                string++;
            }

            else {
                //meter espacios
                zxvision_print_string_defaults(ventana,x,y," ");
            }
        }
        x++;
        rel_x++;


    }

    zxvision_draw_window_contents(ventana);


}

void menu_scanf_cursor_izquierda(int *offset_string,int *pos_cursor_x)
{

                //Desplazar siempre offset que se pueda
                if ((*offset_string)>0) {
                    (*offset_string)--;
                    //printf ("offset string: %d\n",*offset_string);
                }

                else if ((*pos_cursor_x)>0) (*pos_cursor_x)--;
}

void menu_scanf_cursor_derecha(char *texto,int *pos_cursor_x,int *offset_string,int max_length_shown)
{

            int i;
            i=strlen(texto);

            int pos_final=(*offset_string)+(*pos_cursor_x);


            if (pos_final<i) {

                if ((*pos_cursor_x)<max_length_shown-1) {
                        (*pos_cursor_x)++;
                        //printf ("mover cursor\n");
                }
                    //Si no mueve cursor, puede que haya que desplazar offset del inicio

                else if (i>=max_length_shown) {
                    //printf ("Scroll\n");
                    (*offset_string)++;
                }
            }
}

//devuelve cadena de texto desde teclado
//max_length contando caracter 0 del final, es decir, para un texto de 4 caracteres, debemos especificar max_length=5
//ejemplo, si el array es de 50, se le debe pasar max_length a 50
//volver_si_fuera_foco dice si vuelve al pulsar en linea de edicion pero mas a la izquierda o derecha de esa zona
int zxvision_scanf(zxvision_window *ventana,char *string,unsigned int max_length,int max_length_shown,int x,int y,
    int volver_si_fuera_foco,int volver_si_flecha_abajo,int forzar_cursor)
{


    //Al menos 2 de maximo a mostrar. Si no, salir
    if (max_length_shown<2) {
        debug_printf (VERBOSE_ERR,"Edit field size too small. Returning null string");
        string[0]=0;
        return 2; //Devolvemos escape

    }

    //Enviar a speech
    char buf_speech[MAX_BUFFER_SPEECH+1];
    sprintf (buf_speech,"Edit box: %s",string);
    menu_textspeech_send_text(buf_speech);


    z80_byte tecla;

    //ajustar offset sobre la cadena de texto visible en pantalla
    int offset_string;

    int j;
    j=strlen(string);
    if (j>max_length_shown-1) offset_string=j-max_length_shown+1;
    else offset_string=0;

    int pos_cursor_x; //Donde esta el cursor

    pos_cursor_x=j;

    if (forzar_cursor) pos_cursor_x=forzar_cursor;

    //printf("pos_cursor_x before %d\n",pos_cursor_x);

    if (pos_cursor_x>max_length_shown-1) pos_cursor_x=max_length_shown-1;

    //printf("pos_cursor_x after %d\n",pos_cursor_x);


    //max_length ancho maximo del texto, sin contar caracter 0
    //por tanto si el array es de 50, se le debe pasar max_length a 50

    max_length--;

    //cursor siempre al final del texto

    int refrescar=1;

    do {

        //para acelerar este proceso, solo llamar aqui cuando haya alguna modificacion
        //asi se puede mover bien la ventana sin usar mucha cpu
        if (refrescar) {
            zxvision_scanf_print_string(ventana,string,offset_string,max_length_shown,x,y,pos_cursor_x);
            refrescar=0;
        }

        //printf("After print string\n");
        //if (menu_multitarea==0) menu_refresca_pantalla();

        //menu_espera_tecla();
        //printf ("Despues de espera tecla\n");
        tecla=zxvision_common_getkey_refresh();
        //printf ("tecla leida=%d\n",tecla);
        int mouse_left_estaba_pulsado=mouse_left;

        //no esperar a liberar tecla asi podemos gestionar repeticiones
        //menu_espera_no_tecla();



        //si tecla normal, agregar en la posicion del cursor:
        if (tecla>31 && tecla<128) {
            if (strlen(string)<max_length) {

                //int i;
                //i=strlen(string);

                int pos_agregar=pos_cursor_x+offset_string;
                //printf ("agregar letra en %d\n",pos_agregar);
                util_str_add_char(string,pos_agregar,tecla);


                //Enviar a speech letra pulsada
                menu_speech_reset_tecla_pulsada();
                sprintf (buf_speech,"%c",tecla);
                menu_textspeech_send_text(buf_speech);

                //Y mover cursor a la derecha
                menu_scanf_cursor_derecha(string,&pos_cursor_x,&offset_string,max_length_shown);

                refrescar=1;

                //printf ("offset_string %d pos_cursor %d\n",offset_string,pos_cursor_x);

            }

            else {
                //printf ("llegado al maximo\n");
                //-que diga "too long" en speech cuando llega al maximo de longitud en scanf
                menu_speech_reset_tecla_pulsada();
                menu_textspeech_send_text("Too long");

            }
        }

        //tecla derecha
        if (tecla==9) {
                menu_scanf_cursor_derecha(string,&pos_cursor_x,&offset_string,max_length_shown);
                //printf ("offset_string %d pos_cursor %d\n",offset_string,pos_cursor_x);
                refrescar=1;

                zxvision_sound_event_cursor_movement();
        }

        //tecla End
        //27 End
        if (tecla==27) {
                int mover=strlen(string)-(pos_cursor_x+offset_string);
                //printf("mover: %d\n",mover);

                while (mover>0) {
                    menu_scanf_cursor_derecha(string,&pos_cursor_x,&offset_string,max_length_shown);
                    mover--;
                };
                //printf ("offset_string %d pos_cursor %d\n",offset_string,pos_cursor_x);
                refrescar=1;

                zxvision_sound_event_cursor_movement();
        }

        //tecla borrar
        if (tecla==12) {
            //Si longitud texto no es 0
            if (strlen(string)>0) {

                int pos_eliminar=pos_cursor_x+offset_string-1;

                //no borrar si cursor a la izquierda del todo
                if (pos_eliminar>=0) {

                    //printf ("borrar\n");


                    //Enviar a speech letra borrada

                    menu_speech_reset_tecla_pulsada();
                    sprintf (buf_speech,"%c",string[pos_eliminar]);
                    menu_textspeech_send_text(buf_speech);

                    //Eliminar ese caracter
                    util_str_del_char(string,pos_eliminar);

                    //Y mover cursor a la izquierda
                    menu_scanf_cursor_izquierda(&offset_string,&pos_cursor_x);

                    refrescar=1;

                }
            }

        }

        //tecla izquierda
        if (tecla==8) {
                menu_scanf_cursor_izquierda(&offset_string,&pos_cursor_x);
                //printf ("offset_string %d pos_cursor %d\n",offset_string,pos_cursor_x);

                refrescar=1;

                zxvision_sound_event_cursor_movement();
        }

        //tecla home
        //26 Home
        if (tecla==26) {
                int mover=offset_string+pos_cursor_x;
                //printf("mover: %d\n",mover);

                while (mover>0) {
                    menu_scanf_cursor_izquierda(&offset_string,&pos_cursor_x);
                    mover--;
                };
                //printf ("offset_string %d pos_cursor %d\n",offset_string,pos_cursor_x);

                refrescar=1;

                zxvision_sound_event_cursor_movement();
        }

        //tecla abajo.
        if (tecla==10) {
            if (volver_si_flecha_abajo) {
                //Volver
                //tecla=13;
                zxvision_sound_event_cursor_movement();
            }

            else {
                //borrar todo si es que no se sale con flecha abajo
                //Enviar a speech decir borrar todo
                menu_speech_reset_tecla_pulsada();
                strcpy (buf_speech,"delete all");
                menu_textspeech_send_text(buf_speech);

                string[0]=0;
                offset_string=0;
                pos_cursor_x=0;
                tecla=0; //para no volver

                refrescar=1;
            }

        }

        //Si pulsado boton
        //printf ("mouse_left: %d\n",mouse_left_estaba_pulsado);
        if (volver_si_fuera_foco && tecla==0 && mouse_left_estaba_pulsado) {
            //printf ("Pulsado boton izquierdo en zxvision_scanf\n");

            //Si se pulsa mas alla de la zona de edicion
            //printf("mouse_x %d mouse_y %d x %d y %d\n",menu_mouse_x,menu_mouse_y,x,y);



            int mouse_y_ventana=menu_mouse_y-1;

            //printf("mouse_x %d mouse_y_ventana %d x %d y %d max_length_shown %d\n",menu_mouse_x,mouse_y_ventana,x,y,max_length_shown);

            if (mouse_y_ventana==y &&

                (menu_mouse_x>=x+max_length_shown || menu_mouse_x<x)

            ){
                //printf("Pulsado mas a la derecha de la zona de edicion\n");

                //Como si fuera enter , para volver
                tecla=13;
            }

            //O si se pulsa en coordenada y por debajo de input pero dentro de ventana
            if (mouse_y_ventana>y && mouse_y_ventana<y+ventana->visible_height-1) {
                //printf("pulsado por debajo coordenada Y\n");
                tecla=13;
            }
        }


    } while (tecla!=13 && tecla!=15 && tecla!=2 && tecla!=10);

    //if (tecla==13) printf ("salimos con enter\n");
    //if (tecla==15) printf ("salimos con tab\n");

    //Mostrar texto quitando cursor
    zxvision_scanf_print_string(ventana,string,offset_string,max_length_shown,x,y,-1);

    menu_reset_counters_tecla_repeticion();
    return tecla;

//papel=7+8;
//tinta=0;

}


int zxvision_generic_message_cursor_down(zxvision_window *ventana)
{

    //int linea_retornar;

    if (ventana->visible_cursor) {

        //Movemos el cursor si es que es posible
        if (ventana->cursor_line<ventana->total_height-1) {
            //printf ("Incrementamos linea cursor\n");
            zxvision_inc_cursor_line(ventana);
        }
        else {

            return ventana->cursor_line;
        }

        //Ver en que offset estamos
        int offset_y=ventana->offset_y;
        //Y donde esta el cursor
        int cursor=ventana->cursor_line;

        //Y si cursor no esta visible, lo ponemos para que este abajo del todo (hemos de suponer que estaba abajo y ha bajado 1 mas)
        if (cursor<offset_y || cursor>=offset_y+ventana->visible_height-2) {
            zxvision_set_cursor_line(ventana,offset_y+ventana->visible_height-2);
            zxvision_send_scroll_down(ventana);
            //printf ("Bajamos linea cursor y bajamos offset\n");
        }
        else {
            //Redibujamos contenido
            //printf ("Solo redibujamos\n");
            zxvision_draw_window_contents(ventana);
            //zxvision_draw_scroll_bars(w);
        }

        return ventana->cursor_line;
    }

    else {
        zxvision_send_scroll_down(ventana);
        return (ventana->offset_y + ventana->visible_height-3);
    }



}

int zxvision_generic_message_cursor_up(zxvision_window *ventana)
{
    if (ventana->visible_cursor) {

        //Movemos el cursor si es que es posible
        if (ventana->cursor_line>0) {
            //printf ("Decrementamos linea cursor\n");
            zxvision_dec_cursor_line(ventana);
        }
        else return ventana->cursor_line;

        //Ver en que offset estamos
        int offset_y=ventana->offset_y;
        //Y donde esta el cursor
        int cursor=ventana->cursor_line;

        //Y si cursor no esta visible, lo ponemos para que este arriba del todo (hemos de suponer que estaba arriba i ha subido 1 mas)
        if (cursor<offset_y || cursor>=offset_y+ventana->visible_height-2) {
            if (offset_y>0) {
                zxvision_set_cursor_line(ventana,offset_y-1);
            }
            zxvision_send_scroll_up(ventana);
            //printf ("Subimos linea cursor y subimos offset\n");
        }
        else {
            //Redibujamos contenido
            //printf ("Solo redibujamos\n");
            zxvision_draw_window_contents(ventana);
            //zxvision_draw_scroll_bars(w);
        }

        return ventana->cursor_line;
    }

    else {
        zxvision_send_scroll_up(ventana);
        return ventana->offset_y;
    }



}

//Retorna el ancho en caracteres del ext desktop
int menu_get_width_characters_ext_desktop(void)
{
    return get_effective_zxdesktop_width()/menu_char_width/menu_gui_zoom;
}

//Retorna el alto en caracteres del ext desktop
int menu_get_height_characters_ext_desktop(void)
{
    return get_effective_zxdesktop_height()/menu_char_height/menu_gui_zoom;
}

int menu_get_origin_x_zxdesktop_aux(int divisor)
{
    //Esta zxdesktop. Intentamos mantener ventanas localizadas ahi por defecto, si hay esa opcion activada
    int ancho_total=scr_get_menu_width();

    //Quitamos el tamaño maximo ventana (normalmente 32), entre 2
    //int pos_x=ancho_total-ZXVISION_MAX_ANCHO_VENTANA/2;

    int restar=menu_get_width_characters_ext_desktop();
    //printf ("restar: %d\n",restar);
    //al menos 32 de ancho para zona de menu
    if (restar<32) restar=32;
    int pos_x=ancho_total-restar/divisor;

    //Por si acaso
    if (pos_x<0) pos_x=0;
    return pos_x;
}

int menu_origin_x(void)
{


    if (screen_ext_desktop_place_menu && screen_ext_desktop_enabled*scr_driver_can_ext_desktop()) {
        //Esta zxdesktop. Intentamos mantener ventanas localizadas ahi por defecto, si hay esa opcion activada
        return menu_get_origin_x_zxdesktop_aux(1);
    }

    return 0;
}


//Dice si esta habilitado el ext desktop, y si los menús se deben abrir ahi por defecto
int menu_ext_desktop_enabled_place_menu(void)
{
    return screen_ext_desktop_place_menu && screen_ext_desktop_enabled*scr_driver_can_ext_desktop();
}

int menu_center_x(void)
{


    //if (screen_ext_desktop_place_menu && screen_ext_desktop_enabled*scr_driver_can_ext_desktop()) {
    if (menu_ext_desktop_enabled_place_menu()) {
        //Esta zxdesktop. Intentamos mantener ventanas localizadas ahi por defecto, si hay esa opcion activada
        return menu_get_origin_x_zxdesktop_aux(2);
    }

    return scr_get_menu_width()/2;
}

//Retorna inicio x para una nueva ventana considerando ancho y el centro
int menu_center_x_from_width(int ancho_ventana)
{

    int x_ventana=menu_center_x()-ancho_ventana/2;

    int ancho_total_menu=scr_get_menu_width();

    //Si sobresale la ventana por la derecha (por ejemplo en caso de zxdesktop habilitado con tamaño 256 y ancho mayor que 32)
    if (x_ventana+ancho_ventana>=ancho_total_menu) {
        //printf("Sale de rango. Corrigiendo\n");
        x_ventana=ancho_total_menu-ancho_ventana;
        //Por si acaso
        if (x_ventana<0) x_ventana=0;
    }

    return x_ventana;
}

/*
No usado, se agregó al implementar zx desktop en vertical, pero no se usa
int menu_get_origin_y_zxdesktop_aux(int divisor)
{
    //Esta zxdesktop. Intentamos mantener ventanas localizadas ahi por defecto, si hay esa opcion activada
    int alto_total=scr_get_menu_height();

    //Quitamos el tamaño maximo ventana (normalmente 24), entre 2

    int restar=menu_get_height_characters_ext_desktop();
    //printf ("restar: %d\n",restar);
    //al menos 24 de alto para zona de menu
    if (restar<24) restar=24;
    int pos_y=alto_total-restar/divisor;

    //Por si acaso
    if (pos_y<0) pos_y=0;
    return pos_y;
}
*/

int menu_center_y(void)
{
    /*if (menu_ext_desktop_enabled_place_menu()) {
        //Esta zxdesktop. Intentamos mantener ventanas localizadas ahi por defecto, si hay esa opcion activada
        return menu_get_origin_y_zxdesktop_aux(2);
    }*/

    return scr_get_menu_height()/2;
}

//Funcion generica para preguntar por un archivo de texto a grabar, con un unico filtro de texto
//Retorna 0 si se cancela
int menu_ask_file_to_save(char *titulo_ventana,char *filtro,char *file_save)
{
    //char file_save[PATH_MAX];

    char *filtros[2];

    filtros[0]=filtro;
    filtros[1]=0;

    int ret;

    ret=menu_filesel_save(titulo_ventana,filtros,file_save);

    if (ret==1) {

        //Ver si archivo existe y preguntar
        if (si_existe_archivo(file_save)) {

            if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return 0;

        }

        return 1;

    }

    return 0;
}


//Trocear el texto de entrada
//Nota: el resultado se guarda en buffer_lineas, este es un array de punteros a cada linea del texto de salida
//la alternativa a este array seria pasar el array de dos dimensiones
//Tal cual buffer_lineas[MAX_LINEAS_TOTAL_GENERIC_MESSAGE][NEW_MAX_ANCHO_LINEAS_GENERIC_MESSAGE], supuestamente eso el compilador
//lo pasa como un puntero al inicio pero no me gusta como quedaria asi
int zxvision_generic_message_aux_justificar_lineas(char *orig_texto,int longitud,int max_ancho_texto,char **buffer_lineas)
{

    //Copiar texto original en un buffer aparte porque lo modificamos quitando saltos de linea y otros caracteres no deseados
    char *texto;

    texto=malloc(longitud);

    if (texto==NULL) {
        cpu_panic("Can not allocate text");
    }

    memcpy(texto,orig_texto,longitud);

    int indice_linea=0;  //Numero total de lineas??

    int indice_texto=0;
    int ultimo_indice_texto=0;



    //printf("Inicio texto:\n");
    //printf("%s\n",texto);
    //printf("\nFin texto\n");

    //char buffer_lineas[MAX_LINEAS_TOTAL_GENERIC_MESSAGE][NEW_MAX_ANCHO_LINEAS_GENERIC_MESSAGE]

    //int indice_segunda_linea;

    //int texto_no_cabe=0;

    do {
        indice_texto+=max_ancho_texto;

        //temp
        //printf ("indice_linea: %d\n",indice_linea);

        //Controlar final de texto
        if (indice_texto>=longitud) indice_texto=longitud;

        //Si no, miramos si hay que separar por espacios
        else indice_texto=menu_generic_message_aux_wordwrap(texto,ultimo_indice_texto,indice_texto);

        //Separamos por salto de linea, filtramos caracteres extranyos
        indice_texto=menu_generic_message_aux_filter(texto,ultimo_indice_texto,indice_texto);

        //copiar texto
        int longitud_texto=indice_texto-ultimo_indice_texto;


        //snprintf(buffer_lineas[indice_linea],longitud_texto,&texto[ultimo_indice_texto]);


        menu_generic_message_aux_copia(&texto[ultimo_indice_texto],buffer_lineas[indice_linea],longitud_texto);
        buffer_lineas[indice_linea++][longitud_texto]=0;
        //printf ("copiado %d caracteres desde %d hasta %d: %s\n",longitud_texto,ultimo_indice_texto,indice_texto,buffer_lineas[indice_linea-1]);


        //printf ("texto: %s\n",buffer_lineas[indice_linea-1]);

        if (indice_linea==MAX_LINEAS_TOTAL_GENERIC_MESSAGE) {
                        //cpu_panic("Max lines on menu_generic_message reached");
            debug_printf(VERBOSE_INFO,"Max lines on menu_generic_message reached (%d)",MAX_LINEAS_TOTAL_GENERIC_MESSAGE);
            //finalizamos bucle
            indice_texto=longitud;
        }

        ultimo_indice_texto=indice_texto;
        //printf ("ultimo indice: %d %c\n",ultimo_indice_texto,texto[ultimo_indice_texto]);

    } while (indice_texto<longitud);


    free(texto);

    return indice_linea;

}

void zxvision_generic_message_crea_ventana(zxvision_window *ventana,int disable_special_chars,int xventana,int yventana,int ancho_ventana,int alto_ventana,
    int alto_total_ventana,char *titulo,int resizable,int mostrar_cursor,int lineas,char **buffer_lineas,
    int is_minimized,int is_maximized,int ancho_antes_minimize,int alto_antes_minimize)
{
    zxvision_new_window(ventana,xventana,yventana,ancho_ventana,alto_ventana,
                            ancho_ventana-1,alto_total_ventana,titulo);


    //printf ("despues de zxvision_new_window\n");

    if (!resizable) {
        zxvision_set_not_resizable(ventana);
        zxvision_set_not_minimizable(ventana);
    }

    if (mostrar_cursor) zxvision_set_visible_cursor(ventana);

    if (disable_special_chars) ventana->disable_special_chars=1;

    ventana->is_minimized=is_minimized;
    ventana->is_maximized=is_maximized;
    ventana->width_before_max_min_imize=ancho_antes_minimize;
    ventana->height_before_max_min_imize=alto_antes_minimize;

    //printf("is min %d is max %d %dX%d\n",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);

    zxvision_draw_window(ventana);

    //printf ("despues de zxvision_draw_window\n");


    //Decir que se ha pulsado tecla asi no se lee todo cuando el cursor esta visible
    if (ventana->visible_cursor) menu_speech_set_tecla_pulsada();

    int i;

    for (i=0;i<lineas;i++) {
        zxvision_print_string(ventana,1,i,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,buffer_lineas[i]);
        //printf("Linea %d : [%s]\n",i,buffer_lineas[i]);
    }

    zxvision_draw_window_contents(ventana);
}


//Muestra un mensaje en ventana troceando el texto en varias lineas de texto con estilo zxvision
//disable_special_chars: a 1 si queremos desactivar el procesado de caracteres especiales ~~, $$ etc
//volver_timeout: si vale 1, significa timeout normal como ventanas splash. Si vale 2, no finaliza, muestra franjas de color continuamente
//return_after_print_text, si no es 0, se usa para que vuelva a la funcion que llama justo despues de escribir texto,
//usado en opciones de mostrar First Aid y luego agregarle opciones de menu tabladas,
//por lo que agrega cierta altura a la ventana. Se agregan tantas lineas como diga el parametro return_after_print_text
//no_trocear_lineas indica que las lineas no se cortaran en ancho. De momento solo Tape Viewer utiliza el no trocear lineas
void zxvision_generic_message_tooltip(char *titulo, int disable_special_chars, int return_after_print_text,int volver_timeout, int tooltip_enabled,
    int mostrar_cursor, generic_message_tooltip_return *retorno, int resizable, int no_trocear_lineas, const char * texto_format , ...)
{
    //Buffer de entrada

    char *texto=util_malloc_max_texto_generic_message("Can not allocate memory for message");
    va_list args;
    va_start (args, texto_format);
    vsprintf (texto,texto_format, args);
    va_end (args);

    //printf ("input text: %s\n",texto);

    if (volver_timeout) {
        menu_window_splash_counter=0;
        menu_window_splash_counter_ms=0;
    }

    //printf("return_after_print_text: %d\n",return_after_print_text);

    //En caso de stdout, es mas simple, mostrar texto y esperar tecla
    if (menu_es_stdout()) {
        //printf ("%d\n",strlen(texto));


        printf ("-%s-\n",titulo);
        printf ("\n");
        printf ("%s\n",texto);

        scrstdout_menu_print_speech_macro(titulo);
        scrstdout_menu_print_speech_macro(texto);

        menu_espera_no_tecla();
        menu_espera_tecla();

        if (retorno!=NULL) {
            int linea_final;

            //en stdout no podemos seleccionar lineas y por tanto decimos siempre primera linea 0
            linea_final=0;

            //realmente no sabemos el texto de esa linea pues aun no hemos troceado a buffer_lineas. Ponemos ""
            //esto actualmente solo se usa en copy from eprom, por tanto esto no funcionaria,
            //aunque como le decimos que volvemos con ESC, no hemos seleccionado realmente ninguna linea
            strcpy(retorno->texto_seleccionado,"");

            retorno->linea_seleccionada=linea_final;

            // Retorna 1 si sale con enter. Retorna 0 si sale con ESC
            retorno->estado_retorno=0;


        }

        free(texto);

        return;
    }

    //En caso de simpletext, solo mostrar texto sin esperar tecla
    if (!strcmp(scr_new_driver_name,"simpletext")) {
        printf ("-%s-\n",titulo);
        printf ("\n");
        printf ("%s\n",texto);

        free(texto);

        return;
    }


    int tecla;


    //texto que contiene cada linea con ajuste de palabra. Al trocear las lineas aumentan
    char *new_buffer_lineas=util_malloc(MAX_LINEAS_TOTAL_GENERIC_MESSAGE*MAX_ANCHO_LINEAS_GENERIC_MESSAGE,"Can not allocate memory for message");

    //const int max_ancho_texto=NEW_MAX_ANCHO_LINEAS_GENERIC_MESSAGE-2;

    //Ancho de linea inicial en ventana
    int default_ancho_ventana=40;
    int max_ancho_texto=default_ancho_ventana;


    //En principio, el ancho del contenido de la ventana no tiene limite.
    //con 200 caracteres en principio nos aseguramos que habra suficiente
    //De momento solo se usa para Tape Viewer, que es suficiente con 200
    int total_width_cuando_no_trocear=200;

    if (no_trocear_lineas) {
        max_ancho_texto=total_width_cuando_no_trocear;
    }

    int ancho_total_permitido;

    if (menu_ext_desktop_enabled_place_menu()) {
        ancho_total_permitido=menu_get_width_characters_ext_desktop();
    }
    else {
        ancho_total_permitido=ZXVISION_MAX_ANCHO_VENTANA;
    }

    //al menos 30 de ancho
    if (ancho_total_permitido<30) ancho_total_permitido=30;

    //Controlar que el maximo no salga de pantalla
    if (no_trocear_lineas==0) {
        if (max_ancho_texto>ancho_total_permitido-2) max_ancho_texto=ancho_total_permitido-2;
    }

    int longitud=strlen(texto);

    //Copia del texto de entrada (ya formateado con vsprintf) que se leera solo al copiar clipboard
    //Al pulsar tecla de copy a cliboard, se lee el texto que haya aqui,
    //y no el contenido en el char *texto, pues ese se ha alterado quitando saltos de linea y otros caracteres
    char *menu_generic_message_tooltip_text_initial;


    debug_printf(VERBOSE_INFO,"Allocating %d bytes to initial text",longitud+1);
    menu_generic_message_tooltip_text_initial=malloc(longitud+1);
    if (menu_generic_message_tooltip_text_initial==NULL) {
        debug_printf(VERBOSE_ERR,"Can not allocate buffer for initial text");
    }


    //En caso que se haya podido asignar el buffer de clonado
    if (menu_generic_message_tooltip_text_initial!=NULL) {
        strcpy(menu_generic_message_tooltip_text_initial,texto);
    }



    //Crear el array de punteros a cada linea
    int i;

    char *punteros_buffer_lineas[MAX_LINEAS_TOTAL_GENERIC_MESSAGE];

    for (i=0;i<MAX_LINEAS_TOTAL_GENERIC_MESSAGE;i++) {
        punteros_buffer_lineas[i]=&new_buffer_lineas[i*MAX_ANCHO_LINEAS_GENERIC_MESSAGE];
    }

    //Obtener cada linea justificada
    int indice_linea=zxvision_generic_message_aux_justificar_lineas(texto,longitud,max_ancho_texto,punteros_buffer_lineas);

    int ultima_linea_buscada=-1;
    char buffer_texto_buscado[33];
    //printf ("\ntext after converting to lines: %s\n",texto);


    debug_printf (VERBOSE_INFO,"Read %d lines (word wrapped)",indice_linea);

    int linea_a_speech=0;
    int enviar_linea_a_speech=0;



    int alto_ventana=indice_linea+2;

    int alto_total_ventana=indice_linea;

    if (return_after_print_text) {
        //Darle mas altura
        alto_ventana +=return_after_print_text;
        alto_total_ventana +=return_after_print_text;
    }

    if (alto_ventana-2>MAX_LINEAS_VENTANA_GENERIC_MESSAGE) {
        alto_ventana=MAX_LINEAS_VENTANA_GENERIC_MESSAGE+2;
        //texto_no_cabe=1;
    }


    //printf ("alto ventana: %d\n",alto_ventana);
    //int ancho_ventana=max_ancho_texto+2;
    int ancho_ventana;

    if (no_trocear_lineas) {
        ancho_ventana=default_ancho_ventana+2;
    }

    else {
        ancho_ventana=max_ancho_texto+2;
    }



    //printf("max_ancho_texto %d ancho_ventana %d ancho_total_permitido: %d Maximo ancho: %d\n",
    //    max_ancho_texto,ancho_ventana,ancho_total_permitido,ZXVISION_MAX_ANCHO_VENTANA);

    //int xventana=menu_center_x()-ancho_ventana/2;
    int xventana=menu_center_x_from_width(ancho_ventana);
    int yventana=menu_center_y()-alto_ventana/2;

    if (tooltip_enabled==0) {
        menu_espera_no_tecla_con_repeticion();

    }

    zxvision_window *ventana;

    //Se puede usar en el bloque else siguiente
    //Definirla aqui a nivel de funcion y no en el bloque else o seria un error
    zxvision_window mi_ventana;

    if (return_after_print_text) {
        //Dado que vamos a volver con la ventana activa que se crea aquí, hay que asignar la estructura en memoria global
        ventana=malloc(sizeof(zxvision_window));
        //printf ("tamanyo memoria ventana %d\n",sizeof(zxvision_window));
        if (ventana==NULL) cpu_panic("Can not allocate memory for zxvision window");
    }

    else {
        ventana=&mi_ventana;
    }



    zxvision_generic_message_crea_ventana(ventana,disable_special_chars,xventana,yventana,ancho_ventana,alto_ventana,alto_total_ventana,
        titulo,resizable,mostrar_cursor,indice_linea,punteros_buffer_lineas,0,0,ancho_ventana,alto_ventana);


    if (return_after_print_text) {
        free(texto);
        free(new_buffer_lineas);

        if (menu_generic_message_tooltip_text_initial!=NULL) {
            debug_printf(VERBOSE_INFO,"Freeing previous buffer for initial text");
            free(menu_generic_message_tooltip_text_initial);
        }
        return;
    }

    do {



        //Enviar primera linea o ultima a speech

        //La primera linea puede estar oculta por .., aunque para speech mejor que diga esa primera linea oculta
        //debug_printf (VERBOSE_DEBUG,"First line: %s",buffer_lineas[primera_linea]);
        //debug_printf (VERBOSE_DEBUG,"Last line: %s",buffer_lineas[i+primera_linea-1]);

        //printf ("Line to speech: %s\n",buffer_lineas[linea_a_speech]);

        if (enviar_linea_a_speech) {
            menu_speech_reset_tecla_pulsada();
            enviar_linea_a_speech=0;
            menu_textspeech_send_text(punteros_buffer_lineas[linea_a_speech]);
        }



        menu_speech_set_tecla_pulsada();
        enviar_linea_a_speech=0;



        if (!menu_multitarea) {
            //printf ("refresca pantalla\n");
            menu_refresca_pantalla();
        }

        /*else {
            menu_cpu_core_loop();
        }*/

        if (volver_timeout) {
            zxvision_espera_tecla_timeout_window_splash(volver_timeout);
            if (volver_timeout==2) menu_espera_no_tecla();
        }

        else {
            menu_cpu_core_loop();
            menu_espera_tecla();
        }

        tecla=zxvision_read_keyboard();


        //Si se pulsa boton mouse, al final aparece como enter y no es lo que quiero
        //if (tecla==13 && mouse_left && zxvision_keys_event_not_send_to_machine && !mouse_is_dragging) {
        if (tecla==13 && mouse_left) {
            tecla=0;
        }


        if (volver_timeout) tecla=13;



        if (tooltip_enabled==0 && tecla) {
            //printf ("Esperamos no tecla\n");
            menu_espera_no_tecla_con_repeticion();
        }


        int contador_pgdnup;

        int limite_pgdnup;

        //Teclas para teclados simples
        if (zxvision_setting_use_speccy_keys.v && tecla=='5') tecla=8;
        else if (zxvision_setting_use_speccy_keys.v && tecla=='6') tecla=10;
        else if (zxvision_setting_use_speccy_keys.v && tecla=='7') tecla=11;
        else if (zxvision_setting_use_speccy_keys.v && tecla=='8') tecla=9;

        switch (tecla) {

            //Nota: No llamamos a funcion generica zxvision_handle_cursors_pgupdn en caso de arriba,abajo, pg, pgdn,
            //dado que se comporta distinto cuando cursor esta visible

            //abajo
            case 10:
                linea_a_speech=zxvision_generic_message_cursor_down(ventana);

                //printf("Cursor: %d\n",ventana->cursor_line);

                zxvision_sound_event_cursor_movement();

                //Decir que se ha pulsado tecla para que no se relea
                menu_speech_set_tecla_pulsada();
                enviar_linea_a_speech=1;
            break;

            //arriba
            case 11:
                linea_a_speech=zxvision_generic_message_cursor_up(ventana);

                zxvision_sound_event_cursor_movement();

                //Decir que se ha pulsado tecla para que no se relea
                menu_speech_set_tecla_pulsada();
                //primera_linea_a_speech=1;
                enviar_linea_a_speech=1;
            break;


            //izquierda
            case 8:
                zxvision_handle_cursors_pgupdn(ventana,tecla);
                zxvision_sound_event_cursor_movement();
            break;

            //derecha
            case 9:
                zxvision_handle_cursors_pgupdn(ventana,tecla);
                zxvision_sound_event_cursor_movement();
            break;

            //PgUp
            //Home
            case 24:
            case 26:
                limite_pgdnup=ventana->visible_height-2;
                if (tecla==26) limite_pgdnup=ventana->cursor_line+ventana->offset_y;

                //printf("limite_pgdnup: %d\n",limite_pgdnup);

                for (contador_pgdnup=0;contador_pgdnup<limite_pgdnup;contador_pgdnup++) {
                    zxvision_generic_message_cursor_up(ventana);
                }
                zxvision_sound_event_cursor_movement();

                //Decir que no se ha pulsado tecla para que se relea
                menu_speech_reset_tecla_pulsada();

                //Y recargar ventana para que la relea
                zxvision_draw_window_contents(ventana);
            break;

            //PgDn
            //End
            case 25:
            case 27:
                limite_pgdnup=ventana->visible_height-2;

                if (tecla==27) limite_pgdnup=ventana->total_height - ventana->cursor_line - ventana->offset_y;

                //printf("limite_pgdnup: %d\n",limite_pgdnup);

                for (contador_pgdnup=0;contador_pgdnup<limite_pgdnup;contador_pgdnup++) {
                    zxvision_generic_message_cursor_down(ventana);
                }
                zxvision_sound_event_cursor_movement();

                //Decir que no se ha pulsado tecla para que se relea
                menu_speech_reset_tecla_pulsada();

                //Y recargar ventana para que la relea
                zxvision_draw_window_contents(ventana);
            break;

            case 'c':
                menu_copy_clipboard(menu_generic_message_tooltip_text_initial);
                menu_generic_message_splash("Clipboard","Text copied to ZEsarUX clipboard. Go to file utils and press P to paste to a file");

                zxvision_draw_window(ventana);
                zxvision_draw_window_contents(ventana);
            break;


            /*
            Desactivado para evitar confusiones. Mejor hay que hacer antes copy y paste en file utls
                case 's':
                menu_save_text_to_file(menu_generic_message_tooltip_text_initial,"Save Text");
                                                zxvision_draw_window(ventana);
                                zxvision_draw_window_contents(ventana);
            break;
            */



            //Buscar texto
            case 'f':
            case 'n':

                if (tecla=='f' || ultima_linea_buscada==-1) {

                    buffer_texto_buscado[0]=0;
                        menu_ventana_scanf("Text to find",buffer_texto_buscado,33);

                    //ultima_linea_buscada=0; //Si lo pusiera a 0, no encontraria nada en primera linea
                    //pues no se cumpliria la condicion de mas abajo de i>ultima_linea_buscada

                    ultima_linea_buscada=-1;

                }

                int i;
                char *encontrado=NULL;
                for (i=0;i<indice_linea;i++) {
                    debug_printf(VERBOSE_DEBUG,"Searching text on line %d: %s",i,punteros_buffer_lineas[i]);
                    encontrado=util_strcasestr(punteros_buffer_lineas[i], buffer_texto_buscado);
                    if (encontrado && i>ultima_linea_buscada) {
                        break;
                    }
                }

                if (encontrado) {
                    ultima_linea_buscada=i;
                    //mover cursor hasta ahi
                    //primera_linea=0;
                    //linea_cursor=0;

                    //printf ("mover cursor hasta linea: %d\n",ultima_linea_buscada);

                    //Mostramos cursor para poder indicar en que linea se ha encontrado el texto
                    mostrar_cursor=1;

                    zxvision_set_visible_cursor(ventana);

                    zxvision_set_cursor_line(ventana,i);

                    //Si no esta visible, cambiamos offset
                    zxvision_set_offset_y_visible(ventana,i);
                }

                else {
                    menu_speech_reset_tecla_pulsada(); //para decir que siempre se escuchara el mensaje
                    menu_warn_message("Text not found");
                }

                zxvision_draw_window(ventana);
                zxvision_draw_window_contents(ventana);


            break;

            //Movimiento y redimensionado ventana con teclado

            case 'Q':
            case 'A':
            case 'O':
            case 'P':
            case 'W':
            case 'S':
            case 'K':
            case 'L':
                zxvision_handle_cursors_pgupdn(ventana,tecla);
            break;


            default:
                //Si cambia ancho ventana y no hemos pulsado enter para salir
                if (ancho_ventana!=ventana->visible_width && tecla!=13) {
                    //printf("Cambio ancho ventana: %d %d\n",ancho_ventana,ventana->visible_width);

                    ancho_ventana=ventana->visible_width;
                    alto_ventana=ventana->visible_height;
                    xventana=ventana->x;
                    yventana=ventana->y;
                    int is_minimized=ventana->is_minimized;
                    int is_maximized=ventana->is_maximized;
                    int ancho_antes_minimize=ventana->width_before_max_min_imize;
                    int alto_antes_minimize=ventana->height_before_max_min_imize;


                    zxvision_destroy_window(ventana);

                    int ancho_linea=ancho_ventana-2;


                    //Controlar maximo
                    if (ancho_linea>MAX_ANCHO_LINEAS_GENERIC_MESSAGE-2) ancho_linea=MAX_ANCHO_LINEAS_GENERIC_MESSAGE-2;

                    //Y minimo
                    if (ancho_linea<5) ancho_linea=5;

                    if (no_trocear_lineas) {
                        ancho_linea=total_width_cuando_no_trocear;
                    }

                    indice_linea=zxvision_generic_message_aux_justificar_lineas(texto,longitud,ancho_linea,punteros_buffer_lineas);

                    //printf("Total lineas: %d\n",indice_linea);

                    alto_total_ventana=indice_linea;

                    //printf("caso nueva ventana por cambio de ancho. tecla=%d\n",tecla);

                    zxvision_generic_message_crea_ventana(ventana,disable_special_chars,xventana,yventana,ancho_ventana,alto_ventana,alto_total_ventana,
                        titulo,resizable,mostrar_cursor,indice_linea,punteros_buffer_lineas,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);

                }


            break;

        }

    //Salir con Enter o ESC o fin de tooltip
    } while (tecla!=13 && tecla!=2 && tooltip_enabled==0);

    if (retorno!=NULL) {
        int linea_final;

        //printf ("mostrar cursor %d cursor_line %d ventana->offset_x %d\n",mostrar_cursor,ventana->cursor_line,ventana->offset_x);

        if (mostrar_cursor) linea_final=ventana->cursor_line;
        else linea_final=ventana->offset_x;


        strcpy(retorno->texto_seleccionado,punteros_buffer_lineas[linea_final]);
        retorno->linea_seleccionada=linea_final;

        // int estado_retorno; //Retorna 1 si sale con enter. Retorna 0 si sale con ESC
        if (tecla==2) retorno->estado_retorno=0;
        else retorno->estado_retorno=1;

        //printf ("\n\nLinea seleccionada: %d (%s)\n",linea_final,punteros_buffer_lineas[linea_final]);

    }


    zxvision_destroy_window(ventana);



    if (tooltip_enabled==0) menu_espera_no_tecla_con_repeticion();




    if (menu_generic_message_tooltip_text_initial!=NULL) {
        debug_printf(VERBOSE_INFO,"Freeing previous buffer for initial text");
        free(menu_generic_message_tooltip_text_initial);
    }

    free(texto);
    free(new_buffer_lineas);


}

/*
Esto es parte de zxvision_generic_message_tooltip que usamos desde alguna funcion externa
*/
int zxvision_trocear_string_lineas(char *texto,char *buffer_lineas[])
{
    //texto que contiene cada linea con ajuste de palabra. Al trocear las lineas aumentan
    //char buffer_lineas[MAX_LINEAS_TOTAL_GENERIC_MESSAGE][MAX_ANCHO_LINEAS_GENERIC_MESSAGE];

    const int max_ancho_texto=MAX_ANCHO_LINEAS_GENERIC_MESSAGE-2;

    //Primera linea que mostramos en la ventana
    //int primera_linea=0;

    int indice_linea=0;  //Numero total de lineas??
    int indice_texto=0;
    int ultimo_indice_texto=0;
    int longitud=strlen(texto);


    //Copia del texto de entrada (ya formateado con vsprintf) que se leera solo al copiar clipboard
    //Al pulsar tecla de copy a cliboard, se lee el texto que haya aqui,
    //y no el contenido en el char *texto, pues ese se ha alterado quitando saltos de linea y otros caracteres
    char *menu_generic_message_tooltip_text_initial;


    //debug_printf(VERBOSE_INFO,"Allocating %d bytes to initial text",longitud+1);
    menu_generic_message_tooltip_text_initial=malloc(longitud+1);
    if (menu_generic_message_tooltip_text_initial==NULL) {
        debug_printf(VERBOSE_ERR,"Can not allocate buffer for initial text");
    }


    //En caso que se haya podido asignar el buffer de clonado
    if (menu_generic_message_tooltip_text_initial!=NULL) {
        strcpy(menu_generic_message_tooltip_text_initial,texto);
    }



    do {
        indice_texto+=max_ancho_texto;

        //temp
        //printf ("indice_linea: %d\n",indice_linea);

        //Controlar final de texto
        if (indice_texto>=longitud) indice_texto=longitud;

        //Si no, miramos si hay que separar por espacios
        else indice_texto=menu_generic_message_aux_wordwrap(texto,ultimo_indice_texto,indice_texto);

        //Separamos por salto de linea, filtramos caracteres extranyos
        indice_texto=menu_generic_message_aux_filter(texto,ultimo_indice_texto,indice_texto);

        //copiar texto
        int longitud_texto=indice_texto-ultimo_indice_texto;


        //snprintf(buffer_lineas[indice_linea],longitud_texto,&texto[ultimo_indice_texto]);


        menu_generic_message_aux_copia(&texto[ultimo_indice_texto],buffer_lineas[indice_linea],longitud_texto);
        buffer_lineas[indice_linea++][longitud_texto]=0;
        //printf ("copiado %d caracteres desde %d hasta %d: %s\n",longitud_texto,ultimo_indice_texto,indice_texto,buffer_lineas[indice_linea-1]);


        //printf ("texto: %s\n",buffer_lineas[indice_linea-1]);

        if (indice_linea==MAX_LINEAS_TOTAL_GENERIC_MESSAGE) {
                        //cpu_panic("Max lines on menu_generic_message reached");
            debug_printf(VERBOSE_INFO,"Max lines on menu_generic_message reached (%d)",MAX_LINEAS_TOTAL_GENERIC_MESSAGE);
            //finalizamos bucle
            indice_texto=longitud;
        }

        ultimo_indice_texto=indice_texto;
        //printf ("ultimo indice: %d %c\n",ultimo_indice_texto,texto[ultimo_indice_texto]);
        //printf("indice linea: %d\n",indice_linea);

    } while (indice_texto<longitud);

    return indice_linea;

}


//Imprimir mensaje en una ventana ya creada con el texto troceado por espacios
void zxvision_print_mensaje_lineas_troceado(zxvision_window *ventana,char *mensaje_entrada)
{


    int total_lineas;


    //texto que contiene cada linea con ajuste de palabra. Al trocear las lineas aumentan
    char buffer_lineas[MAX_LINEAS_TOTAL_GENERIC_MESSAGE][MAX_ANCHO_LINEAS_GENERIC_MESSAGE];

    //Punteros a cada linea de esas
    char *punteros_lineas[MAX_LINEAS_TOTAL_GENERIC_MESSAGE];


    //Inicializar punteros a lineas
    int i;
    for (i=0;i<MAX_LINEAS_TOTAL_GENERIC_MESSAGE;i++) {
        punteros_lineas[i]=&buffer_lineas[i][0];
    }


    total_lineas=zxvision_trocear_string_lineas(mensaje_entrada,punteros_lineas);



    for (i=0;i<total_lineas;i++) {
            //printf("linea %d : %s\n",i,punteros_lineas[i]);
            zxvision_print_string_defaults_fillspc(ventana,1,i,punteros_lineas[i]);
    }

}






//Retorna 1 si se debe perder 1 de ancho visible por la linea de scroll vertical (lo habitual)
//Retorna 0 si no
int zxvision_get_minus_width_byscrollvbar(zxvision_window *w)
{
    if (w->can_use_all_width==0) return 1;
    else return 0;
}

int zxvision_get_effective_width(zxvision_window *w)
{
    //Ancho del contenido es 1 menos, por la columna a la derecha de margen
    return w->visible_width-zxvision_get_minus_width_byscrollvbar(w);
}

int zxvision_get_effective_height(zxvision_window *w)
{
    //Alto del contenido es 2 menos, por el titulo de ventana y la linea por debajo de margen
    return w->visible_height-2;
}

int zxvision_if_vertical_scroll_bar(zxvision_window *w)
{
    if (!w->can_be_scrolled) return 0;

    //Si esta minimizada, no hay scroll
    if (w->is_minimized) return 0;

    if (w->can_use_all_width==1) {
        //w->applied_can_use_all_width=1;
        return 0;
    }
    int effective_height=zxvision_get_effective_height(w);
    if (w->total_height>effective_height && w->visible_height>=6) return 1;

    return 0;
}

int zxvision_if_horizontal_scroll_bar(zxvision_window *w)
{
    if (!w->can_be_scrolled) return 0;

    //Si esta minimizada, no hay scroll
    if (w->is_minimized) return 0;

    int effective_width=zxvision_get_effective_width(w);
    if (w->total_width>effective_width && w->visible_width>=6) return 1;

    return 0;
}

void zxvision_draw_vertical_scroll_bar(zxvision_window *w,int estilo_invertido)
{

    int effective_height=zxvision_get_effective_height(w);
        //Dibujar barra vertical
        int valor_parcial=w->offset_y+effective_height;
        if (valor_parcial<0) valor_parcial=0;

        //Caso especial arriba del todo cero, valor_parcial es 0 y no contemplamos el alto visible
        //if (w->offset_y==0) valor_parcial=0;

        int valor_total=w->total_height;
        if (valor_total<=0) valor_total=1; //Evitar divisiones por cero o negativos


        int porcentaje=(valor_parcial*100)/(1+valor_total);

        //Caso especial arriba del todo
        if (w->offset_y==0) {
            //printf ("Scroll vertical cursor is at the minimum\n");
            porcentaje=0;
        }

        //Caso especial abajo del todo
        if (w->offset_y+(w->visible_height)-2==w->total_height) { //-2 de perder linea titulo y linea scroll
            //printf ("Scroll vertical cursor is at the maximum\n");
            porcentaje=100;
        }

        menu_ventana_draw_vertical_perc_bar(w,w->x,w->y,w->visible_width,w->visible_height-1,porcentaje,estilo_invertido);
}

void zxvision_draw_horizontal_scroll_bar(zxvision_window *w,int estilo_invertido)
{

    int effective_width=w->visible_width-1;
    //Dibujar barra horizontal
        int valor_parcial=w->offset_x+effective_width;
        if (valor_parcial<0) valor_parcial=0;

        //Si offset es cero, valor_parcial es 0 y no contemplamos el ancho visible
        //if (w->offset_x==0) valor_parcial=0;

        int valor_total=w->total_width;
        if (valor_total<=0) valor_total=1; //Evitar divisiones por cero o negativos


        int porcentaje=(valor_parcial*100)/(1+valor_total);

        //Caso especial izquierda del todo
        if (w->offset_x==0) {
            //printf ("Scroll horizontal cursor is at the minimum\n");
            porcentaje=0;
        }

        //Caso especial derecha del todo
        if (w->offset_x+(w->visible_width)-1==w->total_width) { //-1 de perder linea scroll
            //printf ("Scroll horizontal cursor is at the maximum\n");
            porcentaje=100;
        }

        menu_ventana_draw_horizontal_perc_bar(w,w->x,w->y,effective_width,w->visible_height,porcentaje,estilo_invertido);
}

void zxvision_draw_scroll_bars(zxvision_window *w)
{
    //Barras de desplazamiento
    //Si hay que dibujar barra derecha de desplazamiento vertical
    //int effective_height=zxvision_get_effective_height(w);
    //int effective_width=zxvision_get_effective_width(w);
    //int effective_width=w->visible_width-1;

    if (!w->can_be_scrolled) return;

    if (zxvision_if_vertical_scroll_bar(w)) {
        zxvision_draw_vertical_scroll_bar(w,0);
    }

    if (zxvision_if_horizontal_scroll_bar(w)) {
        zxvision_draw_horizontal_scroll_bar(w,0);
    }
}

void zxvision_set_mark_tried_write_beyond_size(zxvision_window *w)
{
    if (w->do_not_warn_tried_write_beyond_size==0) {
        cuadrado_activo_marca_redimensionado_aviso=1;
    }
}

void zxvision_draw_window(zxvision_window *w)
{

    if (!zxvision_show_minimized(w)) return;

    menu_dibuja_ventana(w->x,w->y,w->visible_width,w->visible_height,w->window_title);


    //Ver si se puede redimensionar
    //Dado que cada vez que se dibuja ventana, la marca de resize se establece por defecto a desactivada
    cuadrado_activo_resize=w->can_be_resized;
    ventana_activa_puede_minimizar=w->can_be_minimized;
    //ventana_activa_tipo_zxvision=1;

    //Avisar que se ha intentado escribir mas alla del tamanyo ventana, pero siempre que
    //la ventana no haya desactivado esto
    cuadrado_activo_marca_redimensionado_aviso=0;

    if (w->tried_write_beyond_size) {
        zxvision_set_mark_tried_write_beyond_size(w);
    }


    //Si no hay barras de desplazamiento, alterar scroll horiz o vertical segun corresponda
    if (!zxvision_if_horizontal_scroll_bar(w)) {
        //printf ("no hay barra scroll horizontal y por eso ponemos offset x a 0\n");
        w->offset_x=0;
    }
    if (!zxvision_if_vertical_scroll_bar(w)) {
        //printf ("no hay barra scroll vertical y por eso ponemos offset y a 0\n");
        w->offset_y=0;
    }

    zxvision_draw_scroll_bars(w);

    //Mostrar boton de minimizar
    menu_dibuja_ventana_botones();

    //Mostrar boton background
    menu_dibuja_ventana_boton_background(w->x,w->y,w->visible_width,w);

    //Dado que se ha borrado el contenido, luego en zxvision_draw_window_contents hay que refrescar
    zxvision_set_flag_dirty_must_draw_contents(w);


}

void zxvision_set_not_minimizable(zxvision_window *w)
{
    //Decimos que no se puede minimizar
    //printf ("set not resizable\n");
    ventana_activa_puede_minimizar=0;

    w->can_be_minimized=0;
}

void zxvision_set_not_resizable(zxvision_window *w)
{
    //Decimos que no se puede redimensionar
    //printf ("set not resizable\n");
    cuadrado_activo_resize=0;

    w->can_be_resized=0;
}

void zxvision_set_resizable(zxvision_window *w)
{
    //Decimos que se puede redimensionar
    //printf ("set resizable\n");
    cuadrado_activo_resize=1;

    w->can_be_resized=1;
}

int zxvision_maximum_offset_x(zxvision_window *w)
{
    int offset_x=w->total_width+1-w->visible_width;

    //+1 porque se pierde 1 a la derecha con la linea scroll

    //Si se pasa por la izquierda
    if (offset_x<0) offset_x=0;

    return offset_x;
}

void zxvision_set_offset_x(zxvision_window *w,int offset_x)
{

    if (!w->can_be_scrolled) return;

    int maximum_offset=zxvision_maximum_offset_x(w);

    //Si se pasa por la derecha
    if (offset_x>maximum_offset) {
        offset_x=maximum_offset;
    }

    //Si se pasa por la izquierda
    if (offset_x<0) offset_x=0;

    w->offset_x=offset_x;

    if (w->no_refresh_change_offset==0) {
        //borrar cache por si hay restos de pixeles
        w->must_clear_cache_on_draw_once=1;
        zxvision_draw_window_contents(w);
    }
    zxvision_draw_scroll_bars(w);

    zxvision_set_flag_dirty_must_draw_contents(w);
}

int zxvision_maximum_offset_y(zxvision_window *w)
{
    int offset_y=w->total_height+2-w->visible_height;

    //Si se pasa por arriba
    if (offset_y<0) offset_y=0;

    return offset_y;
}

void zxvision_set_offset_y(zxvision_window *w,int offset_y)
{

    if (!w->can_be_scrolled) return;

    //Si se pasa por abajo

    //if (offset_y+w->visible_height-2 > (w->total_height ) ) return; //-2 porque se pierde 2 linea scroll y la linea titulo
    int maximum_offset=zxvision_maximum_offset_y(w);

    if (offset_y>maximum_offset) {
        //printf ("Maximum offset y reached\n");
        offset_y=maximum_offset;
    }

    //Si se pasa por arriba
    if (offset_y<0) offset_y=0;

    w->offset_y=offset_y;

    if (w->no_refresh_change_offset==0) {
        //borrar cache por si hay restos de pixeles
        w->must_clear_cache_on_draw_once=1;
        zxvision_draw_window_contents(w);
    }
    zxvision_draw_scroll_bars(w);

    zxvision_set_flag_dirty_must_draw_contents(w);

}

void zxvision_set_offset_y_or_maximum(zxvision_window *w,int offset_y)
{
        int maximum_offset=zxvision_maximum_offset_y(w);

        if (offset_y>maximum_offset) {
                //printf ("Maximum offset y reached. Setting maximum\n");
        offset_y=maximum_offset;
        }

    zxvision_set_offset_y(w,offset_y);
}



//Si no esta visible, cambiamos offset
void zxvision_set_offset_y_visible(zxvision_window *w,int y)
{

    int linea_final;

    //El cursor esta por arriba. Decimos que este lo mas arriba posible
    if (y<w->offset_y) {
        linea_final=y;
        //printf ("adjust verticall scroll por arriba to %d\n",linea_final);

    }

    //El cursor esta por abajo. decimos que el cursor este lo mas abajo posible
    else if (y>=w->offset_y+w->visible_height-2) {
        linea_final=y-(w->visible_height-2)+1;
        //Ejemplo
        //total height 12
        //visble 10->efectivos son 8
        //establecemos a linea 7
        //linea_final=7-(10-2)+1 = 7-8+1=0

        //printf ("adjust verticall scroll por abajo to %d\n",linea_final);
    }

    else return;

    int ultima_linea_scroll=w->total_height-(w->visible_height-2);

    /*
    Ejemplo: visible_height 10-> efectivos son 8
    total_height 12
    podremos hacer 4 veces scroll
    12-(10-2)=12-8=4
    */

    if (ultima_linea_scroll<0) ultima_linea_scroll=0;
    if (linea_final>ultima_linea_scroll) linea_final=ultima_linea_scroll;

    //printf ("final scroll %d\n",linea_final);

    zxvision_set_offset_y(w,linea_final);


}


void zxvision_send_scroll_down(zxvision_window *w)
{
    if (w->offset_y<(w->total_height-1)) {
                        zxvision_set_offset_y(w,w->offset_y+1);
    }
}


void zxvision_send_scroll_up(zxvision_window *w)
{
    if (w->offset_y>0) {
        zxvision_set_offset_y(w,w->offset_y-1);
    }
}


void zxvision_send_scroll_left(zxvision_window *w)
{
    if (w->offset_x>0) {
        zxvision_set_offset_x(w,w->offset_x-1);
    }
}

void zxvision_send_scroll_right(zxvision_window *w)
{
    if (w->offset_x<(w->total_width-1)) {
        zxvision_set_offset_x(w,w->offset_x+1);
    }
}

int zxvision_cursor_out_view(zxvision_window *ventana)
{

    //int linea_retornar;

    if (ventana->visible_cursor) {

        //Ver en que offset estamos
        int offset_y=ventana->offset_y;
        //Y donde esta el cursor
        int cursor=ventana->cursor_line;

        //Y si cursor no esta visible, lo ponemos para que este abajo del todo (hemos de suponer que estaba abajo y ha bajado 1 mas)
        if (cursor<offset_y || cursor>=offset_y+ventana->visible_height-2) {
            return 1;
        }
    }

        return 0;

}

void zxvision_set_cursor_line(zxvision_window *ventana,int linea)
{
    //printf("llamado a zxvision_set_cursor_line linea=%d\n",linea);
    ventana->cursor_line=linea;

    zxvision_set_flag_dirty_must_draw_contents(ventana);
}

void zxvision_inc_cursor_line(zxvision_window *ventana)
{
    ventana->cursor_line++;

    zxvision_set_flag_dirty_must_draw_contents(ventana);
}

void zxvision_dec_cursor_line(zxvision_window *ventana)
{
    ventana->cursor_line--;

    zxvision_set_flag_dirty_must_draw_contents(ventana);
}

void zxvision_set_visible_cursor(zxvision_window *ventana)
{
    ventana->visible_cursor=1;

    zxvision_set_flag_dirty_must_draw_contents(ventana);
}

void zxvision_reset_visible_cursor(zxvision_window *ventana)
{
    ventana->visible_cursor=0;

    zxvision_set_flag_dirty_must_draw_contents(ventana);
}

//Retorna 1 si ha reajustado el cursor
int zxvision_adjust_cursor_bottom(zxvision_window *ventana)
{

    //int linea_retornar;

    if (zxvision_cursor_out_view(ventana)) {

        //Ver en que offset estamos
        int offset_y=ventana->offset_y;
        //Y donde esta el cursor
        //int cursor=ventana->cursor_line;

        //printf ("Reajustar cursor\n");
        zxvision_set_cursor_line(ventana,offset_y+ventana->visible_height-2-ventana->upper_margin-ventana->lower_margin);
        return 1;
    }

    return 0;

}

//Retorna 1 si ha reajustado el cursor
int zxvision_adjust_cursor_top(zxvision_window *ventana)
{

    if (zxvision_cursor_out_view(ventana)) {

        //Ver en que offset estamos
        int offset_y=ventana->offset_y;
        //Y donde esta el cursor
        //int cursor=ventana->cursor_line;

            if (offset_y>0) {
                //printf ("Reajustar cursor\n");
                zxvision_set_cursor_line(ventana,offset_y-1);
                return 1;
            }

    }

    return 0;


}

int zxvision_out_bonds(int x,int y,int ancho,int alto)
{
    if (x<0 || y<0) return 1;


    if (zxvision_allow_windows_beyond_limit.v) {
        //Permitir ventanas que salgan del ZX Desktop
        //Minimo 2x2
        ancho=2;
        alto=2;
    }

    //No permitir ventanas que salgan del ZX Desktop
    if (x+ancho>scr_get_menu_width() || y+alto>scr_get_menu_height()) return 1;

    return 0;
}



//Dibujar todas las ventanas que hay debajo de esta en cascada, desde la mas antigua hasta arriba
void zxvision_draw_below_windows(zxvision_window *w)
{
    //Primero ir a buscar la de abajo del todo
    zxvision_window *pointer_window;

    //printf ("original window: %p\n",w);
        //printf ("\noriginal window: %p. Title: %s\n",w,w->window_title);




    pointer_window=zxvision_find_first_window_below_this(w);

    //printf ("after while pointer_window->previous_window!=NULL\n");

    int antes_ventana_tipo_activa=ventana_tipo_activa;
    zxvision_reset_ventana_tipo_activa(); //Redibujar las de debajo como inactivas

    //Redibujar diciendo que estan por debajo
    ventana_es_background=1;

    //Y ahora de ahi hacia arriba
    //Si puntero es NULL, es porque se ha borrado alguna ventana de debajo. Salir
    //esto puede suceder haciendo esto:
    //entrar a debug cpu-breakpoints. activarlo y dejar que salte el tooltip
    //ir a ZRCP. Meter breakpoint que de error, ejemplo: "sb 1 pc=kkkk("
    //ir a menu. enter y enter. Se provoca esta situacion. Por que? Probablemente porque se ha llamado a destroy window y
    //se ha generado una ventana de error cuando habia un tooltip abierto
    //Ver comentarios en zxvision_destroy_window

    //if (pointer_window==NULL) {
    //	printf ("Pointer was null before loop redrawing below windows\n");
    //}

    //printf ("\nStart loop redrawing below windows\n");

    zxvision_window *always_visible_window=NULL;

    //no mostrar mensajes de error pendientes
    //si eso se hiciera, aparece en medio de la lista de ventanas una que apunta a null y de ahi la condicion pointer_window!=NULL
    //asi entonces dicha condicion pointer_window!=NULL ya no seria necesaria pero la dejamos por si acaso...
    int antes_no_dibuja_ventana_muestra_pending_error_message=no_dibuja_ventana_muestra_pending_error_message;
    no_dibuja_ventana_muestra_pending_error_message=1;

    while (pointer_window!=w && pointer_window!=NULL) {
        //printf ("window from bottom to top %p\n",pointer_window);
        //printf ("window from bottom to top %p. name: %s\n",pointer_window,pointer_window->window_title);

        debug_printf(VERBOSE_DEBUG,"Redrawing window %s",pointer_window->window_title);

        zxvision_draw_window(pointer_window);
        //printf("Decimos tecla pulsada\n");
        //menu_speech_set_tecla_pulsada();
        zxvision_draw_window_contents(pointer_window);
        //printf("Despues de tecla pulsada: menu_speech_tecla_pulsada: %d\n",menu_speech_tecla_pulsada);

        if (pointer_window->always_visible) always_visible_window=pointer_window;


        pointer_window=pointer_window->next_window;
    }


    //Si hay alguna que tiene always visible, redibujarla encima solo la ventana (el contenido ya se habra mostrado en teoria)
    //Nota: en este caso se dibujaria la visible que esta mas arriba de todas
    if (always_visible_window!=NULL) {
        //printf("Redibujando arriba del todo ventana %s\n",always_visible_window->window_title);
        zxvision_draw_window(always_visible_window);
    }


    no_dibuja_ventana_muestra_pending_error_message=antes_no_dibuja_ventana_muestra_pending_error_message;



    ventana_es_background=0;
    ventana_tipo_activa=antes_ventana_tipo_activa;
}


//int zxvision_drawing_in_background=0;


//Llama al overlay de la ventana, si es que existe
int zxvision_draw_overlay_if_exists(zxvision_window *w)
{
        void (*overlay_function)(void);
        overlay_function=w->overlay_function;

        //printf ("Funcion overlay: %p. ventana: %s. current window: %p\n",overlay_function,w->window_title,zxvision_current_window);


        //Esto pasa en ventanas que por ejemplo actualizan no a cada frame, al menos refrescar aqui con ultimo valor

        if (overlay_function!=NULL) {
            //printf ("llamando a funcion overlay %p\n",overlay_function);

            overlay_function(); //llamar a funcion overlay
            return 1;
        }
        else {
            //printf ("NO llamando a funcion overlay %p\n",overlay_function);
            return 0;
        }
}

//Tiempo total transcurrido dibujando overlays
//esto ya lo tenemos con core_render_menu_overlay_difftime
//long zxvision_time_total_drawing_overlay=0;

//tiempo total transcurrido dibujando overlays excepto ventana actual
long zxvision_time_total_drawing_overlay_except_current=0;

//Dibujar todos los overlay de las ventanas que hay debajo de esta en cascada, desde la mas antigua hasta arriba, pero llamando solo las que tienen overlay
void zxvision_draw_overlays_below_windows(zxvision_window *w)
{
    //printf ("drawing overlay start---------\n");

    //Primero ir a buscar la de abajo del todo
    zxvision_window *pointer_window;


    //if (w!=NULL) printf ("\nDraw below windows with overlay. original window: %p. Title: %s\n",w,w->window_title);


    //Si no hay ventanas, volver
    if (zxvision_current_window==NULL) return;

    pointer_window=w;

    while (pointer_window->previous_window!=NULL) {
            //debug_printf (VERBOSE_PARANOID,"zxvision_draw_overlays_below_windows below window: %p",pointer_window->previous_window);
            pointer_window=pointer_window->previous_window;
    }

    int antes_ventana_tipo_activa=ventana_tipo_activa;
    zxvision_reset_ventana_tipo_activa(); //Redibujar las de debajo como inactivas

    //Redibujar diciendo que estan por debajo
    ventana_es_background=1;

    //Y ahora de ahi hacia arriba, incluido la ultima


    //printf ("\n");

    //zxvision_drawing_in_background=1;


    zxvision_time_total_drawing_overlay_except_current=0;


    //Dibujar todas ventanas excepto la de mas arriba.
    //while (pointer_window!=w && pointer_window!=NULL) {

    //Dibujar todas ventanas.
    while (pointer_window!=NULL) {
        //while (pointer_window!=w) {
                //printf ("window from bottom to top %p. next: %p nombre: %s\n",pointer_window,pointer_window->next_window,pointer_window->window_title);

        //Somos la ventana de mas arriba
        if (pointer_window==w) {
            ventana_es_background=0;
            ventana_tipo_activa=antes_ventana_tipo_activa;
        };

        //en principio no hace falta. Ya se redibuja por el redibujado normal
        //zxvision_draw_window(pointer_window);

        //Dibujamos contenido anterior, ya que draw_window la borra con espacios
        //en principio no hace falta. Ya se redibuja por el redibujado normal
        //zxvision_draw_window_contents(pointer_window);

        //printf ("drawing overlay %p name: %s\n",pointer_window,pointer_window->window_title);

        struct timeval zxvision_time_antes,zxvision_time_despues;

        //calcular tiempo que tarda en dibujarse

        timer_stats_current_time(&zxvision_time_antes);


        int existe=zxvision_draw_overlay_if_exists(pointer_window);

        //TODO: posible implementacion para parametro frameskip de cada overlay de zxvision_window

        if (existe) {
            //printf("existe\n");
            long transcurrido=timer_stats_diference_time(&zxvision_time_antes,&zxvision_time_despues);
            pointer_window->last_spent_time_overlay=transcurrido;
            //printf ("tiempo transcurrido: %ld microsec\n\n",transcurrido);

            zxvision_time_total_drawing_overlay_except_current +=transcurrido;
        }
        else {
            //printf("no existe\n");
            //Si no tiene overlay se puede decir que el tiempo transcurrido es tal cual, 0
            pointer_window->last_spent_time_overlay=0;
        }




        pointer_window=pointer_window->next_window;
    }

    //if (w!=NULL) printf ("\nEND Draw below windows with overlay. original window: %p. Title: %s\n\n",w,w->window_title);

    //printf ("tiempo TOTAL transcurrido: %ld microsec\n",zxvision_time_total_drawing_overlay);

    //zxvision_drawing_in_background=0;

    ventana_es_background=0;
    ventana_tipo_activa=antes_ventana_tipo_activa;

}

void old_zxvision_draw_overlays_below_windows(zxvision_window *w)
{


        //Primero ir a buscar la de abajo del todo
        zxvision_window *pointer_window;


        //if (w!=NULL) printf ("\nDraw with overlay. original window: %p. Title: %s\n",w,w->window_title);


        //Si no hay ventanas, volver
        if (zxvision_current_window==NULL) return;

        pointer_window=w;

        while (pointer_window->previous_window!=NULL) {
                        //debug_printf (VERBOSE_PARANOID,"zxvision_draw_overlays_below_windows below window: %p",pointer_window->previous_window);
                        pointer_window=pointer_window->previous_window;
        }

        int antes_ventana_tipo_activa=ventana_tipo_activa;
        zxvision_reset_ventana_tipo_activa(); //Redibujar las de debajo como inactivas

        //Redibujar diciendo que estan por debajo
        ventana_es_background=1;

        //Y ahora de ahi hacia arriba, incluido la ultima


        //printf ("\n");

        //zxvision_drawing_in_background=1;

        //Dibujar todas ventanas excepto la de mas arriba.
        //while (pointer_window!=w && pointer_window!=NULL) {

        //Dibujar todas ventanas.
        while (pointer_window!=NULL) {
                //while (pointer_window!=w) {
                                //printf ("window from bottom to top %p. next: %p nombre: %s\n",pointer_window,pointer_window->next_window,pointer_window->window_title);

                //Somos la ventana de mas arriba
                if (pointer_window==w) {
                        ventana_es_background=0;
                        ventana_tipo_activa=antes_ventana_tipo_activa;
                };

                //en principio no hace falta. Ya se redibuja por el redibujado normal
                //zxvision_draw_window(pointer_window);

                //Dibujamos contenido anterior, ya que draw_window la borra con espacios
                //en principio no hace falta. Ya se redibuja por el redibujado normal
                //zxvision_draw_window_contents(pointer_window);


                zxvision_draw_overlay_if_exists(pointer_window);



                pointer_window=pointer_window->next_window;
        }



        //zxvision_drawing_in_background=0;

        ventana_es_background=0;

    ventana_tipo_activa=antes_ventana_tipo_activa;

}

void zxvision_message_put_window_background(void)
{
    //Conviene esperar no tecla porque a veces esta ventana splash no aparece
    menu_espera_no_tecla();
    //menu_generic_message_splash("Background task","OK. Window put on the background");
    debug_printf (VERBOSE_DEBUG,"OK. Window put on the background");


    //si opcion background windows even with menu closed
    //cerrar todos menus, y decir al gestor de ventanas que se ha ido a background
    //esto afecta tanto si estamos desde el gestor de ventanas (conmutando ventanas) o si abrimos un menu directo
    //por ejemplo display->view color palettes, y pulsamos F6
    //Nota: quiza comprobar menu_allow_background_windows && menu_multitarea no tiene sentido,
    //pues si la ventana ha recibido esta accion de background, es que tanto la multitarea como el background estan permitidos
    if (menu_allow_background_windows && menu_multitarea && always_force_overlay_visible_when_menu_closed) {
        menu_window_manager_window_went_background=1;
        salir_todos_menus=1;
    }


}

//Pone en la estructura de ventana la funcion de overlay que haya activa ahora
//Siempre que no sea la de normal overlay
void zxvision_set_window_overlay_from_current(zxvision_window *ventana)
{

    /*
    realmente comparar con la de normal overlay nos sirve para evitar que ventanas que no tienen overlay
    pero que pueden ir a background (como debug cpu) les ponga como overlay el propio de normal overlay
    Es un poco chapucero pero funciona
    TODO: seria mejor indicar con un flag (por defecto a 0) que la ventana tiene un overlay activo diferente de normal_overlay
    */
    if (menu_overlay_function!=normal_overlay_texto_menu) {
        ventana->overlay_function=menu_overlay_function;
    }
}

//Pone en la estructura de ventana la funcion de overlay indicada
void zxvision_set_window_overlay(zxvision_window *ventana,void (*overlay_function) (void))
{
    ventana->overlay_function=overlay_function;
}

//Quita de la estructura de ventana la funcion de overlay
void zxvision_reset_window_overlay(zxvision_window *ventana)
{
    ventana->overlay_function=NULL;
}

void zxvision_redraw_window_on_move_if_disable_speech(zxvision_window *w,int disable_speech)
{
    cls_menu_overlay();
    zxvision_draw_below_windows_nospeech(w);

    debug_printf(VERBOSE_DEBUG,"Redrawing window %s",w->window_title);

    if (disable_speech) {
        //no hacer speech tampoco de la ventana actual
        menu_speech_set_tecla_pulsada();
    }

    zxvision_draw_window(w);
    zxvision_draw_window_contents(w);
}

void zxvision_redraw_window_on_move(zxvision_window *w)
{
    zxvision_redraw_window_on_move_if_disable_speech(w,0);
}

void zxvision_redraw_all_windows(void)
{
    if (zxvision_current_window!=NULL) {
        zxvision_redraw_window_on_move(zxvision_current_window);
    }
}

void zxvision_redraw_all_windows_no_speech(void)
{
    if (zxvision_current_window!=NULL) {
        zxvision_redraw_window_on_move_if_disable_speech(zxvision_current_window,1);
    }
}

void zxvision_set_x_position(zxvision_window *w,int x)
{
    if (zxvision_out_bonds(x,w->y,w->visible_width,w->visible_height)) return;

    w->x=x;
    zxvision_redraw_window_on_move(w);

    zxvision_set_all_flag_dirty_must_draw_contents();

}

void zxvision_set_y_position(zxvision_window *w,int y)
{
    if (zxvision_out_bonds(w->x,y,w->visible_width,w->visible_height)) return;

    w->y=y;
    zxvision_redraw_window_on_move(w);

    zxvision_set_all_flag_dirty_must_draw_contents();

}

//copiar contenido al aumentar la ventana
void zxvision_copy_contents_on_enlarge(overlay_screen *origen,overlay_screen *destino,int ancho_previo,int alto_previo,int ancho_final)
{
    int x,y;

    for (y=0;y<alto_previo;y++) {
        for (x=0;x<ancho_previo;x++) {
            int offset_origen=(y*ancho_previo)+x;
            int offset_final=(y*ancho_final)+x;

            /*
struct s_overlay_screen {
        int tinta,papel,parpadeo;
        z80_byte caracter;
};
            */
           destino[offset_final].tinta=origen[offset_origen].tinta;
           destino[offset_final].papel=origen[offset_origen].papel;
           destino[offset_final].parpadeo=origen[offset_origen].parpadeo;
           destino[offset_final].caracter=origen[offset_origen].caracter;
        }
    }
}

//funcion comun para reasignar el contenido de la ventana al aumentar en ancho o alto
//Si ancho_alto=1, redimensiona en ancho
//si ancho_alto=0, redimensiona en alto
//nueva_dimension indica nuevo alto o nuevo ancho
void zxvision_enlarge_common(zxvision_window *w,int ancho_alto,int nueva_dimension)
{

    if (nueva_dimension<1) return;

    int redimensionar=0;

    //Si se hace mas grande en ancho, recrearla
    if (ancho_alto) {
        if (nueva_dimension>w->total_width) {
            //debug_printf(VERBOSE_DEBUG,"Reallocate window memory to enlarge width");
            redimensionar=1;
        }
    }

    //Si se hace mas grande en alto, recrearla
    else {
        if (nueva_dimension>w->total_height) {
            //debug_printf(VERBOSE_DEBUG,"Reallocate window memory to enlarge height");
            redimensionar=1;
        }
    }

    if (redimensionar) {

        overlay_screen *puntero_antes_memory=w->memory;
        int previous_total_width=w->total_width;
        int previous_total_height=w->total_height;

        if (ancho_alto) {
            w->total_width=nueva_dimension;
            debug_printf(VERBOSE_DEBUG,"Reallocate window memory to enlarge width. New total_width: %d",nueva_dimension);
        }
        else {
            w->total_height=nueva_dimension;
            debug_printf(VERBOSE_DEBUG,"Reallocate window memory to enlarge height. New total_height: %d",nueva_dimension);
        }

        //Asignamos memoria para la nueva
        zxvision_alloc_memory(w,w->total_width,w->total_height);

        //Copiamos el contenido previo de la ventana al nuevo
        zxvision_copy_contents_on_enlarge(puntero_antes_memory,w->memory,previous_total_width,previous_total_height,
            w->total_width);

        //Liberar memoria del contenido de ventana previo
        free(puntero_antes_memory);
    }


}

//Funcion que cambia ancho visible pero tambien ancho total de la ventana,
//que se incrementa si se hace la ventana mas ancha (pero no se decrementa nunca)
void zxvision_set_visible_width(zxvision_window *w,int visible_width)
{
    if (zxvision_out_bonds(w->x,w->y,visible_width,w->visible_height)) {
        //printf ("Window out of bounds trying to set width\n");
        return;
    }

    if (visible_width<1) return;

    if (w->contents_can_be_enlarged) {
        debug_printf(VERBOSE_DEBUG,"Window set visible width new: %d previous total_width: %d",visible_width,w->total_width);

        //Asumimos que ancho total deberia ser al menos el ancho visible-1 (dado que no siempre se usa columna final de scroll)
        int ancho_deseado=visible_width-1;

        //Si se usa columna de scroll
        if (w->can_use_all_width) ancho_deseado++;

        zxvision_enlarge_common(w,1,ancho_deseado);

    }

    w->visible_width=visible_width;
    zxvision_redraw_window_on_move(w);

    zxvision_set_all_flag_dirty_must_draw_contents();
    w->tried_write_beyond_size=0;

}

//Funcion que cambia alto visible pero tambien alto total de la ventana,
//que se incrementa si se hace la ventana mas alta (pero no se decrementa nunca)
void zxvision_set_visible_height(zxvision_window *w,int visible_height)
{
    if (zxvision_out_bonds(w->x,w->y,w->visible_width,visible_height)) return;

    if (visible_height<1) return;

    if (w->contents_can_be_enlarged) {
        debug_printf(VERBOSE_DEBUG,"Window set visible height new: %d previous total_height: %d",visible_height,w->total_height);

        //Asumimos que alto total deberia ser al menos el alto visible-2 (dado que la primera linea es el titulo de ventana y la ultima el scroll)
        zxvision_enlarge_common(w,0,visible_height-2);

    }

    w->visible_height=visible_height;
    zxvision_redraw_window_on_move(w);

    zxvision_set_all_flag_dirty_must_draw_contents();
    w->tried_write_beyond_size=0;

}

//Funcion que cambia alto total de la ventana
//Corresponde a la funcion que llama aqui, redibujar la ventana, idealmente llamando a:
//	zxvision_redraw_window_on_move(w);
//zxvision_set_all_flag_dirty_must_draw_contents();
//Esto se hace asi para que desde donde llama, pueda ser un overlay de ventana, sin tener que redibujar
//ni alterar otra escritura de ventana que este por encima
void zxvision_set_total_height(zxvision_window *w,int total_height)
{

    if (total_height<1) return;

    if (w->contents_can_be_enlarged) {
        debug_printf(VERBOSE_DEBUG,"Window set total height new: %d previous total_height: %d",total_height,w->total_height);

        zxvision_enlarge_common(w,0,total_height);

    }

    w->total_height=total_height;


}

//Ajustar el alto de ventana al minimo necesario (o sea, cortar lineas en blanco por abajo)
int zxvision_resize_minimum_height_get_max_y(zxvision_window *w)
{

    int y,x;

    for (y=w->total_height-1;y>=0;y--) {
        int offset_caracter=y*(w->total_width);

        for (x=0;x<(w->total_width);x++) {

            overlay_screen *caracter;
            caracter=w->memory;
            caracter=&caracter[offset_caracter];

            z80_byte caracter_escribir=caracter->caracter;

            if (caracter_escribir!=32) return y;
            offset_caracter++;
        }

    }

    return 0;


}

//Ajustar el alto de ventana al minimo necesario (o sea, cortar lineas en blanco por abajo)
void zxvision_resize_minimum_height(zxvision_window *w)
{

    int linea=zxvision_resize_minimum_height_get_max_y(w);

    zxvision_set_visible_height(w,linea+3);
    zxvision_set_total_height(w,linea+1);

}





/*char *zxvision_get_text_margin(zxvision_window *w,int linea)
{
    int i;
    char *text_margin;
    for (i=0;i<linea;i++) {
        text_margin=w->text_margin[linea];
        if (text_margin==NULL) return NULL;
    }

    return text_margin;

}*/

void zxvision_draw_window_contents_stdout(zxvision_window *w)
{
    //Simple. Mostrarlo todo
    int y,x;


    //Simple. Mostrar todas lineas
    char buffer_linea[MAX_BUFFER_SPEECH+1];


    for (y=0;y<w->total_height;y++) {
        int offset_caracter=y*w->total_width;



        for (x=0;x<w->total_width && x<MAX_BUFFER_SPEECH;x++) {

                                overlay_screen *caracter;
                                caracter=w->memory;
                                caracter=&caracter[offset_caracter];

                                z80_byte caracter_escribir=caracter->caracter;

            buffer_linea[x]=caracter_escribir;
            offset_caracter++;
        }

        buffer_linea[x]=0;


                printf ("%s\n",buffer_linea);

                scrstdout_menu_print_speech_macro(buffer_linea);

        }


    //menu_espera_no_tecla();
    //menu_espera_tecla();
}


//Dice si unas coordenadas están dentro de una ventana concreta
int zxvision_coords_in_window(zxvision_window *w,int x,int y)
{

    if (!zxvision_show_minimized(w)) return 0;

    int other_x=w->x;
    int other_y=w->y;
    int other_width=w->visible_width;
    int other_height=w->visible_height;

    //printf ("x %d y %d other x %d y %d w %d h %d\n",x,y,other_x,other_y,other_width,other_height);

    if (x>=other_x && x<other_x+other_width &&
        y>=other_y && y<other_y+other_height
        )
        {
            return 1;
        }

    return 0;

}

//Dice si las coordenadas de ventana indicada coinciden con cualquiera de las ventanas que tenga encima
int zxvision_coords_in_superior_windows(zxvision_window *w,int x,int y)
{
    //Si esta ventana tiene flag always_visible, siempre escribe
    if (w->always_visible) return 0;

    //if (!menu_allow_background_windows) return 0;

    zxvision_window *orig_w;

    orig_w=w;

    if (w==NULL) return 0;

    //if (zxvision_current_window==w) return 0;

    do {
        zxvision_window *superior_window;

        superior_window=w->next_window;

        if (superior_window!=NULL) {
            //printf("ventana %s encima de la que se redibuja %s\n",superior_window->window_title,w->window_title);
            if (zxvision_coords_in_window(superior_window,x,y)) return 1;

        }


        w=superior_window;

    } while (w!=zxvision_current_window && w!=NULL);

    //return 0;

    //O si hay alguna ventana por debajo que tenga el flag de siempre por encima
    w=orig_w;
    do {
        zxvision_window *inferior_window;

        inferior_window=w->previous_window;

        if (inferior_window!=NULL) {
            if (inferior_window->always_visible) {
                //printf("ventana %s con always visible encima de la que se redibuja %s\n",inferior_window->window_title,orig_w->window_title);
                if (zxvision_coords_in_window(inferior_window,x,y)) return 1;
            }

        }


        w=inferior_window;

    } while (w!=zxvision_current_window && w!=NULL);

    return 0;

}

//Buscar una ventana entre las activas que corresponda al geometry name
zxvision_window *zxvision_find_window_in_background(char *geometry_name)
{
    if (!menu_allow_background_windows) return NULL;

    zxvision_window *w;
    w=zxvision_current_window;

    //Empezamos de arriba hacia abajo

    while (w!=NULL) {

        if (!strcasecmp(w->geometry_name,geometry_name)) return w;

        zxvision_window *lower_window;

        lower_window=w->previous_window;

        w=lower_window;

    }

    return NULL;

}


//Dice si las coordenadas indicadas coinciden con cualquiera de las ventanas que estén en las ventanas de debajo de la indicada
//Retorna la ventana implicada, o NULL si no
zxvision_window *zxvision_coords_in_below_windows(zxvision_window *w,int x,int y)
{
    if (!menu_allow_background_windows) return NULL;

    if (w==NULL) return NULL;

    //Empezamos de arriba hacia abajo

    //La primera que encontramos y que contiene las coordenadas indicadas
    zxvision_window *final_lower_window=NULL;

    do {
        zxvision_window *lower_window;

        lower_window=w->previous_window;

        if (lower_window!=NULL) {

            //Si hay ventana por debajo y no hemos escogido una ya
            //O si esta ventana tiene switch de always visible
            //Nota: en caso que haya varias ventanas con always visible, tendra precedencia, en este caso, la de mas abajo
            if (final_lower_window==NULL || lower_window->always_visible) {

                if (zxvision_coords_in_window(lower_window,x,y)) {
                    //printf("Elegida como final ventana: %s\n",lower_window->window_title);
                    final_lower_window=lower_window;
                }

            }

        }


        w=lower_window;

    } while (w!=NULL);

    return final_lower_window;

    //return NULL;

}


//Dice si las coordenadas indicadas coinciden con cualquiera de las ventanas
//Retorna la ventana implicada, o NULL si no
zxvision_window *zxvision_coords_in_any_window(int x,int y)
{
    //Si no hay current, no hay ventanas
    if (zxvision_current_window==NULL) return NULL;

    //Si apunta a la current
    if (zxvision_coords_in_window(zxvision_current_window,x,y)) return zxvision_current_window;

    //Si apunta a cualquiera de las que hay por debajo de la current
    return zxvision_coords_in_below_windows(zxvision_current_window,x,y);
}

//Dice si las coordenadas de ventana indicada coinciden con la zona ocupada por la ventana current
//esto se usa cuando está activado background window, para que las ventanas por detrás no tapen a la ventana actual
//Realmente es un poco chapuza, aunque efectivo. Lo ideal seria que las ventanas en background no se redibujasen
//desde una funcion de overlay, sino de otra manera mas limpia y ordenada
//Esto no impide que los pixeles de los overlay puedan pasar por encima de cualquier ventana (excepto la current, pues llamamos a aqui tambien)
//Creo ademas que esta funcion ya no se usa
/*
int zxvision_coords_in_front_window(zxvision_window *w,int x,int y)
{

    if (!menu_allow_background_windows) return 0;

    if (zxvision_current_window==NULL) return 0;

    if (zxvision_current_window==w) return 0;

    return zxvision_coords_in_window(zxvision_current_window,x,y);


}
*/

int zxvision_coords_in_front_window(int x,int y)
{

    if (!menu_allow_background_windows) return 0;

    if (zxvision_current_window==NULL) return 0;

    return zxvision_coords_in_window(zxvision_current_window,x,y);


}


void zxvision_draw_window_contents(zxvision_window *w)
{

    if (menu_es_stdout()) {
        zxvision_draw_window_contents_stdout(w);
        zxvision_reset_flag_dirty_must_draw_contents(w);
        return;
    }

    if (!zxvision_show_minimized(w)) return;

    //menu_textspeech_send_text(texto);

    //Buffer para speech
    char buffer_linea[MAX_BUFFER_SPEECH+1];

    int width,height;

    width=zxvision_get_effective_width(w);

    //Alto del contenido es 2 menos, por el titulo de ventana y la linea por debajo de margen
    height=zxvision_get_effective_height(w);

    int x,y;

    //Si hay que volver de la funcion sin hacer nada
    int must_return=1;

    //si usamos cache de putchar de mismo caracter, por defecto la usamos
    int use_cache=1;

    //decimos que hay que borrar fondo, por tanto no usamos cache
    if (w->must_clear_cache_on_draw) {
        use_cache=0;
        must_return=0;
    }

    //decimos que hay que borrar fondo, por tanto no usamos cache. Y este flag se resetea solo
    if (w->must_clear_cache_on_draw_once) {
        use_cache=0;
        w->must_clear_cache_on_draw_once=0;
        must_return=0;
    }

    if (w->dirty_must_draw_contents) {
        must_return=0;
    }

    //if (w!=NULL) printf("Start draw window contents: %s\n",w->window_title);

    if (must_return) return;

    //printf("menu_speech_tecla_pulsada en 1: %d\n",menu_speech_tecla_pulsada);


    for (y=0;y<height;y++) {
        int indice_speech=0;
        for (x=0;x<width;x++) {

            //printf ("x %d y %d\n",x,y);

            int xdestination=w->x+x;
            int ydestination=(w->y)+1+y; //y +1 porque empezamos a escribir debajo del titulo

            //Ver si caracter final tiene ventana por encima
            int ventana_encima=zxvision_coords_in_superior_windows(w,xdestination,ydestination);

            //if (ventana_encima) printf("ventana encima x %d y %d\n",xdestination,ydestination);


            //obtener caracter
            int out_of_bonds=0;

            int offset_x_final=x+w->offset_x;
            if (offset_x_final>=w->total_width) out_of_bonds=1;

            int offset_y_final=y+w->offset_y;

            int lower_margin_starts_at=height-(w->lower_margin);

            //printf ("sonda 1\n");

                //Texto leyenda parte superior
                if (y<w->upper_margin) {
                    offset_y_final=y;
                }
                //Texto leyenda parte inferior
                else if (y>=lower_margin_starts_at) {
                    int effective_height=height-w->upper_margin-w->lower_margin;
                    int final_y=y-effective_height;
                    offset_y_final=final_y;
                }
                else {
                    offset_y_final +=w->lower_margin; //Dado que ya hemos pasado la parte superior, saltar la inferior
                }
            //printf ("sonda 2\n");

            if (offset_y_final>=w->total_height) out_of_bonds=1;

            if (!out_of_bonds) {

                //Origen de donde obtener el texto
                int offset_caracter;

                offset_caracter=((offset_y_final)*w->total_width)+offset_x_final;

                overlay_screen *caracter;
                caracter=w->memory;
                caracter=&caracter[offset_caracter];

                z80_byte caracter_escribir=caracter->caracter;

                int tinta=caracter->tinta;
                int papel=caracter->papel;

                //Si esta linea cursor visible
                int linea_cursor=w->cursor_line;
                //tener en cuenta desplazamiento de margenes superior e inferior
                linea_cursor +=w->lower_margin;
                linea_cursor +=w->upper_margin;
                if (w->visible_cursor && linea_cursor==offset_y_final && x>=w->acortar_cursor && x<width-w->acortar_cursor) {
                    //printf("cursor en linea %d\n",linea_cursor);
                    tinta=ESTILO_GUI_TINTA_SELECCIONADO;
                    papel=ESTILO_GUI_PAPEL_SELECCIONADO;
                }

                //Chapucilla para evitar que las ventanas en background sobreescriban a la current
                //if (!zxvision_coords_in_front_window(w,xdestination,ydestination)) {

                //Chapucilla para evitar que las ventanas en background sobreescriban a cualquiera que haya encima
                if (!ventana_encima) {
                //if (!zxvision_coords_in_superior_windows(w,xdestination,ydestination)) {

                //printf ("antes de putchar\n");
                putchar_menu_overlay_parpadeo_cache_or_not(xdestination,ydestination,
                    caracter_escribir,tinta,papel,caracter->parpadeo,use_cache);

                    //printf ("despues de putchar\n");

                if (indice_speech<MAX_BUFFER_SPEECH) {
                    //Evitar caracteres fuera de rango
                    //por ejemplo el 255 puede entrar como caracter transparente en ventana de keyboard help
                    if (caracter_escribir<127) buffer_linea[indice_speech++]=caracter_escribir;
                }

                }
            }

            //Fuera de rango. Metemos espacio
            else {
                //printf ("fuera de rango\n");
                if (!ventana_encima) {
                    int color_beyond=w->default_paper;

                    if (w->beyond_height>0 && w->beyond_width>0) {
                        if (x>=w->beyond_x && x<w->beyond_x+w->beyond_width &&
                            y>=w->beyond_y && y<=w->beyond_y+w->beyond_height) {

                            color_beyond=w->beyond_color;
                        }
                    }


                putchar_menu_overlay_parpadeo_cache_or_not(xdestination,ydestination,
                ' ',ESTILO_GUI_TINTA_NORMAL,color_beyond/*ESTILO_GUI_PAPEL_NORMAL*/,0,use_cache);
                }
            }
            //printf ("sonda 3\n");

        }

        //printf("menu_speech_tecla_pulsada en 2: %d\n",menu_speech_tecla_pulsada);
        buffer_linea[indice_speech]=0;
        menu_textspeech_send_text(buffer_linea);

        //printf ("sonda 4\n");
    }


    zxvision_reset_flag_dirty_must_draw_contents(w);

    w->has_been_drawn_contents=1;



    //if (w!=NULL) printf("YES draw window contents: %s\n",w->window_title);

}


//Funcion derivada de zxvision_draw_window_contents
//usada para obtener los hotkeys de mouse
void zxvision_get_character_at_mouse(zxvision_window *w,int x,int y,overlay_screen *caracter_retorno)
{


    //int width;
    int height;

    //width=zxvision_get_effective_width(w);

    //Alto del contenido es 2 menos, por el titulo de ventana y la linea por debajo de margen
    height=zxvision_get_effective_height(w);



    //for (y=0;y<height;y++) {

        //for (x=0;x<width;x++) {

            //printf ("x %d y %d\n",x,y);

            //int xdestination=w->x+x;
            //int ydestination=(w->y)+1+y; //y +1 porque empezamos a escribir debajo del titulo

            //Ver si caracter final tiene ventana por encima
            int ventana_encima=0;


            //obtener caracter
            int out_of_bonds=0;

            int offset_x_final=x+w->offset_x;
            if (offset_x_final>=w->total_width) out_of_bonds=1;

            int offset_y_final=y+w->offset_y;

            int lower_margin_starts_at=height-(w->lower_margin);

            //printf ("sonda 1\n");

                //Texto leyenda parte superior
                if (y<w->upper_margin) {
                    offset_y_final=y;
                }
                //Texto leyenda parte inferior
                else if (y>=lower_margin_starts_at) {
                    int effective_height=height-w->upper_margin-w->lower_margin;
                    int final_y=y-effective_height;
                    offset_y_final=final_y;
                }
                else {
                    offset_y_final +=w->lower_margin; //Dado que ya hemos pasado la parte superior, saltar la inferior
                }
            //printf ("sonda 2\n");

            if (offset_y_final>=w->total_height) out_of_bonds=1;

            if (!out_of_bonds) {

                //Origen de donde obtener el texto
                int offset_caracter;

                offset_caracter=((offset_y_final)*w->total_width)+offset_x_final;

                overlay_screen *caracter;
                caracter=w->memory;
                caracter=&caracter[offset_caracter];

                z80_byte caracter_escribir=caracter->caracter;

                //int tinta=caracter->tinta;
                //int papel=caracter->papel;

                //Si esta linea cursor visible
                int linea_cursor=w->cursor_line;
                //tener en cuenta desplazamiento de margenes superior e inferior
                linea_cursor +=w->lower_margin;
                linea_cursor +=w->upper_margin;
                if (w->visible_cursor && linea_cursor==offset_y_final) {
                    //tinta=ESTILO_GUI_TINTA_SELECCIONADO;
                    //papel=ESTILO_GUI_PAPEL_SELECCIONADO;
                }

                //Chapucilla para evitar que las ventanas en background sobreescriban a la current
                //if (!zxvision_coords_in_front_window(w,xdestination,ydestination)) {

                //Chapucilla para evitar que las ventanas en background sobreescriban a cualquiera que haya encima
                if (!ventana_encima) {
                //if (!zxvision_coords_in_superior_windows(w,xdestination,ydestination)) {

                //printf ("antes de putchar\n");

                caracter_retorno->caracter=caracter_escribir;

                caracter_retorno->tinta=caracter->tinta;
                caracter_retorno->papel=caracter->papel;
                caracter_retorno->parpadeo=caracter->parpadeo;
                return;
                //putchar_menu_overlay_parpadeo(xdestination,ydestination,
                //	caracter_escribir,tinta,papel,caracter->parpadeo);

                    //printf ("despues de putchar\n");


                }
            }

            //Fuera de rango. Retornamos 0
            else {
                //printf ("fuera de rango\n");
                if (!ventana_encima) {
                    caracter_retorno->caracter=0;
                }
            }


    caracter_retorno->caracter=0;




}


void zxvision_draw_window_contents_no_speech(zxvision_window *ventana)
{
                //No queremos que el speech vuelva a leer la ventana, solo cargar ventana
        int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
                menu_speech_set_tecla_pulsada();
                zxvision_draw_window_contents(ventana);

        /*if (menu_speech_tecla_pulsada && !antes_menu_speech_tecla_pulsada) {
            printf("Pasar de 1 a 0 desde \n");
            debug_exec_show_backtrace();
        }*/

        menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;

}

//Alterar atributos de caracteres en la memoria de la ventana
//Escribir caracter en la memoria de la ventana
void zxvision_set_attr(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo)
{
    //Comprobar limites
    if (x>=w->total_width || x<0 || y>=w->total_height || y<0) return;

    //Sacamos offset
    int offset=(y*w->total_width)+x;

    zxvision_set_flag_dirty_must_draw_contents(w);

    //Puntero
    overlay_screen *p;

    p=w->memory; //Puntero inicial

    p=&p[offset]; //Puntero con offset

    p->tinta=tinta;
    p->papel=papel;
    p->parpadeo=parpadeo;


}

//Escribir caracter en la memoria de la ventana
void zxvision_print_char(zxvision_window *w,int x,int y,overlay_screen *caracter)
{
    //Comprobar limites
    if (x>=w->total_width || x<0 || y>=w->total_height || y<0) {

        z80_byte letra_escribir=caracter->caracter;
        if (letra_escribir!=32 && (x>=w->total_width || y>=w->total_height)) {
            //Indicar que se intentó escribir mas alla del limite de la ventana, siempre que caracter no sea espacio
            //printf("pasa limites. caracter %c (%d)\n",letra_escribir,letra_escribir);
            w->tried_write_beyond_size=1;

            //Si es la ventana actual, avisar al momento
            if (zxvision_current_window==w) {
                zxvision_set_mark_tried_write_beyond_size(w);
            }
        }

        return;
    }

    zxvision_set_flag_dirty_must_draw_contents(w);

    //Sacamos offset
    int offset=(y*w->total_width)+x;



    //Puntero
    overlay_screen *p;

    p=w->memory; //Puntero inicial

    p=&p[offset]; //Puntero con offset

    p->tinta=caracter->tinta;
    p->papel=caracter->papel;
    p->parpadeo=caracter->parpadeo;
    p->caracter=caracter->caracter;


}

void zxvision_print_char_simple(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,z80_byte caracter)
{
    overlay_screen caracter_aux;
    caracter_aux.caracter=caracter;
    caracter_aux.tinta=tinta;
    caracter_aux.papel=papel;
    caracter_aux.parpadeo=parpadeo;

    zxvision_print_char(w,x,y,&caracter_aux);
}

void zxvision_print_string(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,char *texto)
{

    //Si la ventana tiene desactivado el parseo de caracteres especiales
    int antes_menu_disable_special_chars=menu_disable_special_chars.v;
    if (w->disable_special_chars) menu_disable_special_chars.v=1;

    //Si la ventana muestra las letras de atajos de la ventana en inverso
    int antes_menu_writing_inverse_color=menu_writing_inverse_color.v;
    if (w->writing_inverse_color) menu_writing_inverse_color.v=1;

    int inverso_letra=0;
    int minuscula_letra=1;
    int era_utf=0;

    while (*texto) {

        overlay_screen caracter_aux;
        caracter_aux.caracter=*texto;

        int invertir_siempre_independientemente_del_estilo=0;

        //TODO: gestion caracteres de control
//Si dos ^ seguidas, invertir estado parpadeo
        if (menu_escribe_texto_si_parpadeo(texto,0)) {
            parpadeo ^=1;
            //y saltamos esos codigos de negado
                        texto +=2;
                        caracter_aux.caracter=*texto;
        }

        //Codigo control color tinta
        if (menu_escribe_texto_si_cambio_tinta(texto,0)) {
            tinta=texto[2]-'0';
            texto+=3;
            caracter_aux.caracter=*texto;
        }

        //ver si dos ~~ o ~^ o ~! seguidas y cuidado al comparar que no nos vayamos mas alla del codigo 0 final
        if (menu_escribe_texto_si_inverso(texto,0)) {
            minuscula_letra=1;
            //y saltamos esos codigos de negado. Ver si era ~^, con lo que indica que no hay que bajar a minusculas
            texto++;
            if (*texto=='^') minuscula_letra=0;

            if (*texto=='!') {
                invertir_siempre_independientemente_del_estilo=1;
                minuscula_letra=0;
            }

            texto++;
            caracter_aux.caracter=*texto;

            if (menu_writing_inverse_color.v) inverso_letra=1;
            else inverso_letra=0;

        }

        //else {

            //Si estaba prefijo utf activo

            if (era_utf) {
                caracter_aux.caracter=menu_escribe_texto_convert_utf(era_utf,*texto);
                era_utf=0;

                //Caracter final utf
                //putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
            }


            //Si no, ver si entra un prefijo utf
            else {
                //printf ("letra: %02XH\n",letra);
                //Prefijo utf
                if (menu_es_prefijo_utf(*texto)) {
                    //DUDA: por que asigno a *texto y no a 1? quiza es por detectar si final de texto (codigo 0) ?
                    era_utf=*texto;
                    //printf ("activado utf\n");

                    //Si era con inverso y es utf, hay que decir que se mantiene el inverso activado durante 2 bytes, que es lo que suelen ocupar los
                    //utf con acentos
                    if (inverso_letra) inverso_letra++;
                }

                /*else {
                    //Caracter normal
                    putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
                }*/
            }


        //}


        if (!inverso_letra) {
            caracter_aux.tinta=tinta;
            caracter_aux.papel=papel;
        }
        else {
            if (ESTILO_GUI_INVERSE_TINTA!=-1 && !invertir_siempre_independientemente_del_estilo) {
                caracter_aux.tinta=ESTILO_GUI_INVERSE_TINTA;
                caracter_aux.papel=papel;
            }
            else {
                caracter_aux.tinta=papel;
                caracter_aux.papel=tinta;
            }


            //Los hotkeys de menu siempre apareceran en minusculas para ser coherentes
            //De la misma manera, no se soportan hotkeys en menus que sean minusculas
            if (minuscula_letra) caracter_aux.caracter=letra_minuscula(caracter_aux.caracter);
        }

        //Poder soportar color inverso para mas de 1 byte, especial para caracteres utf
        if (inverso_letra) inverso_letra--;


        caracter_aux.parpadeo=parpadeo;


        zxvision_print_char(w,x,y,&caracter_aux);
        if (!era_utf) x++;
        texto++;
    }

    menu_disable_special_chars.v=antes_menu_disable_special_chars;
    menu_writing_inverse_color.v=antes_menu_writing_inverse_color;
}


void zxvision_print_string_format (zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo, const char * format , ...)
{

    char buffer_final[4096];

    va_list args;
    va_start (args, format);
    vsprintf (buffer_final,format, args);
    va_end (args);

    zxvision_print_string(w,x,y,tinta,papel,parpadeo,buffer_final);

}

void zxvision_print_string_defaults(zxvision_window *w,int x,int y,char *texto)
{

    zxvision_print_string(w,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,texto);

}


void zxvision_print_string_defaults_format (zxvision_window *w,int x,int y, const char * format , ...)
{

    char buffer_final[4096];

    va_list args;
    va_start (args, format);
    vsprintf (buffer_final,format, args);
    va_end (args);

    zxvision_print_string_defaults(w,x,y,buffer_final);

}

//Imprimir 1 caracter
void zxvision_print_char_defaults(zxvision_window *w,int x,int y,char c)
{

    char buffer[2];

    buffer[0]=c;
    buffer[1]=0;

    zxvision_print_string_defaults(w,x,y,buffer);
}

void zxvision_fill_width_spaces_paper_width(zxvision_window *w,int y,int papel,int start_x,int after_end_x)
{
    overlay_screen caracter_aux;
    caracter_aux.caracter=' ';
    caracter_aux.tinta=ESTILO_GUI_TINTA_NORMAL;
    caracter_aux.papel=papel;
    caracter_aux.parpadeo=0;

    int i;
    for (i=start_x;i<after_end_x;i++) {
        zxvision_print_char(w,i,y,&caracter_aux);
    }
}

void zxvision_fill_width_spaces_paper(zxvision_window *w,int y,int papel)
{
    zxvision_fill_width_spaces_paper_width(w,y,papel,0,w->total_width);
}

void zxvision_fill_width_spaces(zxvision_window *w,int y)
{
    zxvision_fill_width_spaces_paper(w,y,ESTILO_GUI_PAPEL_NORMAL);
}

//Igual que la anterior pero antes borra la linea con espacios
void zxvision_print_string_defaults_fillspc(zxvision_window *w,int x,int y,char *texto)
{


    zxvision_fill_width_spaces(w,y);

    zxvision_print_string(w,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,texto);

}

void zxvision_print_string_fillspc(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,char *texto)
{
    zxvision_fill_width_spaces(w,y);

    zxvision_print_string(w,x,y,tinta,papel,parpadeo,texto);
}

void zxvision_print_string_defaults_fillspc_format(zxvision_window *w,int x,int y,const char * format , ...)
{

    char buffer_final[4096];

    va_list args;
    va_start (args, format);
    vsprintf (buffer_final,format, args);
    va_end (args);

    zxvision_print_string_defaults_fillspc(w,x,y,buffer_final);

}


void zxvision_putpixel(zxvision_window *w,int x,int y,int color)
{

    //int final_x,final_y;

    /*

-Como puede ser que al redimensionar ay sheet la ventana tenga más tamaño total... se crea de nuevo al redimensionar?
->no, porque dibuja pixeles con overlay y eso no comprueba si sale del limite virtual de la ventana
Creo que el putpixel en overlay no controla ancho total sino ancho visible. Exacto

Efectivamente. Se usa tamaño visible
Hacerlo constar en zxvision_putpixel como un TODO. Aunque si no hubiera este “fallo”, al redimensionar ay sheet no se vería el tamaño adicional , habría que cerrar la ventana y volverla a abrir ya con el tamaño total nuevo (ya que guarda geometría)
Es lo que pasa con otras ventanas de texto, que no se amplía el ancho total al no recrearse la ventana , y hay que salir y volver a entrar. Ejemplos??


*/

    //Obtener coordenadas en pixeles de zona ventana dibujable
    int window_pixel_start_x=(w->x)*menu_char_width;
    int window_pixel_start_y=((w->y)+1)*menu_char_height;
    int window_pixel_final_x=window_pixel_start_x+((w->visible_width)-zxvision_get_minus_width_byscrollvbar(w))*menu_char_width;
    int window_pixel_final_y=window_pixel_start_y+((w->visible_height)-2)*menu_char_height;

    //Obtener coordenada x,y final donde va a parar
    int xfinal=x+window_pixel_start_x-(w->offset_x)*menu_char_width;
    int yfinal=y+window_pixel_start_y-(w->offset_y)*menu_char_height;

    //Ver si esta dentro de rango
    if (xfinal>=window_pixel_start_x && xfinal<window_pixel_final_x && yfinal>=window_pixel_start_y && yfinal<window_pixel_final_y) {

    //Chapucilla para evitar que las ventanas en background sobreescriban a las de arriba
    //if (!zxvision_coords_in_front_window(w,xfinal/menu_char_width,yfinal/8)) {
    if (!zxvision_coords_in_superior_windows(w,xfinal/menu_char_width,yfinal/menu_char_height)) {
        menu_scr_putpixel(xfinal,yfinal,color);
    }

    }
    else {
        //printf ("pixel out of window %d %d\n",x,y);
    }
}


//Hacer putpixel sin tener en cuenta zoom_x ni y. Usado en help keyboard
void zxvision_putpixel_no_zoom(zxvision_window *w,int x,int y,int color)
{

    //int final_x,final_y;

    /*

-Como puede ser que al redimensionar ay sheet la ventana tenga más tamaño total... se crea de nuevo al redimensionar?
->no, porque dibuja pixeles con overlay y eso no comprueba si sale del limite virtual de la ventana
Creo que el putpixel en overlay no controla ancho total sino ancho visible. Exacto

Efectivamente. Se usa tamaño visible
Hacerlo constar en zxvision_putpixel como un TODO. Aunque si no hubiera este “fallo”, al redimensionar ay sheet no se vería el tamaño adicional , habría que cerrar la ventana y volverla a abrir ya con el tamaño total nuevo (ya que guarda geometría)
Es lo que pasa con otras ventanas de texto, que no se amplía el ancho total al no recrearse la ventana , y hay que salir y volver a entrar. Ejemplos??


*/

    //Obtener coordenadas en pixeles de zona ventana dibujable
    //En este caso multiplicar por zoom_x zoom_y pues coordenadas finales no tienen en cuenta zoom
    int window_pixel_start_x=(w->x)*menu_char_width*zoom_x;
    int window_pixel_start_y=((w->y)+1)*menu_char_height*zoom_y;

    int window_pixel_final_x=window_pixel_start_x+((w->visible_width)-zxvision_get_minus_width_byscrollvbar(w))*menu_char_width*zoom_x;
    int window_pixel_final_y=window_pixel_start_y+((w->visible_height)-2)*menu_char_height*zoom_y;

    //Obtener coordenada x,y final donde va a parar
    int xfinal=x+window_pixel_start_x-(w->offset_x)*menu_char_width*zoom_x;
    int yfinal=y+window_pixel_start_y-(w->offset_y)*menu_char_height*zoom_y;


    //int total_width_window=((w->visible_width)-zxvision_get_minus_width_byscrollvbar(w))*menu_char_width*zoom_x;
    //int total_height_window=((w->visible_height)-2)*8*zoom_y;



    //Ver si esta dentro de rango. Metodo nuevo pero que no va bien
    //if (x>=0 && x<total_width_window && y>=0 && y<=total_height_window) {

    //Antiguo metodo que tiene en cuenta los offsets
    if (xfinal>=window_pixel_start_x && xfinal<window_pixel_final_x && yfinal>=window_pixel_start_y && yfinal<window_pixel_final_y) {

        //Chapucilla para evitar que las ventanas en background sobreescriban a las de arriba
        //if (!zxvision_coords_in_front_window(w,xfinal/menu_char_width,yfinal/8)) {
        if (!zxvision_coords_in_superior_windows(w,(xfinal/menu_char_width)/zoom_x,(yfinal/menu_char_height)/zoom_y)  ) {
            menu_scr_putpixel_no_zoom(xfinal,yfinal,color);
        }

    }
    else {
        //printf ("pixel out of window %d %d width %d height: %d\n",x,y,total_width_sin_zoom,total_height_sin_zoom);
    }
}



//Funcion para trazar una linea usando algoritmo de bresenham
void zxvision_draw_line(zxvision_window *w,int x1,int y1,int x2,int y2,int c, void (*fun_putpixel) (zxvision_window *w,int x,int y,int color) )
{
 int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;
 dx=x2-x1;
 dy=y2-y1;
 dx1=util_abs(dx);
 dy1=util_abs(dy);
 px=2*dy1-dx1;
 py=2*dx1-dy1;
 if(dy1<=dx1)
 {
  if(dx>=0)
  {
   x=x1;
   y=y1;
   xe=x2;
  }
  else
  {
   x=x2;
   y=y2;
   xe=x1;
  }
  fun_putpixel(w,x,y,c);
  for(i=0;x<xe;i++)
  {
   x=x+1;
   if(px<0)
   {
    px=px+2*dy1;
   }
   else
   {
    if((dx<0 && dy<0) || (dx>0 && dy>0))
    {
     y=y+1;
    }
    else
    {
     y=y-1;
    }
    px=px+2*(dy1-dx1);
   }
   fun_putpixel(w,x,y,c);
  }
 }
 else
 {
  if(dy>=0)
  {
   x=x1;
   y=y1;
   ye=y2;
  }
  else
  {
   x=x2;
   y=y2;
   ye=y1;
  }
  fun_putpixel(w,x,y,c);
  for(i=0;y<ye;i++)
  {
   y=y+1;
   if(py<=0)
   {
    py=py+2*dx1;
   }
   else
   {
    if((dx<0 && dy<0) || (dx>0 && dy>0))
    {
     x=x+1;
    }
    else
    {
     x=x-1;
    }
    py=py+2*(dx1-dy1);
   }
   fun_putpixel(w,x,y,c);
  }
 }
}

//Funcion para trazar un arco
void zxvision_draw_arc(zxvision_window *w,int x1,int y1,int radius_x,int radius_y,int c, void (*fun_putpixel) (zxvision_window *w,int x,int y,int color) ,int inicio_grados,int limite_grados)
{

    int grados;

    //A partir de circulos de radio 57 (360/2/pi) se quedaria punteado, y hay que hacer lineas para unirlos
    //Debido logicamente a que la funcion que uso de coseno y seno solo tiene tablas de 360 grados, sin decimales,
    //si usase tablas con decimales y el bucle for de abajo incrementase grados en decimal, y no en entero de 1, no pasaria
    const int umbral_punteado=57;
    int radio_mayor;

    if (radius_x>radius_y) radio_mayor=radius_x;
    else radio_mayor=radius_y;

    for (grados=inicio_grados;grados<limite_grados;grados++) {
        int xdestino=x1+((radius_x*util_get_cosine(grados))/10000);
        int ydestino=y1-((radius_y*util_get_sine(grados))/10000);
        fun_putpixel(w,xdestino,ydestino,c);

        if (radio_mayor>=umbral_punteado) {
            //generar linea que una con el punto siguiente
            int xdestino2=x1+((radius_x*util_get_cosine(grados+1))/10000);
            int ydestino2=y1-((radius_y*util_get_sine(grados+1))/10000);

            zxvision_draw_line(w,xdestino,ydestino,xdestino2,ydestino2,c,fun_putpixel);
        }
    }

}

//Funcion para trazar una elipse
void zxvision_draw_ellipse(zxvision_window *w,int x1,int y1,int radius_x,int radius_y,int c, void (*fun_putpixel) (zxvision_window *w,int x,int y,int color) ,int limite_grados)
{

    zxvision_draw_arc(w,x1,y1,radius_x,radius_y,c,fun_putpixel,0,limite_grados);


}

//Cada caracter se define en una cuadricula de 8x8
//El significado de cada string es:
//Empezar en primer caracter
//bucle:
//si caracter es "P", indica moverse a posicion x,y de los dos siguientes caracteres
//si caracter es un digito, indica hacer linea desde posicion anterior a posicion indicada en siguientes caracteres, y moverse a dicha posicion
//cualquier otro caracter no es admitido
//Nota: están obtenidos desde The Liner 75, donde el 0,0 está abajo
//TODO: FALTAN todos los que van a continuacion de la Z. La rutina convierte minusculas a mayusculas, esos quedan solventados,
//pero no hay [ \ ] ^ ... { | } ... etc
char *zxvision_vectorial_characters[]={

 "","P3236P3030","P2526P4546","P0565P0161P1016P5056","P551513535111P3036","66P0516P1506P5061P6051","P5014253401103052","P3546","P40313546","P20313526", //: REM )

 "66P0660P0363P3036","P0363P3036","P304142","P0363","P1010","66","66P0006666000","P143630","P066663030060","P06666000P0363", // REM 3

 "P060363P6066","P660603636000","P660600606303","P30336606","06666000P0363","6066060363","P3131P3535","P203132P3434", // REM ;

 "P502356","P0161P0565","P104316","P141526365432P3030","P40100104154554534222233332","066660P0363","P5000065665645303P53626150", // REM B

 "P60000666","P300006366330","P60000666P0343","0666P0343","P660600606333","06P6066P0363", // REM H

 "60P0666P3036","P03006066","06P0336P0330","P060060","06336660","066066","06666000","06666303", // REM P

 "06666000P6033","06666303P5023","P660603636000","P3036P0666","P06006066","P0603306366", // REM V

 "P0600336066","66P0660","P063366P3033","P06660060" // REM Z
};


void zxvision_print_vectorial_text(zxvision_window *w,int x,int y,int text_zoom,int color,char *texto,
    void (*fun_putpixel) (zxvision_window *w,int parm_x,int parm_y,int parm_color) )
{

    //calcular el punto mas bajo
    int origen_x=x;
    int origen_y=y+7*text_zoom;

    int xactual=0;
    int yactual=0;

    while (*texto) {
        char caracter=*texto;
        texto++;

        if (caracter>='a' && caracter<='z') caracter -=32;

        if (caracter<32 || caracter>'Z') caracter='?';

        int indice_caracter_vectorial=caracter-32;

        char *caracter_vectorial=zxvision_vectorial_characters[indice_caracter_vectorial];
        //printf("caracter vectorial: [%s]\n",caracter_vectorial);

        int i;
        int x_en_caracter=0;
        int y_en_caracter=0;
        for (i=0;caracter_vectorial[i];) {
            if (caracter_vectorial[i]=='P') {
                i++;
                x_en_caracter=caracter_vectorial[i++]-'0';
                y_en_caracter=caracter_vectorial[i++]-'0';
            }
            else {
                //Sera digito
                int nueva_x=caracter_vectorial[i++]-'0';
                int nueva_y=caracter_vectorial[i++]-'0';

                //trazar linea de antes a ahora
                int xorig_escalada=xactual+x_en_caracter*text_zoom;
                xorig_escalada +=origen_x;
                int yorig_escalada=yactual+y_en_caracter*text_zoom;
                //y hacia arriba
                yorig_escalada=origen_y-yorig_escalada;

                int xdest_escalada=xactual+nueva_x*text_zoom;
                xdest_escalada +=origen_x;
                int ydest_escalada=yactual+nueva_y*text_zoom;
                //y hacia arriba
                ydest_escalada=origen_y-ydest_escalada;

                zxvision_draw_line(w,xorig_escalada,yorig_escalada,xdest_escalada,ydest_escalada,color,fun_putpixel);

                x_en_caracter=nueva_x;
                y_en_caracter=nueva_y;
            }


        }

        xactual +=8*text_zoom;

    }
}


void zxvision_widgets_draw_speedometer(zxvision_window *ventana,int xcentro_widget,int ycentro_widget,int longitud_linea,int grados,int color_linea,int color_contorno)
{
        //calcular punto final linea. Algo menos para que no toque con el contorno
        int xfinal_linea=xcentro_widget+(longitud_linea-5)*util_get_cosine(grados)/10000;

        int yfinal_linea=ycentro_widget-(longitud_linea-5)*util_get_sine(grados)/10000;

        zxvision_draw_line(ventana,xcentro_widget,ycentro_widget,xfinal_linea,yfinal_linea,color_linea,zxvision_putpixel);

        //Y el contorno
        int centro_x=xcentro_widget;
        int centro_y=ycentro_widget;
        int radio=longitud_linea;

        zxvision_draw_ellipse(ventana,centro_x,centro_y,radio,radio,color_contorno,zxvision_putpixel,180);
        //doble contorno. Con radio algo menor
        zxvision_draw_ellipse(ventana,centro_x,centro_y,radio-1,radio-1,color_contorno,zxvision_putpixel,180);

}

void zxvision_widgets_draw_speedometer_common(zxvision_window *ventana,int xcentro_widget,int ycentro_widget,int percentaje,int color_linea,int color_contorno)
{

    //180 grados = 0%
    //0 grados=100%
    int grados=180-(percentaje*180)/100;
    //printf("%s: %d grados : %d\n",texto,percentaje,grados);

    int longitud_linea=ZXVISION_WIDGET_TYPE_SPEEDOMETER_LINE_LENGTH;

    zxvision_widgets_draw_speedometer(ventana,xcentro_widget,ycentro_widget,longitud_linea,grados,color_linea,color_contorno);


}



void zxvision_draw_rectangle_function(zxvision_window *ventana,int x1,int y1,int ancho,int alto,int color,
    void (*fun_putpixel) (zxvision_window *w,int x,int y,int color)  )
{

    int x2=x1+ancho-1;
    int y2=y1+alto-1;

    //arriba
    zxvision_draw_line(ventana,x1,y1,x2,y1,color,fun_putpixel);
    //abajo
    zxvision_draw_line(ventana,x1,y2,x2,y2,color,fun_putpixel);
    //izquierda
    zxvision_draw_line(ventana,x1,y1,x1,y2,color,fun_putpixel);
    //derecha
    zxvision_draw_line(ventana,x2,y1,x2,y2,color,fun_putpixel);

}

void zxvision_draw_rectangle(zxvision_window *ventana,int x1,int y1,int ancho,int alto,int color)
{
    zxvision_draw_rectangle_function(ventana,x1,y1,ancho,alto,color,zxvision_putpixel);
}


void zxvision_draw_filled_rectangle(zxvision_window *ventana,int xinicio,int yinicio,int ancho,int alto,int color)
{

    int x,y;
    for (x=xinicio;x<xinicio+ancho;x++) {
        for (y=yinicio;y<yinicio+alto;y++) {
            zxvision_putpixel(ventana,x,y,color);
        }
    }

}

void zxvision_widgets_erase_speedometer(zxvision_window *ventana,int xcentro_widget,int ycentro_widget)
{

    zxvision_draw_filled_rectangle(ventana,
        xcentro_widget-ZXVISION_WIDGET_TYPE_SPEEDOMETER_LINE_LENGTH,ycentro_widget-ZXVISION_WIDGET_TYPE_SPEEDOMETER_LINE_LENGTH,
        ZXVISION_WIDGET_TYPE_SPEEDOMETER_LINE_LENGTH*2+1,ZXVISION_WIDGET_TYPE_SPEEDOMETER_LINE_LENGTH+1,ESTILO_GUI_PAPEL_NORMAL);
}

void zxvision_widgets_draw_circle_ellipse(zxvision_window *ventana,int x,int y,int percentaje,int color,int radio_total_x,int radio_total_y,int concentrico)
{

    int radio_porcentaje_x=(radio_total_x*percentaje)/100;
    int radio_porcentaje_y=(radio_total_y*percentaje)/100;


    zxvision_draw_ellipse(ventana,x,y,radio_porcentaje_x,radio_porcentaje_y,color,zxvision_putpixel,360);

    //Y rellenar
    if (concentrico) {
        int i;
        for (i=2;i<5;i++) {
            zxvision_draw_ellipse(ventana,x,y,radio_porcentaje_x/i,radio_porcentaje_y/i,color,zxvision_putpixel,360);
        }
    }

}

void zxvision_widgets_draw_curve_common(zxvision_window *ventana,int xinicio_widget,int ycentro_widget,int percentaje,int color,int longitud_linea)
{


    int radio_total=longitud_linea;

    int radio_porcentaje=(radio_total*percentaje)/100;


    int centro_widget=xinicio_widget+radio_total;

    //Hacer 180 grados de curva
    zxvision_draw_ellipse(ventana,centro_widget,ycentro_widget,radio_porcentaje,radio_porcentaje,color,zxvision_putpixel,180);

    //Y lineas izquierda y derecha
    int trozo_linea=longitud_linea-radio_porcentaje;
    int xfinal_linea=xinicio_widget+trozo_linea;
    zxvision_draw_line(ventana,xinicio_widget,ycentro_widget,xfinal_linea,ycentro_widget,color,zxvision_putpixel);

    xfinal_linea=xinicio_widget+longitud_linea*2;
    int xinicio_linea=xfinal_linea-trozo_linea; //centro_widget+radio_porcentaje;
    zxvision_draw_line(ventana,xinicio_linea,ycentro_widget,xfinal_linea,ycentro_widget,color,zxvision_putpixel);



}

void zxvision_widgets_draw_volumen(char *texto,int valor,int longitud_texto)
{


    int i;
    int destino;


    for (i=0,destino=0;i<valor;i++) {
        texto[destino++]='=';

    }

    for (;i<longitud_texto;i++) {
        texto[destino++]=' ';
    }

    texto[destino]=0;

}



void zxvision_widgets_draw_volumen_maxmin(zxvision_window *ventana,int columna_texto,int fila_texto,
    int tinta,int papel,int valor_actual,int max_longitud_texto)
{

    //char buffer_texto_meters[100];
    //sprintf(buffer_texto_meters,"%s %3d%%",texto,valor_actual);
    //zxvision_print_string_defaults(ventana,columna_texto,fila_texto,buffer_texto_meters);

    int barra_volumen;

    //Gestionar valores negativos y limites
    if (valor_actual<0 || valor_actual>100) {
        barra_volumen=max_longitud_texto;
    }

    else {
        barra_volumen=(valor_actual*max_longitud_texto)/100;
    }

    char buffer_texto[32];

    zxvision_widgets_draw_volumen(buffer_texto,barra_volumen,max_longitud_texto);


    zxvision_print_string(ventana,columna_texto,fila_texto+1,tinta,papel,0,buffer_texto);
}


//Conversion de coordenadas 3D a 2D. Usando funcion coseno y seno
void zxvision_widgets_draw_particles_3d_convert(int x,int y,int z,int *xfinal,int *yfinal)
{

    int grados=45;

    /*

        Pasar de:
                +z

                ^     ^ +y
                |    /
                |   /
                |  /
                | /
                |/
                ----------------->  +x
            0,0


        A:
                +y

                ^
                |
                |
                |
                |
                |
                ----------------->  +x
            0,0


    */
    //*xfinal=x-y/2;
    //*yfinal=z-y/2;

    *xfinal=x+ ((y *util_get_cosine(grados))/10000);
    *yfinal=z+ ((y *util_get_sine  (grados))/10000);


}


//Conversion de coordenadas 3D a 2D. Simple sin calculo coseno, da mejor perspectiva que el anterior que usa coseno y seno
void zxvision_widgets_draw_particles_3d_convert_simple(int x,int y,int z,int *xfinal,int *yfinal)
{



    /*

        Pasar de:
                +z

                ^
                |
                |
                |
                |
            0,0 |
                \
               / \
              /   \
             /     \
            /       \
           y         x


        A:
                +y

                ^
                |
                |
                |
                |
                |
                ----------------->  +x
            0,0


    */
    //*xfinal=x-y/2;
    //*yfinal=z-y/2;

    *xfinal=x+ y;
    *yfinal=z+ y/2-x/2;


    //*yfinal=z+ ((y *util_get_sine  (45))/10000) - ((x *util_get_cosine(45))/10000);


}

void zxvision_widgets_draw_particles(zxvision_window *ventana,int xinicio_widget,int ycentro_widget,int percentaje,int color,int longitud_linea)
{


    int radio_total=longitud_linea;

    //int radio_porcentaje=(radio_total*percentaje)/100;


    int x_centro_widget=xinicio_widget+radio_total;

    //con el uso de perspectiva simple, hay que desplazar algo mas a la derecha
    x_centro_widget +=radio_total/3;

    //radio total * 100 para poder usar "decimales"

    radio_total *=100;

    int grados;

    int z=0;  //Z estara multiplicada por 100

    int vueltas;

    int max_vueltas;

    max_vueltas=radio_total/360; //para que al acabar la figura, este con radio 0

    for (vueltas=0;vueltas<max_vueltas;vueltas++) {

        for (grados=0;grados<360;grados++) {
            int xdestino=((radio_total/100)*util_get_cosine(grados))/10000;
            int ydestino=((radio_total/100)*util_get_sine(grados))/10000;

            int xplano,yplano;
            //zxvision_widgets_draw_particles_3d_convert(xdestino,ydestino,z/100,&xplano,&yplano);

            zxvision_widgets_draw_particles_3d_convert_simple(xdestino,ydestino,z/100,&xplano,&yplano);
            //if (yplano<0) printf("%d %d\n",xplano,yplano);
            zxvision_putpixel(ventana,x_centro_widget+xplano,ycentro_widget-yplano,color); //es -yplano porque si y es positiva, restamos (hacia arriba, pues el 0 de la y esta arriba del todo)

            //cada grado, reducir radio
            if (radio_total>=0) radio_total--;


            //cada 1/4 vuelta, aumentar Z segun porcentaje
            if ((grados % 90)==0) {
                z +=percentaje;
            }
        }


    }





}


void zxvision_widgets_draw_sphere(zxvision_window *ventana,int xinicio_widget,int ycentro_widget,int percentaje,int color,int longitud_linea)
{


    int radio_total=longitud_linea;

    //int radio_porcentaje=(radio_total*percentaje)/100;

    int radio_vertical=(radio_total*percentaje)/100;


    int x_centro_widget=xinicio_widget+radio_total;

    //con el uso de perspectiva simple, hay que desplazar algo mas a la derecha
    x_centro_widget +=radio_total/3;

    //radio total * 100 para poder usar "decimales"

    radio_total *=100;

    int grados;

    int zorigen=radio_vertical;



    // Dibujar una linea de referencia de suelo
    // De largo igual que el doble de radio
    // Nota: visualmente en 2D el suelo es menos largo que el ancho del radio,
    // pero es debido al paso de coordenadas 3D a 2D en que la coordenada Y de 3D hace desplazar la X en plano 2D y
    // por tanto el espacio ocupado en pantalla es algo mayor del suelo
    int largo_suelo=longitud_linea*2;
    int i;
    for (i=-largo_suelo/2;i<largo_suelo/2;i++) {
        //printf("i: %d\n",i);
        zxvision_putpixel(ventana,x_centro_widget+i,ycentro_widget,ESTILO_GUI_TINTA_NORMAL);
    }


    //Hacer varios circulos a diferentes alturas para simular esfera
    int grados_vertical;

    for (grados_vertical=0;grados_vertical<360;grados_vertical +=20) {

        //coordenada z variable dado que la esfera se aplasta
        int z=zorigen+(radio_vertical*util_get_cosine(grados_vertical))/10000;

        //hacemos tambien que la esfera rebote. Si tiene porcentaje 100 el sensor, estara mas arriba. Si tiene 0, estara abajo
        int sumar_z=(8*percentaje)/100;

        z +=sumar_z;

        int radio_aplicar=((radio_total/100)*util_get_sine(grados_vertical))/10000;

        //printf("grados_vertical: %d radio_aplicar: %d\n",grados_vertical,radio_aplicar);
        //printf("radio_aplicar: %d\n",radio_aplicar);

        for (grados=0;grados<360;grados++) {
            int xdestino=(radio_aplicar*util_get_cosine(grados))/10000;
            int ydestino=(radio_aplicar*util_get_sine(grados))/10000;

            //int xdestino=((radio_total/100)*util_get_cosine(grados))/10000;
            //int ydestino=((radio_total/100)*util_get_sine(grados))/10000;

            int xplano,yplano;

            zxvision_widgets_draw_particles_3d_convert_simple(xdestino,ydestino,z,&xplano,&yplano);

            zxvision_putpixel(ventana,x_centro_widget+xplano,ycentro_widget-yplano,color); //es -yplano porque si y es positiva, restamos (hacia arriba, pues el 0 de la y esta arriba del todo)

        }

    }






}


int zxvision_widgets_draw_sierpinsky_last_x=0;
int zxvision_widgets_draw_sierpinsky_last_y=0;


void zxvision_widgets_draw_sierpinsky(zxvision_window *ventana,int xinicio_widget,int yinicio_widget,int percentaje,int color)
{

    int i;

    int ancho=64;
    int alto=48;

    //Cantidad de iteracciones depende del porcentaje
    for (i=0;i<percentaje*2;i++) {

        //Sacar random
        ay_randomize(0);

        //randomize_noise es valor de 16 bits. sacar uno de 8 bits
        int punto=value_16_to_8h(randomize_noise[0]) % 3;
        int x2,y2;


        switch(punto) {
            case 0:
                //abajo izquierda
                x2=0;
                y2=alto-1;
            break;

            case 1:
                //abajo derecha
                x2=ancho-1;
                y2=alto-1;
            break;

            default:
                //arriba en medio
                x2=(ancho-1)/2;
                y2=0;
            break;

        }

        int x=(zxvision_widgets_draw_sierpinsky_last_x+x2)/2;
        int y=(zxvision_widgets_draw_sierpinsky_last_y+y2)/2;

        zxvision_widgets_draw_sierpinsky_last_x=x;
        zxvision_widgets_draw_sierpinsky_last_y=y;

        zxvision_putpixel(ventana,xinicio_widget+x,yinicio_widget+y,color); //es -y porque si y es positiva, restamos (hacia arriba, pues el 0 de la y esta arriba del todo)

    }

}

char *zxvision_widget_types_names[ZXVISION_TOTAL_WIDGET_TYPES]={
    "Speedometer",
    "Speaker",
    "Circle",
    "Circle Concentric",
    "Ellipse",
    "Ellipse Concentric",
    "Curve",
    "3DParticles",
    "Sphere",
    "Volume",
    "Only Value",
    "Sierpinsky"
};

void widget_list_print(void)
{

    int i;

    for (i=0;i<ZXVISION_TOTAL_WIDGET_TYPES;i++) {
        printf("%s%c ",zxvision_widget_types_names[i],(i==ZXVISION_TOTAL_WIDGET_TYPES-1 ? ' ' :','));
    }


}

int zxvision_widget_find_name_type(char *name)
{
    int i;

    for (i=0;i<ZXVISION_TOTAL_WIDGET_TYPES;i++) {
        if (!strcasecmp(name,zxvision_widget_types_names[i])) {
            return i;
        }
    }

    return -1;
}


void zxvision_widgets_draw_metter_common_by_shortname(zxvision_window *ventana,int columna_texto,int fila_texto,char *short_name,int tipo,int valor_en_vez_de_perc,int tinta_texto_descripcion,int papel_texto_descripcion,int escribir_espacios)
{


    int sensor_id=sensor_find(short_name);

    if (sensor_id<0) return;

    char *display_name;
    display_name=sensors_array[sensor_id].display_short_name;

    int media_cpu_perc=sensor_get_percentaje_value_by_id(sensor_id);


    int tinta_texto=ESTILO_GUI_TINTA_NORMAL;
    int color_pixeles=ESTILO_GUI_COLOR_WAVEFORM;

    //Obtener umbrales de aviso. Por porcentajes
    int upper_warning_perc=sensors_array[sensor_id].upper_warning_perc;
    int lower_warning_perc=sensors_array[sensor_id].lower_warning_perc;

    if (media_cpu_perc>upper_warning_perc || media_cpu_perc<lower_warning_perc) {
        color_pixeles=ESTILO_GUI_COLOR_AVISO;
        tinta_texto=ESTILO_GUI_COLOR_AVISO;
    }

    //Obtener umbrales de aviso. Por valores
    int valor_cpu=sensor_get_value_by_id(sensor_id);
    int upper_warning_value=sensors_array[sensor_id].upper_warning_value;
    int lower_warning_value=sensors_array[sensor_id].lower_warning_value;

    if (valor_cpu>upper_warning_value || valor_cpu<lower_warning_value) {
        color_pixeles=ESTILO_GUI_COLOR_AVISO;
        tinta_texto=ESTILO_GUI_COLOR_AVISO;
    }

    char buffer_texto_meters[100];
    if (valor_en_vez_de_perc) {
        sprintf(buffer_texto_meters,"%s %d",display_name,valor_cpu);
    }
    else {
        sprintf(buffer_texto_meters,"%s %3d%%",display_name,media_cpu_perc);
    }
    //zxvision_print_string_defaults(ventana,columna_texto,fila_texto,buffer_texto_meters);

    //Rellenar primero con espacios
    int i;
    for (i=0;i<escribir_espacios;i++) {
        zxvision_print_char_simple(ventana,columna_texto+i,fila_texto,tinta_texto_descripcion,papel_texto_descripcion,0,' ');
    }

    zxvision_print_string(ventana,columna_texto,fila_texto,tinta_texto_descripcion,papel_texto_descripcion,0,buffer_texto_meters);

    if (tipo==ZXVISION_WIDGET_TYPE_SPEEDOMETER) {
        int longitud_linea=ZXVISION_WIDGET_TYPE_SPEEDOMETER_LINE_LENGTH;

        int yorigen_linea=(fila_texto*menu_char_height)+longitud_linea+menu_char_height*2;  //menu_char_height*2 para que este dos lineas por debajo del texto
        int xcentro_widget=(columna_texto*menu_char_width)+longitud_linea; //Para ajustarlo por la derecha

        zxvision_widgets_draw_speedometer_common(ventana,xcentro_widget,yorigen_linea,media_cpu_perc,color_pixeles,color_pixeles);

    }

    if (tipo==ZXVISION_WIDGET_TYPE_SPEAKER) {
        int concentrico=1;

        int radio_circulo=ZXVISION_WIDGET_TYPE_SPEAKER_RADIUS;

        //Cuadrado
        int xorig=(columna_texto*menu_char_width);
        int yorig=(fila_texto*menu_char_height)+menu_char_height;
        int ancho=radio_circulo*2;
        int alto=radio_circulo*2+menu_char_height;

        int y;
        for (y=0;y<alto;y++) {
            //un poco redondeado por arriba
            if (y==0 || y==alto-1) zxvision_draw_line(ventana,xorig+1,yorig+y,xorig+ancho-1,yorig+y,ESTILO_GUI_TINTA_NORMAL,zxvision_putpixel);
            else zxvision_draw_line(ventana,xorig,yorig+y,xorig+ancho,yorig+y,ESTILO_GUI_TINTA_NORMAL,zxvision_putpixel);
        }

        int ycirculo=(fila_texto*menu_char_height)+(radio_circulo)+menu_char_height*2;  //menu_char_height*2 para que este dos lineas por debajo del texto
        int xcirculo=(columna_texto*menu_char_width)+radio_circulo;

        zxvision_widgets_draw_circle_ellipse(ventana,xcirculo,ycirculo,media_cpu_perc,color_pixeles,radio_circulo-3,radio_circulo-3,concentrico);

        //Y subwoofer fijo
        zxvision_draw_ellipse(ventana,xcirculo,yorig+5,4,4,color_pixeles,zxvision_putpixel,360);

    }

    if (tipo==ZXVISION_WIDGET_TYPE_CIRCLE || tipo==ZXVISION_WIDGET_TYPE_CIRCLE_CONCEN) {
        int concentrico=(tipo==ZXVISION_WIDGET_TYPE_CIRCLE_CONCEN ? 1 : 0);

        int radio_circulo=ZXVISION_WIDGET_TYPE_CIRCLE_RADIUS;

        int ycirculo=(fila_texto*menu_char_height)+(radio_circulo)+menu_char_height*2;  //menu_char_height*2 para que este dos lineas por debajo del texto
        int xcirculo=(columna_texto*menu_char_width)+radio_circulo;

        zxvision_widgets_draw_circle_ellipse(ventana,xcirculo,ycirculo,media_cpu_perc,color_pixeles,radio_circulo,radio_circulo,concentrico);

    }

    if (tipo==ZXVISION_WIDGET_TYPE_ELLIPSE || tipo==ZXVISION_WIDGET_TYPE_ELLIPSE_CONCEN) {
        int concentrico=(tipo==ZXVISION_WIDGET_TYPE_ELLIPSE_CONCEN ? 1 : 0);

        int radio_circulo_x=ZXVISION_WIDGET_TYPE_CIRCLE_RADIUS*2;
        int radio_circulo_y=ZXVISION_WIDGET_TYPE_CIRCLE_RADIUS;

        int ycirculo=(fila_texto*menu_char_height)+(radio_circulo_y)+menu_char_height*2;  //menu_char_height*2 para que este dos lineas por debajo del texto
        int xcirculo=(columna_texto*menu_char_width)+radio_circulo_x;

        zxvision_widgets_draw_circle_ellipse(ventana,xcirculo,ycirculo,media_cpu_perc,color_pixeles,radio_circulo_x,radio_circulo_y,concentrico);

    }

    if (tipo==ZXVISION_WIDGET_TYPE_CURVE) {
        int longitud_linea=ZXVISION_WIDGET_TYPE_CURVE_LENGTH;

        int yorigen_linea=(fila_texto*menu_char_height)+longitud_linea+menu_char_height*2;  //menu_char_height*2 para que este dos lineas por debajo del texto
        int xorigen_widget=(columna_texto*menu_char_width);

        zxvision_widgets_draw_curve_common(ventana,xorigen_widget,yorigen_linea,media_cpu_perc,color_pixeles,longitud_linea);

    }

    if (tipo==ZXVISION_WIDGET_TYPE_PARTICLES) {
        int longitud_linea=ZXVISION_WIDGET_TYPE_PARTICLES_RADIUS;

        int yorigen_linea=(fila_texto*menu_char_height)+longitud_linea+menu_char_height*2;  //menu_char_height*2 para que este dos lineas por debajo del texto
        int xorigen_widget=(columna_texto*menu_char_width);

        zxvision_widgets_draw_particles(ventana,xorigen_widget,yorigen_linea,media_cpu_perc,color_pixeles,longitud_linea);

    }

    if (tipo==ZXVISION_WIDGET_TYPE_SPHERE) {
        int longitud_linea=ZXVISION_WIDGET_TYPE_SPHERE_RADIUS;

        int yorigen_linea=(fila_texto*menu_char_height)+longitud_linea+menu_char_height*2;  //menu_char_height*2 para que este dos lineas por debajo del texto
        int xorigen_widget=(columna_texto*menu_char_width);

        zxvision_widgets_draw_sphere(ventana,xorigen_widget,yorigen_linea,media_cpu_perc,color_pixeles,longitud_linea);

    }

    if (tipo==ZXVISION_WIDGET_TYPE_SIERPINSKY) {

        int xorigen_widget=(columna_texto*menu_char_width);
        int yorigen_widget=(fila_texto*menu_char_height)+menu_char_height*2;  //menu_char_height*2 para que este dos lineas por debajo del texto;

        zxvision_widgets_draw_sierpinsky(ventana,xorigen_widget,yorigen_widget,media_cpu_perc,color_pixeles);

    }

    if (tipo==ZXVISION_WIDGET_TYPE_VOLUME) {
        zxvision_widgets_draw_volumen_maxmin(ventana,columna_texto,fila_texto,tinta_texto,ESTILO_GUI_PAPEL_NORMAL,media_cpu_perc,15);

    }

    if (tipo==ZXVISION_WIDGET_TYPE_VALUE) {
        //Solo texto. Sin dibujo
        //zxvision_widgets_draw_volumen_maxmin(ventana,columna_texto,fila_texto,tinta_texto,ESTILO_GUI_PAPEL_NORMAL,media_cpu_perc,15,display_name);

    }


}


int mouse_is_dragging=0;
int window_is_being_moved=0;
int window_is_being_resized=0;
int configurable_icon_is_being_moved=0;
int configurable_icon_is_being_moved_which=-1;
int configurable_icon_is_being_moved_previous_x=0;
int configurable_icon_is_being_moved_previous_y=0;

//ultima posicion que se ha dibujado arrastrando
int configurable_icon_is_being_moved_previous_dragged_x=0;
int configurable_icon_is_being_moved_previous_dragged_y=0;

//Para evitar que se mueva un icono sin querer si pulsamos y movemos un poco el raton
int zxvision_posicion_inicial_mover_icono_x=0;
int zxvision_posicion_inicial_mover_icono_y=0;

int window_mouse_x_before_move=0;
int window_mouse_y_before_move=0;

int last_x_mouse_clicked=0;
int last_y_mouse_clicked=0;
int mouse_is_clicking=0;
int menu_mouse_left_double_click_counter=0;
int menu_mouse_left_double_click_counter_initial=0;

int mouse_is_double_clicking=0;


void zxvision_handle_mouse_move_aux(zxvision_window *w)
{
                int movimiento_x=menu_mouse_x-window_mouse_x_before_move;
                int movimiento_y=menu_mouse_y-window_mouse_y_before_move;

                //printf ("Windows has been moved. menu_mouse_x: %d (%d) menu_mouse_y: %d (%d)\n",menu_mouse_x,movimiento_x,menu_mouse_y,movimiento_y);



                //Actualizar posicion
                int new_x=w->x+movimiento_x;
                int new_y=w->y+movimiento_y;


                zxvision_set_x_position(w,new_x);
                zxvision_set_y_position(w,new_y);
}

void zxvision_handle_mouse_resize_aux(zxvision_window *w)
{
                int incremento_ancho=menu_mouse_x-(w->visible_width)+1;
                int incremento_alto=menu_mouse_y-(w->visible_height)+1;

                //printf ("Incremento %d x %d\n",incremento_ancho,incremento_alto);

                int ancho_final=(w->visible_width)+incremento_ancho;
                int alto_final=(w->visible_height)+incremento_alto;

                //Evitar ventana de ancho pequeño, aunque se puede hacer, pero las franjas de colores se van por la izquierda
                if (ancho_final>=ZXVISION_MINIMUM_WIDTH_WINDOW) {
                    zxvision_set_visible_width(w,ancho_final);
                }

                //Evitar ventana de alto 1, aunque se puede hacer, pero luego no habria zona de redimensionado
                if (alto_final>1) {
                    zxvision_set_visible_height(w,alto_final);
                }
}

int zxvision_mouse_in_bottom_right(zxvision_window *w)
{
    if (menu_mouse_x==(w->visible_width)-1 && menu_mouse_y==w->visible_height-1) return 1;

    return 0;
}


void zxvision_handle_click_minimize(zxvision_window *w)
{

    if (w->can_be_minimized) {

        //Para cualquiera de los dos casos, la ponemos como minimizada
        //Luego en restaurar, restauramos valores originales
        //Se hace asi para que se pueda partir de un tamaño minimo y poder restaurar a su tamaño original
        //Si no, las funciones de establecer x,y, ancho, alto, podrian detectar fuera de rango de pantalla y no restaurar

        //Cambiar alto
        zxvision_set_visible_height(w,2);

        //Cambiar ancho
        //primero poner ancho inicial y luego reducir a ancho minimo para que quepa el titulo
        zxvision_set_visible_width(w,w->width_before_max_min_imize);

        int ancho_ventana_final=menu_dibuja_ventana_ret_ancho_titulo(w->visible_width,w->window_title);

        //printf ("ancho final: %d\n",ancho_ventana_final);
        zxvision_set_visible_width(w,ancho_ventana_final);

        //Al minimizar/restaurar, desactivamos maximizado
        w->is_maximized=0;

        if (w->is_minimized) {
            //Des-minimizar. Dejar posicion y tamaño original
            //printf("Desminimizar. before min: %d X %d\n",w->width_before_max_min_imize,w->height_before_max_min_imize);
            //printf ("Unminimize window. set size to %dX%d\n",w->height_before_max_min_imize,w->width_before_max_min_imize);
            zxvision_set_x_position(w,w->x_before_max_min_imize);
            zxvision_set_y_position(w,w->y_before_max_min_imize);
            zxvision_set_visible_height(w,w->height_before_max_min_imize);
            zxvision_set_visible_width(w,w->width_before_max_min_imize);
            w->is_minimized=0;
        }

        else {
            //Ya la hemos minimizado antes. solo indicarlo
            //printf ("Minimize window\n");
            //printf("Minimizar. before min: %d X %d\n",w->width_before_max_min_imize,w->height_before_max_min_imize);
            w->is_minimized=1;

            //Almacenar ultima posicion en caso que se haya movido
            w->x_before_max_min_imize=w->x;
            w->y_before_max_min_imize=w->y;
        }

        zxvision_draw_window(w);
        zxvision_draw_window_contents(w);

        /*
        Si está el setting de "Ocultar ventanas minimizadas",
        al minimizar se ve inicialmente la ventana minimizada, y al refrescar, desaparece.
        Esto es un efecto no provocado aunque creo que queda bien
        aunque si se pulsa en el titulo de la ventana, desaparece pero se queda activa
        Es que para hacer que desapareciera habría que o bien pasar el foco a otra ventana o bien simular F6 de esa ventana,
        y se puede complicar desde todos los sitios donde llame a este minimizado.
        Mejor lo dejamos asi, se ve minimizada visible hasta que se conmute a otra
        */


    }

}

void zxvision_toggle_minimize_window(zxvision_window *w)
{
    if (w!=NULL) {

        //Luego simulamos accion de pulsar boton de minimizar ventana
        zxvision_handle_click_minimize(w);


        //Y guardar la geometria
        util_add_window_geometry_compact(w);
    }
}

void zxvision_minimize_window(zxvision_window *w)
{
    if (w!=NULL) {

        //Primero decimos que no esta minimizada
        w->is_minimized=0;

        zxvision_toggle_minimize_window(w);
    }
}

//Si es una ventana minimizada y se debe mostrar
int zxvision_show_minimized(zxvision_window *w)
{

    if (zxvision_hide_minimized_windows.v==0) return 1;

    //No se si esto puede suceder pero por si acaso
    if (w==NULL) return 1;

    //Si la ventana no se puede enviar a segundo plano, no ocultar
    if (!w->can_be_backgrounded) return 1;


    if (w->is_minimized) return 0;

    return 1;
}


void zxvision_handle_maximize(zxvision_window *w)
{

    if (w->can_be_resized) {

        //Para cualquiera de los dos casos, la ponemos como minimizada
        //Luego en restaurar, restauramos valores originales
        //Se hace asi para que se pueda partir de un tamaño minimo y poder restaurar a su tamaño original
        //Si no, las funciones de establecer x,y, ancho, alto, podrian detectar fuera de rango de pantalla y no restaurar

        //Cambiar alto
        zxvision_set_visible_height(w,2);

        //Cambiar ancho
        //primero poner ancho inicial y luego reducir a ancho minimo para que quepa el titulo
        zxvision_set_visible_width(w,w->width_before_max_min_imize);

        int ancho_ventana_final=menu_dibuja_ventana_ret_ancho_titulo(w->visible_width,w->window_title);

        //printf ("ancho final: %d\n",ancho_ventana_final);
        zxvision_set_visible_width(w,ancho_ventana_final);



        //Al maximizar/restaurar, desactivamos minimizado
        w->is_minimized=0;

        if (w->is_maximized) {
            //Des-minimizar. Dejar posicion y tamaño original
            debug_printf (VERBOSE_DEBUG,"Unmaximize window");
            zxvision_set_x_position(w,w->x_before_max_min_imize);
            zxvision_set_y_position(w,w->y_before_max_min_imize);
            zxvision_set_visible_height(w,w->height_before_max_min_imize);
            zxvision_set_visible_width(w,w->width_before_max_min_imize);

            w->is_maximized=0;
        }

        else {
            debug_printf (VERBOSE_DEBUG,"Maximize window");
            int max_width;
            int max_height;
            int xinicial;
            int yinicial;

            //Tratar esto diferente si hay zx desktop activado y si la apertura de ventanas es en zx desktop
            //En ese caso obtiene el maximo que cabe en zxdesktop
            if (if_zxdesktop_enabled_and_driver_allows() && screen_ext_desktop_place_menu) {
                xinicial=menu_origin_x();
                yinicial=0;

                max_width=menu_get_width_characters_ext_desktop();
                max_height=scr_get_menu_height();



                //Si hay botones parte superior zxdesktop, origen_y lo incrementamos
                if (menu_zxdesktop_upper_buttons_enabled.v && zxvision_topbar_menu_enabled.v==0) {
                    yinicial=EXT_DESKTOP_BUTTONS_TOTAL_SIZE/8;

                    //Y quitamos ese alto disponible para no sobreescribir botones inferiores
                    max_height-=(EXT_DESKTOP_BUTTONS_TOTAL_SIZE/8)*2;

                }



            }

            //En este caso obtiene el maximo total en pantalla
            else {
                xinicial=0;
                yinicial=0;
                max_width=scr_get_menu_width();
                max_height=scr_get_menu_height();
            }


            //printf ("visible width %d\n",max_width);
            //Almacenar ultima posicion en caso que se haya movido
            w->x_before_max_min_imize=w->x;
            w->y_before_max_min_imize=w->y;

            zxvision_set_x_position(w,xinicial);
            zxvision_set_y_position(w,yinicial);
            zxvision_set_visible_width(w,max_width);
            zxvision_set_visible_height(w,max_height);

            w->is_maximized=1;
        }

        zxvision_draw_window(w);
        zxvision_draw_window_contents(w);
    }

}

void zxvision_toggle_maximize_window(zxvision_window *w)
{
    if (w!=NULL) {

        //Luego simulamos accion de pulsar boton de maximizar ventana
        zxvision_handle_maximize(w);


        //Y guardar la geometria
        util_add_window_geometry_compact(w);
    }
}

void zxvision_maximize_window(zxvision_window *w)
{
    if (w!=NULL) {

        //Primero decimos que no esta maximizada
        w->is_maximized=0;

        zxvision_toggle_maximize_window(w);
    }
}

void zxvision_send_scroll_right_and_draw(zxvision_window *w)
{
                        //printf ("Pulsado en scroll derecha\n");
                        zxvision_send_scroll_right(w);

                        //Redibujar botones scroll. Esto es necesario solo en el caso que,
                        //al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
                        //se quedaria el color del boton invertido
                        zxvision_draw_horizontal_scroll_bar(w,0);
}

void zxvision_send_scroll_left_and_draw(zxvision_window *w)
{
                        //printf ("Pulsado en scroll izquierda\n");
                        zxvision_send_scroll_left(w);

                        //Redibujar botones scroll. Esto es necesario solo en el caso que,
                        //al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
                        //se quedaria el color del boton invertido
                        zxvision_draw_horizontal_scroll_bar(w,0);
}

void zxvision_send_scroll_up_and_draw(zxvision_window *w)
{
                        //printf ("Pulsado en scroll arriba\n");
                        zxvision_send_scroll_up(w);

                        //Redibujar botones scroll. Esto es necesario solo en el caso que,
                        //al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
                        //se quedaria el color del boton invertido
                        zxvision_draw_vertical_scroll_bar(w,0);
}

void zxvision_send_scroll_down_and_draw(zxvision_window *w)
{
                        //printf ("Pulsado en scroll abajo\n");
                        zxvision_send_scroll_down(w);

                        //Redibujar botones scroll. Esto es necesario solo en el caso que,
                        //al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
                        //se quedaria el color del boton invertido
                        zxvision_draw_vertical_scroll_bar(w,0);
}

//Si se habia pulsado en una ventana por debajo de la actual
int clicked_on_background_windows=0;

zxvision_window *which_window_clicked_on_background=NULL;

void zxvision_handle_mouse_ev_switch_back_wind(zxvision_window *ventana_pulsada)
{
    clicked_on_background_windows=1;
    which_window_clicked_on_background=ventana_pulsada;

    //desminimizar ventana si estaba minimizada
    if (ventana_pulsada->is_minimized) {
        //printf("desminimizar ventana\n");
        zxvision_toggle_minimize_window(ventana_pulsada);
    }

    //Se ha pulsado en otra ventana. Conmutar a dicha ventana. Cerramos el menu y todos los menus raíz
    salir_todos_menus=1;

    /*
    Estas decisiones son parecidas en casos:
    pulsar tecla menu cuando menu activo (menu_if_pressed_menu_button en menu_get_pressed_key_no_modifier), conmutar ventana, pulsar logo ZEsarUX en ext desktop
    */

    //Si la ventana activa permite ir a background, mandarla a background
    if (zxvision_current_window->can_be_backgrounded) {
        mouse_pressed_background_window=1;
    }

    //Si la ventana activa no permite ir a background, cerrarla
    else {
        mouse_pressed_close_window=1;
    }

}

//Comun para obtener posicion de raton y de botones de zxdesktop
void zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_common(int *p_x,int *p_y,int *p_xboton,int *p_yboton)
{

    //TODO: Estas posiciones donde estan los botones, se obtienen de manera distinta en las funciones:
    //menu_put_switch_zxdesktop_footer
    //zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_common
    //Aunque se obtienen de diferentes maneras pero el resultado final (en teoria) es el mismo

    int mouse_pixel_x,mouse_pixel_y;
    menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

    //printf("si pulsado en boton switch zxdesktop. x %d y %d\n",mouse_pixel_x,mouse_pixel_y);

    int x=mouse_x;
    int y=mouse_y;
    //Quitarle el zoom
    x=x/zoom_x;
    y=y/zoom_y;

    //y la escala de 8
    x /=8;
    y /=8;
    //printf("si pulsado en boton switch zxdesktop. mouse_x %d mouse_y %d\n",x,y);

    //donde esta el boton
    //int yboton=screen_get_emulated_display_height_no_zoom_border_en()/8;
    int yboton=(screen_get_emulated_display_height_no_zoom_border_en()+screen_get_ext_desktop_height_no_zoom()) /8;

    int xboton=screen_get_window_size_width_no_zoom_border_en()/8-2; //justo 2 posicion menos
    //esta es la posicion x de los botones de +- ancho zx desktop
    //printf("si pulsado en boton switch zxdesktop. xboton %d yboton %d x %d y %d\n",xboton,yboton,x,y);

    *p_x=x;
    *p_y=y;
    *p_xboton=xboton;
    *p_yboton=yboton;
}


//Si ampliar_reducir_ancho=1, dice si posicion de arriba de ampliar ancho
//Si no, dice posicion de abajo de reducir ancho
int zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_width(int ampliar_reducir_ancho)
{
    if (zxvision_if_lower_button_switch_zxdesktop_visible() && mouse_left) {

        int x,y,xboton,yboton;

        zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_common(&x,&y,&xboton,&yboton);

        //Boton arriba: ampliar ancho
        if (ampliar_reducir_ancho) {
            if (x==xboton && y==yboton) {
                debug_printf(VERBOSE_INFO,"Pressed on ZX Desktop enlarge width button");
                return 1;
            }
        }

        //Boton abajo: reducir ancho
        else {
            if (x==xboton && y==yboton+1) {
                debug_printf(VERBOSE_INFO,"Pressed on ZX Desktop reduce width button");
                return 1;
            }
        }


    }

    return 0;
}

//Si ampliar_reducir_ancho=1, dice si posicion ampliar alto
//Si no, dice posicion de reducir alto
int zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_height(int ampliar_reducir_alto)
{
    if (zxvision_if_lower_button_switch_zxdesktop_visible() && mouse_left) {

        int x,y,xboton,yboton;

        zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_common(&x,&y,&xboton,&yboton);

        //Los de cambio de alto estan en posicion x -1
        xboton--;

        //Boton abajo: ampliar alto
        if (ampliar_reducir_alto) {
            if (x==xboton && y==yboton+1) {
                debug_printf(VERBOSE_INFO,"Pressed on ZX Desktop enlarge height button");
                return 1;
            }
        }

        //Boton arriba: reducir alto
        else {
            if (x==xboton && y==yboton) {
                debug_printf(VERBOSE_INFO,"Pressed on ZX Desktop reduce height button");
                return 1;
            }
        }


    }

    return 0;
}


int zxvision_if_mouse_in_lower_button_enlarge_zxdesktop_width(void)
{
    return zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_width(1);
}

int zxvision_if_mouse_in_lower_button_reduce_zxdesktop_width(void)
{
    return zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_width(0);
}

int zxvision_if_mouse_in_lower_button_enlarge_zxdesktop_height(void)
{
    return zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_height(1);
}

int zxvision_if_mouse_in_lower_button_reduce_zxdesktop_height(void)
{
    return zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_height(0);
}

//Funcion de detectar si raton en botones upper, lower o iconos de desktop, pero ademas
//con variable activar_accion que dice si se dispara la accion que habilita ese boton
//Esa variable sirve para que en casos que solo queremos comprobar que el raton esta ahi pero sin disparar la accion
int zxvision_if_mouse_in_zlogo_or_buttons_desktop_if_trigger(int activar_accion)
{

    //printf("zxvision_if_mouse_in_zlogo_or_buttons_desktop. traza: \n");
    //debug_exec_show_backtrace();
    //printf("zxvision_if_mouse_in_zlogo_or_buttons_desktop. fin traza\n");


    //Ver si estamos por la zona del logo en el ext desktop o de los botones
    if (screen_ext_desktop_enabled && scr_driver_can_ext_desktop() ) {

        int mouse_pixel_x,mouse_pixel_y;
        menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

        //multiplicamos por zoom
        mouse_pixel_x *=zoom_x;
        mouse_pixel_y *=zoom_y;



        //Si esta en zona botones de zx desktop. Y si estan habilitados

        if (menu_zxdesktop_upper_buttons_enabled.v && zxvision_topbar_menu_enabled.v==0) {
            int ancho_boton,alto_boton,total_botones,xinicio_botones,xfinal_botones;
            menu_ext_desktop_buttons_get_geometry(&ancho_boton,&alto_boton,&total_botones,&xinicio_botones,&xfinal_botones);

            if (mouse_pixel_x>=xinicio_botones && mouse_pixel_x<xfinal_botones &&
                mouse_pixel_y>=0 && mouse_pixel_y<alto_boton
            ) {
                //printf ("Pulsado en zona botones del ext desktop\n");

                //en que boton?
                int numero_boton=(mouse_pixel_x-xinicio_botones)/ancho_boton;
                DBG_PRINT_ZXVISION_EVENTS VERBOSE_DEBUG,"ZXVISION_EVENTS: zxvision_if_mouse_in_zlogo_or_buttons_desktop. Upper button pressed: %d",numero_boton);
                if (activar_accion) {
                    menu_pressed_zxdesktop_button_which=numero_boton;

                    //Puede ser un poco redundante que desde aqui tambien miremos boton derecho,
                    //porque se llama desde zxvision_if_mouse_in_zlogo_or_buttons_desktop al pulsar boton izquierdo
                    //pero se usa al abrirse el menu_inicio con boton derecho
                    if (mouse_right) menu_pressed_zxdesktop_button_which_right_button=1;
                }

                return 1;
            }
        }


        //Si esta en zona de iconos lower de zx desktop. Y si estan habilitados

        if (menu_zxdesktop_lower_buttons_enabled.v) {
            int ancho_boton,alto_boton,xinicio_botones,xfinal_botones,yinicio_botones;
            menu_ext_desktop_lower_icons_get_geometry(&ancho_boton,&alto_boton,NULL,&xinicio_botones,&xfinal_botones,&yinicio_botones);

            if (mouse_pixel_x>=xinicio_botones && mouse_pixel_x<xfinal_botones &&
                mouse_pixel_y>=yinicio_botones && mouse_pixel_y<yinicio_botones+alto_boton
            ) {
                //printf ("Pulsado en zona lower icons del ext desktop\n");

                //en que boton?
                int numero_boton=(mouse_pixel_x-xinicio_botones)/ancho_boton;
                //printf("boton pulsado: %d\n",numero_boton);

                //Buscar indice array
                int indice_array=zxdesktop_lowericon_find_index(numero_boton);

                if (indice_array>=0) {



                    //printf ("boton esta visible\n");
                    DBG_PRINT_ZXVISION_EVENTS VERBOSE_INFO,"ZXVISION_EVENTS: Lower icon pressed: %d",numero_boton);

                        if (activar_accion) {
                            menu_pressed_zxdesktop_lower_icon_which=numero_boton;

                            //Puede ser un poco redundante que desde aqui tambien miremos boton derecho,
                            //porque se llama desde zxvision_if_mouse_in_zlogo_or_buttons_desktop al pulsar boton izquierdo
                            //pero se usa al abrirse el menu_inicio con boton derecho

                            if (mouse_right) menu_pressed_zxdesktop_lower_icon_which_right_button=1;
                        }

                        return 1;
                }
                else {
                    //printf ("boton NO esta visible\n");
                }

            }
        }


        //si pulsa en algun icono configurable
        if (zxdesktop_configurable_icons_enabled_and_visible()) {
            //aqui tanto entra cuando se pulsa como cuando se libera
            //printf("mouse %d %d\n",mouse_pixel_x,mouse_pixel_y);
            int icono_pulsado=if_position_in_desktop_icons(mouse_pixel_x,mouse_pixel_y);

            //Si se pulsa alguno y si no habiamos pulsado ya (estamos arrastrando)
            if (icono_pulsado>=0 && menu_pressed_zxdesktop_configurable_icon_which==-1) {
                debug_printf(VERBOSE_DEBUG,"Icon %d pressed",icono_pulsado);

                //debug_exec_show_backtrace();

                if (activar_accion) {
                    menu_pressed_zxdesktop_configurable_icon_which=icono_pulsado;
                    //printf("Establecer menu_pressed_zxdesktop_configurable_icon_which desde linea 16107\n");

                    //Puede ser un poco redundante que desde aqui tambien miremos boton derecho,
                    //porque se llama desde zxvision_if_mouse_in_zlogo_or_buttons_desktop al pulsar boton izquierdo
                    //pero se usa al abrirse el menu_inicio con boton derecho
                    if (mouse_right) menu_pressed_zxdesktop_configurable_icon_right_button=1;


                    //Para saber si se arrastra
                    menu_pressed_zxdesktop_configurable_icon_where_x=mouse_pixel_x;
                    menu_pressed_zxdesktop_configurable_icon_where_y=mouse_pixel_y;
                }

                return 1;
            }
        }
    }
    return 0;
}


int zxvision_if_mouse_in_zlogo_or_buttons_desktop(void)
{
    return zxvision_if_mouse_in_zlogo_or_buttons_desktop_if_trigger(1);
}

//Decir que el siguiente menu se abre en posicion fija
//Coordenadas en caracteres
void zxvision_set_next_menu_position(int x,int y)
{
    force_next_menu_position.v=1;

    force_next_menu_position_x=x;
    force_next_menu_position_y=y;
}

void zxvision_reset_set_next_menu_position(void)
{
    force_next_menu_position.v=0;
}


void zxvision_set_next_menu_position_from_current_mouse(void)
{
    int absolute_mouse_x,absolute_mouse_y;

    menu_calculate_mouse_xy_absolute_interface(&absolute_mouse_x,&absolute_mouse_y);
    zxvision_set_next_menu_position(absolute_mouse_x,absolute_mouse_y);
}

//Dice si raton en zona de zxdesktop, o sea:
//No en pantalla emulada
//No en footer
//No en botones superiores ni inferiores
int zxvision_if_mouse_in_background(void)
{
    int mouse_pixel_x,mouse_pixel_y;
    menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

    //multiplicamos por zoom
    mouse_pixel_x *=zoom_x;
    mouse_pixel_y *=zoom_y;

    int total_ancho_zxdesktop=screen_get_total_width_window_plus_zxdesktop();
    int total_alto_zxdesktop=screen_get_total_height_window_no_footer_plus_zxdesktop();

    //si no esta dentro de la ventana (y excluyendo footer)
    if (mouse_pixel_x>=total_ancho_zxdesktop || mouse_pixel_y>=total_alto_zxdesktop) {
        return 0;
    }

    //si esta dentro de pantalla emulada
    int ancho_maquina=screen_get_emulated_display_width_zoom_border_en();
    int alto_maquina=screen_get_emulated_display_height_zoom_border_en();

    if (mouse_pixel_x<ancho_maquina && mouse_pixel_y<alto_maquina) {
        return 0;
    }

    //si apunta a botones inferiores o superiores
    int xinicio_botones,xfinal_botones,yinicio_botones,alto_boton;

    //Ver si en posicion de iconos superiores
    menu_ext_desktop_buttons_get_geometry(NULL,&alto_boton,NULL,&xinicio_botones,&xfinal_botones);
    if (mouse_pixel_y<=alto_boton && mouse_pixel_x>=xinicio_botones && mouse_pixel_x<=xfinal_botones) {
        //printf("On upper buttons\n");
        return 0;
    }

    //Ver si en posicion de iconos inferiores
    menu_ext_desktop_lower_icons_get_geometry(NULL,NULL,NULL,&xinicio_botones,&xfinal_botones,&yinicio_botones);
    if (mouse_pixel_y>=yinicio_botones && mouse_pixel_x>=xinicio_botones && mouse_pixel_x<xfinal_botones) {
        //printf("On lower buttons\n");
        return 0;
    }


    return 1;
}

int zxvision_if_mouse_in_zlogo_or_buttons_desktop_right_button(void)
{


    //Ver si estamos por la zona del logo en el ext desktop o de los botones
    if (screen_ext_desktop_enabled && scr_driver_can_ext_desktop() ) {

        //printf("Boton derecho zxvision_if_mouse_in_zlogo_or_buttons_desktop_right_button\n");

        int mouse_pixel_x,mouse_pixel_y;
        menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

        //multiplicamos por zoom
        mouse_pixel_x *=zoom_x;
        mouse_pixel_y *=zoom_y;


        //Si se pulsa boton derecho en alguna ventana
        int absolute_mouse_x,absolute_mouse_y;

        menu_calculate_mouse_xy_absolute_interface(&absolute_mouse_x,&absolute_mouse_y);

        //Vamos a ver en que ventana se ha pulsado, si tenemos background activado
        zxvision_window *ventana_pulsada;

        ventana_pulsada=NULL;

        if (zxvision_current_window!=NULL) {
            //ventana_pulsada=zxvision_coords_in_below_windows(zxvision_current_window,absolute_mouse_x,absolute_mouse_y);
            ventana_pulsada=zxvision_coords_in_any_window(absolute_mouse_x,absolute_mouse_y);
        }

        //printf("Pulsado boton derecho. zxvision_current_window %p ventana_pulsada: %p si_menu_mouse_en_ventana: %d"
        //        " menu_mouse_x %d menu_mouse_y %d current_win_ancho %d current_win_alto %d\n",
        //        zxvision_current_window,ventana_pulsada,si_menu_mouse_en_ventana(),menu_mouse_x,menu_mouse_y,current_win_ancho,current_win_alto);

        //if (ventana_pulsada!=NULL || si_menu_mouse_en_ventana()) {
        if (ventana_pulsada!=NULL) {
            //printf("Pulsado boton derecho sobre ventana\n");

        }
        else {
            //No se pulsa en ventanas.
            //printf("No pulsado en ventanas\n");

            //Ver si en iconos del zx desktop
            //si pulsa en algun icono configurable, con menu cerrado
            //aqui tanto entra cuando se pulsa como cuando se libera
            //printf("mouse %d %d\n",mouse_pixel_x,mouse_pixel_y);
            int icono_pulsado=if_position_in_desktop_icons(mouse_pixel_x,mouse_pixel_y);
            //printf("Icono pulsado %d menu_pressed_zxdesktop_configurable_icon_which %d\n",icono_pulsado,menu_pressed_zxdesktop_configurable_icon_which);

            //Si se pulsa alguno
            if (icono_pulsado>=0 && zxdesktop_configurable_icons_enabled_and_visible()) {
                //printf("Icono pulsado desde zxvision_if_mouse_in_zlogo_or_buttons_desktop_right_button: %d\n",icono_pulsado);
                //printf("menu_pressed_zxdesktop_configurable_icon_which=%d\n",menu_pressed_zxdesktop_configurable_icon_which);

                //y si no habiamos pulsado ya (estamos arrastrando)
                if (menu_pressed_zxdesktop_configurable_icon_which==-1) {

                    //debug_exec_show_backtrace();

                    //printf("Icono pulsado desde zxvision_if_mouse_in_zlogo_or_buttons_desktop_right_button con boton derecho: %d\n",icono_pulsado);

                    menu_pressed_zxdesktop_configurable_icon_which=icono_pulsado;
                    menu_pressed_zxdesktop_configurable_icon_right_button=1;

                    //Para saber si se arrastra
                    menu_pressed_zxdesktop_configurable_icon_where_x=mouse_pixel_x;
                    menu_pressed_zxdesktop_configurable_icon_where_y=mouse_pixel_y;


                    zxvision_set_next_menu_position_from_current_mouse();

                }


                return 1;
            }

            //Ver si en fondo desktop

            if (zxdesktop_configurable_icons_enabled_and_visible()) {

                if (zxvision_if_mouse_in_background()) {

                    //printf("Pulsado en ZX desktop con boton derecho desde zxvision_if_mouse_in_zlogo_or_buttons_desktop_right_button\n");

                    menu_pressed_zxdesktop_right_button_background=1;

                    //de momento no altero posicion menu si se pulsa en background
                    //zxvision_set_next_menu_position(absolute_mouse_x,absolute_mouse_y);

                    return 1;

                }

            }

            //printf("Pulsado en cualquier otro sitio\n");



        }
    }
    return 0;
}



z80_byte zxvision_get_char_at_position(zxvision_window *w,int x,int y,int *inverso)
{

    //Asumimos
    *inverso=0;

    overlay_screen caracter;

    zxvision_get_character_at_mouse(w,x,y,&caracter);

    //printf ("Caracter: %c (%d)\n",(caracter.caracter>31 && caracter.caracter<126 ? caracter.caracter : '.') ,caracter.caracter);

    //Interpretar si es inverso
    if (caracter.caracter>=32 && caracter.caracter<=126) {

        //En el caso habitual de letra con texto inverso
        if (ESTILO_GUI_INVERSE_TINTA==-1) {

            if (caracter.tinta==ESTILO_GUI_PAPEL_NORMAL && caracter.papel==ESTILO_GUI_TINTA_NORMAL) {
                //printf ("Caracter %c es tratado como inverso\n",caracter.caracter);
                *inverso=1;

            }
        }

        //Caso de letra hotkey como estilo turbovision y otros en que es solo cambio de tinta
        else {
            if (caracter.tinta==ESTILO_GUI_INVERSE_TINTA && caracter.papel==ESTILO_GUI_PAPEL_NORMAL) {
                //printf ("Caracter %c es tratado como inverso (aunque solo cambia tinta)\n",caracter.caracter);
                *inverso=1;

            }
        }
    }


    return caracter.caracter;
}

z80_byte zxvision_get_key_hotkey(zxvision_window *w,int x,int y)
{


    //int xorig=x;

    int inverso;
            /*
-Hot key ratón:

-buscar a la izquierda hasta x cero o espacio
-de ahí hacia la derecha hasta espacio, final de ancho o : puntos
-contar caracteres inverso: si solo 1, enviar Tecla. Si es más de 1 puede ser “enter” y por tanto ignorar. O si es “ent” enviar 13

-cuidado con Y siendo mayor que el rango
            */

            //Ir hacia inicio del todo a la izquierda



    for (;x>=0;x--) {


        z80_byte caracter=zxvision_get_char_at_position(w,x,y,&inverso);

        //Espacio, salir
        if (caracter==32) break;

    }

    x++;

    //de ahí hacia la derecha hasta espacio, final de ancho o : puntos
    //-contar caracteres inverso: si solo 1, enviar Tecla. Si es más de 1 puede ser “enter” y por tanto ignorar. O si es “ent” enviar 13

    int total_inversos=0;

    z80_byte caracter_inverso=0;

    for (;x<=w->visible_width;x++) {


        z80_byte caracter=zxvision_get_char_at_position(w,x,y,&inverso);

        //printf ("X %d Y %d car: %c inverso: %d\n",x,y,caracter,inverso);

        //Interpretar si es inverso
        if (inverso) {
            total_inversos++;
            caracter_inverso=caracter;
        }

        //Espacio, salir
        if (caracter==32 || caracter==':') break;

    }

    if (total_inversos==1 && caracter_inverso!=0) {
        //printf ("Detectada tecla hotkey: %c\n",caracter_inverso);
        return caracter_inverso;
    }



    return 0;
}

//x,y en coordenadas sin zoom, de la posicion del cursor
int zxvision_si_icono_cerca_de_papelera(int icono,int mouse_pixel_x,int mouse_pixel_y)
{
    //int mover_a_papelera=0;
    int hay_papelera=zxvision_search_trash_configurable_icon();
    if (hay_papelera>=0) {
        //printf("hay una papelera\n");
        //Y siempre que no sea ya una papelera este icono
        int indice_funcion=zxdesktop_configurable_icons_list[icono].indice_funcion;
        enum defined_f_function_ids id_funcion=defined_direct_functions_array[indice_funcion].id_funcion;

        if (id_funcion!=F_FUNCION_DESKTOP_TRASH) {

            int xpapelera=zxdesktop_configurable_icons_list[hay_papelera].pos_x;
            int ypapelera=zxdesktop_configurable_icons_list[hay_papelera].pos_y;

            //Ver si cerca
            int deltax=util_get_absolute(mouse_pixel_x-xpapelera);
            int deltay=util_get_absolute(mouse_pixel_y-ypapelera);

            //printf("Distancia a la papelera: %d,%d\n",deltax,deltay);

            if (deltax<=20*menu_gui_zoom && deltay<=20*menu_gui_zoom) {
                return 1;
            }
        }
    }

    return 0;
}

void zxvision_mover_icono_papelera_si_conviene(void)
{
    if (configurable_icon_is_being_moved_which>=0) {
        //Ver si se ha movido a la papelera


        int mouse_pixel_x,mouse_pixel_y;
        menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

        /*
        //Ver si en el destino no hay cerca la papelera
        int mover_a_papelera=0;
        int hay_papelera=zxvision_search_trash_configurable_icon();
        if (hay_papelera>=0) {
            printf("hay una papelera\n");
            //Y siempre que no sea ya una papelera este icono
            int indice_funcion=zxdesktop_configurable_icons_list[configurable_icon_is_being_moved_which].indice_funcion;
            enum defined_f_function_ids id_funcion=defined_direct_functions_array[indice_funcion].id_funcion;

            if (id_funcion!=F_FUNCION_DESKTOP_TRASH) {

                int xpapelera=zxdesktop_configurable_icons_list[hay_papelera].pos_x;
                int ypapelera=zxdesktop_configurable_icons_list[hay_papelera].pos_y;

                //Ver si cerca
                int deltax=util_get_absolute(mouse_pixel_x-xpapelera);
                int deltay=util_get_absolute(mouse_pixel_y-ypapelera);

                printf("Distancia a la papelera: %d,%d\n",deltax,deltay);

                if (deltax<=20 && deltay<=20) {
                    printf("Mover icono a la papelera\n");

                    //Cambiarle la posicion que tenia inicial antes de ir a la papelera
                    zxvision_set_configurable_icon_position(configurable_icon_is_being_moved_which,configurable_icon_is_being_moved_previous_x,configurable_icon_is_being_moved_previous_y);
                    zxvision_move_configurable_icon_to_trash(configurable_icon_is_being_moved_which);
                }
            }
        }
        */

        //Ver si el icono se mueve a la papelera
        if (zxvision_si_icono_cerca_de_papelera(configurable_icon_is_being_moved_which,mouse_pixel_x,mouse_pixel_y)) {
            debug_printf(VERBOSE_DEBUG,"Move icon to the Trash");

            //printf("Mover icono a la papelera -- xx\n");

            //Si ya estaba en la papelera. Esto pasa en determinadas combinaciones al arrastrar icono, primero detecta arrastre y envia a la papelera,
            //pero luego gestiona pulsacion y detecta arrastre e intenta enviar a la papelera de nuevo
            int estado_icono=zxdesktop_configurable_icons_list[configurable_icon_is_being_moved_which].status;

            if (estado_icono==ZXDESKTOP_CUSTOM_ICON_DELETED) {
                debug_printf(VERBOSE_DEBUG,"Icon was already on the Trash, this may happen, it's pretty normal");
            }

            else {

                //Cambiarle la posicion que tenia inicial antes de ir a la papelera
                zxvision_set_configurable_icon_position(configurable_icon_is_being_moved_which,configurable_icon_is_being_moved_previous_x,configurable_icon_is_being_moved_previous_y);
                zxvision_move_configurable_icon_to_trash(configurable_icon_is_being_moved_which);
            }
        }
    }

    zxvision_draw_icon_papelera_abierta=0;
}

void zxvision_handle_mouse_events_stop_drag_icon(void)
{
    if (configurable_icon_is_being_moved) {

        debug_printf(VERBOSE_DEBUG,"Stopped moving configurable icon");
        //Parece que aqui solo se llama cuando esta el menu abierto


        configurable_icon_is_being_moved=0;
        //Para que cuando se vuelva a pulsar no interprete movimiento
        menu_pressed_zxdesktop_configurable_icon_where_x=99999;
        menu_pressed_zxdesktop_configurable_icon_where_y=99999;

        //printf("zxvision_mover_icono_papelera_si_conviene 1\n");
        //debug_exec_show_backtrace();

        zxvision_mover_icono_papelera_si_conviene();

        zxvision_reorder_configurable_icons_if_auto();

    }
}

void zxvision_handle_mouse_events_start_drag_icon(void)
{
    int mouse_pixel_x,mouse_pixel_y;
    menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

    //Conservar posicion inicial por si se mueve a la papelera, cuando se saque que retorne a dicha posicion
    configurable_icon_is_being_moved_previous_x=mouse_pixel_x;
    configurable_icon_is_being_moved_previous_y=mouse_pixel_y;

    //multiplicamos por zoom
    mouse_pixel_x *=zoom_x;
    mouse_pixel_y *=zoom_y;

    //printf("arrastrando mouse %d %d\n",mouse_pixel_x,mouse_pixel_y);
    configurable_icon_is_being_moved_which=if_position_in_desktop_icons(mouse_pixel_x,mouse_pixel_y);
    //printf("Icono arrastrando: %d\n",configurable_icon_is_being_moved_which);

    //Si se arrastra alguno
    if (configurable_icon_is_being_moved_which>=0) {
        configurable_icon_is_being_moved=1;

        //Indicar posicion inicial donde empieza a moverse, para evitar pulsaciones con ligero movimiento de raton
        //que provoca mover el icono
        zxvision_posicion_inicial_mover_icono_x=mouse_pixel_x;
        zxvision_posicion_inicial_mover_icono_y=mouse_pixel_y;

    }
}

void zxvision_handle_mouse_events_drag_icon(void)
{
    //AQUI MOVER ICONO
    if (configurable_icon_is_being_moved) {
        //printf("Icon %d being moved\n",configurable_icon_is_being_moved_which);
        //Actualizar posicion
        if (configurable_icon_is_being_moved_which>=0) {
            //printf("Moving icon %d\n",configurable_icon_is_being_moved_which);
            int mouse_pixel_x,mouse_pixel_y;
            menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);
            //printf("Moving icon %d to %d,%d\n",configurable_icon_is_being_moved_which,mouse_pixel_x,mouse_pixel_y);

            //Ver si se ha movido lo suficiente, al menos 10 pixeles
            //TODO: esto provoca que si queremos desplazar un icono menos de 10 pixeles, hay que hacerlo en dos veces:
            //1: moverlo lejos mas de 10 pixeles
            //2: moverlo hacia atras donde queriamos
            //Aunque hay que tener en cuenta que el icono se mueve a donde apunta el raton, por tanto, si apuntamos en medio del icono
            //y arrastramos mas de 10 pixeles, el icono saltara bastante mas de 10 pixeles, o sea, saltara 10 pixeles +
            //la distancia entre la esquina superior izquierda del icono y la posicion del raton

            //multiplicamos por zoom
            int mouse_pixel_x_zoom=mouse_pixel_x*zoom_x;
            int mouse_pixel_y_zoom=mouse_pixel_y*zoom_y;

            int deltax=util_abs(zxvision_posicion_inicial_mover_icono_x-mouse_pixel_x_zoom);
            int deltay=util_abs(zxvision_posicion_inicial_mover_icono_y-mouse_pixel_y_zoom);

            if (deltax>10 || deltay>10) {
                zxvision_set_configurable_icon_position(configurable_icon_is_being_moved_which,mouse_pixel_x,mouse_pixel_y);
            }
            else {
                debug_printf(VERBOSE_DEBUG,"Do not change icon position because it has not been moved enough");
            }

            //Refrescar pantalla si se ha movido lo suficiente

            deltax=util_abs(configurable_icon_is_being_moved_previous_dragged_x-mouse_pixel_x);
            deltay=util_abs(configurable_icon_is_being_moved_previous_dragged_y-mouse_pixel_y);


            if (deltax>0 || deltay>0) {


                //Ver si icono cerca de papelera
                if (zxvision_si_icono_cerca_de_papelera(configurable_icon_is_being_moved_which,mouse_pixel_x,mouse_pixel_y)) {
                    debug_printf(VERBOSE_DEBUG,"There is a Trash near the moved icon");
                    zxvision_draw_icon_papelera_abierta=1;
                }
                else {
                    zxvision_draw_icon_papelera_abierta=0;
                }


                //printf("Refrescar %d\n",contador_segundo);

                //Necesario para que veamos moverse el icono
                menu_draw_ext_desktop();

                if (!menu_multitarea) {
                    //printf ("refresca pantalla\n");
                    menu_refresca_pantalla();
                }

                configurable_icon_is_being_moved_previous_dragged_x=mouse_pixel_x;
                configurable_icon_is_being_moved_previous_dragged_y=mouse_pixel_y;
            }
        }


    }
}

//Gestionar arrastre de iconos
//esto solo lo llamo desde zxvision_handle_mouse_events cuando no hay ventanas abiertas
//Al final hay una pequeña gestion de activar mouse_is_dragging etc al igual que en zxvision_handle_mouse_events
//TODO: la idea es que solo hubiera una funcion zxvision_handle_mouse_events y que funcionase tanto si hubiera ventanas abiertas como si no
void zxvision_handle_mouse_events_on_icons(void)
{

    //printf("zxvision_handle_mouse_events_on_icons\n");
    if (!zxdesktop_configurable_icons_enabled_and_visible()) return;

    //printf("zxvision_handle_mouse_events_on_icons. icons enabled\n");

    if (!si_menu_mouse_activado()) return;

    menu_calculate_mouse_xy();

    if (!mouse_is_dragging) {
        if (mouse_left && mouse_movido) {
            debug_printf (VERBOSE_DEBUG,"Mouse has begun to drag");

            if (auto_frameskip_even_when_movin_windows.v==0) {
                autoframeskip_setting_before_moving_windows.v=autoframeskip.v;
                debug_printf(VERBOSE_DEBUG,"Disabling autoframeskip while moving or resizing window");
                autoframeskip.v=0;
            }

            mouse_is_dragging=1;


            //Esta en un icono?
            //Si estaba en un icono
            zxvision_handle_mouse_events_start_drag_icon();

        }
    }

    if (mouse_is_dragging) {
        //printf ("mouse is dragging\n");
        if (!mouse_left) {
            debug_printf (VERBOSE_DEBUG,"Mouse has stopped to drag");

            if (auto_frameskip_even_when_movin_windows.v==0) {
                autoframeskip.v=autoframeskip_setting_before_moving_windows.v;
            }

            mouse_is_dragging=0;
            mouse_is_clicking=0; //Cuando se deja de arrastrar decir que se deja de pulsar tambien

            zxvision_handle_mouse_events_stop_drag_icon();


        }

        else {
            //AQUI MOVER ICONO
            zxvision_handle_mouse_events_drag_icon();
        }
    }

}

int zxvision_pressed_right_mouse_button(void)
{
    if (mouse_right && menu_mouse_right_send_esc.v==0) return 1;
    else return 0;
}

void zxvision_handle_mouse_events_aux_open_menu_while_in_menu(void)
{
    menu_pressed_open_menu_while_in_menu.v=1;
    salir_todos_menus=1;

    /*
    Estas decisiones son parecidas en casos:
    pulsar tecla menu cuando menu activo (menu_if_pressed_menu_button en menu_get_pressed_key_no_modifier), conmutar ventana, pulsar logo ZEsarUX en ext desktop
    */

    if (!menu_allow_background_windows) {
            mouse_pressed_close_window=1;
    }

    else {
        //Si la ventana activa permite ir a background, mandarla a background
        if (zxvision_current_window->can_be_backgrounded) {
                mouse_pressed_background_window=1;
        }

        //Si la ventana activa no permite ir a background, cerrarla
        else {
                mouse_pressed_close_window=1;
        }
    }
}

//Para poder indicar desde el window manager que se ha cerrado una ventana y no queremos que se reabra el menu
//int pressed_close_window_on_window_manager=0;

//TODO: tiene sentido pasarle el parametro de ventana w, si siempre que llamamos aqui es con el mismo parametro? (zxvision_current_window)
void zxvision_handle_mouse_events(zxvision_window *w)
{

    //printf("inicio zxvision_handle_mouse_events\n");

    if (w==NULL) {
        //printf("Retorno de zxvision_handle_mouse_events porque w=NULL\n");

        //Pero llamamos a gestion movimiento iconos, que no son dependientes de la ventana
        zxvision_handle_mouse_events_on_icons();
        return; // 0;
    }

    //printf("inicio zxvision_handle_mouse_events. ventana no es NULL\n");

    menu_pressed_shift_cursor_window_doesnot_allow=0;

    //Capturar aquí el cambio de ventana mediante shift+cursor, como si fuera otro evento de mouse mas
    if (menu_pressed_shift_left || menu_pressed_shift_right) {
        menu_pressed_shift_left=0;
        menu_pressed_shift_right=0;

        //Si ventana no lo permite, simularemos pulsado ESC
        if (!(w->can_be_backgrounded)) {
            debug_printf(VERBOSE_DEBUG,"Pressed shift-left/right in a window that can not be switched. Closing it");
            menu_pressed_shift_cursor_window_doesnot_allow=1;
            return;
        }

        if (menu_allow_background_windows && zxvision_current_window!=NULL && w->can_be_backgrounded) {
            debug_printf(VERBOSE_DEBUG,"Switch to next window");
            menu_contador_conmutar_ventanas++;
            zxvision_window *pointer_window;

            pointer_window=w;

            zxvision_window *previous_window;

            //al conmutar ventanas siempre vamos solo de la anterior a la actual. Hay que tener
            //un contador de cuantas nos tenemos que mover
            /*
            Cuando se conmuta a una ventana, simplemente se mueve esa ventana arriba del todo: Ejemplo:

            Ventanas A,B,C,D siendo la primera la de mas arriba y siendo la ultima la de mas abajo en pantalla
            A
            B
            C
            D
            Si conmutamos a ventana B, se tiene:
            B
            A
            C
            D

            Con lo que se puede ver que el orden se ha alterado. Luego si conmutamos a ventana C, tenemos
            C
            B
            A
            D

            Y si conmutamos a D tenemos:
            D
            C
            B
            A

            Con lo que el orden se ha invertido completamente

            Es por eso que al pulsar en este ejemplo 3 veces shift+left, hemos invertido completamente el orden.
            Si volvemos a conmutar, subiendo A arriba tenemos:
            A
            D
            C
            B

            Con lo que, para el usuario ve la A arriba, pero la de abajo ya no es la B como inicialmente, sino la D, con lo que es confuso
            Realmente habria que tener una funcion de conmutar ventana que teniendo esto
            A
            B
            C
            D

            Al conmutar B arriba hiciera
            B
            A
            C
            D

            Pero esto de momento no es asi. TODO...

            */
            int i;
            for (i=0;i<menu_contador_conmutar_ventanas;i++) {
                previous_window=pointer_window->previous_window;

                //Llegado a la de abajo
                if (previous_window==NULL) {
                    //Y la siguiente ventana sera justo la inicial
                    pointer_window=w;

                }
                else {
                    pointer_window=previous_window;
                }
            }


            if (pointer_window!=NULL) {
                //Si es la misma ventana, hacer que sea la siguiente
                //Esto puede suceder dado que vamos buscando en el bucle de antes hasta contador menu_contador_conmutar_ventanas
                if (pointer_window==w) pointer_window=w->previous_window;

                if (pointer_window!=NULL) {

                    zxvision_handle_mouse_ev_switch_back_wind(pointer_window);
                    //Ya no decimos que se pulsa tecla background, pues en retorno dira que es tecla 3 de salir,
                    //y ha habremos cambiado la ventana a la que nos interesa
                    mouse_pressed_background_window=0;
                    //sleep(1);
                    return;
                }
            }

            //printf("pointer window es null. no deberia suceder\n");
            return;
        }


        else {
            //Pulsado shift left/right pero o bien no se permite background windows, o bien ventana no lo permite, o bien ventana es NULL
            //simular escape
            debug_printf(VERBOSE_DEBUG,"Pressed shift left/right but background is not allowed or window can not be bacgrounded, or window NULL. Simulate close window");
            //mouse_pressed_background_window=1;

            return;
        }
    }


    if (!si_menu_mouse_activado()) return; // 0;

    //printf ("zxvision_handle_mouse_events: mouse_left: %d\n",mouse_left);
    //int pulsado_boton_cerrar=0;

    menu_calculate_mouse_xy();

    if (mouse_left && !mouse_is_dragging) {

        //printf("if_menu_topbarmenu_pressed_bar() =%d\n",if_menu_topbarmenu_pressed_bar());

        if (if_menu_topbarmenu_enabled_and_pressed_bar() ) {
            //printf("pulsado en top bar desde zxvision_handle_mouse_events\n");

            menu_pressed_open_menu_while_in_menu.v=1;
            salir_todos_menus=1;

            /*
            Estas decisiones son parecidas en casos:
            pulsar tecla menu cuando menu activo (menu_if_pressed_menu_button en menu_get_pressed_key_no_modifier), conmutar ventana, pulsar logo ZEsarUX en ext desktop
            */

            if (!menu_allow_background_windows) {
                    mouse_pressed_close_window=1;
            }

            else {
                //Si la ventana activa permite ir a background, mandarla a background
                if (zxvision_current_window->can_be_backgrounded) {
                        mouse_pressed_background_window=1;
                }

                //Si la ventana activa no permite ir a background, cerrarla
                else {
                        mouse_pressed_close_window=1;
                }
            }
        }

        //Si se pulsa dentro de ventana y no esta arrastrando
         else if (si_menu_mouse_en_ventana() && !zxvision_keys_event_not_send_to_machine) {
            debug_printf (VERBOSE_DEBUG,"Clicked inside window. Events are not sent to emulated machine");
            zxvision_keys_event_not_send_to_machine=1;
            zxvision_set_ventana_tipo_activa();

            //Reflejar cambio de color en recuadro alrededor de maquina emulada
            menu_refresca_pantalla();

            zxvision_draw_window(w);
            zxvision_draw_window_contents(w);
        }



        else if (!si_menu_mouse_en_ventana() && zxvision_keys_event_not_send_to_machine) {
            //Si se pulsa fuera de ventana
            debug_printf (VERBOSE_DEBUG,"Clicked outside window. Events are sent to emulated machine. X=%d Y=%d",menu_mouse_x,menu_mouse_y);
            //printf ("Clicked outside window. Events are sent to emulated machine. X=%d Y=%d\n",menu_mouse_x,menu_mouse_y);
            zxvision_keys_event_not_send_to_machine=0;
            zxvision_reset_ventana_tipo_activa();

            //Reflejar cambio de color en recuadro alrededor de maquina emulada
            menu_refresca_pantalla();

            zxvision_draw_window(w);
            zxvision_draw_window_contents(w);

            int absolute_mouse_x,absolute_mouse_y;

            menu_calculate_mouse_xy_absolute_interface(&absolute_mouse_x,&absolute_mouse_y);

            //Vamos a ver en que ventana se ha pulsado, si tenemos background activado
            zxvision_window *ventana_pulsada;

            ventana_pulsada=zxvision_coords_in_below_windows(zxvision_current_window,absolute_mouse_x,absolute_mouse_y);


            if (ventana_pulsada!=NULL && zxvision_show_minimized(ventana_pulsada)) {
                debug_printf (VERBOSE_DEBUG,"Clicked on window: %s",ventana_pulsada->window_title);

                zxvision_handle_mouse_ev_switch_back_wind(ventana_pulsada);

                //printf("despues de conmutar ventana\n");

            }

            //Ver si no se ha pulsado shift, con lo que cerraremos las ventanas, siempre que no se haya pulsado en botones o iconos
            //esa comprobación de pulsado en botones se hace mediante !zxvision_if_mouse_in_zlogo_or_buttons_desktop_if_trigger(0)
            //con parámetro 0 dado que solo queremos ver que el ratón no esté por esas zonas, sin lanzar la acción de activado de esos
            //botones o iconos
            //El comportamiento es muy parecido al otro else if de abajo , donde se
            //entra cuando se pulsan botones del desktop: cerrar menus (y en el caso de pulsar botones, avisar pulsar botones)
            //Se podria haber incluido esta condicion de no shift en el else if de abajo de pulsar botones, pero creo que queda
            //el codigo mas claro asi

            else if ((puerto_65278&1)==1 && !zxvision_if_mouse_in_zlogo_or_buttons_desktop_if_trigger(0)) {
                //printf("zxvision_if_mouse_in_zlogo_or_buttons_desktop 1\n");
                //printf("No pulsado shift. Cerramos ventanas\n");
                salir_todos_menus=1;

                if (!menu_allow_background_windows) {
                        mouse_pressed_close_window=1;
                }

                else {
                    //Si la ventana activa permite ir a background, mandarla a background
                    if (zxvision_current_window->can_be_backgrounded) {
                            mouse_pressed_background_window=1;
                    }

                    //Si la ventana activa no permite ir a background, cerrarla
                    else {
                            mouse_pressed_close_window=1;
                    }
                }
            }



            //Ver si hemos pulsado por la zona del logo en el ext desktop
            else if (zxvision_if_mouse_in_zlogo_or_buttons_desktop()) {
                //printf("zxvision_if_mouse_in_zlogo_or_buttons_desktop 2\n");
                //printf("pulsado en un boton desde handle mouse events. menu_abierto %d\n",menu_abierto);
                menu_draw_ext_desktop_dibujar_boton_or_lower_icon_pulsado();

                menu_pressed_open_menu_while_in_menu.v=1;
                salir_todos_menus=1;

                /*
                Estas decisiones son parecidas en casos:
                pulsar tecla menu cuando menu activo (menu_if_pressed_menu_button en menu_get_pressed_key_no_modifier), conmutar ventana, pulsar logo ZEsarUX en ext desktop
                */

                if (!menu_allow_background_windows) {
                        mouse_pressed_close_window=1;
                }

                else {
                    //Si la ventana activa permite ir a background, mandarla a background
                    if (zxvision_current_window->can_be_backgrounded) {
                            mouse_pressed_background_window=1;
                    }

                    //Si la ventana activa no permite ir a background, cerrarla
                    else {
                            mouse_pressed_close_window=1;
                    }
                }


            }
        }
    }


    if (mouse_movido) {
        //printf ("mouse movido\n");

        //Asumimos raton no apunta a zona de redimensionado
        ventana_marca_redimensionado_raton_encima=0;

        if (si_menu_mouse_en_ventana() ) {
                //if (menu_mouse_x>=0 && menu_mouse_y>=0 && menu_mouse_x<ventana_ancho && menu_mouse_y<ventana_alto ) {
                    //printf ("dentro ventana\n");
                    if (menu_mouse_y==0) {
                        //printf ("En barra titulo\n");
                    }
                    //Descartar linea titulo y ultima linea

                if (zxvision_mouse_in_bottom_right(w) && w->can_be_resized) {
                    //printf("Raton en zona dimensionado\n");
                    //Como continuamente se redibuja el marco de la ventana, se actualizara automaticamente
                    //la zona de redimensionado indicando que el raton esta encima
                    ventana_marca_redimensionado_raton_encima=1;
                }

        }


    }

    if (mouse_left) {
        //printf ("Pulsado boton izquierdo\n");

        if (!mouse_movido) {
            if (!mouse_is_clicking) {
                //printf ("Mouse started clicking\n");
                mouse_is_clicking=1;
                last_x_mouse_clicked=menu_mouse_x;
                last_y_mouse_clicked=menu_mouse_y;


                //Gestion doble click
                if (menu_multitarea) {
                    if (menu_mouse_left_double_click_counter-menu_mouse_left_double_click_counter_initial<25) {
                        //printf ("-IT is DOBLE click\n");
                        mouse_is_double_clicking=1;
                    }
                    else {
                        menu_mouse_left_double_click_counter_initial=menu_mouse_left_double_click_counter;
                        mouse_is_double_clicking=0;
                    }
                }

                else {
                    //Sin multitarea nunca hay doble click
                    mouse_is_double_clicking=0;
                }
            }
        }
    }

    //Si empieza a pulsar botón izquierdo
    if (mouse_left && mouse_is_clicking) {

        if (si_menu_mouse_en_ventana()) {
            //Pulsado en barra titulo
            if (last_y_mouse_clicked==0) {
                if (!mouse_is_double_clicking) {
                        //Si pulsa boton cerrar ventana
                    if (last_x_mouse_clicked==0 && menu_hide_close_button.v==0) {
                        //printf ("pulsado boton cerrar\n");
                        //pulsado_boton_cerrar=1;
                        mouse_pressed_close_window=1;
                        //Mostrar boton cerrar pulsado
                        putchar_menu_overlay(w->x,w->y,menu_retorna_caracter_cerrar(),ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_TINTA_TITULO);
                        //pressed_close_window_on_window_manager=1;
                    }

                    //Si pulsa zona background  window
                    if (last_x_mouse_clicked==zxvision_return_background_button_position(w->visible_width) && w->can_be_backgrounded && menu_allow_background_windows) {
                        mouse_pressed_background_window=1;
                        //Mostrar boton background pulsado
                        putchar_menu_overlay(w->x+zxvision_return_background_button_position(w->visible_width),w->y,zxvision_get_character_backgrounded_window(),ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_TINTA_TITULO);
                    }


                    //Si se pulsa en boton minimizar, indicar que se esta pulsando
                    if (last_x_mouse_clicked==zxvision_return_minimize_button_position(w->visible_width) && menu_hide_minimize_button.v==0 && w->can_be_minimized) {
                        putchar_menu_overlay(w->x+zxvision_return_minimize_button_position(w->visible_width),w->y,menu_retorna_caracter_minimizar(w),ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_TINTA_TITULO);
                    }

                    //Si se pulsa en boton maximizar, indicar que se esta pulsando
                    if (last_x_mouse_clicked==zxvision_return_maximize_button_position(w->visible_width) && menu_hide_maximize_button.v==0 && w->can_be_resized) {
                        putchar_menu_overlay(w->x+zxvision_return_maximize_button_position(w->visible_width),w->y,menu_retorna_caracter_maximizar(w),ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_TINTA_TITULO);
                    }

                }
            }


            //Si se pulsa en ventana y alrededor tecla hotkey
            /*
            if (w->can_mouse_send_hotkeys && last_y_mouse_clicked>0) {


                z80_byte caracter=zxvision_get_key_hotkey(w,last_x_mouse_clicked,last_y_mouse_clicked-1);

                if (caracter>=32 && caracter<=126) {

                        mouse_pressed_hotkey_window=1;
                        mouse_pressed_hotkey_window_key=caracter;

                }

                else {
                    mouse_pressed_hotkey_window=0;
                }
            }
            */


            //Pulsado en botones Scroll horizontal
            if (zxvision_if_horizontal_scroll_bar(w)) {
                if (last_y_mouse_clicked==w->visible_height-1) {
                    //Linea scroll horizontal
                    int posicion_flecha_izquierda=1;
                    int posicion_flecha_derecha=w->visible_width-2;

                    //Flecha izquierda
                    if (last_x_mouse_clicked==posicion_flecha_izquierda) {
                        //printf ("Pulsado en scroll izquierda\n");
                        //putchar_menu_overlay(w->x+posicion_flecha_izquierda,w->y+w->visible_height-1,'<',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
                        zxvision_draw_horizontal_scroll_bar(w,1);

                    }
                    //Flecha derecha
                    if (last_x_mouse_clicked==posicion_flecha_derecha) {
                        //printf ("Pulsado en scroll derecha\n");
                        //putchar_menu_overlay(w->x+posicion_flecha_derecha,w->y+w->visible_height-1,'>',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
                        zxvision_draw_horizontal_scroll_bar(w,2);

                    }

                    //Zona porcentaje
                    if (last_x_mouse_clicked>posicion_flecha_izquierda && last_x_mouse_clicked<posicion_flecha_derecha) {
                        //printf ("Pulsado en zona scroll horizontal\n");
                        zxvision_draw_horizontal_scroll_bar(w,3);
                    }

                }
            }

            //Pulsado en botones Scroll vertical
            if (zxvision_if_vertical_scroll_bar(w)) {
                if (last_x_mouse_clicked==w->visible_width-1) {
                    //Linea scroll vertical
                    int posicion_flecha_arriba=1;
                    int posicion_flecha_abajo=w->visible_height-2;

                    //Flecha arriba
                    if (last_y_mouse_clicked==posicion_flecha_arriba) {
                        //printf ("Pulsado en scroll arriba\n");
                        //putchar_menu_overlay(w->x+w->visible_width-1,w->y+posicion_flecha_arriba,'^',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
                        zxvision_draw_vertical_scroll_bar(w,1);
                    }

                    //Flecha abajo
                    if (last_y_mouse_clicked==posicion_flecha_abajo) {
                        //printf ("Pulsado en scroll abajo\n");
                        //putchar_menu_overlay(w->x+w->visible_width-1,w->y+posicion_flecha_abajo,'v',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
                        zxvision_draw_vertical_scroll_bar(w,2);
                    }

                    if (last_y_mouse_clicked>posicion_flecha_arriba && last_y_mouse_clicked<posicion_flecha_abajo) {
                        //printf ("Pulsado en zona scroll vertical\n");
                        zxvision_draw_vertical_scroll_bar(w,3);
                    }

                }
            }

        }
    }

    //Liberación boton izquierdo
    if (!mouse_left && mouse_is_clicking && !mouse_is_dragging) {
            //printf ("Mouse stopped clicking mouse_is_dragging %d\n",mouse_is_dragging);
            mouse_is_clicking=0;
            //Pulsacion en sitios de ventana
            //Si en barra titulo
            if (si_menu_mouse_en_ventana() && last_y_mouse_clicked==0) {
                //printf ("Clicked on title\n");
                //Y si ha sido doble click
                if (mouse_is_double_clicking) {
                    debug_printf (VERBOSE_DEBUG,"Double clicked on title");

                    zxvision_handle_maximize(w);


                }
                else {
                    //Simple click
                    //Si pulsa zona minimizar
                    if (last_x_mouse_clicked==zxvision_return_minimize_button_position(w->visible_width) && menu_hide_minimize_button.v==0) {
                        //Mostrar boton minimizar pulsado
                        //printf ("minimizar\n");
                        zxvision_handle_click_minimize(w);
                    }

                    //Si pulsa zona maximizar y no se pulsa en minimizar
                    else if (last_x_mouse_clicked==zxvision_return_maximize_button_position(w->visible_width) && menu_hide_maximize_button.v==0) {
                        //Mostrar boton maximizar pulsado
                        //printf ("maximizar\n");
                        zxvision_handle_maximize(w);
                    }
                    //Si pulsa boton cerrar ventana
                    /*if (last_x_mouse_clicked==0 && menu_hide_close_button.v==0) {
                        printf ("pulsado boton cerrar\n");
                        //pulsado_boton_cerrar=1;
                        mouse_pressed_close_window=1;
                    }*/



                }


            }


            //Si se pulsa en ventana y alrededor tecla hotkey

            if (w->can_mouse_send_hotkeys && si_menu_mouse_en_ventana() && last_y_mouse_clicked>0 && last_y_mouse_clicked<w->visible_height-1) {

                //printf ("visible height: %d\n",w->visible_height);
                debug_printf (VERBOSE_DEBUG,"Looking for hotkeys at mouse position");


                z80_byte caracter=zxvision_get_key_hotkey(w,last_x_mouse_clicked,last_y_mouse_clicked-1);

                if (caracter>=32 && caracter<=126) {
                        debug_printf (VERBOSE_DEBUG,"Sending hotkey from mouse: %c",caracter);
                        mouse_pressed_hotkey_window=1;
                        mouse_pressed_hotkey_window_key=caracter;

                        return;

                }

                else {
                    mouse_pressed_hotkey_window=0;
                }
            }




        //Si se pulsa dentro de cualquier otra ventana o en logo Z. Esto solo cuando se libera boton
        //Y si no tenemos el foco
        if (!zxvision_keys_event_not_send_to_machine) {

            int absolute_mouse_x,absolute_mouse_y;

            menu_calculate_mouse_xy_absolute_interface(&absolute_mouse_x,&absolute_mouse_y);

            //Vamos a ver en que ventana se ha pulsado, si tenemos background activado
            zxvision_window *ventana_pulsada;

            ventana_pulsada=zxvision_coords_in_below_windows(zxvision_current_window,absolute_mouse_x,absolute_mouse_y);

            if (ventana_pulsada!=NULL || zxvision_if_mouse_in_zlogo_or_buttons_desktop()  /*&& !zxvision_keys_event_not_send_to_machine*/) {
                //printf("zxvision_if_mouse_in_zlogo_or_buttons_desktop 3\n");
                debug_printf (VERBOSE_DEBUG,"Clicked inside other window or zlogo. Events are not sent to emulated machine");
                zxvision_keys_event_not_send_to_machine=1;
                zxvision_set_ventana_tipo_activa();
                zxvision_draw_window(w);
                zxvision_draw_window_contents(w);


            }
        }


            //Scroll horizontal
            if (zxvision_if_horizontal_scroll_bar(w)) {
                if (last_y_mouse_clicked==w->visible_height-1) {
                    //Linea scroll horizontal
                    int posicion_flecha_izquierda=1;
                    int posicion_flecha_derecha=w->visible_width-2;

                    //Flecha izquierda
                    if (last_x_mouse_clicked==posicion_flecha_izquierda) {
                        zxvision_send_scroll_left_and_draw(w);
                    }

                    //Flecha derecha
                    if (last_x_mouse_clicked==posicion_flecha_derecha) {
                        zxvision_send_scroll_right_and_draw(w);
                    }

                    if (last_x_mouse_clicked>posicion_flecha_izquierda && last_x_mouse_clicked<posicion_flecha_derecha) {
                        //printf ("Pulsado en zona scroll horizontal\n");
                        //Sacamos porcentaje
                        int total_ancho=posicion_flecha_derecha-posicion_flecha_izquierda;
                        if (total_ancho==0) total_ancho=1; //Evitar dividir por cero

                        int parcial_ancho=last_x_mouse_clicked-posicion_flecha_izquierda;

                        int porcentaje=(parcial_ancho*100)/total_ancho;

                        //printf ("Porcentaje: %d\n",porcentaje);

                        int offset_to_mult=w->total_width-w->visible_width+1; //+1 porque se pierde linea derecha por scroll
                        //printf ("Multiplicando sobre %d\n",offset_to_mult);

                        //Establecemos offset horizontal
                        int offset=((offset_to_mult)*porcentaje)/100;

                        //printf ("set offset: %d\n",offset);

                        //Casos especiales de izquierda del todo y derecha del todo
                        if (last_x_mouse_clicked==posicion_flecha_izquierda+1) {
                            //printf ("Special case: clicked on the top left. Set offset 0\n");
                            offset=0;
                        }

                        if (last_x_mouse_clicked==posicion_flecha_derecha-1) {
                            //printf ("Special case: clicked on the top right. Set offset to maximum\n");
                            offset=w->total_width-w->visible_width+1;
                        }

                        zxvision_set_offset_x(w,offset);


                        //Redibujar botones scroll. Esto es necesario solo en el caso que,
                                                //al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
                                                //se quedaria el color del boton invertido
                                                zxvision_draw_horizontal_scroll_bar(w,0);

                    }
                }
            }


            //Scroll vertical
            if (zxvision_if_vertical_scroll_bar(w)) {
                if (last_x_mouse_clicked==w->visible_width-1) {
                    //Linea scroll vertical
                    int posicion_flecha_arriba=1;
                    int posicion_flecha_abajo=w->visible_height-2;

                    //Flecha arriba
                    if (last_y_mouse_clicked==posicion_flecha_arriba) {
                        zxvision_send_scroll_up_and_draw(w);
                    }

                    //Flecha abajo
                    if (last_y_mouse_clicked==posicion_flecha_abajo) {
                        zxvision_send_scroll_down_and_draw(w);
                    }

                    if (last_y_mouse_clicked>posicion_flecha_arriba && last_y_mouse_clicked<posicion_flecha_abajo) {
                        //printf ("Pulsado en zona scroll vertical\n");
                        //Sacamos porcentaje
                        int total_alto=posicion_flecha_abajo-posicion_flecha_arriba;
                        if (total_alto==0) total_alto=1; //Evitar dividir por cero

                        int parcial_alto=last_y_mouse_clicked-posicion_flecha_arriba;

                        int porcentaje=(parcial_alto*100)/total_alto;

                        //printf ("Porcentaje: %d\n",porcentaje);


                        int offset_to_mult=w->total_height-w->visible_height+2; //+2 porque se pierde linea abajo de scroll y titulo
                        //printf ("Multiplicando sobre %d\n",offset_to_mult);

                        //Establecemos offset vertical
                        int offset=((offset_to_mult)*porcentaje)/100;

                        //printf ("set offset: %d\n",offset);

                        //Casos especiales de arriba del todo y abajo del todo
                        if (last_y_mouse_clicked==posicion_flecha_arriba+1) {
                            //printf ("Special case: clicked on the top. Set offset 0\n");
                            offset=0;
                        }

                        if (last_y_mouse_clicked==posicion_flecha_abajo-1) {
                            //printf ("Special case: clicked on the bottom. Set offset to maximum\n");
                            offset=w->total_height-w->visible_height+2;
                        }

                        zxvision_set_offset_y(w,offset);

                        //Redibujar botones scroll. Esto es necesario solo en el caso que,
                                                //al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
                                                //se quedaria el color del boton invertido
                                                zxvision_draw_vertical_scroll_bar(w,0);

                    }
                }
            }


    }

    if (mouse_wheel_vertical && zxvision_if_vertical_scroll_bar(w)) {
        int leido_mouse_wheel_vertical=mouse_wheel_vertical;
        //printf ("Read mouse vertical wheel from zxvision_handle_mouse_events : %d\n",leido_mouse_wheel_vertical);

        //Si invertir movimiento
        if (menu_invert_mouse_scroll.v) leido_mouse_wheel_vertical=-leido_mouse_wheel_vertical;

        while (leido_mouse_wheel_vertical<0) {
            zxvision_send_scroll_down_and_draw(w);
            leido_mouse_wheel_vertical++;
        }

        while (leido_mouse_wheel_vertical>0) {
            zxvision_send_scroll_up_and_draw(w);
            leido_mouse_wheel_vertical--;
        }

        //Y resetear a 0. importante
        mouse_wheel_vertical=0;
    }

    if (mouse_wheel_horizontal && zxvision_if_horizontal_scroll_bar(w)) {
        int leido_mouse_wheel_horizontal=mouse_wheel_horizontal;
        //printf ("Read mouse horizontal wheel from zxvision_handle_mouse_events : %d\n",leido_mouse_wheel_horizontal);


        //Si invertir movimiento
        if (menu_invert_mouse_scroll.v) leido_mouse_wheel_horizontal=-leido_mouse_wheel_horizontal;


        while (leido_mouse_wheel_horizontal<0) {
            zxvision_send_scroll_right_and_draw(w);
            leido_mouse_wheel_horizontal++;
        }

        while (leido_mouse_wheel_horizontal>0) {
            zxvision_send_scroll_left_and_draw(w);
            leido_mouse_wheel_horizontal--;
        }

        //Y resetear a 0. importante
        mouse_wheel_horizontal=0;
    }

    if (!mouse_is_dragging) {
        if (mouse_left && mouse_movido) {
            debug_printf (VERBOSE_DEBUG,"Mouse has begun to drag");

            if (auto_frameskip_even_when_movin_windows.v==0) {
                autoframeskip_setting_before_moving_windows.v=autoframeskip.v;
                debug_printf(VERBOSE_DEBUG,"Disabling autoframeskip while moving or resizing window");
                autoframeskip.v=0;
            }

            mouse_is_dragging=1;

            //Si estaba en titulo
            if (si_menu_mouse_en_ventana()) {
                if (menu_mouse_y==0) {
                    //printf ("Arrastrando ventana\n");
                    window_is_being_moved=1;
                    window_mouse_x_before_move=menu_mouse_x;
                    window_mouse_y_before_move=menu_mouse_y;
                }

                //Si esta en esquina inferior derecha (donde se puede redimensionar) y se permite resize
                if (zxvision_mouse_in_bottom_right(w) && w->can_be_resized) {
                    //printf ("Mouse dragging in bottom right\n");

                    window_is_being_resized=1;
                    window_mouse_x_before_move=menu_mouse_x;
                    window_mouse_y_before_move=menu_mouse_y;

                    //printf("Ventana deja de estar minimizada\n");
                    w->is_minimized=0;
                    w->is_maximized=0;
                }
            }

            //No esta en una ventana. Esta en un icono?
            else {
                //Si estaba en un icono
                if (zxdesktop_configurable_icons_enabled_and_visible()) {
                    zxvision_handle_mouse_events_start_drag_icon();
                }
            }



        }
    }

    if (mouse_is_dragging) {
        //printf ("mouse is dragging\n");
        if (!mouse_left) {
            debug_printf (VERBOSE_DEBUG,"Mouse has stopped to drag");

            if (auto_frameskip_even_when_movin_windows.v==0) {
                autoframeskip.v=autoframeskip_setting_before_moving_windows.v;
            }

            mouse_is_dragging=0;
            mouse_is_clicking=0; //Cuando se deja de arrastrar decir que se deja de pulsar tambien
            if (window_is_being_moved) {

                //printf ("Handle moved window\n");
                zxvision_handle_mouse_move_aux(w);
                window_is_being_moved=0;

            }

            if (window_is_being_resized) {

                //printf ("Handle resized window\n");
                zxvision_handle_mouse_resize_aux(w);

                window_is_being_resized=0;
            }

            zxvision_handle_mouse_events_stop_drag_icon();
        }

        else {
            if (window_is_being_moved) {
                //Si se ha movido un poco
                if (menu_mouse_y!=window_mouse_y_before_move || menu_mouse_x!=window_mouse_x_before_move) {
                    //printf ("Handle moved window\n");
                    zxvision_handle_mouse_move_aux(w);

                    //Hay que recalcular menu_mouse_x y menu_mouse_y dado que son relativos a la ventana que justo se ha movido
                    //menu_mouse_y siempre sera 0 dado que el titulo de la ventana, donde se puede arrastrar para mover, es posicion relativa 0
                    menu_calculate_mouse_xy();

                    window_mouse_y_before_move=menu_mouse_y;
                    window_mouse_x_before_move=menu_mouse_x;

                }
            }

            if (window_is_being_resized) {
                //Si se ha redimensionado un poco
                if (menu_mouse_y!=window_mouse_y_before_move || menu_mouse_x!=window_mouse_x_before_move) {
                    //printf ("Handle resized window\n");
                    zxvision_handle_mouse_resize_aux(w);

                    window_mouse_y_before_move=menu_mouse_y;
                    window_mouse_x_before_move=menu_mouse_x;
                }
            }

            //AQUI MOVER ICONO
            zxvision_handle_mouse_events_drag_icon();

        }
    }

    if (zxvision_pressed_right_mouse_button()) {
        //acciones con boton derecho, con menu abierto
        //Este mensaje es cpu intensivo. Desactivado por defecto
        //DBG_PRINT_ZXVISION_EVENTS VERBOSE_INFO,"ZXVISION_EVENTS: Pressed right button on zxvision_handle_mouse_events");

        //printf("Pulsado boton derecho desde zxvision_handle_mouse_events\n");
        //Si se pulsa boton derecho en alguna ventana
        int absolute_mouse_x,absolute_mouse_y;

        menu_calculate_mouse_xy_absolute_interface(&absolute_mouse_x,&absolute_mouse_y);

        //Vamos a ver en que ventana se ha pulsado, si tenemos background activado
        zxvision_window *ventana_pulsada;

        ventana_pulsada=zxvision_coords_in_below_windows(zxvision_current_window,absolute_mouse_x,absolute_mouse_y);
        if (ventana_pulsada!=NULL || si_menu_mouse_en_ventana()) {
            //Se pulsa en ventana de abajo o bien en la ventana actual
            DBG_PRINT_ZXVISION_EVENTS VERBOSE_INFO,"ZXVISION_EVENTS: Pressed right click on a window");

        }
        else {
            //No se pulsa ni en icono ni en ventanas. Quedaria ver si en botones de menu superior o en botones de dispositivo inferior.
            //printf("mouse pixel x,y desde handle mouse events: %d,%d\n",mouse_pixel_x,mouse_pixel_y);

            //Este mensaje es cpu intensivo. Desactivado por defecto
            //DBG_PRINT_ZXVISION_EVENTS VERBOSE_INFO,"ZXVISION_EVENTS: Not pressed right click on a window");


            int mouse_pixel_x,mouse_pixel_y;
            menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

            //multiplicamos por zoom
            mouse_pixel_x *=zoom_x;
            mouse_pixel_y *=zoom_y;


            //Si esta en zona botones superiores de zx desktop. Y si estan habilitados
            //Si pulsado boton derecho sobre iconos superiores

            if (menu_zxdesktop_upper_buttons_enabled.v && zxvision_topbar_menu_enabled.v==0) {
                int ancho_boton,alto_boton,total_botones,xinicio_botones,xfinal_botones;
                menu_ext_desktop_buttons_get_geometry(&ancho_boton,&alto_boton,&total_botones,&xinicio_botones,&xfinal_botones);

                if (mouse_pixel_x>=xinicio_botones && mouse_pixel_x<xfinal_botones &&
                    mouse_pixel_y>=0 && mouse_pixel_y<alto_boton
                ) {
                    //printf ("Pulsado boton derecho en zona botones del ext desktop\n");

                    //en que boton?
                    int numero_boton=(mouse_pixel_x-xinicio_botones)/ancho_boton;
                    //printf("boton pulsado: %d\n",numero_boton);
                    menu_pressed_zxdesktop_button_which=numero_boton;


                    menu_pressed_zxdesktop_button_which_right_button=1;

                    zxvision_handle_mouse_events_aux_open_menu_while_in_menu();

                }
            }

            if (menu_zxdesktop_lower_buttons_enabled.v) {
                //Si esta en zona de iconos lower de zx desktop. Y si estan habilitados


                    int ancho_boton,alto_boton,xinicio_botones,xfinal_botones,yinicio_botones;
                    menu_ext_desktop_lower_icons_get_geometry(&ancho_boton,&alto_boton,NULL,&xinicio_botones,&xfinal_botones,&yinicio_botones);

                    if (mouse_pixel_x>=xinicio_botones && mouse_pixel_x<xfinal_botones &&
                        mouse_pixel_y>=yinicio_botones && mouse_pixel_y<yinicio_botones+alto_boton
                    ) {
                        //printf ("Pulsado en zona lower icons del ext desktop\n");

                        //en que boton?
                        int numero_boton=(mouse_pixel_x-xinicio_botones)/ancho_boton;
                        //printf("boton pulsado: %d\n",numero_boton);

                        //Buscar indice array
                        int indice_array=zxdesktop_lowericon_find_index(numero_boton);

                        if (indice_array>=0) {

                                //printf ("boton esta visible\n");


                                menu_pressed_zxdesktop_lower_icon_which=numero_boton;

                                menu_pressed_zxdesktop_lower_icon_which_right_button=1;

                                zxvision_handle_mouse_events_aux_open_menu_while_in_menu();

                                //return 1;
                        }
                        else {
                            //printf ("boton NO esta visible\n");
                        }

                    }

            }


            //Asumimos pulsado en fondo desktop
            if (zxdesktop_configurable_icons_enabled_and_visible()) {

                //Si pulsado en algun icono

                int icono_pulsado=if_position_in_desktop_icons(mouse_pixel_x,mouse_pixel_y);

                //Si se pulsa alguno
                if (icono_pulsado>=0 && zxdesktop_configurable_icons_enabled_and_visible()) {
                    //printf("Icono pulsado desde zxvision_handle_mouse_events con boton derecho: %d\n",icono_pulsado);

                    menu_pressed_zxdesktop_configurable_icon_right_button=1;

                    menu_pressed_zxdesktop_configurable_icon_which=icono_pulsado;

                    //printf("Establecer menu_pressed_zxdesktop_configurable_icon_which desde linea 17688\n");

                    zxvision_set_next_menu_position_from_current_mouse();


            //printf("pulsado en un boton desde handle mouse events. menu_abierto %d\n",menu_abierto);
                        //menu_draw_ext_desktop_dibujar_boton_or_lower_icon_pulsado();

                    zxvision_handle_mouse_events_aux_open_menu_while_in_menu();

                }

                else {



                    if (zxvision_if_mouse_in_background() ) {
                        //printf("Pulsado en ZX desktop con boton derecho desde handle mouse events\n");

                        menu_pressed_zxdesktop_right_button_background=1;

                        //de momento no altero posicion menu si se pulsa en background
                        //zxvision_set_next_menu_position(absolute_mouse_x,absolute_mouse_y);

                        zxvision_handle_mouse_events_aux_open_menu_while_in_menu();


                    }

                }

            }

        }

    }

    //if (mouse_left && mouse_movido) printf ("Mouse is dragging\n");
    //return pulsado_boton_cerrar;
}

//Si se pulsa algun boton de raton (y raton habilitado)
int zxvision_clicked_mouse_button(void)
{
    if (!si_menu_mouse_activado()) return 0;
    if (mouse_left || mouse_right) return 1;
    return 0;
}

//Guardar tamanyo en variables por si cambia
void zxvision_window_save_size(zxvision_window *ventana,int *ventana_ancho_antes,int *ventana_alto_antes)
{

    //Guardar ancho y alto anterior para recrear la ventana si cambia
    *ventana_ancho_antes=ventana->visible_width;
    *ventana_alto_antes=ventana->visible_height;
}

//Retornar la posicion x en pixeles de esa ventana
int zxvision_window_get_pixel_x_position(zxvision_window *ventana)
{
    int this_win_x=(ventana->x)*menu_char_width*menu_gui_zoom*zoom_x;
    return this_win_x;
}

//Retornar la posicion y en pixeles de esa ventana
int zxvision_window_get_pixel_y_position(zxvision_window *ventana)
{
    int this_win_y=(ventana->y)*menu_char_height*menu_gui_zoom*zoom_y;
    return this_win_y;
}


//Funcion comun que usan algunas ventanas para movimiento de cursores y pgup/dn
void zxvision_handle_cursors_pgupdn(zxvision_window *ventana,z80_byte tecla)
{
    int contador_pgdnup;
    int tecla_valida=1;
                    switch (tecla) {

                        //abajo
                        case 10:
                        zxvision_send_scroll_down(ventana);

                        //Decir que se ha pulsado tecla para que no se relea
                        menu_speech_set_tecla_pulsada();
                        break;

                        //arriba
                        case 11:
                        zxvision_send_scroll_up(ventana);

                        //Decir que se ha pulsado tecla para que no se relea
                        menu_speech_set_tecla_pulsada();
                        break;

                        //izquierda
                        case 8:
                        zxvision_send_scroll_left(ventana);

                        //Decir que se ha pulsado tecla para que no se relea
                        menu_speech_set_tecla_pulsada();
                        break;

                        //derecha
                        case 9:
                        zxvision_send_scroll_right(ventana);

                        //Decir que se ha pulsado tecla para que no se relea
                        menu_speech_set_tecla_pulsada();
                        break;

                        //PgUp
                        case 24:
                            for (contador_pgdnup=0;contador_pgdnup<ventana->visible_height-2-ventana->upper_margin;contador_pgdnup++) {
                                zxvision_send_scroll_up(ventana);
                            }
                            //Decir que no se ha pulsado tecla para que se relea
                            menu_speech_reset_tecla_pulsada();
                        break;

                        //PgDn
                        case 25:
                            for (contador_pgdnup=0;contador_pgdnup<ventana->visible_height-2-ventana->upper_margin;contador_pgdnup++) {
                                zxvision_send_scroll_down(ventana);
                            }

                            //Decir que no se ha pulsado tecla para que se relea
                            menu_speech_reset_tecla_pulsada();
                        break;

                        //Mover ventana
                        case 'Q':
                            zxvision_set_y_position(ventana,ventana->y-1);
                        break;

                        case 'A':
                            zxvision_set_y_position(ventana,ventana->y+1);
                        break;

                        case 'O':
                            zxvision_set_x_position(ventana,ventana->x-1);
                        break;

                        case 'P':
                            zxvision_set_x_position(ventana,ventana->x+1);
                        break;

                        //Redimensionar ventana
                        //Mover ventana
                        case 'W':
                            if (ventana->visible_height-1>1 && ventana->can_be_resized) {
                                zxvision_set_visible_height(ventana,ventana->visible_height-1);
                                //printf("Ventana deja de estar minimizada\n");
                                ventana->is_minimized=0;
                                ventana->is_maximized=0;
                            }
                        break;

                        case 'S':
                            if (ventana->can_be_resized) {
                                zxvision_set_visible_height(ventana,ventana->visible_height+1);
                                //printf("Ventana deja de estar minimizada\n");
                                ventana->is_minimized=0;
                                ventana->is_maximized=0;
                            }
                        break;

                        case 'K':
                            if (ventana->visible_width-1>5 && ventana->can_be_resized) {
                                zxvision_set_visible_width(ventana,ventana->visible_width-1);
                                //printf("Ventana deja de estar minimizada\n");
                                ventana->is_minimized=0;
                                ventana->is_maximized=0;
                            }
                        break;

                        case 'L':
                            if (ventana->can_be_resized) {
                                zxvision_set_visible_width(ventana,ventana->visible_width+1);
                                //printf("Ventana deja de estar minimizada\n");
                                ventana->is_minimized=0;
                                ventana->is_maximized=0;
                            }
                        break;

                        default:
                            tecla_valida=0;
                        break;

                }

    if (tecla_valida) {
        //Refrescamos pantalla para reflejar esto, util con multitask off
        if (!menu_multitarea) {
            //printf ("refresca pantalla\n");
            menu_refresca_pantalla();
        }
    }

}

/*
Funcion comun usados en algunas ventanas que:
-refrescan pantalla
-ejecutan core loop si multitask activo
-leen tecla y esperan a liberar dicha tecla
*/
z80_byte zxvision_common_getkey_refresh(void)
{
    z80_byte tecla;

    if (!menu_multitarea) {
        //printf ("refresca pantalla\n");
        menu_refresca_pantalla();
    }

    //Por qué ejecutamos esto aqui??? Solo ejecutará una instrucción
    menu_cpu_core_loop();

    menu_espera_tecla();
    tecla=zxvision_read_keyboard();

    //con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
    //lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
    if (tecla==13 && mouse_left) {
        tecla=0;
    }

    if (tecla) {
        //printf ("Esperamos no tecla\n");
        menu_espera_no_tecla_con_repeticion();
    }

    //printf("zxvision_common_getkey_refresh. tecla=%d\n",tecla);

    return tecla;
}

/*
Funcion comun usados en algunas ventanas que:
-refrescan pantalla
-ejecutan core loop si multitask activo
-leen tecla y esperan a liberar dicha tecla
-sale también si hay pendiente mostrar un error
*/
z80_byte zxvision_common_getkey_refresh_o_pending_error_message(void)
{
    z80_byte tecla;

        if (!menu_multitarea) {
        //printf ("refresca pantalla\n");
        menu_refresca_pantalla();
    }

    //Por qué ejecutamos esto aqui??? Solo ejecutará una instrucción
    menu_cpu_core_loop();

    menu_espera_tecla_o_pending_error_message();
    tecla=zxvision_read_keyboard();

    //con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
    //lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
    if (tecla==13 && mouse_left) {
        tecla=0;
    }

    if (tecla) {
        //printf ("Esperamos no tecla\n");
        menu_espera_no_tecla_con_repeticion();
    }

    //printf("zxvision_common_getkey_refresh. tecla=%d\n",tecla);

    return tecla;
}

//Igual que zxvision_common_getkey_refresh pero sin esperar a no tecla
//todas_teclas dice si espera a cualquier tecla, no solo las habituales de spectrum sino tambien teclas extendidas de z88 etc
z80_byte zxvision_common_getkey_refresh_noesperanotec_common(int todas_teclas)
{
    z80_byte tecla;

         if (!menu_multitarea) {
            //printf ("refresca pantalla\n");
            menu_refresca_pantalla();
        }


                menu_cpu_core_loop();


                if (todas_teclas) menu_espera_tecla_cualquiera();
                else menu_espera_tecla();

                tecla=zxvision_read_keyboard();

                //con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
                //lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
                if (tecla==13 && mouse_left) {
                    tecla=0;
                }

    return tecla;
}

//Igual que zxvision_common_getkey_refresh pero sin esperar a no tecla
z80_byte zxvision_common_getkey_refresh_noesperanotec(void)
{
    return zxvision_common_getkey_refresh_noesperanotec_common(0);
}

z80_byte zxvision_common_getkey_refresh_noesperanotec_todasteclas(void)
{
    return zxvision_common_getkey_refresh_noesperanotec_common(1);
}

//Igual que zxvision_common_getkey_refresh_noesperanotec pero puede volver con wheel
z80_byte zxvision_common_getkey_wheel_refresh_noesperanotec(void)
{
    z80_byte tecla;

         if (!menu_multitarea) {
            //printf ("refresca pantalla\n");
            menu_refresca_pantalla();
        }


                menu_cpu_core_loop();


                menu_espera_tecla_o_wheel();
                tecla=zxvision_read_keyboard();

                //con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
                //lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
                if (tecla==13 && mouse_left) {
                    tecla=0;
                }

    return tecla;
}

z80_byte zxvision_common_getkey_refresh_noesperatecla(void)
//Igual que zxvision_common_getkey_refresh pero sin esperar tecla cuando multitarea activa
{

    z80_byte tecla;

                menu_cpu_core_loop();

                //si no hay multitarea, refrescar pantalla para mostrar contenido ventana rellenada antes, esperar tecla,
                if (menu_multitarea==0) {
                        menu_refresca_pantalla();
                        menu_espera_tecla();
                        //acumulado=0;
                }

                tecla=zxvision_read_keyboard();

                //Nota: No usamos zxvision_common_getkey_refresh porque necesitamos que el bucle se ejecute continuamente para poder
                //refrescar contenido de ventana, dado que aqui no llamamos a menu_espera_tecla
                //(a no ser que este multitarea off)

                if (tecla==13 && mouse_left) {
                    tecla=0;
                }


        if (tecla) {
            //printf ("Esperamos no tecla\n");
            menu_espera_no_tecla_con_repeticion();
        }

    return tecla;
}

//Retorna 1 si la tecla no se tiene que enviar a la maquina emulada
//esto es , cuando el menu esta abierto y la ventana tiene el foco
//En cambio retorna 0 (la tecla se va a enviar a la maquina emulada), cuando el menu esta cerrado o la ventana no tiene el foco
int zxvision_key_not_sent_emulated_mach(void)
{
    if (menu_abierto==1 && zxvision_keys_event_not_send_to_machine) return 1;
    else return 0;
}



//Crea ventana simple de 2 de alto con funcion para condicion de salida, y funcion de print.
void zxvision_simple_progress_window(char *titulo, int (*funcioncond) (zxvision_window *),void (*funcionprint) (zxvision_window *) )
{
        zxvision_window ventana;

        //2 de alto de texto y 40 de ancho, para que quepan dos lineas de manera holgada
        int alto_ventana=5;
        int ancho_ventana=40;


        int x_ventana=menu_center_x_from_width(ancho_ventana);
        int y_ventana=menu_center_y()-alto_ventana/2;

        zxvision_new_window(&ventana,x_ventana,y_ventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,titulo);

        zxvision_draw_window(&ventana);


        zxvision_draw_window_contents(&ventana);


        zxvision_espera_tecla_condicion_progreso(&ventana,funcioncond,funcionprint);




        zxvision_destroy_window(&ventana);
}



void zxvision_rearrange_background_windows(int si_cascada,int si_aplicar_a_inmutables)
{

    //printf("rearrange windows\n");
    //debug_exec_show_backtrace();

    //Por si acaso
    if (!menu_allow_background_windows) return;

    int origen_x=menu_origin_x();

    //printf ("origen_x: %d\n",origen_x);

    //Si abrir ventanas en zxdesktop o no, contar todo el ancho visible o bien solo el de zxdesktop

    int ancho;

    //Si ZX Desktop habilitado y si los menús se abren en el ZX Desktop
    if (menu_ext_desktop_enabled_place_menu() ) {

        ancho=menu_get_width_characters_ext_desktop();

        //+1 para que no queden pegadas a la pantalla emulada
        origen_x++;
        ancho--;
    }

    else ancho=scr_get_menu_width();

    //printf ("ancho: %d\n",ancho);

    int xfinal=origen_x+ancho;


    int alto=scr_get_menu_height();

    //printf ("alto: %d\n",alto);

    int yfinal=alto;


    //Empezamos una a una, desde la de mas abajo
    zxvision_window *ventana;

    ventana=zxvision_current_window;

    if (ventana==NULL) return;

    ventana=zxvision_find_first_window_below_this(ventana);

    if (ventana==NULL) return;

    int origen_y=0;

    //Si hay botones parte superior zxdesktop, origen_y lo incrementamos
    if (screen_ext_desktop_enabled && scr_driver_can_ext_desktop() && menu_zxdesktop_upper_buttons_enabled.v && zxvision_topbar_menu_enabled.v==0) {
        origen_y=EXT_DESKTOP_BUTTONS_TOTAL_SIZE/8;

        //Y quitamos ese alto disponible para no sobreescribir botones inferiores
        yfinal-=EXT_DESKTOP_BUTTONS_TOTAL_SIZE/8;

    }

    //printf("origen y: %d\n",origen_y);
    //printf("yfinal: %d\n",yfinal);

    //Y de ahi para arriba
    int x=origen_x;
    int y=origen_y;

    int alto_maximo_en_fila=0;

    int cambio_coords_origen=0;

    while (ventana!=NULL) {

        debug_printf (VERBOSE_DEBUG,"Setting window %s to %d,%d",ventana->window_title,x,y);
        //printf ("Setting window %s to %d,%d\n",ventana->window_title,x,y);
        //printf ("x %d width %d final %d\n",x,ventana->visible_width,xfinal);
        //Controlar limites despues de los movimientos anteriores
        if (x+ventana->visible_width>xfinal) {
            //Si se sale por la derecha, mover ventana a la izquierda para que no se salga
            //printf("Ajuste x\n");
            x=xfinal-ventana->visible_width;
            //printf("x despues ajuste: %d\n",x);
        }

        if (y+ventana->visible_height>yfinal) {
            //Si se sale por la derecha, mover ventana a la izquierda para que no se salga
            y=yfinal-ventana->visible_height;
        }

        int forzar_process_switcher=0;

        //Si es process switcher, ajustar abajo a la izquierda
        if (setting_process_switcher_force_left_bottom.v) {
            if (!strcasecmp(ventana->geometry_name,"processswitcher")) {
                forzar_process_switcher=1;
                //printf("Es process switcher\n");
                x=0;
                y=scr_get_menu_height()-ventana->visible_height;
            }
        }

        if (x<0) x=0;
        if (y<0) y=0;

        //printf ("Setting window %s to %d,%d\n",ventana->window_title,x,y);

        //TODO: en caso de ventanas de este tipo (como process switcher) se la considera igual aunque no se graban las modificaciones,
        //o sea por ejemplo que si se hace cascade, esta ventana no se reubicará aunque habrá un "hueco" en la cascada donde deberia ir ubicada
        if (ventana->not_altered_by_massive_changes==0 || si_aplicar_a_inmutables || forzar_process_switcher) {

        ventana->x=x;
        ventana->y=y;

        //Y guardar la geometria
        util_add_window_geometry_compact(ventana);

        }

        if (ventana->visible_height>alto_maximo_en_fila) alto_maximo_en_fila=ventana->visible_height;

        int ancho_antes=ventana->visible_width;

        ventana=ventana->next_window;
        if (ventana!=NULL) {

            //En cascada la ventana se va alternando en diagonal abajo
            if (si_cascada) {
                x++;
                y++;
            }

            //En no cascada, la ventana se pone al lado
            else {
                x +=ancho_antes;
            }

            //Si se sale por la derecha
            //printf ("x %d width %d final %d\n",x,ventana->visible_width,xfinal);
            if (x+ventana->visible_width>xfinal) {

                //printf ("Next column\n");
                //Siguiente fila

                if (si_cascada) {
                    /*
                    //Si se sale por la derecha, mover ventana a la izquierda para que no se salga
                    x=xfinal-ventana->visible_width-1;//origen_x;
                    if (x<0) x=0;
                    */

                }

                else {
                    x=origen_x;

                    y+=alto_maximo_en_fila;
                }

                alto_maximo_en_fila=0;


            }

            //Si ha llegado abajo del todo, volver al principio
            //printf("y %d height %d final %d\n",y,ventana->visible_height,yfinal);
            if (y+ventana->visible_height>yfinal) {

                debug_printf (VERBOSE_DEBUG,"Restart x,y coordinates");
                //printf ("Restart x,y coordinates. ventana %s\n",ventana->window_title);


                x=origen_x;
                y=origen_y;

                if (si_cascada) {

                }

                else {

                    //alternamos coordenadas origen, para darles cierto "movimiento", 4 caracteres derecha y abajo
                    cambio_coords_origen ^=4;

                    x += cambio_coords_origen;
                    y += cambio_coords_origen;

                }

                //printf("x,y after restart: %d,%d\n",x,y);

            }


        }
    }

    cls_menu_overlay();
}





//Retorna el item i
menu_item *old_menu_retorna_item(menu_item *m,int i)
{

    menu_item *item_next;

        while (i>0)
        {
            //printf ("m: %p i: %d\n",m,i);
            item_next=m->siguiente_item;
            if (item_next==NULL) return m;  //Controlar si final

                m=item_next;
        i--;
        }

    return m;


}


//Retorna el item avanzado y/o item sencillo, en caso de estar en interfaz sencilla
int menu_item_retornar_avanzados(menu_item *m)
{
    if (menu_show_simple_items.v) {
        if (m->item_sencillo) return 1;
        else return 0;
    }

    if (m->item_avanzado && menu_show_advanced_items.v==0) return 0;

    else return 1;

}

//Retorna el item buscado, excluyendo los avanzados
menu_item *menu_retorna_item(menu_item *m,int buscado)
{

    int contador=0;

    menu_item *item_next;

    while (1) {
        //Si llegado al deseado
        if (contador==buscado && menu_item_retornar_avanzados(m)) return m;

        //Siguiente item. incrementar contador si es que no se excluye item por ser avanzado
        if (menu_item_retornar_avanzados(m)) contador++;

        item_next=m->siguiente_item;

        //Si llegado al final
        if (item_next==NULL) return m;  //Controlar si final

        //Siguiente item
        m=item_next;

    }

}

//Retorna el texto del item de menu, segun el idioma
char *menu_retorna_item_language(menu_item *m)
{
    //printf("Item spanish (%s) english (%s)\n",m->texto_opcion_spanish,m->texto_opcion);
    //Dado que hay que concatenar el prefijo, guardamos el string final en un campo del mismo item de menu

    char *texto_opcion;

    if (gui_language==GUI_LANGUAGE_SPANISH && m->texto_opcion_spanish[0]!=0) {
        //printf("Retornando spanish %s (%s)\n",m->texto_opcion_spanish,m->texto_opcion);
        texto_opcion=m->texto_opcion_spanish;
    }
    else if (gui_language==GUI_LANGUAGE_CATALAN) {

        if (m->texto_opcion_catalan[0]!=0) {
            texto_opcion=m->texto_opcion_catalan;
        }
        //Si no hay en catalan, al menos retornar en español, si es que existe, antes que en ingles.
        //ademas quiza algunos items seran iguales en ambos idiomas y nos ahorramos tener que escribirlos dos veces
        else if (m->texto_opcion_spanish[0]!=0) {
            texto_opcion=m->texto_opcion_spanish;
        }

        else texto_opcion=m->texto_opcion;
    }

    else {
        texto_opcion=m->texto_opcion;
    }

    //concatenar
    sprintf(m->texto_opcion_concatenado,"%s%s%s",m->texto_opcion_prefijo,texto_opcion,m->texto_opcion_sufijo);

    return m->texto_opcion_concatenado;
}


//Retorna el item i segun posicion x,y del mouse
menu_item *menu_retorna_item_tabulado_xy(menu_item *m,int x,int y,int *linea_buscada)
{

    menu_item *item_next;
    int indice=0;
    int encontrado=0;

    //printf ("buscar item x: %d y: %d\n",x,y);

        while (!encontrado)
        {

            //Ver si coincide y. x tiene que estar en el rango del texto
            int longitud_texto=menu_calcular_ancho_string_item(menu_retorna_item_language(m));
            if (y==m->menu_tabulado_y &&
                x>=m->menu_tabulado_x && x<m->menu_tabulado_x+longitud_texto &&
                menu_item_retornar_avanzados(m)
                )
            {
                encontrado=1;
            }

            else {

                if (menu_item_retornar_avanzados(m)) indice++;

                //printf ("m: %p i: %d\n",m,i);
                item_next=m->siguiente_item;
                if (item_next==NULL) return NULL;  //Controlar si final

                m=item_next;
                //i--;

            }
        }

        if (encontrado) {
            *linea_buscada=indice;
        return m;
    }

    else return NULL;


}

void menu_cpu_core_loop(void)
{
    if (menu_multitarea==1) {
        //multitarea en menu y emulacion en menu
        if (!menu_emulation_paused_on_menu && !menu_emulation_paused_on_menu_by_debug_step_mode) {
            cpu_core_loop();
        }


        else {
            //multitarea en menu aunque no emula cpu
            //hay que leer timers, etc
            timer_check_interrupt();

            if (interrupcion_timer_generada.v==0) {
                //printf("NO Interrupt\n");
                //Esto suele hacer 1 ms de pausa
                timer_pause_waiting_end_frame();
            }

            //Fin de frame de pantalla (han pasado 20 ms)
            else {
                //printf("Interrupt\n");
                interrupcion_timer_generada.v=0;
                scr_actualiza_tablas_teclado();
                realjoystick_main();
                scr_refresca_pantalla();

                contador_parpadeo--;

                if (!contador_parpadeo) {
                    contador_parpadeo=16;
                    toggle_flash_state();
                }
            }
        }
    }

    //Sin multitarea: ni se emula cpu en menu, ni hay background de ventanas, ni timers, ni caracteres parpadeo etc
    else {
        scr_actualiza_tablas_teclado();
        realjoystick_main();

        //0.5 ms
        usleep(MENU_CPU_CORE_LOOP_SLEEP_NO_MULTITASK);


        //printf ("en menu_cpu_core_loop\n");
    }

}

//Dice si o bien multitask esta desactivado o emulacion de cpu esta desactivado en menu,
//para saber que la emulacion se pausa al abrir el menu, por cualquiera de las dos opciones
int menu_if_emulation_paused(void)
{
    if (!menu_multitarea) return 1;
    if (menu_emulation_paused_on_menu) return 1;
    if (menu_emulation_paused_on_menu_by_debug_step_mode) return 1;

    return 0;
}

int si_menu_mouse_en_ventana(void)
{
    if (menu_mouse_x>=0 && menu_mouse_y>=0 && menu_mouse_x<current_win_ancho && menu_mouse_y<current_win_alto ) return 1;
    return 0;
}

//Si en ventana pero no en zona de scrolls
int si_menu_mouse_en_ventana_no_en_scrolls(void)
{
    if (menu_mouse_x>=0 && menu_mouse_y>=0 && menu_mouse_x<current_win_ancho-1 && menu_mouse_y<current_win_alto-1 ) return 1;
    return 0;
}

int menu_allows_mouse(void)
{
    //Primero, fbdev no permite raton
    if (!strcmp(scr_new_driver_name,"fbdev")) return 0;

    //Luego, el resto de los drivers completos (xwindows, sdl, cocoa, ...)

    return si_complete_video_driver();
}


//Retorna las coordenadas absolutas del raton (en tamaño de pixel) teniendo en cuenta todo el tamaño de la interfaz del emulador
void menu_calculate_mouse_xy_absolute_interface_pixel(int *resultado_x,int *resultado_y)
{
        int x,y;


        int mouse_en_emulador=0;
        //printf ("x: %04d y: %04d\n",mouse_x,mouse_y);

        int ancho=screen_get_window_size_width_zoom_border_en();

        ancho +=screen_get_ext_desktop_width_zoom();

        int alto=screen_get_window_size_height_zoom_border_en();

        alto +=screen_get_ext_desktop_height_zoom();


        if (mouse_x>=0 && mouse_y>=0
            && mouse_x<=ancho && mouse_y<=alto ) {
                //Si mouse esta dentro de la ventana del emulador
                mouse_en_emulador=1;
        }

        if (  (mouse_x!=last_mouse_x || mouse_y !=last_mouse_y) && mouse_en_emulador) {
            mouse_movido=1;
        }
        else mouse_movido=0;

        last_mouse_x=mouse_x;
        last_mouse_y=mouse_y;

        //printf ("x: %04d y: %04d movido=%d\n",mouse_x,mouse_y,mouse_movido);

        //Quitarle el zoom
        x=mouse_x/zoom_x;
        y=mouse_y/zoom_y;

        //Considerar borde pantalla

        //Todo lo que sea negativo o exceda border, nada.

        //printf ("x: %04d y: %04d\n",x,y);



        //margenes de zona interior de pantalla. para modo rainbow
                int margenx_izq;
                int margeny_arr;
                menu_retorna_margenes_border(&margenx_izq,&margeny_arr);

    //Ya no hace falta restar margenes
    margenx_izq=margeny_arr=0;

    x -=margenx_izq;
    y -=margeny_arr;

    //printf ("x: %04d y: %04d\n",x,y);

    //Aqui puede dar negativo, en caso que cursor este en el border
    //si esta justo en los ultimos 8 pixeles, dara entre -7 y -1. al dividir entre 8, retornaria 0, diciendo erroneamente que estamos dentro de ventana

    if (x<0) x-=(menu_char_width*menu_gui_zoom); //posicion entre -7 y -1 y demas, cuenta como -1, -2 al dividir entre 8
    if (y<0) y-=(menu_char_height*menu_gui_zoom);

    //x /=menu_char_width;
    //y /=8;

    //x /= menu_gui_zoom;
    //y /= menu_gui_zoom;

    //printf ("antes de restar: %d,%d\n",x,y);
    *resultado_x=x;
    *resultado_y=y;
}

//Retorna las coordenadas absolutas del raton (en tamaño de caracter) teniendo en cuenta todo el tamaño de la interfaz del emulador
void menu_calculate_mouse_xy_absolute_interface(int *resultado_x,int *resultado_y)
{
    int x,y;

    menu_calculate_mouse_xy_absolute_interface_pixel(&x,&y);


    x /=menu_char_width;
    y /=menu_char_height;

    x /= menu_gui_zoom;
    y /= menu_gui_zoom;

    //printf ("antes de restar: %d,%d\n",x,y);
    *resultado_x=x;
    *resultado_y=y;




}

void menu_calculate_mouse_xy_absolute_interface_zoom(int *resultado_x,int *resultado_y)
{
    int x,y;

    menu_calculate_mouse_xy_absolute_interface_pixel(&x,&y);


    //multiplicamos por zoom
    x *=zoom_x;
    y *=zoom_y;

    //printf ("antes de restar: %d,%d\n",x,y);
    *resultado_x=x;
    *resultado_y=y;

}

void zxvision_get_mouse_in_window(zxvision_window *ventana,int *posx,int *posy)
{

    int cursor_mouse_x;
    int cursor_mouse_y;
    menu_calculate_mouse_xy_absolute_interface_zoom(&cursor_mouse_x,&cursor_mouse_y);

    int this_win_x=zxvision_window_get_pixel_x_position(ventana);
    int this_win_y=zxvision_window_get_pixel_y_position(ventana);

    //saltar la linea de titulo
    this_win_y +=menu_char_height*zoom_y*menu_gui_zoom;

    //printf("mouse on %d,%d\n",cursor_mouse_x,cursor_mouse_y);
    //printf("Window starts at %d,%d\n",this_win_x,this_win_y);

    int offset_in_window_x=cursor_mouse_x-this_win_x;
    int offset_in_window_y=cursor_mouse_y-this_win_y;

    offset_in_window_x /=zoom_x;
    offset_in_window_y /=zoom_y;

    offset_in_window_x /=menu_gui_zoom;
    offset_in_window_y /=menu_gui_zoom;

    //Obtener coordenada del icono pulsado dentro de la ventana
    //if (offset_in_window_x>=0 && offset_in_window_y>=0) {
        *posx=offset_in_window_x;
        *posy=offset_in_window_y;

    //}

    //else {
    //    return -1;
    //}
}

//Parecido al anterior pero considerando coordenadas relativas a la ventana actual
void menu_calculate_mouse_xy(void)
{
    int x,y;
    if (menu_allows_mouse() ) {
        menu_calculate_mouse_xy_absolute_interface(&x,&y);
    /*
        int mouse_en_emulador=0;
        //printf ("x: %04d y: %04d\n",mouse_x,mouse_y);

        int ancho=screen_get_window_size_width_zoom_border_en();

        ancho +=screen_get_ext_desktop_width_zoom();

        if (mouse_x>=0 && mouse_y>=0
            && mouse_x<=ancho && mouse_y<=screen_get_window_size_height_zoom_border_en() ) {
                //Si mouse esta dentro de la ventana del emulador
                mouse_en_emulador=1;
        }

        if (  (mouse_x!=last_mouse_x || mouse_y !=last_mouse_y) && mouse_en_emulador) {
            mouse_movido=1;
        }
        else mouse_movido=0;

        last_mouse_x=mouse_x;
        last_mouse_y=mouse_y;

        //printf ("x: %04d y: %04d movido=%d\n",mouse_x,mouse_y,mouse_movido);

        //Quitarle el zoom
        x=mouse_x/zoom_x;
        y=mouse_y/zoom_y;

        //Considerar borde pantalla

        //Todo lo que sea negativo o exceda border, nada.

        //printf ("x: %04d y: %04d\n",x,y);



        //margenes de zona interior de pantalla. para modo rainbow
                int margenx_izq;
                int margeny_arr;
                menu_retorna_margenes_border(&margenx_izq,&margeny_arr);

    //Ya no hace falta restar margenes
    margenx_izq=margeny_arr=0;

    x -=margenx_izq;
    y -=margeny_arr;

    //printf ("x: %04d y: %04d\n",x,y);

    //Aqui puede dar negativo, en caso que cursor este en el border
    //si esta justo en los ultimos 8 pixeles, dara entre -7 y -1. al dividir entre 8, retornaria 0, diciendo erroneamente que estamos dentro de ventana

    if (x<0) x-=(menu_char_width*menu_gui_zoom); //posicion entre -7 y -1 y demas, cuenta como -1, -2 al dividir entre 8
    if (y<0) y-=(8*menu_gui_zoom);

    x /=menu_char_width;
    y /=8;

    x /= menu_gui_zoom;
    y /= menu_gui_zoom;

    //printf ("antes de restar: %d,%d\n",x,y);
    */

    zxvision_mouse_x=x;
    zxvision_mouse_y=y;

    x -=current_win_x;
    y -=current_win_y;

    menu_mouse_x=x;
    menu_mouse_y=y;

    //if (x<=0 || y<=0) printf ("x: %04d y: %04d final\n",x,y);

    //printf ("ventana_x %d margen_izq %d\n",ventana_x,margenx_izq);

    //Coordenadas menu_mouse_x y tienen como origen 0,0 en zona superior izquierda de ventana (titulo ventana)
    //Y en coordenadas de linea (y=0 primera linea, y=1 segunda linea, etc)

    }
}



//No dejar aparecer el osd keyboard dentro del mismo osd keyboard
int osd_kb_no_mostrar_desde_menu=0;
int timer_osd_keyboard_menu=0;

//reset_mouse_movido dice que el movimiento de mouse no se considera
//absolutamente_todas_teclas indica si se leen todas las teclas, no solo las habituales de spectrum,
//tambien adicionales de Z88 etc. eso se usa en help keyboard
z80_byte menu_da_todas_teclas_si_reset_mouse_movido(int reset_mouse_movido,int absolutamente_todas_teclas)
{

    //if (mouse_movido) printf("mouse movido en menu_da_todas_teclas 1: %d\n",mouse_movido);

    //Inicializo a 0 para evitar warnings en el compilador
    int ancho_anterior=0;
    int alto_anterior=0;

    if (zxvision_current_window!=NULL) {
        zxvision_window_save_size(zxvision_current_window,&ancho_anterior,&alto_anterior);
        //printf("--ancho antes: %d\n",zxvision_current_window->visible_width);
    }

    //Ver tambien eventos de mouse de zxvision
    //int pulsado_boton_cerrar=
    //if (mouse_movido) printf("menu_da_todas_teclas. mouse movido antes de zxvision_handle_mouse_events: %d\n",mouse_movido);
    zxvision_handle_mouse_events(zxvision_current_window);
    //if (mouse_movido) printf("menu_da_todas_teclas. mouse movido despues de zxvision_handle_mouse_events: %d\n",mouse_movido);

    //if (mouse_movido) printf("mouse movido en menu_da_todas_teclas 2: %d\n",mouse_movido);

    //On screen keyboard desde el propio menu. Necesita multitask
    if (menu_si_pulsada_tecla_osd() && !osd_kb_no_mostrar_desde_menu && !timer_osd_keyboard_menu && menu_multitarea) {
            debug_printf(VERBOSE_INFO,"Calling osd keyboard from menu keyboard read routine");

            osd_kb_no_mostrar_desde_menu=1;
                        menu_call_onscreen_keyboard_from_menu();
                        //TODO: si se pulsa CS o SS, no lo detecta como tecla pulsada (en parte logico)
                        //pero esto hara que al pulsar una de esas teclas no se abra el menu osd de nuevo hasta que se pulse otra
                        //tecla distinta
                        //printf ("despues de haber leido tecla de osd\n");
            osd_kb_no_mostrar_desde_menu=0;

            //Esperar 1 segundo hasta poder abrir menu osd. La pulsacion de teclas desde osd se hace por medio segundo,
            //con lo que al retornar a 1 segundo ya es correcto
            timer_osd_keyboard_menu=50;
    }



    z80_byte acumulado;

    acumulado=255;

    //symbol i shift no cuentan por separado
    acumulado=acumulado & (puerto_65278 | 1) & puerto_65022 & puerto_64510 & puerto_63486 & puerto_61438 & puerto_57342 & puerto_49150 & (puerto_32766 |2) & puerto_especial1 & puerto_especial2 & puerto_especial3 & puerto_especial4;

    if (absolutamente_todas_teclas) {
        //Symbol y shift
        acumulado=acumulado & puerto_65278 & puerto_32766;

        //Z88
        acumulado=acumulado & blink_kbd_a15 & blink_kbd_a14 & blink_kbd_a13 & blink_kbd_a12 & blink_kbd_a11 & blink_kbd_a10 & blink_kbd_a9 & blink_kbd_a8;

        //QL
        acumulado=acumulado &   ql_keyboard_table[0] & ql_keyboard_table[1] & ql_keyboard_table[2] & ql_keyboard_table[3] &
                                ql_keyboard_table[4] & ql_keyboard_table[5] & ql_keyboard_table[6] & ql_keyboard_table[7];
    }

    if (zxvision_setting_use_speccy_keys.v && (puerto_65278 & 1)==0 && (puerto_32766 & 2)==0) {
        //Considerar si pulsado symbol y shift a la vez (simula TAB)
        //Habitualmente no consideramos ni symbol ni shift en esta función
        acumulado &=(255-1);
    }

    //Boton shift+cursor en ventana que no permite background, cerrarla
    if (menu_pressed_shift_cursor_window_doesnot_allow) {
        //printf("en menu_da_todas_teclas pulsado shift+cursor en ventana que no permite background\n");
        acumulado |=1;
    }

    //Boton cerrar ventana
    if (mouse_pressed_close_window) {
        acumulado |=1;
    }

    //Boton background ventana
    if (mouse_pressed_background_window) {
        //printf ("pulsado background en menu_da_todas_teclas\n");
        acumulado |=1;
    }

    //Boton hotkey ventana
    if (mouse_pressed_hotkey_window) {
        //printf ("pulsado hotkey desde menu_da_todas_teclas\n");
        acumulado &=(255-1);
        //NOTA: indicamos aqui que ha habido pulsacion de tecla,
        //dado que partimos de mascara 255, poner ese bit a 0 le decimos que hay pulsada una tecla
        //Misterio: porque con mouse_pressed_close_window y mouse_pressed_background_window le hago OR 1? no tiene sentido....
    }


    //no ignorar disparo
    //z80_byte valor_joystick=(puerto_especial_joystick&31)^255;

    //no ignorar ninguno de los 4 botones de disparo
    z80_byte valor_joystick=puerto_especial_joystick ^ 255;

    acumulado=acumulado & valor_joystick;

    //printf ("acumulado %d\n",acumulado);

    //contar tambien botones mouse
    if (si_menu_mouse_activado()) {
        //menu_calculate_mouse_xy(); //Ya no hacemos esto pues se ha calculado ya arriba en zxvision_handle_mouse_events
        //quiza pareceria que no hay problema en leerlo dos veces, el problema es con la variable mouse_leido,
        //que al llamarla aqui la segunda vez, siempre dira que el mouse no se ha movido

        //printf("mouse left %d mouse_right %d mouse_movido %d\n",mouse_left,mouse_right,mouse_movido);

        int valor_mouse_movido=mouse_movido;

        if (reset_mouse_movido) valor_mouse_movido=0;

        z80_byte valor_botones_mouse=(mouse_left | mouse_right | valor_mouse_movido)^255;
        acumulado=acumulado & valor_botones_mouse;
    }

    //printf("mouse left %d mouse right %d mouse_movido %d\n",mouse_left,mouse_right,mouse_movido);
    //printf ("acumulado 0 %d\n",acumulado);

    //Contar también algunas teclas solo menu:
    z80_byte valor_teclas_menus=(menu_backspace.v|menu_tab.v)^255;

    //printf("valor_teclas_menus: %d\n",valor_teclas_menus);

    acumulado=acumulado & valor_teclas_menus;

    //Si ha cambiado tamanyo ventana, notificar como si se hubiera pulsado una tecla
    //asi en menus como por ejemplo help keyboard, o generic message tooltip, reciben al momento como si se hubiera pulsado tecla
    //y veran que el tamaño ha cambiado y actuan en consecuencia, por ejemplo en help keyboard recrea la ventana en transparente,
    //y en generic message tooltip justifica de nuevo todo el texto al ancho actual
    if (zxvision_current_window!=NULL) {
        if (zxvision_current_window->visible_width!=ancho_anterior || zxvision_current_window->visible_height!=alto_anterior) {
            //printf("salir por cambio geometria\n");
            acumulado &=(255-1);
            //NOTA: indicamos aqui que ha habido pulsacion de tecla,
            //dado que partimos de mascara 255, poner ese bit a 0 le decimos que hay pulsada una tecla
        }
    }

    //printf("acumulado 00 %d\n",acumulado);

    if ( (acumulado&MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA) {
        //printf ("Retornamos acumulado en menu_da_todas_teclas: %d\n",acumulado);
        return acumulado;
    }


    //printf ("Retornamos acumulado en menu_da_todas_teclas_2: %d\n",acumulado);

    return acumulado;


}


z80_byte menu_da_todas_teclas(void)
{
    return menu_da_todas_teclas_si_reset_mouse_movido(0,0);
}

z80_byte menu_da_todas_teclas_cualquiera(void)
{
    return menu_da_todas_teclas_si_reset_mouse_movido(0,1);
}

int menu_si_tecla_pulsada(void)
{
    z80_byte acumulado=menu_da_todas_teclas();
    if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) != MENU_PUERTO_TECLADO_NINGUNA) return 1;
    else return 0;
}

//Para forzar desde remote command a salir de la funcion, sin haber pulsado tecla realmente
//z80_bit menu_espera_tecla_no_cpu_loop_flag_salir={0};

//Esperar a pulsar una tecla sin ejecutar cpu
void menu_espera_tecla_no_cpu_loop(void)
{

    z80_byte acumulado;

        do {

                scr_actualiza_tablas_teclado();
        realjoystick_main();

                //0.5 ms
                usleep(500);

                acumulado=menu_da_todas_teclas();

        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA
                //&&	menu_espera_tecla_no_cpu_loop_flag_salir.v==0
                            );


    //menu_espera_tecla_no_cpu_loop_flag_salir.v=0;

}



void menu_espera_no_tecla_no_cpu_loop(void)
{

        //Esperar a liberar teclas
        z80_byte acumulado;

        do {

                scr_actualiza_tablas_teclado();
        realjoystick_main();

                //0.5 ms
                usleep(500);

                acumulado=menu_da_todas_teclas();

        //printf ("menu_espera_no_tecla_no_cpu_loop acumulado: %d\n",acumulado);

        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) != MENU_PUERTO_TECLADO_NINGUNA);

}


void menu_espera_tecla_timeout_tooltip(void)
{

        //Esperar a pulsar una tecla o timeout de tooltip
        z80_byte acumulado;

        acumulado=menu_da_todas_teclas();

        int resetear_contadores=0;

        //Si se entra y no hay tecla pulsada, resetear contadores
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA) {
            resetear_contadores=1;
        }

        do {
                menu_cpu_core_loop();


                acumulado=menu_da_todas_teclas();

        //printf ("menu_espera_tecla_timeout_tooltip acumulado: %d\n",acumulado);

        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && menu_tooltip_counter<TOOLTIP_SECONDS);

    if (resetear_contadores) {
            menu_reset_counters_tecla_repeticion();
    }

}



//tipo: 1 volver timeout normal como ventanas splash. 2. no finaliza, franjas continuamente moviendose
void zxvision_espera_tecla_timeout_window_splash(int tipo)
{

    z80_byte tecla;
    //printf ("espera splash\n");
    do {

        //Esperar a pulsar una tecla o timeout de window splash
        //z80_byte acumulado;

        int contador_antes=menu_window_splash_counter_ms;
        int trozos=4;
        //WINDOW_SPLASH_SECONDS.
        //5 pasos. total de WINDOW_SPLASH_SECONDS
        int tiempototal=1000*WINDOW_SPLASH_SECONDS;
        //Quitamos 1 segundo
        tiempototal-=1000;

        //Intervalo de cambio
        int intervalo=tiempototal/5; //5 pasos
        //printf ("intervalo: %d\n",intervalo);

        int indice_apagado=0;




    do {
                menu_cpu_core_loop();


                //acumulado=menu_da_todas_teclas();
                tecla=zxvision_read_keyboard();

                //con boton izquierdo no salimos
                if (tecla==13 && mouse_left) {
                    tecla=0;
                }

                //Cada 400 ms
                if (menu_window_splash_counter_ms-contador_antes>intervalo) {
                    //trozos--;
                    contador_antes=menu_window_splash_counter_ms;
                    //printf ("dibujar franjas trozos: %d\n",trozos);
                    if (trozos>=0) {
                        if (tipo==1) {
                            menu_dibuja_ventana_franja_arcoiris_trozo_current(trozos);
                            //printf("trozos: %d\n",trozos);
                        }
                    }

                    trozos--;

                    if (tipo==2) menu_dibuja_ventana_franja_arcoiris_oscuro_current(indice_apagado);
                    indice_apagado++;
                }


        //printf ("menu_espera_tecla_timeout_tooltip acumulado: %d\n",acumulado);
        //printf ("contador splash: %d\n",menu_window_splash_counter);


    } while (tecla==0 && menu_window_splash_counter<WINDOW_SPLASH_SECONDS);

    menu_window_splash_counter=0;
    menu_window_splash_counter_ms=0;

    } while (tipo==2 && tecla==0);

}



//Esperar a tecla ESC, o que la condicion de funcion sea diferente de 0
//Cada x fraccion de segundo se llama la condicion y tambien la funcion de print
//Poner alguna a NULL si no se quiere llamar
//Funciones de condicion y progreso tambien funcionan aun sin multitarea
void zxvision_espera_tecla_condicion_progreso(zxvision_window *w,int (*funcioncond) (zxvision_window *),void (*funcionprint) (zxvision_window *) )
{

    z80_byte tecla;
    int condicion=0;
    int contador_antes=menu_window_splash_counter_ms;
    int intervalo=20*ZXVISION_SIMPLE_PROGRESS_WINDOW_FRAMES_REFRESH; //En milisegundos

    //contador en us
    int contador_no_multitask=0;


    //printf ("espera splash\n");
    do {

                menu_cpu_core_loop();
                int pasado_cuarto_segundo=0;

                //TODO: se puede dar el caso que se llame aqui pero el thread aun no se haya creado, lo que provoca
                //que dice que el thread no esta en ejecucion aun y por tanto cree que esta finalizado, diciendo que la condicion de salida es verdadera
                //y salga cuando aun no ha finalizado
                //Seria raro, porque el intervalo de comprobacion es cada 1/4 de segundo, y en ese tiempo se tiene que haber lanzado el thread de sobra

                //En caso que no haya multitarea, activar las funciones de print y condicion de otra manera
                 if (!menu_multitarea) {
                    contador_no_multitask+=MENU_CPU_CORE_LOOP_SLEEP_NO_MULTITASK;


                    if (contador_no_multitask>=intervalo*1000) {
                        //printf ("Pasado medio segundo %d\n",contador_segundo);
                        contador_no_multitask=0;
                        pasado_cuarto_segundo=1;

                        //printf ("refresca pantalla\n");
                        menu_refresca_pantalla();

                    }
                }

                //acumulado=menu_da_todas_teclas();
                tecla=zxvision_read_keyboard();

                //con boton izquierdo no salimos
                if (tecla==13 && mouse_left) {
                    tecla=0;
                }

                if (menu_window_splash_counter_ms-contador_antes>intervalo) pasado_cuarto_segundo=1;

                //Cada 224 ms
                if (pasado_cuarto_segundo) {
                    //trozos--;
                    contador_antes=menu_window_splash_counter_ms;
                    //printf ("dibujar franjas trozos: %d\n",trozos);
                    //llamar a la condicion
                    if (funcioncond!=NULL) condicion=funcioncond(w);

                    //llamar a funcion print
                    if (funcionprint!=NULL) funcionprint(w);

                }


    } while (tecla==0 && !condicion);


}

void menu_espera_tecla_cualquiera(void)
{

        //Esperar a pulsar una tecla
        z80_byte acumulado;

    //Si al entrar aqui ya hay tecla pulsada, volver
        acumulado=menu_da_todas_teclas_cualquiera();
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA) return;


    do {
        menu_cpu_core_loop();


        acumulado=menu_da_todas_teclas_cualquiera();


    } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA);

    //Al salir del bucle, reseteamos contadores de repeticion
    menu_reset_counters_tecla_repeticion();

}

void menu_espera_tecla(void)
{

        //Esperar a pulsar una tecla
        z80_byte acumulado;

    //Si al entrar aqui ya hay tecla pulsada, volver
        acumulado=menu_da_todas_teclas();
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA) return;


    do {
        menu_cpu_core_loop();


        acumulado=menu_da_todas_teclas();


    } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA);

    //Al salir del bucle, reseteamos contadores de repeticion
    menu_reset_counters_tecla_repeticion();

}

void menu_espera_tecla_o_pending_error_message(void)
{

    //Esperar a pulsar una tecla o hay pendiente mostrar un mensaje de error
    z80_byte acumulado;

    //Si al entrar aqui ya hay tecla pulsada, volver
    acumulado=menu_da_todas_teclas();
    if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA) return;


    do {
        menu_cpu_core_loop();

        acumulado=menu_da_todas_teclas();


    } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && !if_pending_error_message);

    //Al salir del bucle, reseteamos contadores de repeticion
    menu_reset_counters_tecla_repeticion();

}

void menu_espera_tecla_o_wheel(void)
{

        //Esperar a pulsar una tecla
        z80_byte acumulado;

    //Si al entrar aqui ya hay tecla pulsada, volver
        acumulado=menu_da_todas_teclas();
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA) return;


    do {
        menu_cpu_core_loop();


        acumulado=menu_da_todas_teclas();


    } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && mouse_wheel_vertical==0);

    //Al salir del bucle, reseteamos contadores de repeticion
    menu_reset_counters_tecla_repeticion();

}

void menu_espera_tecla_o_joystick(void)
{

        realjoystick_hit=0;

        //Esperar a pulsar una tecla o joystick
        z80_byte acumulado;

        do {
                menu_cpu_core_loop();


                acumulado=menu_da_todas_teclas();

        //printf ("menu_espera_tecla_o_joystick acumulado: %d\n",acumulado);

        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && !realjoystick_hit );

}




void menu_espera_no_tecla(void)
{

        //Esperar a liberar teclas. No ejecutar ni una instruccion cpu si la tecla esta liberada
    //con eso evitamos que cuando salte un breakpoint, que llama aqui, no se ejecute una instruccion y el registro PC apunte a la siguiente instruccion
        z80_byte acumulado;
    int salir=0;

        do {
        acumulado=menu_da_todas_teclas();
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) == MENU_PUERTO_TECLADO_NINGUNA) {
            salir=1;
        }

        else {
            menu_cpu_core_loop();
        }

    //printf ("menu_espera_no_tecla acumulado: %d\n",acumulado);

    } while (!salir);

}

//Igual que menu_espera_no_tecla pero si se mueve el mouse no considera que eso haya sido una pulsacion y no haya que esperar
void old_menu_espera_no_tecla_no_mouse_movido(void)
{

    //Esperar a liberar teclas. No ejecutar ni una instruccion cpu si la tecla esta liberada
    //con eso evitamos que cuando salte un breakpoint, que llama aqui, no se ejecute una instruccion y el registro PC apunte a la siguiente instruccion
    z80_byte acumulado;
    int salir=0;

    do {
        acumulado=menu_da_todas_teclas();
        //printf("menu_espera_no_tecla_no_mouse_movido: acumulado %02XH MENU_PUERTO_TECLADO_NINGUNA %02XH\n",acumulado,MENU_PUERTO_TECLADO_NINGUNA);
        //printf("menu_espera_no_tecla_no_mouse_movido: mouse movido: %d\n",mouse_movido);

        //Si se mueve y no hay mouse pulsado
        if (mouse_movido && !mouse_left && !mouse_right) {
            //printf("menu_espera_no_tecla_no_mouse_movido: ignorar mouse_movido\n");
            acumulado |=1; //como si no se hubiera movido
        }
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) == MENU_PUERTO_TECLADO_NINGUNA) {
            salir=1;
        }

        else {
            menu_cpu_core_loop();
        }

    //printf ("menu_espera_no_tecla acumulado: %d\n",acumulado);

    } while (!salir);

}

void menu_espera_no_tecla_no_mouse_movido(void)
{

        //Esperar a liberar teclas. No ejecutar ni una instruccion cpu si la tecla esta liberada
    //con eso evitamos que cuando salte un breakpoint, que llama aqui, no se ejecute una instruccion y el registro PC apunte a la siguiente instruccion
        z80_byte acumulado;
    int salir=0;

        do {
            //no tener en cuenta que se mueva mouse
        acumulado=menu_da_todas_teclas_si_reset_mouse_movido(1,0);
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) == MENU_PUERTO_TECLADO_NINGUNA) {
            salir=1;
        }

        else {
            menu_cpu_core_loop();
        }

    //printf ("menu_espera_no_tecla acumulado: %d\n",acumulado);

    } while (!salir);

}

/*
void menu_espera_no_joystick(void)
{

    //z80_byte acumulado;
    //int salir=0;

    do {
        //acumulado=menu_da_todas_teclas();
        //if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) == MENU_PUERTO_TECLADO_NINGUNA) {
        //	salir=1;
        //}

        //else {
        //	menu_cpu_core_loop();
        //}

        menu_cpu_core_loop();

    } while (menu_info_joystick_last_button>=0);

}
*/

//#define CONTADOR_HASTA_REPETICION (MACHINE_IS_Z88 ? 75 : 25)
//#define CONTADOR_ENTRE_REPETICION (MACHINE_IS_Z88 ? 3 : 1)

#define CONTADOR_HASTA_REPETICION (25)
#define CONTADOR_ENTRE_REPETICION (1)


//0.5 segundos para empezar repeticion (25 frames)
int menu_contador_teclas_repeticion;

int menu_segundo_contador_teclas_repeticion;


void menu_reset_counters_tecla_repeticion(void)
{
    //printf ("menu_reset_counters_tecla_repeticion\n");
                        menu_contador_teclas_repeticion=CONTADOR_HASTA_REPETICION;
                        menu_segundo_contador_teclas_repeticion=CONTADOR_ENTRE_REPETICION;
}

void menu_espera_no_tecla_con_repeticion(void)
{

    //Tambien salir si se pulsa tecla diferente, para poder escribir rapido y considerar que en esos casos,
    //al pulsar rapido las teclas por ejemplo AB, se pasa de la A a la B sin pasar por un momento de "no tecla"
    z80_byte tecla_inicial=menu_get_pressed_key();


    //Esperar a liberar teclas, pero si se deja pulsada una tecla el tiempo suficiente, se retorna
    z80_byte acumulado;

    //printf ("menu_espera_no_tecla_con_repeticion %d\n",menu_contador_teclas_repeticion);

    //x frames de segundo entre repeticion
    menu_segundo_contador_teclas_repeticion=CONTADOR_ENTRE_REPETICION;

    do {
        menu_cpu_core_loop();

        acumulado=menu_da_todas_teclas();

        //printf ("menu_espera_no_tecla_con_repeticion acumulado: %d\n",acumulado);

        //si no hay tecla pulsada, restablecer contadores
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) == MENU_PUERTO_TECLADO_NINGUNA) menu_reset_counters_tecla_repeticion();

        //printf ("contadores: 1 %d  2 %d\n",menu_contador_teclas_repeticion,menu_segundo_contador_teclas_repeticion);

        z80_byte tecla_actual=menu_get_pressed_key();
        if (tecla_actual!=tecla_inicial) {
            //printf("menu_espera_no_tecla_con_repeticion tecla_actual %d no es tecla_inicial %d\n",tecla_actual,tecla_inicial);
            return;
        }

    } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) != MENU_PUERTO_TECLADO_NINGUNA && menu_segundo_contador_teclas_repeticion!=0);


}






//Quita de la linea los caracteres de atajo ~~ o ^^ o $$X
void menu_dibuja_menu_stdout_texto_sin_atajo(char *origen, char *destino)
{

    int indice_orig,indice_dest;

    indice_orig=indice_dest=0;

    while (origen[indice_orig]) {
        if (menu_escribe_texto_si_inverso(origen,indice_orig)) {
            indice_orig +=2;
        }

        if (menu_escribe_texto_si_parpadeo(origen,indice_orig)) {
            indice_orig +=2;
        }

        if (menu_escribe_texto_si_cambio_tinta(origen,indice_orig)) {
            indice_orig +=3;
        }


        destino[indice_dest++]=origen[indice_orig++];
    }

    destino[indice_dest]=0;

}


void menu_dibuja_menu_stdout_print_numero_opcion(int max_opciones,int opcion_marcada)
{
    printf ("%2d)",max_opciones);
    //Si es opcion marcada
    //TODO: quiza otros caracteres distintos para indicar que es una opcion marcada
    if (opcion_marcada) printf("__");

    char buf[10];
    sprintf (buf,"%d",max_opciones);
    if (!menu_speech_tecla_pulsada) {
        scrstdout_menu_print_speech_macro(buf);
    }

}

int menu_dibuja_menu_stdout(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo)
{
    int linea_seleccionada=*opcion_inicial;
    char texto_linea_sin_shortcut[64];

    menu_item *aux;

    //Titulo
    printf ("\n");
    printf ("%s\n",titulo);
    printf ("------------------------\n\n");
    scrstdout_menu_print_speech_macro(titulo);

    aux=m;

    int max_opciones=0;

    int tecla=13;

    //para speech stdout. asumir no tecla pulsada. si se pulsa tecla, para leer menu
    menu_speech_reset_tecla_pulsada();

    do {
        if (menu_item_retornar_avanzados(aux)) {
            //scrstdout_menu_kbhit_macro();
            max_opciones++;

            if (aux->tipo_opcion!=MENU_OPCION_SEPARADOR) {

                //Ver si esta activa
                t_menu_funcion_activo menu_funcion_activo;

                menu_funcion_activo=aux->menu_funcion_activo;

                int opcion_activada=1;

                //Si tiene condicion de activa
                if ( menu_funcion_activo!=NULL) {
                    if (!menu_funcion_activo()) {
                        opcion_activada=0;
                    }

                }



                if (opcion_activada) {
                    menu_dibuja_menu_stdout_print_numero_opcion(max_opciones,aux->opcion_marcada);
                }

                else {
                    printf ("x  ");
                    if (!menu_speech_tecla_pulsada) {
                        scrstdout_menu_print_speech_macro("Unavailable option: ");
                    }
                }

                //Imprimir linea menu pero descartando los ~~ de los atajo de teclado o ^^
                menu_dibuja_menu_stdout_texto_sin_atajo(menu_retorna_item_language(aux),texto_linea_sin_shortcut);


                printf ( "%s",texto_linea_sin_shortcut);
                if (!menu_speech_tecla_pulsada) {
                    scrstdout_menu_print_speech_macro (texto_linea_sin_shortcut);
                }

            }

            else {
                printf("%s",menu_retorna_item_language(aux));
                scrstdout_menu_print_speech_macro(menu_retorna_item_language(aux));
            }


            printf ("\n");
        }

        aux=aux->siguiente_item;
    } while (aux!=NULL);

    printf ("\n");

    char texto_opcion[256];

    int salir_opcion;


    do {

        salir_opcion=1;
        printf("Option number? (prepend the option with h for help, t for tooltip). Write esc to go back. ");
        //printf ("menu_speech_tecla_pulsada: %d\n",menu_speech_tecla_pulsada);
        if (!menu_speech_tecla_pulsada) {
            scrstdout_menu_print_speech_macro("Option number? (prepend the option with h for help, t for tooltip). Write esc to go back. ");
        }

        //paso de curses a stdout deja stdout que no hace flush nunca. forzar
        fflush(stdout);
        scanf("%s",texto_opcion);

        if (!strcasecmp(texto_opcion,"esc")) {
            tecla=MENU_RETORNO_ESC;
        }

        else if (texto_opcion[0]=='h' || texto_opcion[0]=='t') {
            salir_opcion=0;
            char *texto_ayuda;
            linea_seleccionada=atoi(&texto_opcion[1]);
            linea_seleccionada--;
            if (linea_seleccionada>=0 && linea_seleccionada<max_opciones) {

                char *titulo_texto;

                if (texto_opcion[0]=='h') {
                    texto_ayuda=menu_retorna_item(m,linea_seleccionada)->texto_ayuda;
                    titulo_texto="Help";
                }

                else {
                    texto_ayuda=menu_retorna_item(m,linea_seleccionada)->texto_tooltip;
                    titulo_texto="Tooltip";
                }


                if (texto_ayuda!=NULL) {
                    menu_generic_message(titulo_texto,texto_ayuda);
                }
                else {
                    printf ("Item has no %s\n",titulo_texto);
                }
            }

            else {
                printf ("Invalid option number\n");
            }
        }

        else {
            linea_seleccionada=atoi(texto_opcion);

            if (linea_seleccionada<1 || linea_seleccionada>max_opciones) {
                printf ("Incorrect option number\n");
                salir_opcion=0;
            }

            //Numero correcto
            else {
                linea_seleccionada--;

                //Ver si item no es separador
                menu_item *item_seleccionado;
                item_seleccionado=menu_retorna_item(m,linea_seleccionada);

                if (item_seleccionado->tipo_opcion==MENU_OPCION_SEPARADOR) {
                    salir_opcion=0;
                    printf ("Item is a separator\n");
                }

                else {

                    //Ver si item esta activo
                    t_menu_funcion_activo menu_funcion_activo;
                    menu_funcion_activo=item_seleccionado->menu_funcion_activo;

                    if ( menu_funcion_activo!=NULL) {

                        if (menu_funcion_activo()==0) {
                            //opcion inactiva
                            salir_opcion=0;
                            printf ("Item is disabled\n");
                        }
                    }
                }
            }
        }

    } while (salir_opcion==0);

    menu_item *menu_sel;
    menu_sel=menu_retorna_item(m,linea_seleccionada);

    item_seleccionado->menu_funcion=menu_sel->menu_funcion;
    item_seleccionado->tipo_opcion=menu_sel->tipo_opcion;
    item_seleccionado->valor_opcion=menu_sel->valor_opcion;

    strcpy(item_seleccionado->texto_opcion,menu_retorna_item_language(menu_sel));
    strcpy(item_seleccionado->texto_misc,menu_sel->texto_misc);


    //Liberar memoria del menu
    aux=m;
    menu_item *nextfree;

    do {
        //printf ("Liberando %x\n",aux);
        nextfree=aux->siguiente_item;
        free(aux);
        aux=nextfree;
    } while (aux!=NULL);

    *opcion_inicial=linea_seleccionada;


    if (tecla==MENU_RETORNO_ESC) return MENU_RETORNO_ESC;

    return MENU_RETORNO_NORMAL;
}


//devuelve a que numero de opcion corresponde el atajo pulsado
//-1 si a ninguno
//NULL si a ninguno
int menu_retorna_atajo(menu_item *m,z80_byte tecla)
{

    //Si letra en mayusculas, bajar a minusculas
    if (tecla>='A' && tecla<='Z') tecla +=('a'-'A');

    int linea=0;

    while (m!=NULL) {
        if (m->atajo_tecla==tecla && menu_item_retornar_avanzados(m)) {
            debug_printf (VERBOSE_DEBUG,"Shortcut found at entry number: %d",linea);
            return linea;
        }

        if (menu_item_retornar_avanzados(m)) linea++;
        m=m->siguiente_item;
    }

    //no encontrado atajo. escribir entradas de menu con atajos para informar al usuario
    menu_writing_inverse_color.v=1;
    return -1;

}

int menu_active_item_primera_vez=1;



void menu_escribe_opciones_zxvision(zxvision_window *ventana,menu_item *aux,int linea_seleccionada,int max_opciones)
{

        int i;

        //Opcion esta permitida seleccionarla (no esta en rojo)
        int opcion_activada;

        int menu_tabulado=0;
        if (aux->es_menu_tabulado) menu_tabulado=1;


        //Opcion de donde esta el cursor
        //Margen suficiente para que quepa dicha linea y el texto "Selected item: "
        char texto_opcion_seleccionada[MAX_TEXTO_OPCION+30];
        //Asumimos por si acaso que no hay ninguna activa
        texto_opcion_seleccionada[0]=0;
        //La opcion donde esta el cursor, si esta activada o no. Asumimos que si, por si acaso
        int opcion_seleccionada_activada=1;


        int pos_y_destino=0;


        for (i=0;i<max_opciones;/*i++*/) {

            if (menu_item_retornar_avanzados(aux)) {

                //si la opcion seleccionada es un separador, el cursor saltara a la siguiente
                //Nota: el separador no puede ser final de menu
                //if (linea_seleccionada==i && aux->tipo_opcion==MENU_OPCION_SEPARADOR) linea_seleccionada++;

                t_menu_funcion_activo menu_funcion_activo;

                menu_funcion_activo=aux->menu_funcion_activo;

                if (menu_funcion_activo!=NULL) {
                    opcion_activada=menu_funcion_activo();
                }

                else {
                    opcion_activada=1;
                }

                //Al listar opciones de menu, decir si la opcion está desabilitada
                if (!opcion_activada) menu_textspeech_send_text("Unavailable option: ");

                int opcion_marcada=aux->opcion_marcada;

                //Cuando haya opcion_activa, nos la apuntamos para decirla al final en speech.
                //Y si es la primera vez en ese menu, dice "Selected item". Sino, solo dice el nombre de la opcion
                if (linea_seleccionada==i) {
                    if (menu_active_item_primera_vez) {
                        sprintf (texto_opcion_seleccionada,"Selected item: %s",menu_retorna_item_language(aux));
                        menu_active_item_primera_vez=0;
                    }

                    else {
                        sprintf (texto_opcion_seleccionada,"%s",menu_retorna_item_language(aux));
                    }

                    opcion_seleccionada_activada=opcion_activada;
                }

                if (menu_tabulado) {
                    menu_escribe_linea_opcion_tabulado_zxvision(ventana,i,linea_seleccionada,
                        opcion_activada,menu_retorna_item_language(aux),aux->menu_tabulado_x,aux->menu_tabulado_y,opcion_marcada,aux->opcion_campo_seleccionable);
                }


                else {
                    int y_destino=pos_y_destino;
                    int linea_seleccionada_destino=linea_seleccionada;

                    if (y_destino>=0) {

                            menu_escribe_linea_opcion_zxvision(ventana,y_destino,linea_seleccionada_destino,
                                opcion_activada,menu_retorna_item_language(aux),aux->tiene_submenu,opcion_marcada,aux->genera_ventana,aux->opcion_campo_seleccionable);
                            //menu_escribe_linea_opcion_zxvision(ventana,y_destino,linea_seleccionada_destino,opcion_activada,aux->texto_opcion,aux->tiene_submenu);

                    }


                }

                //TODO: creo que estas dos variables hacen lo mismo
                pos_y_destino++;
                i++;

            }


            aux=aux->siguiente_item;

        }



        if (texto_opcion_seleccionada[0]!=0) {
            //Selected item siempre quiero que se escuche

            //Guardamos estado actual
            int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
            menu_speech_reset_tecla_pulsada();


            //Al decir la linea seleccionada de menu, decir si la opcion está desabilitada
            if (!opcion_seleccionada_activada) menu_textspeech_send_text("Unavailable option: ");

            menu_textspeech_send_text(texto_opcion_seleccionada);

        /*if (menu_speech_tecla_pulsada && !antes_menu_speech_tecla_pulsada) {
            printf("Pasar de 1 a 0 desde \n");
            debug_exec_show_backtrace();
        }*/

            //Restauro estado
            //Pero si se ha pulsado tecla, no restaurar estado
            //Esto sino provocaria que , por ejemplo, en la ventana de confirmar yes/no,
            //se entra con menu_speech_reset_tecla_pulsada(), se pulsa tecla mientras se esta leyendo el item activo,
            //y luego al salir de aqui, se pierde el valor que se habia metido (1) y se vuelve a poner el 0 del principio
            //provocando que cada vez que se mueve el cursor, se relea la ventana entera
            if (menu_speech_tecla_pulsada==0) menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;
        }
}



int menu_dibuja_menu_cursor_arriba(int linea_seleccionada,int max_opciones,menu_item *m)
{
    if (linea_seleccionada==0) linea_seleccionada=max_opciones-1;
    else {
        linea_seleccionada--;
        //ver si la linea seleccionada es un separador
        int salir=0;
        while (menu_retorna_item(m,linea_seleccionada)->tipo_opcion==MENU_OPCION_SEPARADOR && !salir) {
            linea_seleccionada--;
            //si primera linea es separador, nos iremos a -1
            if (linea_seleccionada==-1) {
                linea_seleccionada=max_opciones-1;
                salir=1;
            }
        }
    }
    //si linea resultante es separador, decrementar
    while (menu_retorna_item(m,linea_seleccionada)->tipo_opcion==MENU_OPCION_SEPARADOR) linea_seleccionada--;

    //Decir que se ha pulsado tecla
    menu_speech_set_tecla_pulsada();

    return linea_seleccionada;
}

int menu_dibuja_menu_cursor_abajo(int linea_seleccionada,int max_opciones,menu_item *m)
{
    if (linea_seleccionada==max_opciones-1) linea_seleccionada=0;
    else {
        linea_seleccionada++;
        //ver si la linea seleccionada es un separador
        int salir=0;
        while (menu_retorna_item(m,linea_seleccionada)->tipo_opcion==MENU_OPCION_SEPARADOR && !salir) {
            linea_seleccionada++;
            //si ultima linea es separador, nos salimos de rango
            if (linea_seleccionada==max_opciones) {
                linea_seleccionada=0;
                salir=0;
            }
        }
    }
    //si linea resultante es separador, incrementar
    while (menu_retorna_item(m,linea_seleccionada)->tipo_opcion==MENU_OPCION_SEPARADOR) linea_seleccionada++;

    //Decir que se ha pulsado tecla
    menu_speech_set_tecla_pulsada();

    return linea_seleccionada;
}


int menu_dibuja_menu_cursor_abajo_tabulado(int linea_seleccionada,int max_opciones,menu_item *m)
{

    if (linea_seleccionada==max_opciones-1) linea_seleccionada=0;

    else {

        //Ubicarnos primero en el item de menu seleccionado
        menu_item *m_aux=menu_retorna_item(m,linea_seleccionada);

        //Su coordenada y original
        int orig_tabulado_y=m_aux->menu_tabulado_y;
        int orig_tabulado_x=m_aux->menu_tabulado_x;


        //Y vamos hacia abajo hasta que coordenada y sea diferente. Y si nunca es diferente y hemos recorrido todas, salir
        int contador_bucle;

        contador_bucle=0;
        do {
            //printf ("1 antes vert orig y: %d y: %d linea_seleccionada: %d texto: %s max_opciones: %d\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion,max_opciones);
            linea_seleccionada=menu_dibuja_menu_cursor_abajo(linea_seleccionada,max_opciones,m);
            m_aux=menu_retorna_item(m,linea_seleccionada);
            //printf ("1 despues vert orig y: %d y: %d linea_seleccionada: %d texto: %s max_opciones: %d\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion,max_opciones);
            contador_bucle++;
        } while (m_aux->menu_tabulado_y==orig_tabulado_y && contador_bucle<max_opciones);

        int posible_posicion=linea_seleccionada;
        int final_y=m_aux->menu_tabulado_y;

        //Y ahora buscar la que tenga misma coordenada x o mas a la derecha, si la hubiera
        contador_bucle=0;
        while (m_aux->menu_tabulado_y==final_y && m_aux->menu_tabulado_x<orig_tabulado_x && contador_bucle<max_opciones) {
            posible_posicion=linea_seleccionada;
            //printf ("2 antes horiz orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
            linea_seleccionada=menu_dibuja_menu_cursor_abajo(linea_seleccionada,max_opciones,m);
            m_aux=menu_retorna_item(m,linea_seleccionada);
            //printf ("2 despues horiz orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
            contador_bucle++;
        };

        //Si no estamos en misma posicion y, volver a posicion
        if (m_aux->menu_tabulado_y!=final_y) linea_seleccionada=posible_posicion;
    }

    //Decir que se ha pulsado tecla
    menu_speech_set_tecla_pulsada();

    return linea_seleccionada;
}

int menu_dibuja_menu_cursor_arriba_tabulado(int linea_seleccionada,int max_opciones,menu_item *m)
{

    if (linea_seleccionada==0) linea_seleccionada=max_opciones-1;

    else {

        //Ubicarnos primero en el item de menu seleccionado
        menu_item *m_aux=menu_retorna_item(m,linea_seleccionada);

        //Su coordenada y original
        int orig_tabulado_y=m_aux->menu_tabulado_y;
        int orig_tabulado_x=m_aux->menu_tabulado_x;


        //Y vamos hacia arriba hasta que coordenada y sea diferente. Y si nunca es diferente y hemos recorrido todas, salir
        int contador_bucle;
        contador_bucle=0;
        do {
            //printf ("antes vert orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
            linea_seleccionada=menu_dibuja_menu_cursor_arriba(linea_seleccionada,max_opciones,m);
            m_aux=menu_retorna_item(m,linea_seleccionada);
            //printf ("despues vert orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
            contador_bucle++;
        } while (m_aux->menu_tabulado_y==orig_tabulado_y && contador_bucle<max_opciones);

        int posible_posicion=linea_seleccionada;
        int final_y=m_aux->menu_tabulado_y;

        //Y ahora buscar la que tenga misma coordenada x o mas a la derecha, si la hubiera
        contador_bucle=0;
        while (m_aux->menu_tabulado_y==final_y && m_aux->menu_tabulado_x>orig_tabulado_x && contador_bucle<max_opciones) {
            posible_posicion=linea_seleccionada;
            //printf ("antes horiz orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
            linea_seleccionada=menu_dibuja_menu_cursor_arriba(linea_seleccionada,max_opciones,m);
            m_aux=menu_retorna_item(m,linea_seleccionada);
            //printf ("despues horiz orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
            contador_bucle++;
        };

        //Si no estamos en misma posicion y, volver a posicion
        if (m_aux->menu_tabulado_y!=final_y) linea_seleccionada=posible_posicion;

    }

    //Decir que se ha pulsado tecla
    menu_speech_set_tecla_pulsada();

    return linea_seleccionada;
}


int menu_dibuja_menu_cursor_abajo_common(int linea_seleccionada,int max_opciones,menu_item *m)
{
    //Mover abajo

    if (m->es_menu_tabulado==0) linea_seleccionada=menu_dibuja_menu_cursor_abajo(linea_seleccionada,max_opciones,m);
    else linea_seleccionada=menu_dibuja_menu_cursor_abajo_tabulado(linea_seleccionada,max_opciones,m);

    return linea_seleccionada;
}


int menu_dibuja_menu_cursor_arriba_common(int linea_seleccionada,int max_opciones,menu_item *m)
{
    //Mover arriba

    if (m->es_menu_tabulado==0) linea_seleccionada=menu_dibuja_menu_cursor_arriba(linea_seleccionada,max_opciones,m);
    else linea_seleccionada=menu_dibuja_menu_cursor_arriba_tabulado(linea_seleccionada,max_opciones,m);

    return linea_seleccionada;

}



void menu_dibuja_menu_help_tooltip(char *texto, int si_tooltip)
{



        //Guardar funcion de texto overlay activo, para menus como el de visual memory por ejemplo, para desactivar temporalmente
        //Esto solo es necesario cuando no hay background windows
        //void (*previous_function)(void);

        //previous_function=menu_overlay_function;

        //restauramos modo normal de texto de menu
        //set_menu_overlay_function(normal_overlay_texto_menu);



        if (si_tooltip) {
            //menu_generic_message_tooltip("Tooltip",0,1,0,NULL,"%s",texto);
            //printf ("justo antes de message tooltip\n");
            zxvision_generic_message_tooltip("Tooltip" , 0, 0 ,0,1,0,NULL,0,0, "%s",texto);
        }

        else menu_generic_message("Help",texto);

        //Restauramos funcion anterior de overlay
        //set_menu_overlay_function(previous_function);

        /*if (zxvision_current_window!=NULL) {
            zxvision_draw_window(zxvision_current_window);
            //printf ("antes draw windows contents\n");
            zxvision_draw_window_contents(zxvision_current_window);
        }*/

        //zxvision_draw_below_windows_nospeech(zxvision_current_window);
        zxvision_redraw_all_windows();

        //printf ("antes refrescar pantalla\n");
        menu_refresca_pantalla();



}


//Indica que el menu permite repeticiones de teclas. Solo valido al pulsar hotkeys
int menu_dibuja_menu_permite_repeticiones_hotk=0;

void menu_dibuja_menu_espera_no_tecla(void)
{
    if (menu_dibuja_menu_permite_repeticiones_hotk) menu_espera_no_tecla_con_repeticion();
    else menu_espera_no_tecla();
}

int menu_calcular_ancho_string_item(char *texto)
{
    //Devuelve longitud de texto teniendo en cuenta de no sumar caracteres ~~ o ^^ o $$X
    unsigned int l;
    int ancho_calculado=strlen(texto);

    for (l=0;l<strlen(texto);l++) {
            if (menu_escribe_texto_si_inverso(texto,l)) ancho_calculado-=2;
            if (menu_escribe_texto_si_parpadeo(texto,l)) ancho_calculado-=2;
            if (menu_escribe_texto_si_cambio_tinta(texto,l)) ancho_calculado-=3;
    }

    return ancho_calculado;
}

//Para una leyenda de ventana, retornar texto corto o largo segun si cabe on no en la ventana
void menu_get_legend_short_long(char *destination_string,int ancho_visible,char *short_string,char *long_string)
{

    int longitud_largo=menu_calcular_ancho_string_item(long_string);

    //Texto mas largo cuando tenemos mas ancho

    //+2 por contar 1 espacio a la izquierda y otro a la derecha
    if (ancho_visible>=longitud_largo+2) strcpy(destination_string,long_string);


    else strcpy(destination_string,short_string);
}

//Decir si usamos hasta la ultima columna, pues no se muestra barra scroll,
//o bien se muestra barra scroll y no usamos hasta ultima columna
//Si hemos cambiado la ventana, retornar no 0
int menu_dibuja_menu_adjust_last_column(zxvision_window *w,int ancho,int alto)
{
            //Si no hay barra scroll vertical, usamos hasta la ultima columna
        int incremento_por_columna=0;
        //printf ("visible height: %d ancho %d alto %d\n",w->visible_height,ancho,alto);
        if (w->visible_height>=alto) {
            incremento_por_columna=1;
        }

        if (incremento_por_columna) {
            if (w->can_use_all_width==0) {
                //printf ("Usamos hasta la ultima columna\n");
                w->can_use_all_width=1; //Para poder usar la ultima columna de la derecha donde normalmente aparece linea scroll
                w->total_width=ancho-1+1;
                return 1;
            }
        }
        else {
            if (w->can_use_all_width) {
                //printf ("NO usamos hasta la ultima columna\n");
                w->can_use_all_width=0;
                w->total_width=ancho-1;
                return 1;
            }
        }

        /*printf ("total width: %d ancho: %d\n",ventana_menu.total_width,ancho);
        ventana_menu.total_width=10;
        printf ("total width: %d ancho: %d\n",ventana_menu.total_width,ancho);*/
        //ventana_menu.total_width=
        //printf ("total width: %d visible width %d\n",ventana_menu.total_width,ventana_menu.visible_width);
        //ventana_menu.can_use_all_width=1;  //Esto falla porque en algun momento despues se pierde este parametro

    return 0;

}

void menu_dibuja_menu_set_offset_y_common(zxvision_window *ventana,menu_item *m,int opcion_inicial)
{
        //Si menu tabulado, ajustamos scroll de zxvision
        if (m->es_menu_tabulado) {
            int linea_cursor=menu_retorna_item(m,opcion_inicial)->menu_tabulado_y;
            //printf ("ajustar scroll a %d\n",linea_cursor);
            zxvision_set_offset_y_visible(ventana,linea_cursor);
        }

        else {
            zxvision_set_offset_y_visible(ventana,opcion_inicial);
        }

}

z80_int menu_mouse_frame_counter=0;
z80_int menu_mouse_frame_counter_anterior=0;



//
//Inicio funciones de visibilidad de submenus previos
//Todas las funciones relacionadas con esto empiezan por menu_dibuja_submenu*
//

//Primer submenu visible cuando se van abriendo menus (el de mas abajo)
zxvision_window *menu_dibuja_submenu_primer_submenu=NULL;

zxvision_window *menu_dibuja_submenu_find_last_submenu(void)
{
    zxvision_window *w=menu_dibuja_submenu_primer_submenu;

    if (w==NULL) return NULL;

    while (w->submenu_next!=NULL) {
        w=w->submenu_next;
    }

    return w;
}

void menu_dibuja_submenu_get_full_path_menu(char *s)
{

    *s=0;

    zxvision_window *w=menu_dibuja_submenu_primer_submenu;

    if (w==NULL) return;

    strcpy(s,w->window_title);

    while (w->submenu_next!=NULL) {

        w=w->submenu_next;

        int longitud=strlen(s);
        sprintf(&s[longitud],"-> %s",w->window_title);

    }


}

void menu_dibuja_submenu_add_submenu(zxvision_window *w)
{

    if (menu_show_submenus_tree.v==0) return;

    zxvision_window *last_submenu=menu_dibuja_submenu_find_last_submenu();

    if (last_submenu==NULL) {
        menu_dibuja_submenu_primer_submenu=w;
    }
    else {
        last_submenu->submenu_next=w;
    }

    w->submenu_next=NULL;
    w->submenu_previous=last_submenu;
}

void menu_dibuja_submenu_cierra_n_submenus(int veces)
{

    /*if (zxvision_current_window!=NULL) {
        printf("Current window before on menu_dibuja_submenu_cierra_n_submenus: %s\n",zxvision_current_window->window_title);
        //sleep(10);
    }*/

    if (menu_show_submenus_tree.v==0) return;

    //printf("--Inicio menu_dibuja_submenu_cierra_n_submenus (%d)\n",veces);

    //Tenemos que cerrar de arriba hacia abajo
    zxvision_window *w=menu_dibuja_submenu_find_last_submenu();


    while (w!=NULL && veces>0) {
        //printf("Cerrando submenu %p [%s]\n",w,w->window_title);
        //No andar redibujando, hacerlo al final del todo
        //decir que no haga sonido de cerrar ventana porque es un menu
        zxvision_next_do_not_play_close_window_sound=1;

        zxvision_destroy_window_if_redraw(w,0);
        //printf("Despues cerrado submenu %p [%s]\n",w,w->window_title);

        zxvision_window *memory_to_free=w;
        w=w->submenu_previous;
        free(memory_to_free);

        veces--;

        if (w!=NULL) w->submenu_next=NULL;
    }

    if (w==NULL) {
        menu_dibuja_submenu_primer_submenu=NULL;
        //printf("Cerrados todos menus\n");
    }

    cls_menu_overlay();

    //Problema: aqui el current window es el primer menu del todo y cuando hace el redraw, lo envia a speech ese menu principal

    zxvision_redraw_all_windows_no_speech();

    /*if (zxvision_current_window!=NULL) {
        printf("Current window on menu_dibuja_submenu_cierra_n_submenus: %s\n",zxvision_current_window->window_title);
        //sleep(10);
    }*/

    //Lo siguiente ya lo enviara a speech
    menu_speech_reset_tecla_pulsada();

    //printf("--Fin menu_dibuja_submenu_cierra_submenu_dos_ultimos\n");
}



//liberar memoria de todos los submenus
void menu_dibuja_submenu_free_all(void)
{
    if (menu_show_submenus_tree.v==0) return;

    zxvision_window *w=menu_dibuja_submenu_primer_submenu;


    while (w!=NULL) {
        zxvision_window *next_w=w->submenu_next;
        free(w);
        w=next_w;
    }


    menu_dibuja_submenu_primer_submenu=NULL;

}

void menu_dibuja_submenu_cierra_todos_submenus(void)
{

    if (menu_show_submenus_tree.v==0) return;

    menu_dibuja_submenu_cierra_n_submenus(9999);

}

void menu_dibuja_submenu_cierra_submenu_dos_ultimos(void)
{

    if (menu_show_submenus_tree.v==0) return;

    menu_dibuja_submenu_cierra_n_submenus(2);

}



void menu_dibuja_submenu_get_menu_pos(int *xnormal,int *ynormal)
{
    //menu centrado normalmente
    //*xnormal=menu_center_x()-ancho/2;
    //*ynormal=menu_center_y()-alto/2;

    //menu a la derecha del anterior siempre que tengamos zx desktop habilitado y opcion de situar menus en zx desktop
    if (menu_show_submenus_tree.v && screen_ext_desktop_enabled && screen_ext_desktop_place_menu) {


        //es el primero
        if (menu_dibuja_submenu_primer_submenu==NULL) {

            //Al primero se le puede forzar posicion, por los botones superiores por ejemplo
            if (force_next_menu_position.v) {
                *xnormal=force_next_menu_position_x;
                *ynormal=force_next_menu_position_y;
            }

            //Posicionamos al inicio del zx desktop
            else {
                int x,y;


                int alto_boton;
                int ancho_boton;
                menu_ext_desktop_buttons_get_geometry(&ancho_boton,&alto_boton,NULL,NULL,NULL);

                //Ajustar coordenada y
                int alto_texto=menu_char_height*menu_gui_zoom*zoom_y;
                y=(alto_boton/alto_texto); //antes sumaba +1, porque? de esa manera quedaba 1 linea de separación con los botones...

                //Ajustar coordenada x
                int origen_x=menu_get_origin_x_zxdesktop_aux(1);

                //int offset_x=cual_boton*ancho_boton;
                //int ancho_texto=menu_char_width*menu_gui_zoom*zoom_x;
                x=origen_x; //+(offset_x/ancho_texto);

                *xnormal=x;
                *ynormal=y;
            }

        }

        else {
            //a la derecha del anterior

            zxvision_window *ultima_ventana=menu_dibuja_submenu_find_last_submenu();

            int x=ultima_ventana->x+ultima_ventana->visible_width;
            int y=ultima_ventana->y+ultima_ventana->submenu_linea_seleccionada+1;

            *xnormal=x;
            *ynormal=y;
        }


    }


}


int menu_dibuja_submenu_mouse_en_menus_anteriores(void)
{

    if (!si_menu_mouse_en_ventana() && mouse_left && !mouse_is_dragging && menu_show_submenus_tree.v) {

        int x,y;

        //menu_calculate_mouse_xy_absolute_interface(&x,&y);

        //printf("antes   %d,%d\n",x,y);

        x=zxvision_mouse_x;
        y=zxvision_mouse_y;

        //printf("despues %d,%d\n",x,y);


        zxvision_window *w=menu_dibuja_submenu_primer_submenu;


        while (w!=NULL) {
            if (x>=(w->x) && y>=(w->y) && x<=(w->x+w->visible_width-1) && y<=(w->y+w->visible_height-1)) return 1;

            w=w->submenu_next;
        }

    }

    return 0;



}


//
//Fin funciones de visibilidad de submenus previos
//

//Usado en el reindexado de el indice de busqueda
int menu_dibuja_menu_recorrer_menus=0;

//cuando entramos en submenu, resetear la opcion a la primera
int menu_dibuja_menu_recorrer_menus_entrado_submenu=0;

//Funcion de gestion de menu
//Entrada: opcion_inicial: puntero a opcion inicial seleccionada
//m: estructura de menu (estructura en forma de lista con punteros)
//titulo: titulo de la ventana del menu
//Nota: x,y, ancho, alto de la ventana se calculan segun el contenido de la misma

//Retorno
//valor retorno: tecla pulsada: 0 normal (ENTER), 1 ESCAPE,... MENU_RETORNO_XXXX

//opcion_inicial contiene la opcion seleccionada.
//asigna en item_seleccionado valores de: tipo_opcion, menu_funcion (debe ser una estructura ya asignada)
int menu_dibuja_menu(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo_en,char *titulo_es,char *titulo_ca)
{

    //printf ("inicio menu. menu_dibuja_submenu_primer_submenu=%p\n",menu_dibuja_submenu_primer_submenu);

    char *titulo;

    if (gui_language==GUI_LANGUAGE_SPANISH) titulo=titulo_es;
    else if (gui_language==GUI_LANGUAGE_CATALAN) titulo=titulo_ca;
    else titulo=titulo_en;


    //Se esta recreando todo el indice de busqueda y se acaba de entrar en un menu, resetear opcion a la primera
    if (menu_dibuja_menu_recorrer_menus && menu_dibuja_menu_recorrer_menus_entrado_submenu) {
        menu_dibuja_menu_recorrer_menus_entrado_submenu=0;
        *opcion_inicial=0;
    }


    //Nota: la variable de opción seleccionada (*opcion_inicial) se manipula con el puntero para poderse leer desde otros sitios
    //(ejemplo del overlay al elegir botones a acciones en ZX Desktop)
    //esteticamente queda algo raro manipular siempre (*opcion_inicial) pero es lo mas fácil para
    //poder hacer esa lectura de la variable, continuamente, desde fuera

    //desactivamos salir_todos_menus, por si viene activado de un menu anterior, por ejemplo:
    //menu->Audio->Waveform: ahi se quedaria salir_todos_menus=1
    //Y entonces al por ejemplo cambiar tipo onda en waveform, se cerraria todo
    salir_todos_menus=0;

    //no escribir letras de atajos de teclado al entrar en un menu
    menu_writing_inverse_color.v=0;

    //Si se fuerza siempre que aparezcan letras de atajos
    if (menu_force_writing_inverse_color.v) menu_writing_inverse_color.v=1;

    //Primera vez decir selected item. Luego solo el nombre del item
    menu_active_item_primera_vez=1;

    if (menu_es_stdout()) {

        //Para que se envie a speech
        //TODO: el texto se muestra dos veces en consola:
        //1- pues es un error y todos se ven en consola.
        //2- pues es una ventana de stdout y se "dibuja" tal cual en consola
        menu_muestra_pending_error_message();

        //se abre menu con driver stdout. Llamar a menu alternativo

        //si hay menu tabulado, agregamos ESC (pues no se incluye nunca)
        if (m->es_menu_tabulado) menu_add_ESC_item(m);

        return menu_dibuja_menu_stdout(opcion_inicial,item_seleccionado,m,titulo);
    }
/*
        if (if_pending_error_message) {
                if_pending_error_message=0;
                menu_generic_message("ERROR",pending_error_message);
        }
*/


    //esto lo haremos ligeramente despues menu_speech_reset_tecla_pulsada();

    if (!menu_dibuja_menu_permite_repeticiones_hotk) {
        //printf ("llamar a menu_reset_counters_tecla_repeticion desde menu_dibuja_menu al inicio\n");
        menu_reset_counters_tecla_repeticion();
    }


    //nota: parece que scr_actualiza_tablas_teclado se debe llamar en el caso de xwindows para que refresque la pantalla->seguramente viene por un evento


    int max_opciones;


    //si la anterior opcion era la final (ESC), establecemos el cursor a 0
    //if ((*opcion_inicial)<0) (*opcion_inicial)=0;

    int x,y,ancho,alto;

    menu_item *aux;

    aux=m;

    //contar el numero de opciones totales
    //calcular ancho maximo de la ventana
    int ancho_calculado=0;

    //Para permitir menus mas grandes verticalmente de lo que cabe en ventana.
    //int scroll_opciones=0;


    ancho=menu_dibuja_ventana_ret_ancho_titulo(ZXVISION_MAX_ANCHO_VENTANA,titulo);

    index_menu *indice_menu_actual;

    //Indexar siempre que no diga que no hay que indexarlo, ni tampoco menus tabulados,
    //pues menus tabulados son mas bien para ventanas, como visual memory, y no menus de opciones
    //Y siempre que indique el full path en el primer item de menu

    //Si no tiene titulo...
    if (m->index_full_path==NULL) {

        char buf_full_path[MAX_LENGTH_FULL_PATH_SUBMENU];

        //Lo deducimos teniendo en cuenta que hay submenus previos
        if (menu_show_submenus_tree.v && m->no_es_realmente_un_menu==0) {


            //Obtener titulo considerando las ventanas anteriores
            menu_dibuja_submenu_get_full_path_menu(buf_full_path);

            //Agregarle la ventana actual, que aun no esta en la lista de ventanas
            int longitud=strlen(buf_full_path);
            if (longitud==0) strcpy(buf_full_path,titulo);
            else sprintf(&buf_full_path[longitud],"-> %s",titulo);

            //generar uno aunque no sepa los subtitulos anteriores
            //sprintf(buf_full_path,"... -> %s",titulo);



            //printf("Titulo: [%s]\n",buf_full_path);
        }

        else {
            //no tenemos submenus previos. Metemos un texto generico
            sprintf(buf_full_path,"... -> %s",titulo);
        }

        m->index_full_path=buf_full_path;

    }

    if (m->no_indexar_busqueda==0 && m->es_menu_tabulado==0 && index_menu_enabled.v && m->index_full_path!=NULL) {
        indice_menu_actual=zxvision_index_entrada_menu(m->index_full_path);
    }




//printf ("despues menu_dibuja_ventana_ret_ancho_titulo\n");


    max_opciones=0;
    do {

        if (menu_item_retornar_avanzados(aux)) {

            ancho_calculado=menu_calcular_ancho_string_item(menu_retorna_item_language(aux))+2; //+2 espacios

            //Espacio para los ...
            if (aux->genera_ventana) ancho_calculado+=3;


            if (ancho_calculado>ancho) ancho=ancho_calculado;
            //printf ("%s\n",aux->texto);
            max_opciones++;

            //Para el indice de opciones de menu
            //Indexar siempre que no diga que no hay que indexarlo, ni tampoco menus tabulados,
            //pues menus tabulados son mas bien para ventanas, como visual memory, y no menus de opciones
            if (m->no_indexar_busqueda==0 && m->es_menu_tabulado==0 && index_menu_enabled.v && m->index_full_path!=NULL) {
                zxvision_index_add_menu_linea(indice_menu_actual,menu_retorna_item_language(aux));
            }

        }

        aux=aux->siguiente_item;

    } while (aux!=NULL);

    //printf ("Opciones totales: %d\n",max_opciones);

    alto=max_opciones+2;



    x=menu_center_x()-ancho/2;
    y=menu_center_y()-alto/2;

    int posicionar_submenu=0;

    if (menu_show_submenus_tree.v && m->no_es_realmente_un_menu==0 && screen_ext_desktop_enabled && screen_ext_desktop_place_menu) posicionar_submenu=1;

    if (!posicionar_submenu) {

        //Solo alterar posicion si se abre desde botones de menu por ejemplo
        if (force_next_menu_position.v) {

            x=force_next_menu_position_x;
            y=force_next_menu_position_y;

        }

    }

    else {
        //Posicionar segun politica de submenus
        menu_dibuja_submenu_get_menu_pos(&x,&y);

    }

    //Reajustar x,y por si se ha salido con los posicionados anteriores
    if (x+ancho>scr_get_menu_width()) x=scr_get_menu_width()-ancho;
    if (y+alto>scr_get_menu_height()) y=scr_get_menu_height()-alto;


    int ancho_visible=ancho;
    int alto_visible=alto;

    if (x<0 || y<0 || x+ancho>scr_get_menu_width() || y+alto>scr_get_menu_height()) {
        //char window_error_message[100];
        //sprintf(window_error_message,"Window out of bounds: x: %d y: %d ancho: %d alto: %d",x,y,ancho,alto);
        //cpu_panic(window_error_message);

        //Ajustar limites
        if (x<0) x=0;
        if (y<0) y=0;
        if (x+ancho>scr_get_menu_width()) ancho_visible=scr_get_menu_width()-x;
        if (y+alto>scr_get_menu_height()) alto_visible=scr_get_menu_height()-y;
    }

    int redibuja_ventana;
    int tecla;

    //Apuntamos a ventana usada. Si no es menu tabulado, creamos una nosotros
    //Si es tabulado, usamos current_window (pues ya alguien la ha creado antes)
    zxvision_window *ventana;
    zxvision_window ventana_menu;



    if (m->es_menu_tabulado==0) {

        if (menu_show_submenus_tree.v && m->no_es_realmente_un_menu==0) {
            //asignar espacio ventana con malloc, para no perderlo en el stack
            zxvision_window *ventana_para_submenus=util_malloc(sizeof(zxvision_window),"Can not allocate memory for submenu window");

            ventana=ventana_para_submenus;

            menu_dibuja_submenu_add_submenu(ventana);
        }

        else ventana=&ventana_menu;

        //zxvision_new_window(&ventana_menu,x,y,ancho_visible,alto_visible,
        //					ancho-1,alto-2,titulo);		 //hacer de momento igual de ancho que ancho visible para poder usar ultima columna


        //Hacer 1 mas de ancho total para poder usar columna derecha
        zxvision_new_window(ventana,x,y,ancho_visible,alto_visible,
                            ancho-1+1,alto-2,titulo);		 //hacer de momento igual de ancho que ancho visible para poder usar ultima columna



        //los menus estrictamente (que no sean cuadros de dialogo) no se pueden redimiensionar ni minimizar
        //dialogos se pueden. y menus tabulados (que entran por el else de mas abajo) también
        if (m->no_es_realmente_un_menu==0) {
            ventana->can_be_resized=0;
            ventana->can_be_minimized=0;
        }

        //Si no hay barra scroll vertical, usamos hasta la ultima columna
        menu_dibuja_menu_adjust_last_column(ventana,ancho,alto);


    }

    else {
        ventana=zxvision_current_window;
    }


    zxvision_draw_window(ventana);

    //printf ("despues de zxvision_draw_window\n");

    int salir_con_flecha_izquierda=0;

    //Entrar aqui cada vez que se dibuje otra subventana aparte, como tooltip o ayuda
    do {
        redibuja_ventana=0;

        //printf ("Entrada desde subventana aparte, como tooltip o ayuda\n");


        menu_tooltip_counter=0;


        tecla=0;

        //si la opcion seleccionada es mayor que el total de opciones, seleccionamos linea 0
        //esto pasa por ejemplo cuando activamos realvideo, dejamos el cursor por debajo, y cambiamos a zxspectrum
        //printf ("linea %d max %d\n",(*opcion_inicial),max_opciones);
        if ((*opcion_inicial)>=max_opciones) {
                debug_printf(VERBOSE_INFO,"Selected Option beyond limits. Set option to 0");
                (*opcion_inicial)=0;
        }


    //menu_retorna_item(m,(*opcion_inicial))->tipo_opcion==MENU_OPCION_SEPARADOR
    //si opcion activa es un separador (que esto pasa por ejemplo cuando activamos realvideo, dejamos el cursor por debajo, y cambiamos a zxspectrum)
    //en ese caso, seleccionamos linea 0
    if (menu_retorna_item(m,(*opcion_inicial))->tipo_opcion==MENU_OPCION_SEPARADOR) {
        debug_printf(VERBOSE_INFO,"Selected Option is a separator. Set option to 0");
        (*opcion_inicial)=0;
    }


    while (tecla!=13 && tecla!=32 && tecla!=MENU_RETORNO_ESC && tecla!=MENU_RETORNO_F1 && tecla!=MENU_RETORNO_F2 &&
        tecla!=MENU_RETORNO_F10 && tecla!=MENU_RETORNO_BACKGROUND && redibuja_ventana==0 &&
        !salir_con_flecha_izquierda && menu_tooltip_counter<TOOLTIP_SECONDS) {

        //printf ("tecla desde bucle: %d\n",tecla);

        //Ajustar scroll
        //scroll_opciones=0;
        //desactivado en zxvision , tiene su propio scroll


        //Ajustar scroll de ventana
        menu_dibuja_menu_set_offset_y_common(ventana,m,*opcion_inicial);



        menu_item *funcion_seleccionado_item=menu_retorna_item(m,(*opcion_inicial));
        if (funcion_seleccionado_item!=NULL) {
            if (funcion_seleccionado_item->menu_funcion_seleccionada!=NULL) {
                //printf("Llamando a funcion de item seleccionado\n");
                funcion_seleccionado_item->menu_funcion_seleccionada(funcion_seleccionado_item);
            }
        }


        //escribir todas opciones
        //printf ("Escribiendo de nuevo las opciones\n");
        menu_escribe_opciones_zxvision(ventana,m,(*opcion_inicial),max_opciones);



        //printf ("Linea seleccionada: %d\n",(*opcion_inicial));
        //No queremos que el speech vuelva a leer la ventana
        //menu_speech_set_tecla_pulsada();
        zxvision_draw_window_contents_no_speech(ventana);

        //printf ("despues de zxvision_draw_window_contents_no_speech\n");


        menu_refresca_pantalla();

        if (menu_dibuja_menu_recorrer_menus) {
            debug_printf(VERBOSE_INFO,"Entering menu [%s]",titulo);
            if (menu_dibuja_menu_recorrer_menus==2) {
                menu_dibuja_menu_recorrer_menus=1;
                //printf("Volvemos de una flecha izquierda. Aumentar\n");
                (*opcion_inicial)=menu_dibuja_menu_cursor_abajo_common((*opcion_inicial),max_opciones,m);
            }

            //Recorrer cada submenu
            int salir_recorrer=0;

            do {

                menu_item *item_recorrer=menu_retorna_item(m,(*opcion_inicial));

                //printf("item [%s]. *opcion_inicial %d max_opciones %d\n",item_recorrer->texto_opcion,*opcion_inicial,max_opciones);

                int item_test_submenu_activo=1;

                t_menu_funcion_activo item_test_sel_activo;
                item_test_sel_activo=item_recorrer->menu_funcion_activo;

                if (item_test_sel_activo!=NULL) {
                    if ( item_test_sel_activo()==0 ) item_test_submenu_activo=0;
                }


                if (item_recorrer->tiene_submenu && item_test_submenu_activo) {
                    //entrar a submenu con  enter
                    salir_recorrer=1;
                    tecla=13;
                    debug_printf(VERBOSE_INFO,"Entering submenu [%s]",item_recorrer->texto_opcion);
                    menu_dibuja_menu_recorrer_menus_entrado_submenu=1;

                }

                else {

                    if (*opcion_inicial>=max_opciones-1) {
                        //Salir con flecha izquierda
                        //printf("salir con flecha izquierda\n");
                        salir_recorrer=1;

                        if (m->no_es_realmente_un_menu) {
                            //printf("salir con esc\n");
                            tecla=MENU_RETORNO_ESC;
                        }

                        else tecla='5';
                        menu_dibuja_menu_recorrer_menus=2;
                    }

                    else {

                        int anterior_opcion=*opcion_inicial;
                        //printf("bajar\n");
                        (*opcion_inicial)=menu_dibuja_menu_cursor_abajo_common((*opcion_inicial),max_opciones,m);

                        //Si ha dado la vuelta
                        if (*opcion_inicial<anterior_opcion) {
                            //printf("salir con flecha izquierda\n");
                            salir_recorrer=1;

                            if (m->no_es_realmente_un_menu) {
                                //printf("salir con esc\n");
                                tecla=MENU_RETORNO_ESC;
                            }

                            else tecla='5';
                            menu_dibuja_menu_recorrer_menus=2;
                        }



                    }

                }
            } while (!salir_recorrer);



        }

        else {

        //printf ("despues de menu_refresca_pantalla\n");

        tecla=0;

        }

        //la inicializamos a 0. aunque parece que no haga falta, podria ser que el bucle siguiente
        //no se entrase (porque menu_tooltip_counter<TOOLTIP_SECONDS) y entonces tecla_leida tendria valor indefinido
        int tecla_leida=0;


        //Si se estaba escuchando speech y se pulsa una tecla, esa tecla debe entrar aqui tal cual y por tanto, no hacemos espera_no_tecla
        //temp menu_espera_no_tecla();
        if (menu_speech_tecla_pulsada==0) {
            //menu_espera_no_tecla();
            menu_dibuja_menu_espera_no_tecla();
        }
        menu_speech_reset_tecla_pulsada();

        while (tecla==0 && redibuja_ventana==0 && menu_tooltip_counter<TOOLTIP_SECONDS) {


            //Si no hay barra scroll vertical, usamos hasta la ultima columna, solo para menus no tabulados
            if (m->es_menu_tabulado==0) {
                if (menu_dibuja_menu_adjust_last_column(ventana,ancho,alto)) {
                    //printf ("Redibujar ventana pues hay cambio en columna final de scroll\n");

                    //Es conveniente llamar antes a zxvision_draw_window pues este establece parametros de ventana_ancho y alto,
                    //que se leen luego en menu_escribe_opciones_zxvision
                    //sin embargo, al llamar a menu_escribe_opciones_zxvision, el cursor sigue apareciendo como mas pequeño hasta que
                    //no se pulsa tecla
                    //printf ("ventana ancho antes: %d\n",ventana_ancho);
                    zxvision_draw_window(ventana);
                    //printf ("ventana ancho despues: %d\n",ventana_ancho);

                    //borrar contenido ventana despues de redimensionarla con espacios
                    int i;
                    for (i=0;i<ventana->total_height;i++) zxvision_print_string_defaults_fillspc(ventana,0,i,"");

                    menu_escribe_opciones_zxvision(ventana,m,(*opcion_inicial),max_opciones);

                    zxvision_draw_window_contents(ventana);
                }
            }

            //Si no hubera este menu_refresca_pantalla cuando multitask esta a off,
            //no se moverian las ventanas con refresco al mover raton
            //el resto de cosas funcionaria bien
             if (!menu_multitarea) {
                        menu_refresca_pantalla();
                }


            menu_espera_tecla_timeout_tooltip();

            //Guardamos valor de mouse_movido pues se perdera el valor al leer el teclado de nuevo
            int antes_mouse_movido=mouse_movido;

            tecla_leida=zxvision_read_keyboard();

            //printf ("Despues tecla leida: %d\n",tecla_leida);

            mouse_movido=antes_mouse_movido;

            //Para poder usar repeticiones
            if (tecla_leida==0) {
                //printf ("llamar a menu_reset_counters_tecla_repeticion desde menu_dibuja_menu cuando tecla=0\n");
                menu_reset_counters_tecla_repeticion();
            }

            else {
                //printf ("no reset counter tecla %d\n",tecla);
            }





            //printf ("mouse_movido: %d\n",mouse_movido);


            //printf ("tecla_leida: %d\n",tecla_leida);
            if (mouse_movido) {
                //printf ("mouse x: %d y: %d menu mouse x: %d y: %d\n",mouse_x,mouse_y,menu_mouse_x,menu_mouse_y);
                //printf ("ventana x %d y %d ancho %d alto %d\n",ventana_x,ventana_y,ventana_ancho,ventana_alto);
                if (si_menu_mouse_en_ventana() ) {
                //if (menu_mouse_x>=0 && menu_mouse_y>=0 && menu_mouse_x<ventana_ancho && menu_mouse_y<ventana_alto ) {
                    //printf ("dentro ventana\n");
                    //Descartar linea titulo y ultima linea

                    if (menu_mouse_y>0 && menu_mouse_y<current_win_alto-1) {
                        //printf ("dentro espacio efectivo ventana\n");
                        //Ver si hay que subir o bajar cursor
                        int posicion_raton_y=menu_mouse_y-1;

                        //tener en cuenta scroll
                        posicion_raton_y +=ventana->offset_y;

                        //Si no se selecciona separador. Menu no tabulado
                        if (m->es_menu_tabulado==0) {
                            if (menu_retorna_item(m,posicion_raton_y)->tipo_opcion!=MENU_OPCION_SEPARADOR) {
                                (*opcion_inicial)=posicion_raton_y;
                                redibuja_ventana=1;
                                menu_tooltip_counter=0;
                            }
                        }
                        else {
                            menu_item *buscar_tabulado;
                            int linea_buscada;
                            int posicion_raton_x=menu_mouse_x;
                            buscar_tabulado=menu_retorna_item_tabulado_xy(m,posicion_raton_x,posicion_raton_y,&linea_buscada);

                            if (buscar_tabulado!=NULL) {
                                //Buscar por coincidencia de coordenada x,y
                                if (buscar_tabulado->tipo_opcion!=MENU_OPCION_SEPARADOR) {
                                    (*opcion_inicial)=linea_buscada;
                                    redibuja_ventana=1;
                                    menu_tooltip_counter=0;
                                }
                            }
                            else {
                                //printf ("item no encontrado\n");
                            }
                        }

                    }
                    //else {
                    //	printf ("En espacio ventana no usable\n");
                    //}
                }
                //else {
                //	printf ("fuera ventana\n");
                //}
            }

            //mouse boton izquierdo es como enter
            int mouse_en_zona_opciones=1;

            //if (menu_mouse_x>=0 && menu_mouse_y>=0 && menu_mouse_x<ventana_ancho && menu_mouse_y<ventana_alto ) return 1;

            //Mouse en columna ultima de la derecha,
            //o mouse en primera linea
            //o mouse en ultima linea
            // no enviamos enter si pulsamos boton
            if (menu_mouse_x==current_win_ancho-1 || menu_mouse_y==0 || menu_mouse_y==current_win_alto-1) mouse_en_zona_opciones=0;

            //printf ("Despues tecla leida2: %d\n",tecla_leida);

            if (si_menu_mouse_en_ventana() && mouse_left && mouse_en_zona_opciones && !mouse_is_dragging) {
                //printf ("Enviamos enter\n");
                tecla=13;
            }

            //Si se pulsa flecha izquierda en zona de menus anteriores
            else if (menu_dibuja_submenu_mouse_en_menus_anteriores() && m->no_es_realmente_un_menu==0) {
                //printf ("Enviamos flecha izquierda porque raton en zona menus anteriores\n");
                tecla='5';
                //mouse_left=0;
                //menu_espera_no_tecla();


                //esto se ha habilitado porque se ha detectado pulsacion del raton fuera de la ventana activa
                salir_todos_menus=0;
            }

            //enviar flecha izquierda si boton derecho raton
            else if (/*si_menu_mouse_en_ventana() &&  mouse_en_zona_opciones &&*/ m->es_menu_tabulado==0 && mouse_right && !mouse_is_dragging && menu_mouse_right_send_esc.v==0) {
                //printf ("Enviamos flecha izquierda\n");
                tecla='5';
            }

            else if (tecla_leida==11) tecla='7';
            else if (tecla_leida==10) tecla='6';
            else if (tecla_leida==13) tecla=13;
            else if (tecla_leida==24) tecla=24;
            else if (tecla_leida==25) tecla=25;
            else if (tecla_leida==26) tecla=26;
            else if (tecla_leida==27) tecla=27;

            //Teclas para teclados simples
            else if (zxvision_setting_use_speccy_keys.v && tecla_leida=='5') tecla='5';
            else if (zxvision_setting_use_speccy_keys.v && tecla_leida=='6') tecla='6';
            else if (zxvision_setting_use_speccy_keys.v && tecla_leida=='7') tecla='7';
            else if (zxvision_setting_use_speccy_keys.v && tecla_leida=='8') tecla='8';


            //Teclas para menus tabulados
            else if (tecla_leida==8) tecla='5';
            else if (tecla_leida==9) tecla='8';

            else if (tecla_leida==2) {
                //tecla=2; //ESC que viene de cerrar ventana al pulsar con raton boton de cerrar en titulo
                tecla=MENU_RETORNO_ESC;
                //printf ("tecla final es ESC desde tecla_leida\n");
            }

            else if (tecla_leida==3) {
                //printf("Pressed background key on menu\n");
                tecla=MENU_RETORNO_BACKGROUND;
            }


            else if ((puerto_especial1 & 1)==0) {
                //Enter
                //printf ("Leido ESC desde puerto_especial1\n");
                tecla=MENU_RETORNO_ESC;
            }



            //En principio ya no volvemos mas con F1, dado que este se usa para ayuda contextual de cada funcion

            //F1 (ayuda) o h en drivers que no soportan F
            else if ((puerto_especial2 & 1)==0 || (tecla_leida=='h' && f_functions==0) ) {
                                //F1
                char *texto_ayuda;
                texto_ayuda=menu_retorna_item(m,(*opcion_inicial))->texto_ayuda;
                if (texto_ayuda!=NULL) {
                    //Forzar que siempre suene
                    //Esperamos antes a liberar tecla, sino lo que hara sera que esa misma tecla F1 cancelara el speech texto de ayuda
                    menu_espera_no_tecla();
                    menu_speech_reset_tecla_pulsada();


                    menu_dibuja_menu_help_tooltip(texto_ayuda,0);


                    redibuja_ventana=1;
                    menu_tooltip_counter=0;
                    //Y volver a decir "selected item"
                    menu_active_item_primera_vez=1;

                }
                        }


                        else if ((puerto_especial2 & 2)==0) {
                                //F2
                                tecla=MENU_RETORNO_F2;
                        }

                        else if ((puerto_especial3 & 16)==0) {
                                //F10
                                tecla=MENU_RETORNO_F10;
                        }


            //teclas de atajos. De momento solo admitido entre a y z
            else if ( (tecla_leida>='a' && tecla_leida<='z') || (tecla_leida>='A' && tecla_leida<='Z')) {
                debug_printf (VERBOSE_DEBUG,"Read key: %c. Possibly shortcut",tecla_leida);
                tecla=tecla_leida;
            }

            //tecla espacio. acciones adicionales. Ejemplo en breakpoints para desactivar
            else if (tecla_leida==32) {
                debug_printf (VERBOSE_DEBUG,"Pressed key space");
                tecla=32;
            }




            else {
                //printf ("Final ponemos tecla a 0. Era %d\n",tecla);
                tecla=0;
            }


            //printf ("menu tecla: %d\n",tecla);
        }

        //Si no se ha pulsado tecla de atajo:
        if (!((tecla_leida>='a' && tecla_leida<='z') || (tecla_leida>='A' && tecla_leida<='Z')) ) {
            menu_espera_no_tecla();



        }



        t_menu_funcion_activo sel_activo;

        t_menu_funcion funcion_espacio;

        if (tecla!=0) menu_tooltip_counter=0;

        //int lineas_mover_pgup_dn;
        int conta_mover_pgup_dn;
        int scroll_esperado_pgdn;
        int scroll_esperado_pgup;
        int movido_en_ventana;
        //int scroll_antes_pgdn;

        //printf ("tecla en dibuja menu: %d\n",tecla);

        //flecha derecha se comporta como enter en menus no tabulados y si tiene submenu
        if (m->es_menu_tabulado==0) {
            if (tecla=='8' && menu_old_behaviour_close_menus.v==0 && menu_retorna_item(m,(*opcion_inicial))->tiene_submenu) tecla=13;
        }

        //printf("tecla: %d\n",tecla);

        switch (tecla) {
            case 13:
                //ver si la opcion seleccionada esta activa

                sel_activo=menu_retorna_item(m,(*opcion_inicial))->menu_funcion_activo;

                if (sel_activo!=NULL) {
                            if ( sel_activo()==0 ) tecla=0;  //desactivamos seleccion
                }
                        break;


            //Mover Izquierda:
            //En tabulados, sigue el cursor
            //En no tabulados y en no dialogos, menu anterior
            case '5':

                if (m->es_menu_tabulado==0 && m->no_es_realmente_un_menu==0) {
                    if (menu_old_behaviour_close_menus.v==0) {
                        salir_con_flecha_izquierda=1;
                        //printf("salir con flecha izquierda\n");
                    }
                }

                else {

                    //en menus tabulados, misma funcion que arriba para un no tabulado
                    //Si es tabulado, seguira hasta la opcion '7'
                    (*opcion_inicial)=menu_dibuja_menu_cursor_arriba((*opcion_inicial),max_opciones,m);
                    zxvision_sound_event_cursor_movement();
                }
            break;


            //Mover Derecha, solo en tabulados
            case '8':
                //en menus tabulados, misma funcion que abajo para un no tabulado
                if (m->es_menu_tabulado==0) break;

                (*opcion_inicial)=menu_dibuja_menu_cursor_abajo((*opcion_inicial),max_opciones,m);
                zxvision_sound_event_cursor_movement();
            break;

            //Mover abajo
            case '6':
                (*opcion_inicial)=menu_dibuja_menu_cursor_abajo_common((*opcion_inicial),max_opciones,m);
                zxvision_sound_event_cursor_movement();
            break;

            //Mover arriba
            case '7':
                (*opcion_inicial)=menu_dibuja_menu_cursor_arriba_common((*opcion_inicial),max_opciones,m);
                zxvision_sound_event_cursor_movement();
            break;

/*
            //PgUp
            case 24:
                lineas_mover_pgup_dn=ventana->visible_height-3;
                //Ver si al limite de arriba
                if ((*opcion_inicial)-lineas_mover_pgup_dn<0) {
                    lineas_mover_pgup_dn=(*opcion_inicial)-1; //el -1 final es por tener en cuenta el separador de siempre
                }

                //TODO esto movera el cursor tantas lineas como lineas visibles tiene el menu,
                //si hay algun item como separador, se lo saltara, moviendo el cursor mas lineas de lo deseado
                //printf ("lineas mover: %d\n",lineas_mover_pgup_dn);
                for (conta_mover_pgup_dn=0;conta_mover_pgup_dn<lineas_mover_pgup_dn;conta_mover_pgup_dn++) (*opcion_inicial)=menu_dibuja_menu_cursor_arriba_common((*opcion_inicial),max_opciones,m);

                zxvision_sound_event_cursor_movement();

            break;
*/

            //PgUp
            //Home
            case 24:
            case 26:
                //Mover hacia arriba hasta que llega el scroll a valor esperado o se llega al maximo de opciones o se va a abajo del todo

                //calcular el cursor donde esta relativamente en ventana
                movido_en_ventana=(*opcion_inicial)-(ventana->offset_y);

                //Si el cursor esta en la mitad inferior de la ventana, hacer pgup lo que hace es llevarnos a la posicion del cursor arriba-1 posicion
                if (movido_en_ventana>(ventana->visible_height/2)) scroll_esperado_pgup=ventana->offset_y-1;
                else scroll_esperado_pgup=ventana->offset_y - (ventana->visible_height-3);
                //printf("scroll esperado: %d\n",scroll_esperado_pgup);

                //Tecla Home. hasta arriba del todo. Lo gestiono en el for

                int salir_pgup=0;

                for (conta_mover_pgup_dn=0;conta_mover_pgup_dn<max_opciones && (ventana->offset_y>scroll_esperado_pgup || tecla==26) && !salir_pgup;conta_mover_pgup_dn++) {
                    int posicion_antes_pgdn=*opcion_inicial;
                    (*opcion_inicial)=menu_dibuja_menu_cursor_arriba_common((*opcion_inicial),max_opciones,m);
                    //Ajustar scroll de ventana
                    menu_dibuja_menu_set_offset_y_common(ventana,m,*opcion_inicial);
                    //printf("mover arriba. offset_y: %d\n",ventana->offset_y);

                    //Si el cursor actual ha llegado arriba del todo y dado la vuelta
                    if (*opcion_inicial>posicion_antes_pgdn) {
                        //printf("llegado al final y dado la vuelta. compensar hacia abajo\n");
                        //subir uno hacia arriba y salir
                        (*opcion_inicial)=menu_dibuja_menu_cursor_abajo_common((*opcion_inicial),max_opciones,m);
                        salir_pgup=1;
                    }

                }

                zxvision_sound_event_cursor_movement();

            break;



            //PgDn
            //End
            case 25:
            case 27:

                //Mover hacia abajo hasta que llega el scroll a valor esperado o se llega al maximo de opciones o se va a arriba del todo

                //calcular el cursor donde esta relativamente en ventana
                movido_en_ventana=(*opcion_inicial)-(ventana->offset_y);

                //Si el cursor esta en la mitad superior de la ventana, hacer pgdn lo que hace es llevarnos a la posicion del cursor abajo+1 posicion
                if (movido_en_ventana<(ventana->visible_height/2)) scroll_esperado_pgdn=ventana->offset_y+1;
                else scroll_esperado_pgdn=ventana->offset_y + ventana->visible_height-3;


                //printf("scroll esperado: %d\n",scroll_esperado_pgdn);
                //printf("linea_seleccionada: %d offset_y: %d movido_en_ventana: %d\n",(*opcion_inicial),ventana->offset_y,movido_en_ventana);

                //Tecla End. hasta abajo del todo. Lo gestiono en el for

                int salir_pgdn=0;

                for (conta_mover_pgup_dn=0;conta_mover_pgup_dn<max_opciones && (ventana->offset_y<scroll_esperado_pgdn || tecla==27) && !salir_pgdn;conta_mover_pgup_dn++) {
                    int posicion_antes_pgdn=*opcion_inicial;
                    (*opcion_inicial)=menu_dibuja_menu_cursor_abajo_common((*opcion_inicial),max_opciones,m);
                    //Ajustar scroll de ventana
                    menu_dibuja_menu_set_offset_y_common(ventana,m,*opcion_inicial);
                    //printf("mover abajo. offset_y: %d\n",ventana->offset_y);

                    //Si el cursor actual ha llegado al final del todo y dado la vuelta
                    if (*opcion_inicial<posicion_antes_pgdn) {
                        //printf("llegado al final y dado la vuelta. compensar hacia arriba\n");
                        //subir uno hacia arriba y salir
                        (*opcion_inicial)=menu_dibuja_menu_cursor_arriba_common((*opcion_inicial),max_opciones,m);
                        salir_pgdn=1;
                    }

                }

                zxvision_sound_event_cursor_movement();

            break;


            case 32:
                //Accion para tecla espacio
                //printf ("Pulsado espacio\n");
                                //decimos que se ha pulsado Enter
                                //tecla=13;

                //Ver si tecla asociada a espacio
                funcion_espacio=menu_retorna_item(m,(*opcion_inicial))->menu_funcion_espacio;

                if (funcion_espacio==NULL) {
                    debug_printf (VERBOSE_DEBUG,"No space key function associated to this menu item");
                    tecla=0;
                }

                else {

                    debug_printf (VERBOSE_DEBUG,"Found space key function associated to this menu item");

                                    //ver si la opcion seleccionada esta activa

                                    sel_activo=menu_retorna_item(m,(*opcion_inicial))->menu_funcion_activo;

                                    if (sel_activo!=NULL) {
                                            if ( sel_activo()==0 ) {
                            tecla=0;  //desactivamos seleccion
                            debug_printf (VERBOSE_DEBUG,"Menu item is disabled");
                        }
                                    }

                }

            break;



        }

        //teclas de atajos. De momento solo admitido entre a y z
        if ( (tecla>='a' && tecla<='z') || (tecla>='A' && tecla<='Z')) {
            //printf ("buscamos atajo\n");

            int entrada_atajo;
            entrada_atajo=menu_retorna_atajo(m,tecla);


            //Encontrado atajo
            if (entrada_atajo!=-1) {
                (*opcion_inicial)=entrada_atajo;

                //Mostrar por un momento opciones y letras
                menu_writing_inverse_color.v=1;
                menu_escribe_opciones_zxvision(ventana,m,entrada_atajo,max_opciones);
                menu_refresca_pantalla();
                //menu_espera_no_tecla();
                menu_dibuja_menu_espera_no_tecla();

                //int tecla_atajo=tecla;

                //decimos que se ha pulsado Enter
                tecla=13;

                //Ver si esa opcion esta habilitada o no
                t_menu_funcion_activo sel_activo;
                sel_activo=menu_retorna_item(m,(*opcion_inicial))->menu_funcion_activo;
                if (sel_activo!=NULL) {
                    //opcion no habilitada
                    if ( sel_activo()==0 ) {
                            debug_printf (VERBOSE_DEBUG,"Shortcut found at entry number %d but entry disabled",(*opcion_inicial));
                            tecla=0;
                    }
                }

                /*if (tecla!=0) {
                    //printf("Shortcut for key %c\n",tecla_atajo);
                    zxvision_helper_menu_shortcut_print(tecla_atajo);
                }*/




            }

            else {
                debug_printf (VERBOSE_DEBUG,"No shortcut found for read key: %c",tecla);
                tecla=0;
                menu_espera_no_tecla();
            }
        }



    }

    //NOTA: contador de tooltip se incrementa desde bucle de timer, ejecutado desde cpu loop
    //Si no hay multitask de menu, NO se incrementa contador y por tanto no hay tooltip

    if (menu_tooltip_counter>=TOOLTIP_SECONDS) {

        redibuja_ventana=1;

        //Por defecto asumimos que no saltara tooltip y por tanto que no queremos que vuelva a enviar a speech la ventana
        //Aunque si que volvera a decir el "Selected item: ..." en casos que se este en una opcion sin tooltip,
        //no aparecera el tooltip pero vendra aqui con el timeout y esto hara redibujar la ventana por redibuja_ventana=1
        //si quitase ese redibujado, lo que pasaria es que no aparecerian los atajos de teclado para cada opcion
        //Entonces tal y como esta ahora:
        //Si la opcion seleccionada tiene tooltip, salta el tooltip
        //Si no tiene tooltip, no salta tooltip, pero vuelve a decir "Selected item: ..."
        menu_speech_set_tecla_pulsada();

        //Si ventana no esta activa, no mostrar tooltips,
        //porque esto hace que, por ejemplo, si el foco está en la máquina emulada, al saltar el tooltip, cambiaria el foco a la ventana de menu
        if (tooltip_enabled.v && ventana_tipo_activa) {
            char *texto_tooltip;
            texto_tooltip=menu_retorna_item(m,(*opcion_inicial))->texto_tooltip;
            if (texto_tooltip!=NULL) {
                //printf ("mostramos tooltip\n");
                //Forzar que siempre suene
                menu_speech_reset_tecla_pulsada();


                menu_dibuja_menu_help_tooltip(texto_tooltip,1);

                //printf ("despues de mostrar tooltip\n");


                //Esperar no tecla
                menu_espera_no_tecla();


                //Y volver a decir "Selected item"
                menu_active_item_primera_vez=1;


                //Y reactivar parametros ventana usados en menu_dibuja_ventana
                //zxvision_set_draw_window_parameters(ventana);

            }

            else {
                //printf ("no hay tooltip\n");

                //No queremos que se vuelva a leer cuando tooltip es inexistente. si no, estaria todo el rato releyendo la linea
                //TODO: esto no tiene efecto, sigue releyendo cuando estas sobre item que no tiene tooltip
                //menu_speech_set_tecla_pulsada();

            }

        }

        //else printf ("No mostrar tooltip\n");

        //Hay que dibujar las letras correspondientes en texto inverso
        menu_writing_inverse_color.v=1;

        menu_tooltip_counter=0;
    }

    } while (redibuja_ventana==1);

    //*opcion_inicial=(*opcion_inicial);

    //nos apuntamos valor de retorno

    //force_next_menu_position.v=0;

    menu_item *menu_sel;
    menu_sel=menu_retorna_item(m,(*opcion_inicial));

    //Si tecla espacio
    if (tecla==32) {
        item_seleccionado->menu_funcion=menu_sel->menu_funcion_espacio;
        tecla=13;
    }
    else item_seleccionado->menu_funcion=menu_sel->menu_funcion;

    item_seleccionado->tipo_opcion=menu_sel->tipo_opcion;
    item_seleccionado->valor_opcion=menu_sel->valor_opcion;
    item_seleccionado->atajo_tecla=menu_sel->atajo_tecla;
    item_seleccionado->opcion_conmuta=menu_sel->opcion_conmuta;
    strcpy(item_seleccionado->texto_opcion,menu_retorna_item_language(menu_sel));
    strcpy(item_seleccionado->texto_misc,menu_sel->texto_misc);

    int menu_se_cerrara=menu_sel->menu_se_cerrara;
    int tiene_submenu=menu_sel->tiene_submenu;
    int genera_ventana=menu_sel->genera_ventana;

    //printf ("misc selected: %s %s\n",item_seleccionado->texto_misc,menu_sel->texto_misc);

    //guardamos antes si el tipo es tabulado y otros parámetros antes de
    //liberar el item de menu, pues quedará inaccesible después de liberar
    int es_tabulado=m->es_menu_tabulado;
    int no_es_realmente_un_menu=m->no_es_realmente_un_menu;

    //Al salir de menu, reescribir las opciones pero sin que se vea cursor activo
    //Esto evita que se vea el cursor seleccionado en ventanas abiertas en segundo plano, como ay player,
    //que es confuso para el usuario tener ventanas en segundo plano con cursor activo. Solo es para esto, es por corregir
    //ese "glitch" de la ventana
    //Nota: se podria hacer esto solo para menus tabulados y no para menus normales, pero asi tengo el código uniforme
    //y funciona igual en cualquier caso de menu
    //Nota 2: mi intuicion me dice que provocaria parpadeo del cursor al pulsar enter en una opcion, porque se
    //sale y se entra en la funcion de menu, pero parece que no se ve dicho parpadeo porque no se está refrescando la ventana entre medias
    //Si se hiciera zxvision_draw_window_contents(ventana); menu_refresca_pantalla();
    //despues de la llamada a menu_dibuja_menu, si que se veria parpadeo
    //Nota 3: Para mi yo del futuro: considerar esta Nota 2 por si alguna vez en alguna ventana con menu tabulado
    //se ve parpadeo al pulsar enter en alguna opción, es debido a esto

    //No enviarlo a speech
    menu_speech_set_tecla_pulsada();
    menu_escribe_opciones_zxvision(ventana,m,-1,max_opciones);


    //Liberar memoria del menu
    aux=m;
    menu_item *nextfree;

    do {
        //printf ("Liberando %x\n",aux);
        nextfree=aux->siguiente_item;
        free(aux);
        aux=nextfree;
    } while (aux!=NULL);

    //
    // CUIDADO!!!!!!
    //
    // A partir de aqui no se pueden usar items de menu (lista m)
    // Cualquier referencia se tiene que haber hecho antes de liberar sobre otra variable, como por ejemplo
    // no_es_realmente_un_menu, que guardan el estado del primer item


    //Salir del menu diciendo que no se ha pulsado tecla
    menu_speech_reset_tecla_pulsada();

    ventana->submenu_linea_seleccionada=*opcion_inicial;

    //En caso de menus tabulados, es responsabilidad de este de borrar con cls y liberar ventana
    if (es_tabulado==0) {

        //Decir que no haga el sonido de cierre de ventana, ese sonido
        //solo deberia ser cuando son ventanas no de menu
        //printf("decir no sonido al cerrar. se pone zxvision_next_do_not_play_close_window_sound a 1\n");
        zxvision_next_do_not_play_close_window_sound=1;

        //si hay setting de submenu, no siempre se destruye ventana
        if (menu_show_submenus_tree.v && no_es_realmente_un_menu==0) {
            if (salir_con_flecha_izquierda ||

                //con antiguo comportamiento, al pulsar ESC o seleccionar item esc, se comporta igual que salir con flecha izquierda
                (  (tecla==MENU_RETORNO_ESC || ((item_seleccionado->tipo_opcion)&MENU_OPCION_ESC)  ) && menu_old_behaviour_close_menus.v)

            ) {
                //Quitamos el actual, y el anterior tambien, dado que el anterior se va a redibujar
                menu_dibuja_submenu_cierra_submenu_dos_ultimos();
            }

            else {
                int cerrar_ventanas=0;

                //if (tiene_submenu==0) cerrar_ventanas=1;

                if (menu_se_cerrara) cerrar_ventanas=1;
                if (genera_ventana) cerrar_ventanas=1;
                //Si pulsamos F6 desde menú principal, queremos que se cierren también, no que se vaya duplicando el menú principal
                if (tecla==MENU_RETORNO_BACKGROUND) cerrar_ventanas=1;


                if (cerrar_ventanas) menu_dibuja_submenu_cierra_todos_submenus();

                else {
                    if (tiene_submenu==0) {
                        //Si no tiene submenu ni genera ventana, asumimos que se recargara el menu (es un setting que conmuta)
                        //por tanto borrar solo esta ventana actual
                        menu_dibuja_submenu_cierra_n_submenus(1);
                    }
                }
            }

        }
        else {
            zxvision_destroy_window(ventana);
        }
    }

    //printf ("tecla al salir de dibuja menu: %d\n",tecla);

    //Parecido a ESC, pero no cerramos todos menus
    if (salir_con_flecha_izquierda) {
        //printf("Return con flecha izquierda\n");
        zxvision_helper_menu_shortcut_delete_last();

        return MENU_RETORNO_ESC;
    }

    //Se deja cierre pendiente para la ventana que se abre, y al cerrarla se cerraran los menus
    if (menu_se_cerrara && menu_old_behaviour_close_menus.v==0) {
        salir_todos_menus=1;
        //printf("pendiente para cerrar todos menus\n");
    }

    //Para el helper shortcut
    int tecla_atajo=item_seleccionado->atajo_tecla;
    if (tecla_atajo==0) tecla_atajo='?';

    int ya_borrado_helper_atras=0;

    //Se selecciona opcion de ESC
    if ((item_seleccionado->tipo_opcion)&MENU_OPCION_ESC) {
        //hay que cerrar todos menus
        if (menu_old_behaviour_close_menus.v==0) {
            salir_todos_menus=1;
            //printf("Seleccionado opcion ESC\n");
            //no indicar helper
            tecla_atajo=0;
        }
        else {
            //Comportamiento antiguo. ESC nos lleva hacia atras
            zxvision_helper_menu_shortcut_delete_last();

            ya_borrado_helper_atras=1;

            //no indicar helper
            tecla_atajo=0;
        }
    }

    //Se pulsa tecla ESC
    if (tecla==MENU_RETORNO_ESC) {
        //printf("tecla ESC\n");
        //printf("salir con ESC. menu_dibuja_submenu_primer_submenu=%p\n",menu_dibuja_submenu_primer_submenu);
        //Cerrar todos menus
        if (menu_old_behaviour_close_menus.v==0) {
            salir_todos_menus=1;
        }

        //Comportamiento antiguo. ESC nos lleva hacia atras
        else {
            //Aqui podria suceder que pulsamos ESC en la propia opcion de ESC
            //Solo borrar en total una letra
            if (!ya_borrado_helper_atras) {
                zxvision_helper_menu_shortcut_delete_last();
            }
        }

        return MENU_RETORNO_ESC;
    }

    else if (tecla==MENU_RETORNO_F1) return MENU_RETORNO_F1;
    else if (tecla==MENU_RETORNO_F2) return MENU_RETORNO_F2;
    else if (tecla==MENU_RETORNO_F10) return MENU_RETORNO_F10;
    else if (tecla==MENU_RETORNO_BACKGROUND) return MENU_RETORNO_BACKGROUND;

    else {
        //printf("ultimo else. menu_dibuja_submenu_primer_submenu=%p\n",menu_dibuja_submenu_primer_submenu);
        if (tecla_atajo) zxvision_helper_menu_shortcut_print(tecla_atajo);

        //innecesario con nuevo metodo de indexacion if (es_one_time) zxvision_index_delete_last_submenu_path();

        //Si se gestiona la conmutacion de opcion directamente desde aqui
        if (item_seleccionado->opcion_conmuta!=NULL) {
            //printf("Conmutar variable directamente desde menu\n");
            item_seleccionado->opcion_conmuta->v ^=1;
        }

        return MENU_RETORNO_NORMAL;
    }

}

//Para dibujar menu con titulo de menu igual en los 3 idiomas
int menu_dibuja_menu_no_title_lang(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo)
{
    return menu_dibuja_menu(opcion_inicial,item_seleccionado,m,titulo,titulo,titulo);
}

//Igual que menu_dibuja_menu pero cuando no queremos que este indexado, por ejemplo para ventana de Tracks List
//TODO: quiza cambiar el nombre de esta funcion a algo que diga que aun usando funcion de menu, no es un menu como tal,
//sino que nos beneficiamos de la funcion de menu para mostrar una ventana con opciones en lineas
//Función no usada
/*
int menu_dibuja_menu_no_indexado(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo)
{
    m->no_indexar_busqueda=1;

    return menu_dibuja_menu_no_title_lang(opcion_inicial,item_seleccionado,m,titulo);
}
*/

//con titulo de menu igual en los 3 idiomas
int menu_dibuja_menu_dialogo_no_title_lang(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo)
{
    m->no_es_realmente_un_menu=1;
    m->no_indexar_busqueda=1;

    return menu_dibuja_menu_no_title_lang(opcion_inicial,item_seleccionado,m,titulo);
}

//Igual que menu_dibuja_menu pero este menu se selecciona enter una vez o escape una vez, pero la funcion que la llamada no vuelve
//a dibujar el menu, por tanto a nivel de path de indexacion de busqueda, se pierde el ultimo path de submenu
//Ejemplo de esto: menu_simple_ten_choices
//Función no usada
/*
int menu_dibuja_menu_one_time(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo)
{
    m->one_time=1;

    return menu_dibuja_menu_no_title_lang(opcion_inicial,item_seleccionado,m,titulo);
}
*/

void menu_add_item_menu_common_defaults(menu_item *m,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo)
{
    ////

    //Texto en español vacio por defecto
    m->texto_opcion_spanish[0]=0;

    //Texto en catalan vacio por defecto
    m->texto_opcion_catalan[0]=0;

    //Prefijo vacio por defecto
    m->texto_opcion_prefijo[0]=0;

    //Sufijo vacio por defecto
    m->texto_opcion_sufijo[0]=0;

    m->tipo_opcion=tipo_opcion;
    m->menu_funcion=menu_funcion;
    m->menu_funcion_activo=menu_funcion_activo;
    m->texto_ayuda=NULL;
    m->texto_tooltip=NULL;

    m->index_full_path=NULL;

    //Por defecto inicializado a ""
    m->texto_misc[0]=0;

    m->atajo_tecla=0;
    m->tiene_submenu=0;
    m->genera_ventana=0;
    m->item_avanzado=0;
    m->item_sencillo=0;
    m->no_indexar_busqueda=0;
    m->one_time=0;
    m->opcion_marcada=0;
    m->opcion_campo_seleccionable=0;
    m->menu_se_cerrara=0;
    m->opcion_conmuta=NULL;

    m->menu_funcion_espacio=NULL;

    m->menu_funcion_seleccionada=NULL;

    m->siguiente_item=NULL;

    ////
}


//Agregar el item inicial del menu
//Parametros: puntero al puntero de menu_item inicial. texto
void menu_add_item_menu_inicial(menu_item **p,char *texto,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo)
{

    menu_item *m;

    m=malloc(sizeof(menu_item));

    //printf ("%d\n",sizeof(menu_item));

        if (m==NULL) cpu_panic("Cannot allocate initial menu item");

    //comprobacion de maximo
    if (strlen(texto)>MAX_TEXTO_OPCION) cpu_panic ("Text item greater than maximum");

    //m->texto=texto;
    strcpy(m->texto_opcion,texto);

    ////

    menu_add_item_menu_common_defaults(m,tipo_opcion,menu_funcion,menu_funcion_activo);

    ////

    m->es_menu_tabulado=0; //por defecto no es menu tabulado. esta opcion se hereda en cada item, desde el primero

    m->no_es_realmente_un_menu=0; //por defecto se comporta como un menu. Esto se altera por ejemplo en menus (cuadros de dialogo)

    *p=m;
}

//Agregar un item al menu
//Parametros: puntero de menu_item inicial. texto
void menu_add_item_menu(menu_item *m,char *texto,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo)
{
    //busca el ultimo item i le añade el indicado. O hasta que encuentre uno con MENU_OPCION_UNASSIGNED, que tendera a ser el ultimo

    while (m->siguiente_item!=NULL && m->tipo_opcion!=MENU_OPCION_UNASSIGNED)
    {
        m=m->siguiente_item;
    }

    menu_item *next;

    int era_un_assigned_no_indexar_busqueda=0;

    if (m->tipo_opcion==MENU_OPCION_UNASSIGNED) {
        debug_printf (VERBOSE_DEBUG,"Overwrite last item menu because it was MENU_OPCION_UNASSIGNED");
        next=m;
        era_un_assigned_no_indexar_busqueda=m->no_indexar_busqueda;
    }

    else {

        next=malloc(sizeof(menu_item));
        //printf ("%d\n",sizeof(menu_item));

        if (next==NULL) cpu_panic("Cannot allocate menu item");

        m->siguiente_item=next;
    }


    //Si era menu tabulado. Heredamos la opcion. Aunque se debe establecer la x,y luego para cada item, lo mantenemos asi para que cada item,
    //tengan ese parametro
    int es_menu_tabulado;
    es_menu_tabulado=m->es_menu_tabulado;

    //comprobacion de maximo
    if (strlen(texto)>MAX_TEXTO_OPCION) cpu_panic ("Text item greater than maximum");

    //next->texto=texto;
    strcpy(next->texto_opcion,texto);

    ////

    menu_add_item_menu_common_defaults(next,tipo_opcion,menu_funcion,menu_funcion_activo);

    ////

    next->es_menu_tabulado=es_menu_tabulado;
    if (era_un_assigned_no_indexar_busqueda) next->no_indexar_busqueda=era_un_assigned_no_indexar_busqueda;
}

//Agregar un item separador
void menu_add_item_menu_separator(menu_item *m)
{
    menu_add_item_menu(m,"",MENU_OPCION_SEPARADOR,NULL,NULL);
}

//Agregar ayuda al ultimo item de menu
void menu_add_item_menu_ayuda(menu_item *m,char *texto_ayuda)
{
       //busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

    m->texto_ayuda=texto_ayuda;
}

//Agregar tooltip al ultimo item de menu
void menu_add_item_menu_tooltip(menu_item *m,char *texto_tooltip)
{
       //busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

        m->texto_tooltip=texto_tooltip;
}

//Agregar atajo de tecla al ultimo item de menu
void menu_add_item_menu_shortcut(menu_item *m,z80_byte tecla)
{
       //busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

        m->atajo_tecla=tecla;
}

//Agregar decirle que tiene submenu al ultimo item de menu
void menu_add_item_menu_tiene_submenu(menu_item *m)
{
       //busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

        m->tiene_submenu=1;
}

//Agregar flags al ultimo item de menu
void menu_add_item_menu_add_flags(menu_item *m,int flags)
{

    //Nos situamos ya en el último item para que las llamadas a los diferentes flags sean mas rápidas
    while (m->siguiente_item!=NULL)
    {
        m=m->siguiente_item;
    }

    if ( (flags & MENU_ITEM_FLAG_TIENE_SUBMENU)) menu_add_item_menu_tiene_submenu(m);
    if ( (flags & MENU_ITEM_FLAG_GENERA_VENTANA)) menu_add_item_menu_genera_ventana(m);
    if ( (flags & MENU_ITEM_FLAG_ES_AVANZADO)) menu_add_item_menu_es_avanzado(m);
    if ( (flags & MENU_ITEM_FLAG_SE_CERRARA)) menu_add_item_menu_se_cerrara(m);

}

//Agregar decirle que tiene opcion que conmuta, en el item de menu actual
void menu_add_item_menu_opcion_conmuta(menu_item *m,z80_bit *opcion)
{
       //busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

        m->opcion_conmuta=opcion;
}

//Agregar el nombre de full path de indexado
void menu_add_item_menu_index_full_path(menu_item *m,char *index_name_en,char *index_name_es,char *index_name_ca)
{
    //Esto lo agrega tal cual al primer item, por tanto podemos llamarlo desde cualquier momento de la creacion del menu

    char *titulo;

    if (gui_language==GUI_LANGUAGE_SPANISH) titulo=index_name_es;
    else if (gui_language==GUI_LANGUAGE_CATALAN) titulo=index_name_ca;
    else titulo=index_name_en;


    m->index_full_path=titulo;
}

//Agregar decirle que tiene genera ventana al ultimo item de menu
void menu_add_item_menu_genera_ventana(menu_item *m)
{
       //busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

        m->genera_ventana=1;
}

//Decirle que todos los menus se cerraran cuando se cierre el menu que dispara esta accion
void menu_add_item_menu_se_cerrara(menu_item *m)
{
       //busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

        m->menu_se_cerrara=1;
}

void menu_add_item_menu_es_avanzado(menu_item *m)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    m->item_avanzado=1;
}

void menu_add_item_menu_es_sencillo(menu_item *m)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    m->item_sencillo=1;
}

//Indicar que ese menu no se debe indexar la busqueda. Indicarlo en el primer item siempre
void menu_add_item_menu_no_indexar_busqueda(menu_item *m)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    m->no_indexar_busqueda=1;
}

void menu_add_item_menu_marcar_opcion(menu_item *m,int valor)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    m->opcion_marcada=valor;
}

void menu_add_item_menu_campo_seleccionable(menu_item *m)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    m->opcion_campo_seleccionable=1;
}

//Retorna linea del item actual
int menu_item_get_linea_actual(menu_item *m)
{

    int linea=0;

    while (m->siguiente_item!=NULL)
    {
        m=m->siguiente_item;
        linea++;
    }

    return linea;
}

//Agregar decirle que es un item avanzado al ultimo item de menu
/*
void old_menu_add_item_menu_es_avanzado(menu_item *m)
{

    menu_item *inicial;

    inicial=m;

    menu_item *previo;
    previo=NULL;

    //busca el ultimo item. si no tenemos el modo avanzado, quitarlo

    while (m->siguiente_item!=NULL)
    {
        previo=m;
        m=m->siguiente_item;
    }

    if (menu_show_advanced_items.v==0) {
        //Quitarlo, siempre que no seamos el item inicial
        //printf("Quitar item ultimo pues no estamos en modo avanzado\n");
        if (previo!=NULL) {
            //printf("Quitado item ultimo\n");
            //Liberar este ultimo
            free(m);
            //Indicar al previo que no hay siguiente
            previo->siguiente_item=NULL;
        }
        else {
            debug_printf(VERBOSE_DEBUG,"Can not remove advanced item because it is the first one");
        }
    }

}
*/

char *menu_text_string_sure_spanish="Seguro?";
char *menu_text_string_sure_catalan="Segur?";
char *menu_text_string_sure_english="Sure?";

char *menu_text_string_autoframeskip_spanish="Auto Saltar Frames";
char *menu_text_string_autoframeskip_catalan="Auto Saltar Frames";
char *menu_text_string_autoframeskip_english="Auto Frameskip";

char *menu_text_string_enabled_spanish="Activado";
char *menu_text_string_enabled_catalan="Activat";
char *menu_text_string_enabled_english="Enabled";

char *menu_text_string_welcome_spanish="Bienvenido";
char *menu_text_string_welcome_catalan="Benvingut";
char *menu_text_string_welcome_english="Welcome";

//char *menu_text_string_select_manufacturer_spanish="Selecciona fabricante";
//char *menu_text_string_select_manufacturer_catalan="Selecciona fabricant";
//char *menu_text_string_select_manufacturer_english="Select manufacturer";


//Funcion para devolver ciertas strings de ingles a español
//Nota: funcion no optimizada, no usar para muchas strings o sera muy lento
char *menu_get_string_language(char *texto)
{
    if (!strcmp(texto,"Sure?")) {
        if (gui_language==GUI_LANGUAGE_SPANISH) return menu_text_string_sure_spanish;
        else if (gui_language==GUI_LANGUAGE_CATALAN) return menu_text_string_sure_catalan;
        else return menu_text_string_sure_english;
    }

    else if (!strcmp(texto,"Auto Frameskip")) {
        if (gui_language==GUI_LANGUAGE_SPANISH) return menu_text_string_autoframeskip_spanish;
        else if (gui_language==GUI_LANGUAGE_CATALAN) return menu_text_string_autoframeskip_catalan;
        else return menu_text_string_autoframeskip_english;
    }

    else if (!strcmp(texto,"Enabled")) {
        if (gui_language==GUI_LANGUAGE_SPANISH) return menu_text_string_enabled_spanish;
        else if (gui_language==GUI_LANGUAGE_CATALAN) return menu_text_string_enabled_catalan;
        else return menu_text_string_enabled_english;
    }

    else if (!strcmp(texto,"Welcome")) {
        if (gui_language==GUI_LANGUAGE_SPANISH) return menu_text_string_welcome_spanish;
        else if (gui_language==GUI_LANGUAGE_CATALAN) return menu_text_string_welcome_catalan;
        else return menu_text_string_welcome_english;
    }

    /*else if (!strcmp(texto,"Select manufacturer")) {
        if (gui_language==GUI_LANGUAGE_SPANISH) return menu_text_string_select_manufacturer_spanish;
        else if (gui_language==GUI_LANGUAGE_CATALAN) return menu_text_string_select_manufacturer_catalan;
        else return menu_text_string_select_manufacturer_english;
    }*/

    else return texto;
}


//Agregar texto de item menu en spanish
void menu_add_item_menu_spanish(menu_item *m,char *s)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    strcpy(m->texto_opcion_spanish,s);

}

//Agregar texto de item menu en spanish
//Parametros: puntero de menu_item inicial. texto con formato
void menu_add_item_menu_spanish_format(menu_item *m,const char * format , ...)
{
    char buffer[100];
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    va_end (args);

    menu_add_item_menu_spanish(m,buffer);
}


//Agregar texto de item menu en catalan
void menu_add_item_menu_catalan(menu_item *m,char *s)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    strcpy(m->texto_opcion_catalan,s);

}

//Agregar texto de item menu en catalan
//Parametros: puntero de menu_item inicial. texto con formato
void menu_add_item_menu_catalan_format(menu_item *m,const char * format , ...)
{
    char buffer[100];
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    va_end (args);

    menu_add_item_menu_catalan(m,buffer);
}

//Insertar los dos desde una misma funcion
void menu_add_item_menu_spanish_catalan(menu_item *m,char *spanish,char *catalan)
{
    menu_add_item_menu_spanish(m,spanish);
    menu_add_item_menu_catalan(m,catalan);
}

//Agregar texto de prefijo de item menu
void menu_add_item_menu_prefijo(menu_item *m,char *s)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    strcpy(m->texto_opcion_prefijo,s);

}

//Agregar texto de prefijo de item menu
//Parametros: puntero de menu_item inicial. texto con formato
void menu_add_item_menu_prefijo_format(menu_item *m,const char * format , ...)
{
    char buffer[100];
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    va_end (args);

    menu_add_item_menu_prefijo(m,buffer);
}


//Agregar texto de sufijo de item menu
void menu_add_item_menu_sufijo(menu_item *m,char *s)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    strcpy(m->texto_opcion_sufijo,s);

}

//Agregar texto de sufijo de item menu
//Parametros: puntero de menu_item inicial. texto con formato
void menu_add_item_menu_sufijo_format(menu_item *m,const char * format , ...)
{
    char buffer[100];
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    va_end (args);

    menu_add_item_menu_sufijo(m,buffer);
}

//Agregar funcion de gestion de tecla espacio
void menu_add_item_menu_espacio(menu_item *m,t_menu_funcion menu_funcion_espacio)
{
//busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

        m->menu_funcion_espacio=menu_funcion_espacio;
}


//Agregar funcion de gestion de salto a funcion cuando se selecciona un item con el cursor (sin tener que pulsar Enter)
void menu_add_item_menu_seleccionado(menu_item *m,void (*menu_funcion_seleccionada)(struct s_menu_item *))
{
//busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

        m->menu_funcion_seleccionada=menu_funcion_seleccionada;
}


//Indicar que es menu tabulado. Se hace para todos los items, dado que establece coordenada x,y
void menu_add_item_menu_tabulado(menu_item *m,int x,int y)
{
//busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

        m->es_menu_tabulado=1;
    m->menu_tabulado_x=x;
    m->menu_tabulado_y=y;
}




//Agregar un valor como opcion al ultimo item de menu
//Esto sirve, por ejemplo, para que cuando esta en el menu de z88, insertar slot,
//se pueda saber que slot se ha seleccionado
void menu_add_item_menu_valor_opcion(menu_item *m,int valor_opcion)
{
       //busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

    //printf ("temp. agregar valor opcion %d\n",valor_opcion);

        m->valor_opcion=valor_opcion;
}


//Agregar texto misc al ultimo item de menu
//Esto sirve, por ejemplo, para guardar url en navegador online
void menu_add_item_menu_misc(menu_item *m,char *texto_misc)
{
       //busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }



        strcpy(m->texto_misc,texto_misc);

        //printf ("agregado texto misc %s\n",m->texto_misc);
}


//Agregar un item al menu
//Parametros: puntero de menu_item inicial. texto con formato
void menu_add_item_menu_format(menu_item *m,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,const char * format , ...)
{
    char buffer[100];
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    va_end (args);

    menu_add_item_menu(m,buffer,tipo_opcion,menu_funcion,menu_funcion_activo);
}

//Agregar item de menu en inglés, castellano y catalán
void menu_add_item_menu_en_es_ca(menu_item *m,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,
    char *english,char *spanish,char *catalan)
{
    menu_add_item_menu(m,english,tipo_opcion,menu_funcion,menu_funcion_activo);
    menu_add_item_menu_spanish_catalan(m,spanish,catalan);
}


void menu_add_item_menu_en_es_ca_inicial(menu_item **m,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,
char *english,char *spanish,char *catalan)
{
    menu_add_item_menu_inicial(m,english,tipo_opcion,menu_funcion,menu_funcion_activo);
    menu_add_item_menu_spanish_catalan(*m,spanish,catalan);
}


//Agregar el item inicial del menu
//Parametros: puntero al puntero de menu_item inicial. texto con formato
void menu_add_item_menu_inicial_format(menu_item **p,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,const char * format , ...)
{
        char buffer[100];
        va_list args;
        va_start (args, format);
        vsprintf (buffer,format, args);
    va_end (args);

        menu_add_item_menu_inicial(p,buffer,tipo_opcion,menu_funcion,menu_funcion_activo);

}

char *string_esc_closes_menus="ESC closes all menus";

//Agrega item de ESC normalmente.  En caso de aalib y consola es con tecla TAB
void menu_add_ESC_item(menu_item *array_menu_item)
{

        char mensaje_esc[32];

        char *accion_esc="Cerrar";
        char *accio_esc="Tancar";
        char *action_esc="Close";

        //Si antiguo comportamiento o si driver stdout
        //pues el menu de stdout no tiene el comportamiento de "cerrar todos menus" que se activa con menu_add_item_menu_se_cerrara()
        if (menu_old_behaviour_close_menus.v || menu_es_stdout() ) {
            accion_esc="Volver";
            accio_esc="Tornar";
            action_esc="Back";
        }

        if (gui_language==GUI_LANGUAGE_SPANISH) sprintf (mensaje_esc,"%s %s",esc_key_message,accion_esc);
        else if (gui_language==GUI_LANGUAGE_CATALAN) sprintf (mensaje_esc,"%s %s",esc_key_message,accio_esc);
        else sprintf (mensaje_esc,"%s %s",esc_key_message,action_esc);

        menu_add_item_menu(array_menu_item,mensaje_esc,MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
        menu_add_item_menu_tooltip(array_menu_item,string_esc_closes_menus);
        menu_add_item_menu_ayuda(array_menu_item,string_esc_closes_menus);

}





/*int menu_cond_spectrum(void)
{
    return (MACHINE_IS_SPECTRUM);
    //return !menu_cond_zx8081();
}*/







//			//Hacer decaer el volumen
//			if (menu_waveform_previous_volume>menu_audio_draw_sound_wave_volumen_escalado) menu_waveform_previous_volume--;

//Funcion usada por los vu-meters para hacer el efecto de "decae" del maximo
//Retorna valor de variable de decae, segun el ultimo valor del volumen
int menu_decae_dec_valor_volumen(int valor_decae,int valor_volumen)
{
    //Hacer decaer el volumen
    if (valor_decae>valor_volumen) valor_decae--;

    return valor_decae;
}


//	//Volume. Mostrarlo siempre, no solo dos veces por segundo, para que se actualice mas frecuentemente
//	if (menu_waveform_previous_volume<menu_audio_draw_sound_wave_volumen_escalado) menu_waveform_previous_volume=menu_audio_draw_sound_wave_volumen_escalado;

//Funcion usada por los vu-meters cada vez que se quieren mostrar, ver si valor de variable decae es menor que volumen,
//entonces modificarla

int menu_decae_ajusta_valor_volumen(int valor_decae,int valor_volumen)
{
    if (valor_decae<valor_volumen) valor_decae=valor_volumen;

    return valor_decae;

}

//llena el string con el valor del volumen - para chip de sonido
//mete tambien caracter de "decae" si conviene (si >=0 y <=15)
void menu_string_volumen(char *texto,z80_byte registro_volumen,int indice_decae)
{
    if ( (registro_volumen & 16)!=0) sprintf (texto,"ENV            ");
    else {
        registro_volumen=registro_volumen & 15;
        int i;
        int destino;
        int indicado_rojo=0;



        for (i=0,destino=0;i<registro_volumen;i++) {
            texto[destino++]='=';


            //Codigo control color tinta.
            if (i==11) {
                texto[destino++]='$';
                texto[destino++]='$';
                texto[destino++]='0'+ESTILO_GUI_COLOR_AVISO; //'2';
                indicado_rojo=1;
            }
        }

        for (;i<15;i++) {
            texto[destino++]=' ';
        }

        texto[destino]=0;

        //Si indice es menor que volumen, forzar a valor que volumen
        if (indice_decae<registro_volumen) indice_decae=registro_volumen;

        if (indice_decae>=0 && indice_decae<=14 && indice_decae>=registro_volumen) texto[indice_decae+indicado_rojo*3]='>';

        //printf ("registro volumen: %d indice decae: %d pos decae: %d\n",registro_volumen,indice_decae,indice_decae+indicado_rojo*3);
    }
}


//llena el string con el valor del volumen
//mete tambien caracter de "decae" si conviene (si >=0 y <=longitud)
void menu_string_volumen_n_longitud(char *texto,z80_byte registro_volumen,int indice_decae,int longitud)
{

    if (registro_volumen>longitud) registro_volumen=longitud;
    int i;
    int destino;
    int indicado_rojo=0;



    for (i=0,destino=0;i<registro_volumen;i++) {
        texto[destino++]='=';


        //Codigo control color tinta.
        if (i==longitud-4) {
            texto[destino++]='$';
            texto[destino++]='$';
            texto[destino++]='0'+ESTILO_GUI_COLOR_AVISO;
            indicado_rojo=1;
        }
    }

    for (;i<longitud;i++) {
        texto[destino++]=' ';
    }

    texto[destino]=0;

    //Si indice es menor que volumen, forzar a valor que volumen
    if (indice_decae<registro_volumen) indice_decae=registro_volumen;

    if (indice_decae>=0 && indice_decae<=longitud-1 && indice_decae>=registro_volumen) texto[indice_decae+indicado_rojo*3]='>';

    //printf ("registro volumen: %d indice decae: %d pos decae: %d\n",registro_volumen,indice_decae,indice_decae+indicado_rojo*3);

}


//llama a menu_string_volumen gestionando maximos. Retorna el valor escalado entre 0 y 15, para poder asignarlo como valor previo
int menu_string_volumen_maxmin(char *texto,int valor_actual,int valor_previo,int valor_maximo)
{



    int barra_volumen;

    //Gestionar divisiones por cero y valores negativos y limites
    if (valor_maximo<=0 || valor_actual<0 || valor_actual>valor_maximo) {
        barra_volumen=15;
    }

    else {
        barra_volumen=(valor_actual*15)/valor_maximo;
    }

    //char buf_volumen_canal[32];
    menu_string_volumen(texto,barra_volumen,valor_previo);

    return barra_volumen;

}






/*
Antigua funcion de linea vertical
void menu_linea_zxvision(zxvision_window *ventana,int x,int y1,int y2,int color)
{

    int yorigen;
    int ydestino;


    //empezamos de menos a mas
    if (y1<y2) {
        yorigen=y1;
        ydestino=y2;
    }

    else {
        yorigen=y2;
        ydestino=y1;
    }


    for (;yorigen<=ydestino;yorigen++) {
        zxvision_putpixel(ventana,x,yorigen,color);
    }
}
*/



/*
int menu_inves_cond_realvideo(void)
{
    if (!menu_inves_cond()) return 0;
    return rainbow_enabled.v;

}
*/




#ifndef NETWORKING_DISABLED
//Nota: realmente deberiamos mirar que hay soporte de threads, en vez de que haya networking
//lo que sucede es que si no hay threads (no existe USE_PTHREADS), tampoco hay networking (definira NETWORKING_DISABLED), por tanto esto seria coherente
//El unico problema que podria haber es que se quiera usar estas ventanas de descompresion de zip en algun sitio que no venga
//de una descarga por red, pero actualmente solo se usa esta rutina al descargar por red, y por tanto, si no hay red, no hay descarga
//que se pueda ejecutar y por tanto no se usara la rutina de descompresion de zip


// Para el thread de descompresion de zip
int menu_uncompress_zip_progress_thread_running=0;
pthread_t menu_uncompress_zip_progress_thread;
int contador_menu_uncompress_zip_progress_print=0;


int menu_uncompress_zip_progress_cond(zxvision_window *w GCC_UNUSED)
{
        return !menu_uncompress_zip_progress_thread_running;
}


struct menu_uncompress_zip_progress_struct {
    char archivo_zip[PATH_MAX];
    char directorio_destino[PATH_MAX];
};

void *menu_uncompress_zip_progress_thread_function(void *entrada)
{
    debug_printf (VERBOSE_DEBUG,"Starting menu_uncompress_zip_progress_thread");

    char *archivo_zip;
    char *directorio_destino;


    archivo_zip=((struct menu_uncompress_zip_progress_struct *)entrada)->archivo_zip;
    directorio_destino=((struct menu_uncompress_zip_progress_struct *)entrada)->directorio_destino;

    debug_printf (VERBOSE_DEBUG,"Uncompressing %s to %s directory",archivo_zip,directorio_destino);

    util_extract_zip(archivo_zip,directorio_destino);

    debug_printf (VERBOSE_DEBUG,"Finishing menu_uncompress_zip_progress_thread");
    menu_uncompress_zip_progress_thread_running=0;

    return 0;

}


void menu_uncompress_zip_progress_print(zxvision_window *w)
{
        char *mensaje="|/-\\";

        int max=strlen(mensaje);
        char mensaje_dest[32];

        int pos=contador_menu_uncompress_zip_progress_print % max;

        sprintf(mensaje_dest,"Uncompressing %c",mensaje[pos]);

        zxvision_print_string_defaults_fillspc(w,1,0,mensaje_dest);
        zxvision_draw_window_contents(w);

        contador_menu_uncompress_zip_progress_print++;

}


struct menu_uncompress_zip_progress_struct menu_uncompress_zip_progress_parametros;

//Funcion para descomprimir con ventana de progreso y pthread aparte de descompresion
void menu_uncompress_zip_progress(char *zip_file,char *dest_dir)
{
        //Thread aparte para descomprimir. Necesario en caso de imagen de 2 gb que tarda mucho
        //Inicializar thread
        debug_printf (VERBOSE_DEBUG,"Initializing thread menu_uncompress_zip_progress_thread");


        //Lanzar el thread de descarga

        //Todo parametro a un thread debe estar en una memoria estatica, nada de stack ni punteros a variables de stack
        strcpy(menu_uncompress_zip_progress_parametros.archivo_zip,zip_file);
        strcpy(menu_uncompress_zip_progress_parametros.directorio_destino,dest_dir);


        //Antes de lanzarlo, decir que se ejecuta, por si el usuario le da enter rapido a la ventana de progreso y el thread aun no se ha lanzado
        menu_uncompress_zip_progress_thread_running=1;

        if (pthread_create( &menu_uncompress_zip_progress_thread, NULL, &menu_uncompress_zip_progress_thread_function,
            (void *)&menu_uncompress_zip_progress_parametros) ) {
                debug_printf(VERBOSE_ERR,"Can not create menu_uncompress_zip_progress_thread thread");
                return;
        }

        //y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
        pthread_detach(menu_uncompress_zip_progress_thread);

        contador_menu_uncompress_zip_progress_print=0;
        zxvision_simple_progress_window("Uncompressing", menu_uncompress_zip_progress_cond,menu_uncompress_zip_progress_print );

        if (menu_uncompress_zip_progress_thread_running) {

            //Al parecer despues de ventana de zxvision_simple_progress_window no se espera a liberar tecla
            menu_espera_no_tecla();
            menu_warn_message("Uncompression has not ended yet");
        }
}

#else

void menu_uncompress_zip_progress(char *zip_file,char *dest_dir)
{
    menu_error_message("Feature not available on non-pthreads version");
}


#endif
// Fin del thread de descompresion de zip












/*
void menu_testeo_scanf_numero(MENU_ITEM_PARAMETERS)
{

        char string_zoom[3];
    int temp_zoom;


    //comprobaciones previas para no petar el sprintf
    if (zoom_x>9 || zoom_x<1) zoom_x=1;

        sprintf (string_zoom,"%d",zoom_x);


        int retorno=menu_ventana_scanf_numero("Number test",string_zoom,3,+2,0,9,0);
        if (retorno<0) {
            menu_warn_message("Pulsado ESC");
        }
        else {

            temp_zoom=parse_string_to_number(string_zoom);


            menu_generic_message_format("Test","Value %d",temp_zoom);
        }

}
*/






void menu_dibuja_rectangulo_relleno(zxvision_window *w,int x, int y, int ancho, int alto, int color)
{
    int x1,y1;

    for (y1=y;y1<y+alto;y1++) {
        for (x1=x;x1<=x+ancho;x1++) {
            zxvision_putpixel(w,x1,y1,color);
        }
    }
}




void menu_warn_message(char *texto)
{
    menu_generic_message_warn("Warning",texto);

}

void menu_error_message(char *texto)
{
    zxvision_sound_event_error_menu();
    menu_generic_message_warn("ERROR",texto);

}

void menu_error_message_format(const char * format , ...)
{

    char buffer_final[4096];

    va_list args;
    va_start (args, format);
    vsprintf (buffer_final,format, args);
    va_end (args);

    menu_error_message(buffer_final);

}

//Similar a snprintf
void menu_generic_message_aux_copia(char *origen,char *destino, int longitud)
{
    while (longitud) {
        *destino=*origen;
        origen++;
        destino++;
        longitud--;
    }
}

//Aplicar filtros para caracteres extranyos y cortar linea en saltos de linea
int menu_generic_message_aux_filter(char *texto,int inicio, int final)
{
    //int copia_inicio=inicio;

    unsigned char caracter;

    int prefijo_utf=0;

        while (inicio!=final) {
        caracter=texto[inicio];

                if (caracter=='\n' || caracter=='\r') {
            //printf ("detectado salto de linea en posicion %d\n",inicio);
            texto[inicio]=' ';
            return inicio+1;
        }

        //TAB. Lo cambiamos por espacio
        else if (caracter==9) {
            texto[inicio]=' ';
        }

        else if (menu_es_prefijo_utf(caracter)) {
            //Si era prefijo utf, saltar
            prefijo_utf=1;
        }

        //Y si venia de prefijo utf, saltar ese caracter
        else if (prefijo_utf) {
            prefijo_utf=0;
        }

        //Caracter 255 significa "transparente"
        else if ( !(si_valid_char(caracter)) && caracter!=255 ) {
            //printf ("detectado caracter extranyo %d en posicion %d\n",caracter,inicio);

            texto[inicio]='?';
        }

                inicio++;
        }

        return final;

}


//Cortar las lineas, si se puede, por espacio entre palabras
int menu_generic_message_aux_wordwrap(char *texto,int inicio, int final)
{

    int copia_final=final;

    //ya acaba en espacio, volver
    //if (texto[final]==' ') return final;

    while (final!=inicio) {
        if (texto[final]==' ' || texto[final]=='\n' || texto[final]=='\r') return final+1;
        final--;
    }

    return copia_final;
}

int menu_generic_message_cursor_arriba(int primera_linea)
{
    if (primera_linea>0) primera_linea--;
    return primera_linea;
}

int menu_generic_message_cursor_arriba_mostrar_cursor(int primera_linea,int mostrar_cursor,int *linea_cursor)
{
                                     if (mostrar_cursor) {
                                                        int off=0;
                                                        //no limitar primera linea if (primera_linea) off++;
                                                        if (*linea_cursor>off) (*linea_cursor)--;
                                                        else primera_linea=menu_generic_message_cursor_arriba(primera_linea);
                                                }
                                                else {
                                                        primera_linea=menu_generic_message_cursor_arriba(primera_linea);
                                                }

    return primera_linea;
}



int menu_generic_message_cursor_abajo (int primera_linea,int alto_ventana,int indice_linea)
{


    //if (primera_linea<indice_linea-2) primera_linea++;
    if (primera_linea+alto_ventana-2<indice_linea) primera_linea++;
    return primera_linea;


}


int menu_generic_message_cursor_abajo_mostrar_cursor(int primera_linea,int alto_ventana,int indice_linea,int mostrar_cursor,int *linea_cursor)
{
                                                if (mostrar_cursor) {
                                                        if (*linea_cursor<alto_ventana-3) (*linea_cursor)++;
                                                        else primera_linea=menu_generic_message_cursor_abajo(primera_linea,alto_ventana,indice_linea);
                                                }
                                                else {
                                                        primera_linea=menu_generic_message_cursor_abajo(primera_linea,alto_ventana,indice_linea);
                                                }

    return primera_linea;
}


//int menu_generic_message_final_abajo(int primera_linea,int alto_ventana,int indice_linea,int mostrar_cursor,int linea_cursor)
int menu_generic_message_final_abajo(int primera_linea,int alto_ventana,int indice_linea)
{
    /*if (mostrar_cursor) {
        if (linea_cursor<alto_ventana-3) return 1;
    }

    else*/ if (primera_linea+alto_ventana-2<indice_linea) return 1;

    return 0;
}


//dibuja ventana simple, una sola linea de texto interior, sin esperar tecla
/*
void menu_simple_ventana(char *titulo,char *texto)
{


    unsigned int ancho_ventana=strlen(titulo);
    if (strlen(texto)>ancho_ventana) ancho_ventana=strlen(texto);

    int alto_ventana=3;

    ancho_ventana +=2;

    //(unsigned int) para evitar el warning al compilar de: comparison of integers of different signs: 'unsigned int' and 'int' [-Wsign-compare]
    if (ancho_ventana>(unsigned int)ZXVISION_MAX_ANCHO_VENTANA) {
        cpu_panic("window width too big");
    }

        int xventana=menu_center_x()-ancho_ventana/2;
        int yventana=menu_center_y()-alto_ventana/2;


        menu_dibuja_ventana(xventana,yventana,ancho_ventana,alto_ventana,titulo);

    menu_escribe_linea_opcion(0,-1,1,texto);

}
*/

void menu_copy_clipboard(char *texto)
{

    //Si puntero no NULL, liberamos clipboard anterior
    if (menu_clipboard_pointer!=NULL) {
        debug_printf(VERBOSE_INFO,"Freeing previous clipboard memory");
        free(menu_clipboard_pointer);
        menu_clipboard_pointer=NULL;
    }

    //Si puntero NULL, asignamos memoria
    if (menu_clipboard_pointer==NULL) {
        menu_clipboard_size=strlen(texto);
        debug_printf(VERBOSE_INFO,"Allocating %d bytes to clipboard",menu_clipboard_size+1);
        menu_clipboard_pointer=malloc(menu_clipboard_size+1); //+1 del 0 final
        if (menu_clipboard_pointer==NULL) {
            debug_printf(VERBOSE_ERR,"Error allocating clipboard memory");
            return;
        }
        strcpy((char *)menu_clipboard_pointer,texto);
    }


}

void menu_paste_clipboard_to_file(char *destination_file)
{
    //util_file_save(destination_file,menu_clipboard_pointer,menu_clipboard_size);
    //extern void util_file_save(char *filename,z80_byte *puntero, long long int tamanyo);
    //extern void util_save_file(z80_byte *origin, long long int tamanyo_origen, char *destination_file);

    util_save_file(menu_clipboard_pointer,menu_clipboard_size,destination_file);
}

void zxvision_copy_contents_to_clipboard(zxvision_window *ventana)
{
    int total_width=ventana->total_width;
    int total_height=ventana->total_height;

    //Calcular memoria necesaria
    int buffer_size=total_width*total_height;
    //Sumarle los saltos de linea de cada linea
    buffer_size +=total_height;

    //Y el 0 del final
    buffer_size++;

    char *texto_destino=util_malloc(buffer_size,"Can not allocate to copy windows content");

    char *puntero_destino=texto_destino;
    overlay_screen *puntero_origen=ventana->memory;

    int x,y;


    for (y=0;y<total_height;y++) {
        for (x=0;x<total_width;x++,puntero_origen++,puntero_destino++) {
            char caracter=puntero_origen->caracter;

            //Filtrar caracteres extraños
            if (caracter<32 || caracter>126) caracter='?';

            *puntero_destino=caracter;
        }
        *puntero_destino='\n';

        puntero_destino++;
    }

    *puntero_destino=0;



    menu_copy_clipboard(texto_destino);

    free(texto_destino);


}


//Funcion generica para guardar un archivo de texto a disco
//Supondra que es de texto y por tanto pone filtro de "*.txt"
//Ademas el tamaño del archivo a guardar se determina por el caracter 0 final
//de momento funcion no usada
/*void menu_save_text_to_file(char *puntero_memoria,char *titulo_ventana)
{
    char file_save[PATH_MAX];

    char *filtros[2];

    filtros[0]="txt";
    filtros[1]=0;

    int ret;

    ret=menu_filesel(titulo_ventana,filtros,file_save);

    if (ret==1) {

        //Ver si archivo existe y preguntar
        if (si_existe_archivo(file_save)) {

            if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

        }

        int file_size=strlen(puntero_memoria);

        util_save_file((z80_byte *)puntero_memoria,file_size,file_save);

        menu_generic_message_splash(titulo_ventana,"OK File saved");

        menu_espera_no_tecla();


    }
}*/


//Cortar linea en dos, pero teniendo en cuenta que solo puede cortar por los espacios
void menu_util_cut_line_at_spaces(int posicion_corte, char *texto,char *linea1, char *linea2)
{

    int indice_texto=0;
    int ultimo_indice_texto=0;
    int longitud=strlen(texto);

    indice_texto+=posicion_corte;


        //Si longitud es menor
        if (indice_texto>=longitud) {
            strcpy(linea1,texto);
            linea2[0]=0;
            return;
        }


        //Si no, miramos si hay que separar por espacios
        else indice_texto=menu_generic_message_aux_wordwrap(texto,ultimo_indice_texto,indice_texto);

        //Separamos por salto de linea, filtramos caracteres extranyos
        //indice_texto=menu_generic_message_aux_filter(texto,ultimo_indice_texto,indice_texto);


        //snprintf(buffer_lineas[indice_linea],longitud_texto,&texto[ultimo_indice_texto]);
        //printf ("indice texto: %d\n",indice_texto);

        menu_generic_message_aux_copia(texto,linea1,indice_texto);
        linea1[indice_texto]=0;

        //copiar texto
        int longitud_texto=longitud-indice_texto;

        //printf ("indice texto: %d longitud: %d\n",indice_texto,longitud_texto);

        menu_generic_message_aux_copia(&texto[indice_texto],linea2,longitud_texto);
        linea2[longitud_texto]=0;

}

void menu_ventana_draw_perc_bar_aux(zxvision_window *w,int x,int y,z80_byte caracter,int tinta,int papel)
{
    //Ver si caracter final tiene ventana por encima
    int ventana_encima=zxvision_coords_in_superior_windows(w,x,y);

    if (!ventana_encima) {
        putchar_menu_overlay(x,y,caracter,tinta,papel);
    }

}

//estilo_invertido:
//0: no invertir colores
//1: invertir color boton arriba
//2: invertir color boton abajo
//3: invertir color barra
void menu_ventana_draw_horizontal_perc_bar(zxvision_window *w,int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido)
{
        if (porcentaje<0) porcentaje=0;
        if (porcentaje>100) porcentaje=100;

        // mostrar * abajo para indicar donde estamos en porcentaje
        int xbase=x+2;

        int tinta_boton_arriba=ESTILO_GUI_TINTA_NORMAL;
        int tinta_boton_abajo=ESTILO_GUI_TINTA_NORMAL;
        int tinta_barra=ESTILO_GUI_TINTA_NORMAL;

        int papel_boton_arriba=ESTILO_GUI_PAPEL_NORMAL;
        int papel_boton_abajo=ESTILO_GUI_PAPEL_NORMAL;
        int papel_barra=ESTILO_GUI_PAPEL_NORMAL;

        int tinta_aux;

        switch (estilo_invertido) {
            case 1:
                tinta_aux=tinta_boton_arriba;
                tinta_boton_arriba=papel_boton_arriba;
                papel_boton_arriba=tinta_aux;
            break;

            case 2:
                tinta_aux=tinta_boton_abajo;
                tinta_boton_abajo=papel_boton_abajo;
                papel_boton_abajo=tinta_aux;
            break;

            case 3:
                tinta_aux=tinta_barra;
                tinta_barra=papel_barra;
                papel_barra=tinta_aux;
            break;

        }


            //mostrar cursores izquierda y derecha
        menu_ventana_draw_perc_bar_aux(w,xbase-1,y+alto-1,zxvision_retorna_caracter_flecha_izquierda(),tinta_boton_arriba,papel_boton_arriba);
        menu_ventana_draw_perc_bar_aux(w,xbase+ancho-3,y+alto-1,zxvision_retorna_caracter_flecha_derecha(),tinta_boton_abajo,papel_boton_abajo);

        //mostrar linea horizontal para indicar que es zona de porcentaje
        z80_byte caracter_barra='-';
        if (menu_hide_vertical_percentaje_bar.v) caracter_barra=' ';

        int i;
        for (i=0;i<ancho-3;i++) menu_ventana_draw_perc_bar_aux(w,xbase+i,y+alto-1,caracter_barra,tinta_barra,papel_barra);


        int sumarancho=((ancho-4)*porcentaje)/100;

        menu_ventana_draw_perc_bar_aux(w,xbase+sumarancho,y+alto-1,'*',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
}



//estilo_invertido:
//0: no invertir colores
//1: invertir color boton arriba
//2: invertir color boton abajo
//3: invertir color barra
void menu_ventana_draw_vertical_perc_bar(zxvision_window *w,int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido)
{
        if (porcentaje<0) porcentaje=0;
        if (porcentaje>100) porcentaje=100;

        // mostrar * a la derecha para indicar donde estamos en porcentaje
        int ybase=y+2;

        int tinta_boton_arriba=ESTILO_GUI_TINTA_NORMAL;
        int tinta_boton_abajo=ESTILO_GUI_TINTA_NORMAL;
        int tinta_barra=ESTILO_GUI_TINTA_NORMAL;

        int papel_boton_arriba=ESTILO_GUI_PAPEL_NORMAL;
        int papel_boton_abajo=ESTILO_GUI_PAPEL_NORMAL;
        int papel_barra=ESTILO_GUI_PAPEL_NORMAL;

        int tinta_aux;

        switch (estilo_invertido) {
            case 1:
                tinta_aux=tinta_boton_arriba;
                tinta_boton_arriba=papel_boton_arriba;
                papel_boton_arriba=tinta_aux;
            break;

            case 2:
                tinta_aux=tinta_boton_abajo;
                tinta_boton_abajo=papel_boton_abajo;
                papel_boton_abajo=tinta_aux;
            break;

            case 3:
                tinta_aux=tinta_barra;
                tinta_barra=papel_barra;
                papel_barra=tinta_aux;
            break;

        }


        //mostrar cursores arriba y abajo
        //putchar_menu_overlay(x+ancho-1,ybase-1,'^',tinta_boton_arriba,papel_boton_arriba);
        menu_ventana_draw_perc_bar_aux(w,x+ancho-1,ybase-1,zxvision_retorna_caracter_flecha_arriba(),tinta_boton_arriba,papel_boton_arriba);

        //putchar_menu_overlay(x+ancho-1,ybase+alto-3,'v',tinta_boton_abajo,papel_boton_abajo);
        menu_ventana_draw_perc_bar_aux(w,x+ancho-1,ybase+alto-3,zxvision_retorna_caracter_flecha_abajo(),tinta_boton_abajo,papel_boton_abajo);

        //mostrar linea vertical para indicar que es zona de porcentaje
        z80_byte caracter_barra='|';
        if (menu_hide_vertical_percentaje_bar.v) caracter_barra=' ';

        //mostrar linea vertical para indicar que es zona de porcentaje
        int i;
        for (i=0;i<alto-3;i++) 	menu_ventana_draw_perc_bar_aux(w,x+ancho-1,ybase+i,caracter_barra,tinta_barra,papel_barra);


        int sumaralto=((alto-4)*porcentaje)/100;
        menu_ventana_draw_perc_bar_aux(w,x+ancho-1,ybase+sumaralto,'*',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
}



int splash_zesarux_logo_paso=0;
int splash_zesarux_logo_active=0;

void reset_splash_zesarux_logo(void)
{
    splash_zesarux_logo_active=0;
}




//Almacenar la estructura de ventana aqui para que se pueda referenciar desde otros sitios
zxvision_window zxvision_window_splash_text;

void screen_print_splash_text_by_window(int lineas)
{


    zxvision_window *ventana;
    ventana=&zxvision_window_splash_text;


    //Si ya existe, cerrarla esa ventana
    if (zxvision_if_window_already_exists(ventana)) {
        debug_printf(VERBOSE_INFO,"Splash window already exists. Closing it and generating a new one");
        zxvision_destroy_window(ventana);
    }



    int xventana,yventana,ancho_ventana,alto_ventana;


    //printf("lineas: %d\n",lineas);

    ancho_ventana=33;
    alto_ventana=lineas+2;

    xventana=menu_center_x()-ancho_ventana/2;
    yventana=menu_center_y()-alto_ventana/2;

    int is_minimized=0;
    int is_maximized=0;
    int ancho_antes_minimize=ancho_ventana;
    int alto_antes_minimize=alto_ventana;


    menu_speech_set_tecla_pulsada(); //no anunciar por speech la creacion de esta ventana

    zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"Splash",
        "splashwindow",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);


    zxvision_draw_window(ventana);



}


//Esta rutina estaba originalmente en screen.c pero dado que se ha modificado para usar rutinas auxiliares de aqui, mejor que este aqui
void screen_print_splash_text(int y,int tinta,int papel,char *texto)
{

    //Si no hay driver video
    if (scr_putpixel==NULL || scr_putpixel_zoom==NULL) return;


    if (menu_abierto==0 && screen_show_splash_texts.v==1) {

        //Si background windows y even when menu closed
        //Temporalmente siempre lo hacemos asi
        int mostrar_splash_con_ventana=0;

        if (menu_allow_background_windows && menu_multitarea && always_force_overlay_visible_when_menu_closed) mostrar_splash_con_ventana=1;


        int x;

        #define MAX_LINEAS_SPLASH 24
        const int max_ancho_texto=31;
        //al trocear, si hay un espacio despues, se agrega, y por tanto puede haber linea de 31+1=32 caracteres

        //texto que contiene cada linea con ajuste de palabra. Al trocear las lineas aumentan
        //33 es ancho total linea(32)+1
        char buffer_lineas[MAX_LINEAS_SPLASH][33];



        int indice_linea=0;
        int indice_texto=0;
        int ultimo_indice_texto=0;
        int longitud=strlen(texto);

        //int indice_segunda_linea;


        do {
            indice_texto+=max_ancho_texto;

            //Controlar final de texto
            if (indice_texto>=longitud) indice_texto=longitud;

            //Si no, miramos si hay que separar por espacios
            else indice_texto=menu_generic_message_aux_wordwrap(texto,ultimo_indice_texto,indice_texto);

            //Separamos por salto de linea, filtramos caracteres extranyos
            indice_texto=menu_generic_message_aux_filter(texto,ultimo_indice_texto,indice_texto);

            //copiar texto
            int longitud_texto=indice_texto-ultimo_indice_texto;



            menu_generic_message_aux_copia(&texto[ultimo_indice_texto],buffer_lineas[indice_linea],longitud_texto);
            buffer_lineas[indice_linea++][longitud_texto]=0;
            //printf ("copiado %d caracteres desde %d hasta %d: %s\n",longitud_texto,ultimo_indice_texto,indice_texto,buffer_lineas[indice_linea-1]);


            //printf ("texto indice: %d : longitud: %d: -%s-\n",indice_linea-1,longitud_texto,buffer_lineas[indice_linea-1]);
            //printf ("indice_linea: %d indice_linea+y: %d MAX: %d\n",indice_linea,indice_linea+y,MAX_LINEAS_SPLASH);

            if (indice_linea==MAX_LINEAS_SPLASH) {
                    //cpu_panic("Max lines on menu_generic_message reached");
                    debug_printf(VERBOSE_INFO,"Max lines on screen_print_splash_text reached (%d)",MAX_LINEAS_SPLASH);
                    //finalizamos bucle
                    indice_texto=longitud;
            }

            ultimo_indice_texto=indice_texto;
            //printf ("ultimo indice: %d %c\n",ultimo_indice_texto,texto[ultimo_indice_texto]);

        } while (indice_texto<longitud);

        if (mostrar_splash_con_ventana) {
            screen_print_splash_text_by_window(indice_linea);
        }

        else {

            cls_menu_overlay();

        }

        int i;
        for (i=0;i<indice_linea && y<scr_get_menu_height();i++) {
            int longitud_linea=strlen(buffer_lineas[i]);
            debug_printf (VERBOSE_DEBUG,"line %d y: %d length: %d contents: -%s-",i,y,longitud_linea,buffer_lineas[i]);


            if (mostrar_splash_con_ventana) {
                x=(zxvision_window_splash_text.total_width)/2-strlen(buffer_lineas[i])/2;
                if (x<1) x=1;
                //printf("ZXvision print %d,%d : %s\n",x,i,buffer_lineas[i]);
                zxvision_print_string_defaults_fillspc(&zxvision_window_splash_text,x,i,buffer_lineas[i]);
            }
            else {
                x=menu_center_x()-strlen(buffer_lineas[i])/2;
                if (x<0) x=0;
                menu_escribe_texto(x,y,tinta,papel,buffer_lineas[i]);
            }
            y++;
        }

        if (mostrar_splash_con_ventana) {
            menu_speech_set_tecla_pulsada(); //Si no, envia continuamente todo ese texto a speech

            zxvision_draw_window_contents(&zxvision_window_splash_text);
        }
        else {
            set_menu_overlay_function(normal_overlay_texto_menu);
        }


        menu_splash_text_active.v=1;
        menu_splash_segundos=5;

        //no queremos que reaparezca el logo, por si no había llegado al final de splash. Improbable? Si. Pero mejor ser precavidos
        reset_splash_zesarux_logo();
   }

}



//Esta rutina estaba originalmente en screen.c pero dado que se ha modificado para usar rutinas auxiliares de aqui, mejor que este aqui
void screen_print_splash_text_center(int tinta,int papel,char *texto)
{
    screen_print_splash_text(menu_center_y(),tinta,papel,texto);
}

//No muestra mensaje si hay otro previo activo
//Se usa para evitar mostrar continuamente mensajes y saturar la cpu
//por ejemplo en casos de demos y juegos que continuamente cambian de modo de video y hacen texto splash por ejemplo
void screen_print_splash_text_center_no_if_previous(int tinta,int papel,char *texto)
{
    if (menu_splash_text_active.v) return;


    screen_print_splash_text_center(tinta,papel,texto);
}

//retorna 1 si y
//otra cosa, 0
//texto_adicional y funcion_texto_adicional sirven para agregar una opcion, debajo del si/no:
// texto_adicional: funcion que retorna el texto a mostrar, debe ser un puntero a algo fuera del stack
// funcion_trigger_texto_adicional: funcion que se ejecuta al darle al enter en dicho item adicional
//texto_adicional2 y funcion_trigger_texto_adicional2: lo mismo pero para un segundo item
//Si no se quiere adicional, pasarlas como NULL
int menu_confirm_yesno_texto_additional_item(char *texto_ventana,char *texto_interior,
    char *(*texto_adicional)(void),void (*funcion_trigger_texto_adicional) (void),
    char *(*texto_adicional2)(void),void (*funcion_trigger_texto_adicional2) (void)
)
{

    //conservar estado de salir todos menus antes de invocar funcion de menu
    //por si esta activo, permitir que se cierren los menus despues de invocar a la funcion de pregunta yes/no
    int antes_salir_todos_menus=salir_todos_menus;

    //Si se fuerza siempre yes
    if (force_confirm_yes.v) return 1;


    //printf ("confirm\n");

    //En caso de stdout, es mas simple, mostrar texto y esperar tecla
    if (menu_es_stdout()) {
        char buffer_texto[256];
        printf ("%s\n%s\n",texto_ventana,texto_interior);

        scrstdout_menu_print_speech_macro(texto_ventana);
        scrstdout_menu_print_speech_macro(texto_interior);

        fflush(stdout);
        scanf("%s",buffer_texto);
        if (buffer_texto[0]=='y' || buffer_texto[0]=='Y') return 1;
        return 0;
    }




    menu_espera_no_tecla();


    menu_item *array_menu_confirm_yes_no;
    menu_item item_seleccionado;
    int retorno_menu;

    //Siempre indicamos el NO
    int confirm_yes_no_opcion_seleccionada=2;
    do {

        menu_add_item_menu_inicial_format(&array_menu_confirm_yes_no,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);
        menu_add_item_menu_es_sencillo(array_menu_confirm_yes_no);


        if (gui_language==GUI_LANGUAGE_SPANISH || gui_language==GUI_LANGUAGE_CATALAN) {
            menu_add_item_menu_format(array_menu_confirm_yes_no,MENU_OPCION_NORMAL,NULL,NULL,"~~Si");
            menu_add_item_menu_shortcut(array_menu_confirm_yes_no,'s');
            menu_add_item_menu_es_sencillo(array_menu_confirm_yes_no);
        }

        else {
            menu_add_item_menu_format(array_menu_confirm_yes_no,MENU_OPCION_NORMAL,NULL,NULL,"~~Yes");
            menu_add_item_menu_shortcut(array_menu_confirm_yes_no,'y');
            menu_add_item_menu_es_sencillo(array_menu_confirm_yes_no);
        }

        menu_add_item_menu_format(array_menu_confirm_yes_no,MENU_OPCION_NORMAL,NULL,NULL,"~~No");
        menu_add_item_menu_shortcut(array_menu_confirm_yes_no,'n');
        menu_add_item_menu_es_sencillo(array_menu_confirm_yes_no);

        //separador adicional para que quede mas grande la ventana y mas mono
        menu_add_item_menu_format(array_menu_confirm_yes_no,MENU_OPCION_SEPARADOR,NULL,NULL," ");
        menu_add_item_menu_es_sencillo(array_menu_confirm_yes_no);


        if (texto_adicional!=NULL) {
            menu_add_item_menu_format(array_menu_confirm_yes_no,MENU_OPCION_NORMAL,NULL,NULL,texto_adicional() );
            menu_add_item_menu_es_sencillo(array_menu_confirm_yes_no);
        }

        if (texto_adicional2!=NULL) {
            menu_add_item_menu_format(array_menu_confirm_yes_no,MENU_OPCION_NORMAL,NULL,NULL,texto_adicional2() );
            menu_add_item_menu_es_sencillo(array_menu_confirm_yes_no);
        }


        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&confirm_yes_no_opcion_seleccionada,&item_seleccionado,array_menu_confirm_yes_no,texto_ventana);



        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (funcion_trigger_texto_adicional!=NULL && confirm_yes_no_opcion_seleccionada==4) {
                 funcion_trigger_texto_adicional();
            }

            else if (funcion_trigger_texto_adicional2!=NULL && confirm_yes_no_opcion_seleccionada==5) {
                 funcion_trigger_texto_adicional2();
            }

            else {

                salir_todos_menus=antes_salir_todos_menus;

                //Al volver de esta manera, hay que indicar al index_search que se "va atras" un menu
                //Esto ya se llama por defecto en gestion de menu, cuando se pulsa ESC o flecha atras,
                //pero en este caso, se sale con la aceptacion de la opcion, y no es ni ESC ni flecha atras
                //innecesario con nuevo metodo de indexacion zxvision_index_delete_last_submenu_path();

                if (confirm_yes_no_opcion_seleccionada==1) return 1;
                else return 0;

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    salir_todos_menus=antes_salir_todos_menus;

    return 0;


}


//retorna 1 si y
//otra cosa, 0
int menu_confirm_yesno_texto(char *texto_ventana,char *texto_interior)
{
    return menu_confirm_yesno_texto_additional_item(texto_ventana,texto_interior,NULL,NULL,NULL,NULL);
}


//Retorna 0=Cancel, 1=Append, 2=Truncate, 3=Rotate
int menu_ask_no_append_truncate_texto(char *texto_ventana,char *texto_interior,int allow_rotate)
{


        menu_espera_no_tecla();


    menu_item *array_menu_ask_no_append_truncate;
        menu_item item_seleccionado;
        int retorno_menu;

    //Siempre indicamos el Cancel
    int ask_no_append_truncate_opcion_seleccionada=1;
    do {

        menu_add_item_menu_inicial_format(&array_menu_ask_no_append_truncate,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

        menu_add_item_menu_format(array_menu_ask_no_append_truncate,MENU_OPCION_NORMAL,NULL,NULL,"~~Cancel");
        menu_add_item_menu_shortcut(array_menu_ask_no_append_truncate,'c');
        menu_add_item_menu_tooltip(array_menu_ask_no_append_truncate,"Cancel operation and don't set file");
        menu_add_item_menu_ayuda(array_menu_ask_no_append_truncate,"Cancel operation and don't set file");

        menu_add_item_menu_format(array_menu_ask_no_append_truncate,MENU_OPCION_NORMAL,NULL,NULL,"~~Append");
        menu_add_item_menu_shortcut(array_menu_ask_no_append_truncate,'a');
        menu_add_item_menu_tooltip(array_menu_ask_no_append_truncate,"Open the selected file in append mode");
        menu_add_item_menu_ayuda(array_menu_ask_no_append_truncate,"Open the selected file in append mode");

        menu_add_item_menu_format(array_menu_ask_no_append_truncate,MENU_OPCION_NORMAL,NULL,NULL,"~~Truncate");
        menu_add_item_menu_shortcut(array_menu_ask_no_append_truncate,'t');
        menu_add_item_menu_tooltip(array_menu_ask_no_append_truncate,"Truncates selected file to 0 size");
        menu_add_item_menu_ayuda(array_menu_ask_no_append_truncate,"Truncates selected file to 0 size");


        if (allow_rotate) {
        menu_add_item_menu_format(array_menu_ask_no_append_truncate,MENU_OPCION_NORMAL,NULL,NULL,"~~Rotate");
        menu_add_item_menu_shortcut(array_menu_ask_no_append_truncate,'r');
        menu_add_item_menu_tooltip(array_menu_ask_no_append_truncate,"Rotate selected file to keep history files");
        menu_add_item_menu_ayuda(array_menu_ask_no_append_truncate,"Rename selected file adding extension suffix .1. \n"
            "If that file also exists, the extension suffix is renamed to .2. \n"
            "If that file also exists, the extension suffix is renamed to .3, and so on... \n"
            "You can set the maximum file rotations, by default 10."

            );
        }

        //separador adicional para que quede mas grande la ventana y mas mono
        menu_add_item_menu_format(array_menu_ask_no_append_truncate,MENU_OPCION_SEPARADOR,NULL,NULL," ");



        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&ask_no_append_truncate_opcion_seleccionada,&item_seleccionado,array_menu_ask_no_append_truncate,texto_ventana);



        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
    //if (ask_no_append_truncate_opcion_seleccionada==1) return 1;
    //else return 0;
                return ask_no_append_truncate_opcion_seleccionada-1; //0=Cancel, 1=Append, 2=Truncate, 3=Rotate
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}

//retorna 1 si opcion 1
//retorna 0 si ESC
int menu_simple_one_choices(char *texto_ventana,char *texto_interior,char *opcion1)
{



        menu_espera_no_tecla();


    menu_item *array_menu_simple_two_choices;
        menu_item item_seleccionado;
        int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_two_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_two_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_two_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);


            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_two_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_two_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_two_choices,texto_ventana);



            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    return simple_two_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}


//retorna 1 si opcion 1
//retorna 2 si opcion 2
//retorna 0 si ESC
int menu_simple_two_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2)
{



        menu_espera_no_tecla();


    menu_item *array_menu_simple_two_choices;
        menu_item item_seleccionado;
        int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_two_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_two_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_two_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

            menu_add_item_menu_format(array_menu_simple_two_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_two_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_two_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_two_choices,texto_ventana);



            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    return simple_two_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}


//retorna 1 si opcion 1
//retorna 2 si opcion 2
//retorna 3 si opcion 3
//retorna 0 si ESC
int menu_simple_three_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3)
{



        menu_espera_no_tecla();


    menu_item *array_menu_simple_three_choices;
        menu_item item_seleccionado;
        int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_three_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_three_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_three_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

            menu_add_item_menu_format(array_menu_simple_three_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

            menu_add_item_menu_format(array_menu_simple_three_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion3);

            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_three_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_three_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_three_choices,texto_ventana);



            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    return simple_three_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}


//retorna 1 si opcion 1
//retorna 2 si opcion 2
//retorna 3 si opcion 3
//retorna 4 si opcion 4
//retorna 0 si ESC
int menu_simple_four_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,char *opcion4)
{



        menu_espera_no_tecla();


    menu_item *array_menu_simple_four_choices;
        menu_item item_seleccionado;
        int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_four_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_four_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_four_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

            menu_add_item_menu_format(array_menu_simple_four_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

            menu_add_item_menu_format(array_menu_simple_four_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion3);

            menu_add_item_menu_format(array_menu_simple_four_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion4);

            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_four_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_four_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_four_choices,texto_ventana);



            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    return simple_four_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}


//retorna 1 si opcion 1
//retorna 2 si opcion 2
//retorna 3 si opcion 3
//retorna 4 si opcion 4
//retorna 5 si opcion 5
//retorna 0 si ESC
int menu_simple_five_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,char *opcion4,char *opcion5)
{


    menu_espera_no_tecla();


    menu_item *array_menu_simple_five_choices;
    menu_item item_seleccionado;
    int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_five_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_five_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_five_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

            menu_add_item_menu_format(array_menu_simple_five_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

            menu_add_item_menu_format(array_menu_simple_five_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion3);

            menu_add_item_menu_format(array_menu_simple_five_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion4);

            menu_add_item_menu_format(array_menu_simple_five_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion5);

            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_five_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            //retorno_menu=menu_dibuja_menu_one_time(&simple_five_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_five_choices,texto_ventana);

            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_five_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_five_choices,texto_ventana);

            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    return simple_five_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}


//retorna 1 si opcion 1
//retorna 2 si opcion 2
//retorna 3 si opcion 3
//retorna 4 si opcion 4
//retorna 5 si opcion 5
//retorna 6 si opcion 6
//retorna 0 si ESC
int menu_simple_six_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,
    char *opcion4,char *opcion5,char *opcion6)
{


    menu_espera_no_tecla();


    menu_item *array_menu_simple_six_choices;
    menu_item item_seleccionado;
    int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_six_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_six_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_six_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

            menu_add_item_menu_format(array_menu_simple_six_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

            menu_add_item_menu_format(array_menu_simple_six_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion3);

            menu_add_item_menu_format(array_menu_simple_six_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion4);

            menu_add_item_menu_format(array_menu_simple_six_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion5);

            menu_add_item_menu_format(array_menu_simple_six_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion6);

            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_six_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            //retorno_menu=menu_dibuja_menu_one_time(&simple_six_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_six_choices,texto_ventana);
            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_six_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_six_choices,texto_ventana);


            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    return simple_six_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}

//retorna 1 si opcion 1
//retorna 2 si opcion 2
//retorna 3 si opcion 3
//retorna 4 si opcion 4
//retorna 5 si opcion 5
//retorna 6 si opcion 6
//retorna 7 si opcion 7
//retorna 0 si ESC
int menu_simple_seven_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,
    char *opcion4,char *opcion5,char *opcion6,char *opcion7)
{


    menu_espera_no_tecla();


    menu_item *array_menu_simple_seven_choices;
    menu_item item_seleccionado;
    int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_seven_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_seven_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_seven_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

            menu_add_item_menu_format(array_menu_simple_seven_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

            menu_add_item_menu_format(array_menu_simple_seven_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion3);

            menu_add_item_menu_format(array_menu_simple_seven_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion4);

            menu_add_item_menu_format(array_menu_simple_seven_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion5);

            menu_add_item_menu_format(array_menu_simple_seven_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion6);

            menu_add_item_menu_format(array_menu_simple_seven_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion7);

            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_seven_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_seven_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_seven_choices,texto_ventana);



            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    return simple_seven_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}


//retorna 1 si opcion 1
//retorna 2 si opcion 2
//retorna 3 si opcion 3
//retorna 4 si opcion 4
//retorna 5 si opcion 5
//retorna 6 si opcion 6
//retorna 7 si opcion 7
//retorna 8 si opcion 8
//retorna 0 si ESC
int menu_simple_eight_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,
    char *opcion4,char *opcion5,char *opcion6,char *opcion7,char *opcion8)
{


    menu_espera_no_tecla();


    menu_item *array_menu_simple_eight_choices;
    menu_item item_seleccionado;
    int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_eight_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_eight_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_eight_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

            menu_add_item_menu_format(array_menu_simple_eight_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

            menu_add_item_menu_format(array_menu_simple_eight_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion3);

            menu_add_item_menu_format(array_menu_simple_eight_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion4);

            menu_add_item_menu_format(array_menu_simple_eight_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion5);

            menu_add_item_menu_format(array_menu_simple_eight_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion6);

            menu_add_item_menu_format(array_menu_simple_eight_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion7);

            menu_add_item_menu_format(array_menu_simple_eight_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion8);

            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_eight_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_eight_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_eight_choices,texto_ventana);



            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    return simple_eight_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}

//retorna 1 si opcion 1
//retorna 2 si opcion 2
//retorna 3 si opcion 3
//retorna 4 si opcion 4
//retorna 5 si opcion 5
//retorna 6 si opcion 6
//retorna 7 si opcion 7
//retorna 8 si opcion 8
//retorna 9 si opcion 9
//retorna 0 si ESC
int menu_simple_nine_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,
    char *opcion4,char *opcion5,char *opcion6,char *opcion7,char *opcion8,char *opcion9)
{


    menu_espera_no_tecla();


    menu_item *array_menu_simple_nine_choices;
    menu_item item_seleccionado;
    int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_nine_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_nine_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion3);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion4);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion5);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion6);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion7);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion8);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion9);

            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_nine_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_nine_choices,texto_ventana);



            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    return simple_nine_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}


//retorna 1 si opcion 1
//retorna 2 si opcion 2
//retorna 3 si opcion 3
//retorna 4 si opcion 4
//retorna 5 si opcion 5
//retorna 6 si opcion 6
//retorna 7 si opcion 7
//retorna 8 si opcion 8
//retorna 9 si opcion 9
//retorna 10 si opcion 10
//retorna 0 si ESC
int menu_simple_ten_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,
    char *opcion4,char *opcion5,char *opcion6,char *opcion7,char *opcion8,char *opcion9,char *opcion10)
{


    menu_espera_no_tecla();


    menu_item *array_menu_simple_nine_choices;
    menu_item item_seleccionado;
    int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_nine_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_nine_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion3);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion4);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion5);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion6);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion7);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion8);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion9);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion10);

            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_nine_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_nine_choices,texto_ventana);


            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    return simple_nine_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}


//Funcion para preguntar opcion de una lista, usando interfaz de menus
//Entradas de lista finalizadas con NULL
//Retorna 0=Primera opcion, 1=Segunda opcion, etc
//Retorna <0 si salir con ESC
int menu_ask_list_texto(char *texto_ventana,char *texto_interior,char *entradas_lista[])
{



        menu_espera_no_tecla();


        menu_item *array_menu_ask_list;
        menu_item item_seleccionado;
        int retorno_menu;

        //Siempre indicamos primera opcion
        int ask_list_texto_opcion_seleccionada=1;
        do {

                menu_add_item_menu_inicial_format(&array_menu_ask_list,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

                int i=0;

                while(entradas_lista[i]!=NULL) {
                        menu_add_item_menu_format(array_menu_ask_list,MENU_OPCION_NORMAL,NULL,NULL,entradas_lista[i]);
            i++;
                }

                //separador adicional para que quede mas grande la ventana y mas mono
                menu_add_item_menu_format(array_menu_ask_list,MENU_OPCION_SEPARADOR,NULL,NULL," ");



                retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&ask_list_texto_opcion_seleccionada,&item_seleccionado,array_menu_ask_list,texto_ventana);



                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        return ask_list_texto_opcion_seleccionada-1;
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

        return -1;

}


void menu_generic_message_format(char *titulo, const char * texto_format , ...)
{

        //Buffer de entrada
        char *texto;
        texto=malloc(MAX_TEXTO_GENERIC_MESSAGE);
        if (texto==NULL) {
            cpu_panic("Can not allocate memory for message");
        }


        va_list args;
        va_start (args, texto_format);
        vsprintf (texto,texto_format, args);
        va_end (args);




    //menu_generic_message_tooltip(titulo, 0, 0, 0, NULL, "%s", texto);
    zxvision_generic_message_tooltip(titulo , 0, 0 , 0, 0, 0, NULL, 1, 0, "%s", texto);


    //En Linux esto funciona bien sin tener que hacer las funciones va_ previas:
    //menu_generic_message_tooltip(titulo, 0, texto_format)
    //Pero en Mac OS X no obtiene los valores de los parametros adicionales

    free(texto);
}

void menu_generic_message(char *titulo, const char * texto)
{

        //menu_generic_message_tooltip(titulo, 0, 0, 0, NULL, "%s", texto);
        zxvision_generic_message_tooltip(titulo , 0, 0 , 0, 0, 0, NULL, 1, 0, "%s", texto);
}

//Mensaje con setting para marcar
//El contenido de valor_opcion tiene que venir definido en 0/1 según se quiera (habitualmente a 1)
void zxvision_menu_generic_message_setting(char *titulo, const char *texto, char *texto_opcion, int *valor_opcion)
{

    int lineas_agregar=4;

    //Asumimos opcion ya marcada
    //*valor_opcion=1;

    if (menu_es_stdout()) {
        printf("%s\n",titulo);
        scrstdout_menu_print_speech_macro(titulo);
        printf("%s\n",texto);
        scrstdout_menu_print_speech_macro((char *)texto);
        printf ("%s\n",texto_opcion);
        scrstdout_menu_print_speech_macro (texto_opcion);
        printf("Enable or disable setting? 0 or 1?\n");
        scrstdout_menu_print_speech_macro("Enable or disable setting? 0 or 1?");
        scanf("%d",valor_opcion);
        return;
    }


    zxvision_generic_message_tooltip(titulo , 0, lineas_agregar , 0, 0, 0, NULL, 1, 0, "%s", texto);



    zxvision_window *ventana;

    //Nuestra ventana sera la actual
    ventana=zxvision_current_window;

    int posicion_y_opcion=ventana->visible_height-lineas_agregar-1;
    //printf ("%d %d\n",posicion_y_opcion,ventana->visible_height);

    int ancho_ventana=ventana->visible_width;
    int posicion_centro_x=ancho_ventana/2-1; //un poco mas a la izquierda

    if (posicion_centro_x<0) posicion_centro_x=0;


        menu_item *array_menu_generic_message_setting;
        menu_item item_seleccionado;
        int array_menu_generic_message_setting_opcion_seleccionada=1;
        int retorno_menu;

    int salir=0;
    do {


        char buffer_texto_opcion[64];
        char buffer_texto_ok[64];

        sprintf (buffer_texto_opcion,"[%c] %s",(*valor_opcion ? 'X' : ' ' ),texto_opcion);
        strcpy(buffer_texto_ok,"<OK>");

        //Tengo antes los textos para sacar longitud y centrarlos

        int posicion_x_opcion=posicion_centro_x-strlen(buffer_texto_opcion)/2;
        int posicion_x_ok=posicion_centro_x-strlen(buffer_texto_ok)/2;


        menu_add_item_menu_inicial_format(&array_menu_generic_message_setting,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto_opcion);
        menu_add_item_menu_tabulado(array_menu_generic_message_setting,posicion_x_opcion,posicion_y_opcion);


        menu_add_item_menu_format(array_menu_generic_message_setting,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto_ok);
        menu_add_item_menu_tabulado(array_menu_generic_message_setting,posicion_x_ok,posicion_y_opcion+2);


        //Nombre de ventana solo aparece en el caso de stdout
        retorno_menu=menu_dibuja_menu_no_title_lang(&array_menu_generic_message_setting_opcion_seleccionada,&item_seleccionado,array_menu_generic_message_setting,titulo);


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

                //Si opcion 1, conmutar valor
                //Conmutar valor
                if (array_menu_generic_message_setting_opcion_seleccionada==0) *valor_opcion ^=1;

                //Si opcion 2, volver
                if (array_menu_generic_message_setting_opcion_seleccionada==1) {
                    //Al volver de esta manera, hay que indicar al index_search que se "va atras" un menu
                    //Esto ya se llama por defecto en gestion de menu, cuando se pulsa ESC o flecha atras,
                    //pero en este caso, se sale con la aceptacion de la opcion, y no es ni ESC ni flecha atras
                    //innecesario con nuevo metodo de indexacion zxvision_index_delete_last_submenu_path();
                    salir=1;
                }

        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus && !salir);



    zxvision_destroy_window(ventana);

    //Y liberar esa memoria, dado que la ventana esta asignada en memoria global
    free(ventana);
}


//Mensaje con dos botones
int zxvision_menu_generic_message_two_buttons(char *titulo, const char *texto,
    char *texto_opcion1,
    char *texto_opcion2
    )
{

    int lineas_agregar=2;


    if (menu_es_stdout()) {
        printf("%s\n",texto);
        char buffer_opciones[200];
        sprintf(buffer_opciones,"1) %s  2) %s  0) exit",texto_opcion1,texto_opcion2);
        printf ("%s\n",buffer_opciones);
        scrstdout_menu_print_speech_macro (buffer_opciones);
        int retorno;
        scanf("%d",&retorno);

        if (retorno==1) return 0;
        if (retorno==2) return 1;
        return -1;
    }

    zxvision_generic_message_tooltip(titulo , 0, lineas_agregar , 0, 0, 0, NULL, 1, 0, "%s", texto);

    zxvision_window *ventana;

    //Nuestra ventana sera la actual
    ventana=zxvision_current_window;

    int posicion_y_opcion=ventana->visible_height-lineas_agregar-1;
    //printf ("%d %d\n",posicion_y_opcion,ventana->visible_height);

    int ancho_ventana=ventana->visible_width;
    int posicion_centro_x=ancho_ventana/2-1; //un poco mas a la izquierda

    if (posicion_centro_x<0) posicion_centro_x=0;


    menu_item *array_menu_generic_message_setting;
    menu_item item_seleccionado;
    int array_menu_generic_message_setting_opcion_seleccionada=0;
    int retorno_menu;

    int salir=0;
    do {



        //Tengo antes los textos para sacar longitud y centrarlos
        int longitud_texto1=strlen(texto_opcion1);
        int longitud_texto2=strlen(texto_opcion2);
        int longitud_dos_textos=longitud_texto1+longitud_texto2+1; //+1 para dejar un espacio en medio

        int posicion_x_opcion1=posicion_centro_x-longitud_dos_textos/2;
        int posicion_x_opcion2=posicion_x_opcion1+longitud_texto1+1;


        menu_add_item_menu_inicial_format(&array_menu_generic_message_setting,MENU_OPCION_NORMAL,NULL,NULL,texto_opcion1);
        menu_add_item_menu_tabulado(array_menu_generic_message_setting,posicion_x_opcion1,posicion_y_opcion);


        menu_add_item_menu_format(array_menu_generic_message_setting,MENU_OPCION_NORMAL,NULL,NULL,texto_opcion2);
        menu_add_item_menu_tabulado(array_menu_generic_message_setting,posicion_x_opcion2,posicion_y_opcion);


        //Nombre de ventana solo aparece en el caso de stdout
        retorno_menu=menu_dibuja_menu_no_title_lang(&array_menu_generic_message_setting_opcion_seleccionada,&item_seleccionado,array_menu_generic_message_setting,titulo);



        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                salir=1;
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus && !salir);



    zxvision_destroy_window(ventana);

    //Y liberar esa memoria, dado que la ventana esta asignada en memoria global
    free(ventana);

    //Si salido con ESC
    if (item_seleccionado.tipo_opcion&MENU_OPCION_ESC || retorno_menu==MENU_RETORNO_ESC) return -1;

    return array_menu_generic_message_setting_opcion_seleccionada;
}


void menu_generic_message_splash(char *titulo, const char * texto)
{

        //menu_generic_message_tooltip(titulo, 1, 0, 0, NULL, "%s", texto);
        zxvision_generic_message_tooltip(titulo , 0, 0 , 1, 0, 0, NULL, 0, 0, "%s", texto);
        menu_espera_no_tecla();
}

void menu_generic_message_warn(char *titulo, const char * texto)
{

        //menu_generic_message_tooltip(titulo, 1, 0, 0, NULL, "%s", texto);
        zxvision_generic_message_tooltip(titulo , 0, 0 , 2, 0, 0, NULL, 0, 0, "%s", texto);
}

int menu_confirm_yesno(char *texto_ventana)
{
    return menu_confirm_yesno_texto(texto_ventana,menu_get_string_language("Sure?"));
}


//max_length contando caracter 0 del final, es decir, para un texto de 4 caracteres, debemos especificar max_length=5
//Retorna -1 si se ha salido con ESC
int menu_ventana_scanf(char *titulo,char *texto,int max_length)
{

    //En caso de stdout, es mas simple, mostrar texto y esperar texto
    if (menu_es_stdout()) {
        printf ("%s\n",titulo);
        scrstdout_menu_print_speech_macro(titulo);

        //Controlar maximo en cadena de texto aparte
        char buffer_temporal[1024];
        scanf("%s",buffer_temporal);
        int l=strlen(buffer_temporal);
        if (l>max_length-1) {
            printf ("Too long\n");
            scrstdout_menu_print_speech_macro("Too long");
            return 0;
        }


        strcpy(texto,buffer_temporal);

        //scanf("%s",texto);

        return 0;
    }

    //int scanf_x=1;
    //int scanf_y=10;
    int scanf_ancho=30;
    int scanf_alto=3;
    int scanf_x=menu_center_x()-scanf_ancho/2;
    int scanf_y=menu_center_y()-scanf_alto/2;


        menu_espera_no_tecla();

    zxvision_window ventana;

    zxvision_new_window(&ventana,scanf_x,scanf_y,scanf_ancho,scanf_alto,
                            scanf_ancho-1,scanf_alto-2,titulo);

    //No queremos que se pueda redimensionar ni minimizar
    ventana.can_be_resized=0;
    ventana.can_be_minimized=0;

    zxvision_draw_window(&ventana);


    int tecla=zxvision_scanf(&ventana,texto,max_length,scanf_ancho-2,1,0,0,0,0);

    //menu_scanf(texto,max_length,scanf_ancho-2,scanf_x+1,scanf_y+1);
    //int menu_scanf(char *string,unsigned int max_length,int max_length_shown,int x,int y)

    //Al salir
    //menu_refresca_pantalla();
    menu_cls_refresh_emulated_screen();

    zxvision_destroy_window(&ventana);

    if (tecla==2) return -1;
    else return 0;

}


//funcion como scanf pero con lineas adicionales por debajo que permiten elegir valores historicos
//max_length contando caracter 0 del final, es decir, para un texto de 4 caracteres, debemos especificar max_length=5
//textos_historial acabados en NULL
//Retorna tecla pulsada
int zxvision_scanf_history(char *titulo,char *texto,int max_length,char **textos_historial)
{

    //En caso de stdout, es mas simple, mostrar texto y esperar texto
    if (menu_es_stdout()) {
        printf ("%s\n",titulo);
        scrstdout_menu_print_speech_macro(titulo);

        //Controlar maximo en cadena de texto aparte
        char buffer_temporal[1024];
        scanf("%s",buffer_temporal);
        int l=strlen(buffer_temporal);
        if (l>max_length-1) {
            printf ("Too long\n");
            scrstdout_menu_print_speech_macro("Too long");
            return 0;
        }


        strcpy(texto,buffer_temporal);

        //scanf("%s",texto);

        return 0;
    }

    //copiar texto original por si se sale con ESC
    //int longitud_texto_original=strlen(texto)+1;
    //char *copia_texto_original=util_malloc(longitud_texto_original,"Can not allocate buffer for zxvision_scanf_history original string");
    //strcpy(copia_texto_original,texto);

    int lineas_historial=util_scanf_history_get_total_lines(textos_historial);

    int scanf_ancho=30;
    int scanf_alto=3+lineas_historial;
    int scanf_x=menu_center_x()-scanf_ancho/2;
    int scanf_y=menu_center_y()-scanf_alto/2;

    menu_espera_no_tecla();

    zxvision_window ventana;

    zxvision_new_window(&ventana,scanf_x,scanf_y,scanf_ancho,scanf_alto,
                            scanf_ancho-1,scanf_alto-2,titulo);

    //No queremos que se pueda redimensionar, el tamaño es suficiente para que quepa historial
    //ademas eventos de resize habria que tratarlos debajo adecuadamente
    ventana.can_be_resized=0;
    ventana.can_be_minimized=0;

    int tecla=0;

    //Mostrar lineas historial
    int i;
    for (i=0;i<lineas_historial;i++) {
        zxvision_print_string_defaults(&ventana,1,i+1,textos_historial[i]);
    }

    zxvision_draw_window(&ventana);

    do {

        //En primera linea, input de texto
        if (ventana.cursor_line==0) {

            tecla=zxvision_scanf(&ventana,texto,max_length,scanf_ancho-2,1,0,1,1,0);

            //flecha abajo
            if (tecla==10 && lineas_historial>0) {

                zxvision_inc_cursor_line(&ventana);
                zxvision_set_visible_cursor(&ventana);

                //decimos que hay que leer tecla
                tecla=0;
            }

            else {
                //Si se hace click con raton, tecla es 13 tal cual

                //printf("menu_mouse %dx%d\n",menu_mouse_x,menu_mouse_y);
                //printf("tecla: %d\n",tecla);

                //Se lee 13 (de zxvision_scanf, que puede ser enter o click izquierdo raton)
                //y ademas click izquierdo
                if (tecla==13 && mouse_left) {

                    //Ver si apuntamos a linea 1 tal cual
                    if (menu_mouse_y==1) {
                        //apuntamos a mismo sitio, no hacemos nada
                        tecla=0;
                    }
                    else {
                        if (menu_mouse_x>=0 && menu_mouse_x<ventana.visible_width) {
                            //apuntamos a linea historial
                            //linea 2 es la primera del historial
                            zxvision_set_cursor_line(&ventana,menu_mouse_y-1);
                            zxvision_set_visible_cursor(&ventana);
                            zxvision_draw_window_contents(&ventana);

                            menu_espera_no_tecla();
                            //Y dejar tecla=13 tal cual para que entre por el if de abajo donde gestiona lineas historial
                        }
                    }
                }
            }
        }

        //Gestion lineas historial
        if (ventana.cursor_line!=0) {
            //printf("menu_mouse %dx%d\n",menu_mouse_x,menu_mouse_y);
            //printf("tecla: %d\n",tecla);

            zxvision_draw_window_contents(&ventana);

            //Si tecla no venia definida de antes, leerla
            if (!tecla) tecla=zxvision_common_getkey_refresh();

            //printf("menu_mouse %dx%d\n",menu_mouse_x,menu_mouse_y);

            //Cursor abajo
            if (tecla==10) {
                zxvision_set_visible_cursor(&ventana);

                if (ventana.cursor_line<lineas_historial) {
                    zxvision_inc_cursor_line(&ventana);
                    zxvision_sound_event_cursor_movement();
                }
                tecla=0;
            }

            //Cursor arriba
            if (tecla==11) {
                if (ventana.cursor_line>0) {
                   zxvision_dec_cursor_line(&ventana);
                   zxvision_sound_event_cursor_movement();

                   if (ventana.cursor_line==0) {
                    zxvision_reset_visible_cursor(&ventana);
                   }
                }
                tecla=0;
            }

            //Pulsado boton izquierdo raton
            if (tecla==0 && mouse_left) {
                //Si linea 1, volver al input
                if (menu_mouse_x>=0 && menu_mouse_x<ventana.visible_width) {
                    if (menu_mouse_y<2) {
                        zxvision_set_cursor_line(&ventana,0);
                        zxvision_reset_visible_cursor(&ventana);
                        zxvision_draw_window_contents(&ventana);

                        menu_espera_no_tecla();
                        tecla=0;
                    }
                    else {
                        //pulsado en linea historial. salir
                        //printf("Click linea\n");
                        zxvision_set_cursor_line(&ventana,menu_mouse_y-1);
                        zxvision_draw_window_contents(&ventana);

                        menu_espera_no_tecla();
                        tecla=13;
                        //saldra como si se leyera tecla enter desde abajo
                    }
                }
            }

            if (tecla==13) {
                //printf("Pulsado enter\n");
                //Asignar a texto la linea seleccionada
                //asegurarnos que cursor no sale del total de historial
                int cursor_historial=ventana.cursor_line-1;

                if (cursor_historial>=0 && cursor_historial<lineas_historial) {
                    strcpy(texto,textos_historial[cursor_historial]);
                }
                else {
                    debug_printf(VERBOSE_DEBUG,"Trying to set history line %d which does not exist, this should not happen!",cursor_historial);
                }
            }

            if (tecla!=10 && tecla!=11 && tecla!=13 && tecla!=2) {
                //Cualquier otra tecla no gestionada, quitarla, si no, se quedaria con tecla diferente de 0 en bucle
                //printf("Establecer tecla=0\n");
                tecla=0;
            }
        }


        //printf("Tecla: %d\n",tecla);

    }  while (tecla!=13 && tecla!=2);

    /*if (tecla==2) {
        printf("Pulsado ESC\n");

        //Recuperar texto original
        strcpy(texto,copia_texto_original);
    }*/

    if (tecla!=2) {

        //Insertar en primera posicion del historial el texto elegido
        //solo si hemos escrito el texto en el input
        if (ventana.cursor_line==0) {
            //Si la cadena ya existe en otra entrada, no insertar
            int encontrado=0;
            for (i=0;i<lineas_historial && !encontrado;i++) {
                if (!strcmp(texto,textos_historial[i])) encontrado=1;
            }

            if (encontrado) {
                debug_printf(VERBOSE_PARANOID,"String (%s) already on list. Do not insert",texto);
            }
            else {
                util_scanf_history_insert(textos_historial,texto);
            }
        }

    }

    //printf("Tecla al salir: %d\n",tecla);
    //free(copia_texto_original);

    zxvision_destroy_window(&ventana);

    return tecla;

}




void menu_ventana_scanf_number_aux(zxvision_window *ventana,char *texto,int max_length,int x_texto_input)
{
    //En entrada de texto no validamos el maximo y minimo. Eso lo tiene que seguir haciendo la funcion que llama a menu_ventana_scanf_numero
    //Si que se controla al pulsar botones de + y -
    zxvision_scanf(ventana,texto,max_length,max_length,x_texto_input,0,1,0,0);
}

void menu_ventana_scanf_number_print_buttons(zxvision_window *ventana,char *texto,int x_boton_menos,int x_boton_mas,int x_texto_input,int x_boton_ok,int x_boton_cancel)
{
            //Borrar linea entera
        zxvision_print_string_defaults_fillspc(ventana,x_boton_menos,0,"");

        //Escribir - +
        zxvision_print_string_defaults(ventana,x_boton_menos,0,"-");
        zxvision_print_string_defaults(ventana,x_boton_mas,0,"+");

        //Escribir numero
        zxvision_print_string_defaults(ventana,x_texto_input,0,texto);

        zxvision_print_string_defaults(ventana,x_boton_ok,2,"<OK>");

        zxvision_print_string_defaults(ventana,x_boton_cancel,2,"<Cancel>");
}

//busca donde apunta el mouse y retorna opcion seleccionada
int menu_ventana_scanf_number_ajust_cursor_mouse(menu_item *m,int posicion_raton_x,int posicion_raton_y)
{

    //printf ("buscando en %d,%d\n",posicion_raton_x,posicion_raton_y);

    menu_item *buscar_tabulado;
    int linea_buscada;

    buscar_tabulado=menu_retorna_item_tabulado_xy(m,posicion_raton_x,posicion_raton_y,&linea_buscada);

    int linea_seleccionada=-1;

    if (buscar_tabulado!=NULL) {
        //Buscar por coincidencia de coordenada x,y
        if (buscar_tabulado->tipo_opcion!=MENU_OPCION_SEPARADOR) {
            linea_seleccionada=linea_buscada;
            //printf("encontrada opcion en %d\n",linea_buscada);
            //redibuja_ventana=1;
            //menu_tooltip_counter=0;
        }
    }
    else {
        //printf ("item no encontrado\n");
    }

    return linea_seleccionada;
}

/*
max_length: maxima longitud, contando caracter 0 del final
minimo: valor minimo admitido
maximo: valor maximo admitido
circular: si al pasar umbral, se resetea al otro umbral

En entrada de texto no validamos el maximo y minimo. Eso lo tiene que seguir haciendo la funcion que llama a menu_ventana_scanf_numero
Si que se controla al pulsar botones de + y -

*/

//Retorna -1 si pulsado ESC
int menu_ventana_scanf_numero(char *titulo,char *texto,int max_length,int incremento,int minimo,int maximo,int circular)
{

    //En caso de stdout, es mas simple, mostrar texto y esperar texto
    //Lo gestiona la propia rutina de menu_ventana_scanf
    if (menu_es_stdout()) {
        menu_ventana_scanf(titulo,texto,max_length);
        return 0;
    }


    int ancho_ventana=32;
    int alto_ventana=5;


    int xventana=menu_center_x()-ancho_ventana/2;
    int yventana=menu_center_y()-alto_ventana/2;

    zxvision_window ventana;

    zxvision_new_window(&ventana,xventana,yventana,ancho_ventana,alto_ventana,
                                                        ancho_ventana-1,alto_ventana-2,titulo);

    //Dado que es una variable local, siempre podemos usar este nombre array_menu_common
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

    //El foco en el numero
    int comun_opcion_seleccionada=1;


    //Donde van los bloques

    //int inicio_bloque_x=8;
    //int inicio_bloque_y=2;
    //int ancho_bloque=6;

    //int linea=inicio_bloque_y;

    int max_input_visible=ancho_ventana-2-2-2; //2 laterales, 2 de los botones, y 2 de espacios entre botones
    if (max_length<max_input_visible) max_input_visible=max_length;

    int x_boton_menos=1;
    int x_texto_input=x_boton_menos+2;
    int x_boton_mas=x_texto_input+max_input_visible+1;
    int x_boton_ok=1;
    int x_boton_cancel=x_boton_ok+5;

    //Dibujar texto interior
    menu_ventana_scanf_number_print_buttons(&ventana,texto,x_boton_menos,x_boton_mas,x_texto_input,x_boton_ok,x_boton_cancel);

    //Dibujar ventana antes de scanf
    zxvision_draw_window(&ventana);
    zxvision_draw_window_contents(&ventana);

    //Entramos primero editando el numero
    menu_ventana_scanf_number_aux(&ventana,texto,max_length,x_texto_input);

    //Cambiar la opcion seleccionada a la del OK, al pulsar enter
    //comun_opcion_seleccionada=3;



    //Decir que habra que ajustar raton segun posicion mouse actual
    int debe_ajustar_cursor_segun_mouse=1;


    do {



        //Escribir primero numero

        //Dibujar texto interior
        menu_ventana_scanf_number_print_buttons(&ventana,texto,x_boton_menos,x_boton_mas,x_texto_input,x_boton_ok,x_boton_cancel);


        menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"-");
        menu_add_item_menu_tabulado(array_menu_common,x_boton_menos,0);
        //no indexar esta busqueda
        menu_add_item_menu_no_indexar_busqueda(array_menu_common);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,texto);
        menu_add_item_menu_tabulado(array_menu_common,x_texto_input,0);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"+");
        menu_add_item_menu_tabulado(array_menu_common,x_boton_mas,0);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"<OK>");
        menu_add_item_menu_tabulado(array_menu_common,x_boton_ok,2);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL,"<Cancel>");
        menu_add_item_menu_tabulado(array_menu_common,x_boton_cancel,2);


        //Antes de abrir el menu, ajustar la opcion seleccionada cuando ha salido del input de numero
        //y a que boton apunta el mouse
        if (debe_ajustar_cursor_segun_mouse) {
            debe_ajustar_cursor_segun_mouse=0;

            //Asumimos que esta en OK
            //Cambiar la opcion seleccionada a la del OK, al pulsar enter
            comun_opcion_seleccionada=3;

            int opcion_sel=menu_ventana_scanf_number_ajust_cursor_mouse(array_menu_common,menu_mouse_x,menu_mouse_y-1);
            //printf("opcion seleccionada: %d\n",opcion_sel);
            if (opcion_sel>=0) {
                comun_opcion_seleccionada=opcion_sel;
            }
        }

        retorno_menu=menu_dibuja_menu_no_title_lang(&comun_opcion_seleccionada,&item_seleccionado,array_menu_common,titulo);


            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {


                    //botones menos y mas
                    if (comun_opcion_seleccionada==0 || comun_opcion_seleccionada==2) {

                        int numero=parse_string_to_number(texto);

                        if (comun_opcion_seleccionada==0) {
                            numero-=incremento;
                            if (numero<minimo) {
                                if (circular) {
                                    numero=maximo;
                                }
                                else {
                                    numero=minimo;
                                }
                            }
                        }

                        if (comun_opcion_seleccionada==2) {
                            numero+=incremento;

                            if (numero>maximo) {
                                if (circular) {
                                    numero=minimo;
                                }
                                else {
                                    numero=maximo;
                                }
                            }
                        }

                        sprintf(texto,"%d",numero);
                    }

                    if (comun_opcion_seleccionada==1) {
                        menu_ventana_scanf_number_aux(&ventana,texto,max_length,x_texto_input);
                        //zxvision_scanf(&ventana,texto,max_length,max_length,x_texto_input,0,1);
                        //menu_espera_no_tecla();



                        //Pero ajustar el mouse si apunta a alguna opcion
                        debe_ajustar_cursor_segun_mouse=1;
                    }



            }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus && comun_opcion_seleccionada<3);


    //En caso de menus tabulados, es responsabilidad de este de liberar ventana
    zxvision_destroy_window(&ventana);



    if (comun_opcion_seleccionada==4 || retorno_menu==MENU_RETORNO_ESC) return -1; //Pulsado Cancel

    else return 0;
}

//Similar a menu_ventana_scanf_numero pero evita tener que crear el buffer de char temporal
//Y ademas muestra error si limites se exceden
//Retorna -1 si pulsado ESC o valor fuera de rango
//Si se ha pulsado ESC o si se exceden los limites, no se modifica *variable
//max_length: maxima longitud, contando caracter 0 del final
//minimo: valor minimo admitido
//maximo: valor maximo admitido
//circular: si al pasar umbral, se resetea al otro umbral
int menu_ventana_scanf_numero_enhanced(char *titulo,int *variable,int max_length,int incremento,int minimo,int maximo,int circular)
{

    //Asignar memoria para el buffer
    char *buf_texto;

    buf_texto=malloc(max_length);
    if (buf_texto==NULL) cpu_panic("Can not allocate memory for input text");

    sprintf(buf_texto,"%d",*variable);

    int ret=menu_ventana_scanf_numero(titulo,buf_texto,max_length,incremento,minimo,maximo,circular);
    //Si no pulsado ESC
    if (ret>=0) {

        int numero=parse_string_to_number(buf_texto);

        if (numero<minimo || numero>maximo) {
            debug_printf(VERBOSE_ERR,"Out of range. Allowed range: minimum: %d maximum: %d",minimo,maximo);
            ret=-1;
        }
        else {
            *variable=numero;
        }
    }

    //printf("liberando memoria\n");
    free(buf_texto);

    if (ret>=0) return 0;
    else return -1;

}







void menu_inicio_pre_retorno_reset_flags(void)
{
    //desactivar botones de acceso directo
    menu_button_smartload.v=0;
    menu_button_osdkeyboard.v=0;
    menu_button_osd_adv_keyboard_return.v=0;
    menu_button_osd_adv_keyboard_openmenu.v=0;
    menu_button_exit_emulator.v=0;
    menu_event_drag_drop.v=0;
    menu_event_open_zmenu_file.v=0;
    menu_breakpoint_exception.v=0;
    menu_event_remote_protocol_enterstep.v=0;
    menu_button_f_function.v=0;
    menu_event_open_menu.v=0;
}


//Gestionar pulsaciones de botones inferiores
void menu_inicio_handle_lower_icon_presses(void)
{

    int pulsado_boton=menu_pressed_zxdesktop_lower_icon_which;

    DBG_PRINT_ZXVISION_EVENTS VERBOSE_DEBUG,"ZXVISION_EVENTS: menu_inicio_handle_lower_icon_presses. menu_pressed_zxdesktop_lower_icon_which: %d menu_pressed_zxdesktop_lower_icon_which_right_button: %d",
            menu_pressed_zxdesktop_lower_icon_which,menu_pressed_zxdesktop_lower_icon_which_right_button);

    //Para que no vuelva a saltar
    menu_pressed_zxdesktop_lower_icon_which=-1;

    // Ver que este activo

        int total_botones;

    int ancho_boton;
    int alto_boton;

    int yinicio;

    menu_ext_desktop_lower_icons_get_geometry(&ancho_boton,&alto_boton,&total_botones,NULL,NULL,&yinicio);

    //printf("yinicio: %d\n",)


    if (pulsado_boton>=total_botones) return;

    //Ver indice del array

    int indice_array=zxdesktop_lowericon_find_index(pulsado_boton);

    if (indice_array<0) return;



    //Ejecutar accion

    void (*funcion_accion)(void);

    funcion_accion=zdesktop_lowericons_array[indice_array].accion;


    //Si pulsado con boton derecho
    if (menu_pressed_zxdesktop_lower_icon_which_right_button) {
        menu_pressed_zxdesktop_lower_icon_which_right_button=0;
        DBG_PRINT_ZXVISION_EVENTS VERBOSE_INFO,"ZXVISION_EVENTS: menu_inicio_handle_lower_icon_presses. Pressed right button on a lower icon %d",pulsado_boton);

        //printf("salir_todos_menus %d menu_pressed_open_menu_while_in_menu.v %d "
        //        "mouse_pressed_background_window %d mouse_pressed_close_window %d\n",
        //        salir_todos_menus,menu_pressed_open_menu_while_in_menu.v,mouse_pressed_background_window,mouse_pressed_close_window);

        //Resetear ciertas variables para que no se cierre de nuevo el menu que vamos a abrir
        //salir_todos_menus=0;
        //menu_pressed_open_menu_while_in_menu.v=0;


        mouse_pressed_background_window=0;
        mouse_pressed_close_window=0;

        //Si accion boton derecho no es NULL
        if (zdesktop_lowericons_array[indice_array].accion_boton_derecho==NULL) {
            menu_warn_message("This device has no right-click associated action");
        }
        else {
            zxvision_set_next_menu_position_from_current_mouse();
            funcion_accion=zdesktop_lowericons_array[indice_array].accion_boton_derecho;
            funcion_accion();
            zxvision_reset_set_next_menu_position();
        }
    }

    else {
        funcion_accion();
    }

    salir_todos_menus=1;



}

void menu_zxdesktop_add_direct_smartload(void)
{
    //Los mismos que smartload

        char *filtros[40];

        filtros[0]="zx";
        filtros[1]="sp";
        filtros[2]="z80";
        filtros[3]="sna";

        filtros[4]="o";
        filtros[5]="p";
        filtros[6]="80";
        filtros[7]="81";
    filtros[8]="z81";

        filtros[9]="tzx";
        filtros[10]="tap";

    filtros[11]="rwa";
    filtros[12]="smp";
    filtros[13]="wav";

    filtros[14]="epr";
    filtros[15]="63";
    filtros[16]="eprom";
    filtros[17]="flash";

    filtros[18]="ace";

    filtros[19]="dck";

    filtros[20]="cdt";

    filtros[21]="ay";

    filtros[22]="scr";

    filtros[23]="rzx";

    filtros[24]="zsf";

    filtros[25]="spg";

    filtros[26]="trd";

    filtros[27]="nex";

    filtros[28]="dsk";

    filtros[29]="pzx";

    filtros[30]="rom";

    filtros[31]="col";

    filtros[32]="sg";

    filtros[33]="cas";

    filtros[34]="snx";

    filtros[35]="sms";

    filtros[36]="bin";

    filtros[37]="p81";

    filtros[38]="m3u";

    filtros[39]=0;


    //guardamos directorio actual
    char directorio_actual[PATH_MAX];
    getcwd(directorio_actual,PATH_MAX);

    //Obtenemos directorio de ultimo quickload
    if (quickfile!=NULL) {
            char directorio[PATH_MAX];
            util_get_dir(quickfile,directorio);
            //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

            //cambiamos a ese directorio, siempre que no sea nulo
            if (directorio[0]!=0) {
                    debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                    //printf ("antes zvfs_chdir\n");
                    zvfs_chdir(directorio);
                    //printf ("despues zvfs_chdir\n");
            }


    }



    char ruta_a_archivo[PATH_MAX];


    int ret;

    //printf ("antes menu_filesel\n");

    ret=menu_filesel("Select file",filtros,ruta_a_archivo);

    //printf ("despues menu_filesel\n");

    //volvemos a directorio inicial

    zvfs_chdir(directorio_actual);



    if (ret==1) {


        //Crear un icono. Por defecto smartload sin indicar tipo
        enum defined_f_function_ids id_funcion=F_FUNCION_DESKTOP_GENERIC_SMARTLOAD;


        //Si es snapshot...
        if (!util_compare_file_extension(ruta_a_archivo,"zx") ||
            !util_compare_file_extension(ruta_a_archivo,"sp") ||
            !util_compare_file_extension(ruta_a_archivo,"z80") ||
            !util_compare_file_extension(ruta_a_archivo,"sna") ||
            !util_compare_file_extension(ruta_a_archivo,"snx") ||
            !util_compare_file_extension(ruta_a_archivo,"p") ||
            !util_compare_file_extension(ruta_a_archivo,"o") ||
            !util_compare_file_extension(ruta_a_archivo,"80") ||
            !util_compare_file_extension(ruta_a_archivo,"81") ||
            !util_compare_file_extension(ruta_a_archivo,"p81") ||
            !util_compare_file_extension(ruta_a_archivo,"z81") ||
            !util_compare_file_extension(ruta_a_archivo,"zsf") ||
            !util_compare_file_extension(ruta_a_archivo,"ace") ||
            !util_compare_file_extension(ruta_a_archivo,"spg") ||
            !util_compare_file_extension(ruta_a_archivo,"nex")
            ) {
            id_funcion=F_FUNCION_DESKTOP_SNAPSHOT;
        }

        //Si es cinta...
        if (!util_compare_file_extension(ruta_a_archivo,"tap") ||
            !util_compare_file_extension(ruta_a_archivo,"tzx") ||
            !util_compare_file_extension(ruta_a_archivo,"cdt") ||
            !util_compare_file_extension(ruta_a_archivo,"rwa") ||
            !util_compare_file_extension(ruta_a_archivo,"smp") ||
            !util_compare_file_extension(ruta_a_archivo,"wav") ||
            !util_compare_file_extension(ruta_a_archivo,"pzx") ||
            !util_compare_file_extension(ruta_a_archivo,"cas")
            ) {
            id_funcion=F_FUNCION_DESKTOP_TAPE;
        }


        int indice_icono=zxvision_add_configurable_icon_by_id_action(id_funcion);

        if (indice_icono>=0) {
            //Indicarle la ruta al snapshot
            strcpy(zxdesktop_configurable_icons_list[indice_icono].extra_info,ruta_a_archivo);
            //Agregarle texto
            char name_no_dir[PATH_MAX];
            util_get_file_no_directory(ruta_a_archivo,name_no_dir);

            strcpy(zxdesktop_configurable_icons_list[indice_icono].text_icon,name_no_dir);
            //sprintf(zxdesktop_configurable_icons_list[indice_icono].text_icon,"Snap%03d",menu_snapshot_quicksave_contador_archivo++);
        }




    }


}

//Gestionar pulsacion boton derecho sobre background
void menu_inicio_handle_right_button_background(void)
{

    //debug_exec_show_backtrace();

    //Liberar esto o se cerrara la siguiente ventana
    //Esto estara activado si venimos de boton derecho con alguna ventana activa
    mouse_pressed_close_window=0;

    //Liberar pulsada tecla background
    mouse_pressed_background_window=0;

    if (menu_pressed_zxdesktop_right_button_background>=0) {
        menu_pressed_zxdesktop_right_button_background=-1;

        //propiedades zx desktop, agregar icono
        int opcion=menu_simple_five_choices("ZX Desktop","--Action--","New icon","New file link","Customize icons","Reorder icons","ZX Desktop settings");

        switch (opcion) {


            case 1:
                menu_zxdesktop_add_configurable_icons(0);
            break;

            case 2:
                menu_zxdesktop_add_direct_smartload();
            break;

            case 3:
                menu_zxdesktop_set_configurable_icons(0);
            break;

            case 4:
                if (menu_confirm_yesno("Reorder icons")) {
                    zxvision_reorder_configurable_icons();
                }
            break;

            case 5:
                menu_ext_desktop_settings(0);
            break;

        }

        salir_todos_menus=1;

        //Y decir que el siguiente menu ya no se abre desde boton y por tanto no se posiciona debajo del boton
        //Antes se quitaba el flag tambien en menu_dibuja_menu, pero ya no. Asi conseguimos que todos los menus
        //que se abran dependiendo del boton, queden debajo de dicho boton
        force_next_menu_position.v=0;
    }
}

//Gestionar pulsaciones sobre iconos, tanto si es boton derecho como izquierdo
void menu_inicio_handle_configurable_icon_presses(void)
{

    //Liberar esto o se cerrara la siguiente ventana
    //Esto estara activado si venimos de boton derecho con alguna ventana activa
    mouse_pressed_close_window=0;

    //Liberar pulsada tecla background
    mouse_pressed_background_window=0;

    if (menu_pressed_zxdesktop_configurable_icon_right_button) {

        //Acciones secundarias
        int pulsado_boton=menu_pressed_zxdesktop_configurable_icon_which;

        DBG_PRINT_ZXVISION_EVENTS VERBOSE_DEBUG,"ZXVISION_EVENTS: Handling right button press on a configurable icon (%d)",pulsado_boton);

        //porque queremos que el menu aparezca pegado al raton
        zxvision_set_next_menu_position_from_current_mouse();

        menu_zxdesktop_set_configurable_icons_modify(pulsado_boton);


        //Para que no vuelva a saltar
        menu_pressed_zxdesktop_configurable_icon_which=-1;
        menu_pressed_zxdesktop_configurable_icon_right_button=0;



        salir_todos_menus=1;

        //menu_pressed_open_menu_while_in_menu.v=0;
        //salir_todos_menus=0;

        return;
    }


    int pulsado_boton=menu_pressed_zxdesktop_configurable_icon_which;

    debug_printf(VERBOSE_DEBUG,"Handling left button press on a configurable icon (%d)",pulsado_boton);

    //Para que no vuelva a saltar
    menu_pressed_zxdesktop_configurable_icon_which=-1;


    //Si se ha movido
    //mouse_movido solo se habilita si ventanas estaban cerradas
    //por tanto buscamos otra manera


    int current_mouse_pixel_x,current_mouse_pixel_y;
    menu_calculate_mouse_xy_absolute_interface_pixel(&current_mouse_pixel_x,&current_mouse_pixel_y);

    current_mouse_pixel_x *=zoom_x;
    current_mouse_pixel_y *=zoom_y;


    //Esto es un poco puñetero. Decir que el raton no se ha movido o nos quedariamos esperando continuamente en bucle
    //a liberar raton en funciones de liberar tecla
    //Y esto se puede habilitar desde menu_calculate_mouse_xy_absolute_interface_pixel, por eso va despues de esa llamada
    mouse_movido=0;

    int icono_se_ha_movido=0;

    //Ver si se mueve una cantidad suficiente
    int deltax=util_get_absolute(current_mouse_pixel_x-menu_pressed_zxdesktop_configurable_icon_where_x);
    int deltay=util_get_absolute(current_mouse_pixel_y-menu_pressed_zxdesktop_configurable_icon_where_y);

    if (deltax>=5 || deltay>=5) {
        icono_se_ha_movido=1;
    }


    if (icono_se_ha_movido) {
        debug_printf(VERBOSE_DEBUG,"Icon has been moved. Do not execute icon action");
        //TODO: quitar este codigo de abajo que no se llamara nunca. Pero dejar el if else para que el else solo se haga cuando no se haya movido icono
        //TODO: gestion de movimiento de iconos se hace cuando se arrastra. Esto lo desactivo.

        //Estos iconos son un poco particulares, no se comportan ni como botones ni como ventanas, pues se invocan al abrir menu,
        //pero tambien se pueden arrastrar
        //Aqui suele entrar cuando menu esta cerrado
        if (!mouse_left) {
            //configurable_icon_is_being_moved=0;
            mouse_is_dragging=0;
            //Para que cuando se vuelva a pulsar no interprete movimiento
            menu_pressed_zxdesktop_configurable_icon_where_x=99999;
            menu_pressed_zxdesktop_configurable_icon_where_y=99999;

            //printf("zxvision_mover_icono_papelera_si_conviene 2\n");
            //debug_exec_show_backtrace();

            //int estado_icono=zxdesktop_configurable_icons_list[pulsado_boton].indice_funcion;


            zxvision_mover_icono_papelera_si_conviene();

            zxvision_reorder_configurable_icons_if_auto();

            //configurable_icon_is_being_moved_which
        }

        /*

        int mouse_pixel_x,mouse_pixel_y;
        menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

        //Ver si en el destino no hay cerca la papelera
        int mover_a_papelera=0;
        int hay_papelera=zxvision_search_trash_configurable_icon();
        if (hay_papelera>=0) {
            printf("hay una papelera\n");
            //Y siempre que no sea ya una papelera este icono
            int indice_funcion=zxdesktop_configurable_icons_list[pulsado_boton].indice_funcion;
            enum defined_f_function_ids id_funcion=defined_direct_functions_array[indice_funcion].id_funcion;

            if (id_funcion!=F_FUNCION_DESKTOP_TRASH) {

                int xpapelera=zxdesktop_configurable_icons_list[hay_papelera].x;
                int ypapelera=zxdesktop_configurable_icons_list[hay_papelera].y;

                //Ver si cerca
                deltax=util_get_absolute(mouse_pixel_x-xpapelera);
                deltay=util_get_absolute(mouse_pixel_y-ypapelera);

                printf("Distancia a la papelera: %d,%d\n",deltax,deltay);

                if (deltax<=20 && deltay<=20) {
                    printf("Mover icono a la papelera\n");
                    mover_a_papelera=1;
                }
            }
        }

        if (mover_a_papelera) {
            zxvision_move_configurable_icon_to_trash(pulsado_boton);
        }

        else {
            printf("Mover icono hasta %d %d\n",mouse_pixel_x,mouse_pixel_y);
            //guardarlas sin zoom
            zxdesktop_configurable_icons_list[pulsado_boton].x=mouse_pixel_x;
            zxdesktop_configurable_icons_list[pulsado_boton].y=mouse_pixel_y;
        }
        */
    }

    else {
        int indice_funcion=zxdesktop_configurable_icons_list[pulsado_boton].indice_funcion;

        int id_funcion=defined_direct_functions_array[indice_funcion].id_funcion;

        debug_printf(VERBOSE_DEBUG,"Executing function %d",id_funcion);

        //printf("Antes procesar funcion. menu_overlay_activo=%d\n",menu_overlay_activo);

        zxdesktop_configurable_icons_current_executing=pulsado_boton;

        menu_process_f_functions_by_action_name(id_funcion,1,-1,0,0);
        //printf("Despues procesar funcion\n");
    }

    salir_todos_menus=1;
    configurable_icon_is_being_moved=0;


}


//Mira si el boton pulsado esta redefinido por el usuario y lanza accion si conviene
int menu_inicio_handle_button_presses_userdef(int boton)
{
    boton--;

    //El 0 no esta permitido
    if (boton<0 || boton>=MAX_USERDEF_BUTTONS) return 0;

    enum defined_f_function_ids accion;

    int indice_tabla=defined_buttons_functions_array[boton];
    accion=menu_da_accion_direct_functions_indice(indice_tabla);

    //printf("Pulsado boton: %d\n",boton);

    if (accion!=F_FUNCION_DEFAULT) {
        menu_process_f_functions_by_action_index(indice_tabla,1,boton); //Indicar que viene de boton redefinido por el usuario
        return 1;
    }

    return 0;

}

void menu_inicio_handle_button_pressed_set_next_menu_position(int cual_boton)
{

    force_next_menu_position.v=1;

    int x,y;


    int alto_boton;
    int ancho_boton;
    menu_ext_desktop_buttons_get_geometry(&ancho_boton,&alto_boton,NULL,NULL,NULL);

    //Ajustar coordenada y
    int alto_texto=menu_char_height*menu_gui_zoom*zoom_y;
    y=(alto_boton/alto_texto); //antes sumaba +1, porque? de esa manera quedaba 1 linea de separación con los botones...

    //Ajustar coordenada x
    int origen_x=menu_get_origin_x_zxdesktop_aux(1);

    int offset_x=cual_boton*ancho_boton;
    int ancho_texto=menu_char_width*menu_gui_zoom*zoom_x;
    x=origen_x+(offset_x/ancho_texto);

    force_next_menu_position_x=x;
    force_next_menu_position_y=y;
}

//Gestionar pulsaciones de botones superiores
void menu_inicio_handle_button_presses(void)
{

    int pulsado_boton=menu_pressed_zxdesktop_button_which;

    //Avisar que se abren menus desde botones directo, para cambiar coordenada x,y
    //force_next_menu_position.v=1;
    //force_next_menu_position_which=menu_pressed_zxdesktop_button_which;
    menu_inicio_handle_button_pressed_set_next_menu_position(pulsado_boton);


    //Para que no vuelva a saltar
    menu_pressed_zxdesktop_button_which=-1;

    //Si pulsado con boton derecho, configurar botones
    if (menu_pressed_zxdesktop_button_which_right_button) {
        menu_pressed_zxdesktop_button_which_right_button=0;
        //printf("Pulsado con boton derecho sobre boton %d\n",pulsado_boton);

        //printf("salir_todos_menus %d menu_pressed_open_menu_while_in_menu.v %d "
        //        "mouse_pressed_background_window %d mouse_pressed_close_window %d\n",
        //        salir_todos_menus,menu_pressed_open_menu_while_in_menu.v,mouse_pressed_background_window,mouse_pressed_close_window);

        //Resetear ciertas variables para que no se cierre de nuevo el menu que vamos a abrir
        //salir_todos_menus=0;
        //menu_pressed_open_menu_while_in_menu.v=0;
        mouse_pressed_background_window=0;
        mouse_pressed_close_window=0;

        menu_zxdesktop_set_userdef_buttons_functions(0);
    }

    else {

        if (menu_inicio_handle_button_presses_userdef(pulsado_boton)==0) {

            //Inicializar indice search, indicar ruta vacia
            //innecesario con nuevo metodo de indexacion zxvision_index_search_init_menu_path_main_menu();

            switch (pulsado_boton) {
                case 0:
                    //Nada. Solo abrir el menu
                break;

                case 1:
                    //printf("antes smartload\n");
                    menu_smartload(0);
                    //printf("despues smartload\n");
                break;

                case 2:
                    menu_snapshot(0);
                break;

                case 3:
                    menu_machine_selection(0);
                break;

                case 4:
                    menu_audio(0);
                break;

                case 5:
                    menu_display_settings(0);
                break;

                case 6:
                    menu_storage(0);
                break;

                case 7:
                    menu_debug_main(0);
                break;

                case 8:
                    menu_network(0);
                break;

                case 9:
                    menu_windows(0);
                break;

                case 10:
                    menu_settings(0);
                break;

                case 11:
                    menu_help(0);
                break;

                case 12:
                    //printf("pulsado en boton de cerrar todos menus\n");
                    menu_pressed_close_all_menus.v=1;
                    menu_pressed_open_menu_while_in_menu.v=1;
                break;

                case 13:
                    menu_principal_salir_emulador(0);
                break;
            }

        }

    }


    //si ha generado error, no salir. Esto cubre el caso de que se ha abierto un menu que activa salir_todos_menus
    //pero dicho menu ha generado un error
    //Entonces iremos al menu principal
    if (if_pending_error_message) {
        salir_todos_menus=0;
    }

    else {
        //Cerrar todos menus. De lo contrario nos iriamos al menu principal si por ejemplo al pulsar el boton Network pulsamos flecha izquierda
        salir_todos_menus=1;
    }

    //Y decir que el siguiente menu ya no se abre desde boton y por tanto no se posiciona debajo del boton
    //Antes se quitaba el flag tambien en menu_dibuja_menu, pero ya no. Asi conseguimos que todos los menus
    //que se abran dependiendo del boton, queden debajo de dicho boton
    force_next_menu_position.v=0;

}

//Si se pulsa con boton izquierdo en alguna ventana con el menu cerrado
int pulsado_alguna_ventana_con_menu_cerrado=0;



int zxvision_simple_window_manager(int reopen_menu)
{
    //Ver si se habia pulsado en una ventana que habia en background
    //Aqui nos quedamos siempre que se pulse en otra ventana, digamos que esto es como el gestor de ventanas "sencillo"
    //es un tanto mágico pero también muy simple
    while (clicked_on_background_windows) {

        //Por si hemos llegado hasta aqui cerrando todos los menus al haber pulsado en otra ventana y la actual no permite background
        salir_todos_menus=0;


        clicked_on_background_windows=0;
        debug_printf(VERBOSE_DEBUG,"Clicked on background window, notified at the end of menus");

        //Para que al simular cerrado de ventana al pulsar shift+left/right en ventana que no permite conmutar, la cerramos
        //y alguien tiene que simular la liberacion de ese pulsado de boton sobre el cerrado de ventana
        //printf("Liberar boton de cierre ventana\n");
        mouse_pressed_close_window=0;

        if (which_window_clicked_on_background!=NULL) {
            //printf ("Ventana: %s\n",which_window_clicked_on_background->window_title);
            //printf ("Geometry name ventana: %s\n",which_window_clicked_on_background->geometry_name);

            char *geometry_name;

            geometry_name=which_window_clicked_on_background->geometry_name;

            if (geometry_name[0]!=0) {

                int indice=zxvision_find_known_window(geometry_name);

                if (indice<0) {
                        //debug_printf (VERBOSE_ERR,"Unknown window to restore: %s",geometry_name);
                }

                else {
                //Lanzar funcion que la crea

                    //Guardar funcion de texto overlay activo, para menus como el de visual memory por ejemplo, para desactivar  temporalmente
                    //void (*previous_function)(void);

                    //previous_function=menu_overlay_function;

                    //int antes_menu_overlay_activo=menu_overlay_activo;

                    debug_printf (VERBOSE_DEBUG,"Starting window %s",geometry_name);
                    menu_window_manager_window_went_background=0;

                    zxvision_known_window_names_array[indice].start(0);


                    //Restauramos funcion anterior de overlay
                    //set_menu_overlay_function(previous_function);

                    //menu_overlay_activo=antes_menu_overlay_activo;



                    //Logica 1 de Abrir el menu cuando está este window manager
                    /*
                    //Y reabriremos el menu cuando dejemos de conmutar entre ventanas
                    reopen_menu=1;

                    //Si desde ventana se ha pulsado F6, boton de background, pero sin conmutar a otra ventana ni pulsar F5, cerrar todo
                    if (menu_pressed_open_menu_while_in_menu.v==0 && !clicked_on_background_windows && menu_window_manager_window_went_background) {
                        reopen_menu=0;
                        debug_printf(VERBOSE_INFO,"From window manager, window goes to background, background windows is allowed even when menu closed, so closing menu");
                    }
                    */


                   //Logica 2 de Abrir el menu cuando está este window manager
                   //reabrir solo al pulsar F5

                   reopen_menu=0;

                   if (menu_pressed_open_menu_while_in_menu.v) {
                        reopen_menu=1;
                   }





                    //Se reabre el menu tambien si pulsada tecla F5 en cualquiera de los menus
                    if (menu_pressed_open_menu_while_in_menu.v) {
                        menu_pressed_open_menu_while_in_menu.v=0;
                    }



                }
            }

        }
    }

    return reopen_menu;

}




void menu_inicio_bucle(void)
{

    //printf("menu_inicio_bucle: menu_pressed_zxdesktop_button_which %d menu_pressed_zxdesktop_lower_icon_which %d pulsado_alguna_ventana_con_menu_cerrado %d\n",
    //            menu_pressed_zxdesktop_button_which,menu_pressed_zxdesktop_lower_icon_which,pulsado_alguna_ventana_con_menu_cerrado);

    //printf ("inicio de menu_inicio_bucle\n");

    //Si se ha pulsado el logo Z antes de abrir menu principal
    //Si no hiciera esto, se abriria menu, y luego se reabriria al cerrarlo,
    //dado que tenemos menu_pressed_open_menu_while_in_menu.v activado
    if (menu_pressed_open_menu_while_in_menu.v) {
        //printf ("Forgetting Z logo click action done before executing main menu\n");
        menu_pressed_open_menu_while_in_menu.v=0;
        salir_todos_menus=0;

    }

    //printf("salir todos menus en menu_inicio_bucle: %d\n",salir_todos_menus);

    //A partir de aqui ya se debe mostrar boton de cerrar todos menus
    //menu_mostrar_boton_close_all_menus.v=1;


    //Si reabrimos menu despues de conmutar entre ventanas en background
    int reopen_menu;

    do {
        reopen_menu=0;

        //Si se ha pulsado en algun boton de menu
        if (menu_pressed_zxdesktop_button_which>=0 || menu_pressed_zxdesktop_lower_icon_which>=0
            || menu_pressed_zxdesktop_configurable_icon_which>=0 || menu_pressed_zxdesktop_right_button_background>=0) {
            //printf ("Reabrimos menu para boton pulsado %d lower icon %d\n",menu_pressed_zxdesktop_button_which,menu_pressed_zxdesktop_lower_icon_which);
        }

        //menu en barra arriba del todo
        if (zxvision_topbar_menu_enabled.v) {
            menu_topbarmenu();

            printf("despues topbar menu. menu_pressed_open_menu_while_in_menu.v=%d\n",menu_pressed_open_menu_while_in_menu.v);

            //Aqui miramos si hay un evento de reapertura de menu, este evento menu_pressed_open_menu_while_in_menu.v
            //se definio para el menu "clasico" y no el topbar, aunque nos sirve igual
            //Se podria haber usado otro evento diferente? quiza si
            //esto viene de la funcion zxvision_handle_mouse_events y el if (if_menu_topbarmenu_enabled_and_pressed_bar ) {
            if (menu_pressed_open_menu_while_in_menu.v && if_menu_topbarmenu_pressed_bar() ) {
                menu_topbarmenu_pressed_bar=1;
                reopen_menu=1;
            }

        }

        else {

            //printf("antes menu_inicio_bucle_main. salir_todos_menus=%d\n",salir_todos_menus);
            menu_inicio_bucle_main();
            //printf("despues menu_inicio_bucle_main\n");

        }

        //Se reabre el menu tambien si pulsada tecla F5 en cualquiera de los menus
        if (menu_pressed_open_menu_while_in_menu.v) {
            menu_pressed_open_menu_while_in_menu.v=0;
            reopen_menu=1;
            //printf ("menu_inicio_bucle. Reabrimos menu\n");
        }

        //printf("antes zxvision_simple_window_manager\n");

        //Esta variable solo se comprueba antes y despues de llamar a zxvision_simple_window_manager, solo tiene sentido aqui
        //pressed_close_window_on_window_manager=0;

        reopen_menu=zxvision_simple_window_manager(reopen_menu);


        which_window_clicked_on_background=NULL;

        /*if (pressed_close_window_on_window_manager) {
            printf("A Window has been closed. Do not reopen menu\n");
            reopen_menu=0;
        }*/

        //pressed_close_window_on_window_manager=0;


        //Si hay que reabrirlo, resetear estado de salida de todos
        if (reopen_menu) {
            salir_todos_menus=0;
            //printf("menu_inicio_bucle. reopen_menu despues zxvision_simple_window_manager\n");

            /*
            este cls_menu_overlay solo parece ser necesario para una cosa:
            en estilo BeOS, el titulo de ventana activa tiene los botones de minimizar, maximizar, etc.
            al ser estilo BeOS, la franja del titulo no llega a la derecha del todo
            Una ventana en segundo plano no tiene esos botones, la franja del titulo solo contiene el titulo y es algo
            mas corta al no contener los botones
            Cuando se esta en una ventana, al pulsar F5, esa ventana se ira a background,
            aunque al llegar aqui aun no esta en background, y le sigue apareciendo el titulo con botones
            Al abrir el menu, ese titulo de ventana se refresca, la parte de la derecha no se altera,
            y aunque sale como tipo inactiva, los botones de antes se ven, cosa que no deberia
            Por tanto, hacemos aqui un cls_menu_overlay, para limpiar toda la pantalla (incluido, claro esta, los titulos y los botones),
            y que sea el siguiente
            refresco, al abrir el menu, el que dibuje las ventanas. Al ser ahora ya esa ventana una en background,
            no tiene botones visibles

            Nota: si en algun momento este cls_menu_overlay provoca algun efecto inesperado, como pantalla en blanco al abrir el menu
            y cosas asi, pues quito este cls_menu_overlay. Total solo es un "glitch" que afecta al estilo BeOS,
            al pulsar F5 cuando esta una ventana activa
            */
            cls_menu_overlay();

        }


        //Pero si se ha pulsado tecla que cierra todos menus, salor
        if (menu_pressed_close_all_menus.v) {
            debug_printf(VERBOSE_INFO,"Do not reopen main menu because key to close all menus has been pressed");
            menu_pressed_close_all_menus.v=0;
            reopen_menu=0;
        }



    } while (reopen_menu);

    //printf("menu_inicio_bucle. despues while reopen_menu\n");

    //Ya no se deberia mostrar boton de cerrar todos menus
    //menu_mostrar_boton_close_all_menus.v=0;

    if (textspeech_also_send_menu.v) textspeech_print_speech("Closing emulator menu and going back to emulated machine");


}



void menu_inicio_pre_retorno(void)
{
    menu_inicio_pre_retorno_reset_flags();

    reset_menu_overlay_function();
    menu_set_menu_abierto(0);

    //Necesario para cerrar submenus, por ejemplo si estamos en un item de menu con submenus,
    //y simplemente pulsamos fuera del menu, con lo que se simula pulsado ESC
    //pero deja submenus abiertos
    menu_dibuja_submenu_cierra_todos_submenus();

    //Ya no se deberia mostrar boton de cerrar todos menus
    //como desde aqui ya va bien, se quitó desde el otro sitio donde se deshabilita, en menu_inicio_bucle
    menu_mostrar_boton_close_all_menus.v=0;


    //Para refrescar border en caso de tsconf por ejemplo, en que el menu sobreescribe el border
    //modificado_border.v=1;

    timer_reset();

    //Y refrescar footer. Hacer esto para que redibuje en pantalla y no en layer de mezcla de menu
    menu_init_footer();
/*
menu_init_footer hace falta pues el layer de menu se borra y se queda negro en las zonas izquierda y derecha del footer
*/

    redraw_footer();


    //Redibujar ext desktop, para que no se vea el logo (logo solo aparece si menu abierto)
    menu_draw_ext_desktop();

    //Si salimos de menu y se ha pulsado dicha tecla F para activar la función, volver con menu overlay activo

    //O si siempre se fuerza
    if (menu_allow_background_windows && menu_multitarea) {
        if (always_force_overlay_visible_when_menu_closed) overlay_visible_when_menu_closed=1;
    }

    if (overlay_visible_when_menu_closed) {
        menu_overlay_activo=1;

        //Y redibujamos las ventanas, para que se vean los titulos sobretodo (pues los overlay en background no redibujan los titulos)
        //decir que ventana principal no esta activa, para indicar que están todas en background
        zxvision_reset_ventana_tipo_activa();

        //este footer me parece ya molesto.
        //generic_footertext_print_operating("BKWIND");

        //No queremos que al redibujar todas las ventanas se relean en speech, dado que estan en background
        zxvision_redraw_all_windows_no_speech();

        //Y luego dejar que se lea speech normalmente
        menu_speech_reset_tecla_pulsada();
    }

    //if (snapshot_in_ram_pending_message_footer) {
    //   generic_footertext_print_operating("REWIND");
    //}

    //Quitar pausado de emulacion que se pueda haber activado desde un step mode de debug cpu
    //printf("Desactivar menu_emulation_paused_on_menu_by_debug_step_mode al cerrar menu\n");
    menu_emulation_paused_on_menu_by_debug_step_mode=0;

}

void menu_process_f_functions_by_action_index(int indice,int si_pulsado_boton_redefinido,int boton)
{

    //printf("numero boton: %d\n",boton);

    int id_funcion=menu_da_accion_direct_functions_indice(indice);

    menu_process_f_functions_by_action_name(id_funcion,0,menu_button_f_function_index,si_pulsado_boton_redefinido,boton);
}

void menu_process_f_functions(void)
{



    int indice=menu_button_f_function_index;

    int indice_tabla=defined_f_functions_keys_array[indice];

    //printf ("Menu process Tecla: F%d Accion: %s\n",indice+1,defined_direct_functions_array[indice_tabla].texto_funcion);

    menu_process_f_functions_by_action_index(indice_tabla,0,0);

}


void menu_inicio_reset_emulated_keys(void)
{
    //Resetear todas teclas excepto bits de puertos especiales y esperar a no pulsar tecla
    z80_byte p_puerto_especial1,p_puerto_especial2,p_puerto_especial3,p_puerto_especial4;

    p_puerto_especial1=puerto_especial1;
    p_puerto_especial2=puerto_especial2;
    p_puerto_especial3=puerto_especial3;
    p_puerto_especial4=puerto_especial4;

    reset_keyboard_ports();

    //Restaurar estado teclas especiales, para poder esperar a liberar dichas teclas, por ejemplo
    puerto_especial1=p_puerto_especial1;
    puerto_especial2=p_puerto_especial2;
    puerto_especial3=p_puerto_especial3;
    puerto_especial4=p_puerto_especial4;


    //Desactivar fire, por si esta disparador automatico
    joystick_release_fire(1,0);

    //printf("menu_inicio_reset_emulated_keys antes menu_espera_no_tecla\n");

    menu_espera_no_tecla_no_mouse_movido();
    //menu_espera_no_tecla();

    //printf("menu_inicio_reset_emulated_keys despues menu_espera_no_tecla\n");
}

void handle_menu_open_cpu_step(void)
{
                //printf("Entrando step en menu\n");
            //Entrada
            //menu_espera_no_tecla();
            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

            remote_ack_enter_cpu_step.v=1; //Avisar que nos hemos enterado
            //Mientras no se salga del modo step to step del remote protocol
            while (menu_event_remote_protocol_enterstep.v) {
                timer_sleep(100);

                //printf("En menu en cpu step\n");

    #ifdef MINGW
                int antes_menu_abierto=menu_abierto;
                menu_abierto=0; //Para que no aparezca en gris al refrescar
                    scr_refresca_pantalla();
                menu_abierto=antes_menu_abierto;

    #endif
                //Si se pulsa ESC, salir de cpu-step
                scr_actualiza_tablas_teclado();


                if ((puerto_especial1&1)==0 && zrcp_easter_egg_running.v==0) {
                    debug_printf(VERBOSE_DEBUG,"Exiting cpu-step by pressing ESC");
#ifndef NETWORKING_DISABLED
                    remote_cpu_exit_step_continue_restore_multitask();
#endif
                }

            }

            debug_printf (VERBOSE_DEBUG,"Exiting remote enter step from menu");

            //Salida
            cls_menu_overlay();

}

void enable_zxdesktop_and_background(void)
{
    //No esta habilitado, habilitar
    if (!screen_ext_desktop_enabled) {


        //establecemos un minimo de ancho de zxdesktop (512/zoom_x) al habilitar
        //ideal para maquinas que usualmente usan zoom 1, como tbblue o ql
        zxdesktop_width=ZXDESKTOP_MINIMUM_WIDTH_BY_BUTTON;
        //printf("Generando valor nuevo\n");


        //aparte de conmutar estado, decimos tambien que los menus se abriran en zona zx desktop
        screen_ext_desktop_place_menu=1;

        //Activar zx desktop
        menu_ext_desk_settings_enable(0);

        //Activar background windows si no estaba
        if (!menu_allow_background_windows) menu_allow_background_windows=1;

        //Background windows incluso con menu cerrado
        if (!always_force_overlay_visible_when_menu_closed) always_force_overlay_visible_when_menu_closed=1;

    }
}

//menu principal
void menu_inicio(void)
{

    //printf ("inicio menu_inicio. menu_event_remote_protocol_enterstep.v=%d\n",menu_event_remote_protocol_enterstep.v);
    pulsado_alguna_ventana_con_menu_cerrado=0;

    menu_topbarmenu_pressed_bar=0;

    int indice_abrir_ventana_sin_multitarea=-1;


    if (zxvision_switch_to_window_on_open_menu) {
        //printf("zxvision_switch_to_window_on_open_menu\n");
        zxvision_switch_to_window_on_open_menu=0;

        //printf ("0 salir_todos_menus: %d\n",salir_todos_menus);


        if (!menu_allow_background_windows || !menu_multitarea) {
            //abrirla tal cual sin mas
            indice_abrir_ventana_sin_multitarea=zxvision_find_known_window(zxvision_switch_to_window_on_open_menu_name);
            //se abre un poco mas adelante

        }

        else {


            //¿TODO?: si no existe ventana, crearla
            zxvision_window *ventana_abrir=zxvision_find_window_in_background(zxvision_switch_to_window_on_open_menu_name);

            //printf("ventana_abrir=%p\n",ventana_abrir);

            if (ventana_abrir==NULL) {
                zxvision_restore_one_window(zxvision_switch_to_window_on_open_menu_name);
                //buscarla de nuevo
                ventana_abrir=zxvision_find_window_in_background(zxvision_switch_to_window_on_open_menu_name);

                //printf("ventana_abrir 2=%p\n",ventana_abrir);
            }

            if (ventana_abrir!=NULL) {
                //printf("ventana_abrir 3=%p\n",ventana_abrir);

                debug_printf(VERBOSE_DEBUG,"Restoring window %s",zxvision_switch_to_window_on_open_menu_name);
                zxvision_handle_mouse_ev_switch_back_wind(ventana_abrir);
                pulsado_alguna_ventana_con_menu_cerrado=1;

                //Esto se ha habilitado desde zxvision_handle_mouse_ev_switch_back_wind porque esta pensado para ejecutarse con el menu abierto
                salir_todos_menus=0;

                //Idem para esto
                mouse_pressed_background_window=0;
            }

        }

        //printf ("en zxvision_switch_to_window_on_open_menu salir_todos_menus: %d\n",salir_todos_menus);




    }

//printf ("0inicio menu_inicio\n");

    //Comprobar si se ha pulsado un boton para colorearlo
    if (mouse_left) {
        //Si pulsado en alguna ventana
        if (menu_allow_background_windows && menu_multitarea && always_force_overlay_visible_when_menu_closed) {
            int absolute_mouse_x,absolute_mouse_y;

            menu_calculate_mouse_xy_absolute_interface(&absolute_mouse_x,&absolute_mouse_y);

            //Vamos a ver en que ventana se ha pulsado, si tenemos background activado
            zxvision_window *ventana_pulsada;

            //Si pulsamos en la ventana que esta arriba
            if (zxvision_coords_in_front_window(absolute_mouse_x,absolute_mouse_y) && zxvision_show_minimized(zxvision_current_window)) {
                //printf("abierto menu y pulsado en ventana en foreground: %s\n",zxvision_current_window->window_title);

                zxvision_handle_mouse_ev_switch_back_wind(zxvision_current_window);

                //printf("despues de conmutar ventana\n");
                pulsado_alguna_ventana_con_menu_cerrado=1;

            }

            //O en alguna de background
            else {

                ventana_pulsada=zxvision_coords_in_below_windows(zxvision_current_window,absolute_mouse_x,absolute_mouse_y);


                if (ventana_pulsada!=NULL && zxvision_show_minimized(ventana_pulsada)) {
                    //printf("abierto menu y pulsado en ventana en background: %s\n",ventana_pulsada->window_title);

                    zxvision_handle_mouse_ev_switch_back_wind(ventana_pulsada);

                    //printf("despues de conmutar ventana\n");
                    pulsado_alguna_ventana_con_menu_cerrado=1;

                }
            }



        }

        //Si pulsado en boton pero no pulsado en ventanas (ventanas siempre estan por encima y por tanto tienen prioridad)
        //Hay que ver antes pulsado_alguna_ventana_con_menu_cerrado; si metemos en un solo if las dos condiciones
        //!pulsado_alguna_ventana_con_menu_cerrado y zxvision_if_mouse_in_zlogo_or_buttons_desktop(), la segunda
        //hace alterar el valor de menu_pressed_zxdesktop_button_which y por tanto estariamos diciendo que se ha pulsado en boton
        if (!pulsado_alguna_ventana_con_menu_cerrado) {
            //printf("llamar a if mouse in logo desde 26214\n");
            if (zxvision_if_mouse_in_zlogo_or_buttons_desktop()) {
                //printf("zxvision_if_mouse_in_zlogo_or_buttons_desktop 4\n");
                debug_printf(VERBOSE_DEBUG,"Pressed in a button from menu_inicio. Mouse is dragging: %d mouse_movido: %d",mouse_is_dragging,mouse_movido);

                //Dibujamos de otro color ese boton
                //que boton=menu_pressed_zxdesktop_button_which

                //menu_draw_ext_desktop_dibujar_boton_pulsado(menu_pressed_zxdesktop_button_which);
                menu_draw_ext_desktop_dibujar_boton_or_lower_icon_pulsado();

            }
        }

        //Si pulsado en boton pero no pulsado en ventanas (ventanas siempre estan por encima y por tanto tienen prioridad)
        //Hay que ver antes pulsado_alguna_ventana_con_menu_cerrado por la misma razon que la explicada anterior con zxvision_if_mouse_in_zlogo_or_buttons_desktop
        if (!pulsado_alguna_ventana_con_menu_cerrado) {

            //printf("pulsado_alguna_ventana_con_menu_cerrado\n");

            //Pulsado en boton de aumentar zx desktop en ancho
            if (zxvision_if_mouse_in_lower_button_enlarge_zxdesktop_width()) {


                //decir que mouse no se ha movido, porque si no, nos quedariamos en bucle continuamente en menu_espera_no_tecla
                mouse_movido=0;
                menu_espera_no_tecla();

                //No esta habilitado, habilitar
                if (!screen_ext_desktop_enabled) {

                    enable_zxdesktop_and_background();

                }

                //Si ya habilitado, aumentar zx desktop de ancho
                else {
                    if (zxdesktop_width<ZXDESKTOP_MAXIMUM_WIDTH_BY_BUTTON) {
                        menu_ext_desk_settings_width_enlarge_reduce(1);
                    }
                }


                //Importante lo siguiente, si hemos iniciado con:
                //./zesarux --noconfigfile --disablebetawarning 10.1-B1 --nowelcomemessage
                //Entonces en este punto no se ha establecido overlay function, como se hace algo mas abajo en esta funcion de menu_inicio
                //y lo que sucede al salir con menu_inicio_pre_retorno() es que acaba generando segfault
                //Esto solo es necesario pues hemos habilitado always_force_overlay_visible_when_menu_closed
                //La razon exacta del segfault es porque al hacer el set_menu_overlay_function se llama a scr_reallocate_layers_menu,
                //que es quien asigna la memoria de overlay menu, y quien lo asigna para el tamaño adecuado nuevo (teniendo en cuenta la ampliacion del zxdesktop)
                //CUIDADO: lo mismo puede pasar en codigo justo por arriba de este, y por debajo de este hasta donde hay un set_menu_overlay_function,
                //en otras condiciones que pueden llegar a petar
                //teniendo en cuenta que no se ha establecido aun set_menu_overlay_function
                //aunque esto es casi remotamente imposible, pues aqui se da el caso de un aumento de tamaño de zxdesktop
                //pero que aun no se ha establecido la funcion de overlay que es la que asigna la memoria para layers del menu
                //Otra alternativa en vez de llamar a set_menu_overlay_function seria scr_init_layers_menu
                set_menu_overlay_function(normal_overlay_texto_menu);

                //menu_set_menu_abierto(0);
                menu_inicio_pre_retorno();

                //Volver a escribir los botones
                zxdesktop_make_switchbutton_visible();

                //Este clear_putpixel_cache necesario, la razon no esta clara pero sucede lo mismo en zxdesktop_make_switchbutton_visible
                clear_putpixel_cache();

                return;

            }

            if (zxvision_if_mouse_in_lower_button_reduce_zxdesktop_width()) {
                //Pulsado en boton de reducir zx desktop en ancho

                //decir que mouse no se ha movido, porque si no, nos quedariamos en bucle continuamente en menu_espera_no_tecla
                mouse_movido=0;
                menu_espera_no_tecla();

                //Si esta habilitado
                if (screen_ext_desktop_enabled) {
                    //lo va a ocultar. Preservar valor anterior

                    //printf("Conservando valor anterior\n");

                    //Si esta por debajo del limite, desactivar zx desktop
                    if (zxdesktop_width<=ZXDESKTOP_MINIMUM_WIDTH_BY_BUTTON) {
                        //Desactivar zx desktop
                        menu_ext_desk_settings_enable(0);
                    }

                    //Si esta en el limite, reducir ancho
                    else {
                        menu_ext_desk_settings_width_enlarge_reduce(0);
                    }
                }

                set_menu_overlay_function(normal_overlay_texto_menu);

                //menu_set_menu_abierto(0);
                menu_inicio_pre_retorno();

                //Volver a escribir los botones
                zxdesktop_make_switchbutton_visible();

                //Este clear_putpixel_cache necesario, la razon no esta clara pero sucede lo mismo en zxdesktop_make_switchbutton_visible
                clear_putpixel_cache();

                return;

            }

            //Pulsado en boton de aumentar zx desktop en alto
            if (zxvision_if_mouse_in_lower_button_enlarge_zxdesktop_height()) {


                //decir que mouse no se ha movido, porque si no, nos quedariamos en bucle continuamente en menu_espera_no_tecla
                mouse_movido=0;
                menu_espera_no_tecla();


                //aumentar zx desktop de alto

                if (zxdesktop_height<ZXDESKTOP_MAXIMUM_HEIGHT_BY_BUTTON) {
                    menu_ext_desk_settings_height_enlarge_reduce(1);
                }


                set_menu_overlay_function(normal_overlay_texto_menu);

                //menu_set_menu_abierto(0);
                menu_inicio_pre_retorno();

                //Volver a escribir los botones
                zxdesktop_make_switchbutton_visible();

                //Este clear_putpixel_cache necesario, la razon no esta clara pero sucede lo mismo en zxdesktop_make_switchbutton_visible
                clear_putpixel_cache();

                return;

            }

            if (zxvision_if_mouse_in_lower_button_reduce_zxdesktop_height()) {
                //Pulsado en boton de reducir zx desktop en alto

                //decir que mouse no se ha movido, porque si no, nos quedariamos en bucle continuamente en menu_espera_no_tecla
                mouse_movido=0;
                menu_espera_no_tecla();

                //Si esta por debajo del limite, ponerlo a 0
                if (zxdesktop_height<=ZXDESKTOP_MINIMUM_HEIGHT_BY_BUTTON) {
                    zxdesktop_height=0;
                }

                //Si esta en el limite, reducir ancho
                else {
                    menu_ext_desk_settings_height_enlarge_reduce(0);
                }


                set_menu_overlay_function(normal_overlay_texto_menu);

                //menu_set_menu_abierto(0);
                menu_inicio_pre_retorno();

                //Volver a escribir los botones
                zxdesktop_make_switchbutton_visible();

                //Este clear_putpixel_cache necesario, la razon no esta clara pero sucede lo mismo en zxdesktop_make_switchbutton_visible
                clear_putpixel_cache();

                return;

            }

        }

        //Si pulsado en topbar menu
        if (!pulsado_alguna_ventana_con_menu_cerrado) {
            if (if_menu_topbarmenu_pressed_bar() ) menu_topbarmenu_pressed_bar=1;
        }

        //printf("menu_inicio: menu_pressed_zxdesktop_button_which %d menu_pressed_zxdesktop_lower_icon_which %d pulsado_alguna_ventana_con_menu_cerrado %d\n",
        //    menu_pressed_zxdesktop_button_which,menu_pressed_zxdesktop_lower_icon_which,pulsado_alguna_ventana_con_menu_cerrado);

    }

    if (mouse_right) {
        DBG_PRINT_ZXVISION_EVENTS VERBOSE_INFO,"ZXVISION_EVENTS: Opened menu by right click");

        int pulsado_alguna_ventana_con_menu_cerrado_boton_derecho=0;


        //Si pulsado en alguna ventana
        if (menu_allow_background_windows && menu_multitarea && always_force_overlay_visible_when_menu_closed) {
            int absolute_mouse_x,absolute_mouse_y;

            menu_calculate_mouse_xy_absolute_interface(&absolute_mouse_x,&absolute_mouse_y);

            //Vamos a ver en que ventana se ha pulsado, si tenemos background activado
            zxvision_window *ventana_pulsada;

            //Si pulsamos en la ventana que esta arriba
            if (zxvision_coords_in_front_window(absolute_mouse_x,absolute_mouse_y)) {

                pulsado_alguna_ventana_con_menu_cerrado_boton_derecho=1;

            }

            //O en alguna de background
            else {

                ventana_pulsada=zxvision_coords_in_below_windows(zxvision_current_window,absolute_mouse_x,absolute_mouse_y);

                if (ventana_pulsada!=NULL) {

                    pulsado_alguna_ventana_con_menu_cerrado_boton_derecho=1;

                }
            }



        }


        //Si pulsado en boton pero no pulsado en ventanas (ventanas siempre estan por encima y por tanto tienen prioridad)
        //Hay que ver antes pulsado_alguna_ventana_con_menu_cerrado; si metemos en un solo if las dos condiciones
        //!pulsado_alguna_ventana_con_menu_cerrado y zxvision_if_mouse_in_zlogo_or_buttons_desktop(), la segunda
        //hace alterar el valor de menu_pressed_zxdesktop_button_which y por tanto estariamos diciendo que se ha pulsado en boton

        if (!pulsado_alguna_ventana_con_menu_cerrado && !pulsado_alguna_ventana_con_menu_cerrado_boton_derecho) {
            //printf("llamar a if mouse in logo desde 26462\n");
            //TODO: aqui se deberia llamar realmente a zxvision_if_mouse_in_zlogo_or_buttons_desktop_boton_derecho,
            //o bien una funcion unificada que detecte si mouse_left o mouse_right
            if (zxvision_if_mouse_in_zlogo_or_buttons_desktop()) {
                //printf("zxvision_if_mouse_in_zlogo_or_buttons_desktop 5\n");
                DBG_PRINT_ZXVISION_EVENTS VERBOSE_INFO,"ZXVISION_EVENTS: Pressed on a button with right click");
                //debug_printf(VERBOSE_DEBUG,"Pressed in a button from menu_inicio. Mouse is dragging: %d mouse_movido: %d",mouse_is_dragging,mouse_movido);

                //Dibujamos de otro color ese boton
                //que boton=menu_pressed_zxdesktop_button_which

                //menu_draw_ext_desktop_dibujar_boton_pulsado(menu_pressed_zxdesktop_button_which);
                menu_draw_ext_desktop_dibujar_boton_or_lower_icon_pulsado();

            }
        }
    }

                //Esto se ha puesto a 1 antes desde zxvision_if_mouse_in_zlogo_or_buttons_desktop,
                //indirectamente cuando llama a menu_calculate_mouse_xy_absolute_interface_pixel

                mouse_movido=0;

    //Pulsado boton salir del emulador, en drivers xwindows, sdl, etc, en casos con menu desactivado, sale del todo
    if (menu_button_exit_emulator.v && (menu_desactivado.v || menu_desactivado_andexit.v)
        ) {
        end_emulator_autosave_snapshot();
    }

    //Menu desactivado y volver
    if (menu_desactivado.v) {
        menu_inicio_pre_retorno_reset_flags();

        menu_set_menu_abierto(0);
        return;
    }

//printf ("1inicio menu_inicio\n");



    //Menu desactivado y salida del emulador
    if (menu_desactivado_andexit.v) end_emulator_autosave_snapshot();

    //No permitir aparecer osd keyboard desde aqui. Luego se reactiva en cada gestion de tecla
    osd_kb_no_mostrar_desde_menu=1;

    menu_contador_teclas_repeticion=CONTADOR_HASTA_REPETICION;


    //Resetear todas teclas. Por si esta el spool file activo. Conservando estado de tecla ESC pulsada o no
    /*z80_byte estado_ESC=puerto_especial1&1;
    reset_keyboard_ports();

    //estaba pulsado ESC
    if (!estado_ESC) puerto_especial1 &=(255-1);

    */

    /*No liberar teclas ni esperar a no pulsar teclas si solo hay evento printe, etc
      Pero tener en cuenta que en los eventos se pueden abrir menus tambien
    /*/

    int liberar_teclas_y_esperar=1; //Si se liberan teclas y se espera a liberar teclado


    //printf("antes breakpoint exception. menu_breakpoint_exception.v=%d\n",menu_breakpoint_exception.v);


    if (menu_breakpoint_exception.v) {
        if (!debug_if_breakpoint_action_menu(catch_breakpoint_index)) {
            //Accion no es de abrir menu
            /*
            Tecnicamente, haciendo esto, no estamos controlando que se dispare un evento de breakpoin accion, por ejemplo , printe,
            y a la vez, se genere otro evento, por ejemplo quickload. En ese caso sucederia que al llamar a quickload, no se liberarian
            las teclas en la maquina emulada ni se esperaria a no pulsar tecla
            Para evitar este remoto caso, habria que hacer que no se liberen las teclas aqui al principio, sino que cada evento
            libere teclas por su cuenta
            */
            liberar_teclas_y_esperar=0;
        }

        //printf("despues if breakpoint exception. menu_breakpoint_exception.v=%d\n",menu_breakpoint_exception.v);
    }

//printf ("2inicio menu_inicio\n");

    //printf("0 antes liberar_teclas_y_esperar\n");
    //printf("PC=%04XH\n",reg_pc);

    if (liberar_teclas_y_esperar) {
        //Si se arrastra icono con menu cerrado se llega aqui,
        //si no hay ninguna ventana abierta, el icono no se arrastra porque de eso depende zxvision_current_window que no sea NULL
        //(en handle mouse events)
        //Quiza abrir menu solo para que haya ventana abierta?
        //De momento la solución es que el usuario, si quiere mover un icono desde el menu cerrado y no hay ninguna ventana abierta,
        //tiene que abrir el menu y luego ya puede arrastrar

        menu_inicio_reset_emulated_keys();
    }

    //printf("1 despues liberar_teclas_y_esperar\n");
    //printf("PC=%04XH\n",reg_pc);

    //printf ("after menu_inicio_reset_emulated_keys\n");

    //Si se ha pulsado tecla de OSD keyboard, al llamar a espera_no_tecla, se abrira osd y no conviene.



            //printf ("Event open menu: %d\n",menu_event_open_menu.v);

    //printf ("after menu_espera_no_tecla\n");


        if (!strcmp(scr_new_driver_name,"stdout")) {
        //desactivar menu multitarea con stdout
        menu_multitarea=0;
        }

    //simpletext no soporta menu
    //null no soporta menu
        if (
            !strcmp(scr_new_driver_name,"simpletext") ||
            !strcmp(scr_new_driver_name,"null")

        ) {


        //Soportar cpu step
        if (menu_event_remote_protocol_enterstep.v) {
            handle_menu_open_cpu_step();
        }
        else {
            printf ("Can not open menu: this video driver does not support menu.\n");
        }

        menu_inicio_pre_retorno();
        return;
        }



    if (menu_if_emulation_paused() ) {
        audio_playing.v=0;
    }


//printf ("3inicio menu_inicio\n");

    //printf("2\n");
    //quitar splash text por si acaso
    menu_splash_segundos=1;
    reset_welcome_message();


    cls_menu_overlay();
    set_menu_overlay_function(normal_overlay_texto_menu);
    overlay_visible_when_menu_closed=0;


    //Y refrescar footer. Hacer esto para que redibuje en pantalla y no en layer de mezcla de menu
    //menu_init_footer();
    menu_clear_footer();
    redraw_footer();

    //Establecemos variable de salida de todos menus a 0

    //no alterar variable de salir si hemos pulsado en alguna ventana, para que cierre el menu y vaya a esa ventana
    salir_todos_menus=0;

    //printf ("inicio menu_inicio2 salir todos menus: %d\n",salir_todos_menus);

    //a partir de este momento ya mostrar boton de cerrar todos menus
    //como desde aqui ya va bien, se quitó desde el otro sitio donde se habilita, en menu_inicio_bucle
    menu_mostrar_boton_close_all_menus.v=1;


    if (indice_abrir_ventana_sin_multitarea>=0) {
        //printf("Iniciando ventana tal cual porque no tenemos multitarea o no permitido background windows\n");
        zxvision_known_window_names_array[indice_abrir_ventana_sin_multitarea].start(0);
    }

    if (zesarux_has_been_downgraded.v) {
        zesarux_has_been_downgraded.v=0;

        if (save_configuration_file_on_exit.v==0) {
            menu_generic_message_format("Downgraded version","It seems you have downgraded ZEsarUX from %s to %s.\n"
                "If there is any unknown parameter on the configuration file, from the moment that parameter is detected, the rest of the parameters are tried to be read\n",
                last_version_text_string,EMULATOR_VERSION);
        }
        else {
            char buffer_mensaje[1024];
            sprintf(buffer_mensaje,"It seems you have downgraded ZEsarUX from %s to %s.\n"
                "If there is any unknown parameter on the configuration file, from the moment that parameter is detected, the rest of the parameters are tried to be read.\n"
                "Autosave configuration is enabled. Do you want to keep it enabled?",
                last_version_text_string,EMULATOR_VERSION);

                int valor_opcion=1;
                zxvision_menu_generic_message_setting("Downgraded version",buffer_mensaje,"Keep autosave",&valor_opcion);

                if (!valor_opcion) {
                    save_configuration_file_on_exit.v=0;
                    menu_generic_message_format("Disabled save settings","Ok. Current configuration settings will NOT be saved on exit");
                }
        }

    }

    if (zesarux_first_start) {
        osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd
        zesarux_first_start=0;
        //Y si no se ha deshabilitado por opcion de linea de comandos
        if (!first_start_wizard_disabled) {
            menu_first_start_wizard();
        }
        appeared_zesarux_first_start=1;

        cls_menu_overlay();
    }

    if (activated_in_memoriam_david.v) {
        osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd
        activated_in_memoriam_david.v=0;
        menu_in_memoriam_david_about(0);

        cls_menu_overlay();
    }

//printf ("4inicio menu_inicio. salir_todos_menus=%d\n",salir_todos_menus);

    //Si first aid al inicio
    if (menu_first_aid_must_show_startup) {
        menu_first_aid_must_show_startup=0;
        menu_first_aid_title(string_config_key_aid_startup,"First aid of the day");
        //No mostrara nada mas que esto y luego volvera del menu
    }

    //Si detectado joystick real y
    if (realjoystick_detected_startup) {
        realjoystick_detected_startup=0;
        menu_first_aid_title("realjoystick_detected","Joystick detected");
    }

//printf ("5inicio menu_inicio\n");

    if (menu_button_osdkeyboard.v) {
        //menu_espera_no_tecla();
        menu_onscreen_keyboard(0);
        osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer
        cls_menu_overlay();
      }


    else {


        //Evento para generar siguiente tecla
        if (menu_button_osd_adv_keyboard_return.v) {
            //printf ("Debe abrir menu adventure keyboard\n");
            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

            menu_osd_adventure_keyboard_next();
            //menu_osd_adventure_keyboard(0);
            cls_menu_overlay();

        }

        //Evento de abrir menu adventure text
        if (menu_button_osd_adv_keyboard_openmenu.v) {
            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

                    menu_osd_adventure_keyboard(0);
                    cls_menu_overlay();
                            //printf ("Returning from osd keyboard\n");
            }


        //Gestionar pulsaciones directas de teclado o joystick
        if (menu_button_smartload.v) {
            //Pulsado smartload
            //menu_button_smartload.v=0;

            //para evitar que entre con la pulsacion de teclas activa
            //menu_espera_no_tecla_con_repeticion();
            //menu_espera_no_tecla();
            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

            menu_smartload(0);
            cls_menu_overlay();
        }

        if (menu_button_exit_emulator.v) {
            //Pulsado salir del emulador
            //para evitar que entre con la pulsacion de teclas activa
            //menu_espera_no_tecla_con_repeticion();
            //menu_espera_no_tecla();
            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

            menu_exit_emulator(0);
            cls_menu_overlay();
        }


            //Abrir menu lanzador de juegos
            //Esto antes de la gestion de drag & drop, porque si no, si estuviera despues,
            //al hacer drag & drop entraria dos veces aqui (o se podria corregir aqui haciendo menu_event_pending_zmenu_file_menu_open.v=0;
            if (menu_event_open_zmenu_file.v) {
                zmenu_parse_file(menu_open_zmenu_file_path);
            }


            if (menu_event_drag_drop.v) {
                debug_printf(VERBOSE_INFO,"Received drag and drop event with file %s",quickload_file);
                //printf("Received drag and drop event with file %s\n",quickload_file);
                //Entrado drag-drop de archivo
                //para evitar que entre con la pulsacion de teclas activa
                //menu_espera_no_tecla_con_repeticion();
                //menu_espera_no_tecla();
                osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd
                quickfile=quickload_file;

                //Si es archivo comprimido
                if (menu_util_file_is_compressed(quickload_file) ) {
                    debug_printf (VERBOSE_DEBUG,"Is a compressed file");


                    //guardamos directorio actual
                    char directorio_actual[PATH_MAX];
                    getcwd(directorio_actual,PATH_MAX);

                    //Cambiamos a esa ruta. Requerido por menu_filesel_uncompress
                    char ruta_a_comprimido[PATH_MAX];
                    util_get_dir(quickload_file,ruta_a_comprimido);
                    char archivo_sin_ruta[PATH_MAX];
                    util_get_file_no_directory(quickload_file,archivo_sin_ruta);

                    zvfs_chdir(ruta_a_comprimido);

                    //Descomprimimos
                    char tmpdir[PATH_MAX];
                    int retorno=menu_filesel_uncompress(archivo_sin_ruta,tmpdir);

                    //volvemos a directorio inicial
                    zvfs_chdir(directorio_actual);

                    //printf("retorno: %d\n",retorno);

                    if (!retorno) {
                        //quickfile=tmpdir;

                        //Necesario decirle a smartload que ruta es directorio (acabarlo con /)
                        sprintf(quickload_file,"%s/",tmpdir);
                        //printf("dir: [%s]\n",quickload_file);
                        menu_smartload(0);
                    }

                }

                else {
                    last_filesused_insert(quickload_file); //Agregar a lista de archivos recientes

                    if (quickload(quickload_file)) {
                        debug_printf (VERBOSE_ERR,"Unknown file format");

                        //menu_generic_message("ERROR","Unknown file format");
                    }
                }
                menu_muestra_pending_error_message(); //Si se genera un error derivado del quickload
                cls_menu_overlay();
            }



        //ha saltado un breakpoint
        //esto ahora se gestiona diferente, abriendo la ventana directamente por el gestor multitarea
        /*
        if (menu_breakpoint_exception.v) {
            //Ver tipo de accion para ese breakpoint
            //printf ("indice breakpoint & accion : %d\n",catch_breakpoint_index);
            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd


            //Si accion nula o menu o break
            if (debug_if_breakpoint_action_menu(catch_breakpoint_index)) {

                //menu_espera_no_tecla();
        //desactivamos multitarea, guardando antes estado multitarea


                int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
                menu_emulation_paused_on_menu=1;

                    audio_playing.v=0;
                //printf ("pc: %d\n",reg_pc);

                menu_breakpoint_fired(catch_breakpoint_message);


                menu_debug_registers(0);


                //printf("Despues de debug cpu, menu_emulation_paused_on_menu_by_debug_step_mode=%d\n",menu_emulation_paused_on_menu_by_debug_step_mode);

                //restaurar estado multitarea

                menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;

        cls_menu_overlay();


                //Y despues de un breakpoint hacer que aparezca el menu normal y no vuelva a la ejecucion
                //if (!salir_todos_menus) menu_inicio_bucle();

                //Lo habitual aqui seria:
                //if (!salir_todos_menus) menu_inicio_bucle();

                //Pero, si se conmutase a otra ventana, se activa salir_todos_menus y acabariamos cerrando todas las ventanas

                //En cambio lo que hacemos es que, Despues de un breakpoint y al salir de debug cpu, abrir menu. Independientemente de si se cierra ventana con ESC,
                //si se intenta conmutar a otra ventana, o si se pulsa tecla de cerrar todos menus
                //Con esto conseguimos que si estamos en modo step,
                //se conserve modo step en el menu. De otra manera, si cuando se conmuta a otra ventana,
                //esto activa salir_todos_menus
                //aqui se saldria y no entraria en en bucle de gestion de multitarea


                //TODO: idealmente aqui habria que mirarse si:
                //si se ha pulsado en otra ventana, conmutar a ella. Esto habitualmente se mira con el menu cerrado y justo se abre,
                //y desde el zxvision_simple_window_manager. Ambas funciones son innaccesibles desde aqui, y por tanto es uyn poco liado
                //tambien habria que ver si se cierran todas las ventanas, no llamar a menu_inicio_bucle
                //Otra manera de lanzar breakpoint seria que cuando hay un breakpoint se lanzase un comando de:
                //abre ventana debug cpu, pero esto no deberia abrirse como una pulsacion de tecla F, pues ahi no entra tampoco
                //en el zxvision_simple_window_manager, sino que tendria que haber una manera que, desde el menu cerrado, saltase a
                //zxvision_simple_window_manager con una ventana concreta
                //printf("pulsado_alguna_ventana_con_menu_cerrado: %d\n",pulsado_alguna_ventana_con_menu_cerrado);
                menu_inicio_bucle();
            }

            else {
                //Gestion acciones. Se gestionan desde el mismo core de debug y aqui no deberian escalarse nunca
                //debug_run_action_breakpoint(debug_breakpoints_actions_array[catch_breakpoint_index]);
            }


        }
        */
    //printf("antes si enterstep. menu_event_remote_protocol_enterstep.v=%d\n",menu_event_remote_protocol_enterstep.v);
        if (menu_event_remote_protocol_enterstep.v) {
            handle_menu_open_cpu_step();
        }

        if (menu_button_f_function.v) {

            //Si se reabre menu, resetear flags de teclas pulsadas especiales
            //Esto evita por ejemplo que al abrir menu con F5, si se entra a submenu, se crea que hemos pulsado F5 y cierre el menu y vuelva a abrir menu principal
            menu_button_f_function.v=0;

            //printf ("pulsada tecla de funcion\n");
            //Entrada
            //menu_espera_no_tecla();
            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

            //Procesar comandos F

            //Procesamos cuando se pulsa tecla F concreta desde joystick
            if (menu_button_f_function_action==0) menu_process_f_functions();
            else {
                //O procesar cuando se envia una accion concreta, normalmente viene de evento de joystick
                menu_process_f_functions_by_action_name(menu_button_f_function_action,0,-1,0,0);
                menu_button_f_function_action=0;
            }

            menu_muestra_pending_error_message(); //Si se genera un error derivado de funcion F
            cls_menu_overlay();

            //printf("Fin procesamiento f funciones\n");


        }



        if (menu_event_new_version_show_changes.v) {
            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd
            menu_event_new_version_show_changes.v=0;
            menu_generic_message_format("Updated version","You have updated ZEsarUX :)\nPlease take a look at the changes:");
            //No mostramos error si el Changelog es mayor de lo que puede leer el visor de text (y es mayor de 64000 desde versión 9.2)
            menu_about_changelog_common(0);

            cls_menu_overlay();
        }

        if (menu_event_new_update.v) {
            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd
            menu_event_new_update.v=0;
            menu_generic_message_format("New version available","ZEsarUX version %s is available on github",stats_last_remote_version);

            cls_menu_overlay();
        }

//printf ("6inicio menu_inicio. menu_event_open_menu.v=%d\n",menu_event_open_menu.v);

        if (menu_event_open_menu.v) {

            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

            //Abrir menu normal
            //printf ("Abrir menu normal. mouse left: %d\n",mouse_left);

            //printf("menu_event_open_menu.v xx\n");

//printf ("7inicio menu_inicio\n");

            //Ver si se ha pulsado en botones de zx desktop
            if (menu_was_open_by_left_mouse_button.v) {
                debug_printf(VERBOSE_DEBUG,"Menu was open by left mouse button");
                menu_was_open_by_left_mouse_button.v=0;

                if (!pulsado_alguna_ventana_con_menu_cerrado) {
                    //printf("llamar a if mouse in logo desde 26928\n");
                    if (zxvision_if_mouse_in_zlogo_or_buttons_desktop() ) {
                        //printf("zxvision_if_mouse_in_zlogo_or_buttons_desktop 6\n");
                        //necesario para que no se piense que se está moviendo el raton
                        //Esto es un poco puñetero porque si no lo pongo aqui a 0,
                        //al lanzar por ejemplo smartload se queda al principio esperando que se
                        //libere el "movimiento" desde menu_espera_no_tecla desde menu_filesel
                        //como no se llama a eventos handle_mouse pues no se pone a 0


                        //Esto se ha puesto a 1 antes desde zxvision_if_mouse_in_zlogo_or_buttons_desktop,
                        //indirectamente cuando llama a menu_calculate_mouse_xy_absolute_interface_pixel

                        mouse_movido=0;

                        debug_printf(VERBOSE_DEBUG,"Pressed on buttons zone with menu closed");
                    }
                    else {
                        debug_printf(VERBOSE_DEBUG,"Not pressed on buttons zone with menu closed");
                    }
                }
            }

//printf ("8inicio menu_inicio\n");

            //Ver si se ha pulsado en botones de zx desktop
            if (menu_was_open_by_right_mouse_button.v) {
                debug_printf(VERBOSE_DEBUG,"Menu was open by right mouse button");
                menu_was_open_by_right_mouse_button.v=0;

                if (!pulsado_alguna_ventana_con_menu_cerrado) {

                    if (zxvision_if_mouse_in_zlogo_or_buttons_desktop_right_button() ) {
                        //necesario para que no se piense que se está moviendo el raton
                        //Esto es un poco puñetero porque si no lo pongo aqui a 0,
                        //al lanzar por ejemplo smartload se queda al principio esperando que se
                        //libere el "movimiento" desde menu_espera_no_tecla desde menu_filesel
                        //como no se llama a eventos handle_mouse pues no se pone a 0


                        //Esto se ha puesto a 1 antes desde zxvision_if_mouse_in_zlogo_or_buttons_desktop,
                        //indirectamente cuando llama a menu_calculate_mouse_xy_absolute_interface_pixel

                        mouse_movido=0;

                        debug_printf(VERBOSE_DEBUG,"Pressed on buttons zone (right button) with menu closed");
                    }
                    else {
                        debug_printf(VERBOSE_DEBUG,"Not pressed on buttons zone (right button) with menu closed");
                    }
                }
            }

//printf ("9inicio menu_inicio\n");

            menu_inicio_bucle();

//printf ("10inicio menu_inicio\n");

        }

    }


    menu_was_open_by_left_mouse_button.v=0;
    menu_was_open_by_right_mouse_button.v=0;


//printf("antes menu_inicio_pre_retorno. menu_breakpoint_exception.v=%d\n",menu_breakpoint_exception.v);
    //Volver
    menu_inicio_pre_retorno();

//printf("salir menu. menu_breakpoint_exception.v=%d\n",menu_breakpoint_exception.v);

    //Si se ha hecho drag & drop con el menu abierto, decir de abrir de nuevo el menu para gestionar ese drag & drop
    if (menu_event_pending_drag_drop_menu_open.v) {
        menu_event_pending_drag_drop_menu_open.v=0;
        menu_event_drag_drop.v=1;
        menu_abierto=1;
        //printf("Reabrir menu para gestionar drag & drop\n");
    }

    //Si se ha cargado un archivo zmenu con el menu abierto, decir de abrir de nuevo el menu para gestionar zmenu
    if (menu_event_pending_zmenu_file_menu_open.v) {
        menu_event_pending_zmenu_file_menu_open.v=0;
        menu_event_open_zmenu_file.v=1;
        menu_abierto=1;
        //printf("Reabrir menu para gestionar zmenu\n");
    }

}

//Escribe bloque de cuadrado de color concreto
void set_splash_zesarux_logo_put_space_color(int x,int y,int color)
{


    if (!strcmp(scr_new_driver_name,"aa")) {
        putchar_menu_overlay(x,y,'X',7,0);
    }
    else {
        putchar_menu_overlay(x,y,' ',7,color);
    }
}

//Escribe bloque de cuadrado de color concreto, con *2
void set_splash_zesarux_logo_put_space_color_double(int x,int y,int color)
{

    set_splash_zesarux_logo_put_space_color(x,y,color);
    set_splash_zesarux_logo_put_space_color(x+1,y,color);
    set_splash_zesarux_logo_put_space_color(x,y+1,color);
    set_splash_zesarux_logo_put_space_color(x+1,y+1,color);

}

//Escribe bloque de cuadrado de color negro
void set_splash_zesarux_logo_put_space(int x,int y)
{

    int color_negro=0;

    if (christmas_mode.v) {
        //En logo X anniversary y en navidad, no alteramos color negro
        if (xanniversary_logo.v==0) color_negro=2+8;
    }

    if (!strcmp(scr_new_driver_name,"aa")) {
        putchar_menu_overlay(x,y,'X',7,0);
    }
    else putchar_menu_overlay(x,y,' ',7,color_negro);
}


//Hace cuadrado de 2x2
void set_splash_zesarux_logo_cuadrado(int x,int y)
{
    set_splash_zesarux_logo_put_space(x,y);
    set_splash_zesarux_logo_put_space(x+1,y);
    set_splash_zesarux_logo_put_space(x,y+1);
    set_splash_zesarux_logo_put_space(x+1,y+1);
}




//Escribe caracter  128 (franja de color-triangulo)
void set_splash_zesarux_franja_color(int x,int y,int tinta, int papel)
{
    if (si_complete_video_driver() ) {
        putchar_menu_overlay(x,y,128,tinta,papel);
    }
    else {
        putchar_menu_overlay(x,y,'/',tinta,7);
    }
}

//Escribe caracter ' ' con color
void set_splash_zesarux_cuadrado_color(int x,int y,int color)
{
    if (si_complete_video_driver() ) {
        putchar_menu_overlay(x,y,' ',0,color);
    }
}

void set_splash_zesarux_franja_color_repetido(int x,int y,int longitud, int color1, int color2)
{

    int j;
    for (j=0;j<longitud;j++) {
        set_splash_zesarux_franja_color(x+j,y-j,color1,color2);
    }

}


int get_zsplash_y_coord(void)
{
    return menu_center_y()-4;
}


//Dibuja el logo pero en diferentes pasos:
//0: solo la z
//1: franja roja
//2: franja roja y amarilla
//3: franja roja y amarilla y verde
//4 o mayor: franja roja y amarilla y verde y cyan
void set_splash_zesarux_logo_paso_normal(int paso)
{
    int x,y;

    int ancho_z=6;
    int alto_z=6;

    int x_inicial=menu_center_x()-ancho_z;  //Centrado
    int y_inicial=get_zsplash_y_coord();

    debug_printf(VERBOSE_DEBUG,"Drawing ZEsarUX splash logo, step %d",paso);

    int color_fondo=7;
    int color_rojo=2;
    int color_amarillo=6;
    int color_verde=4;
    int color_cyan=5;

    if (christmas_mode.v) {
        color_fondo=15;
        color_rojo=2+8;
        color_amarillo=4+8;
        color_verde=2+8;
        color_cyan=4+8;
    }

    //Luto
    if (activated_in_memoriam_david.v) {
        color_rojo=0;
        color_amarillo=0;
        color_verde=0;
        color_cyan=0;
    }


    //Primero todo texto en gris. Envolvemos un poco mas
    for (y=y_inicial-1;y<y_inicial+ancho_z*2+1;y++) {
        for (x=x_inicial-1;x<x_inicial+ancho_z*2+1;x++) {
            putchar_menu_overlay_parpadeo(x,y,' ',0,color_fondo,0);


        }
    }


    y=y_inicial;

    //Linea Arriba Z, Abajo
    for (x=x_inicial;x<x_inicial+ancho_z*2;x++) {
        set_splash_zesarux_logo_put_space(x,y);
        set_splash_zesarux_logo_put_space(x,y+1);

        set_splash_zesarux_logo_put_space(x,y+alto_z*2-2);
        set_splash_zesarux_logo_put_space(x,y+alto_z*2-1);
    }

    //Cuadrados diagonales
    y+=2;

    for (x=x_inicial+(ancho_z-2)*2;x>x_inicial;x-=2,y+=2) {
        set_splash_zesarux_logo_cuadrado(x,y);
    }

    //Y ahora las lineas de colores
    //Rojo amarillo verde cyan
    //2      6       4     5

    if (paso==0) return;

    /*

    012345678901
0	XXXXXXXXXXXX
1	XXXXXXXXXXXX
2	        XX
3	        XX /
4	      XX  /
5	      XX / /
6	    XX  / /
7		XX / / /
8	  XX  / / /
9	  XX / / / /
0	XXXXXXXXXXXX
1	XXXXXXXXXXXX

    012345678901
    */

/*
  RRRY
 RRRYY
RRRYYY

*/

/*
            XX .
          XX  .x
          XX .x.
        XX  .x.x
        XX .x.x.
      XX  .x.x.x
      XX .x.x.x.
    XXXXXXXXXXXX
    XXXXXXXXXXXX

    012345678901
    */



    int j;


    set_splash_zesarux_franja_color_repetido(x_inicial+5,y_inicial+9,7, color_rojo, color_fondo);

    for (j=0;j<6;j++) {
        set_splash_zesarux_cuadrado_color(x_inicial+6+j,y_inicial+9-j,color_rojo);
    }

    //Lo que queda a la derecha de esa franja - el udg diagonal con el color de papel igual que tinta anterior, y papel blanco
    if (paso==1) {
        if (si_complete_video_driver() ) {
            set_splash_zesarux_franja_color_repetido(x_inicial+7,y_inicial+9,5, color_fondo, color_rojo);
        }
        return;
    }


    set_splash_zesarux_franja_color_repetido(x_inicial+7,y_inicial+9,5, color_amarillo, color_rojo);

    for (j=0;j<4;j++) {
        set_splash_zesarux_cuadrado_color(x_inicial+8+j,y_inicial+9-j,color_amarillo);
    }

    //Lo que queda a la derecha de esa franja - el udg diagonal con el color de papel igual que tinta anterior, y papel blanco
    if (paso==2) {
        if (si_complete_video_driver() ) {
            set_splash_zesarux_franja_color_repetido(x_inicial+9,y_inicial+9,3, color_fondo, color_amarillo);
        }
        return;
    }



    set_splash_zesarux_franja_color_repetido(x_inicial+9,y_inicial+9,3, color_verde, color_amarillo);

    for (j=0;j<2;j++) {
        set_splash_zesarux_cuadrado_color(x_inicial+10+j,y_inicial+9-j,color_verde);
    }

    //Lo que queda a la derecha de esa franja - el udg diagonal con el color de papel igual que tinta anterior, y papel blanco
    if (paso==3) {
        if (si_complete_video_driver() ) {
            set_splash_zesarux_franja_color(x_inicial+ancho_z*2-1,y_inicial+ancho_z*2-3,color_fondo,color_verde);
        }
        return;
    }

    set_splash_zesarux_franja_color(x_inicial+ancho_z*2-1,y_inicial+ancho_z*2-3,color_cyan,color_verde);

}

//Dibuja el logo especial de X aniversario pero en diferentes pasos:
//0: solo la X
//1: zona roja
//2: zona roja y amarilla
//3: zona roja y amarilla y verde
//4 o mayor: zona roja y amarilla y verde y cyan
void set_splash_zesarux_logo_paso_xanniversary(int paso)
{
    int x,y;

    int ancho_z=5;
    int alto_z=5;

    int x_inicial=menu_center_x()-ancho_z;  //Centrado
    int y_inicial=get_zsplash_y_coord();

    debug_printf(VERBOSE_DEBUG,"Drawing ZEsarUX X Anniversary splash logo, step %d",paso);

    int color_fondo=7;
    int color_rojo=2;
    int color_amarillo=6;
    int color_verde=4;
    int color_cyan=5;

    if (christmas_mode.v) {
        color_fondo=15;
        color_rojo=2+8;
        color_amarillo=4+8;
        color_verde=2+8;
        color_cyan=4+8;
    }

    //Luto
    if (activated_in_memoriam_david.v) {
        color_rojo=0;
        color_amarillo=0;
        color_verde=0;
        color_cyan=0;
    }


    //Primero todo texto en gris. Envolvemos un poco mas
    for (y=y_inicial-1;y<y_inicial+ancho_z*2+1;y++) {
        for (x=x_inicial-1;x<x_inicial+ancho_z*2+1;x++) {
            putchar_menu_overlay_parpadeo(x,y,' ',0,color_fondo,0);


        }
    }



    int i;
    //Linea X hacia la derecha
    for (i=0;i<ancho_z;i++) {
        set_splash_zesarux_logo_put_space(x_inicial+i*2,    y_inicial+i*2);
        //set_splash_zesarux_logo_put_space(x_inicial+i*2+1,  y_inicial+i*2);
        //set_splash_zesarux_logo_put_space(x_inicial+i*2,    y_inicial+i*2+1);
        set_splash_zesarux_logo_put_space(x_inicial+i*2+1,  y_inicial+i*2+1);
    }

    //Linea X hacia la izquierda
    x=x_inicial+(ancho_z-1)*2;
    for (i=ancho_z-1;i>=0;i--) {
        //set_splash_zesarux_logo_put_space(x-i*2,    y_inicial+i*2);
        set_splash_zesarux_logo_put_space(x-i*2+1,  y_inicial+i*2);
        set_splash_zesarux_logo_put_space(x-i*2,    y_inicial+i*2+1);
        //set_splash_zesarux_logo_put_space(x-i*2+1,  y_inicial+i*2+1);
    }


    //Y ahora las zonas de colores
    //Rojo amarillo verde cyan
    //2      6       4     5

    if (paso==0) return;

    //Zona roja
    set_splash_zesarux_logo_put_space_color_double(x_inicial+2,y_inicial,color_rojo);
    set_splash_zesarux_logo_put_space_color_double(x_inicial+4,y_inicial,color_rojo);
    set_splash_zesarux_logo_put_space_color_double(x_inicial+6,y_inicial,color_rojo);
    set_splash_zesarux_logo_put_space_color_double(x_inicial+4,y_inicial+2,color_rojo);

    set_splash_zesarux_logo_put_space_color(x_inicial+1,  y_inicial,color_rojo);
    set_splash_zesarux_logo_put_space_color(x_inicial+3,y_inicial+2,color_rojo);
    set_splash_zesarux_logo_put_space_color(x_inicial+8,y_inicial,color_rojo);
    set_splash_zesarux_logo_put_space_color(x_inicial+6,y_inicial+2,color_rojo);
    if (paso==1) return;


    //Zona amarilla
    x=x_inicial+(ancho_z-1)*2;
    set_splash_zesarux_logo_put_space_color_double(x,y_inicial+2,color_amarillo);
    set_splash_zesarux_logo_put_space_color_double(x,y_inicial+4,color_amarillo);
    set_splash_zesarux_logo_put_space_color_double(x,y_inicial+6,color_amarillo);
    set_splash_zesarux_logo_put_space_color_double(x-2,y_inicial+4,color_amarillo);

    set_splash_zesarux_logo_put_space_color(x+1,y_inicial+1,color_amarillo);
    set_splash_zesarux_logo_put_space_color(x-1,y_inicial+3,color_amarillo);
    set_splash_zesarux_logo_put_space_color(x-1,y_inicial+6,color_amarillo);
    set_splash_zesarux_logo_put_space_color(x+1,y_inicial+8,color_amarillo);
    if (paso==2) return;

    //Zona verde
    y=y_inicial+(alto_z-1)*2;
    set_splash_zesarux_logo_put_space_color_double(x_inicial+2,y,color_verde);
    set_splash_zesarux_logo_put_space_color_double(x_inicial+4,y,color_verde);
    set_splash_zesarux_logo_put_space_color_double(x_inicial+6,y,color_verde);
    set_splash_zesarux_logo_put_space_color_double(x_inicial+4,y-2,color_verde);

    set_splash_zesarux_logo_put_space_color(x_inicial+1,y+1,color_verde);
    set_splash_zesarux_logo_put_space_color(x_inicial+3,y-1,color_verde);
    set_splash_zesarux_logo_put_space_color(x_inicial+6,y-1,color_verde);
    set_splash_zesarux_logo_put_space_color(x_inicial+8,y+1,color_verde);
    if (paso==3) return;


    //Zona cyan
    x=x_inicial+(ancho_z-1)*2;
    set_splash_zesarux_logo_put_space_color_double(x_inicial,y_inicial+2,color_cyan);
    set_splash_zesarux_logo_put_space_color_double(x_inicial,y_inicial+4,color_cyan);
    set_splash_zesarux_logo_put_space_color_double(x_inicial,y_inicial+6,color_cyan);
    set_splash_zesarux_logo_put_space_color_double(x_inicial+2,y_inicial+4,color_cyan);

    set_splash_zesarux_logo_put_space_color(x_inicial,y_inicial+1,color_cyan);
    set_splash_zesarux_logo_put_space_color(x_inicial+2,y_inicial+3,color_cyan);
    set_splash_zesarux_logo_put_space_color(x_inicial+2,y_inicial+6,color_cyan);
    set_splash_zesarux_logo_put_space_color(x_inicial,y_inicial+8,color_cyan);

}

void set_splash_zesarux_logo_paso(int paso)
{

    if (xanniversary_logo.v) {
        set_splash_zesarux_logo_paso_xanniversary(paso);
    }

    else set_splash_zesarux_logo_paso_normal(paso);
}

//Retorna color de paleta spectrum segun letra color logo ascii W: white, X: Black, etc
//en mayusculas es con brillo, sin mayusculas es sin brillo
int return_color_zesarux_ascii(char c)
{

    int color;

    int brillo=0;

    if (c>='A' && c<='Z') {
        brillo=1;
        c=letra_minuscula(c);
    }

    switch (c) {


        //Black
        case 'x':
            color=0;
        break;

        //Blue
        case 'b':
            color=1;
        break;

        //Red
        case 'r':
            color=2;
        break;

        //Magenta
        case 'm':
            color=3;
        break;

        //Green
        case 'g':
            color=4;
        break;

        //Cyan
        case 'c':
            color=5;
        break;

        //Yellow
        case 'y':
            color=6;
        break;

        //White
        case 'w':
            color=7;
        break;

        //Black default
        default:
            color=0;
        break;
    }

    return color+brillo*8;
}

void set_fast_splash_zesarux_logo(void)
{
    splash_zesarux_logo_paso=4;
    menu_splash_segundos=1;
}

void set_splash_zesarux_logo(void)
{
    splash_zesarux_logo_active=1;
    splash_zesarux_logo_paso=0;
    //set_splash_zesarux_logo_paso(splash_zesarux_logo_paso);
}

void get_welcome_message(char *texto_welcome)
{
    if (gui_language==GUI_LANGUAGE_SPANISH) sprintf(texto_welcome," Bienvenido a ZEsarUX " EMULATOR_VERSION " ");
    else if (gui_language==GUI_LANGUAGE_CATALAN) sprintf(texto_welcome," Benvingut a ZEsarUX " EMULATOR_VERSION " ");
    else sprintf(texto_welcome," Welcome to ZEsarUX " EMULATOR_VERSION " ");
}

void get_press_key_menu_message(char *destino)
{
    if (gui_language==GUI_LANGUAGE_SPANISH) sprintf(destino," Pulse %s para menu ",openmenu_key_message);
    else if (gui_language==GUI_LANGUAGE_CATALAN) sprintf(destino," Premi %s per menu ",openmenu_key_message);
    else sprintf(destino," Press %s for menu ",openmenu_key_message);
}

void set_welcome_message(void)
{
    cls_menu_overlay();
    char texto_welcome[40];
    get_welcome_message(texto_welcome);

    int yinicial=get_zsplash_y_coord()-6;

    //centramos texto
    int x=menu_center_x()-strlen(texto_welcome)/2;
    if (x<0) x=0;

    menu_escribe_texto(x,yinicial++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,texto_welcome);

    set_splash_zesarux_logo();
    menu_splash_segundos=5;

    if (opcion_fast_welcome_message.v) set_fast_splash_zesarux_logo();

    set_splash_zesarux_logo_paso(splash_zesarux_logo_paso);


        char texto_edition[40];
        sprintf(texto_edition," " EMULATOR_EDITION_NAME " ");

        int longitud_texto=strlen(texto_edition);
        //temporal, como estamos usando parpadeo mediante caracteres ^^, no deben contar en la longitud
        //cuando no se use parpadeo, quitar esta resta
        //longitud_texto -=4;

        //centramos texto
        x=menu_center_x()-longitud_texto/2;
        if (x<0) x=0;

        menu_escribe_texto(x,yinicial++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,texto_edition);




    char texto_esc_menu[32];
    get_press_key_menu_message(texto_esc_menu);
    longitud_texto=strlen(texto_esc_menu);
        x=menu_center_x()-longitud_texto/2;
        if (x<0) x=0;
    menu_escribe_texto(x,yinicial++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,texto_esc_menu);

    if (christmas_mode.v) {
        char *christmas_text="Merry Christmas!";
        longitud_texto=strlen(christmas_text);
        x=menu_center_x()-longitud_texto/2;
        if (x<0) x=0;

        menu_escribe_texto(x,yinicial++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,christmas_text);
    }

    set_menu_overlay_function(normal_overlay_texto_menu);
    menu_splash_text_active.v=1;


    //Enviar texto de bienvenida tambien a speech
    //stdout y simpletext no
    if (!strcmp(scr_new_driver_name,"stdout")) return;
    if (!strcmp(scr_new_driver_name,"simpletext")) return;

    if (textspeech_also_send_menu.v) {
        textspeech_print_speech(texto_welcome);
        textspeech_print_speech(texto_edition);
        textspeech_print_speech(texto_esc_menu);
    }

}




//2 segundos antes de que se avise si hay detectado joystick o no
int menu_tell_if_realjoystick_detected_counter=2;

void menu_tell_if_realjoystick_detected(void)
{
            //Si detectado real joystick
            //Si detectado joystick real y si hay autoguardado de config
            if (save_configuration_file_on_exit.v) {
                    if (realjoystick_present.v) {
                            menu_set_menu_abierto(1);
                            //printf ("decir menu abierto\n");
                            realjoystick_detected_startup=1;
                    }
            }
}

//Mostrar las ventanas que hay en background que se han restaurado al arrancar
void show_all_windows_startup(void)
{
    if (menu_allow_background_windows && menu_multitarea && menu_reopen_background_windows_on_start.v && always_force_overlay_visible_when_menu_closed) {
        //para activar overlay llamar a set_menu_overlay_function pues se define la funcion de overlay
        //NO hacer solo un menu_overlay_activo=1 pues aqui en el inicio la funcion de overlay es NULL y provocaria segfault;
        set_menu_overlay_function(normal_overlay_texto_menu);

        //Y redibujamos las ventanas, para que se vean los titulos sobretodo (pues los overlay en background no redibujan los titulos)
        //decir que ventana principal no esta activa, para indicar que están todas en background
        zxvision_reset_ventana_tipo_activa();


        zxvision_redraw_all_windows();

        overlay_visible_when_menu_closed=1;
        //menu_draw_background_windows_overlay_after_normal();
    }
}

void reset_welcome_message(void)
{
    if (menu_splash_text_active.v==1) {

        //printf ("%d\n",menu_splash_segundos);
        menu_splash_segundos--;
        if (menu_splash_segundos==0) {
            reset_splash_zesarux_logo();
            menu_splash_text_active.v=0;


            //Si hay ventana de splash, cerrarla
            zxvision_window *ventana;

            //ventana=zxvision_return_n_window_from_top(valor_opcion);

            ventana=zxvision_find_window_in_background("splashwindow");

            if (ventana!=NULL) {
                debug_printf(VERBOSE_INFO,"Closing splash window");
                zxvision_destroy_window(ventana);
            }


            cls_menu_overlay();
            reset_menu_overlay_function();
            debug_printf (VERBOSE_DEBUG,"End splash text");

            //Quitamos el splash text pero dejamos el F5 menu abajo en el footer durante 60 segundos y hasta que algo borre ese mensaje
            //(por ejemplo que cargamos una cinta/snap con configuracion y genera mensaje en texto inverso en el footer) o se abra el menu
            //if (first_time_menu_footer_f5_menu) {
            //menu_footer_f5_menu_counter=60;
            //menu_footer_zesarux_emulator();
            //}



            //abrir el menu si hay first aid en startup disponible
            //Para que aparezca el mensaje del dia, tiene que estar habilitado el setting de welcome message
            //Si no, no llegara aqui nunca
            if (menu_first_aid_startup) menu_first_aid_random_startup();


            //mostrar ventanas en background
            show_all_windows_startup();


            //Si activado in memoriam de David
            if (activated_in_memoriam_david.v) {
                menu_set_menu_abierto(1);
            }
        }

        else {
            if (splash_zesarux_logo_active) {
                splash_zesarux_logo_paso++;
                set_splash_zesarux_logo_paso(splash_zesarux_logo_paso);
            }
        }


    }
}




//truncar el nombre del archivo a un maximo
//si origen es NULL, poner en destino cadena vacia
//Si es 0 o menor que 0, devuelve siempre cadena vacia acabada en 0 (usa 1 byte)
//Esto se hace en algunos casos para menu_filesel en que se resta sobre el tamaño visible y puede ser <=0
void menu_tape_settings_trunc_name(char *orig,char *dest,int max)
{
    //printf ("max: %d\n",max);
    if (max<=0) {
        dest[0]=0;
        return;
    }
    //en maximo se incluye caracter 0 del final
    max--;

                if (orig!=0) {

                        int longitud=strlen(orig);
                        int indice=longitud-max;

            if (indice<0) indice=0;

                        strncpy(dest,&orig[indice],max);


            //printf ("copiamos %d max caracteres\n",max);

                        //si cadena es mayor, acabar con 0

            //en teoria max siempre es mayor de cero, pero por si acaso...
            if (max>0) dest[max]=0;

            //else printf ("max=%d\n",max);


            if (indice>0) dest[0]='<';

                }

             else {
                        strcpy(dest,"");
                }

}



void estilo_gui_retorna_nombres(void)
{
    int i;

    for (i=0;i<ESTILOS_GUI;i++) {
        printf ("%s",definiciones_estilos_gui[i].nombre_estilo);
        //coma si no es el ultimo item
        if (i!=ESTILOS_GUI-1) printf(",");

        printf(" ");
    }
}

void set_charset_from_gui(void)
{


    char_set=definiciones_estilos_gui[estilo_gui_activo].style_char_set;

    //User charset a ninguno
    user_charset=-1;
}


int total_first_aid=0;

void menu_first_aid_add(char *key_string,int *puntero_setting,char *texto_opcion,int si_startup)
{

    //printf("total_first_aid %d\n",total_first_aid);
    if (total_first_aid==MAX_MENU_FIRST_AID) {
        debug_printf(VERBOSE_ERR,"Can not add more first aid messages");
        return;
    }

    //first_aid_list[total_first_aid].indice_setting=indice_aid;
    strcpy(first_aid_list[total_first_aid].config_name,key_string);
    first_aid_list[total_first_aid].puntero_setting=puntero_setting;
    first_aid_list[total_first_aid].texto_opcion=texto_opcion;
    first_aid_list[total_first_aid].si_startup=si_startup;

    total_first_aid++;
}


//No mostrar la opcion. por defecto a 0 (mostrarla)

//
//Items que se disparan en ciertos eventos, con parametro si_startup=0
//
int first_aid_no_filesel_uppercase_keys=0;
char *first_aid_string_filesel_uppercase_keys="If you want to select a file by its initial letter, please press the letter as it is. "
                            "If you want to execute actions shown in the bottom of the window, in inverted colour, please press shift+letter";

int first_aid_no_filesel_enter_key=0;
char *first_aid_string_filesel_enter_key="Press ENTER or left mouse button to select a file or change directory.\n"
                            "Press Space to expand files, like tap, tzx, trd, scl... etc and also all the compressed supported files";

int first_aid_no_smartload=0;
char *first_aid_string_smartload="This smartload window allows you to load any file known by ZEsarUX. Just select it and go!\n"
                            "Press TAB to change between areas in the file browser";

int first_aid_no_initial_menu=0;
char *first_aid_string_initial_menu="This is the Main Menu. You can select an item by using cursor keys, Enter (or right key) and mouse. Most of them have help, "
    "try pressing F1. Also, many have tooltip help, that means if you don't press a key, it will appear a tooltip "
    "about what the item does. ESC closes all menus, you can also close it by pressing the top-left button in the window. "
    "You can go back a menu pressing cursor left. "
    "You can also use your mouse to resize or move windows";

int first_aid_no_ssl_wos=0;
char *first_aid_string_no_ssl_wos="Warning: as SSL support is not compiled, results newer than 2013 may fail";

int first_aid_no_realjoystick_detected=0;
char *first_aid_string_realjoystick_detected="A real joystick has been detected\n"
                            "You can go to menu Settings->Hardware->Real joystick support and set your buttons configuration";

int first_aid_no_sg1000_boot=0;
char *first_aid_string_sg1000_boot="The SG-1000 doesn't have a BIOS, so you must insert a game rom from menu Storage->SG-1000 Cartridge";

int first_aid_no_tbblue_download_sd_bugs=0;
char *first_aid_string_tbblue_download_sd_bugs="In case of glitches or bugs, maybe the official image uses features not emulated in ZEsarUX yet. "
        "If it's that the case, use the included tbblue sd image in ZEsarUX";


int first_aid_no_mount_mmc_fileutils=0;
char *first_aid_string_mount_mmc_fileutils="Changes done in the image file are only kept in RAM memory, until you execute the Sync action";

int first_aid_no_download_spectrumcomputing=0;
char *first_aid_string_download_spectrumcomputing="This downloaded file is hosted in spectrumcomputing.co.uk website. Thanks to Peter Jones for allowing it";


int first_aid_no_search_zxinfo=0;
char *first_aid_string_search_zxinfo="This search engine is hosted in zxinfo.dk website. Thanks to Thomas Heckmann for allowing it";

int first_aid_no_debug_variables=0;
char *first_aid_string_debug_variables="Browsing the floating point numeric variables is not accurate, please take the numbers as approximate";

int first_aid_no_debug_console=0;
char *first_aid_string_debug_console="First messages appear at the bottom of the window";

int first_aid_no_back_run_rainbow=0;
char *first_aid_string_back_run_rainbow="As you have real video enabled, you may see the machine display now drawn as the initial state, but it's not the actual state, it's a temporary frame that will disappear after you close the menu";

int first_aid_no_language=0;
char *first_aid_string_language="Warning: not all messages are translated.\nAviso: No todos los mensajes están traducidos.\nAvís: No tots els missages estan traduïts.";


int first_aid_no_hilow_format=0;
char *first_aid_string_hilow_format="You can also format the Data Drive using the command SAVE \"FORMAT name\" on the Basic prompt.";


int first_aid_no_hilow_convert_bside=0;
char *first_aid_string_hilow_convert_bside="When changing b-size flag, you must stop and start conversion again in order to read again the input file.";


int first_aid_no_ext_audio_source=0;
char *first_aid_string_ext_audio_source="Audio is read from the current recording device and input on your computer, you must use the "
    "operating system settings to switch to your desired device and input (probably Line In)";


//
//Items que se disparan en startup
//

int first_aid_no_startup_aid=0;
char *first_aid_string_startup_aid="This is a first aid help message. You will be shown some of these at the emulator startup, but also "
    "when opening some menus. All of them are suggestions, advices and pieces of help of ZEsarUX. You can entirely disable them by going to Settings-> "
    "ZX Vision-> First aid help";

int first_aid_no_multiplattform=0;
char *first_aid_string_multiplattform="Do you know that ZEsarUX is multiplattform? There are main versions for Linux, Mac, Windows and Raspberry pi. "
 "But also for Retropie/EmulationStation, Open Pandora, PocketCHIP and MorhpOS. "
 "You can even compile it by yourself, you only need a compatible Unix environment to do that!";

int first_aid_no_accessibility=0;
char *first_aid_string_accessibility="Do you know ZEsarUX has accessibility settings? You can hear the menu or even hear text adventures games. "
 "You will only need an external text-to-speech program to do that. Please read the FAQ to know more";

int first_aid_no_documentation=0;
char *first_aid_string_documentation="You can find a lot of info, videos, documentation, etc on:\n"
  "-Menu entries with help (pressing F1 in most of them) and item tooltips when no key is pressed\n"
  "-FAQ file\n"
  "-docs folder (on the extras package)\n"
  "-Youtube channel: https://www.youtube.com/user/chernandezba\n"
  "-Twitter: @zesarux\n"
  "-Facebook: ZEsarUX group\n";

int first_aid_no_zrcp=0;
char *first_aid_string_zrcp="You can connect to ZEsarUX by using a telnet client using the ZEsarUX Remote Control Protocol (ZRCP). "
    "This protocol allows you to interact, debug and do a lot of internal actions to ZEsarUX. "
    "Just enable it on Settings-> Debug and use a telnet client to connect to port 10000. ";


int first_aid_no_votext=0;
char *first_aid_string_votext="Do you know you can run ZEsarUX using a Text mode video driver? There are ncurses, aalib, cacalib, "
 "stdout and simpletext drivers. They are not all compiled by default, only stdout, you maybe need to compile ZEsarUX by yourself to test all of them";

int first_aid_no_easteregg=0;
char *first_aid_string_eastereg="ZEsarUX includes seven easter eggs. Can you find them? :)";


int first_aid_no_multitask=0;
char *first_aid_string_multitask="The multitask setting (enabled by default) tells ZEsarUX to continue running the emulation when you open the menu. "
    "Sometimes, if you cpu is not fast enough, the emulation can be stopped or drop FPS when you open the menu. You can disable it on Settings->GUI";

int first_aid_no_zxvision_clickout=0;
char *first_aid_string_zxvision_clickout="If you have multitask enabled, with menu open, and you press left shift+left mouse button out of a window menu (inside ZEsarUX window), "
    "that menu window loses focus and the emulated machine gets the focus, so you can use the keyboard on the emulated machine and the menu window "
    "is still alive";


int first_aid_no_conversion=0;
char *first_aid_string_conversion="You can convert some known file formats from the File utilities menu. For example, you can "
    "convert a TAP to a TZX file";


int first_aid_no_fileextensions=0;
char *first_aid_string_fileextensions="If you save a file, for example, a snapshot, you must write the file with the desired extension, "
    "for example test.z80 or test.zsf, so ZEsarUX will know what kind of file you want to save depending on the extension you write";

int first_aid_no_zsfextension=0;
char *first_aid_string_zsfextension="ZEsarUX uses two native snapshot file formats: .zsf and .zx.\n.zsf, which means 'ZEsarUX Snapshot File', "
    "is the preferred snapshot type, as it is supported on almost all emulated computers and can save things like: ZX-Uno memory, Divmmc status, etc.\n"
    ".zx is the old snapshot native file format, which was the default format for ZEsarUX previous versions and also used in my other "
    "emulator, the ZXSpectr";

int first_aid_no_spaceexpand=0;
char *first_aid_string_spaceexpand="Do you know you can navigate inside files, like tap, tzx, trd?\n"
    "Use the fileselector and press space over that kind of file.\n"
    "Remember to change fileselector filter to show all contents";

int first_aid_no_backgroundwindows=0;
char *first_aid_string_backgroundwindows="You can enable background windows and put some windows on the background. "
        "Go to Settings-> ZX Vision settings-> Background windows to enable it";

int first_aid_no_zxdesktop=0;
char *first_aid_string_zxdesktop="Have you enabled ZX Desktop? It allows you to have a space on the right and on the bottom to place "
                "zxvision windows, menus or other widgets. Go to Settings-> ZX Desktop settings to enable it";

int first_aid_no_zxdesktop_custombuttons=0;
char *first_aid_string_zxdesktop_custombuttons="You can customize upper ZX Desktop buttons to trigger different actions. "
                "Go to Settings-> ZX Desktop-> Customize buttons";

int first_aid_no_snapshot_save_zsf=0;
char *first_aid_string_snapshot_save_zsf="When saving snapshots, the recommended extension is ZSF, as this is the native "
    "ZEsarUX snapshot format. If you want to save a different format snapshot, just change the extension. If you want to overwrite "
    "a existing snapshot, just select it";

int first_aid_no_advanced_menu=0;
char *first_aid_string_advanced_menu="The menu interface has many items which are hidden by default. You can show them by "
    "enabling Settings-> Advanced menu items";

int first_aid_no_donate=0;
char *first_aid_string_donate="Have you donated to ZEsarUX? ZEsarUX will always cost you nothing to use, but that doesn't mean it costs me "
                              "nothing to make. So if you want to demonstrate your appreciation to it and also help me cover the costs of my "
                              "development servers, go to the ZEsarUX website and look for the DONATE section. Thanks!";


void menu_first_aid_init(void)
{
    total_first_aid=0;

    //Items que se disparan en ciertos eventos, con parametro si_startup=0
    menu_first_aid_add("filesel_uppercase_keys",&first_aid_no_filesel_uppercase_keys,first_aid_string_filesel_uppercase_keys,0);
    menu_first_aid_add("filesel_enter_key",&first_aid_no_filesel_enter_key,first_aid_string_filesel_enter_key,0);
    menu_first_aid_add("smartload",&first_aid_no_smartload,first_aid_string_smartload,0);
    menu_first_aid_add("initial_menu",&first_aid_no_initial_menu,first_aid_string_initial_menu,0);
    menu_first_aid_add("no_ssl_wos",&first_aid_no_ssl_wos,first_aid_string_no_ssl_wos,0);
    menu_first_aid_add("realjoystick_detected",&first_aid_no_realjoystick_detected,first_aid_string_realjoystick_detected,0);
    menu_first_aid_add("sg1000_boot",&first_aid_no_sg1000_boot,first_aid_string_sg1000_boot,0);
    menu_first_aid_add("tbblue_download_sd_bug",&first_aid_no_tbblue_download_sd_bugs,first_aid_string_tbblue_download_sd_bugs,0);
    menu_first_aid_add("mount_mmc_fileutils",&first_aid_no_mount_mmc_fileutils,first_aid_string_mount_mmc_fileutils,0);
    menu_first_aid_add("download_spectrumcomputing",&first_aid_no_download_spectrumcomputing,first_aid_string_download_spectrumcomputing,0);
    menu_first_aid_add("search_zxinfo",&first_aid_no_search_zxinfo,first_aid_string_search_zxinfo,0);
    menu_first_aid_add("debug_variables",&first_aid_no_debug_variables,first_aid_string_debug_variables,0);
    menu_first_aid_add("debug_console",&first_aid_no_debug_console,first_aid_string_debug_console,0);
    menu_first_aid_add("back_run_rainbow",&first_aid_no_back_run_rainbow,first_aid_string_back_run_rainbow,0);
    menu_first_aid_add("language",&first_aid_no_language,first_aid_string_language,0);
    menu_first_aid_add("snapshot_save_zsf",&first_aid_no_snapshot_save_zsf,first_aid_string_snapshot_save_zsf,0);
    menu_first_aid_add("hilow_format",&first_aid_no_hilow_format,first_aid_string_hilow_format,0);
    menu_first_aid_add("hilow_convert_bside",&first_aid_no_hilow_convert_bside,first_aid_string_hilow_convert_bside,0);
    menu_first_aid_add("ext_audio_source",&first_aid_no_ext_audio_source,first_aid_string_ext_audio_source,0);


    //Items que se disparan en startup
    menu_first_aid_add("startup_aid",&first_aid_no_startup_aid,first_aid_string_startup_aid,1);
    menu_first_aid_add("multiplattform",&first_aid_no_multiplattform,first_aid_string_multiplattform,1);
    menu_first_aid_add("accessibility",&first_aid_no_accessibility,first_aid_string_accessibility,1);
    menu_first_aid_add("documentation",&first_aid_no_documentation,first_aid_string_documentation,1);
    menu_first_aid_add("zrcp",&first_aid_no_zrcp,first_aid_string_zrcp,1);
    menu_first_aid_add("votext",&first_aid_no_votext,first_aid_string_votext,1);
    menu_first_aid_add("easteregg",&first_aid_no_easteregg,first_aid_string_eastereg,1);
    menu_first_aid_add("multitask",&first_aid_no_multitask,first_aid_string_multitask,1);
    menu_first_aid_add("zxvisionclickout",&first_aid_no_zxvision_clickout,first_aid_string_zxvision_clickout,1);
    menu_first_aid_add("conversion",&first_aid_no_conversion,first_aid_string_conversion,1);
    menu_first_aid_add("fileextensions",&first_aid_no_fileextensions,first_aid_string_fileextensions,1);
    menu_first_aid_add("zsfextension",&first_aid_no_zsfextension,first_aid_string_zsfextension,1);
    menu_first_aid_add("spaceexpand",&first_aid_no_spaceexpand,first_aid_string_spaceexpand,1);
    menu_first_aid_add("backgroundwindows",&first_aid_no_backgroundwindows,first_aid_string_backgroundwindows,1);
    menu_first_aid_add("zxdesktop",&first_aid_no_zxdesktop,first_aid_string_zxdesktop,1);
    menu_first_aid_add("zxdesktopcustombuttons",&first_aid_no_zxdesktop_custombuttons,first_aid_string_zxdesktop_custombuttons,1);
    menu_first_aid_add("advancedmenu",&first_aid_no_advanced_menu,first_aid_string_advanced_menu,1);
    menu_first_aid_add("donate",&first_aid_no_donate,first_aid_string_donate,1);

}

//Mostrar random ayuda al iniciar. No se activa si no hay multitask
//Nota: realmente no son random, salen en orden de aparicion
void menu_first_aid_random_startup(void)
{

    //printf ("menu_first_aid_random_startup\n");
    menu_first_aid_startup=0;

    //Si no hay autoguardado de config, no mostrarlo (pues no se podria desactivar)
    if (save_configuration_file_on_exit.v==0) return;

    //Si desactivadas ayudas first aid
    if (menu_disable_first_aid.v) return;

    //Si desactivado multitask
    if (!menu_multitarea) return;

    //si video driver no permite menu normal (no stdout ni simpletext ni null)
    if (!si_normal_menu_video_driver() ) return;

    //Lanzar la primera que este activa y sea de tipo si_startup=1
    int i;
    int encontrado=0;
    for (i=0;i<total_first_aid && !encontrado;i++) {
        int *valor_opcion;
        if (first_aid_list[i].si_startup) {
            valor_opcion=first_aid_list[i].puntero_setting;
            if ((*valor_opcion)==0) {
                string_config_key_aid_startup=first_aid_list[i].config_name;
                encontrado=1;
                menu_abierto=1;
                menu_first_aid_must_show_startup=1;
            }
        }
    }

    if (string_config_key_aid_startup!=NULL) debug_printf (VERBOSE_DEBUG,"Set first aid of the day to: %s",string_config_key_aid_startup);

}

//Retornar indice a opcion implicada. -1 si no
int menu_first_aid_get_setting(char *texto)
{
    //if (!strcasecmp(texto,"filesel_uppercase_keys")) first_aid_no_filesel_uppercase_keys=1;
    //buscar texto en array
    int i;
    int encontrado=-1;
    for (i=0;i<total_first_aid && encontrado==-1;i++) {
        if (!strcasecmp(texto,first_aid_list[i].config_name)) encontrado=i;
    }

    if (encontrado==-1) {
        debug_printf (VERBOSE_DEBUG,"Can not find first aid setting %s",texto);
        return -1;
    }



    //printf ("setting indice %d nombre [%s]\n",encontrado,first_aid_list[encontrado].config_name);

    //return first_aid_list[i].puntero_setting;

    return encontrado;

}

//Restaurar todos mensages first aid
void menu_first_aid_restore_all(void)
{
    int i;
    for (i=0;i<total_first_aid;i++) {
        int *opcion;
        opcion=first_aid_list[i].puntero_setting;
        *opcion=0;
        }
}

//Deshabilitar first aid de lectura de config. Si no existe, volver sin decir nada
//Asi evitamos que si en un futuro borro algun first aid y alguien lo tenga por config, no de error al no existir
void menu_first_aid_disable(char *texto)
{
    int indice;

    indice=menu_first_aid_get_setting(texto);
    if (indice<0) return;

    int *opcion;
    opcion=first_aid_list[indice].puntero_setting;

    *opcion=1; //desactivarla

}



z80_bit menu_disable_first_aid={0};




 //Mostrar first aid si conviene. Retorna 1 si se ha mostrado. 0 si no
int menu_first_aid_title(char *key_setting,char *title) //(enum first_aid_number_list indice)
{

    //Si no hay autoguardado de config, no mostrarlo (pues no se podria desactivar)
    if (save_configuration_file_on_exit.v==0) return 0;

    //Si desactivadas ayudas first aid
    if (menu_disable_first_aid.v) return 0;

    //Si driver no permite menu normal
    if (!si_normal_menu_video_driver()) return 0;

    int indice=menu_first_aid_get_setting(key_setting);
    if (indice<0) return 0;

    int *valor_opcion;
    char *texto_opcion;


    valor_opcion=first_aid_list[indice].puntero_setting;
    texto_opcion=first_aid_list[indice].texto_opcion;

    //Variable a 1. no mostrar nada
    if (*valor_opcion) return 0;


    //Variable a 0. La marcamos por defecto (que significará: no la muestres de nuevo)
    *valor_opcion=1;

    zxvision_menu_generic_message_setting(title,texto_opcion,"Do not show it again",valor_opcion);

    return 1;

}


//Mostrar first aid si conviene. Retorna 1 si se ha mostrado. 0 si no
int menu_first_aid(char *key_setting) //(enum first_aid_number_list indice)
{
    return menu_first_aid_title(key_setting,"First aid");

}

void menu_network_error(int error)
{
    menu_error_message(z_sock_get_error(error));
}

//Retorna indice del estilo. -1 si no existe
int menu_get_gui_index_by_name(char *nombre)
{
    int i;
    for (i=0;i<ESTILOS_GUI;i++) {
        if (!strcasecmp(nombre,definiciones_estilos_gui[i].nombre_estilo)) {
            return i;
        }
    }

    return -1;

}


//si enlarge_reduce=1, ampliar
//si no, reducir
void menu_ext_desk_settings_width_enlarge_reduce(int enlarge_reduce)
{
    debug_printf(VERBOSE_INFO,"End Screen");

    //Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
    //mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

    screen_end_pantalla_save_overlay(&previous_function,&menu_antes);


    int reorganize_windows=0;


    //Cambio ancho
    //zxdesktop_width *=2;
    //if (zxdesktop_width>=2048) zxdesktop_width=128;

    //Incrementos de 128 hasta llegar a ZXDESKTOP_MAX_WIDTH_MENU_FIXED_INCREMENTS
    //Hacerlo multiple de 127 para evitar valores no multiples de custom width

    zxdesktop_width &=(65535-127);

    if (enlarge_reduce) {

        //Si pasa de cierto limite (1280 a la fecha de escribir este comentario), saltar a 2560, y ese es el limite
        if (zxdesktop_width>=ZXDESKTOP_MAX_WIDTH_MENU_FIXED_INCREMENTS && zxdesktop_width<ZXDESKTOP_MAX_WIDTH_MENU_LIMIT) zxdesktop_width=ZXDESKTOP_MAX_WIDTH_MENU_LIMIT;

        //si pasa del limite maximo, volver a tamaño pequeño
        else if (zxdesktop_width>=ZXDESKTOP_MAX_WIDTH_MENU_LIMIT) {
            zxdesktop_width=128;
            reorganize_windows=1;
        }

        //resto de casos, simplemente incrementar
        else zxdesktop_width +=128;

    }

    else {
        //Si esta entre cierto limite (1280..2560 a la fecha de escribir este comentario), saltar a 1280
        if (zxdesktop_width>ZXDESKTOP_MAX_WIDTH_MENU_FIXED_INCREMENTS && zxdesktop_width<=ZXDESKTOP_MAX_WIDTH_MENU_LIMIT) {
            zxdesktop_width=ZXDESKTOP_MAX_WIDTH_MENU_FIXED_INCREMENTS;
            reorganize_windows=1;
        }

        //Si >=256, decrementar
        else if (zxdesktop_width>=256) {
            zxdesktop_width -=128;
            reorganize_windows=1;
        }

        //resto de casos, no hacer nada, no decrementar mas alla del limite inferior
    }


    screen_init_pantalla_and_others_and_realjoystick();

    debug_printf(VERBOSE_INFO,"Creating Screen");

    menu_init_footer();

    screen_restart_pantalla_restore_overlay(previous_function,menu_antes);


    //Cerrar ventamas y olvidar geometria ventanas
    //zxvision_window_delete_all_windows_and_clear_geometry();

    //Reorganizar ventanas solo si conviene (cuando tamaño pasa a ser menor)
    if (reorganize_windows) {
        zxvision_rearrange_background_windows(0,1);

        //Comprobar posiciones iconos y reajustar
        zxvision_check_all_configurable_icons_positions();
    }

    zxvision_reorder_configurable_icons_if_auto();

    //Conveniente esto para borrar "restos" de ventanas
    cls_menu_overlay();

}

//si enlarge_reduce=1, ampliar
//si no, reducir
void menu_ext_desk_settings_height_enlarge_reduce(int enlarge_reduce)
{
    debug_printf(VERBOSE_INFO,"End Screen");

    //Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
    //mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

    screen_end_pantalla_save_overlay(&previous_function,&menu_antes);

    int incremento=32;


    int reorganize_windows=0;


    //Cambio ancho
    //zxdesktop_width *=2;
    //if (zxdesktop_width>=2048) zxdesktop_width=128;

    //Incrementos de 128 hasta llegar a ZXDESKTOP_MAX_WIDTH_MENU_FIXED_INCREMENTS
    //Hacerlo multiple de 127 para evitar valores no multiples de custom width

    zxdesktop_height &=(65535-(incremento-1));

    if (enlarge_reduce) {

        //Si pasa de cierto limite (1280 a la fecha de escribir este comentario), saltar a 2560, y ese es el limite
        /*if (zxdesktop_height>=ZXDESKTOP_MAX_HEIGHT_MENU_FIXED_INCREMENTS && zxdesktop_height<ZXDESKTOP_MAX_HEIGHT_MENU_LIMIT) zxdesktop_height=ZXDESKTOP_MAX_WIDTH_MENU_LIMIT;

        //si pasa del limite maximo, volver a tamaño pequeño
        else*/

            //no hacer ese salto de 1280 a 2560 que si se hace en horizontal
        if (zxdesktop_height>=ZXDESKTOP_MAX_HEIGHT_MENU_LIMIT ) {
            zxdesktop_height=0;
            reorganize_windows=1;
        }

        //resto de casos, simplemente incrementar
        else zxdesktop_height +=incremento;

    }

    else {
        //Si esta entre cierto limite (1280..2560 a la fecha de escribir este comentario), saltar a 1280
        if (zxdesktop_height>ZXDESKTOP_MAX_HEIGHT_MENU_FIXED_INCREMENTS && zxdesktop_height<=ZXDESKTOP_MAX_HEIGHT_MENU_LIMIT) {
            zxdesktop_height=ZXDESKTOP_MAX_HEIGHT_MENU_FIXED_INCREMENTS;
            reorganize_windows=1;
        }

        //Si >=256, decrementar
        else if (zxdesktop_height>=incremento) {
            zxdesktop_height -=incremento;
            reorganize_windows=1;
        }

        else {
            //resto de casos, pasar a 0
            zxdesktop_height=0;
        }


    }


    screen_init_pantalla_and_others_and_realjoystick();

    debug_printf(VERBOSE_INFO,"Creating Screen");

    menu_init_footer();

    screen_restart_pantalla_restore_overlay(previous_function,menu_antes);


    //Cerrar ventamas y olvidar geometria ventanas
    //zxvision_window_delete_all_windows_and_clear_geometry();

    //Reorganizar ventanas solo si conviene (cuando tamaño pasa a ser menor)
    if (reorganize_windows) {
        zxvision_rearrange_background_windows(0,1);

        //Comprobar posiciones iconos y reajustar
        zxvision_check_all_configurable_icons_positions();

    }

    zxvision_reorder_configurable_icons_if_auto();

    //Conveniente esto para borrar "restos" de ventanas
    cls_menu_overlay();

}





char zxvision_helper_shorcuts_accumulated[MAX_ZXVISION_HELPER_SHORTCUTS_LENGTH]="";
const char *shortcut_helper_initial_text="Menu ";

//Funcion para guardar la tecla de shortcut pulsada para mostrarla en otra ventana al usuario
void zxvision_helper_menu_shortcut_print(int tecla)
{
    if (tecla>32 && tecla<127) {
        char buffer_temp[2];
        sprintf(buffer_temp,"%c",tecla);
        util_concat_string(zxvision_helper_shorcuts_accumulated,buffer_temp,MAX_ZXVISION_HELPER_SHORTCUTS_LENGTH);

        //printf("buffer: %s\n",zxvision_helper_shorcuts_accumulated);
    }
}

void zxvision_helper_menu_shortcut_delete_last(void)
{


    //primero borramos ultimo caracter
    int minima_cadena=strlen(shortcut_helper_initial_text);
    int longitud_texto=strlen(zxvision_helper_shorcuts_accumulated);

    if (longitud_texto>minima_cadena) {
        //printf("borrar\n");
        longitud_texto--;
        zxvision_helper_shorcuts_accumulated[longitud_texto]=0;
    }


}

//Inicializar el buffer al abrir el menu
void zxvision_helper_menu_shortcut_init(void)
{
    strcpy(zxvision_helper_shorcuts_accumulated,shortcut_helper_initial_text);
}

index_menu *first_index_menu;

//Inicializar el indice de menu
void zxvision_index_menu_init(void)
{
    first_index_menu=NULL;
}

//Buscar indice de menu. NULL si no encontrado
index_menu *zxvision_index_search_menu(char *nombre)
{

    //Convertir a string sin acentos etc
    char nombre_sin_acentos[MAX_LENGTH_FULL_PATH_SUBMENU];

    util_convert_utf_no_utf(nombre,nombre_sin_acentos,strlen(nombre));


    index_menu *menu=first_index_menu;

    while (menu!=NULL) {
        //printf("Comparar [%s] [%s]\n",nombre_sin_acentos,menu->titulo_menu);
        if (!strcasecmp(nombre_sin_acentos,menu->titulo_menu)) return menu;

        menu=menu->next_menu;
    }

    return NULL;
}

//Agregar indice de menu
index_menu *zxvision_index_add_menu(char *titulo_menu_orig)
{

    //Convertir a string sin acentos etc
    char titulo_menu[MAX_LENGTH_FULL_PATH_SUBMENU];

    util_convert_utf_no_utf(titulo_menu_orig,titulo_menu,strlen(titulo_menu_orig));

    //Ir hasta el ultimo
    index_menu *antes=first_index_menu;

    if (antes!=NULL) {

        while (antes->next_menu!=NULL) {
            antes=antes->next_menu;
        }
    }

    //Asignar memoria
    index_menu *nuevo_menu=util_malloc(sizeof(index_menu),"Can not allocate memory for index menu");

    //Asignar titulo
    strcpy(nuevo_menu->titulo_menu,titulo_menu);

    //Siguiente menu a NULL
    nuevo_menu->next_menu=NULL;

    //Primer item de ese menu a null tambien
    nuevo_menu->first_item_menu=NULL;

    //Y decirle al menu anterior que el siguiente es este (o si no hay, este es el primero)
    if (antes==NULL) first_index_menu=nuevo_menu;
    else antes->next_menu=nuevo_menu;

    return nuevo_menu;
}

//Agregar una linea de menu a un indice de menu existente
void zxvision_index_add_menu_linea(index_menu *indice_menu,char *nombre_linea_orig)
{

    //Si linea en blanco, no hacer nada
    if (nombre_linea_orig[0]==0) return;

    if (nombre_linea_orig[0]==' ' && nombre_linea_orig[1]==0) return;

    //Ver si excede maximo linea
    if (strlen(nombre_linea_orig)>MAX_TEXTO_OPCION-1) {
        debug_printf(VERBOSE_DEBUG,"Adding menu entry to index exceeds limit: [%s]. Do not add it",nombre_linea_orig);
        return;
    }

    //Convertir a string sin acentos etc
    char nombre_linea[MAX_TEXTO_OPCION];

    util_convert_utf_no_utf(nombre_linea_orig,nombre_linea,strlen(nombre_linea_orig));

    //printf("%02X %02X %02X\n",nombre_linea[0],nombre_linea[1],nombre_linea[2]);

    //if (nombre_linea[0]=='\n' && nombre_linea[1]==0) return;

    //printf("Agregando linea %s a menu %s\n",nombre_linea,indice_menu->titulo_menu);


    //Ir hasta la ultima linea
    index_menu_linea *antes=indice_menu->first_item_menu;

    if (antes!=NULL) {

        while (antes->next_item_menu!=NULL) {
            antes=antes->next_item_menu;
        }
    }

    index_menu_linea *nueva_linea=util_malloc(sizeof(index_menu_linea),"Can not allocate memory for index menu line");

    //Quitar de la linea caracteres hotkey ~~
    char texto_opcion[MAX_TEXTO_OPCION];
    int origen=0;
    int destino=0;

    for (;nombre_linea[origen];origen++,destino++) {
        //Saltar dos ~~ seguidos
        if (nombre_linea[origen]=='~' && nombre_linea[origen+1]=='~') origen+=2;

        texto_opcion[destino]=nombre_linea[origen];
    }

    texto_opcion[destino]=0;

    //Asignar texto linea
    strcpy(nueva_linea->texto_opcion,texto_opcion);

    //printf("Agregando linea despues procesado %s a menu %s\n",texto_opcion,indice_menu->titulo_menu);

    //Siguiente linea a null
    nueva_linea->next_item_menu=NULL;

    //Y decirle a la linea anterior que el siguiente es este (o si no hay, este es el primero)
    if (antes==NULL) indice_menu->first_item_menu=nueva_linea;
    else antes->next_item_menu=nueva_linea;

}

//Quitar todas las lineas de menu de un menu
void zxvision_index_erase_all_menu_lines(index_menu *menu)
{
    index_menu_linea *linea_menu=menu->first_item_menu;

    while (linea_menu!=NULL) {
        //Guardarlo antes de liberar memoria
        index_menu_linea *siguiente_linea=linea_menu->next_item_menu;
        free(linea_menu);

        linea_menu=siguiente_linea;
    }

    menu->first_item_menu=NULL;
}

//Agregar o reemplazar indice de menu
index_menu *zxvision_index_add_replace_menu(char *titulo_menu)
{

    //Si menu en blanco, no hacer nada. Esto no deberia suceder, pero por si acaso
    //if (titulo_menu[0]==0) return;

    //ver si ya existe
    index_menu *menu=zxvision_index_search_menu(titulo_menu);

    if (menu==NULL) {
        //printf("Menu %s no existe. Crear\n",titulo_menu);
        return zxvision_index_add_menu(titulo_menu);
    }
    else {
        //reemplaza, borrar items de menu
        zxvision_index_erase_all_menu_lines(menu);
        //printf("Menu %s ya existe. Reemplazar\n",titulo_menu);
        return menu;
    }
}



index_menu *zxvision_index_entrada_menu(char *titulo)
{

    //printf("Menu [%s]\n",nombre_menu_con_submenu_para_indice);
    index_menu *indice_menu_actual=zxvision_index_add_replace_menu(titulo);

    return indice_menu_actual;
}



void zxvision_index_save_to_disk(void)
{
    if (index_menu_enabled.v==0) return;

    FILE *ptr_configfile;

    ptr_configfile=fopen(ZESARUX_INDEX_MENU_FILE,"wb");
    if (!ptr_configfile) {
        debug_printf(VERBOSE_DEBUG,"Cannot write index menu file %s",ZESARUX_INDEX_MENU_FILE);
        return;
    }

    //fwrite(config_settings, 1, strlen(config_settings), ptr_configfile);

    index_menu *menu=first_index_menu;

    /*
    Formato:

    Menu1
    linea menu1
    linea menu2
    linea menu3
    (linea en blanco)
    Menu2
    linea menu1
    linea menu2
    (linea en blanco)
    Menu3
    linea menu1
    linea menu2
    (linea en blanco)
    _EOF
    */

    while (menu!=NULL) {
        //Y ahora buscar en cada linea de menu
        index_menu_linea *linea_menu=menu->first_item_menu;

        fwrite(menu->titulo_menu, 1, strlen(menu->titulo_menu), ptr_configfile);
        fwrite("\n", 1, 1, ptr_configfile);

        while (linea_menu!=NULL) {
            fwrite(linea_menu->texto_opcion, 1, strlen(linea_menu->texto_opcion), ptr_configfile);
            fwrite("\n", 1, 1, ptr_configfile);

            linea_menu=linea_menu->next_item_menu;
        }


        fwrite("\n", 1, 1, ptr_configfile);
        menu=menu->next_menu;
    }


    fclose(ptr_configfile);

}

int zxvision_index_load_from_disk_read_line(z80_byte *origen,z80_byte *destino,int restante,int max_leer,int *llegado_limite)
{
    *llegado_limite=0;

    int leidos=0;

    while (*origen!='\n' && restante) {
        if (leidos<max_leer) {
            *destino=*origen;
            destino++;
        }
        else {
            *llegado_limite=1;
        }

        origen++;
        leidos++;

        restante--;
    }

    //En el max_leer hay que tener espacio al siguiente byte del 0 final
    *destino=0;
    return leidos+1; //agregar el salto linea como leido
}

void zxvision_index_load_from_disk(void)
{

    if (index_menu_enabled.v==0) return;

    if (!si_existe_archivo(ZESARUX_INDEX_MENU_FILE)) return;

    long long int longitud=get_file_size(ZESARUX_INDEX_MENU_FILE);

    //Si longitud 0 (cosa extraña) volver sin mas
    if (!longitud) return;

    z80_byte *buffer_index=util_malloc(longitud,"Can not allocate memory to read index search file");

    int total_leidos=util_load_file_bytes(buffer_index,ZESARUX_INDEX_MENU_FILE,longitud);

    if (!total_leidos) return;

    z80_byte *puntero=buffer_index;
    int llegado_limite_titulo;
    int llegado_limite_item_menu;

    while (total_leidos>0) {
        //leer titulo menu
        char titulo_menu[MAX_LENGTH_FULL_PATH_SUBMENU];
        int leidos=zxvision_index_load_from_disk_read_line(puntero,(z80_byte *)titulo_menu,total_leidos,MAX_LENGTH_FULL_PATH_SUBMENU-1,&llegado_limite_titulo);
        debug_printf(VERBOSE_DEBUG,"Loading help search index. Menu title: [%s]",titulo_menu);

        total_leidos-=leidos;
        puntero +=leidos;

        if (titulo_menu[0]!=0) {

            index_menu *indice_menu_actual=NULL;

            if (!llegado_limite_titulo) {
                indice_menu_actual=zxvision_index_add_replace_menu(titulo_menu);
            }
            else {
                debug_printf(VERBOSE_DEBUG,"Do not add menu [%s] because it exceeds length limit",titulo_menu);
            }

            //leer items de menu
            int salir=0;
            while (!salir) {

                char linea_menu[MAX_TEXTO_OPCION];
                int leidos=zxvision_index_load_from_disk_read_line(puntero,(z80_byte *)linea_menu,total_leidos,MAX_TEXTO_OPCION-1,&llegado_limite_item_menu);

                if (linea_menu[0]==0) salir=1;
                else {
                    debug_printf(VERBOSE_DEBUG,"Loading help search index. Menu entry: [%s]",linea_menu);

                    if (indice_menu_actual!=NULL && !llegado_limite_item_menu) {
                        zxvision_index_add_menu_linea(indice_menu_actual,linea_menu);
                    }
                    else {
                        debug_printf(VERBOSE_DEBUG,"Do not add menu item [%s] because it exceeds length limit",linea_menu);
                    }
                }

                total_leidos-=leidos;
                puntero +=leidos;
            }
        }
    }

    free(buffer_index);
}

void zxvision_change_gui_style(int estilo)
{

    if (estilo<0 || estilo>=ESTILOS_GUI) {
        debug_printf(VERBOSE_ERR,"Can't change GUI style to id %d",estilo);
        return;
    }

    estilo_gui_activo=estilo;
    set_charset_from_gui();
}

//Ajusta estilo del driver de video si este no es driver completo y el seleccionado necesita un driver completo
void menu_adjust_gui_style_to_driver(void)
{

    if (si_complete_video_driver()) {
        //printf("Driver completo. no hacer nada\n");
        return;
    }

    if (definiciones_estilos_gui[estilo_gui_activo].require_complete_video_driver==0) {
        //printf("Driver no completo pero estilo no requiere completo. No hacer nada\n");
        return;
    }

    //Buscar el primero que no necesite driver completo
    int i;
    for (i=0;i<ESTILOS_GUI;i++) {

        if (definiciones_estilos_gui[i].require_complete_video_driver==0) {
            debug_printf(VERBOSE_INFO,"Altering GUI style from %s to %s because current video driver does not allow it",
                definiciones_estilos_gui[estilo_gui_activo].nombre_estilo,
                definiciones_estilos_gui[i].nombre_estilo);

            zxvision_change_gui_style(i);


            return;
        }
    }


}

void add_window_to_restore(char *nombre_ventana)
{
//char restore_window_array[MAX_RESTORE_WINDOWS_START][MAX_NAME_WINDOW_GEOMETRY];
    if (total_restore_window_array_elements>=MAX_RESTORE_WINDOWS_START) {
        debug_printf(VERBOSE_ERR,"Maximum windows to restore reached (%d)",MAX_RESTORE_WINDOWS_START);
    }

    else {

        strcpy(restore_window_array[total_restore_window_array_elements++],nombre_ventana);

    }
}

void zxvision_sound_event_aux(char *nota,int duracion)
{
    if (accessibility_enable_gui_sounds.v) audio_menu_tone_generator_play_note(nota,duracion);
}

void zxvision_sound_event_error_menu(void)
{
    //printf("sonido error\n");

    //Este sonido de error tiene prioridad. Si hay alguno sonando, lo cancelamos para que suene este
    audio_menu_tone_generator_stop();

    zxvision_sound_event_aux("C2",50);
}


//TODO: esto me gustaria que sonase como los botones de reloj Casio, que en teoria, la frecuencia se acerca a 8820 Hz
//aunque como el sampleado de ZEsarUX es 15600, esto no tiene calidad suficiente
//parece que ademas la onda va con un envolvente, no tiene siempre la misma amplitud
void zxvision_sound_event_cursor_movement(void)
{
    zxvision_sound_event_aux("C6",5);
}

//Y este estaria bien que fuese el del mismo boton de Casio pero cuando llegas a la pantalla inicial del reloj
//Cierre de ventana, excepto para ventanas de tipo menu, porque si no, es confuso,
//porque al moverse por menus, se iria escuchando el de cierre, aunque se van abriendo submenus
void zxvision_sound_event_close_window(void)
{
    //printf("sound close\n");
    zxvision_sound_event_aux("C7",5);
}

//TODO: complicado saber cuando disparar este evento,
//y que no sea por ejemplo cuando se restauran ventanas en startup por ejemplo
//de momento no se llama
void zxvision_sound_event_new_window(void)
{
    zxvision_sound_event_aux("C4",5);
}


void set_menu_gui_zoom(void)
{
    //Ajustar zoom del gui. por defecto 1
    menu_gui_zoom=1;
    //printf ("calling set_menu_gui_zoom. driver: %s\n",scr_new_driver_name);
    //printf("machine id: %d si_complete_video_driver: %d\n",current_machine_type ,si_complete_video_driver() );

    //Realmente da igual mirar si driver video completo: como el menu gui zoom solo se usa en drivers
    //completos, no hace falta testear. Ademas, aqui al iniciar el emulador se llama
    //antes de iniciar el driver de video, por tanto el driver de video no existe aun y no retornaria que es completo
    //esto provocaria que al iniciar tbblue por ejemplo, el gui zoom no fuera 2
    //if (si_complete_video_driver() ) {
        if (MACHINE_IS_QL || MACHINE_IS_TSCONF || MACHINE_IS_CPC || MACHINE_IS_PCW || MACHINE_IS_PRISM || MACHINE_IS_SAM || MACHINE_IS_TBBLUE) menu_gui_zoom=2;
    //}

    debug_printf (VERBOSE_INFO,"Setting GUI menu zoom to %d",menu_gui_zoom);
}

#define MAX_PARAMETERS_ZMENU_FILE 1000

char *zmenufile_argv[MAX_PARAMETERS_ZMENU_FILE];
int zmenufile_argc=0;

#define MAX_SIZE_ZMENU_FILE 131072

char *zmenu_parse_file_mem_pointer=NULL;

char zmenu_launcher_title[256];

//Ultimo archivo cargado de zmenu
char last_zmenu_loaded_file[PATH_MAX]="";

void parse_zmenu_launch_entry(menu_item *item_seleccionado)
{
    //Montar ruta considerando path actual para uso posterior
    char directorio[PATH_MAX];
    util_get_dir(last_zmenu_loaded_file,directorio);

    char ruta_desde_path_zmenu[PATH_MAX];
    sprintf(ruta_desde_path_zmenu,"%s/%s",directorio,item_seleccionado->texto_misc);


    //localizar archivo o carpeta
    char buffer_nombre[PATH_MAX];

    //primero ver si existe en ruta donde esta el archivo zmenu
    if (si_existe_archivo(ruta_desde_path_zmenu)) {
        strcpy(buffer_nombre,ruta_desde_path_zmenu);
    }

    //Y si no, buscar en path actual y luego rutas comunes
    else {
        int existe=find_sharedfile(item_seleccionado->texto_misc,buffer_nombre);
        if (!existe)  {
            debug_printf(VERBOSE_ERR,"Unable to find %s",item_seleccionado->texto_misc);
            return;
        }
    }

    z80_bit pre_noautoload;

    //Forzar autoload
    pre_noautoload.v=noautoload.v;
    noautoload.v=0;

    if (item_seleccionado->valor_opcion==1) {
        //Ruta mdv1 QL

        if (!MACHINE_IS_QL) {
            current_machine_type=MACHINE_ID_QL_STANDARD;
            set_machine(NULL);

            //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();

            reset_cpu();
        }

        ql_microdrive_floppy_emulation=1;
        ql_insert_mdv_flp(QL_QDOS_UNIT_MDV1,buffer_nombre);
    }

    //smartload
    else {
        strcpy(quickload_file,buffer_nombre);
        quickfile=quickload_file;

        quickload(quickload_file);
    }

    //Restaurar setting autoload
    noautoload.v=pre_noautoload.v;

}

void parse_zmenufile_options(void)
{

    //opciones para diferentes acciones dentro de un zmenu. Solo permitir una a la vez
    int es_un_lanzador_juegos=0;


    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    int opcion_seleccionada=0;


    while (!siguiente_parametro()) {

        debug_printf (VERBOSE_DEBUG,"Parsing setting %s",argv[puntero_parametro]);

        if (!strcmp(argv[puntero_parametro],"--set-launcher")) {
            es_un_lanzador_juegos=1;
            siguiente_parametro_argumento();

            strcpy(zmenu_launcher_title,argv[puntero_parametro]);

            menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);


        }

        else if (!strcmp(argv[puntero_parametro],"--add-launcher-entry-smartload")
                || !strcmp(argv[puntero_parametro],"--add-launcher-entry-ql-mdv")
            ) {
            if (!es_un_lanzador_juegos) {
                debug_printf (VERBOSE_ERR,"You must set launcher before adding entries");
                return;
            }

            //Por defecto, es smartload
            int valor_opcion=0;

            //Si es Path ql
            if (!strcmp(argv[puntero_parametro],"--add-launcher-entry-ql-mdv")) {
                valor_opcion=1;
            }

            siguiente_parametro_argumento();

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,argv[puntero_parametro]);


            siguiente_parametro_argumento();

            menu_add_item_menu_misc(array_menu_common,argv[puntero_parametro]);


            menu_add_item_menu_valor_opcion(array_menu_common,valor_opcion);


        }


        //
        //Opcion no reconocida. Error
        //
        else {
            debug_printf (VERBOSE_ERR,"Setting %s not allowed on zmenu file",argv[puntero_parametro]);
            return;
        }
    }

    if (es_un_lanzador_juegos) {
        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&opcion_seleccionada,&item_seleccionado,array_menu_common,zmenu_launcher_title);



        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

            parse_zmenu_launch_entry(&item_seleccionado);


        }

        //} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


    }
    else {
        debug_printf (VERBOSE_ERR,"You haven't defined a menu type");
        return;
    }



}

//1 si ok
//0 si error
int zmenufile_read_aux(char *configfile,char *mem)
{

        //Avisar si tamanyo grande
        if (get_file_size(configfile) > (long long int)MAX_SIZE_ZMENU_FILE) cpu_panic("Configuration file is larger than maximum size allowed");

    FILE *ptr_configfile;
        ptr_configfile=fopen(configfile,"rb");


        if (!ptr_configfile) {
                printf("Unable to open configuration file %s\n",configfile);
                return 0;
        }

        int leidos=fread(mem,1,MAX_SIZE_ZMENU_FILE,ptr_configfile);

        //Poner un codigo 0 al final
        mem[leidos]=0;


        fclose(ptr_configfile);

        return 1;

}


//Parsear archivo indicado de zmenu
//Retorna 0 si ok
int zmenu_parse_file_aux(char *archivo)
{

    //Ver si hay que asignar memoria
    //Debe quedar esta memoria asignada al salir de este procedimiento, pues lo usa en parse_customfile_options
    if (zmenu_parse_file_mem_pointer==NULL) {
        debug_printf (VERBOSE_DEBUG,"Allocating memory to read zmenu file");
        zmenu_parse_file_mem_pointer=malloc(MAX_SIZE_ZMENU_FILE+1);
    }

    else {
        debug_printf (VERBOSE_DEBUG,"No need to allocate memory to read zmenu file, we allocated it before");
    }


        if (zmenu_parse_file_mem_pointer==NULL) {
                cpu_panic("Unable to allocate memory for configuration file");
        }


    //Valor inicial
    zmenufile_argc=0;


        if (zmenufile_read_aux(archivo,zmenu_parse_file_mem_pointer)==0) {
                //No hay archivo de configuracion. Parametros vacios
                zmenufile_argv[0]="";
                zmenufile_argc=1;

                return 1;
        }


        configfile_parse_lines(zmenu_parse_file_mem_pointer,&zmenufile_argv[0],&zmenufile_argc);

        return 0;


}


//Parsear archivo de zmenu
void zmenu_parse_file(char *archivo)
{

    //Guardar ruta al archivo para uso posterior
    strcpy(last_zmenu_loaded_file,archivo);

    int retorno=zmenu_parse_file_aux(archivo);
    if (!retorno) {
        argc=zmenufile_argc;
        argv=zmenufile_argv;
        puntero_parametro=0;

        parse_zmenufile_options();
    }
}

void zxvision_vecdraw_get_real_coords(struct zxvision_vectorial_draw *d,int virtual_x,int virtual_y,
    int *p_real_x,int *p_real_y)
{
    //Convertir coordenada
    int real_x=(virtual_x*d->real_width)/d->virtual_width;
    int real_y=(virtual_y*d->real_heigth)/d->virtual_height;

    *p_real_x=real_x;
    *p_real_y=real_y;

}


/*void zxvision_vecdraw_putpixel(zxvision_window *ventana,int x,int y,int color)
{
    zxvision_putpixel(ventana,x,y,color);
}
*/

void zxvision_vecdraw_circle(struct zxvision_vectorial_draw *d,int radio)
{
    //calcular coordenadas reales
    int real_x,real_y;
    zxvision_vecdraw_get_real_coords(d,d->virtual_x,d->virtual_y,&real_x,&real_y);

    //calcular radio real
    int real_radio=(radio*d->real_width)/d->virtual_width;

    zxvision_draw_ellipse(d->ventana,real_x+d->offset_x,real_y+d->offset_y,real_radio,real_radio,d->pencil_colour,zxvision_putpixel,360);

}

void zxvision_vecdraw_arc(struct zxvision_vectorial_draw *d,int radio,int inicio_grados,int final_grados)
{
    //calcular coordenadas reales
    int real_x,real_y;
    zxvision_vecdraw_get_real_coords(d,d->virtual_x,d->virtual_y,&real_x,&real_y);

    //calcular radio real
    int real_radio=(radio*d->real_width)/d->virtual_width;

    zxvision_draw_arc(d->ventana,real_x+d->offset_x,real_y+d->offset_y,real_radio,real_radio,d->pencil_colour,
        zxvision_putpixel,inicio_grados,final_grados);

}


void zxvision_vecdraw_drawfilledrectangle(struct zxvision_vectorial_draw *d,int ancho,int alto)
{
    //calcular coordenadas reales
    int real_x,real_y;
    zxvision_vecdraw_get_real_coords(d,d->virtual_x,d->virtual_y,&real_x,&real_y);

    //calcular ancho real
    int real_ancho=(ancho*d->real_width)/d->virtual_width;
    int real_alto=(alto*d->real_heigth)/d->virtual_height;

    for (;real_alto>0;real_alto--) {
        zxvision_draw_line(d->ventana,
            real_x+d->offset_x,real_y+d->offset_y,
            real_x+d->offset_x+real_ancho-1,real_y+d->offset_y,
            d->pencil_colour,
            zxvision_putpixel
        );
        real_y++;
    }



}

//Funciones de dibujo vectorial, se asocian a la estructura como si fuera orientacion a objetos, aunque no lo es

void zxvision_vecdraw_pencil_on(struct zxvision_vectorial_draw *d)
{
    d->pencil_enabled=1;
}

void zxvision_vecdraw_pencil_off(struct zxvision_vectorial_draw *d)
{
    d->pencil_enabled=0;
}

void zxvision_vecdraw_setcolour(struct zxvision_vectorial_draw *d,int colour)
{
    d->pencil_colour=colour;
}

void zxvision_vecdraw_jumppos(struct zxvision_vectorial_draw *d,int virtual_x,int virtual_y)
{

    d->virtual_x=virtual_x;
    d->virtual_y=virtual_y;

    //calcular coordenadas reales
    int real_x,real_y;
    zxvision_vecdraw_get_real_coords(d,virtual_x,virtual_y,&real_x,&real_y);

    d->real_x=real_x;
    d->real_y=real_y;


}

void zxvision_vecdraw_setpos(struct zxvision_vectorial_draw *d,int virtual_x,int virtual_y)
{

    int old_real_x=d->real_x;
    int old_real_y=d->real_y;


    //calcular coordenadas reales
    int real_x,real_y;
    zxvision_vecdraw_get_real_coords(d,virtual_x,virtual_y,&real_x,&real_y);


    zxvision_vecdraw_jumppos(d,virtual_x,virtual_y);


    if (d->pencil_enabled) {
        //hay que trazar linea
        zxvision_draw_line(d->ventana,
            old_real_x+d->offset_x,old_real_y+d->offset_y,
            real_x+d->offset_x,real_y+d->offset_y,
            d->pencil_colour,
            zxvision_putpixel
        );
    }

}

void zxvision_vecdraw_setrelpos(struct zxvision_vectorial_draw *d,int rel_virtual_x,int rel_virtual_y)
{

    int old_real_x=d->real_x;
    int old_real_y=d->real_y;

    int virtual_x=d->virtual_x+rel_virtual_x;
    int virtual_y=d->virtual_y+rel_virtual_y;

    //calcular coordenadas reales
    int real_x,real_y;
    zxvision_vecdraw_get_real_coords(d,virtual_x,virtual_y,&real_x,&real_y);


    zxvision_vecdraw_jumppos(d,virtual_x,virtual_y);


    if (d->pencil_enabled) {
        //hay que trazar linea
        zxvision_draw_line(d->ventana,
            old_real_x+d->offset_x,old_real_y+d->offset_y,
            real_x+d->offset_x,real_y+d->offset_y,
            d->pencil_colour,
            zxvision_putpixel
        );
    }

}

void zxvision_vecdraw_set_x(struct zxvision_vectorial_draw *d,int virtual_x)
{
    int virtual_y=d->virtual_y;
    zxvision_vecdraw_setpos(d,virtual_x,virtual_y);
}

void zxvision_vecdraw_set_y(struct zxvision_vectorial_draw *d,int virtual_y)
{
    int virtual_x=d->virtual_x;
    zxvision_vecdraw_setpos(d,virtual_x,virtual_y);
}

//Inicializar estructura de dibujo vectorial
void zxvision_vecdraw_init(struct zxvision_vectorial_draw *d,zxvision_window *w,int virtual_width,int virtual_height,
    int real_width,int real_height,int offset_x,int offset_y)
{

    d->virtual_width=virtual_width;
    d->virtual_height=virtual_height;
    d->real_width=real_width;
    d->real_heigth=real_height;
    d->ventana=w;
    d->offset_x=offset_x;
    d->offset_y=offset_y;

    d->virtual_x=d->virtual_y=d->real_x=d->real_y=0;
    d->pencil_enabled=0;
    d->pencil_colour=0;

    //Acciones posibles
    d->setpos=zxvision_vecdraw_setpos;
    d->setrelpos=zxvision_vecdraw_setrelpos;
    d->jumppos=zxvision_vecdraw_jumppos;
    d->set_x=zxvision_vecdraw_set_x;
    d->set_y=zxvision_vecdraw_set_y;
    d->pencil_on=zxvision_vecdraw_pencil_on;
    d->pencil_off=zxvision_vecdraw_pencil_off;
    d->setcolour=zxvision_vecdraw_setcolour;
    d->drawcircle=zxvision_vecdraw_circle;
    d->drawarc=zxvision_vecdraw_arc;
    d->drawfilledrectangle=zxvision_vecdraw_drawfilledrectangle;
}