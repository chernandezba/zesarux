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


//
// Archivo para entradas de menu de Storage
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
#include "menu_items_storage.h"
#include "menu_items.h"
#include "menu_items_settings.h"
#include "menu_debug_cpu.h"
#include "menu_file_viewer_browser.h"
#include "menu_filesel.h"
#include "menu_zeng_online.h"
#include "menu_bitmaps.h"
#include "screen.h"
#include "cpu.h"
#include "start.h"
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
#include "settings.h"
#include "datagear.h"
#include "assemble.h"
#include "expression_parser.h"
#include "uartbridge.h"
#include "zeng.h"
#include "network.h"
#include "stats.h"
#include "vdp_9918a.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "sn76489an.h"
#include "svi.h"
#include "ql_qdos_handler.h"
#include "ql_i8049.h"
#include "gs.h"
#include "zvfs.h"
#include "vdp_9918a_sms.h"
#include "snap_ram.h"
#include "sensors.h"
#include "samram.h"
#include "hilow_datadrive.h"
#include "utils_text_adventure.h"
#include "about_logo.h"
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
#include "zeng_online.h"
#include "mk14.h"
#include "microdrive.h"
#include "microdrive_raw.h"
#include "lec.h"
#include "zesarux.h"


//Opciones seleccionadas para cada menu
int kartusho_opcion_seleccionada=0;
//Fin opciones seleccionadas para cada menu




void menu_kartusho_rom_file(MENU_ITEM_PARAMETERS)
{
	kartusho_disable();

        char *filtros[2];

        filtros[0]="rom";
        filtros[1]=0;


        if (menu_filesel("Select kartusho File",filtros,kartusho_rom_file_name)==1) {
                if (!si_existe_archivo(kartusho_rom_file_name)) {
                        menu_error_message("File does not exist");
                        kartusho_rom_file_name[0]=0;
                        return;



                }

                else {
                        //Comprobar aqui tambien el tamanyo
                        long long int size=get_file_size(kartusho_rom_file_name);
                        if (size!=KARTUSHO_SIZE) {
                                menu_error_message("ROM file must be 512 KB length");
                                kartusho_rom_file_name[0]=0;
                                return;
                        }
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                kartusho_rom_file_name[0]=0;


        }

}

int menu_storage_kartusho_emulation_cond(void)
{
	if (kartusho_rom_file_name[0]==0) return 0;
        return 1;
}

int menu_storage_kartusho_press_button_cond(void)
{
	return kartusho_enabled.v;
}


void menu_storage_kartusho_emulation(MENU_ITEM_PARAMETERS)
{
	if (kartusho_enabled.v) kartusho_disable();
	else kartusho_enable();
}

void menu_storage_kartusho_press_button(MENU_ITEM_PARAMETERS)
{
	kartusho_press_button();
	//Y salimos de todos los menus
	salir_todos_menus=1;

}

void menu_kartusho(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_kartusho;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_kartusho_file_shown[13];


                        menu_tape_settings_trunc_name(kartusho_rom_file_name,string_kartusho_file_shown,13);
                        menu_add_item_menu_inicial_format(&array_menu_kartusho,MENU_OPCION_NORMAL,menu_kartusho_rom_file,NULL,"~~ROM File [%s]",string_kartusho_file_shown);
                        menu_add_item_menu_prefijo(array_menu_kartusho,"    ");
                        menu_add_item_menu_shortcut(array_menu_kartusho,'r');
                        menu_add_item_menu_tooltip(array_menu_kartusho,"ROM Emulation file");
                        menu_add_item_menu_ayuda(array_menu_kartusho,"ROM Emulation file");


                        			menu_add_item_menu_format(array_menu_kartusho,MENU_OPCION_NORMAL,menu_storage_kartusho_emulation,menu_storage_kartusho_emulation_cond,"[%c] ~~Kartusho Enabled", (kartusho_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_kartusho,'k');
                        menu_add_item_menu_tooltip(array_menu_kartusho,"Enable kartusho");
                        menu_add_item_menu_ayuda(array_menu_kartusho,"Enable kartusho");


			menu_add_item_menu_format(array_menu_kartusho,MENU_OPCION_NORMAL,menu_storage_kartusho_press_button,menu_storage_kartusho_press_button_cond,"~~Press button");
             menu_add_item_menu_prefijo(array_menu_kartusho,"    ");
			menu_add_item_menu_shortcut(array_menu_kartusho,'p');
                        menu_add_item_menu_tooltip(array_menu_kartusho,"Press button");
                        menu_add_item_menu_ayuda(array_menu_kartusho,"Press button");


                                menu_add_item_menu(array_menu_kartusho,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_kartusho);

                retorno_menu=menu_dibuja_menu_no_title_lang(&kartusho_opcion_seleccionada,&item_seleccionado,array_menu_kartusho,"Kartusho" );


                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}

