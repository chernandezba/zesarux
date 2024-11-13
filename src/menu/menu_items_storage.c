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
int superupgrade_opcion_seleccionada=0;
int ifrom_opcion_seleccionada=0;
int timexcart_opcion_seleccionada=0;
int dandanator_opcion_seleccionada=0;
int samram_opcion_seleccionada=0;
int hilow_opcion_seleccionada=0;
int storage_tape_opcion_seleccionada=0;
int visualhilow_opcion_seleccionada=0;
int hilow_barbanegra_opcion_seleccionada=0;
int interface1_opcion_seleccionada=0;
int visualmicrodrive_opcion_seleccionada=0;
int menu_mdv_simulate_bad_opcion_seleccionada=0;
int betadisk_opcion_seleccionada=0;
int zxpand_opcion_seleccionada=0;
int esxdos_traps_opcion_seleccionada=0;
int visualfloppy_opcion_seleccionada=0;
int menu_plusthreedisk_info_sectors_list_opcion_seleccionada=0;
int menu_plusthreedisk_info_tracks_list_opcion_seleccionada=0;
int menu_plusthreedisk_info_opcion_seleccionada=0;
int plusthreedisk_opcion_seleccionada=0;

//Fin opciones seleccionadas para cada menu


char last_timex_cart[PATH_MAX]="";

//cinta seleccionada. tapefile apuntara aqui
char tape_open_file[PATH_MAX];
//cinta seleccionada. tape_out_file apuntara aqui
char tape_out_open_file[PATH_MAX];

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




void menu_superupgrade_rom_file(MENU_ITEM_PARAMETERS)
{
        superupgrade_disable();

        char *filtros[2];

        filtros[0]="flash";
        filtros[1]=0;


        if (menu_filesel("Select Superupgrade File",filtros,superupgrade_rom_file_name)==1) {
                if (!si_existe_archivo(superupgrade_rom_file_name)) {
                        menu_error_message("File does not exist");
                        superupgrade_rom_file_name[0]=0;
                        return;



                }

                else {
                        //Comprobar aqui tambien el tamanyo
                        long long int size=get_file_size(superupgrade_rom_file_name);
                        if (size!=SUPERUPGRADE_ROM_SIZE) {
                                menu_error_message("Flash file must be 512 KB length");
                                superupgrade_rom_file_name[0]=0;
                                return;
                        }
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                superupgrade_rom_file_name[0]=0;

        }

}

int menu_storage_superupgrade_emulation_cond(void)
{
        if (superupgrade_rom_file_name[0]==0) return 0;
        return 1;
}


void menu_storage_superupgrade_emulation(MENU_ITEM_PARAMETERS)
{
        if (superupgrade_enabled.v) superupgrade_disable();
        else superupgrade_enable(1);
}

void menu_storage_superupgrade_internal_rom(MENU_ITEM_PARAMETERS)
{
		//superupgrade_puerto_43b ^=0x20;
		//if ( (superupgrade_puerto_43b & (32+64))==32) return 1;

		superupgrade_puerto_43b &=(255-32-64);
		superupgrade_puerto_43b |=32;
}



void menu_superupgrade(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_superupgrade;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_superupgrade_file_shown[13];


                        menu_tape_settings_trunc_name(superupgrade_rom_file_name,string_superupgrade_file_shown,13);
                        menu_add_item_menu_inicial_format(&array_menu_superupgrade,MENU_OPCION_NORMAL,menu_superupgrade_rom_file,NULL,"~~Flash File [%s]",string_superupgrade_file_shown);
                        menu_add_item_menu_prefijo(array_menu_superupgrade,"    ");
                        menu_add_item_menu_shortcut(array_menu_superupgrade,'f');
                        menu_add_item_menu_tooltip(array_menu_superupgrade,"Flash Emulation file");
                        menu_add_item_menu_ayuda(array_menu_superupgrade,"Flash Emulation file");


                        menu_add_item_menu_format(array_menu_superupgrade,MENU_OPCION_NORMAL,menu_storage_superupgrade_emulation,menu_storage_superupgrade_emulation_cond,"[%c] ~~Superupgrade Enabled", (superupgrade_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_superupgrade,'s');
                        menu_add_item_menu_tooltip(array_menu_superupgrade,"Enable superupgrade");
                        menu_add_item_menu_ayuda(array_menu_superupgrade,"Enable superupgrade");


												menu_add_item_menu_format(array_menu_superupgrade,MENU_OPCION_NORMAL,menu_storage_superupgrade_internal_rom,menu_storage_superupgrade_emulation_cond,"[%c] Show ~~internal ROM", (si_superupgrade_muestra_rom_interna() ? 'X' : ' '));
												menu_add_item_menu_shortcut(array_menu_superupgrade,'i');
												menu_add_item_menu_tooltip(array_menu_superupgrade,"Show internal ROM instead of Superupgrade flash");
												menu_add_item_menu_ayuda(array_menu_superupgrade,"Show internal ROM instead of Superupgrade flash");



                                menu_add_item_menu(array_menu_superupgrade,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_superupgrade);


retorno_menu=menu_dibuja_menu_no_title_lang(&superupgrade_opcion_seleccionada,&item_seleccionado,array_menu_superupgrade,"Superupgrade" );


                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}




void menu_ifrom_rom_file(MENU_ITEM_PARAMETERS)
{
	ifrom_disable();

        char *filtros[2];

        filtros[0]="rom";
        filtros[1]=0;


        if (menu_filesel("Select ifrom File",filtros,ifrom_rom_file_name)==1) {
                if (!si_existe_archivo(ifrom_rom_file_name)) {
                        menu_error_message("File does not exist");
                        ifrom_rom_file_name[0]=0;
                        return;



                }

                else {
                        //Comprobar aqui tambien el tamanyo
                        long long int size=get_file_size(ifrom_rom_file_name);
                        if (size!=IFROM_SIZE) {
                                menu_error_message("ROM file must be 512 KB length");
                                ifrom_rom_file_name[0]=0;
                                return;
                        }
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                ifrom_rom_file_name[0]=0;


        }

}

int menu_storage_ifrom_emulation_cond(void)
{
	if (ifrom_rom_file_name[0]==0) return 0;
        return 1;
}

int menu_storage_ifrom_press_button_cond(void)
{
	return ifrom_enabled.v;
}


void menu_storage_ifrom_emulation(MENU_ITEM_PARAMETERS)
{
	if (ifrom_enabled.v) ifrom_disable();
	else ifrom_enable();
}

void menu_storage_ifrom_press_button(MENU_ITEM_PARAMETERS)
{
	ifrom_press_button();
	//Y salimos de todos los menus
	salir_todos_menus=1;

}

void menu_ifrom(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_ifrom;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_ifrom_file_shown[13];


                        menu_tape_settings_trunc_name(ifrom_rom_file_name,string_ifrom_file_shown,13);
                        menu_add_item_menu_inicial_format(&array_menu_ifrom,MENU_OPCION_NORMAL,menu_ifrom_rom_file,NULL,"~~ROM File [%s]",string_ifrom_file_shown);
                        menu_add_item_menu_prefijo(array_menu_ifrom,"    ");
                        menu_add_item_menu_shortcut(array_menu_ifrom,'r');
                        menu_add_item_menu_tooltip(array_menu_ifrom,"ROM Emulation file");
                        menu_add_item_menu_ayuda(array_menu_ifrom,"ROM Emulation file");


                        			menu_add_item_menu_format(array_menu_ifrom,MENU_OPCION_NORMAL,menu_storage_ifrom_emulation,menu_storage_ifrom_emulation_cond,"[%c] ~~iFrom Enabled", (ifrom_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_ifrom,'i');
                        menu_add_item_menu_tooltip(array_menu_ifrom,"Enable ifrom");
                        menu_add_item_menu_ayuda(array_menu_ifrom,"Enable ifrom");


			menu_add_item_menu_format(array_menu_ifrom,MENU_OPCION_NORMAL,menu_storage_ifrom_press_button,menu_storage_ifrom_press_button_cond,"~~Press button");
            menu_add_item_menu_prefijo(array_menu_ifrom,"    ");
			menu_add_item_menu_shortcut(array_menu_ifrom,'p');
                        menu_add_item_menu_tooltip(array_menu_ifrom,"Press button");
                        menu_add_item_menu_ayuda(array_menu_ifrom,"Press button");


                                menu_add_item_menu(array_menu_ifrom,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_ifrom);

                retorno_menu=menu_dibuja_menu_no_title_lang(&ifrom_opcion_seleccionada,&item_seleccionado,array_menu_ifrom,"iFrom" );


                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


void menu_storage_tape_my_soft(MENU_ITEM_PARAMETERS)
{

    char buffer_nombre[PATH_MAX];

	if (find_sharedfile("my_soft/spectrum/vintage/",buffer_nombre)) {
		debug_printf(VERBOSE_INFO,"Loading tape copier %s",buffer_nombre);
		strcpy(quickload_file,buffer_nombre);
		quickfile=quickload_file;

        menu_smartload(0);

        salir_todos_menus=1;
    }
    else {
        debug_printf(VERBOSE_ERR,"Can't find my software");
    }
}

void menu_storage_tape_copier(MENU_ITEM_PARAMETERS)
{

    char copion[256]="";

    int opcion=menu_simple_eight_choices("Tape copier","Select one",
        "Copiador Primi 2  (48K)",
        "Copiador Azul     (48K)",
        "Duplitape         (48K)",
        "Duplitape2        (48K)",
        "Copion9           (48K)",
        "Mancopy           (48K)",
        "Lao-Copy 2        (48K)",
        "SuperTapeCopier  (128K)"
    );

    switch(opcion) {

        case 1:
            strcpy(copion,"copiadorprimi2.zsf");
        break;

        case 2:
            strcpy(copion,"copiador.zsf");
        break;

        case 3:
            strcpy(copion,"duplitape.zsf");
        break;

        case 4:
            strcpy(copion,"duplitape2.zsf");
        break;

        case 5:
            strcpy(copion,"copion9.zsf");
        break;

        case 6:
            strcpy(copion,"mancopy.zsf");
        break;

        case 7:
            strcpy(copion,"laocopy2.zsf");
        break;

        case 8:
            strcpy(copion,"supertapecopier.zsf");
        break;


        default:
            return;
        break;
    }

    char copion_con_carpeta[PATH_MAX];
    sprintf(copion_con_carpeta,"copiers/%s",copion);
    char buffer_nombre[PATH_MAX];

	if (find_sharedfile(copion_con_carpeta,buffer_nombre)) {
		debug_printf(VERBOSE_INFO,"Loading tape copier %s",buffer_nombre);
		strcpy(quickload_file,buffer_nombre);
		quickfile=quickload_file;
		//Forzar autoload
		z80_bit pre_noautoload;
		pre_noautoload.v=noautoload.v;
		noautoload.v=0;
		quickload(quickload_file);

		noautoload.v=pre_noautoload.v;
        salir_todos_menus=1;
    }
    else {
        debug_printf(VERBOSE_ERR,"Tape copier %s not found",copion);
    }
}


void menu_tape_open(MENU_ITEM_PARAMETERS)
{

        char *filtros[8];

	if (MACHINE_IS_ZX80_TYPE) {
		filtros[0]="80";
        	filtros[1]="o";
        	filtros[2]="rwa";
        	filtros[3]="smp";
        	filtros[4]="wav";
        	filtros[5]="z81";
        	filtros[6]=0;
	}

	else if (MACHINE_IS_ZX81_TYPE) {
                filtros[0]="p";
                filtros[1]="81";
                filtros[2]="p81";
                filtros[3]="rwa";
                filtros[4]="smp";
                filtros[5]="z81";
                filtros[6]="wav";
                filtros[7]=0;
        }

	else if (MACHINE_IS_MSX) {
                filtros[0]="cas";
                filtros[1]=0;
        }

	else if (MACHINE_IS_SVI) {
                filtros[0]="cas";
                filtros[1]=0;
        }

	else {
        filtros[0]="tzx";
        filtros[1]="tap";
        filtros[2]="pzx";
        filtros[3]="rwa";
        filtros[4]="smp";
        filtros[5]="wav";
        filtros[6]=0;
	}


	//guardamos directorio actual
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//Obtenemos directorio de cinta
	//si no hay directorio, vamos a rutas predefinidas
	if (tapefile==NULL) menu_chdir_sharedfiles();

	else {
	        char directorio[PATH_MAX];
	        util_get_dir(tapefile,directorio);
	        //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

		//cambiamos a ese directorio, siempre que no sea nulo
		if (directorio[0]!=0) {
			debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
			zvfs_chdir(directorio);
		}
	}



        int ret;

        ret=menu_filesel("Select Input Tape",filtros,tape_open_file);
        //volvemos a directorio inicial
        zvfs_chdir(directorio_actual);


	if (ret==1) {
		tapefile=tape_open_file;
		tape_init();
	}


}




//Retorna 0=Cancel, 1=Append, 2=Truncate, 3=Rotate
int menu_ask_no_append_truncate_texto(char *texto_ventana,char *texto_interior)
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

		menu_add_item_menu_format(array_menu_ask_no_append_truncate,MENU_OPCION_NORMAL,NULL,NULL,"~~Rotate");
		menu_add_item_menu_shortcut(array_menu_ask_no_append_truncate,'r');
		menu_add_item_menu_tooltip(array_menu_ask_no_append_truncate,"Rotate selected file to keep history files");
		menu_add_item_menu_ayuda(array_menu_ask_no_append_truncate,"Rename selected file adding extension suffix .1. \n"
			"If that file also exists, the extension suffix is renamed to .2. \n"
			"If that file also exists, the extension suffix is renamed to .3, and so on... \n"
			"You can set the maximum file rotations, by default 10."

			);

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


int menu_tape_out_open_last_rotated=10;

void menu_tape_out_open(MENU_ITEM_PARAMETERS)
{

	char *filtros[5];
	char mensaje_existe[20];

	if (MACHINE_IS_ZX8081) {

		if (MACHINE_IS_ZX80_TYPE) {
            filtros[0]="o";
            filtros[1]=0;
        }
		else {
            filtros[0]="p";
            filtros[1]="p81";
            filtros[2]=0;
        }

		strcpy(mensaje_existe,"Overwrite?");
	}

	else {
        filtros[0]="tap";
		filtros[1]="tzx";
		filtros[2]="pzx";
		filtros[3]=0;
		strcpy(mensaje_existe,"Append?");
	}

	//guardamos directorio actual
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//Obtenemos directorio de cinta

	if (tape_out_file!=NULL) {
	        char directorio[PATH_MAX];
	        util_get_dir(tape_out_file,directorio);
	        //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

		//cambiamos a ese directorio, siempre que no sea nulo
		if (directorio[0]!=0) {
			debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
			zvfs_chdir(directorio);
		}
	}


        int ret;

        ret=menu_filesel_save("Select Output Tape",filtros,tape_out_open_file);
        //volvemos a directorio inicial
        zvfs_chdir(directorio_actual);


	if (ret==1) {

		//Ver si archivo existe y preguntar
		struct stat buf_stat;

		if (stat(tape_out_open_file, &buf_stat)==0) {

			if (MACHINE_IS_ZX8081) {
					if (menu_confirm_yesno_texto("File exists",mensaje_existe)==0) {
						tape_out_file=NULL;
						tap_out_close();
						return;
				}
			}

			else {
				int opcion=menu_ask_no_append_truncate_texto("File exists","What do you want?");
				//printf ("opcion: %d\n",opcion);

				//Cancel
				if (opcion==0) {
					tape_out_file=NULL;
					tap_out_close();
					return;
				}

				//Truncate
				if (opcion==2) {
					util_truncate_file(tape_out_open_file);
				}

				//Rotate
				if (opcion==3) {
					//Rotar
					char string_rotaciones[3];


					int valor_leido;
					sprintf (string_rotaciones,"%d",menu_tape_out_open_last_rotated);

					menu_ventana_scanf("Number of files",string_rotaciones,3);

					valor_leido=parse_string_to_number(string_rotaciones);

					if (valor_leido<1 || valor_leido>99) {
							debug_printf (VERBOSE_ERR,"Invalid value %d",valor_leido);
							tape_out_file=NULL;
							tap_out_close();
							return;
					}

					menu_tape_out_open_last_rotated=valor_leido;


					util_rotate_file(tape_out_open_file,menu_tape_out_open_last_rotated);
					//El actual ya se creará cuando se escriba la primera vez
				}

			}

		}

		tape_out_file=tape_out_open_file;
		tape_out_init();
	}


	else {
		tape_out_file=NULL;
		tap_out_close();
	}



}

void menu_tape_input_insert(MENU_ITEM_PARAMETERS)
{

	if (tapefile==NULL) return;

	if ((tape_loadsave_inserted & TAPE_LOAD_INSERTED)==0) {
		tap_open();
	}

	else {
		tap_close();
	}
}



void menu_tape_browser(MENU_ITEM_PARAMETERS)
{

    int repetir_browser;

    do {

        repetir_browser=0;

        int linea=menu_tape_browser_show(tapefile,tape_viewer_block_index);

        if (!is_tape_inserted()) return;

        //Si es .tap, podemos hacer browse
        if (!util_compare_file_extension(tapefile,"tap")) {
            if (linea>=0) {
                if (menu_confirm_yesno("Seek to block?")) {
                    tape_seek_to_block(linea);
                    repetir_browser=1;
                }
            }
        }

    } while (repetir_browser);

}

void menu_tape_browser_output(MENU_ITEM_PARAMETERS)
{
	menu_tape_browser_show(tape_out_file,-1);
}

void menu_tape_browser_real(MENU_ITEM_PARAMETERS)
{
	menu_tape_browser_show(realtape_name,-1);
}

int menu_tape_input_insert_cond(void)
{
	if (tapefile==NULL) return 0;
	else return 1;
}

int menu_tape_output_insert_cond(void)
{
        if (tape_out_file==NULL) return 0;
        else return 1;
}

void menu_tape_output_insert(MENU_ITEM_PARAMETERS)
{

        if (tape_out_file==NULL) return;

	if ((tape_loadsave_inserted & TAPE_SAVE_INSERTED)==0) {
                tap_out_open();
        }

        else {
                tap_out_close();
        }
}


void menu_realtape_message_reading(void)
{
    //TODO: lo ideal es que este mensaje se quedase todo el rato mientras se convierte la cinta a
    //Visual Real Tape, que es lo que tarda realmente
    //Pero para eso habria que hacer toda la conversión por debajo con un pthread
    //actualmente solo la parte de obtener los trozos de cada bloque se hace con un phtread
    menu_generic_message_splash("Insert Real Tape","Tape being read...");
}


void menu_realtape_open(MENU_ITEM_PARAMETERS)
{

        char *filtros[11];

        filtros[0]="smp";
        filtros[1]="rwa";
        filtros[2]="wav";
        filtros[3]="tzx";
        filtros[4]="p";
        filtros[5]="p81";
        filtros[6]="o";
        filtros[7]="tap";
        filtros[8]="cdt";
		filtros[9]="pzx";
        filtros[10]=0;


        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de cinta
        //si no hay directorio, vamos a rutas predefinidas
        if (realtape_name==NULL) menu_chdir_sharedfiles();

        else {
                char directorio[PATH_MAX];
                util_get_dir(realtape_name,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

     		//cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        zvfs_chdir(directorio);
                }
        }



        int ret;

        ret=menu_filesel("Select Input Tape",filtros,menu_realtape_name);
        //volvemos a directorio inicial
        zvfs_chdir(directorio_actual);


        if (ret==1) {
                realtape_name=menu_realtape_name;

                menu_realtape_message_reading();

        	realtape_insert();
	}


}

void menu_realtape_insert(MENU_ITEM_PARAMETERS)
{
	if (realtape_inserted.v==0) {
        menu_realtape_message_reading();

        realtape_insert();
    }
	else realtape_eject();
}


int menu_realtape_inserted_cond(void)
{
	if (menu_realtape_cond()==0) return 0;
	return realtape_inserted.v;
}


//menu storage tape
void menu_storage_tape(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_tape_settings;
	menu_item item_seleccionado;
	int retorno_menu;

    do {
        char string_tape_load_shown[20],string_tape_save_shown[20];
        //char string_tape_load_inserted[50],string_tape_save_inserted[50];
		char string_realtape_shown[23];

		menu_add_item_menu_inicial_format(&array_menu_tape_settings,MENU_OPCION_NORMAL,NULL,NULL,"--Standard Tape--");
        menu_add_item_menu_spanish_catalan(array_menu_tape_settings,"--Cinta Estandar--","--Cinta Estandard--");
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Select Standard tape for Input and Output");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Standard tapes are those handled by ROM routines and "
					"have normal speed (no turbo). These tapes are handled by ZEsarUX and loaded or saved "
					"very quickly (for example tap). Audio format files (for example rwa, wav or smp) "
					"are converted by ZEsarUX to bytes and loaded on the machine memory. For every other non standard "
					"tapes (turbo or handled by non-ROM routines like loading stripes on different colours) you must use "
					"Real Input tape for load, and Audio output to file for saving");


		menu_tape_settings_trunc_name(tapefile,string_tape_load_shown,20);
		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_open,NULL,"~~Input [%s]",string_tape_load_shown);
        menu_add_item_menu_prefijo(array_menu_tape_settings,"    ");
		menu_add_item_menu_shortcut(array_menu_tape_settings,'i');


		//sprintf (string_tape_load_inserted,"[%c] Input tape inserted",((tape_loadsave_inserted & TAPE_LOAD_INSERTED)!=0 ? 'X' : ' '));
		//menu_add_item_menu(array_menu_tape_settings,string_tape_load_inserted,MENU_OPCION_NORMAL,menu_tape_input_insert,menu_tape_input_insert_cond);

        menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_input_insert,menu_tape_input_insert_cond,
            "[%c] Input tape inserted",(is_tape_inserted() ? 'X' : ' '));

		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_browser,menu_tape_input_insert_cond,"Tape Vi~~ewer");
        menu_add_item_menu_prefijo(array_menu_tape_settings,"    ");
		menu_add_item_menu_shortcut(array_menu_tape_settings,'e');
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Browse Input tape");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Browse Input tape");
        menu_add_item_menu_add_flags(array_menu_tape_settings,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);


		menu_add_item_menu(array_menu_tape_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);


        menu_tape_settings_trunc_name(tape_out_file,string_tape_save_shown,20);
		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_out_open,NULL,"~~Output [%s]",string_tape_save_shown);
        menu_add_item_menu_prefijo(array_menu_tape_settings,"    ");
		menu_add_item_menu_shortcut(array_menu_tape_settings,'o');

        //sprintf (string_tape_save_inserted,"[%c] Output tape inserted",((tape_loadsave_inserted & TAPE_SAVE_INSERTED)!=0 ? 'X' : ' '));
        //menu_add_item_menu(array_menu_tape_settings,string_tape_save_inserted,MENU_OPCION_NORMAL,menu_tape_output_insert,menu_tape_output_insert_cond);

        menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_output_insert,menu_tape_output_insert_cond,
            "[%c] Output tape inserted",((tape_loadsave_inserted & TAPE_SAVE_INSERTED)!=0 ? 'X' : ' '));

		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_browser_output,menu_tape_output_insert_cond,"Tape Viewe~~r");
        menu_add_item_menu_prefijo(array_menu_tape_settings,"    ");
		menu_add_item_menu_shortcut(array_menu_tape_settings,'r');
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Browse Output tape");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Browse Output tape");
        menu_add_item_menu_add_flags(array_menu_tape_settings,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);


        menu_add_item_menu(array_menu_tape_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_item_menu_en_es_ca(array_menu_tape_settings,MENU_OPCION_NORMAL,NULL,NULL,
            "--Input Real Tape--","--Cinta Real de Entrada--","--Cinta Real d'Entrada--");
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Input Real Tape at normal loading Speed");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"You may select any input valid tape: o, p, tap, tzx, rwa, wav, smp. "
					"This tape is handled the same way as the real machine does, at normal loading speed, and may "
					"select tapes with different loading methods instead of the ROM: turbo loading, alkatraz, etc...\n"
					"When inserted real tape, realvideo is enabled, only to show real loading stripes on screen, but it is "
					"not necessary, you may disable realvideo if you want");




        //Ocultar opciones de Real Tape de archivo cuando se activa External Audio Source
        int ocultar_real_tape_archivo=0;
        if (audio_can_record_input() && audio_is_recording_input) ocultar_real_tape_archivo=1;

        if (!ocultar_real_tape_archivo) {

        menu_tape_settings_trunc_name(realtape_name,string_realtape_shown,23);
        menu_add_item_menu_en_es_ca(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_realtape_open,NULL,
            "~~File","~~Fichero","~~Fitxer");
        menu_add_item_menu_sufijo_format(array_menu_tape_settings," [%s]",string_realtape_shown);
        menu_add_item_menu_prefijo(array_menu_tape_settings,"    ");
		menu_add_item_menu_shortcut(array_menu_tape_settings,'f');
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Audio file to use as the input audio");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Audio file to use as the input audio");


		menu_add_item_menu_en_es_ca(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_realtape_insert,menu_realtape_cond,
            "Inserted","Insertado","Insertat");
        menu_add_item_menu_prefijo_format(array_menu_tape_settings,"[%c] ", (realtape_inserted.v==1 ? 'X' : ' '));
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Insert the audio file");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Insert the audio file");



		menu_add_item_menu_en_es_ca(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_realtape_pause_unpause,menu_realtape_inserted_cond,
            "~~Playing","Re~~produciendose","Re~~produint-se");
        menu_add_item_menu_prefijo_format(array_menu_tape_settings,"[%c] ", (realtape_playing.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_tape_settings,'p');
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Start playing the audio tape");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Start playing the audio tape");

		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_browser_real,menu_realtape_cond,"Tape Vie~~wer");
        menu_add_item_menu_prefijo(array_menu_tape_settings,"    ");
		menu_add_item_menu_shortcut(array_menu_tape_settings,'w');
        menu_add_item_menu_genera_ventana(array_menu_tape_settings);
        menu_add_item_menu_se_cerrara(array_menu_tape_settings);
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Browse Real tape");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Browse Real tape");

        menu_add_item_menu_en_es_ca(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_visual_realtape,NULL,
            "~~Visual Real Tape","Cinta Real ~~Visual","Cinta Real ~~Visual");
        menu_add_item_menu_prefijo(array_menu_tape_settings,"    ");
        menu_add_item_menu_shortcut(array_menu_tape_settings,'v');
        menu_add_item_menu_se_cerrara(array_menu_tape_settings);
        menu_add_item_menu_genera_ventana(array_menu_tape_settings);
        menu_add_item_menu_tooltip(array_menu_tape_settings,"See an audio render of your tape, see tape blocks and rewind or move forward the cassette player");
        menu_add_item_menu_ayuda(array_menu_tape_settings,"See an audio render of your tape, see tape blocks and rewind or move forward the cassette player");


        menu_add_item_menu_en_es_ca(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_visual_cassette_tape,NULL,
            "~~Visual Casette Tape","Cinta Casette ~~Visual","Cinta Casette ~~Visual");
        menu_add_item_menu_prefijo(array_menu_tape_settings,"    ");
        menu_add_item_menu_se_cerrara(array_menu_tape_settings);
        menu_add_item_menu_genera_ventana(array_menu_tape_settings);


        }

        if (MACHINE_IS_SPECTRUM) {
            menu_add_item_menu_separator(array_menu_tape_settings);

            menu_add_item_menu_en_es_ca(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_storage_tape_copier,NULL,
                "Run Tape Copier","Ejecutar copión","Executar copiador");
            menu_add_item_menu_prefijo(array_menu_tape_settings,"    ");
            //menu_add_item_menu_shortcut(array_menu_tape_settings,'v');
            menu_add_item_menu_genera_ventana(array_menu_tape_settings);
            menu_add_item_menu_tooltip(array_menu_tape_settings,"Allow to run a tape copier");
            menu_add_item_menu_ayuda(array_menu_tape_settings,"Allow to run a tape copier");


            menu_add_item_menu_en_es_ca(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_storage_tape_my_soft,NULL,
                "Software made by me","Programas míos","Programes meus");
            menu_add_item_menu_prefijo(array_menu_tape_settings,"    ");
            //menu_add_item_menu_shortcut(array_menu_tape_settings,'v');
            menu_add_item_menu_genera_ventana(array_menu_tape_settings);
            menu_add_item_menu_tooltip(array_menu_tape_settings,"Open folder with some of programs made by me");
            menu_add_item_menu_ayuda(array_menu_tape_settings,"Open folder with some of programs made by me");
        }


        menu_add_item_menu(array_menu_tape_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_ESC_item(array_menu_tape_settings);

        retorno_menu=menu_dibuja_menu_no_title_lang(&storage_tape_opcion_seleccionada,&item_seleccionado,array_menu_tape_settings,"Tape" );



		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
			//llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                    //printf ("actuamos por funcion\n");
                    item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


}








void menu_timexcart_load(MENU_ITEM_PARAMETERS)
{

        char *filtros[2];

        filtros[0]="dck";

        filtros[1]=0;



        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de ultimo archivo
        //si no hay directorio, vamos a rutas predefinidas
        if (last_timex_cart[0]==0) menu_chdir_sharedfiles();

        else {
                char directorio[PATH_MAX];
                util_get_dir(last_timex_cart,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        zvfs_chdir(directorio);
                }
        }


        int ret;

        ret=menu_filesel("Select Cartridge",filtros,last_timex_cart);
        //volvemos a directorio inicial
		zvfs_chdir(directorio_actual);


        if (ret==1) {
		//                sprintf (last_timex_cart,"%s",timexcart_load_file);

                //sin overlay de texto, que queremos ver las franjas de carga con el color normal (no apagado)
                //reset_menu_overlay_function();


                        timex_insert_dck_cartridge(last_timex_cart);

                //restauramos modo normal de texto de menu
                //set_menu_overlay_function(normal_overlay_texto_menu);

                //Y salimos de todos los menus
                salir_todos_menus=1;
        }


}


void menu_timexcart_eject(MENU_ITEM_PARAMETERS)
{
	timex_empty_dock_space();
	menu_generic_message("Eject Cartridge","OK. Cartridge ejected");
}


void menu_timexcart(MENU_ITEM_PARAMETERS)
{

        menu_item *array_menu_timexcart;
        menu_item item_seleccionado;
        int retorno_menu;

        do {


                menu_add_item_menu_inicial(&array_menu_timexcart,"~~Load Cartridge",MENU_OPCION_NORMAL,menu_timexcart_load,NULL);
                menu_add_item_menu_shortcut(array_menu_timexcart,'l');
                menu_add_item_menu_tooltip(array_menu_timexcart,"Load Timex Cartridge");
                menu_add_item_menu_ayuda(array_menu_timexcart,"Supported timex cartridge formats on load:\n"
                                        "DCK");

                menu_add_item_menu(array_menu_timexcart,"~~Eject Cartridge",MENU_OPCION_NORMAL,menu_timexcart_eject,NULL);
                menu_add_item_menu_shortcut(array_menu_timexcart,'e');
                menu_add_item_menu_tooltip(array_menu_timexcart,"Eject Cartridge");
                menu_add_item_menu_ayuda(array_menu_timexcart,"Eject Cartridge");


     				menu_add_item_menu(array_menu_timexcart,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_timexcart);

                retorno_menu=menu_dibuja_menu_no_title_lang(&timexcart_opcion_seleccionada,&item_seleccionado,array_menu_timexcart,"Timex Cartridge" );



                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



void menu_dandanator_rom_file(MENU_ITEM_PARAMETERS)
{
	dandanator_disable();

    char *filtros[2];

    filtros[0]="rom";
    filtros[1]=0;

    //guardamos directorio actual
    char directorio_actual[PATH_MAX];
    getcwd(directorio_actual,PATH_MAX);

	//Obtenemos ultimo directorio visitado
	if (dandanator_rom_file_name[0]!=0) {
		char directorio[PATH_MAX];
		util_get_dir(dandanator_rom_file_name,directorio);
		//printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

		//cambiamos a ese directorio, siempre que no sea nulo
		if (directorio[0]!=0) {
				debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
				zvfs_chdir(directorio);
		}
	}

    int ret;

    ret=menu_filesel("Select dandanator File",filtros,dandanator_rom_file_name);

    //volvemos a directorio inicial
    zvfs_chdir(directorio_actual);


    if (ret==1) {
        if (!si_existe_archivo(dandanator_rom_file_name)) {
            menu_error_message("File does not exist");
            dandanator_rom_file_name[0]=0;
            return;
        }

        else {
            //Comprobar aqui tambien el tamanyo
            long long int size=get_file_size(dandanator_rom_file_name);
            if (size!=DANDANATOR_SIZE) {
                menu_error_message("ROM file must be 512 KB length");
                dandanator_rom_file_name[0]=0;
                return;
            }
        }


    }
    //Sale con ESC
    else {
        //Quitar nombre
        dandanator_rom_file_name[0]=0;


    }

}

int menu_storage_dandanator_emulation_cond(void)
{
	if (dandanator_rom_file_name[0]==0) return 0;
        return 1;
}

int menu_storage_dandanator_press_button_cond(void)
{
	return dandanator_enabled.v;
}


void menu_storage_dandanator_emulation(MENU_ITEM_PARAMETERS)
{
	if (dandanator_enabled.v) dandanator_disable();
	else dandanator_enable();
}

void menu_storage_dandanator_press_button(MENU_ITEM_PARAMETERS)
{
	dandanator_press_button();
	//Y salimos de todos los menus
	salir_todos_menus=1;

}

void menu_dandanator(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_dandanator;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_dandanator_file_shown[13];


                        menu_tape_settings_trunc_name(dandanator_rom_file_name,string_dandanator_file_shown,13);
                        menu_add_item_menu_inicial_format(&array_menu_dandanator,MENU_OPCION_NORMAL,menu_dandanator_rom_file,NULL,"~~ROM File [%s]",string_dandanator_file_shown);
                        menu_add_item_menu_prefijo(array_menu_dandanator,"    ");
                        menu_add_item_menu_shortcut(array_menu_dandanator,'r');
                        menu_add_item_menu_tooltip(array_menu_dandanator,"ROM Emulation file");
                        menu_add_item_menu_ayuda(array_menu_dandanator,"ROM Emulation file");


                        menu_add_item_menu_format(array_menu_dandanator,MENU_OPCION_NORMAL,menu_storage_dandanator_emulation,menu_storage_dandanator_emulation_cond,"[%c] Dandanator ~~Enabled", (dandanator_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_dandanator,'e');
                        menu_add_item_menu_tooltip(array_menu_dandanator,"Enable dandanator");
                        menu_add_item_menu_ayuda(array_menu_dandanator,"Enable dandanator");


			menu_add_item_menu_format(array_menu_dandanator,MENU_OPCION_NORMAL,menu_storage_dandanator_press_button,menu_storage_dandanator_press_button_cond,"~~Press button");
            menu_add_item_menu_prefijo(array_menu_dandanator,"    ");
			menu_add_item_menu_shortcut(array_menu_dandanator,'p');
                        menu_add_item_menu_tooltip(array_menu_dandanator,"Press button");
                        menu_add_item_menu_ayuda(array_menu_dandanator,"Press button");


                                menu_add_item_menu(array_menu_dandanator,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_dandanator);

				char titulo_menu[32];
				if (MACHINE_IS_SPECTRUM) strcpy(titulo_menu,"ZX Dandanator");
				else strcpy(titulo_menu,"CPC Dandanator");

                retorno_menu=menu_dibuja_menu_no_title_lang(&dandanator_opcion_seleccionada,&item_seleccionado,array_menu_dandanator,titulo_menu);


                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



void menu_samram_rom_file(MENU_ITEM_PARAMETERS)
{
	samram_disable();

        char *filtros[2];

        filtros[0]="rom";
        filtros[1]=0;


        if (menu_filesel("Select ROM File",filtros,samram_rom_file_name)==1) {
                if (!si_existe_archivo(samram_rom_file_name)) {
                        menu_error_message("File does not exist");
                        samram_rom_file_name[0]=0;
                        return;



                }

                else {
                        //Comprobar aqui tambien el tamanyo
                        long long int size=get_file_size(samram_rom_file_name);
                        if (size!=32768)  {
                                menu_error_message("ROM file must be 32 KB length");
                                samram_rom_file_name[0]=0;
                                return;
                        }
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                samram_rom_file_name[0]=0;


        }

}

int menu_storage_samram_emulation_cond(void)
{
	if (samram_rom_file_name[0]==0) return 0;
        return 1;
}

/*
int menu_storage_samram_press_button_cond(void)
{
	return samram_enabled.v;
}
*/


void menu_storage_samram_emulation(MENU_ITEM_PARAMETERS)
{
	if (samram_enabled.v) samram_disable();
	else samram_enable();
}

/*
void menu_storage_samram_press_button(MENU_ITEM_PARAMETERS)
{
	samram_press_button();
	//Y salimos de todos los menus
	salir_todos_menus=1;

}
*/

void menu_samram(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_samram;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_samram_file_shown[13];


                        menu_tape_settings_trunc_name(samram_rom_file_name,string_samram_file_shown,13);
                        menu_add_item_menu_inicial_format(&array_menu_samram,MENU_OPCION_NORMAL,menu_samram_rom_file,NULL,"~~ROM File [%s]",string_samram_file_shown);
                        menu_add_item_menu_prefijo(array_menu_samram,"    ");
                        menu_add_item_menu_shortcut(array_menu_samram,'r');
                        menu_add_item_menu_tooltip(array_menu_samram,"ROM Emulation file");
                        menu_add_item_menu_ayuda(array_menu_samram,"ROM Emulation file");


                        			menu_add_item_menu_format(array_menu_samram,MENU_OPCION_NORMAL,menu_storage_samram_emulation,menu_storage_samram_emulation_cond,"[%c] ~~Samram Enabled", (samram_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_samram,'s');
                        menu_add_item_menu_tooltip(array_menu_samram,"Enable samram");
                        menu_add_item_menu_ayuda(array_menu_samram,"Enable samram");


            /*
			menu_add_item_menu_format(array_menu_samram,MENU_OPCION_NORMAL,menu_storage_samram_press_button,menu_storage_samram_press_button_cond,"~~Press button");
			menu_add_item_menu_shortcut(array_menu_samram,'p');
                        menu_add_item_menu_tooltip(array_menu_samram,"Press button");
                        menu_add_item_menu_ayuda(array_menu_samram,"Press button");
            */

                                menu_add_item_menu(array_menu_samram,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_samram);

                retorno_menu=menu_dibuja_menu_no_title_lang(&samram_opcion_seleccionada,&item_seleccionado,array_menu_samram,"Samram" );


                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


void menu_storage_hilow_encendido(MENU_ITEM_PARAMETERS)
{
    if (hilow_reproductor_encendido.v) hilow_raw_power_off_player();
    else hilow_raw_power_on_player();
}

void menu_storage_hilow_diffencial_algorithm(MENU_ITEM_PARAMETERS)
{
    hilow_diffencial_algorithm_enabled.v ^=1;
}

void menu_storage_hilow_diffencial_algorithm_volume_range(MENU_ITEM_PARAMETERS)
{
    hilow_diffencial_algorithm_volume_range +=2;

    if (hilow_diffencial_algorithm_volume_range==30) hilow_diffencial_algorithm_volume_range=0;

}

void menu_storage_hilow_invert_bit(MENU_ITEM_PARAMETERS)
{
    hilow_invert_bit.v ^=1;
}




int menu_storage_hilow_press_button_cond(void)
{
	return hilow_enabled.v;
}


void menu_storage_hilow_emulation(MENU_ITEM_PARAMETERS)
{
	if (hilow_enabled.v) hilow_disable();
	else hilow_enable();
}

void menu_storage_hilow_insert(MENU_ITEM_PARAMETERS)
{
    if (hilow_cinta_insertada_flag.v) {
        hilow_action_open_tape();
    }
    else {
        hilow_action_close_tape();
    }
}

void menu_storage_hilow_cover(MENU_ITEM_PARAMETERS)
{
    hilow_tapa_has_been_opened.v ^=1;
}

void menu_storage_hilow_hear_load(MENU_ITEM_PARAMETERS)
{
    hilow_hear_load_sound.v ^=1;
}

void menu_storage_hilow_hear_save(MENU_ITEM_PARAMETERS)
{
    hilow_hear_save_sound.v ^=1;
}


void menu_storage_hilow_file(MENU_ITEM_PARAMETERS)
{

    //Para que haga flush al cambiar de cinta
    if (hilow_rom_traps.v==0) {
	    hilow_disable();
    }

    char *filtros[3];

    filtros[0]="ddh"; //Data Drive HiLow ddh
    filtros[1]="raw"; //Data Drive HiLow raw
    filtros[2]=0;


    //guardamos directorio actual
    char directorio_actual[PATH_MAX];
    getcwd(directorio_actual,PATH_MAX);

    //Obtenemos directorio de trd
    //si no hay directorio, vamos a rutas predefinidas
    if (hilow_file_name[0]==0) menu_chdir_sharedfiles();

    else {
        char directorio[PATH_MAX];
        util_get_dir(hilow_file_name,directorio);
        //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

        //cambiamos a ese directorio, siempre que no sea nulo
        if (directorio[0]!=0) {
            debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
            zvfs_chdir(directorio);
        }
    }



    int ret=menu_filesel("Select Data Drive File",filtros,hilow_file_name);
    //volvemos a directorio inicial
    zvfs_chdir(directorio_actual);


    if (ret==1) {
		if (!si_existe_archivo(hilow_file_name)) {
			if (menu_confirm_yesno_texto("File does not exist","Create?")==0) {
                hilow_file_name[0]=0;
                return;
            }

            if (!util_compare_file_extension(hilow_file_name,"ddh")) {

                int total_sectors=HILOW_MAX_SECTORS;

	            if (menu_ventana_scanf_numero_enhanced("Total sectors",&total_sectors,4,+1,3,HILOW_MAX_SECTORS,0)>=0) {


                    //Crear archivo vacio
                    FILE *ptr_hilowfile;
                    ptr_hilowfile=fopen(hilow_file_name,"wb");

                    long long int totalsize=total_sectors*HILOW_SECTOR_SIZE; //HILOW_DEVICE_SIZE;
                    z80_byte valor_grabar=0;

                    if (ptr_hilowfile!=NULL) {
                        while (totalsize) {
                            fwrite(&valor_grabar,1,1,ptr_hilowfile);
                            totalsize--;
                        }
                        fclose(ptr_hilowfile);
                    }

                }

                else {
                    //Quitar nombre
                    hilow_file_name[0]=0;
                    return;
                }

            }

            else {
                int opcion=menu_simple_four_choices("Capacity","Select","C15","C30","C60","C90");

                int capacidad;


                if (opcion<1) return;
                switch (opcion) {
                    case 1:
                        capacidad=HILOW_RAW_SAMPLE_FREQ*(60*15); //cinta C15 (de 7.5 minutos por cara)
                    break;

                    case 2:
                        capacidad=HILOW_RAW_SAMPLE_FREQ*(60*30);
                    break;

                    case 3:
                        capacidad=HILOW_RAW_SAMPLE_FREQ*(60*60);
                    break;

                    case 4:
                        capacidad=HILOW_RAW_SAMPLE_FREQ*(60*90);
                    break;
                }



                //Crear archivo vacio
                //para que sea mas rapido, asignamos memoria, generamos vacio y luego grabamos todo de golpe
                z80_byte *temp_mem=util_malloc(capacidad,"Can not allocate memory for image creation");

                memset(temp_mem,0,capacidad);

                FILE *ptr_hilowfile;
                ptr_hilowfile=fopen(hilow_file_name,"wb");

                if (ptr_hilowfile!=NULL) {

                    fwrite(temp_mem,1,capacidad,ptr_hilowfile);

                    fclose(ptr_hilowfile);
                }

                free(temp_mem);
            }

		}

        //si ya estaba habilitado, cargar la imagen
        if (hilow_enabled.v) {
            hilow_load_device_file();
        }


    }
    //Sale con ESC
    else {
        //Quitar nombre
        hilow_file_name[0]=0;
    }
}



void menu_storage_hilow_persistent_writes(MENU_ITEM_PARAMETERS)
{
	hilow_persistent_writes.v ^=1;
}


int menu_storage_hilow_emulation_cond(void)
{

    if (hilow_file_name[0]==0) return 0;
    else return 1;

}


int menu_storage_hilow_enabled_cond(void)
{
    return hilow_enabled.v;
}

void menu_storage_hilow_write_protect(MENU_ITEM_PARAMETERS)
{
	hilow_write_protection.v ^=1;
}


void menu_storage_hilow_format(MENU_ITEM_PARAMETERS)
{
    if (menu_confirm_yesno_texto("Format drive","Sure?")) {

        menu_first_aid("hilow_format");

        char buffer_nombre[10];

        buffer_nombre[0]=0;
        menu_ventana_scanf("Drive label",buffer_nombre,10);

        int opcion=menu_simple_two_choices("Full clear","Select:","Clear all data","Only format");

        int clearing=(opcion==1 ? 1 : 0);

        int lados=menu_simple_two_choices("Two or One sides","Select:","Side A","Side A & B");

        if (lados==1 || lados==2) {



            if (menu_confirm_yesno_texto("Format drive","Really Sure?")) {


                hilow_device_mem_format(0,1,buffer_nombre,lados,clearing);

                //Para indicar que hay que releer el sector 0
                hilow_tapa_action_was_opened();

                menu_generic_message_splash("Format","Ok. Device has been formatted");

            }

        }

    }
}

void menu_storage_hilow_browser(MENU_ITEM_PARAMETERS)
{
    menu_hilow_datadrive_browser(hilow_device_buffer,hilow_ddh_file_size/HILOW_SECTOR_SIZE);
}

//Sectores usados por los archivos. En cada posicion N, indica que el sector N está usado si no es 0 el valor
int menu_storage_hilow_chkdsk_sectors_used[256];

//Sectores libres no usados por los archivos. El contenido son los indices de la tabla tal cual
int menu_storage_hilow_chkdsk_sectors_free[256];

void menu_storage_hilow_chkdsk(MENU_ITEM_PARAMETERS)
{

    //z80_byte *p_sector_zero;
    //z80_byte *p_sector_one;

    //p_sector_zero=hilow_device_buffer;
    //p_sector_one=&hilow_device_buffer[2048];

	char *texto_chkdsk=util_malloc_max_texto_browser();
	int indice_buffer=0;

	char buffer_texto[1024];
    int longitud_texto;
    char *txt_ok="OK ";
    char *txt_err="ERR";

    char buffer_ok_error[10];

    z80_int usage_counter_zero=hilow_util_get_usage_counter(1,hilow_device_buffer);
    z80_int usage_counter_one=hilow_util_get_usage_counter(2,hilow_device_buffer);

    if (usage_counter_zero==usage_counter_one) {
        strcpy(buffer_ok_error,txt_ok);
    }
    else if (usage_counter_zero>usage_counter_one) {
        if (usage_counter_zero==usage_counter_one+1) strcpy(buffer_ok_error,txt_ok);
        else strcpy(buffer_ok_error,txt_err);
    }
    else {
        if (usage_counter_one==usage_counter_zero+1) strcpy(buffer_ok_error,txt_ok);
        else strcpy(buffer_ok_error,txt_err);
    }


    sprintf (buffer_texto,"%s Usage counters: %d/%d",buffer_ok_error,usage_counter_zero,usage_counter_one);
    longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
    sprintf (&texto_chkdsk[indice_buffer],"%s\n",buffer_texto);
    indice_buffer +=longitud_texto;



    //Free sectors
    //No puede ser mayor que el valor de HILOW_MAX_SECTORS-2
    int sector;
    for (sector=1;sector<=2;sector++) {
        //Inicializar tabla de sectores usados
        int i;
        for (i=0;i<256;i++) {
            menu_storage_hilow_chkdsk_sectors_used[i]=0;
        }


        sprintf (buffer_texto,"\nDirectory sector %d",sector);
        longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
        sprintf (&texto_chkdsk[indice_buffer],"%s\n",buffer_texto);
        indice_buffer +=longitud_texto;


        z80_byte free_sectors=hilow_util_get_free_sectors(sector,hilow_device_buffer);
        if (free_sectors>HILOW_MAX_SECTORS-2) strcpy(buffer_ok_error,txt_err);
        else strcpy(buffer_ok_error,txt_ok);

        sprintf (buffer_texto,"%s Free sectors: %d",buffer_ok_error,free_sectors);
        longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
        sprintf (&texto_chkdsk[indice_buffer],"%s\n",buffer_texto);
        indice_buffer +=longitud_texto;

        //Obtener sectores libres
        hilow_util_get_free_sectors_list(sector,hilow_device_buffer,menu_storage_hilow_chkdsk_sectors_free);
        //Si en esa lista esta el 0 o el 1, error
        if (menu_storage_hilow_chkdsk_sectors_free[0] || menu_storage_hilow_chkdsk_sectors_free[1] || menu_storage_hilow_chkdsk_sectors_free[2]) {
            sprintf (buffer_texto,"%s Sector 0 or 1 or 2 can not be in free sectors table",txt_err);
            longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
            sprintf (&texto_chkdsk[indice_buffer],"%s\n",buffer_texto);
            indice_buffer +=longitud_texto;
        }

        //Total files. no puede ser mayor que HILOW_MAX_FILES_DIRECTORY

        z80_byte total_files=hilow_util_get_total_files(sector,hilow_device_buffer);
        if (total_files>HILOW_MAX_FILES_DIRECTORY) strcpy(buffer_ok_error,txt_err);
        else strcpy(buffer_ok_error,txt_ok);

        sprintf (buffer_texto,"%s Total files: %d",buffer_ok_error,total_files);
        longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea


        //Controlar en parte que no se exceda el maximo
        //TODO: esto es un tanto chapuza y habria que mejorarlo
        //llamando a funciones de agregar texto que compruebe limite siempre (que la hay)
        //printf("%d\n",indice_buffer+longitud_texto);
        if (indice_buffer+longitud_texto>MAX_TEXTO_BROWSER-2000) {
            debug_printf (VERBOSE_ERR,"Too many entries. Showing only the allowed in memory");
            //salir

            texto_chkdsk[indice_buffer]=0;

            //printf("browser: %s\n",texto_chkdsk);

            zxvision_generic_message_tooltip("Hilow Data Drive chkdsk" , 1, 0 , 0, 0, 1, NULL, 1, "%s", texto_chkdsk);

            free(texto_chkdsk);
            return;
        }


        sprintf (&texto_chkdsk[indice_buffer],"%s\n",buffer_texto);
        indice_buffer +=longitud_texto;

        //Sectores asignados de cada archivo. Que no se repitan en un archivo, que no se repitan en diferentes archivos,
        //que no haya mas de HILOW_MAX_SECTORS_PER_FILE asignados para un archivo
        //y que los ids no sean mayores o igual que HILOW_MAX_SECTORS
        int f;
        for (f=0;f<total_files;f++) {
            int sectors_file=hilow_get_num_sectors_file(sector,hilow_device_buffer,f);
            if (sectors_file>HILOW_MAX_SECTORS_PER_FILE)  {
                sprintf (buffer_texto,"%s File id %d has %d sectors",txt_err,f,sectors_file);
                longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea

                sprintf (&texto_chkdsk[indice_buffer],"%s\n",buffer_texto);
                indice_buffer +=longitud_texto;
            }

            int sectores[HILOW_MAX_SECTORS_PER_FILE];
            //Este s esta normalizado al maximo de HILOW_MAX_SECTORS_PER_FILE
            int s=hilow_util_get_sectors_file(sector,f,hilow_device_buffer,sectores);

            //Ver si se repite con alguno
            int j;
            for (j=0;j<s;j++) {
                int sector_usado=sectores[j];

                if (sector_usado>0xfd || sector_usado<3) {
                    sprintf (buffer_texto,"%s File id %d uses invalid sector %d",txt_err,f,sector_usado);
                    longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
                    sprintf (&texto_chkdsk[indice_buffer],"%s\n",buffer_texto);
                    indice_buffer +=longitud_texto;
                }

                //Si archivo usa un sector que sale en la tabla de libres
                if (menu_storage_hilow_chkdsk_sectors_free[sector_usado]) {
                    sprintf (buffer_texto,"%s File id %d uses free sector %d",txt_err,f,sector_usado);
                    longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
                    sprintf (&texto_chkdsk[indice_buffer],"%s\n",buffer_texto);
                    indice_buffer +=longitud_texto;
                }

                if (menu_storage_hilow_chkdsk_sectors_used[sector_usado]) {
                    //Ya estaba usado!
                    sprintf (buffer_texto,"%s File id %d uses repeated sector %d",txt_err,f,sector_usado);
                    longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
                    sprintf (&texto_chkdsk[indice_buffer],"%s\n",buffer_texto);
                    indice_buffer +=longitud_texto;
                }
                else {
                    menu_storage_hilow_chkdsk_sectors_used[sector_usado]=1;
                }
            }
        }
    }



	texto_chkdsk[indice_buffer]=0;

    //printf("browser: %s\n",texto_chkdsk);

	zxvision_generic_message_tooltip("Hilow Data Drive chkdsk" , 1, 0 , 0, 0, 1, NULL, 1, "%s", texto_chkdsk);


    free(texto_chkdsk);
}



//
// Inicio ventana de convertir audio HiLow a archivo DDH
//




//Para hacer las pausas entre cada sample de audio
struct timeval menu_hilow_convert_audio_timer_antes, menu_hilow_convert_audio_timer_ahora;


void menu_hilow_convert_audio_tiempo_inicial(void)
{

    gettimeofday(&menu_hilow_convert_audio_timer_antes, NULL);

}

//Calcular tiempo pasado en microsegundos
long menu_hilow_convert_audio_tiempo_final_usec(void)
{

    long menu_hilow_convert_audio_timer_time, menu_hilow_convert_audio_timer_seconds, menu_hilow_convert_audio_timer_useconds;

    gettimeofday(&menu_hilow_convert_audio_timer_ahora, NULL);

    menu_hilow_convert_audio_timer_seconds  = menu_hilow_convert_audio_timer_ahora.tv_sec  - menu_hilow_convert_audio_timer_antes.tv_sec;
    menu_hilow_convert_audio_timer_useconds = menu_hilow_convert_audio_timer_ahora.tv_usec - menu_hilow_convert_audio_timer_antes.tv_usec;

    menu_hilow_convert_audio_timer_time = ((menu_hilow_convert_audio_timer_seconds) * 1000000 + menu_hilow_convert_audio_timer_useconds);

    //printf("Elapsed time: %ld milliseconds\n\r", menu_hilow_convert_audio_timer_mtime);

        return menu_hilow_convert_audio_timer_time;
}

void menu_hilow_convert_audio_precise_usleep(int duracion)
{

    int tiempo_pasado_usec=menu_hilow_convert_audio_tiempo_final_usec();

    while (tiempo_pasado_usec<duracion) {
        //Dormir 1 microsegundo para no saturar la cpu
	//No. esto no es muy preciso en algunos Linux. Casi prefiero saturar una cpu y que se escuche a tiempo real
        //usleep(1);
        tiempo_pasado_usec=menu_hilow_convert_audio_tiempo_final_usec();
    }
}

//Para poder pasar de 44100 a 15600. Hacemos la media de los anteriores 3 valores, aunque esto no sea exacto: 44100/3 no es 15600 pero es aproximado
//Tener la media de los anteriores 3
int menu_hilow_convert_audio_last_audio_sample_one=0;
int menu_hilow_convert_audio_last_audio_sample_two=0;
int menu_hilow_convert_audio_last_audio_sample_three=0;


//Valor de audio ultimo leido que se enviara al output
char menu_hilow_convert_audio_last_audio_sample;

//Si estamos en modo lento o muy lento
int menu_hilow_convert_lento=0;

//Velocidad cuando no esta en fast mode ni en slow (1x, 2x, 4x...)
int menu_hilow_convert_speed=1;

//Si estamos pausados
int menu_hilow_convert_paused=0;

//Scroll de texto
void menu_hilow_convert_audio_scroll_left_string(char *texto)
{
    int longitud=strlen(texto);

    int i;

    //Si longitud 3: ABC
    //bucle desde 0,1
    //Final: BCC

    for (i=0;i<longitud-1;i++) {
        texto[i]=texto[i+1];
    }
}



                                        //   12345678
char menu_hilow_convert_audio_string_bits[]="        ";

                                                //   11 22 33 44 55 66 77 88
char menu_hilow_convert_audio_string_bytes[]=       "                        ";
                                                //   H  O  L  A  Q  U  E  T
char menu_hilow_convert_audio_string_bytes_ascii[]= "                        ";

//Ultimo bit leido
int menu_hilow_convert_audio_last_bit=0;

//Para el texto de "new bit"
int menu_hilow_convert_audio_just_read_bit=0;

//Para el texto de "new byte"
int menu_hilow_convert_audio_just_read_byte=0;

int menu_hilow_convert_mostrar_probable_error=0;

//Callback llamado desde la rutina de conversion al leer un bit
void menu_hilow_convert_audio_write_bit_callback(int valor,int posicion GCC_UNUSED)
{
    //printf("bit: %d\n",valor);

    menu_hilow_convert_audio_last_bit=valor;

    menu_hilow_convert_audio_just_read_bit=1;

    menu_hilow_convert_audio_scroll_left_string(menu_hilow_convert_audio_string_bits);

    int longitud=strlen(menu_hilow_convert_audio_string_bits);

    //Meter el bit a la derecha del string
    menu_hilow_convert_audio_string_bits[longitud-1]='0'+valor;


}

//Callback llamado cuando hay un posible error de sync
void menu_hilow_convert_audio_probably_sync_error(int valor,int posicion)
{
    if (hilow_read_audio_autocorrect) {
        debug_printf(VERBOSE_WARN,"Lenght of S_START_BYTE signal is smaller than expected: lenght: %d in position %d. Autocorrecting",valor,posicion);
    }
    else {
        debug_printf(VERBOSE_WARN,"Lenght of S_START_BYTE signal is smaller than expected: lenght: %d in position %d. You should enable autocorrect",valor,posicion);
    }

    menu_hilow_convert_mostrar_probable_error=1;

}

int menu_hilow_convert_audio_sector=0;


//Callback llamado desde la rutina de conversion al leer un byte
void menu_hilow_convert_audio_write_byte_callback(int valor,int posicion GCC_UNUSED)
{
    //printf("byte: %02XH\n",valor);

    menu_hilow_convert_audio_just_read_byte=1;

    menu_hilow_convert_audio_scroll_left_string(menu_hilow_convert_audio_string_bytes);
    menu_hilow_convert_audio_scroll_left_string(menu_hilow_convert_audio_string_bytes);
    menu_hilow_convert_audio_scroll_left_string(menu_hilow_convert_audio_string_bytes);

    int longitud=strlen(menu_hilow_convert_audio_string_bytes);

    //Meter el bit a la derecha del string
    sprintf(&menu_hilow_convert_audio_string_bytes[longitud-3],"%02X ",valor);

    //Lo mismo para ascii
    menu_hilow_convert_audio_scroll_left_string(menu_hilow_convert_audio_string_bytes_ascii);
    menu_hilow_convert_audio_scroll_left_string(menu_hilow_convert_audio_string_bytes_ascii);
    menu_hilow_convert_audio_scroll_left_string(menu_hilow_convert_audio_string_bytes_ascii);

    longitud=strlen(menu_hilow_convert_audio_string_bytes_ascii);

    //Meter el bit a la derecha del string
    char caracter=(valor>=32 && valor<126 ? valor : '.');
    menu_hilow_convert_audio_string_bytes_ascii[longitud-3]=caracter;

    //Aunque esto se obtiene al final del sector, irlo leyendo ya a ver si hay algo logico dentro
    menu_hilow_convert_audio_sector=hilow_read_audio_buffer_result[0];

}

//Si se ha abierto esta ventana, y entonces la zona de memoria esta disponible
//TODO: esto no se pone a 0 nunca, aunque se salga de aqui la zona de memoria seguira disponible,
//aunque esto no supone un gran problema
int menu_hilow_convert_audio_has_been_opened=0;

//Archivos de entrada y salida para conversión
char menu_hilow_convert_audio_input_raw[PATH_MAX]="";
char menu_hilow_convert_audio_output_ddh[PATH_MAX]="";


//Si el thread esta ejecutandose
int hilow_convert_audio_thread_running=0;

//Posicion en la lectura del archivo de audio
int menu_hilow_convert_audio_posicion_read_raw=0;

//modo sin pausas
int menu_hilow_convert_audio_fast_mode=0;

//Si completamente automatico
int menu_hilow_convert_audio_completamente_automatico=0;

//Si hay que esperar al usuario en el final de sector y decida que quiere hacer
int menu_hilow_convert_audio_esperar_siguiente_sector=0;

//Si quiere repetir el sector
int menu_hilow_convert_audio_must_repeat_sector=0;

//Si se escucha sonido y por tanto tambien se ve por visualmem (modo scroll no necesita que se escuche necesariamente)
int menu_hilow_convert_audio_hear_sound=1;

//Si mostramos microsegundos en vez de frames
int menu_hilow_convert_unidades_microseconds=0;

//para cuando hay sector mismatch, sugerir que el siguiente es este +1
int menu_hilow_convert_audio_anterior_sector_leido=0;

//Para tener un buffer intermedio donde guardar el sonido, luego desde core_spectrum lo usara de aqui
int menu_hilow_convert_audio_buffer_index=0;
//buffer mono, circular
char menu_hilow_convert_audio_buffer[AUDIO_BUFFER_SIZE];


//Decirle que el pthread hay que cancelarlo.
//int menu_hilow_convert_audio_must_stop_thread=0;

//pasar de mi buffer intermedio al buffer final de sonido
void menu_hilow_convert_get_audio_buffer(void)
{
    int i;

    int origen=menu_hilow_convert_audio_buffer_index;
    int destino=0;

    for (i=0;i<AUDIO_BUFFER_SIZE;i++) {
        char audio_leido=menu_hilow_convert_audio_buffer[origen++];
        if (origen==AUDIO_BUFFER_SIZE) origen=0;

        if (!menu_hilow_convert_audio_hear_sound) audio_leido=0;

        //Si estamos esperando input del usuario, tambien silencio
        if (menu_hilow_convert_audio_esperar_siguiente_sector) audio_leido=0;

        //Si en pausa, tambien silencio
        if (menu_hilow_convert_paused) audio_leido=0;

        //Esto tanto sirve para waveform (en modos no scroll) para que se vea toda la ventana con mismo ultimo valor
        if (menu_hilow_convert_lento) audio_leido=menu_hilow_convert_audio_last_audio_sample;

        audio_buffer[destino++]=audio_leido;
        audio_buffer[destino++]=audio_leido;
    }
}

//Usado en conversion de 44100 a 15600 hz
int menu_hilow_convert_counter_resample;

//Usado para alterar el audio segun si velocidad 1x, 2x, 4x o 8x
char menu_hilow_convert_samples_audio_speeds[8];
//Usado en este buffer anterior
int menu_hilow_convert_samples_audio_speeds_index;



//Aqui se entra cada vez que se lee un sample de audio
void menu_hilow_convert_audio_callback(int valor,int posicion)
{
    menu_hilow_convert_audio_posicion_read_raw=posicion;


    //printf("Valor desde callback: %d\n",valor);
    //Aprox para 44100 hz
    //22 seria para 44100 hz
    //Dado que vamos a 15600, son 3 veces menos



    //Si no estamos en modo rapido
    if (!menu_hilow_convert_audio_fast_mode) {


        //Muy lento
        if (menu_hilow_convert_lento) {
            menu_hilow_convert_audio_precise_usleep(20000*menu_hilow_convert_lento);
        }

        //Normal a 1x, 2x, 4x o 8x
        else  {
            //no deberia ser cero nunca, pero por si acaso evitamos divisiones entre cero
            if (menu_hilow_convert_speed!=0) {
                menu_hilow_convert_audio_precise_usleep(22/menu_hilow_convert_speed);
            }
        }

        menu_hilow_convert_audio_tiempo_inicial();
    }

    //Este sleep(0) hace algo de retardo, por eso solo lo llamo cada 1024 veces (1kb leido). Solo es para que si se llama a cancelar el pthread,
    //con pthread_cancel, desde el sleep se lee el estado y se cancela el thread si se ha llamado a pthread_cancel
    /*
   Cancellation Points
     Cancellation points will occur when a thread is executing the following functions: accept(), aio_suspend(), close(), connect(),
     creat(), fcntl(), fsync(), lockf(), msgrcv(), msgsnd(), msync(), nanosleep(), open(), pause(), poll(), pread(), pselect(),
     pthread_cond_timedwait(), pthread_cond_wait(), pthread_join(), pthread_testcancel(), pwrite(), read(), readv(), recv(),
     recvfrom(), recvmsg(), select(), sem_wait(), send(), sendmsg(), sendto(), sigpause(), sigsuspend(), sigwait(), sleep(), system(),
     tcdrain(), usleep(), wait(), waitpid(), write(), writev().
    */


#ifdef USE_PTHREADS
    //Si se quiere cancelar  el thread

    //Esta funcion, si el thread no se tiene que cancelar, no hace nada
    //Y si se tiene que cancelar, la cancela
    pthread_testcancel();
#endif


    menu_hilow_convert_audio_last_audio_sample_three=valor;

    //Sacamos la media de los 3, para convertir de 44100 a 15600
    int valor_final=menu_hilow_convert_audio_last_audio_sample_one+menu_hilow_convert_audio_last_audio_sample_two+menu_hilow_convert_audio_last_audio_sample_three;

    valor_final /=3;

    //Pasar a signed char
    char char_valor_final;


    //int valor_hilow=menu_hilow_convert_audio_last_audio_sample;
    valor_final -=128;

    char_valor_final=valor_final;

    menu_hilow_convert_audio_last_audio_sample=char_valor_final;


    //Hacer cada 3, pues "convertimos" de 44100hz a 15600
    if ((menu_hilow_convert_counter_resample%3)==0) {


        //Acelerar sonido
        if (menu_hilow_convert_speed>1) {
            //char menu_hilow_convert_samples_audio_speeds[8];
            //int menu_hilow_convert_samples_audio_speeds_index;
            int indice=menu_hilow_convert_samples_audio_speeds_index % menu_hilow_convert_speed;

            menu_hilow_convert_samples_audio_speeds[indice]=char_valor_final;

            menu_hilow_convert_samples_audio_speeds_index++;

            //Ultimo sample de esa serie de speed. calcular
            if (indice==menu_hilow_convert_speed-1) {
                //Sacar la media
                int j;
                int result=0;

                for (j=0;j<menu_hilow_convert_speed;j++) {
                    result +=menu_hilow_convert_samples_audio_speeds[j];
                }

                result /=menu_hilow_convert_speed;

                menu_hilow_convert_audio_buffer[menu_hilow_convert_audio_buffer_index++]=result;
            }
        }

        else {
            //Lo mas rapido posible
            if (menu_hilow_convert_audio_fast_mode) {

                ay_randomize(0);

                //randomize_noise es valor de 16 bits. sacar uno de 8 bits
                char randomize_valor=value_16_to_8h(randomize_noise[0]);

                char valor_sonido_final=(char_valor_final+randomize_valor)/2;

                //Y reducimos un poco el volumen
                valor_sonido_final /=2;

                //Completamente simulado, sonido random mezclado con sonido real
                menu_hilow_convert_audio_buffer[menu_hilow_convert_audio_buffer_index++]=valor_sonido_final;

            }

            //Velocidad 1x
            else menu_hilow_convert_audio_buffer[menu_hilow_convert_audio_buffer_index++]=char_valor_final;
        }


        if (menu_hilow_convert_audio_buffer_index==AUDIO_BUFFER_SIZE) menu_hilow_convert_audio_buffer_index=0;
    }

    menu_hilow_convert_counter_resample++;



    //Y "rotarlos"
    menu_hilow_convert_audio_last_audio_sample_one=menu_hilow_convert_audio_last_audio_sample_two;
    menu_hilow_convert_audio_last_audio_sample_two=menu_hilow_convert_audio_last_audio_sample_three;




    do {
        //Y para que no se vaya a silencio, decir que hay sonido y resetear contador de silencio
        silence_detection_counter=0;
        beeper_silence_detection_counter=0;

        if (menu_hilow_convert_paused) usleep(20000);

    } while (menu_hilow_convert_paused);  //Si estamos en modo pausado

}


//Leer archivo sonido de entrada
z80_byte *menu_hilow_convert_audio_read_hilow_audio_file(char *archivo)
{
    z80_byte *puntero;

    char archivo_raw[PATH_MAX];

    //Convertir a raw si conviene
    if (!util_compare_file_extension(archivo,"wav")) {

        debug_printf (VERBOSE_INFO,"Detected WAV file");

        if (convert_wav_to_raw_tmpdir(archivo,archivo_raw)) {
            //debug_printf(VERBOSE_ERR,"Error converting input file");
            return NULL;
        }

        if (!si_existe_archivo(archivo_raw)) {
            debug_printf(VERBOSE_ERR,"Error converting input file. Target file not found");
            return NULL;
        }
        archivo=archivo_raw;
    }

    hilow_read_audio_tamanyo_archivo_audio=get_file_size(archivo);

    //Asignar memoria
    //int tamanyo=hilow_read_audio_get_file_size(archivo);
    puntero=malloc(hilow_read_audio_tamanyo_archivo_audio);

    if (puntero==NULL) {
        cpu_panic("Can not allocate memory for hilow audio file");
    }


    //cargarlo en memoria
    FILE *ptr_rawfile;
    ptr_rawfile=fopen(archivo,"rb");

    if (!ptr_rawfile) {
            debug_printf(VERBOSE_ERR,"Unable to open audio file %s",archivo);
            return NULL;
    }

    fread(puntero,1,hilow_read_audio_tamanyo_archivo_audio,ptr_rawfile);
    fclose(ptr_rawfile);

    //Si leemos cara 2, invertir todo el sonido (el principio al final)
    if (hilow_read_audio_leer_cara_dos) {
        hilow_read_audio_espejar_sonido(puntero,hilow_read_audio_tamanyo_archivo_audio);
    }

    return puntero;
}

//Leer archivo ddh de salida
//Si tiene contenido previo, se carga en memoria, y luego lo que se convierta, sobreescribira los sectores existentes
//Util esto para poder leer primero un archivo de audio de cara A, luego el de cara B, y ambos van a parar a mismo ddh
int menu_hilow_convert_audio_read_hilow_ddh_file(char *archivo)
{
    //z80_byte *puntero;


    //Leer archivo ddh
    //Asignar memoria
    int tamanyo=HILOW_DEVICE_SIZE;
    hilow_read_audio_hilow_ddh=malloc(tamanyo);

    if (hilow_read_audio_hilow_ddh==NULL) {
        cpu_panic("Can not allocate memory for hilow ddh file");
    }

    //Resetear memoria a 0
    memset(hilow_read_audio_hilow_ddh,0,tamanyo);


    //cargarlo en memoria, si es que existe
    FILE *ptr_ddhfile;
    ptr_ddhfile=fopen(archivo,"rb");

    if (!ptr_ddhfile) {
        //Esto es normal, si archivo de output no existe
        debug_printf(VERBOSE_INFO,"Unable to open ddh file for read %s",archivo);
        return 1;
    }

    fread(hilow_read_audio_hilow_ddh,1,tamanyo,ptr_ddhfile);
    fclose(ptr_ddhfile);


    return 1;

}


//Escribir de la memoria a archivo ddh
void menu_hilow_convert_audio_write_hilow_ddh_file(char *archivo)
{
    //z80_byte *puntero;

    int tamanyo=HILOW_DEVICE_SIZE;


    FILE *ptr_ddhfile;
    ptr_ddhfile=fopen(archivo,"wb");

    if (!ptr_ddhfile) {
            printf("Unable to open ddh file %s\n",archivo);
            return;
    }

    fwrite(hilow_read_audio_hilow_ddh,1,tamanyo,ptr_ddhfile);
    fclose(ptr_ddhfile);

}


void menu_hilow_convert_help(void)
{
    /*
    if (gui_language==GUI_LANGUAGE_SPANISH) {
        menu_generic_message("Ayuda",
        "En esta ventana de Debug CPU se pueden tener diferentes vistas (seleccionables con las teclas 1-8), cada una mostrando diferente información:\n"

        );
    }

    else if (gui_language==GUI_LANGUAGE_CATALAN) {
        menu_generic_message("Ajuda",
        "En aquesta finestra de Debug CPU es poden tenir diferents vistes (seleccionables amb les tecles 1-8), cadascuna mostrant diferent informació:\n"

        );
    }


    else {
        menu_generic_message("Help",
        "This Debug CPU window can show different views (chosen with keys 1-8), each of them showing different information:\n"
        "1 - Default view. You see a top section with opcodes disassembly and registers. This section can be changed pressing key m, "

        );
    }*/

        menu_generic_message("Help",
        "This tool reads from a HiLow Tape Audio file and converts it to Data Drive Hilow image file (.ddh) that can be used on ZEsarUX.\n"
        "The input audio file can be a .raw file (44100 Hz, mono, 8 bit, unsigned) but also a .wav file (you will need sox utility configured to read .wav).\n"
        "The input audio file must be divided on two files: one from the side A, and the other one from the side B.\n"
        "The sector being read is enabled as a Memory Zone, that you may see using the Hexadecimal Editor, for example.\n"
        "\n"
        "The keys used in this window are:\n"
        "\n"
        "F1: This help\n"
        "i: Set the input audio file\n"
        "o: Set the output audio file\n"
        "r: Run conversion\n"
        "s: Stop conversion\n"
        "p: Pause conversion\n"
        "l: Set speed conversion to very slow (1 audio frame every 80ms)\n"
        "w: Set speed conversion to slow (1 audio frame every 20ms)\n"
        "1: Set speed conversion to realtime (1x)\n"
        "2: Set speed conversion to 2x\n"
        "4: Set speed conversion to 4x\n"
        "8: Set speed conversion to 8x\n"
        "b: You must set this for B-side audio files. This setting must be set before start running the conversion. "
        "It seems that the B-side will only be correctly read if it was formatted only once; however, I haven't tested it too much "
        "so maybe you have luck reading B-side ;)\n"
        "g: Invert audio input signal ('mirror' vertically), needed for some tapes\n"
        "t: Change noise filter threshold, higher values means increase noise reduction\n"
        "d: Enable adaptative algorithm, which adjusts bit width depending on the S_START_BYTE signal.\n"
        "c: Autocorrect. Try to fix read errors depending on S_START_BYTE signal, useful for bad quality audio tapes\n"
        "u: Enable sound. You may use the Waveform Window to see the signal; the Scroll shape mode from that window "
        "shows detailed wave using slow speed or very slow.\n"
        "a: Automatic mode. Do not ask anything to the user and convert all the sectors; "
        "in non-automatic mode, after every sector you will be asked about what you want to do.\n"
        "m: Show microseconds instead of audio frames on the Elapsed field.\n"
        "\n"
        "After each sector, and if you don't enable automatic mode, you will be asked about what you want to do:\n"
        "\n"
        "e: Repeat sector: in case you have changed some settings and want to read the sector again.\n"
        "v: Save sector: saves sector to a temporary memory. The final ddh file will be saved after all input data has been read.\n"
        "n: Next sector: do not save the current sector\n"
        "h: Change sector: the sector number will be read from the sector data itself; you may change it in case the read sector is not right.\n"
        "\n"
        "Note: in case the sector number is suspected to be wrong (comparing it to the sector marks on an A-side file) you will be warned "
        "with a message like: 'Probably sector mismatch!'"


        );

}



#ifdef USE_PTHREADS
pthread_t hilow_convert_audio_thread;


void *menu_hilow_convert_audio_thread_function(void *nada GCC_UNUSED)
{

    debug_printf(VERBOSE_DEBUG,"Start HiLow convert audio thread");

    //TODO: esto deberia ser un semaforo, pero el usuario tendria que ser muy muy rapido para poder ejecutar esto dos veces seguidas
    //y que suceda esto simultaneamente en dos sitios a la vez
    if (hilow_convert_audio_thread_running) {
        debug_printf(VERBOSE_DEBUG,"Already running HiLow convert audio thread");
        return NULL;
    }


    hilow_convert_audio_thread_running=1;


    //activar callbacks
    hilow_read_audio_byteread_callback=menu_hilow_convert_audio_callback;
    hilow_read_audio_byte_output_write_callback=menu_hilow_convert_audio_write_byte_callback;
    hilow_read_audio_bit_output_write_callback=menu_hilow_convert_audio_write_bit_callback;
    hilow_read_audio_probably_sync_error_callback=menu_hilow_convert_audio_probably_sync_error;


    //Leer archivo entrada
    hilow_read_audio_read_hilow_memoria_audio=menu_hilow_convert_audio_read_hilow_audio_file(menu_hilow_convert_audio_input_raw);



    if (hilow_read_audio_read_hilow_memoria_audio==NULL) {
        hilow_convert_audio_thread_running=0;
        return NULL;
    }


    //Asignar memoria para archivo salida y leerlo (si existe)
    if (!menu_hilow_convert_audio_read_hilow_ddh_file(menu_hilow_convert_audio_output_ddh)) {
        hilow_convert_audio_thread_running=0;
        return NULL;
    }


    menu_hilow_convert_audio_posicion_read_raw=0;
    int total_bytes_leidos;


    //En bucle leer todos los sectores
    while (menu_hilow_convert_audio_posicion_read_raw!=-1) {

        hilow_read_audio_lee_sector_bytes_leidos=0;
        int antes_posicion=menu_hilow_convert_audio_posicion_read_raw;

        debug_printf(VERBOSE_DEBUG,"Position begin search sector: %d",menu_hilow_convert_audio_posicion_read_raw);
        menu_hilow_convert_audio_posicion_read_raw=hilow_read_audio_buscar_inicio_sector(menu_hilow_convert_audio_posicion_read_raw);

        debug_printf(VERBOSE_DEBUG,"Position start sector data: %d",menu_hilow_convert_audio_posicion_read_raw);
        menu_hilow_convert_audio_posicion_read_raw=hilow_read_audio_lee_sector(menu_hilow_convert_audio_posicion_read_raw,&total_bytes_leidos,&menu_hilow_convert_audio_sector);

        hilow_read_audio_current_phase=HILOW_READ_AUDIO_PHASE_NONE;


        int preguntar=1;

        if (menu_hilow_convert_audio_completamente_automatico) preguntar=0;

        //Y si hay sector mismatch, preguntar siempre
        if (hilow_read_audio_warn_if_sector_mismatch(menu_hilow_convert_audio_sector)) preguntar=1;


        //Si en modo automatico, no pregunto nada y grabamos siempre sector
        if (!preguntar) {
            debug_printf(VERBOSE_INFO,"Saving sector %d to memory",menu_hilow_convert_audio_sector);
            hilow_read_audio_write_sector_to_memory(menu_hilow_convert_audio_sector);
            menu_hilow_convert_audio_anterior_sector_leido=menu_hilow_convert_audio_sector;
        }
        else {
            //preguntar que hacer
            menu_hilow_convert_audio_esperar_siguiente_sector=1;

            while (menu_hilow_convert_audio_esperar_siguiente_sector) {
                usleep(20000);
            }
        }

        //Siguiente sector o repetir
        //Antes, reseteamos contador de sector leido, label y buffer marca de 5 bytes sector
        menu_hilow_convert_audio_sector=0;
        hilow_read_audio_reset_buffer_label();
        hilow_read_audio_reset_buffer_sector_five_byte();

        //Y primer byte del buffer de sector a 0, para que la lectura del numero de sector primero sea 0 y luego se pueda obtener a medida que se lean bytes
        hilow_read_audio_buffer_result[0]=0;

        if (menu_hilow_convert_audio_must_repeat_sector) {
            menu_hilow_convert_audio_must_repeat_sector=0;
            menu_hilow_convert_audio_posicion_read_raw=antes_posicion;
        }

    }

    //Escribir memoria a archivo ddh
    menu_hilow_convert_audio_write_hilow_ddh_file(menu_hilow_convert_audio_output_ddh);

    free(hilow_read_audio_read_hilow_memoria_audio);
    free(hilow_read_audio_hilow_ddh);


    //Para que no aparezca posicion -1
    menu_hilow_convert_audio_posicion_read_raw=0;

    debug_printf(VERBOSE_DEBUG,"End HiLow convert audio thread");


    hilow_convert_audio_thread_running=0;


    return NULL;


}

//Iniciar el thread
void menu_hilow_convert_audio_run_thread(void)
{
    //menu_hilow_convert_audio_must_stop_thread=0;

    if (pthread_create( &hilow_convert_audio_thread, NULL, &menu_hilow_convert_audio_thread_function, NULL) ) {
                debug_printf(VERBOSE_ERR,"Can not create HiLow convert audio thread");
                return;
    }

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(hilow_convert_audio_thread);
}

//Detener el thread
void menu_hilow_convert_audio_stop_thread(void)
{


    if (hilow_convert_audio_thread_running) {
        debug_printf(VERBOSE_DEBUG,"Stopping HiLow convert audio thread");

        menu_hilow_convert_audio_esperar_siguiente_sector=0;
        //menu_hilow_convert_audio_must_stop_thread=1;

        if (pthread_cancel(hilow_convert_audio_thread)) {
            menu_error_message("Error canceling thread");
        }

        hilow_convert_audio_thread_running=0;

    }


}

# else

//Si se compila sin pthreads
void menu_hilow_convert_audio_run_thread(void)
{

}

void menu_hilow_convert_audio_stop_thread(void)
{

}

#endif


//Textos de  Fase en curso
char *menu_hilow_convert_phases_strings[]={
    "None",
    "Searching sector marks",
    "Reading sector marks",
    "Searching sector label",
    "Reading sector label",
    "Searching sector data",
    "Reading sector data"

};

zxvision_window *menu_hilow_convert_audio_window;

void menu_hilow_convert_audio_overlay(void)
{



    menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (menu_hilow_convert_audio_window->is_minimized) return;


    zxvision_window *ventana;

    ventana=menu_hilow_convert_audio_window;


    //Forzar a mostrar atajos
    z80_bit antes_menu_writing_inverse_color;
    antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
    menu_writing_inverse_color.v=1;

    //f1: help se sobreescribe si hay archivo input y output activo
    zxvision_print_string_defaults_fillspc(ventana,1,0,"~~input ~~output ~~f~~1:help");

    //No escribir nada del texto siguiente si no hay input u output
    if (menu_hilow_convert_audio_input_raw[0] && menu_hilow_convert_audio_output_ddh[0]) {

        if (!hilow_convert_audio_thread_running) {
            //agregar el texto
            zxvision_print_string_defaults(ventana,14,0,"~~run conversion - STOPPED");
        }
        else {
            //Borrar toda la linea y escribir solo esto
            zxvision_print_string_defaults_fillspc_format(ventana,1,0,"~~stop conversion - RUNNING");
        }

        int linea=6;

        int minutos=menu_hilow_convert_audio_posicion_read_raw/44100/60;
        int segundos=(menu_hilow_convert_audio_posicion_read_raw/44100) % 60;
        long long int frames=menu_hilow_convert_audio_posicion_read_raw % 44100;
        long long int microseconds=(1000000 * frames)/44100;

        long long int porcentaje_leido;

        if (hilow_read_audio_tamanyo_archivo_audio>0) {
            porcentaje_leido=(menu_hilow_convert_audio_posicion_read_raw*100)/hilow_read_audio_tamanyo_archivo_audio;
        }
        else {
            porcentaje_leido=0;
        }


        //char texto_unidades[30];
        char texto_contador_unidades[50];


        if (menu_hilow_convert_unidades_microseconds) {
            sprintf(texto_contador_unidades,"%06lld (mm:ss:microsec)",microseconds);
        }
        else {
            sprintf(texto_contador_unidades,"%05lld (mm:ss:frames)",frames);
        }

        if (menu_hilow_convert_audio_posicion_read_raw==-1) {
            zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Elapsed: End of file");
        }

        else {
            zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Elapsed: %02d:%02d:%s - %d %%",
                minutos,segundos,texto_contador_unidades,
                porcentaje_leido);
        }

        if (menu_hilow_convert_audio_posicion_read_raw!=-1) {
            zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Read position: %d bytes (%d KBytes)",
            menu_hilow_convert_audio_posicion_read_raw,menu_hilow_convert_audio_posicion_read_raw/1024);
        }

        if (hilow_read_audio_current_phase>=HILOW_READ_AUDIO_PHASE_NONE && hilow_read_audio_current_phase<=HILOW_READ_AUDIO_PHASE_READING_SECTOR_DATA) {
            zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Phase: %s",menu_hilow_convert_phases_strings[hilow_read_audio_current_phase]);
        }


        if (menu_hilow_convert_mostrar_probable_error && !hilow_read_audio_autocorrect) {
            //Si no esta autocorrect, avisar al usuario que deberia habilitarlo
                zxvision_print_string_format(ventana,1,linea++,ESTILO_GUI_COLOR_AVISO,ESTILO_GUI_PAPEL_NORMAL,0,
                        "Probably read error, you should enable autocorrect");
        }
        else
        {
            //dejar linea en blanco aparte de borrar si hay algo
            zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"");
        }

        //dejar linea en blanco aparte de borrar si hay algo
        zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"");

        zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Bits read: %s  Last bit: %d  %s",
            menu_hilow_convert_audio_string_bits,menu_hilow_convert_audio_last_bit,
            (menu_hilow_convert_audio_just_read_bit ? "New bit" : "") );
        menu_hilow_convert_audio_just_read_bit=0;


        zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Bytes read: %s %s",menu_hilow_convert_audio_string_bytes,
            (menu_hilow_convert_audio_just_read_byte ? "New byte" : "") );
        menu_hilow_convert_audio_just_read_byte=0;

        zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Ascii read: %s",menu_hilow_convert_audio_string_bytes_ascii);

        //dejar linea en blanco aparte de borrar si hay algo
        zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"");


        zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Sector number read: %d (%02XH)",menu_hilow_convert_audio_sector,menu_hilow_convert_audio_sector);


        if (!hilow_read_audio_leer_cara_dos) {
            char buffer_label[32];
            util_binary_to_ascii(&hilow_read_audio_buffer_label[1],buffer_label,14,14);
            zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Sector label: %s",buffer_label);

            zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Begin Sector id mark: %02X %02X %02X %02X %02X",
                hilow_read_audio_buffer_sector_five_byte[0],hilow_read_audio_buffer_sector_five_byte[1],hilow_read_audio_buffer_sector_five_byte[2],
                hilow_read_audio_buffer_sector_five_byte[3],hilow_read_audio_buffer_sector_five_byte[4]);
        }

        //Realmente hay mas bytes a final de sector pero aqui solo mostramos los 5 ultimos
        zxvision_print_string_defaults_fillspc_format(ventana,1,linea,"End Sector id mark:");

        int tinta_aviso_final_sector=ESTILO_GUI_TINTA_NORMAL;

        //Avisar si hemos leido todo el sector y el byte del final no es el numero de sector
        if (hilow_read_audio_lee_sector_bytes_leidos==HILOW_SECTOR_SIZE &&
            hilow_read_audio_buffer_end_sector[HILOW_LONGITUD_FINAL_SECTOR-1]!=menu_hilow_convert_audio_sector) {
                tinta_aviso_final_sector=ESTILO_GUI_COLOR_AVISO;
        }

        zxvision_print_string_format(ventana,23,linea++,tinta_aviso_final_sector,ESTILO_GUI_PAPEL_NORMAL,0,"%02X %02X %02X %02X %02X",
            hilow_read_audio_buffer_end_sector[HILOW_LONGITUD_FINAL_SECTOR-5],hilow_read_audio_buffer_end_sector[HILOW_LONGITUD_FINAL_SECTOR-4],
            hilow_read_audio_buffer_end_sector[HILOW_LONGITUD_FINAL_SECTOR-3],hilow_read_audio_buffer_end_sector[HILOW_LONGITUD_FINAL_SECTOR-2],
            hilow_read_audio_buffer_end_sector[HILOW_LONGITUD_FINAL_SECTOR-1]);

        zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"Total sector bytes read: %d (%XH)",
            hilow_read_audio_lee_sector_bytes_leidos,hilow_read_audio_lee_sector_bytes_leidos);


        if (menu_hilow_convert_audio_esperar_siguiente_sector) {
            //dejar linea en blanco aparte de borrar si hay algo
            zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"");


            if (hilow_read_audio_warn_if_sector_mismatch(menu_hilow_convert_audio_sector)) {
                zxvision_print_string_format(ventana,1,linea++,ESTILO_GUI_COLOR_AVISO,ESTILO_GUI_PAPEL_NORMAL,0,
                        "Probably sector mismatch! Maybe %d. Do you want to:",menu_hilow_convert_audio_anterior_sector_leido+1);
            }

            else {
                if (!hilow_read_audio_leer_cara_dos) zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"End read sector, seems ok. Do you want to:");
                else zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"End read sector. Do you want to:");
            }


            zxvision_print_string_defaults_fillspc_format(ventana,1,linea++,"r~~epeat sa~~ve ~~next sector c~~hange sector");
        }




    }

    //Restaurar comportamiento atajos
    menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;


    zxvision_draw_window_contents(menu_hilow_convert_audio_window);

}


void menu_hilow_convert_audio_input_file(void)
{

    char *filtros[3];


    filtros[0]="wav";
    filtros[1]="raw";
    filtros[2]=0;


	//guardamos directorio actual
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	int ret;

	//Obtenemos ultimo directorio visitado
	if (menu_hilow_convert_audio_input_raw[0]!=0) {
		char directorio[PATH_MAX];
		util_get_dir(menu_hilow_convert_audio_input_raw,directorio);
		//printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

		//cambiamos a ese directorio, siempre que no sea nulo
		if (directorio[0]!=0) {
				debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
				zvfs_chdir(directorio);
		}
	}


    char buffer_load_file[PATH_MAX];

    ret=menu_filesel("Select Input raw File",filtros,buffer_load_file);

	//volvemos a directorio inicial
	zvfs_chdir(directorio_actual);

    if (ret) {
        strcpy(menu_hilow_convert_audio_input_raw,buffer_load_file);
    }

    //Se pierde el overlay cada vez que se abre file selector
    //set_menu_overlay_function(menu_hilow_convert_audio_overlay);

}


void menu_hilow_convert_audio_output_file(void)
{

    char *filtros[2];


    filtros[0]="ddh";
    filtros[1]=0;


	//guardamos directorio actual
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	int ret;

	//Obtenemos ultimo directorio visitado
	if (menu_hilow_convert_audio_output_ddh[0]!=0) {
		char directorio[PATH_MAX];
		util_get_dir(menu_hilow_convert_audio_output_ddh,directorio);
		//printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

		//cambiamos a ese directorio, siempre que no sea nulo
		if (directorio[0]!=0) {
				debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
				zvfs_chdir(directorio);
		}
	}


    char buffer_load_file[PATH_MAX];

    ret=menu_filesel("Select Output ddh File",filtros,buffer_load_file);

	//volvemos a directorio inicial
	zvfs_chdir(directorio_actual);

    if (ret) {
        strcpy(menu_hilow_convert_audio_output_ddh,buffer_load_file);
    }

    //Se pierde el overlay cada vez que se abre file selector
    //set_menu_overlay_function(menu_hilow_convert_audio_overlay);

}



//Almacenar la estructura de ventana aqui para que se pueda referenciar desde otros sitios
zxvision_window zxvision_window_hilow_convert_audio;


void menu_hilow_convert_audio(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();

    if (!menu_multitarea) {
        menu_warn_message("This window needs multitask enabled");
        return;
    }

#ifndef USE_PTHREADS
        menu_warn_message("This window needs pthreads enabled");
        return;
#endif

    zxvision_window *ventana;
    ventana=&zxvision_window_hilow_convert_audio;

	//IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
	//si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
	//la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
	//zxvision_delete_window_if_exists(ventana);

    //Crear ventana si no existe
    if (!zxvision_if_window_already_exists(ventana)) {



        int xventana,yventana,ancho_ventana,alto_ventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;

        if (!util_find_window_geometry("hilowconvertaudio",&xventana,&yventana,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {
            ancho_ventana=55;
            alto_ventana=25;

            xventana=menu_center_x()-ancho_ventana/2;
            yventana=menu_center_y()-alto_ventana/2;
        }


        zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"HiLow Convert Audio",
            "hilowconvertaudio",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);

        ventana->can_be_backgrounded=1;

    }

    //Si ya existe, activar esta ventana
    else {

        zxvision_activate_this_window(ventana);
    }



	zxvision_draw_window(ventana);

	z80_byte tecla;


	int salir=0;

    menu_hilow_convert_audio_tiempo_inicial();


    menu_hilow_convert_audio_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui



    //cambio overlay
    zxvision_set_window_overlay(ventana,menu_hilow_convert_audio_overlay);


    //Toda ventana que este listada en zxvision_known_window_names_array debe permitir poder salir desde aqui
    //Se sale despues de haber inicializado overlay y de cualquier otra variable que necesite el overlay
    if (zxvision_currently_restoring_windows_on_start) {
            //printf ("Saliendo de ventana ya que la estamos restaurando en startup\n");
            return;
    }



    menu_hilow_convert_audio_has_been_opened=1;

    //char buffer_load_file[PATH_MAX];

    //char *filtros[3];

    do {

        zxvision_cls(ventana);

        //Forzar a mostrar atajos
        z80_bit antes_menu_writing_inverse_color;
        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
        menu_writing_inverse_color.v=1;

        //No escribir nada del texto siguiente si no hay input u output
        if (menu_hilow_convert_audio_input_raw[0] && menu_hilow_convert_audio_output_ddh[0]) {

            //Escribir linea opciones velocidad
            //speed: paused/very slow/slow/1x/2x/4x/8x/fastest
            zxvision_print_string_defaults_fillspc_format(ventana,1,1,"speed: ");


            int x=8;
            char buffer_item[30];

            int i;
            //8 posibles

            int total_opciones=8;

            for (i=0;i<total_opciones;i++) {

                int seleccionado=0;

                switch (i) {
                    case 0:
                        strcpy(buffer_item,"~~paused");
                        if (menu_hilow_convert_paused) seleccionado=1;
                    break;

                    case 1:
                        strcpy(buffer_item,"very s~~low");
                        if (menu_hilow_convert_lento==6) seleccionado=1;
                    break;

                    case 2:
                        strcpy(buffer_item,"slo~~w");
                        if (menu_hilow_convert_lento==1) seleccionado=1;
                    break;

                    case 3:
                        strcpy(buffer_item,"~~1x");
                        if (menu_hilow_convert_speed==1) seleccionado=1;
                    break;

                    case 4:
                        strcpy(buffer_item,"~~2x");
                        if (menu_hilow_convert_speed==2) seleccionado=1;
                    break;

                    case 5:
                        strcpy(buffer_item,"~~4x");
                        if ( menu_hilow_convert_speed==4) seleccionado=1;
                    break;

                    case 6:
                        strcpy(buffer_item,"~~8x");
                        if (menu_hilow_convert_speed==8) seleccionado=1;
                    break;

                    case 7:
                        strcpy(buffer_item,"~~fastest");
                        if (menu_hilow_convert_audio_fast_mode) seleccionado=1;
                    break;

                }

                //int tinta=ESTILO_GUI_TINTA_NORMAL;



                zxvision_print_string_format(ventana,x,1,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,seleccionado,"%s%c",buffer_item,
                (i<total_opciones-1 ? '/' : ' ') );

                x +=strlen(buffer_item)-2; //quitarle 2 del hotkey

                if (i<total_opciones-1) zxvision_print_string_defaults(ventana,x,1,"/");

                x++;
            }


            zxvision_print_string_defaults_fillspc_format(ventana,1,2,"[%c] ~~b-side [%c] invert si~~gnal [%d] fil~~ter",
                (hilow_read_audio_leer_cara_dos ? 'X' : ' '),
                (hilow_read_audio_invertir_senyal ? 'X' : ' '),
                hilow_read_audio_minimo_variacion

            );

            zxvision_print_string_defaults_fillspc_format(ventana,1,3,"[%c] a~~daptative algorithm [%c] auto~~correct",
                (hilow_read_audio_autoajustar_duracion_bits ? 'X' : ' '),
                (hilow_read_audio_autocorrect ? 'X' : ' ')
            );

            zxvision_print_string_defaults_fillspc_format(ventana,1,4,"[%c] ~~automatic [%c] ~~microseconds [%c] so~~und",
                (menu_hilow_convert_audio_completamente_automatico ? 'X' : ' '),
                (menu_hilow_convert_unidades_microseconds ? 'X' : ' ' ),
                (menu_hilow_convert_audio_hear_sound ? 'X' : ' ')
            );

            zxvision_print_string_defaults_fillspc_format(ventana,1,5,"~~F~~1:help");
        }


        //Restaurar comportamiento atajos
        menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;


		tecla=zxvision_common_getkey_refresh();


        switch (tecla) {

            case MENU_TECLA_AYUDA:

                menu_hilow_convert_help();

            break;

            case 'r':
                if (menu_hilow_convert_audio_input_raw[0]==0) menu_error_message("No input file selected");
                else if (menu_hilow_convert_audio_output_ddh[0]==0) menu_error_message("No output file selected");
                else {
                    if (!hilow_convert_audio_thread_running) {
                        menu_hilow_convert_audio_run_thread();

                        if (!hilow_read_audio_leer_cara_dos) menu_hilow_convert_audio_anterior_sector_leido=0;
                        else menu_hilow_convert_audio_anterior_sector_leido=128;

                        //Si hubiera algun aviso de probable error, quitarlo
                        menu_hilow_convert_mostrar_probable_error=0;

                    }
                }


            break;

            case 's':
                menu_hilow_convert_audio_stop_thread();
            break;


            case 'm':
                menu_hilow_convert_unidades_microseconds ^=1;
            break;

            case 'g':
                hilow_read_audio_invertir_senyal ^=1;
            break;

            case 'c':
                hilow_read_audio_autocorrect ^=1;
            break;

            case 't':
                hilow_read_audio_minimo_variacion +=2;
                if (hilow_read_audio_minimo_variacion==30) hilow_read_audio_minimo_variacion=2;
            break;

            case '1':
                menu_hilow_convert_speed=1;

                menu_hilow_convert_lento=0;
                menu_hilow_convert_audio_fast_mode=0;
                menu_hilow_convert_paused=0;
            break;

            case '2':
                menu_hilow_convert_speed=2;

                menu_hilow_convert_lento=0;
                menu_hilow_convert_audio_fast_mode=0;
                menu_hilow_convert_paused=0;
            break;

            case '4':
                menu_hilow_convert_speed=4;

                menu_hilow_convert_lento=0;
                menu_hilow_convert_audio_fast_mode=0;
                menu_hilow_convert_paused=0;
            break;

            case '8':
                menu_hilow_convert_speed=8;

                menu_hilow_convert_lento=0;
                menu_hilow_convert_audio_fast_mode=0;
                menu_hilow_convert_paused=0;
            break;


            case 'w':
                menu_hilow_convert_lento=1;

                if (menu_hilow_convert_lento) {
                    //Poner buffer a silencio para borrar lo anterior
                    memset(menu_hilow_convert_audio_buffer,0,AUDIO_BUFFER_SIZE);
                }

                menu_hilow_convert_audio_fast_mode=0;
                menu_hilow_convert_paused=0;
                menu_hilow_convert_speed=0;
            break;

            case 'l':
                menu_hilow_convert_lento=6;

                if (menu_hilow_convert_lento) {
                    //Poner buffer a silencio para borrar lo anterior
                    memset(menu_hilow_convert_audio_buffer,0,AUDIO_BUFFER_SIZE);
                }

                menu_hilow_convert_audio_fast_mode=0;
                menu_hilow_convert_paused=0;
                menu_hilow_convert_speed=0;
            break;


            case 'p':
                menu_hilow_convert_paused=1;

                menu_hilow_convert_lento=0;
                menu_hilow_convert_audio_fast_mode=0;
                menu_hilow_convert_speed=0;
            break;

            case 'f':
                menu_hilow_convert_audio_fast_mode=1;

                menu_hilow_convert_lento=0;
                menu_hilow_convert_paused=0;
                menu_hilow_convert_speed=0;
            break;

            case 'd':
                hilow_read_audio_autoajustar_duracion_bits ^=1;
            break;

            case 'a':
                menu_hilow_convert_audio_completamente_automatico ^=1;
            break;

            case 'b':
                hilow_read_audio_leer_cara_dos ^=1;
                if (hilow_convert_audio_thread_running) menu_first_aid("hilow_convert_bside");
            break;

            case 'i':

                menu_hilow_convert_audio_input_file();

            break;

            case 'o':

                menu_hilow_convert_audio_output_file();

            break;


            case 'u':
                menu_hilow_convert_audio_hear_sound ^=1;
            break;


            //Salir con ESC
            case 2:
                salir=1;
            break;

            //O tecla background
            case 3:
                salir=1;
            break;
        }


        //Si esperamos accion del usuario a final de sector
        if (menu_hilow_convert_audio_esperar_siguiente_sector) {
            switch(tecla) {
                case 'n':
                    debug_printf(VERBOSE_INFO,"Skipping sector");
                    //Siguiente sector
                    menu_hilow_convert_audio_esperar_siguiente_sector=0;
                break;

                case 'v':
                    //Grabar sector
                    debug_printf(VERBOSE_INFO,"Saving sector %d to memory",menu_hilow_convert_audio_sector);
                    hilow_read_audio_write_sector_to_memory(menu_hilow_convert_audio_sector);
                    menu_hilow_convert_audio_esperar_siguiente_sector=0;
                    menu_hilow_convert_audio_anterior_sector_leido=menu_hilow_convert_audio_sector;
                break;

                case 'e':
                    //Repetir lectura
                    debug_printf(VERBOSE_INFO,"Repeating sector");
                    menu_hilow_convert_audio_must_repeat_sector=1;
                    menu_hilow_convert_audio_esperar_siguiente_sector=0;
                break;


                case 'h':
                    menu_ventana_scanf_numero_enhanced("Sector?",&menu_hilow_convert_audio_sector,4,+1,1,255,0);
                break;



            }
         }


    } while (salir==0);




	util_add_window_geometry_compact(ventana);

	if (tecla==3) {
		zxvision_message_put_window_background();
	}

	else {

        //Cerrar el thread
        menu_hilow_convert_audio_stop_thread();

		zxvision_destroy_window(ventana);
	}


}

zxvision_window *menu_hilow_visual_datadrive_window;

//Ver si ha cambiado el porcentaje sobre los rollos, para redibujar
//eso indica que la cantidad de cinta enrollada es diferente
int menu_hilow_visual_datadrive_porcentaje_anterior=-1;

int antes_hilow_visual_rodillo_arrastre_grados=-1;

int menu_hilow_visual_datadrive_temblor=0;

int menu_hilow_antes_semaforos=-1;

int visual_hilow_datadrive_forzar_dibujado=0;

void menu_hilow_visual_datadrive_overlay(void)
{

    menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (menu_hilow_visual_datadrive_window->is_minimized) return;


    //Si no esta hilow activado y con cinta raw enabled, no mostrar
    if (hilow_enabled.v==0) return;

    if (hilow_rom_traps.v) return;

    if (hilow_cinta_insertada_flag.v==0) return;


    //Mostrar cinta, usar dibujo generico

    int porcentaje_transcurrido=hilow_raw_transcurrido_cinta_porc();
    int minutos_total_cinta=hilow_raw_get_minutes_tape();

    //Tenemos que dar el porcentaje sobre el maximo, que es de una cinta de 90 (45 minutos por cara)
    //Sacar el porcentaje que representa el total de la cinta sobre una de 90
    int porc_90=(minutos_total_cinta*100)/90;

    //Y aplicamos el transcurrido sobre ese valor
    int porcentaje=(porc_90*porcentaje_transcurrido)/100;


    int porcentaje_cinta_izquierdo=porc_90-porcentaje;
    int porcentaje_cinta_derecho=porcentaje;

    int redibujar_rollos=0;

    if (porcentaje!=menu_hilow_visual_datadrive_porcentaje_anterior) redibujar_rollos=1;

    //determinar esto solo cuando sea necesario. probablemente con dirty
    int redibujar_parte_estatica=0;

    int redibujar_rodillos_arrastre=0;

    int redibujar_semaforos=0;

    //Si grados rodillos antes iguales a actual, no redibujar
    if (hilow_visual_rodillo_arrastre_grados!=antes_hilow_visual_rodillo_arrastre_grados) redibujar_rodillos_arrastre=1;

    //No redibujar si no hay cambios de nada
    if (menu_hilow_visual_datadrive_window->dirty_user_must_draw_contents || visual_hilow_datadrive_forzar_dibujado) {
        //printf("Redibujando parte estatica y dinamica y rodillos arrastre. %d\n",contador_segundo_infinito);
        redibujar_parte_estatica=1;
        redibujar_rollos=1;
        redibujar_rodillos_arrastre=1;
        redibujar_semaforos=1;

        menu_hilow_visual_datadrive_window->dirty_user_must_draw_contents=0;
    }

    visual_hilow_datadrive_forzar_dibujado=0;

    if (redibujar_rollos) {
        //printf("Redibujando parte dinamica. %d\n",contador_segundo_infinito);
    }


    int antes_porcentaje_cinta_izquierdo=porc_90-menu_hilow_visual_datadrive_porcentaje_anterior;
    int antes_porcentaje_cinta_derecho=menu_hilow_visual_datadrive_porcentaje_anterior;



    int semaforos_hilow=1;


    //Solo un led a la vez

    //rojo
    if (last_hilow_port_value & HILOW_PORT_MASK_WRITE_EN) {
        semaforos_hilow |=4;
    }

    //amarillo
    else if (last_hilow_port_value & HILOW_PORT_MASK_MOTOR_ON) {
        semaforos_hilow |=8;
    }

    //verde
    else if (!hilow_cinta_en_movimiento && (last_hilow_port_value & HILOW_PORT_MASK_MOTOR_ON)==0 && hilow_reproductor_encendido.v) {
        semaforos_hilow |=16;
    }

    if (semaforos_hilow!=menu_hilow_antes_semaforos) redibujar_semaforos=1;

    menu_hilow_antes_semaforos=semaforos_hilow;

    if (redibujar_semaforos) semaforos_hilow |=2;

    //Si esta motor on, tiembla
    if (last_hilow_port_value & HILOW_PORT_MASK_MOTOR_ON) {
        menu_hilow_visual_datadrive_temblor^=1;
        redibujar_rodillos_arrastre=1;
    }

    zxvision_window *w=menu_hilow_visual_datadrive_window;

    //Calcular tamaños


    int tamanyo_ocupado_hilow_ancho=(w->visible_width-3)*menu_char_width;
    //quitamos 5: barra titulo,barra scroll, 2 lineas menu, 1 linea separacion
    int tamanyo_ocupado_hilow_alto=(w->visible_height-5)*menu_char_height;

    int offset_x=menu_char_width*1;
    int offset_y=menu_char_height*3;

    //Ajustar escalas
    //Relacion de aspecto ideal: 1000 ancho, 630 alto
    //Sumamos 100 mas para los "semaforos" de hilow

    int ancho_total_dibujo_virtual=GENERIC_VISUALTAPE_ANCHO_CINTA;

    //Sumar espacio de los semaforos
    ancho_total_dibujo_virtual+=100;



    int real_width=tamanyo_ocupado_hilow_ancho;


    //Desactivar este trocito si queremos que el ancho pueda crecer independientemente del alto de ventana. SOLO PARA PRUEBAS
    int max_ancho_esperado_por_aspecto=(tamanyo_ocupado_hilow_alto*ancho_total_dibujo_virtual)/GENERIC_VISUALTAPE_ALTO_CINTA;
    if (real_width>max_ancho_esperado_por_aspecto) {
        //Con esto el microdrive siempre esta dentro de la ventana entero, independientemente del tamaño de la ventana
        //printf("relacion ancho mal\n");
        real_width=max_ancho_esperado_por_aspecto;
    }


    int real_height=(real_width*GENERIC_VISUALTAPE_ALTO_CINTA)/ancho_total_dibujo_virtual;



    menu_generic_visualtape(menu_hilow_visual_datadrive_window,
    real_width,real_height,offset_x,offset_y,
    porcentaje_cinta_izquierdo,porcentaje_cinta_derecho,
    antes_porcentaje_cinta_izquierdo,antes_porcentaje_cinta_derecho,
    hilow_visual_rodillo_arrastre_grados,antes_hilow_visual_rodillo_arrastre_grados,
    redibujar_rollos,redibujar_parte_estatica,redibujar_rodillos_arrastre,
    menu_hilow_visual_datadrive_temblor,semaforos_hilow,hilow_write_protection.v ^1);



    menu_hilow_visual_datadrive_porcentaje_anterior=porcentaje;
    antes_hilow_visual_rodillo_arrastre_grados=hilow_visual_rodillo_arrastre_grados;


    //Mostrar contenido
    zxvision_draw_window_contents(menu_hilow_visual_datadrive_window);

}


void menu_visual_hilow_slow_movement(MENU_ITEM_PARAMETERS)
{
    hilow_visual_slow_movement ^=1;
}

//Almacenar la estructura de ventana aqui para que se pueda referenciar desde otros sitios
zxvision_window zxvision_window_hilow_visual_datadrive;


void menu_hilow_visual_datadrive(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();

    if (!menu_multitarea) {
        menu_warn_message("This window needs multitask enabled");
        return;
    }

    //forzar redibujar rollos. esto no hace falta, ya se hace con visual_hilow_datadrive_forzar_dibujado
    //menu_hilow_visual_datadrive_porcentaje_anterior=-1;

    zxvision_window *ventana;
    ventana=&zxvision_window_hilow_visual_datadrive;

	//IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
	//si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
	//la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
	//zxvision_delete_window_if_exists(ventana);

    //Crear ventana si no existe
    if (!zxvision_if_window_already_exists(ventana)) {
        int xventana,yventana,ancho_ventana,alto_ventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;

        if (!util_find_window_geometry("hilowvisualdatadrive",&xventana,&yventana,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {
            ancho_ventana=38;
            alto_ventana=27;

            xventana=menu_center_x()-ancho_ventana/2;
            yventana=menu_center_y()-alto_ventana/2;
        }


        zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"Visual Hilow DataDrive",
            "hilowvisualdatadrive",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);

        ventana->can_be_backgrounded=1;

        //definir color de papel de fondo
        ventana->default_paper=GENERIC_VISUALTAPE_COLOR_FONDO;
        zxvision_cls(ventana);
        //visual_microdrive_forzar_redraw=1;

    }

    //Si ya existe, activar esta ventana
    else {
        zxvision_activate_this_window(ventana);
    }

	zxvision_draw_window(ventana);

    //para mostrar correctamente el color del fondo alterado por default_paper
    zxvision_draw_window_contents(ventana);


    menu_hilow_visual_datadrive_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


    //cambio overlay
    zxvision_set_window_overlay(ventana,menu_hilow_visual_datadrive_overlay);


    //Toda ventana que este listada en zxvision_known_window_names_array debe permitir poder salir desde aqui
    //Se sale despues de haber inicializado overlay y de cualquier otra variable que necesite el overlay
    if (zxvision_currently_restoring_windows_on_start) {
        //printf ("Saliendo de ventana ya que la estamos restaurando en startup\n");
        return;
    }


	menu_item *array_menu_visual_hilow;
	menu_item item_seleccionado;
	int retorno_menu;
	do {

        //Borrar posible texto anterior
        //zxvision_print_string_defaults_fillspc(ventana,1,1,"");

        visual_hilow_datadrive_forzar_dibujado=1;
        zxvision_cls(ventana);


        if (hilow_enabled.v==0) zxvision_print_string_defaults_fillspc(ventana,1,1,"Hilow is not enabled");

        if (hilow_rom_traps.v) zxvision_print_string_defaults_fillspc(ventana,1,1,"You must insert a raw file");

        if (hilow_cinta_insertada_flag.v==0) zxvision_print_string_defaults_fillspc(ventana,1,1,"Tape is not inserted");

        zxvision_draw_window_contents(ventana);


		menu_add_item_menu_inicial_format(&array_menu_visual_hilow,MENU_OPCION_NORMAL,menu_visual_hilow_slow_movement,NULL
            ,"[%c] ~~Slow movement",(hilow_visual_slow_movement ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_visual_hilow,'s');
		menu_add_item_menu_ayuda(array_menu_visual_hilow,"Slow movement");
		menu_add_item_menu_tabulado(array_menu_visual_hilow,1,0);


		//Nombre de ventana solo aparece en el caso de stdout
		retorno_menu=menu_dibuja_menu_no_title_lang(&visualhilow_opcion_seleccionada,&item_seleccionado,array_menu_visual_hilow,"Visual Hilow Datadrive" );

		if (retorno_menu!=MENU_RETORNO_BACKGROUND) {
            //En caso de menus tabulados, es responsabilidad de este de borrar la ventana
            //Con este cls provoca que se borren todas las otras ventanas en background


            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    if (item_seleccionado.menu_funcion!=NULL) {
                            //printf ("actuamos por funcion\n");
                            item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);


                    }
            }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus && retorno_menu!=MENU_RETORNO_BACKGROUND);





	util_add_window_geometry_compact(ventana);

	if (retorno_menu==MENU_RETORNO_BACKGROUND) {
		zxvision_message_put_window_background();
	}

	else {

		zxvision_destroy_window(ventana);
	}


}






void menu_hilow(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_hilow;
    menu_item item_seleccionado;
    int retorno_menu;
    do {


        char string_hilow_file_shown[17];

        menu_tape_settings_trunc_name(hilow_file_name,string_hilow_file_shown,17);
        menu_add_item_menu_en_es_ca_inicial(&array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_file,NULL,
            "HiLow ~~File","~~Fichero HiLow","~~Fitxer HiLow");
        menu_add_item_menu_sufijo_format(array_menu_hilow," [%s]",string_hilow_file_shown);
        menu_add_item_menu_prefijo(array_menu_hilow,"    ");
        menu_add_item_menu_shortcut(array_menu_hilow,'f');
        menu_add_item_menu_tooltip(array_menu_hilow,"HiLow Data Drive Emulation file");
        menu_add_item_menu_ayuda(array_menu_hilow,"HiLow Data Drive Emulation file");



        menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_emulation,menu_storage_hilow_emulation_cond,
            "~~HiLow Enabled","~~HiLow Activado","~~HiLow Activat");
        menu_add_item_menu_prefijo_format(array_menu_hilow,"[%c] ", (hilow_enabled.v ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_hilow,'h');
        menu_add_item_menu_tooltip(array_menu_hilow,"Enable hilow");
        menu_add_item_menu_ayuda(array_menu_hilow,"Enable hilow");



        menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_write_protect,NULL,
            "Wr~~ite protect","Protección escr~~itura","Protecció escr~~iptura");
        menu_add_item_menu_prefijo_format(array_menu_hilow,"[%c] ", (hilow_write_protection.v ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_hilow,'i');
        menu_add_item_menu_tooltip(array_menu_hilow,"If hilow disk is write protected");
        menu_add_item_menu_ayuda(array_menu_hilow,"If hilow disk is write protected");


        menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_persistent_writes,NULL,
            "~~Persistent Writes","Escrituras ~~Persistentes","Escriptures ~~Persistents");
        menu_add_item_menu_prefijo_format(array_menu_hilow,"[%c] ",(hilow_persistent_writes.v ? 'X' : ' ') );
        menu_add_item_menu_shortcut(array_menu_hilow,'p');
        menu_add_item_menu_tooltip(array_menu_hilow,"Tells if hilow writes are saved to disk");
        menu_add_item_menu_ayuda(array_menu_hilow,"Tells if hilow writes are saved to disk. "
            "Note: all writing operations to hilow are always saved to internal memory (unless you disable write permission), but this setting "
            "tells if these changes are written to disk or not."
        );

        if (hilow_rom_traps.v==0) {
            menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_hear_load,NULL,
                "Hear load sound","Escuchar sonido de carga","Escoltar so de càrrega");
            menu_add_item_menu_prefijo_format(array_menu_hilow,"[%c] ",(hilow_hear_load_sound.v ? 'X' : ' ') );

            menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_hear_save,NULL,
                "Hear save sound","Escuchar sonido de grabación","Escoltar so de gravació");
            menu_add_item_menu_prefijo_format(array_menu_hilow,"[%c] ",(hilow_hear_save_sound.v ? 'X' : ' ') );



            menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_diffencial_algorithm,NULL,
                "Differential algorithm","Algoritmo diferencial","Algoritme diferencial");
            menu_add_item_menu_prefijo_format(array_menu_hilow,"[%c] ",(hilow_diffencial_algorithm_enabled.v ? 'X' : ' ') );
            menu_add_item_menu_es_avanzado(array_menu_hilow);

            if (hilow_diffencial_algorithm_enabled.v) {
                menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_diffencial_algorithm_volume_range,NULL,
                    "Volume range","Rango volumen","Rang volum");
                menu_add_item_menu_prefijo_format(array_menu_hilow,"    ");
                menu_add_item_menu_sufijo_format(array_menu_hilow," [%d]",hilow_diffencial_algorithm_volume_range);
                menu_add_item_menu_es_avanzado(array_menu_hilow);
            }

            menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_invert_bit,NULL,
                "Invert signal","Invertir señal","Invertir senyal");
            menu_add_item_menu_prefijo_format(array_menu_hilow,"[%c] ",(hilow_invert_bit.v ? 'X' : ' ') );
            menu_add_item_menu_es_avanzado(array_menu_hilow);

        }


        menu_add_item_menu_separator(array_menu_hilow);

        if (hilow_rom_traps.v==0) {
            menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_encendido,NULL,
                "Tape player powered on","Reproductor encendido","Reproductor activat");
            menu_add_item_menu_prefijo_format(array_menu_hilow,"[%c] ", (hilow_reproductor_encendido.v ? 'X' : ' '));
            menu_add_item_menu_tooltip(array_menu_hilow,"If tape player is powered on or not");
            menu_add_item_menu_ayuda(array_menu_hilow,"If tape player is powered on or not");
            menu_add_item_menu_es_avanzado(array_menu_hilow);

            menu_add_item_menu_separator(array_menu_hilow);
            menu_add_item_menu_es_avanzado(array_menu_hilow);
        }

        menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_insert,NULL,
            "Tape inserted","Cinta insertada","Cinta insertada");
        menu_add_item_menu_prefijo_format(array_menu_hilow,"[%c] ", (hilow_cinta_insertada_flag.v ? 'X' : ' '));
        menu_add_item_menu_tooltip(array_menu_hilow,"This flag is only read by the ROM, tells the tape is inserted or not");
        menu_add_item_menu_ayuda(array_menu_hilow,"This flag is only read by the ROM, tells the tape is inserted or not");

        menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_cover,NULL,
            "Cover has been ~~opened flag","Flag de tapa se ha abiert~~o","Flag de tapa s'ha ~~obert");
        menu_add_item_menu_prefijo_format(array_menu_hilow,"[%c] ", (hilow_tapa_has_been_opened.v ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_hilow,'o');
        menu_add_item_menu_tooltip(array_menu_hilow,"This flag is only read by the ROM, tells the cover tape has been opened (but is not necessarily open now)");
        menu_add_item_menu_ayuda(array_menu_hilow,"This flag is only read by the ROM, tells the cover tape has been opened (but is not necessarily open now). "
                                "Is is automatically reset when reading or writing. "
                                "You can set it to force reading the directory sector when doing a SAVE \"CAT\" for example");
        menu_add_item_menu_es_avanzado(array_menu_hilow);


        menu_add_item_menu_separator(array_menu_hilow);

        if (hilow_rom_traps.v) {
            menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_format,menu_storage_hilow_enabled_cond,
                "Fo~~rmat","Fo~~rmatear","Fo~~rmatejar");
            menu_add_item_menu_prefijo(array_menu_hilow,"    ");
            menu_add_item_menu_shortcut(array_menu_hilow,'r');
            menu_add_item_menu_se_cerrara(array_menu_hilow);
            menu_add_item_menu_genera_ventana(array_menu_hilow);

            menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_browser,menu_storage_hilow_enabled_cond,
                "~~Browse","~~Browse","~~Browse");
            menu_add_item_menu_prefijo(array_menu_hilow,"    ");
            menu_add_item_menu_shortcut(array_menu_hilow,'b');
            menu_add_item_menu_se_cerrara(array_menu_hilow);
            menu_add_item_menu_genera_ventana(array_menu_hilow);

            menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_storage_hilow_chkdsk,menu_storage_hilow_enabled_cond,
                "~~Chkdsk","~~Chkdsk","~~Chkdsk");
            menu_add_item_menu_prefijo(array_menu_hilow,"    ");
            menu_add_item_menu_shortcut(array_menu_hilow,'c');
            menu_add_item_menu_se_cerrara(array_menu_hilow);
            menu_add_item_menu_genera_ventana(array_menu_hilow);


            menu_add_item_menu_separator(array_menu_hilow);
        }

        else {
            menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_hilow_visual_datadrive,NULL,
                "Visual Data Drive","Visual Data Drive","Visual Data Drive");
            menu_add_item_menu_prefijo(array_menu_hilow,"    ");
            menu_add_item_menu_se_cerrara(array_menu_hilow);
            menu_add_item_menu_genera_ventana(array_menu_hilow);
        }


#ifdef USE_PTHREADS
        menu_add_item_menu_en_es_ca(array_menu_hilow,MENU_OPCION_NORMAL,menu_hilow_convert_audio,NULL,
            "Convert Audio","Convertir Audio","Convertir Audio");
        menu_add_item_menu_prefijo(array_menu_hilow,"    ");
        menu_add_item_menu_se_cerrara(array_menu_hilow);
        menu_add_item_menu_genera_ventana(array_menu_hilow);

        menu_add_item_menu_separator(array_menu_hilow);
#endif

        menu_add_ESC_item(array_menu_hilow);

        retorno_menu=menu_dibuja_menu_no_title_lang(&hilow_opcion_seleccionada,&item_seleccionado,array_menu_hilow,"HiLow Data Drive" );


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}




void menu_hardware_hilow_barbanegra_enable(MENU_ITEM_PARAMETERS)
{
    if (hilow_bbn_enabled.v) {
        hilow_bbn_disable();
    }
    else {
        hilow_bbn_enable();
    }
}

void menu_hilow_barbanegra(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    do {



        menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,menu_hardware_hilow_barbanegra_enable,
                NULL,"[%c] ~~HiLow Barbanegra Enabled", (hilow_bbn_enabled.v ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_common,'h');
        menu_add_item_menu_tooltip(array_menu_common,"Enable HiLow barbanegra");
        menu_add_item_menu_ayuda(array_menu_common,"Enable HiLow barbanegra");


        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu_no_title_lang(&hilow_barbanegra_opcion_seleccionada,&item_seleccionado,array_menu_common,"HiLow Barbanegra emulation");


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



void menu_storage_microdrive_file(MENU_ITEM_PARAMETERS)
{

    int microdrive_seleccionado=valor_opcion;

	microdrive_eject(microdrive_seleccionado);

        char *filtros[3];

        filtros[0]="rmd";
        filtros[1]="mdr";
		filtros[2]=0;



	   //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

              //Obtenemos directorio de trd
        //si no hay directorio, vamos a rutas predefinidas
        if (microdrive_status[microdrive_seleccionado].microdrive_file_name[0]==0) menu_chdir_sharedfiles();

        else {
                char directorio[PATH_MAX];
                util_get_dir(microdrive_status[microdrive_seleccionado].microdrive_file_name,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        zvfs_chdir(directorio);
                }
        }



		int ret=menu_filesel("Select MDR File",filtros,microdrive_status[microdrive_seleccionado].microdrive_file_name);
		//volvemos a directorio inicial
        zvfs_chdir(directorio_actual);


        if (ret==1) {

            if (!si_existe_archivo(microdrive_status[microdrive_seleccionado].microdrive_file_name)) {
                if (menu_confirm_yesno_texto("File does not exist","Create?")==0) {
                    microdrive_status[microdrive_seleccionado].microdrive_file_name[0]=0;
                    return;
                }

				if (!util_compare_file_extension(microdrive_status[microdrive_seleccionado].microdrive_file_name,"mdr")) {


					int total_sectors=MDR_MAX_SECTORS;

					//Si se sale con Cancel o se pone valor incorrecto
					if (menu_ventana_scanf_numero_enhanced("Total Sectors?",&total_sectors,4,+1,1,MDR_MAX_SECTORS,0)<0 || if_pending_error_message) {
						microdrive_status[microdrive_seleccionado].microdrive_file_name[0]=0;
						return;
					}

					//Crear archivo vacio
					FILE *ptr_mdrfile;
					ptr_mdrfile=fopen(microdrive_status[microdrive_seleccionado].microdrive_file_name,"wb");

					int totalsize=total_sectors*MDR_BYTES_PER_SECTOR+1; //+1 del byte final de write protect
					z80_byte valor_grabar=0;

					if (ptr_mdrfile!=NULL) {
						while (totalsize) {
							fwrite(&valor_grabar,1,1,ptr_mdrfile);
							totalsize--;
						}
						fclose(ptr_mdrfile);
					}

				}

				else {
					//Para raw
					int total_size=MICRODRIVE_RAW_COMMON_SIZE;

					//Si se sale con Cancel o se pone valor incorrecto
					if (menu_ventana_scanf_numero_enhanced("Total Size?",&total_size,7,+1,1,1000000,0)<0 || if_pending_error_message) {
						microdrive_status[microdrive_seleccionado].microdrive_file_name[0]=0;
						return;
					}

					//Espacio en bytes *2
					total_size *=2;

					//Crear archivo vacio
					FILE *ptr_mdrfile;
					ptr_mdrfile=fopen(microdrive_status[microdrive_seleccionado].microdrive_file_name,"wb");

					//Escribir cabecera
					z80_byte header[MICRODRIVE_RAW_HEADER_SIZE];

					microdrive_raw_create_header(header,0);

					fwrite(header,1,MICRODRIVE_RAW_HEADER_SIZE,ptr_mdrfile);



					z80_byte valor_grabar=0;

					if (ptr_mdrfile!=NULL) {
						while (total_size) {
							fwrite(&valor_grabar,1,1,ptr_mdrfile);
							total_size--;
						}
						fclose(ptr_mdrfile);
					}

				}

            }

            microdrive_insert(microdrive_seleccionado);

		}



        //Sale con ESC
        else {
                //Quitar nombre
                microdrive_status[microdrive_seleccionado].microdrive_file_name[0]=0;


        }
}

void menu_storage_microdrive_enable(MENU_ITEM_PARAMETERS)
{

    int microdrive_seleccionado=valor_opcion;

    if (microdrive_status[microdrive_seleccionado].microdrive_enabled) {
        microdrive_eject(microdrive_seleccionado);
    }
    else {
        microdrive_insert(microdrive_seleccionado);
    }

}



void menu_storage_microdrive_write_protection(MENU_ITEM_PARAMETERS)
{
    microdrive_switch_write_protection(valor_opcion);
}

void menu_storage_microdrive_persistent_writes(MENU_ITEM_PARAMETERS)
{
    microdrive_status[valor_opcion].microdrive_persistent_writes ^=1;
}

int menu_interface1_cond_disabled(void)
{
    return 0;
}

void menu_mdv_simulate_bad_change(MENU_ITEM_PARAMETERS)
{
    int microdrive_seleccionado=valor_opcion & 0xFF;
    int sector=valor_opcion >> 8;

    //printf("MDV: %d sector: %d\n",microdrive_seleccionado,sector);

    int opcion=menu_simple_two_choices("Bad Sector","Do you want to:","Remove from list","No change");

    switch(opcion) {
        case 1:
            microdrive_status[microdrive_seleccionado].bad_sectors_simulated[sector]=0;
        break;

    }
}

void menu_mdv_simulate_raw_bad_change(MENU_ITEM_PARAMETERS)
{
    int microdrive_seleccionado=valor_opcion & 0xFF;
    int posicion=valor_opcion >> 8;

    //printf("MDV: %d sector: %d\n",microdrive_seleccionado,sector);

    int opcion=menu_simple_two_choices("Bad Position","Do you want to:","Remove from list","No change");

    switch(opcion) {
        case 1:
			microdrive_raw_unmark_bad_position(microdrive_seleccionado,posicion);
        break;

    }
}

int menu_mdv_simulate_bad_add_last_position=0;

void menu_mdv_simulate_bad_add(MENU_ITEM_PARAMETERS)
{
    //valor=microdrive_seleccionado+sector*256;
    int microdrive_seleccionado=valor_opcion;

    //printf("MDV: %d\n",microdrive_seleccionado);

	if (microdrive_status[microdrive_seleccionado].raw_format) {
		int max_valor=microdrive_status[microdrive_seleccionado].raw_total_size-1;


		if (menu_ventana_scanf_numero_enhanced("Position?",&menu_mdv_simulate_bad_add_last_position,7,+1,0,max_valor,0)>=0) {
		    microdrive_raw_mark_bad_position(microdrive_seleccionado,menu_mdv_simulate_bad_add_last_position);

            //Indicar siguiente posicion por si se vuelve a agregar otro
            if (menu_mdv_simulate_bad_add_last_position<max_valor) menu_mdv_simulate_bad_add_last_position++;

            //para que el cursor se situe en add bad sector
            menu_mdv_simulate_bad_opcion_seleccionada++;
        }


	}

	else {

		int max_valor=microdrive_status[microdrive_seleccionado].mdr_total_sectors-1;

		int sector=0;

		if (menu_ventana_scanf_numero_enhanced("Sector?",&sector,4,+1,0,max_valor,0)>=0) {
		    microdrive_status[microdrive_seleccionado].bad_sectors_simulated[sector]=1;
        }

	}

}

void menu_mdv_simulate_bad(MENU_ITEM_PARAMETERS)
{

    int microdrive_seleccionado=valor_opcion;

    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

    //int opcion_seleccionada=1;


    do {

        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);



        int i;

		if (microdrive_status[microdrive_seleccionado].raw_format) {
			z80_int *puntero=microdrive_status[microdrive_seleccionado].raw_microdrive_buffer;
			for (i=0;i<microdrive_status[microdrive_seleccionado].raw_total_size;i++) {

				if (puntero[i] & MICRODRIVE_RAW_INFO_BYTE_MASK_BAD_POSITION) {

					menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_mdv_simulate_raw_bad_change,NULL,"Position %d",i);

					//Codificar sector y microdrive seleccionado como:
					//valor=microdrive_seleccionado+sector*256;
					menu_add_item_menu_valor_opcion(array_menu_common,microdrive_seleccionado+i*256);
				}



			}
		}
		else {
			for (i=0;i<MDR_MAX_SECTORS;i++) {

				if (microdrive_status[microdrive_seleccionado].bad_sectors_simulated[i]) {

					menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_mdv_simulate_bad_change,NULL,"Sector %d",i);

					//Codificar sector y microdrive seleccionado como:
					//valor=microdrive_seleccionado+sector*256;
					menu_add_item_menu_valor_opcion(array_menu_common,microdrive_seleccionado+i*256);
				}



			}
		}

		if (microdrive_status[microdrive_seleccionado].raw_format) {
			menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_mdv_simulate_bad_add,NULL,"Add bad position");
		}
		else {
        	menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_mdv_simulate_bad_add,NULL,"Add bad sector");
		}
        menu_add_item_menu_valor_opcion(array_menu_common,microdrive_seleccionado);


        menu_add_item_menu_separator(array_menu_common);


        menu_add_ESC_item(array_menu_common);



        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&menu_mdv_simulate_bad_opcion_seleccionada,&item_seleccionado,array_menu_common,
            "Emulate bad sectors");

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}





void menu_storage_microdrive_map(MENU_ITEM_PARAMETERS)
{
    //para empezar de nuevo cuando se renombra un archivo
    int recargar_microdrive;

    //Si vale -1, mapear todos
    //Si es >=0, mapear uno solo
    int buscar_archivo=-1;

    do {

        recargar_microdrive=0;

        struct s_mdr_file_cat *catalogo;

        catalogo=mdr_get_file_catalogue(microdrive_status[valor_opcion].if1_microdrive_buffer,microdrive_status[valor_opcion].mdr_total_sectors);


        mdr_chkdsk_get_checksums(catalogo,
            microdrive_status[valor_opcion].if1_microdrive_buffer,microdrive_status[valor_opcion].mdr_total_sectors);

        //printf("Nuevo catalogo\n");

        //printf("Label: [%s]\n",catalogo->label);

        int i;

        /*for (i=0;i<catalogo->total_files;i++) {
            printf("%d [%s] size: %d\n",i,catalogo->file[i].name,catalogo->file[i].file_size);

            //bloques
            int j;

            for (j=0;j<catalogo->file[i].total_sectors;j++) {
                printf("%d ",catalogo->file[i].sectors_list[j]);
            }
            printf("\n");
        }*/


        int ancho=43;
        int alto=27;
        int xventana=menu_center_x()-ancho/2;
        int yventana=menu_center_y()-alto/2;

        zxvision_window ventana;

        zxvision_new_window(&ventana,xventana,yventana,ancho,alto,
                                                ancho-1,alto-2,"Microdrive Map");

        int salir=0;



        do {

            //Por defecto mapa total, pero luego se puede ver mapa de cada archivo por separado
            //Que tambien diga fragmentacion archivos

            //Indicar la letra del sector (Used, used y final , X defectuoso, "." sin uso
            char letras_sectores[MDR_MAX_SECTORS];
            //inicializar con "."
            //y los que sean erroneos
            for (i=0;i<microdrive_status[valor_opcion].mdr_total_sectors;i++) {
                char letra='.';

                if (microdrive_status[valor_opcion].bad_sectors_simulated[i]) letra='X';

                letras_sectores[i]=letra;
            }


            int inicio_busqueda=0;
            int final_busqueda=catalogo->total_files;

            if (buscar_archivo>=0) {
                inicio_busqueda=buscar_archivo;
                final_busqueda=inicio_busqueda+1;
            }

            int used_sectors=0;

            //Buscar todos archivos
            //o buscar solo uno concreto
            for (i=inicio_busqueda;i<final_busqueda;i++) {
                //bloques
                int j;

                for (j=0;j<catalogo->file[i].total_sectors;j++) {
                    int sector_usado=catalogo->file[i].sectors_list[j];

                    if (sector_usado>=0) {
                        //printf("%d ",sector_usado);
                        char letra='U';
                        if (j==catalogo->file[i].total_sectors-1) letra='u';

                        letras_sectores[sector_usado]=letra;

                    }

                    used_sectors++;
                }
            }



            int linea=0;

            int sectores_por_linea=32;
            int x=0;

            //char buffer_linea[MAX_ANCHO_LINEAS_GENERIC_MESSAGE+1]="";

            for (i=0;i<microdrive_status[valor_opcion].mdr_total_sectors;i++) {
                char caracter_info=letras_sectores[i];

                int tinta=ESTILO_GUI_TINTA_NORMAL;
                int papel=ESTILO_GUI_PAPEL_NORMAL;

                //Y bad checksum
                if (catalogo->hd_chk[i]!=catalogo->calculated_hd_chk[i] ||
                    catalogo->des_chk[i]!=catalogo->calculated_des_chk[i] ||
                    catalogo->data_chk[i]!=catalogo->calculated_data_chk[i]) {

                    //Si es sector sin uso, puede indicar archivo borrado
                    //Nota: la operación de "undelete" no tiene sentido en microdrive,
                    //porque cuando se borra un archivo se sobreescribe la sección data de los sectores
                    if (caracter_info=='.') {
                        if (buscar_archivo==-1) {
                            caracter_info='-';
                        }
                    }

                    else tinta=ESTILO_GUI_COLOR_AVISO;
                }


                zxvision_print_string_format(&ventana,x+1,linea,tinta,papel,0,"%c",caracter_info);

                x++;

                if (x==sectores_por_linea || i==microdrive_status[valor_opcion].mdr_total_sectors-1) {
                    x=0;
                    linea++;
                }

            }

            //Forzar a mostrar atajos
            z80_bit antes_menu_writing_inverse_color;
            antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
            menu_writing_inverse_color.v=1;

            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Legend:");
            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"U: Used sector");
            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"u: Used sector and final of a file");
            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"X: Bad sector");
            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,".: Unused sector");
            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"-: Unused sector & possible deleted");
            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Coloured Letter: Bad checksum on sector");

            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Microdrive Info:");
            int total_kb=microdrive_status[valor_opcion].mdr_total_sectors*512/1024;

            zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,
                "Label: %s",catalogo->label);

            zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,
                "Total %d KB (%d sectors)",total_kb,microdrive_status[valor_opcion].mdr_total_sectors);

            if (buscar_archivo>=0) {
                zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
                zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"File Info:");

                char buffer_copias[30]="";
                int copias=catalogo->file[buscar_archivo].numero_copias;
                if (copias>1) sprintf(buffer_copias," (%d copies)",copias);

                char buf_nombre[11];
                mdr_get_file_name_escaped(catalogo->file[buscar_archivo].name,buf_nombre);

                zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,
                    "File %3d/%3d Name: %s%s",buscar_archivo+1,catalogo->total_files,buf_nombre,buffer_copias);
            }
            else {
                //zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Microdrive Info:");
            }


            int used_kb=used_sectors/2; //*512/1024

            //Si es medio sector, o 1.5 etc
            int medio=0;
            if (used_sectors % 2 !=0) medio=1;

            int tamanyo_archivo=catalogo->file[buscar_archivo].file_size;

            if (buscar_archivo>=0) {
                zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,
                    "Size: %5d B Used %d%s KB First sect: %d",tamanyo_archivo,used_kb,(medio ? ".5" : ""),
                        catalogo->file[buscar_archivo].sectors_list[0]);
            }
            else {
                zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,
                    "Used %d%s KB",used_kb,(medio ? ".5" : ""));
            }


            if (buscar_archivo>=0) {
                zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,
                        "Fragmentation: %d %%",catalogo->file[buscar_archivo].porcentaje_fragmentacion);
            }
            else {
                zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,
                        "Fragmentation: %d %%",catalogo->porcentaje_fragmentacion);

                //lineas mas en blanco para que ocupe la ventana lo mismo que cuando hace file info y no cambie el tamaño de ventana
                zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
                zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
                zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
            }

            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Use cursors ~~< ~~> to show files info");
            if (buscar_archivo>=0) {
                zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"~~r: Rename file");
            }
            else {
                zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
            }

            //Restaurar comportamiento atajos
            menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

            //Ajustar al final de la leyenda
            zxvision_set_visible_height(&ventana,linea+2);
            zxvision_set_total_height(&ventana,linea);

            //Recalcular centro
            yventana=menu_center_y()-ventana.visible_height/2;

            zxvision_set_y_position(&ventana,yventana);

            zxvision_draw_window(&ventana);
            zxvision_draw_window_contents(&ventana);

            //zxvision_wait_until_esc(&ventana);

            z80_byte tecla=zxvision_common_getkey_refresh();


            switch (tecla) {

                case 8:
                    //izquierda
                    if (buscar_archivo>-1) buscar_archivo--;
                break;

                case 9:
                    //derecha
                    if (buscar_archivo<catalogo->total_files-1) buscar_archivo++;
                break;

                //rename
                case 'r':
                    if (buscar_archivo>=0) {
                        char buffer_nombre[11];

                        mdr_get_file_name_escaped(catalogo->file[buscar_archivo].name,buffer_nombre);

                        mdr_truncate_spaces_name(buffer_nombre);

                        int retorno=menu_ventana_scanf("New name",buffer_nombre,11);

                        if (retorno>=0) {
                            mdr_rename_file(catalogo,microdrive_status[valor_opcion].if1_microdrive_buffer,buscar_archivo,buffer_nombre);

                            //Y hacer flush
                            microdrive_status[valor_opcion].microdrive_must_flush_to_disk=1;

                            menu_generic_message_splash("Rename file","OK. File has been renamed");
                        }

                        salir=1;
                        recargar_microdrive=1;
                    }
                break;

                //Salir con ESC
                case 2:
                    salir=1;
                break;

                //O tecla background
                case 3:
                    salir=1;
                break;
            }

        } while (!salir);

        zxvision_destroy_window(&ventana);

        free(catalogo);

    } while (recargar_microdrive);
}



void menu_storage_microdrive_chkdsk(MENU_ITEM_PARAMETERS)
{
    struct s_mdr_file_cat *catalogo;

    catalogo=mdr_get_file_catalogue(microdrive_status[valor_opcion].if1_microdrive_buffer,microdrive_status[valor_opcion].mdr_total_sectors);

    //printf("Nuevo catalogo\n");

    //printf("Label: [%s]\n",catalogo->label);

    int i;




    int ancho=46;
    int alto=25;
    int xventana=menu_center_x()-ancho/2;
    int yventana=menu_center_y()-alto/2;

    zxvision_window ventana;

    zxvision_new_window(&ventana,xventana,yventana,ancho,alto,
                                            ancho-1,alto-2,"Microdrive Chkdsk");



    int linea=0;

    //Buscar archivos duplicados
    int archivos_con_copias=0;

    int total_extra_copias=0;

    //Array de archivos duplicados para saber si un id esta en uso o no
    int ids_duplicados[MDR_MAX_SECTORS];

    //Inicialmente no hay ninguno en uso
    for (i=0;i<MDR_MAX_SECTORS;i++) ids_duplicados[i]=0;

    //Contar copias, archivos que le faltan bloques, etc
    int archivos_faltan_bloques=0;

    //margen mas que suficiente
    char buf_archivos_faltan_bloques[130]="";

    for (i=0;i<catalogo->total_files;i++) {
        if (catalogo->file[i].faltan_bloques) {
            archivos_faltan_bloques++;

            //meter nombre de archivos en una lista. Limite 100 caracteres o ancho de ventana
            int longitud_texto=strlen(buf_archivos_faltan_bloques);
            if (longitud_texto<100 && longitud_texto<ancho) {
                char nombre_escapado[11];

                mdr_get_file_name_escaped(catalogo->file[i].name,nombre_escapado);

                mdr_truncate_spaces_name(nombre_escapado);

                char buf_linea[20];
                sprintf(buf_linea,"%s,",nombre_escapado);

                //Maximo 111 de 100+11
                util_concat_string(buf_archivos_faltan_bloques,buf_linea,111);
            }
        }

        if (catalogo->file[i].numero_copias>1) {
            //Si este no lo hemos contado ya

            int id_file=catalogo->file[i].id_file;
            if (ids_duplicados[id_file]==0) {

                ids_duplicados[id_file]=1;

                total_extra_copias+=catalogo->file[i].numero_copias-1; //si son dos copias, solo contamos una copia extra
            }
        }
    }

    //Ver si hay bloques no asignados a archivos
    //int archivos_sin_bloque_zero;
    mdr_chkdsk_get_files_no_block_zero(catalogo,
        microdrive_status[valor_opcion].if1_microdrive_buffer,microdrive_status[valor_opcion].mdr_total_sectors);

    //Calcular checksums
    int error_checksum_header=0;
    int error_checksum_descriptor=0;
    int error_checksum_data=0;
    mdr_chkdsk_get_checksums(catalogo,
        microdrive_status[valor_opcion].if1_microdrive_buffer,microdrive_status[valor_opcion].mdr_total_sectors);

    //margen mas que suficiente
    char buf_error_checksum_header[130]="";
    char buf_error_checksum_descriptor[130]="";
    char buf_error_checksum_data[130]="";

    for (i=0;i<MDR_MAX_SECTORS;i++) {
        if (catalogo->hd_chk[i]!=catalogo->calculated_hd_chk[i]) {
            error_checksum_header++;

            //meter nombre de archivos en una lista. Limite 100 caracteres o ancho de ventana
            int longitud_texto=strlen(buf_error_checksum_header);
            if (longitud_texto<100 && longitud_texto<ancho) {
                char buf_linea[20];
                sprintf(buf_linea,"%d,",i);

                //Maximo 111 de 100+11
                util_concat_string(buf_error_checksum_header,buf_linea,111);
            }
        }

        if (catalogo->des_chk[i]!=catalogo->calculated_des_chk[i]) {
            error_checksum_descriptor++;

            //meter nombre de archivos en una lista. Limite 100 caracteres o ancho de ventana
            int longitud_texto=strlen(buf_error_checksum_descriptor);
            if (longitud_texto<100 && longitud_texto<ancho) {
                char buf_linea[20];
                sprintf(buf_linea,"%d,",i);

                //Maximo 111 de 100+11
                util_concat_string(buf_error_checksum_descriptor,buf_linea,111);
            }
        }

        //Sectores de archivos borrados parecen tener mal el data checksum
        //por eso solo indicar error de ese checksum si es un sector en uso
        if (catalogo->data_chk[i]!=catalogo->calculated_data_chk[i] && catalogo->used_sectors_list[i]) {
            error_checksum_data++;

            //meter nombre de archivos en una lista. Limite 100 caracteres o ancho de ventana
            int longitud_texto=strlen(buf_error_checksum_data);
            if (longitud_texto<100 && longitud_texto<ancho) {
                char buf_linea[20];
                sprintf(buf_linea,"%d,",i);

                //Maximo 111 de 100+11
                util_concat_string(buf_error_checksum_data,buf_linea,111);
            }
        }

    }

    //Contar cuantos archivos tienen mas de una copia
    for (i=0;i<MDR_MAX_SECTORS;i++) {
        if (ids_duplicados[i]) archivos_con_copias++;
    }

    zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Info:");
    zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Label: %s",catalogo->label);
    zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Total sectors:                           %3d",microdrive_status[valor_opcion].mdr_total_sectors);
    zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Used sectors:                            %3d",catalogo->used_sectors);
    zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Unique files with more than 1 copy:      %3d",archivos_con_copias);
    zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Total extra file copies:                 %3d",total_extra_copias);

    zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
    zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Problems:");

    int problemas_detectados=0;

    if (archivos_faltan_bloques) {
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Files with missing blocks:               %3d",archivos_faltan_bloques);
        problemas_detectados=1;

        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++," Files: %s",buf_archivos_faltan_bloques);
    }

    if (catalogo->chkdsk_total_files_sin_bloque_zero) {
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Lost blocks missing block zero:          %3d",catalogo->chkdsk_total_files_sin_bloque_zero);
        problemas_detectados=1;

        zxvision_print_string_defaults(&ventana,1,linea," Sectors: ");

        int x=11; //posicionar despues de "Sectors:"

        char buffer_numero[5]; //3 digitos numero, coma, 0 del final

        //hasta ancho-5 para dar un margen de escribir los "..."
        for (i=0;i<catalogo->chkdsk_total_files_sin_bloque_zero && x<ancho-5;i++) {
            sprintf(buffer_numero,"%d,",catalogo->chkdsk_files_sin_bloque_zero_sectors[i]);

            zxvision_print_string_defaults(&ventana,x,linea,buffer_numero);
            int longitud=strlen(buffer_numero);
            x +=longitud;
        }

        //Si no ha acabado de listarlos todos, meter "..."
        if (i!=catalogo->chkdsk_total_files_sin_bloque_zero) zxvision_print_string_defaults(&ventana,x,linea,"...");
        else {
            //quitar la coma del final
            zxvision_print_string_defaults(&ventana,x-1,linea," ");
        }
        linea++;

    }

    if (error_checksum_header) {
        problemas_detectados=1;
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Invalid header checksums");
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++," Sectors: %s",buf_error_checksum_header);
    }

    if (error_checksum_descriptor) {
        problemas_detectados=1;
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Invalid descriptor checksums");
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++," Sectors: %s",buf_error_checksum_descriptor);
    }

    if (error_checksum_data) {
        problemas_detectados=1;
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Invalid data checksums");
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++," Sectors: %s",buf_error_checksum_data);
    }


    if (!problemas_detectados) zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"No problems detected");
    else {
        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Note: problems are only shown, but not fixed");
    }

    //Ajustar alto ventana al maximo necesario (donde haya lineas escritas)
    zxvision_resize_minimum_height(&ventana);

    //Recalcular centro
    int y=menu_center_y()-ventana.visible_height/2;
    zxvision_set_y_position(&ventana,y);


    zxvision_draw_window(&ventana);
    zxvision_draw_window_contents(&ventana);

    zxvision_wait_until_esc(&ventana);



    zxvision_destroy_window(&ventana);

    free(catalogo);
}

z80_byte menu_strg_mdv_sec_info_get_byte(z80_byte *puntero,int sector,int sector_offset)
{

    int offset=sector*MDR_BYTES_PER_SECTOR;

    offset +=sector_offset;

    return puntero[offset];
}

int mdv_sectors_info_current_sector=0;
int mdv_sectors_info_last_microdrive_seleccionado=0;

void menu_storage_microdrive_sectors_info(MENU_ITEM_PARAMETERS)
{


    int ancho=48;
    int alto=31;
    int xventana=menu_center_x()-ancho/2;
    int yventana=menu_center_y()-alto/2;

    zxvision_window ventana;

    zxvision_new_window(&ventana,xventana,yventana,ancho,alto,
                                            ancho-1,alto-2,"Sectors info");




    int microdrive_seleccionado=valor_opcion;


    zxvision_draw_window(&ventana);

    int salir=0;

    int abrir_hex_editor=0;

    //Si microdrive seleccionado no llega al ultimo sector que hemos visualizado (porque se haya cambiado)
    //o porque microdrive_seleccionado no sea el mismo que el anterior,
    //ir a sector 0
    if (mdv_sectors_info_current_sector>=microdrive_status[microdrive_seleccionado].mdr_total_sectors ||
        mdv_sectors_info_last_microdrive_seleccionado != microdrive_seleccionado

    ) {
        mdv_sectors_info_current_sector=0;
    }

    mdv_sectors_info_last_microdrive_seleccionado=microdrive_seleccionado;

    z80_byte *origen=microdrive_status[microdrive_seleccionado].if1_microdrive_buffer;

    do {

        int linea=0;



/*
    offset length
              0      1   HDFLAG   Value 1, to indicate header block  *See note.
      1      1   HDNUMB   sector number (values 254 down to 1)
      2      2            not used (and of undetermined value)
      4     10   HDNAME   microdrive cartridge name (blank padded)
     14      1   HDCHK    header checksum (of first 14 bytes)

     15      1   RECFLG   - bit 0: always 0 to indicate record block
                          - bit 1: set for the EOF block
                          - bit 2: reset for a PRINT file
                          - bits 3-7: not used (value 0)

     16      1   RECNUM   data block sequence number (value starts at 0)
     17      2   RECLEN   data block length (<=512, LSB first)
     19     10   RECNAM   filename (blank padded)
     29      1   DESCHK   record descriptor checksum (of previous 14 bytes)
     30    512            data block
    542      1   DCHK     data block checksum (of all 512 bytes of data
                          block, even when not all bytes are used)
*/

        z80_byte hd_flg=menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,0);
        z80_byte hd_num=menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,1);

        char hd_name[11];
        int i;
        for (i=0;i<10;i++) {
            z80_byte letra=menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,4+i);
            if (letra<32 || letra>126) letra='.';
            hd_name[i]=letra;
        }

        hd_name[i]=0;


        z80_byte hd_chk=menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,14);
        z80_byte calculated_hd_chk=mdr_calculate_checksum(origen,mdv_sectors_info_current_sector,0,14);

        z80_byte data_recflg=menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,15);
        z80_byte rec_num=menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,16);
        z80_int rec_len=menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,17)+256*menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,18);

        char rec_name[11];

        for (i=0;i<10;i++) {
            z80_byte letra=menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,19+i);
            if (letra<32 || letra>126) letra='.';
            rec_name[i]=letra;
        }

        rec_name[i]=0;

        z80_byte des_chk=menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,29);
        z80_byte calculated_des_chk=mdr_calculate_checksum(origen,mdv_sectors_info_current_sector,15,14);

        z80_byte data_chk=menu_strg_mdv_sec_info_get_byte(origen,mdv_sectors_info_current_sector,542);
        z80_byte calculated_data_chk=mdr_calculate_checksum(origen,mdv_sectors_info_current_sector,30,512);

        int sector_usado=0;

        //A used record block is either an EOF block (bit 1 of RECFLG is 1) or
        //contains 512 bytes of data (RECLEN=512, i.e. bit 1 of MSB is 1).
        if (rec_len==512 || (data_recflg & 0x02)==0x02) sector_usado=1;

        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Physical Sector %3d/%3d Offset: %05XH",
            mdv_sectors_info_current_sector,microdrive_status[microdrive_seleccionado].mdr_total_sectors-1,
            mdv_sectors_info_current_sector*MDR_BYTES_PER_SECTOR
        );

        zxvision_print_string_defaults_fillspc(&ventana,1,linea,"Sector");

        if (sector_usado) {
            //color inverso
            zxvision_print_string(&ventana,8,linea,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,0,"Used");
        }
        else {
            zxvision_print_string_defaults(&ventana,8,linea,"Unused");
        }


        if (microdrive_status[microdrive_seleccionado].bad_sectors_simulated[mdv_sectors_info_current_sector]) {
            zxvision_print_string(&ventana,15,linea,ESTILO_GUI_COLOR_AVISO,ESTILO_GUI_PAPEL_NORMAL,0,"Bad Sector");
        }

        linea++;

        int color_aviso_checksum_tinta=ESTILO_GUI_COLOR_AVISO;

        //Si es sector borrado, el checksum puede estar mal, no cambiar color
        if (!sector_usado) color_aviso_checksum_tinta=ESTILO_GUI_TINTA_NORMAL;


        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Off Name");

        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
        zxvision_print_string_defaults_fillspc(&ventana,1,linea++," ---Header--- ");
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"  0 HDFLAG: %02XH",hd_flg);
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"  1 HDNUMB: %02XH   - Sector Number   -",hd_num);
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"  4 HDNAME: %s",hd_name);
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++," 14 HDCHK:  %02XH   - Header Checksum -",hd_chk);
        //printf("calculated_hd_chk: %02XH\n",calculated_hd_chk);
        if (hd_chk!=calculated_hd_chk) {
            zxvision_print_string_format(&ventana,1,linea++,color_aviso_checksum_tinta,ESTILO_GUI_PAPEL_NORMAL,0," Error: should be: %02XH",calculated_hd_chk);
        }
        else zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
        zxvision_print_string_defaults_fillspc(&ventana,1,linea++," ---Record Descriptor--- ");
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++," 15 RECFLG: %02XH",data_recflg);
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"    Bit 0=%d (%srecord block)",
            data_recflg&1,((data_recflg&1)==0 ? "" : "no "));
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"    Bit 1=%d (%sEOF block)",
            (data_recflg&2)>>1,((data_recflg&2) ? "" : "no "));
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"    Bit 2=%d (%sPRINT/STREAM file)",
            (data_recflg&4)>>2,((data_recflg&4)==0 ? "" : "no "));

        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++," 16 RECNUM: %02XH   - data block sequence number",rec_num);
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++," 17 RECLEN: %5d - data block length",rec_len);
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++," 19 RECNAM: %s",rec_name);
        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++," 29 DESCHK: %02XH   - record descriptor checksum",des_chk);
        //printf("calculated_des_chk: %02XH\n",calculated_des_chk);
        if (des_chk!=calculated_des_chk) {
            zxvision_print_string_format(&ventana,1,linea++,color_aviso_checksum_tinta,ESTILO_GUI_PAPEL_NORMAL,0," Error: should be: %02XH",calculated_des_chk);
        }
        else zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
        zxvision_print_string_defaults_fillspc(&ventana,1,linea++," ---Data--- ");

        zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"542 DCHK:   %02XH   - data block checksum",data_chk);

        //printf("calculated_data_chk: %02XH\n",calculated_data_chk);
        if (data_chk!=calculated_data_chk) {
            zxvision_print_string_format(&ventana,1,linea++,color_aviso_checksum_tinta,ESTILO_GUI_PAPEL_NORMAL,0," Error: should be: %02XH",calculated_data_chk);
        }
        else zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");


        //Forzar a mostrar atajos
        z80_bit antes_menu_writing_inverse_color;
        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
        menu_writing_inverse_color.v=1;

        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Use cursors ~~< ~~> to change sector");
        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"~~s: jump to sector  ~~x: open hex editor");

        //Restaurar comportamiento atajos
        menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;


        zxvision_draw_window_contents(&ventana);

        z80_byte tecla=zxvision_common_getkey_refresh();

        int max_valor;

        switch (tecla) {

            case 8:
                //izquierda
                if (mdv_sectors_info_current_sector>0) mdv_sectors_info_current_sector--;
            break;

            case 9:
                //derecha
                if (mdv_sectors_info_current_sector<microdrive_status[microdrive_seleccionado].mdr_total_sectors-1) mdv_sectors_info_current_sector++;
            break;

            //Salir con ESC
            case 2:
                salir=1;
            break;

            //O tecla background
            case 3:
                salir=1;
            break;

            case 's':

                max_valor=microdrive_status[microdrive_seleccionado].mdr_total_sectors-1;

                int sector=mdv_sectors_info_current_sector;

                int retorno=menu_ventana_scanf_numero_enhanced("Sector?",&sector,4,+1,0,max_valor,0);

                //comprobar error
                if (if_pending_error_message) {
                    menu_muestra_pending_error_message(); //Si se genera un error derivado de menu_ventana_scanf_numero_enhanced
                }

                if (retorno>=0) mdv_sectors_info_current_sector=sector;


            break;

            case 'x':
                abrir_hex_editor=1;
                salir=1;
            break;

        }

    } while (!salir);

    zxvision_destroy_window(&ventana);

    if (abrir_hex_editor) {
        switch (microdrive_seleccionado) {
            case 0:
                 menu_set_memzone(MEMORY_ZONE_MDV1);
            break;

            case 1:
                 menu_set_memzone(MEMORY_ZONE_MDV2);
            break;

            case 2:
                 menu_set_memzone(MEMORY_ZONE_MDV3);
            break;

            case 3:
                 menu_set_memzone(MEMORY_ZONE_MDV4);
            break;
        }

        //Inicializar info de tamanyo zona
        menu_debug_set_memory_zone_attr();

        menu_debug_hexdump_direccion=mdv_sectors_info_current_sector*MDR_BYTES_PER_SECTOR;

        menu_debug_hexdump(0);
    }


}

void menu_storage_microdrive_browse(MENU_ITEM_PARAMETERS)
{

    int ancho=50;
    int alto=20;
    int x=menu_center_x()-ancho/2;
    int y=menu_center_y()-alto/2;

    zxvision_window ventana;

    //pueden haber tantos sectores como archivos
    int alto_total_ventana=MDR_MAX_SECTORS;

    zxvision_new_window(&ventana,x,y,ancho,alto,
                                            ancho-1,alto_total_ventana,"Microdrive Browse");



    //Manera antigua de obtener el listado
    //int linea=menu_microdrive_map_browse(&ventana,1,valor_opcion,0,NULL,0);

    int linea=0;

    struct s_mdr_file_cat *catalogo;

    int microdrive_seleccionado=valor_opcion;

    catalogo=mdr_get_file_catalogue(microdrive_status[microdrive_seleccionado].if1_microdrive_buffer,microdrive_status[microdrive_seleccionado].mdr_total_sectors);

    zxvision_print_string_defaults_fillspc_format(&ventana,1,linea++,"Label: %s",catalogo->label);


    int i;

    for (i=0;i<catalogo->total_files;i++) {
        //printf("%d [%s]\n",i,catalogo->file[i].name_extended);
        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,catalogo->file[i].name_extended);
    }

    free(catalogo);




    //Ajustar al final
    zxvision_set_visible_height(&ventana,linea+2);
    zxvision_set_total_height(&ventana,linea);

    //Recalcular centro
    y=menu_center_y()-ventana.visible_height/2;

    zxvision_set_y_position(&ventana,y);

    zxvision_draw_window(&ventana);
    zxvision_draw_window_contents(&ventana);

    zxvision_wait_until_esc(&ventana);



    zxvision_destroy_window(&ventana);
}


zxvision_window *menu_visual_microdrive_window;

//cual (mdv0, 1, 2 estamos mirando)
int menu_visual_microdrive_mirando_microdrive=0;

int visual_micro_antes_grados_rodillo=0;

int visual_microdrive_slow_movement=0;

int visual_microdrive_slow_movement_grados=0;

#define VISUAL_MICRODRIVE_COLOR_FONDO AMIGAOS_COLOUR_blue

void menu_visual_microdrive_slow_movement(MENU_ITEM_PARAMETERS)
{
    visual_microdrive_slow_movement ^=1;
}

void menu_visual_microdrive_cual(MENU_ITEM_PARAMETERS)
{
    menu_visual_microdrive_mirando_microdrive++;
    if (menu_visual_microdrive_mirando_microdrive>=MAX_MICRODRIVES_BY_CONFIG) menu_visual_microdrive_mirando_microdrive=0;
}

void visual_microdrive_get_info_rodillos_interiores(int *radio_cinta_sectores,int *radio_base)
{
    //cinta enrollada. maximo grueso=295-152=143
    int max_radio_cinta_sectores=143-10; //para que no llegue al tope

	//Si es imagen raw, obtener de otra manera
	if (microdrive_status[menu_visual_microdrive_mirando_microdrive].raw_format) {
		int size=microdrive_status[menu_visual_microdrive_mirando_microdrive].raw_total_size;
		*radio_cinta_sectores=(size*max_radio_cinta_sectores)/MICRODRIVE_RAW_COMMON_SIZE;
	}

	else {


		int numero_microdrive=menu_visual_microdrive_mirando_microdrive;

		int total_sectores=microdrive_status[numero_microdrive].mdr_total_sectors;

		//sacar radio
		*radio_cinta_sectores=(total_sectores*max_radio_cinta_sectores)/MDR_MAX_SECTORS;

	}

    //Si es un microdrive super pequeño, al menos 1 de radio
    if (*radio_cinta_sectores==0) *radio_cinta_sectores=1;
	//Controlar maximo tambien
	if (*radio_cinta_sectores>max_radio_cinta_sectores) *radio_cinta_sectores=max_radio_cinta_sectores;

    *radio_base=152;
}

void menu_visual_microdrive_dibujar_microdrive_estatico(struct zxvision_vectorial_draw *d)
{

    //Si microdrive no habilitado, no dibujarlo
    if (!microdrive_status[menu_visual_microdrive_mirando_microdrive].microdrive_enabled) return;

    d->pencil_off(d);
    d->setpos(d,0,0);

    int color_marco=7; //gris
    int color_cinta_enrollada=0; //esto sera negro

    d->setcolour(d,color_marco);
    d->pencil_on(d);

    //Marco exterior del microdrive. Parte de arriba
    d->setpos(d,170,0);
    d->setpos(d,262,84);
    d->set_x(d,414);
    d->setpos(d,506,0);
    d->setpos(d,699,0);

    //hasta pestaña proteccion escritura
    d->set_y(d,340);

    //pestaña proteccion escritura
    //Si esta protegido contra escritura, la pestaña no esta
    if (!microdrive_status[menu_visual_microdrive_mirando_microdrive].microdrive_write_protect) {
        //La pestaña tiene unos 20 de ancho
        d->drawfilledrectangle(d,-20,506-340);
    }


    d->pencil_off(d);
    d->set_y(d,506);
    d->pencil_on(d);

    //hasta abajo
    d->set_y(d,999);

    //Marco de abajo
    d->set_x(d,0);

    //Marco de izquierda
    d->set_y(d,400);
    d->setpos(d,61,225);

    d->pencil_off(d);
    d->setpos(d,0,0);
    d->pencil_on(d);
    d->set_y(d,136);






    //esponjita
    d->pencil_off(d);
    d->setpos(d,284,62);
    d->setcolour(d,6); //esponjita
    d->drawfilledrectangle(d,392-284,21);

    //circulo donde va la cinta enrollada
    d->setcolour(d,15);
    d->pencil_off(d);
    d->setpos(d,357,678);
    d->drawcircle(d,152); //donde se enrolla la cinta
    //d->drawcircle(d,295); //hasta la zona blanca

    int radio_cinta_sectores;
    int radio_base;
    visual_microdrive_get_info_rodillos_interiores(&radio_cinta_sectores,&radio_base);


    d->setcolour(d,15);

    int i;
    //Zona blanca interior
    for (i=0;i<radio_base;i++) {
        d->drawcircle(d,i);
    }


    d->setcolour(d,color_cinta_enrollada);

    //Cinta enrollada
    for (i=radio_base;i<radio_base+radio_cinta_sectores;i++) {
        d->drawcircle(d,i);
    }



    //y de ahi toda la zona blanca exterior
    d->setcolour(d,15);
    for (;i<295;i++) {
        d->drawcircle(d,i);
    }

    //cinta visible.

    d->setcolour(d,color_cinta_enrollada);
    //desde arriba del rodillo izquierdo
    d->pencil_off(d);
    d->setpos(d,108,136-100); //algo mas de radio 98
    d->pencil_on(d);
    //segundo tramo
    d->setpos(d,170,0);

    //de ahi hacia la esponjita
    //tercer tramo
    d->setpos(d,284,62);
    //cuarto tramo
    d->set_x(d,392);
    //hacia arriba
    //quinto tramo
    d->setpos(d,506,0);
    //hacia el rodillo amarillo
    //por arriba
    //sexto tramo
    d->setpos(d,573,136-100); //algo mas de radio 98



}



void menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(struct zxvision_vectorial_draw *d,
    int x_origen_rodillo,int y_origen_rodillo,int longitud,int grados,int color)
{


    d->pencil_off(d);
    d->setcolour(d,color);
    d->setpos(d,x_origen_rodillo,y_origen_rodillo);
    d->pencil_on(d);



    //sacar final linea
    int longitud_y=(longitud*util_get_sine(grados))/10000;
    //printf("longitud_y antes: %d\n",longitud_y);

    //reajustar comportamiento decimales para que se comporte como el dibujado de circulos
    //Nota: eso es debido a que el calculo de coordenadas virtuales se hace diferente al dibujar el circulo o al trazar una linea
    //y/o tambien a la falta de uso de variables con decimales para calcular
    //probablemente con el uso de float para real_radio en zxvision_vecdraw_arc se solventaria

    //Si hacemos paso de coordenadas virtuales a reales, y de vuelta a virtuales,
    //en el calculo sin decimales nos comportaremos ¿igual? que el dibujado de circulos
    //Si no hiciera esto, estas lineas de radio no están siempre exactas desde el centro al circulo,
    //a veces sobresale del circulo (cuando el dibujo es pequeño), a veces no llega a tocar al circulo (cuando el dibujo es grande)

    //pasar a dimensiones reales
    if (d->virtual_width==0) longitud_y=0;
    else longitud_y=(longitud_y*d->real_width)/d->virtual_width;

    //y de vuelta a virtuales
    if (d->real_width==0) longitud_y=0;
    else longitud_y=(longitud_y*d->virtual_width)/d->real_width;

    //printf("longitud_y despues: %d\n",longitud_y);

    int yfinal=y_origen_rodillo-longitud_y;

    int longitud_x=(longitud*util_get_cosine(grados))/10000;

    //reajustar comportamiento decimales para que se comporte como el dibujado de circulos
    //pasar a dimensiones reales
    if (d->virtual_height==0) longitud_x=0;
    else longitud_x=(longitud_x*d->real_heigth)/d->virtual_height;

    //y de vuelta a virtuales
    if (d->real_heigth==0) longitud_x=0;
    else longitud_x=(longitud_x*d->virtual_height)/d->real_heigth;


    int xfinal=x_origen_rodillo+longitud_x;


    d->setpos(d,xfinal,yfinal);

}



void visual_microdrive_marca_sector_cero(struct zxvision_vectorial_draw *d,int color)
{

    int ancho_esponjita=392-284;


    d->pencil_off(d);
    d->setpos(d,284,62);
    d->setcolour(d,color);
    d->pencil_on(d);
    d->set_x(d,284+ancho_esponjita/2); //La "marca" del sector 0 que sea la mitad del ancho de la esponjita

}

void menu_visual_microdrive_dibujar_microdrive_dinamico(struct zxvision_vectorial_draw *d)
{
    int numero_microdrive=menu_visual_microdrive_mirando_microdrive;

    //radios del rodillo amarillo, que va girando
    int sector_actual=microdrive_status[numero_microdrive].mdr_current_sector;
    int offset_actual=microdrive_status[numero_microdrive].mdr_current_offset_in_sector;

	int total_offset;

	if (microdrive_status[menu_visual_microdrive_mirando_microdrive].raw_format) {
		total_offset=microdrive_status[menu_visual_microdrive_mirando_microdrive].raw_current_position;
	}

    else total_offset=(sector_actual*MDR_BYTES_PER_SECTOR)+offset_actual;

    //borrar grados anteriores
    int color_fondo=VISUAL_MICRODRIVE_COLOR_FONDO;

    int x_origen_rodillo=573;
    int y_origen_rodillo=136;
    int longitud=98-1; //1 menos para que no sobresalga


    //asumimos cada byte mueve 1 grado
    //La cinta se mueve hacia la izquierda
    int grados=(total_offset % 360);

    int motor_on=microdrive_status[menu_visual_microdrive_mirando_microdrive].motor_on;

    if (visual_microdrive_slow_movement) {
        grados=visual_microdrive_slow_movement_grados % 360;
        if (motor_on) visual_microdrive_slow_movement_grados++;
    }


    //va hacia abajo. o sea la cinta se mueve hacia la derecha
    //grados=-grados;

    //Dibujar interior solo si esta habilitado ese microdrive
    if (microdrive_status[numero_microdrive].microdrive_enabled) {

        //Borrar los anteriores
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,visual_micro_antes_grados_rodillo,color_fondo);
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,visual_micro_antes_grados_rodillo+120,color_fondo);
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,visual_micro_antes_grados_rodillo+240,color_fondo);

        //Dibujar los actuales
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,grados,6);
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,grados+120,6);
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,grados+240,6);


        //Si estamos en sector 0, se indicara con color rojo donde esta la separacion del sector
        //Borrar antes posibles restos
        int color_cinta_enrollada=0; //esto sera negro
        visual_microdrive_marca_sector_cero(d,color_cinta_enrollada);

		int marca_sector_cero=0;

		if (microdrive_status[numero_microdrive].raw_format) {

			//Asi un poco a ojo, en formato raw, esa posicion mas o menos es la del sector 0
			if (microdrive_status[numero_microdrive].raw_current_position<MICRODRIVE_RAW_COMMON_SECTOR_SIZE) {
				marca_sector_cero=1;
			}
		}

        else {
			if (sector_actual==0) {
            	//De momento solo indicarlo en el trocito que lee la esponjita
            	//TODO: usar offset en el sector. De momento solo poner esa zona en rojo

				marca_sector_cero=1;

        	}
		}

		if (marca_sector_cero) visual_microdrive_marca_sector_cero(d,2);


        int radio_cinta_sectores;
        int radio_base;
        visual_microdrive_get_info_rodillos_interiores(&radio_cinta_sectores,&radio_base);

        //posicion maxima de la cinta enrollada. de ahi saldra una tangente hacia el rodillo superior izquierdo
        int pos_x_rodillo_enrollado=357-(radio_base+radio_cinta_sectores)+1;

        //Efecto de movimiento en la cinta
        //Efecto de temblar al moverse, si esta motor on

        int desplazamiento=6;
        //borrar lineas anteriores
        d->pencil_off(d);
        d->setcolour(d,color_fondo);
        d->setpos(d,pos_x_rodillo_enrollado,678);
        d->pencil_on(d);
        d->setpos(d,10,136);
        d->pencil_off(d);
        d->setpos(d,pos_x_rodillo_enrollado+desplazamiento,678);
        d->pencil_on(d);
        d->setpos(d,10,136);
        d->pencil_off(d);

        int sumar_pos=0;

        if (motor_on) {
            //int offset_actual=microdrive_status[numero_microdrive].mdr_current_offset_in_sector;

			if (microdrive_status[numero_microdrive].raw_format) {
				//Cinta raw tiembla cada paso de 500 bytes
				int paso=total_offset/500;
				if (paso % 2) sumar_pos=desplazamiento;
			}

			else {
				//En caso de mdr, tiembla cada paso de sector
				int sector_actual=microdrive_status[numero_microdrive].mdr_current_sector;
				if (sector_actual % 2) sumar_pos=desplazamiento;
			}
        }



        d->setcolour(d,color_cinta_enrollada);
        d->setpos(d,pos_x_rodillo_enrollado+sumar_pos,678);

        d->pencil_on(d);
        //primer tramo, desde abajo a la izquierda hasta pegado a rodillo izquierdo
        d->setpos(d,10,136);


        //y tangente hacia el interior, el inicio de donde se enrolla el microdrive
        //Temblado de la cinta en horizontal si esta motor on
        //Borrando anteriores
        d->setcolour(d,color_fondo);
        d->pencil_off(d);
        d->setpos(d,573+98,136);
        d->pencil_on(d);
        d->setpos(d,357+152,678);

        d->pencil_off(d);
        d->setpos(d,573+98,136);
        d->pencil_on(d);
        d->setpos(d,357+152-desplazamiento,678);

        //Y dibujar la cinta
        d->setcolour(d,color_cinta_enrollada);
        d->pencil_off(d);
        d->setpos(d,573+98,136);

        d->pencil_on(d);
        //septimo tramo
        d->setpos(d,357+152-sumar_pos,678);


        //rodillo arriba a la derecha. redibujar entero porque el borrado de radios puede borrar parte de este
        //tambien la cinta de la derecha sobrescribe encima de aqui, y queremos que el rodillo siempre este por encima
        d->pencil_off(d);
        d->setcolour(d,6);
        d->setpos(d,573,136);
        d->drawcircle(d,98);
        d->drawcircle(d,97);
        d->drawcircle(d,96);
        d->drawcircle(d,95);

        //y parte del rodillo de la derecha muestra la cinta
        d->pencil_off(d);
        d->setpos(d,573,136);
        d->setcolour(d,color_cinta_enrollada);
        d->drawarc(d,100,0,90); //algo mas que 98 de radio para que no se pegue


        //rodillo arriba a la izquierda
        //radios de movimiento

        x_origen_rodillo=108;
        //borrar anterior

        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,visual_micro_antes_grados_rodillo,color_fondo);
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,visual_micro_antes_grados_rodillo+120,color_fondo);
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,visual_micro_antes_grados_rodillo+240,color_fondo);

        //actual
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,grados,15);
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,grados+120,15);
        menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,grados+240,15);


        //circulo exterior
        d->pencil_off(d);
        d->setcolour(d,15);
        d->setpos(d,108,136);
        d->drawcircle(d,98);
        d->drawcircle(d,97);
        d->drawcircle(d,96);
        d->drawcircle(d,95);

        //y parte de ese rodillo muestra la cinta
        d->setcolour(d,color_cinta_enrollada);
        d->drawarc(d,100,90,180); //algo mas que 98 de radio para que no se pegue


    }

    //Cabezal lector
    int color_cabezal=15;
    //Si está escribiendo, color rojo
    //Si lee, color verde
    //Si no esta motor activado, color blanco
    if (motor_on) {
        if (interface1_last_value_port_ef & 0x04) color_cabezal=4; //read
        else color_cabezal=2; //write

		//escribiendo gap. magenta
		//no se aprecia por tanto no lo habilito
		/*if ((interface1_last_value_port_ef & 0x08)==0 && (interface1_last_value_port_ef & 0x04)) {
			//printf("Magenta\n");
			color_cabezal=3;
		}*/
    }

    d->setcolour(d,color_cabezal);
    //izquierda
    d->pencil_off(d);
    d->setpos(d,284,-10);
    d->pencil_on(d);
    d->set_y(d,30);
    //derecha
    d->pencil_off(d);
    d->setpos(d,392,-10);
    d->pencil_on(d);
    d->set_y(d,30);
    //arco lector parte abajo
    d->pencil_off(d);
    //centro del arco
    d->setpos(d,284+(392-284)/2,-10);
    d->pencil_on(d);

    //tanto inicio arco como radio se han hecho a ojo para que quede bien
    int inicio_arco=210;
    d->drawarc(d,68,inicio_arco,270+(270-inicio_arco));


    //Rodillo exterior de arrastre
    x_origen_rodillo=-35;
    y_origen_rodillo=175;


    //Radios de movimiento. Se mueve al contrario que el rodillo derecho

    longitud=50-1; //1 menos para que no sobresalga
    //borrar los anteriores
    //se mueve mas rapidamente que los rodillos grandes
    //esto es debido porque he hecho el rodillo mas pequeño, y como está en contacto con el grande,
    //el pequeño se movera mas rapido que el grande
    //TODO: no sé el tamaño real del rodillo de arrastre, lo he hecho mas pequeño porque en el dibujo queda mejor asi
    int antes_grados=-visual_micro_antes_grados_rodillo*2;

    menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,antes_grados,color_fondo);
    menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,antes_grados+120,color_fondo);
    menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,antes_grados+240,color_fondo);

    int grados_izquierdo=-grados*2;
    menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,grados_izquierdo,15);
    menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,grados_izquierdo+120,15);
    menu_visual_microdrive_dibujar_microdrive_dinamico_dibuja_radio(d,x_origen_rodillo,y_origen_rodillo,longitud,grados_izquierdo+240,15);


    //circulo exterior
    d->pencil_off(d);
    d->setcolour(d,15);
    d->setpos(d,x_origen_rodillo,y_origen_rodillo);
    d->drawcircle(d,50);
    d->drawcircle(d,49);
    //d->drawcircle(d,48);

    visual_micro_antes_grados_rodillo=grados;

}

//int visual_microdrive_forzar_redraw=0;

void menu_visual_microdrive_overlay(void)
{

    menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (menu_visual_microdrive_window->is_minimized) return;




    //Print....
    //Tambien contar si se escribe siempre o se tiene en cuenta contador_segundo...

    //Dibujo del microdrive
    struct zxvision_vectorial_draw dibujo_microdrive;

    //quitamos 4: 1 columnas izquierda margen, columna derecha margen, columna scroll
    int tamanyo_ocupado_microdrive_ancho=(menu_visual_microdrive_window->visible_width-3)*menu_char_width;
    //quitamos 5: barra titulo,barra scroll, 2 lineas menu, 1 linea separacion
    int tamanyo_ocupado_microdrive_alto=(menu_visual_microdrive_window->visible_height-5)*menu_char_height;

    int offset_x=menu_char_width*1;
    int offset_y=menu_char_height*3;

    //Ajustar escalas
    //Relacion de aspecto ideal: 700 ancho, 1000 alto

    int ancho_total_dibujo_virtual=700;

    //Le tenemos que considerar el rodillo de arrastre. Empieza en -35 y son 50 de radio
    //Este rodillo no estaba considerado en el dibujo original (que era de 700 de ancho) por eso tengo que sumarlo aparte
    //y ademas ajustar el offset_x para que quepa bien
    int ocupado_rodillo_izquierdo=85;
    ancho_total_dibujo_virtual +=ocupado_rodillo_izquierdo;



    int real_width=tamanyo_ocupado_microdrive_ancho;


    //Desactivar este trocito si queremos que el ancho pueda crecer independientemente del alto de ventana. SOLO PARA PRUEBAS
    int max_ancho_esperado_por_aspecto=(tamanyo_ocupado_microdrive_alto*ancho_total_dibujo_virtual)/1000;
    if (real_width>max_ancho_esperado_por_aspecto) {
        //Con esto el microdrive siempre esta dentro de la ventana entero, independientemente del tamaño de la ventana
        //printf("relacion ancho mal\n");
        real_width=max_ancho_esperado_por_aspecto;
    }


    int real_height=(real_width*1000)/ancho_total_dibujo_virtual;


    //Al offset_x hay que sumarle lo que ocupa el rodillo izquierdo (85 puntos en resolucion virtual)
    //El offset es en pixeles reales
    //Convertir de esos 85 a reales
    int sumar_offset_x=(ocupado_rodillo_izquierdo*real_width)/ancho_total_dibujo_virtual;
    //printf("sumar por rodillo izquierdo: %d\n",sumar_offset_x);
    offset_x +=sumar_offset_x;


    zxvision_vecdraw_init(&dibujo_microdrive,menu_visual_microdrive_window,ancho_total_dibujo_virtual,1000,
        real_width,real_height,offset_x,offset_y);


    //No redibujar si no hay cambios de nada
    if (menu_visual_microdrive_window->dirty_user_must_draw_contents /*|| visual_microdrive_forzar_redraw*/) {
        //printf("Redibujando parte estatica. %d\n",contador_segundo_infinito);
        menu_visual_microdrive_dibujar_microdrive_estatico(&dibujo_microdrive);
        menu_visual_microdrive_window->dirty_user_must_draw_contents=0;
        //visual_microdrive_forzar_redraw=0;
    }

    menu_visual_microdrive_dibujar_microdrive_dinamico(&dibujo_microdrive);

    //Mostrar contenido
    zxvision_draw_window_contents(menu_visual_microdrive_window);

}




//Almacenar la estructura de ventana aqui para que se pueda referenciar desde otros sitios
zxvision_window zxvision_window_visual_microdrive;


void menu_visual_microdrive(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();

    if (!menu_multitarea) {
        menu_warn_message("This window needs multitask enabled");
        return;
    }

    zxvision_window *ventana;
    ventana=&zxvision_window_visual_microdrive;

	//IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
	//si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
	//la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
	//zxvision_delete_window_if_exists(ventana);

    //Crear ventana si no existe
    if (!zxvision_if_window_already_exists(ventana)) {
        int xventana,yventana,ancho_ventana,alto_ventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;

        if (!util_find_window_geometry("visualmicrodrive",&xventana,&yventana,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {
            ancho_ventana=30;
            alto_ventana=20;

            xventana=menu_center_x()-ancho_ventana/2;
            yventana=menu_center_y()-alto_ventana/2;
        }


        zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"Visual Microdrive",
            "visualmicrodrive",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);

        ventana->can_be_backgrounded=1;


        //definir color de papel de fondo
        ventana->default_paper=VISUAL_MICRODRIVE_COLOR_FONDO; //AMIGAOS_COLOUR_blue;
        zxvision_cls(ventana);
        //visual_microdrive_forzar_redraw=1;

    }

    //Si ya existe, activar esta ventana
    else {
        zxvision_activate_this_window(ventana);
    }

	zxvision_draw_window(ventana);

    //para mostrar correctamente el color del fondo alterado por default_paper
    zxvision_draw_window_contents(ventana);

	//z80_byte tecla;


	//int salir=0;


    menu_visual_microdrive_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


    //cambio overlay
    zxvision_set_window_overlay(ventana,menu_visual_microdrive_overlay);

    //ventana->dirty_user_must_draw_contents=1;


    //Toda ventana que este listada en zxvision_known_window_names_array debe permitir poder salir desde aqui
    //Se sale despues de haber inicializado overlay y de cualquier otra variable que necesite el overlay
    if (zxvision_currently_restoring_windows_on_start) {
            //printf ("Saliendo de ventana ya que la estamos restaurando en startup\n");
            return;
    }


    	menu_item *array_menu_visual_microdrive;
	menu_item item_seleccionado;
	int retorno_menu;
	do {



        //borrar primera linea por si conmuta parametro rotacion
        //zxvision_fill_width_spaces_paper(ventana,0,HEATMAP_INDEX_FIRST_COLOR);



		menu_add_item_menu_inicial_format(&array_menu_visual_microdrive,MENU_OPCION_NORMAL,menu_visual_microdrive_cual,NULL
            ,"~~Looking MDV%d",menu_visual_microdrive_mirando_microdrive+1);
		menu_add_item_menu_shortcut(array_menu_visual_microdrive,'l');
		menu_add_item_menu_ayuda(array_menu_visual_microdrive,"Which microdrive");
		menu_add_item_menu_tabulado(array_menu_visual_microdrive,1,0);

		menu_add_item_menu_format(array_menu_visual_microdrive,MENU_OPCION_NORMAL,menu_visual_microdrive_slow_movement,NULL
            ,"[%c] ~~Slow movement",(visual_microdrive_slow_movement ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_visual_microdrive,'s');
		menu_add_item_menu_ayuda(array_menu_visual_microdrive,"Slow movement");
		menu_add_item_menu_tabulado(array_menu_visual_microdrive,1,1);


		//Nombre de ventana solo aparece en el caso de stdout
		retorno_menu=menu_dibuja_menu_no_title_lang(&visualmicrodrive_opcion_seleccionada,&item_seleccionado,array_menu_visual_microdrive,"Visual Microdrive" );

		if (retorno_menu!=MENU_RETORNO_BACKGROUND) {
            //En caso de menus tabulados, es responsabilidad de este de borrar la ventana
            //Con este cls provoca que se borren todas las otras ventanas en background


            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    if (item_seleccionado.menu_funcion!=NULL) {
                            //printf ("actuamos por funcion\n");
                            item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);


                    }
            }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus && retorno_menu!=MENU_RETORNO_BACKGROUND);





	util_add_window_geometry_compact(ventana);

	if (retorno_menu==MENU_RETORNO_BACKGROUND) {
		zxvision_message_put_window_background();
	}

	else {

		zxvision_destroy_window(ventana);
	}





}


void menu_visual_microdrive_previo(MENU_ITEM_PARAMETERS)
{
    //Definir cual estara activo
    menu_visual_microdrive_mirando_microdrive=valor_opcion;
    if (menu_visual_microdrive_mirando_microdrive>=MAX_MICRODRIVES_BY_CONFIG) menu_visual_microdrive_mirando_microdrive=0;

    menu_visual_microdrive(0);
}

void menu_interface1_rom_version(MENU_ITEM_PARAMETERS)
{

    if (if1_custom_rom_enabled.v) {
        if1_custom_rom_enabled.v=0;
        if1_rom_version=1;
        return;
    }

    if (if1_rom_version==1) if1_rom_version=2;

    else if1_custom_rom_enabled.v=1;
}

int menu_interface1_rom_cond(void)
{
    if (if1_enabled.v) return 0;
    else return 1;
}

void menu_interface1_rom_file(MENU_ITEM_PARAMETERS)
{
    char *filtros[2];

    filtros[0]="rom";
    filtros[1]=0;


    if (menu_filesel("Select ROM File",filtros, if1_custom_rom_file)==1) {
        //Nada

    }
    //Sale con ESC
    else {
        //Quitar nombre
        if1_custom_rom_file[0]=0;
    }

}



int microdrive_raw_map_selected_unit=0;

//Zoom:
//1=1 pixel por posicion
//2=2 pixel por posicion
//4=4 pixel por posicion
//8=8 pixel por posicion
//-2 = 1 pixel por cada 2 posiciones
//-4 = 1 pixel por cada 4 posiciones
//-8 = 1 pixel por cada 8 posiciones
int microdrive_raw_map_zoom=1;

int microdrive_raw_map_autoscroll=0;

//Donde empieza el mapa en coordenadas de pixel
int microdrive_raw_map_start_x=1;
int microdrive_raw_map_start_y=3;

zxvision_window *menu_microdrive_raw_map_window;

int microdrive_raw_map_forzar_dibujado=0;

int microdrive_raw_map_minimo_zoom_reduccion=16;
int microdrive_raw_map_maximo_zoom_reduccion=16;

int microdrive_raw_map_antes_pos_cabezal_lectura=-1;


int microdrive_raw_map_draw_fin_dibujar=0;

//Si muestra hexa (0) o char (1)
int microdrive_raw_map_draw_zoom_show_char=0;

//Si muestra cabezal de lectura o no
int microdrive_raw_map_dibujar_cabezal=1;


//Si muestra posiciones_sync
int microdrive_raw_map_dibujar_sync=0;


//Inicio desde donde se empieza a leer
int menu_microdrive_raw_map_start_index=0;

//Bytes mostrados en pantalla por linea
int menu_microdrive_raw_map_byte_width=0;

//total lineas dibujadas, usadas en pgup/down
int menu_microdrive_raw_map_lineas_dibujadas=0;

//valor de una pagina entera, sin tener en cuenta paginas parciales que estan cerca del final
int menu_microdrive_raw_map_lineas_dibujadas_final=0;


void menu_microdrive_raw_map_draw_putpixel_char(zxvision_window *w,int x,int y,int tinta,int papel,z80_byte caracter)
{

    if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) caracter='?';

    int color;
    z80_byte bit;
    z80_byte line;
    z80_byte byte_leido;



	z80_byte *puntero;
	puntero=&char_set[(caracter-32)*8];



    for (line=0;line<8;line++,y++) {
        byte_leido=*puntero++;

        for (bit=0;bit<8;bit++) {
            if (byte_leido & 128 ) color=tinta;
            else color=papel;


            byte_leido=(byte_leido&127)<<1;

            //este scr_putpixel_zoom_rainbow tiene en cuenta los timings de la maquina (borde superior, por ejemplo)

            if (color==tinta) {

            int xfinal;

            xfinal=x+bit;

            zxvision_putpixel(w,xfinal,y,color);
            }


        }
    }
}

void menu_microdrive_raw_map_draw_putpixel_bits(zxvision_window *w,int x,int y,int tinta,int papel,z80_byte caracter)
{



    int color;
    z80_byte bit;




    for (bit=0;bit<8;bit++) {
        if (caracter & 128 ) color=tinta;
        else color=papel;


        caracter=(caracter&127)<<1;

        //este scr_putpixel_zoom_rainbow tiene en cuenta los timings de la maquina (borde superior, por ejemplo)

        if (color==tinta) {

        int xfinal;

        xfinal=x+bit;

        zxvision_putpixel(w,xfinal,y,color);
        }


    }

}


//Si zoom >8 , mostramos contenido celda
//dibujar_pixel solo se hace para controlar limites pero no dibuja pixeles realmente
void menu_microdrive_raw_map_draw_putpixel(zxvision_window *w,int zoom,int xorig,int yorig,int color,z80_int dato_leido,int dibujar_pixel)
{
    int x,y;

    int limite_superior=microdrive_raw_map_start_y*menu_char_height;

    if (yorig+zoom>=(w->visible_height+w->offset_y-2)*menu_char_height) {
        //printf("final dibujado mapa en %d\n",yfinal);
        microdrive_raw_map_draw_fin_dibujar=1;
        return;
    }

    for (y=0;y<zoom;y++) {

        int yfinal=yorig;


        yfinal +=y;

        if (yfinal<limite_superior) return;

        //printf("y %d\n",yfinal);

        //detectar si hace putpixel mas alla de zona visible de ventana
        //-2 del titulo y la linea de abajo sin usar
        /*if (yfinal>=(w->visible_height+w->offset_y-2)*menu_char_height) {
            //printf("final dibujado mapa en %d\n",yfinal);
            microdrive_raw_map_draw_fin_dibujar=1;
            return;
        }*/

        if (!dibujar_pixel) return;

        for (x=0;x<zoom;x++) {

            int xfinal=xorig+x;


            //mostrar separacion entre casillas si zoom grande
            if (zoom>=4) {
                if (!(  (x%zoom)==0 || (y%zoom)==0) ) zxvision_putpixel(w,xfinal,yfinal,color);
                else zxvision_putpixel(w,xfinal,yfinal,ESTILO_GUI_PAPEL_NORMAL);
            }
            else zxvision_putpixel(w,xfinal,yfinal,color);
        }
    }

    //Si zoom 8, mostrar el byte como sus pixeles de bit separado
    if (zoom==8) {
        //Escribir caracter, siempre que no sea gap
        if (dato_leido & MICRODRIVE_RAW_INFO_BYTE_MASK_DATA) {
            int yfinal=yorig;

            int color_tinta=7-(color&7);

            z80_byte byte_leido=(dato_leido & 0xFF);

            int xcaracter=xorig;
            int ycaracter=yfinal+4;

            //doble de alto
            menu_microdrive_raw_map_draw_putpixel_bits(w,xcaracter,ycaracter,color_tinta,color,byte_leido);
            menu_microdrive_raw_map_draw_putpixel_bits(w,xcaracter,ycaracter+1,color_tinta,color,byte_leido);
        }

    }

    if (zoom>=16) {
        //Escribir caracter, siempre que no sea gap
        if (dato_leido & MICRODRIVE_RAW_INFO_BYTE_MASK_DATA) {
            int yfinal=yorig;

            int color_tinta=7-(color&7);

            z80_byte byte_leido=(dato_leido & 0xFF);

            char buffer[3];

            if (microdrive_raw_map_draw_zoom_show_char) {
                sprintf(buffer,"%c ",(byte_leido>=32 && byte_leido<=126 ? byte_leido : '.'));
            }

            else sprintf(buffer,"%02X",byte_leido);

            int xcaracter=xorig+1;
            int ycaracter=yfinal+4;

            menu_microdrive_raw_map_draw_putpixel_char(w,xcaracter,ycaracter,color_tinta,color,buffer[0]);
            menu_microdrive_raw_map_draw_putpixel_char(w,xcaracter+8,ycaracter,color_tinta,color,buffer[1]);
        }
    }


}

void menu_microdrive_raw_map_mostrar_opciones(zxvision_window *ventana)
{


    char buffer_show_char[50]="";

    if (microdrive_raw_map_zoom>=16) {
        sprintf(buffer_show_char,"[%c] Show ~~char",
        (microdrive_raw_map_draw_zoom_show_char ? 'X' : ' '));
    }

    char buffer_zoom[50]="";

    if (microdrive_raw_map_zoom>0) {
        sprintf(buffer_zoom,"    %2dX",microdrive_raw_map_zoom);
    }

    else {
        switch (microdrive_raw_map_zoom) {

            case -2:
                strcpy(buffer_zoom,"   0.5X");
            break;

            case -4:
                strcpy(buffer_zoom,"  0.25X");
            break;

            case -8:
                strcpy(buffer_zoom," 0.125X");
            break;

            case -16:
                strcpy(buffer_zoom,"0.0625X");
            break;

            default:
                sprintf(buffer_zoom,"%dX",microdrive_raw_map_zoom);
            break;
        }
    }

    int total_limit=microdrive_status[microdrive_raw_map_selected_unit].raw_total_size;
    int perc;

    total_limit--;

    if (total_limit>0) {
        perc=(menu_microdrive_raw_map_start_index*100)/total_limit;
    }

    else perc=0;

    zxvision_print_string_defaults_fillspc_format(ventana,1,0,"Zoom: %s Curr. ~~pos: %7d/%7d (%3d %%)",
        buffer_zoom,menu_microdrive_raw_map_start_index,
        total_limit,perc);


    zxvision_print_string_defaults_fillspc_format(ventana,1,1,"~~f~~1: help [%d] ~~mdv ~~z: -zoom ~~x: +zoom %s",
        microdrive_raw_map_selected_unit+1,buffer_show_char);

    //zxvision_print_string_defaults_fillspc(ventana,1,1,"");

    //zxvision_print_string_defaults_fillspc_format(ventana,1,2,"[%c] ~~Autoscroll [%c] ~~Head cursors,PgUp,PgDn: change pos",
    zxvision_print_string_defaults_fillspc_format(ventana,1,2,"[%c] ~~Autoscroll [%c] show ~~Head [%c] show ~~Sync",
        (microdrive_raw_map_autoscroll ? 'X' : ' ' ),
        (microdrive_raw_map_dibujar_cabezal ? 'X' : ' ' ),
        (microdrive_raw_map_dibujar_sync ? 'X' : ' ' )
    );




    if (!microdrive_status[microdrive_raw_map_selected_unit].microdrive_enabled) {
        zxvision_print_string_defaults_fillspc_format(ventana,1,0,"Selected MDV is not enabled");
    }

    else if (!microdrive_status[microdrive_raw_map_selected_unit].raw_format) {
        zxvision_print_string_defaults_fillspc_format(ventana,1,0,"Selected MDV is not raw");
    }


}

//Retorna 0 si no es sync
int microdrive_raw_map_detect_sync(int microdrive_seleccionado,int pos)
{
    //Primero ver que al menos hay 8 posiciones posibles
    int total_size=microdrive_status[microdrive_seleccionado].raw_total_size;

    z80_int *p=microdrive_status[microdrive_seleccionado].raw_microdrive_buffer;

    if (pos<=total_size-8) {
        //Ver que los 8 siguientes sean datos con valores esperados
        //Nos quedamos con la mascara del dato y el bit que indica gap o no
        if (
            (p[pos+0] & 0x1FF)==0x0100 &&
            (p[pos+1] & 0x1FF)==0x0100 &&
            (p[pos+2] & 0x1FF)==0x0100 &&
            (p[pos+3] & 0x1FF)==0x0100 &&
            (p[pos+4] & 0x1FF)==0x0100 &&
            (p[pos+5] & 0x1FF)==0x0100 &&
            (p[pos+6] & 0x1FF)==0x01FF &&
            (p[pos+7] & 0x1FF)==0x01FF
        ) {
            return 1;
        }
    }

    return 0;

}

void menu_microdrive_raw_map_draw(zxvision_window *w)
{
    int offset_x=microdrive_raw_map_start_x*menu_char_width;
    int offset_y=microdrive_raw_map_start_y*menu_char_height;

    int total_ancho=(w->visible_width-microdrive_raw_map_start_x)*menu_char_width;
    //int total_alto=(w->visible_height-microdrive_raw_map_start_y-2)*menu_char_height;

    //int max_y=total_alto;
    int max_ancho=total_ancho-offset_x;

    //menu_microdrive_raw_map_max_ancho=max_ancho;

    enum lista_estados_pixel {
        DATO_SIN_USO=0,
        GAP_SIN_USO,
        DATO_LEYENDO,
        GAP_LEYENDO,
        DATO_ESCRIBIENDO,
        GAP_ESCRIBIENDO,
        DEFECTUOSO_SIN_USO,
        DEFECTUOSO_LEYENDO,
        DEFECTUOSO_ESCRIBIENDO
    };

    //byte de gap sin uso (ni read ni write)
    //blanco sin brillo
    int color_gap=7;

    //byte de datos sin uso (ni read ni write)
    //azul
    int color_pixel=1;

    //blanco con brillo
    int color_gap_leyendo=7+8;

    //cyan
    int color_pixel_leyendo=5;

    //magenta
    int color_defectuso=3;
    int color_defectuso_leyendo=3+8;

    //Al escribir, salen rojos
    int color_gap_escribiendo=2;
    int color_pixel_escribiendo=2;
    int color_defectuso_escribiendo=2;

    int color_sync=6;

    //Para que no muestre sync inicialmente en ningun sitio. con -8 ya hubiera sido suficiente
    int ultima_pos_sync=-100;


    int color_posicion_lectura=0; //4

    int total_size=microdrive_status[microdrive_raw_map_selected_unit].raw_total_size;

    int i;
    int x=0;
    int y=0;

    int reducir_zoom=0;

    if (microdrive_raw_map_zoom<0) {
        reducir_zoom=-microdrive_raw_map_zoom;
    }

    //Para indicar cuantos estados hay de cada en cada posicion
    int estados_pixel_zoom[9]={0,0,0,0,0,0,0,0,0};

    //Si esta dibujando mas alla de y visible, finalizar

    int conteo_zoom=0;

    if (microdrive_raw_map_autoscroll) {
        //no limitar alto
        //max_y=max_y*2; //total_size; //es mucho menos que eso, pero para que no limite
    }

    //redibujarla entera cuando se haya movido, o alguna por encima , etc etc
    if (w->dirty_user_must_draw_contents) {
        microdrive_raw_map_forzar_dibujado=1;


        w->dirty_user_must_draw_contents=0;
    }

    //if (microdrive_raw_map_forzar_dibujado) printf("Forzar redibujado\n");
    //else printf("no forzar redibujado\n");

    int total_pixeles_dibujados=0;

    int dibujar_cabezal_lectura=0;

    int pos_cabezal=microdrive_status[microdrive_raw_map_selected_unit].raw_current_position;

    //printf("max alto: %d\n",max_alto);

    microdrive_raw_map_draw_fin_dibujar=0;

    menu_microdrive_raw_map_byte_width=0;

    int salir=0;

    int inicio_index=menu_microdrive_raw_map_start_index;

    menu_microdrive_raw_map_lineas_dibujadas=0;

    //if (microdrive_raw_map_autoscroll) inicio_index=0;

    for (i=inicio_index;i<total_size && !salir;i++) {
        z80_int dato_leido=microdrive_status[microdrive_raw_map_selected_unit].raw_microdrive_buffer[i];

        //Conteo de cantidad de bytes que caben en una linea solo para la primera linea
        if (y==0) menu_microdrive_raw_map_byte_width++;

        enum lista_estados_pixel estado_pixel=DATO_SIN_USO;


        if ((dato_leido & MICRODRIVE_RAW_INFO_BYTE_MASK_DATA)==0) {
            //es un gap
            estado_pixel=GAP_SIN_USO;
        }


        if (dato_leido & MICRODRIVE_RAW_INFO_BYTE_MASK_BAD_POSITION) {
            //Es defectuoso
            estado_pixel=DEFECTUOSO_SIN_USO;
        }

        //Detectar sync
        if (microdrive_raw_map_dibujar_sync) {
            int posible_sync=microdrive_raw_map_detect_sync(microdrive_raw_map_selected_unit,i);

            if (posible_sync) {
                //printf("Detectado sync en %d\n",i);
                ultima_pos_sync=i;
            }
        }

        int dibujar_pixel=0;

        if (microdrive_raw_map_forzar_dibujado) {
            dibujar_pixel=1;
        }

        //Consultar visualmem
#ifdef EMULATE_VISUALMEM
        int posicion_visualmem=microdrive_get_visualmem_position(i);
        if (posicion_visualmem>=0) {

            //Evitar tener que redibujar todos los pixeles. Solo los que se hayan modificado esta vez o la anterior

            //Si lectura
            z80_byte visualmem_read_buffer=visualmem_microdrive_read_buffer[posicion_visualmem];
            if (visualmem_read_buffer) {
                dibujar_pixel=1;

                //Si vale 1, es que ha habido cambio en visualmem. Si vale 2, es que era la posicion anterior
                if (visualmem_read_buffer==1) {
                    visualmem_read_buffer=2;

                    if (estado_pixel==GAP_SIN_USO) estado_pixel=GAP_LEYENDO;
                    else if (estado_pixel==DATO_SIN_USO) estado_pixel=DATO_LEYENDO;
                    else if (estado_pixel==DEFECTUOSO_SIN_USO) estado_pixel=DEFECTUOSO_LEYENDO;
                }

                else if (visualmem_read_buffer>=2) visualmem_read_buffer=0;

                visualmem_microdrive_read_buffer[posicion_visualmem]=visualmem_read_buffer;
            }

            //Si escritura
            z80_byte visualmem_write_buffer=visualmem_microdrive_write_buffer[posicion_visualmem];
            if (visualmem_write_buffer) {
                dibujar_pixel=1;

                if (visualmem_write_buffer==1) {
                    visualmem_write_buffer=2;

                    if (estado_pixel==GAP_SIN_USO) estado_pixel=GAP_ESCRIBIENDO;
                    else if (estado_pixel==DATO_SIN_USO) estado_pixel=DATO_ESCRIBIENDO;
                    else if (estado_pixel==DEFECTUOSO_SIN_USO) estado_pixel=DEFECTUOSO_ESCRIBIENDO;
                }

                else if (visualmem_write_buffer>=2) visualmem_write_buffer=0;

                visualmem_microdrive_write_buffer[posicion_visualmem]=visualmem_write_buffer;
            }

        }
#else

    //Si no tiene visualmem es poco optimo porque al no saber que se ha modificado, necesitamos redibujar todo siempre
    //Y consumira mucha cpu
    dibujar_pixel=1;


#endif

        estados_pixel_zoom[estado_pixel]=estados_pixel_zoom[estado_pixel]+1;

        if (microdrive_raw_map_zoom<0) {

            conteo_zoom++;
            if (conteo_zoom>=reducir_zoom) {

                conteo_zoom=0;

                //Contar cual ha salido vencedor
                int j;
                int pos_vencedor=0;

                for (j=0;j<9;j++) {
                    if (estados_pixel_zoom[j]>estados_pixel_zoom[pos_vencedor]) {
                        pos_vencedor=j;
                    }
                }

                estado_pixel=pos_vencedor;

                estados_pixel_zoom[0]=0;
                estados_pixel_zoom[1]=0;
                estados_pixel_zoom[2]=0;
                estados_pixel_zoom[3]=0;
                estados_pixel_zoom[4]=0;
                estados_pixel_zoom[5]=0;
                estados_pixel_zoom[6]=0;
                estados_pixel_zoom[7]=0;
                estados_pixel_zoom[8]=0;
            }
        }


        int color;

        switch(estado_pixel) {
            case DATO_SIN_USO:
                color=color_pixel;
                if (i>=ultima_pos_sync && i<=ultima_pos_sync+7) color=color_sync;
            break;

            case GAP_SIN_USO:
                color=color_gap;
            break;

            case DEFECTUOSO_SIN_USO:
                color=color_defectuso;
            break;

            case DATO_LEYENDO:
                color=color_pixel_leyendo;
            break;

            case GAP_LEYENDO:
                color=color_gap_leyendo;
            break;

            case DEFECTUOSO_LEYENDO:
                color=color_defectuso_leyendo;
            break;

            case DATO_ESCRIBIENDO:
                color=color_pixel_escribiendo;
            break;

            case GAP_ESCRIBIENDO:
                color=color_gap_escribiendo;
            break;

            case DEFECTUOSO_ESCRIBIENDO:
                color=color_defectuso_escribiendo;
            break;

            default:
                color=color_pixel;
            break;
        }

        //Si posicion cerca del cabezal de lectura. x posiciones atras o adelante
        //Ese x tiene que ser el valor maximo de -zoom, para que siempre se vea el cabezal



        /*if (i>=pos_menos && i<=pos_mas) {
            color=color_posicion_lectura;
            dibujar_pixel=1;
        }*/

       int longitud_cabezal=10;

        //Si en la posicion actual del cabezal
        if (i==pos_cabezal && microdrive_raw_map_dibujar_cabezal) {
            dibujar_cabezal_lectura=longitud_cabezal;
        }

        //O en la anterior del cabezal
        if (i>=microdrive_raw_map_antes_pos_cabezal_lectura && i<microdrive_raw_map_antes_pos_cabezal_lectura+longitud_cabezal) {
            dibujar_pixel=1;
        }



        //Para zoom positivo
        if (microdrive_raw_map_zoom>=1) {
            if (dibujar_cabezal_lectura) {
                dibujar_cabezal_lectura--;
                color=color_posicion_lectura;
                dibujar_pixel=1;
            }

            menu_microdrive_raw_map_draw_putpixel(w,microdrive_raw_map_zoom,x+offset_x,y+offset_y,color,dato_leido,dibujar_pixel);
            if (dibujar_pixel) {
                total_pixeles_dibujados++;
            }




            x+=microdrive_raw_map_zoom;
            if (x>=max_ancho) {
                x=0;
                y+=microdrive_raw_map_zoom;
                menu_microdrive_raw_map_lineas_dibujadas++;
            }
        }

        //zoom negativo
        else {
            if (conteo_zoom==0) {
                if (dibujar_cabezal_lectura) {
                    dibujar_cabezal_lectura--;
                    color=color_posicion_lectura;
                    dibujar_pixel=1;
                }

                menu_microdrive_raw_map_draw_putpixel(w,1,x+offset_x,y+offset_y,color,dato_leido,dibujar_pixel);
                if (dibujar_pixel) {
                    total_pixeles_dibujados++;
                }



                x++;
                if (x>=max_ancho) {
                    x=0;
                    y++;
                    menu_microdrive_raw_map_lineas_dibujadas++;
                }
            }
        }


        //Si ha saltado de linea y se ha detectado final visible
        if (/*x==0 &&*/ microdrive_raw_map_draw_fin_dibujar) {
            //printf("saliendo en y: %d i: %d menu_microdrive_raw_map_lineas_dibujadas: %d\n",y,i,menu_microdrive_raw_map_lineas_dibujadas);
            salir=1;
        }
    }

    if (menu_microdrive_raw_map_lineas_dibujadas>menu_microdrive_raw_map_lineas_dibujadas_final) {
        menu_microdrive_raw_map_lineas_dibujadas_final=menu_microdrive_raw_map_lineas_dibujadas;
    }

    //printf("Salido bucle en i %d microdrive_raw_map_draw_fin_dibujar: %d microdrive_raw_map_forzar_dibujado: %d\n",
    //    i,microdrive_raw_map_draw_fin_dibujar,microdrive_raw_map_forzar_dibujado);

    //printf("Salido bucle en i %d \n",i);

    int forzado_siguiente_redraw=0;

    //Si hay que cambiar scroll. Solo considerar cuando no se haya hecho un full redraw
    //Autoscroll a la posicion del cabezal
    if (!microdrive_raw_map_forzar_dibujado && microdrive_raw_map_autoscroll) {


            int next_pos=pos_cabezal;

            //ajustarla a multiple de linea
            if (menu_microdrive_raw_map_byte_width>0) {
                int resto=next_pos % menu_microdrive_raw_map_byte_width;
                next_pos -=resto;
            }

            if (menu_microdrive_raw_map_start_index!=next_pos) {
                menu_microdrive_raw_map_start_index=next_pos;

                //printf("cambiar inicio a %d\n",menu_microdrive_raw_map_start_index);

                forzado_siguiente_redraw=1;

                w->must_clear_cache_on_draw_once=1;
            }


    }



    microdrive_raw_map_forzar_dibujado=0;

    if (forzado_siguiente_redraw) microdrive_raw_map_forzar_dibujado=1;

    microdrive_raw_map_antes_pos_cabezal_lectura=pos_cabezal;


}

void menu_microdrive_raw_map_overlay(void)
{

    menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (menu_microdrive_raw_map_window->is_minimized) return;


    //Print....
    //Tambien contar si se escribe siempre o se tiene en cuenta contador_segundo...

    //Solo hacer esto con los que son raw
    if (!microdrive_status[microdrive_raw_map_selected_unit].microdrive_enabled) return;
    if (!microdrive_status[microdrive_raw_map_selected_unit].raw_format) return;

    menu_microdrive_raw_map_draw(menu_microdrive_raw_map_window);

    //No alterar w->dirty_user_must_draw_contents, que lo altera al hacer prints
    int antes_draw=menu_microdrive_raw_map_window->dirty_user_must_draw_contents;
    menu_microdrive_raw_map_mostrar_opciones(menu_microdrive_raw_map_window);
    menu_microdrive_raw_map_window->dirty_user_must_draw_contents=antes_draw;

    //Mostrar contenido
    zxvision_draw_window_contents(menu_microdrive_raw_map_window);

}


void menu_microdrive_raw_map_increase_zoom(void)
{

    if (microdrive_raw_map_zoom>0) {

        microdrive_raw_map_zoom *=2;

        if (microdrive_raw_map_zoom>microdrive_raw_map_maximo_zoom_reduccion) microdrive_raw_map_zoom=microdrive_raw_map_maximo_zoom_reduccion;
    }

    else {
        microdrive_raw_map_zoom /=2;
        if (microdrive_raw_map_zoom==-1) microdrive_raw_map_zoom=1;
    }

    //printf("zoom: %d\n",microdrive_raw_map_zoom);

}

void menu_microdrive_raw_map_reduce_zoom(void)
{

    if (microdrive_raw_map_zoom>0) {

        microdrive_raw_map_zoom /=2;

        if (microdrive_raw_map_zoom==0) microdrive_raw_map_zoom=-2;
    }

    else {
        microdrive_raw_map_zoom *=2;
        int menor_zoom=-microdrive_raw_map_minimo_zoom_reduccion;
        if (microdrive_raw_map_zoom<menor_zoom) microdrive_raw_map_zoom=menor_zoom;
    }

    //printf("zoom: %d\n",microdrive_raw_map_zoom);

}

void menu_microdrive_raw_map_abajo(zxvision_window *ventana)
{
    menu_microdrive_raw_map_start_index+=menu_microdrive_raw_map_byte_width;
    if (menu_microdrive_raw_map_start_index>=microdrive_status[microdrive_raw_map_selected_unit].raw_total_size) {
        menu_microdrive_raw_map_start_index=microdrive_status[microdrive_raw_map_selected_unit].raw_total_size-1;
    }
    ventana->must_clear_cache_on_draw_once=1;
    microdrive_raw_map_forzar_dibujado=1;
}

void menu_microdrive_raw_map_arriba(zxvision_window *ventana)
{
    menu_microdrive_raw_map_start_index-=menu_microdrive_raw_map_byte_width;
    if (menu_microdrive_raw_map_start_index<0) menu_microdrive_raw_map_start_index=0;

    ventana->must_clear_cache_on_draw_once=1;
    microdrive_raw_map_forzar_dibujado=1;
}

void microdrive_raw_map_edit_index(void)
{
	int offset=menu_microdrive_raw_map_start_index;

    int max=microdrive_status[microdrive_raw_map_selected_unit].raw_total_size-1;

	menu_ventana_scanf_numero_enhanced("New position",&offset,8,+1024,0,max,0);

    //comprobar error
    if (if_pending_error_message) {
        menu_muestra_pending_error_message(); //Si se genera un error derivado de menu_ventana_scanf_numero_enhanced
    }

    menu_microdrive_raw_map_start_index=offset;

    //printf("offset: %d\n",offset);


}

void microdrive_raw_map_help(void)
{

    menu_generic_message("Help",
    "This window is a raw map of the microdrive. It means that you can see positions on microdrive where data is stored but also "
    "zones that don't have any data (gaps, zones when the head has erased the data but not written anything)\n"
    "\n"
    "The map is refreshed continuosly, if you write data to the microdrive for example, the changes are shown on this window.\n"
    "\n"
    "Every position on the map has different color, depending on the type:\n"
    "- Data has blue color. If reading that Data, the color is cyan. If writing, the color is red\n"
    "- Gap has white color. If reading that Data, the color is bright white. If writing, the color is red\n"
    "- Bad position has magenta color. If reading that Data, the color is bright magenta. If writing, the color is red\n"
    "- Head position is shown in black color\n"
    "- Sync bytes have yellow color. If reading or writing, change to same color as Data. "
    "Sync bytes are this sequence of bytes: 00 00 00 00 00 00 FF FF. They are just like regular data, but they are used "
    "by the interface 1 rom to know when a sector begins or when the data in that sector begins\n"
    "\n"

    "There are several zoom levels which means:\n"
    "1X: Every position on the microdrive (1 byte or 1 gap) is shown by one pixel on the map\n"
    "2X: Every position on the microdrive is shown by a square of 2x2 on the map\n"
    "4X: Every position on the microdrive is shown by a square of 4x4 on the map\n"
    "8X: Every position on the microdrive is shown by a square of 8x8 on the map; "
    "this zoom level also allows to 'see' a byte representation: for example if byte at position has value 255 you will see a horizontal line "
    "of 8 pixels width, or if byte at position has value 128, you will see a pixel at the left part of the square\n"
    "16X: Every position on the microdrive is shown by a square of 16x16 on the map; "
    "this zoom level also shows you the byte value at position (hexadecimal or ascii)\n"
    "0.5X: Every 2 positions on the microdrive are shown by one pixel on the map\n"
    "0.25X: Every 4 positions on the microdrive are shown by one pixel on the map\n"
    "0.125X: Every 8 positions on the microdrive are shown by one pixel on the map\n"
    "0.0625X: Every 16 positions on the microdrive are shown by one pixel on the map\n"
    "\n"

    "The following keys are valid on the map:\n"
    "m: Select microdrive\n"
    "z: Zoom out\n"
    "x: Zoom in\n"
    "c: Show ascii character at position instead of hexadecimal (when zoom 16X)\n"
    "a: Autoscroll. Move map to where the microdrive head position is\n"
    "h: Show microdrive head position\n"
    "s: Highlight sync bytes\n"
    "\n"
    "Apart from these keys, by using the arrow keys, PgUp/Down, you can scroll the map conveniently.\n"
    );
}

//Almacenar la estructura de ventana aqui para que se pueda referenciar desde otros sitios
zxvision_window zxvision_window_microdrive_raw_map;


void menu_microdrive_raw_map(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();

    if (!menu_multitarea) {
        menu_warn_message("This window needs multitask enabled");
        return;
    }

    if (zxvision_find_window_in_background("visualmem")) {
        menu_warn_message("Visual Memory window is opened. It may generate bad behaviour on Microdrive Raw Map having that window opened");
    }

    zxvision_window *ventana;
    ventana=&zxvision_window_microdrive_raw_map;

	//IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
	//si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
	//la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
	//zxvision_delete_window_if_exists(ventana);

    //Crear ventana si no existe
    if (!zxvision_if_window_already_exists(ventana)) {
        int xventana,yventana,ancho_ventana,alto_ventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;

        if (!util_find_window_geometry("microdriverawmap",&xventana,&yventana,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {
            ancho_ventana=50;
            alto_ventana=30;

            xventana=menu_center_x()-ancho_ventana/2;
            yventana=menu_center_y()-alto_ventana/2;
        }


        zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"Microdrive Raw Map",
            "microdriverawmap",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);

        ventana->can_be_backgrounded=1;

        //No permitimos barras de scroll, el desplazamiento desde esta ventana se gestiona desplazando el mapa de microdrive
        ventana->can_be_scrolled=0;

        //Forzar visibles hotkeys en esa ventana
        ventana->writing_inverse_color=1;

    }

    //Si ya existe, activar esta ventana
    else {
        zxvision_activate_this_window(ventana);
    }

	zxvision_draw_window(ventana);

	z80_byte tecla;


	int salir=0;


    menu_microdrive_raw_map_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


    //cambio overlay
    zxvision_set_window_overlay(ventana,menu_microdrive_raw_map_overlay);


    //Toda ventana que este listada en zxvision_known_window_names_array debe permitir poder salir desde aqui
    //Se sale despues de haber inicializado overlay y de cualquier otra variable que necesite el overlay
    if (zxvision_currently_restoring_windows_on_start) {
            //printf ("Saliendo de ventana ya que la estamos restaurando en startup\n");
            return;
    }

    do {
        microdrive_raw_map_forzar_dibujado=1;
        zxvision_cls(ventana);

        menu_microdrive_raw_map_mostrar_opciones(ventana);

        zxvision_draw_window_contents(ventana);


        int total_alto=menu_microdrive_raw_map_lineas_dibujadas_final;
        if (total_alto<1) total_alto=1;

        //printf("Alto pagina: %d\n",total_alto);

        int aux_pgdnup;


		tecla=zxvision_common_getkey_refresh();


        switch (tecla) {

            case 'm':
                microdrive_raw_map_selected_unit++;
                if (microdrive_raw_map_selected_unit>=MAX_MICRODRIVES_BY_CONFIG) microdrive_raw_map_selected_unit=0;
                ventana->must_clear_cache_on_draw_once=1;
                //microdrive_raw_map_forzar_dibujado=1;
            break;

            case 'z':
                menu_microdrive_raw_map_reduce_zoom();
                //cambios de zoom hay que recalcular lo que ocupa total la pagina
                menu_microdrive_raw_map_lineas_dibujadas_final=0;
                ventana->must_clear_cache_on_draw_once=1;
                //recalcular lo que ocupa en lineas una pagina con el zoom actual (variable menu_microdrive_raw_map_lineas_dibujadas_final)
                //llamo desde aqui a la funcion de overlay
                menu_microdrive_raw_map_draw(ventana);
            break;


            case 'x':
                menu_microdrive_raw_map_increase_zoom();
                //cambios de zoom hay que recalcular lo que ocupa total la pagina
                menu_microdrive_raw_map_lineas_dibujadas_final=0;
                ventana->must_clear_cache_on_draw_once=1;
                //recalcular lo que ocupa en lineas una pagina con el zoom actual (variable menu_microdrive_raw_map_lineas_dibujadas_final)
                //llamo desde aqui a la funcion de overlay
                menu_microdrive_raw_map_draw(ventana);
            break;

            case 'a':
                microdrive_raw_map_autoscroll^=1;
            break;

            case 'c':
                microdrive_raw_map_draw_zoom_show_char ^=1;
            break;

            case 'h':
                microdrive_raw_map_dibujar_cabezal ^=1;
            break;

            case 's':
                microdrive_raw_map_dibujar_sync ^=1;
            break;

            case 'p':
                microdrive_raw_map_edit_index();
                ventana->must_clear_cache_on_draw_once=1;
            break;

            case MENU_TECLA_AYUDA:
                microdrive_raw_map_help();
            break;

            //Salir con ESC
            case 2:
                salir=1;
            break;

            //O tecla background
            case 3:
                salir=1;
            break;

            //izquierda
            case 8:

                menu_microdrive_raw_map_start_index--;
                if (menu_microdrive_raw_map_start_index<0) menu_microdrive_raw_map_start_index=0;

                ventana->must_clear_cache_on_draw_once=1;
                //microdrive_raw_map_forzar_dibujado=1;


            break;

            //Derecha
            case 9:
                menu_microdrive_raw_map_start_index++;
                if (menu_microdrive_raw_map_start_index>=microdrive_status[microdrive_raw_map_selected_unit].raw_total_size) {
                    menu_microdrive_raw_map_start_index=microdrive_status[microdrive_raw_map_selected_unit].raw_total_size-1;
                }
                ventana->must_clear_cache_on_draw_once=1;
                //microdrive_raw_map_forzar_dibujado=1;


            break;

            //abajo
            case 10:
                menu_microdrive_raw_map_abajo(ventana);
            break;

            //arriba
            case 11:
                menu_microdrive_raw_map_arriba(ventana);
            break;


            //PgDn
            case 25:
                for (aux_pgdnup=0;aux_pgdnup<total_alto;aux_pgdnup++) menu_microdrive_raw_map_abajo(ventana);
            break;

            //PgUp
            case 24:
                for (aux_pgdnup=0;aux_pgdnup<total_alto;aux_pgdnup++) menu_microdrive_raw_map_arriba(ventana);
            break;
        }


    } while (salir==0);


	util_add_window_geometry_compact(ventana);

	if (tecla==3) {
		zxvision_message_put_window_background();
	}

	else {

		zxvision_destroy_window(ventana);
	}


}

void menu_microdrive_raw_full_erase(MENU_ITEM_PARAMETERS)
{
    if (menu_confirm_yesno("Erase")) {
        microdrive_raw_full_erase(valor_opcion);
        menu_generic_message_splash("Erase","OK. Microdrive has been erased");
    }

}


void menu_microdrive_raw_enlarge(MENU_ITEM_PARAMETERS)
{

	int perc=2;

	if (menu_ventana_scanf_numero_enhanced("Stretch (%)",&perc,4,+1,1,100,0)>=0) {

        if (menu_confirm_yesno("Stretch")) {

            int parametro=100/perc;
            microdrive_raw_enlarge(valor_opcion,parametro);
            menu_generic_message_splash("Enlarge","OK. Microdrive has been stretched");

            //Por alguna razon despues de menu_ventana_scanf_numero_enhanced hay que forzar salir_todos_menus si quieremos que se cierren los menus
            salir_todos_menus=1;

        }

    }

}

void menu_interface1_raw_real_life_problems(MENU_ITEM_PARAMETERS)
{
    microdrive_raw_real_life_problems.v ^=1;
}

void menu_interface1_simulate_sound_read(MENU_ITEM_PARAMETERS)
{
    microdrive_simulate_sound_read.v ^=1;
}

void menu_interface1_simulate_sound_write(MENU_ITEM_PARAMETERS)
{
    microdrive_simulate_sound_write.v ^=1;
}

void menu_interface1_enable(MENU_ITEM_PARAMETERS)
{
	if (if1_enabled.v==0) enable_if1();
	else disable_if1();
}

void menu_interface1(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;


    do {

        menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,menu_interface1_enable,NULL,"~~Enable");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(if1_enabled.v ? 'X' : ' ' ));
        menu_add_item_menu_shortcut(array_menu_common,'e');


        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_interface1_rom_version,menu_interface1_rom_cond,"ROM selection");
        menu_add_item_menu_prefijo(array_menu_common,"    ");
        menu_add_item_menu_es_avanzado(array_menu_common);

        if (if1_custom_rom_enabled.v) {
            menu_add_item_menu_sufijo_format(array_menu_common," [Custom]");

            char string_if1_rom_file_shown[10];

            menu_tape_settings_trunc_name(if1_custom_rom_file, string_if1_rom_file_shown,10);
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface1_rom_file,NULL,
                "IF1 ROM File","Archivo ROM IF1","Arxiu ROM IF1");
            menu_add_item_menu_sufijo_format(array_menu_common," [%s]", string_if1_rom_file_shown);
            menu_add_item_menu_prefijo(array_menu_common,"    ");
            menu_add_item_menu_es_avanzado(array_menu_common);
        }
        else {
            menu_add_item_menu_sufijo_format(array_menu_common," [Version %d]",if1_rom_version);
            menu_add_item_menu_es_avanzado(array_menu_common);
        }

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface1_raw_real_life_problems,NULL,
            "Simulate real problems","Simular problemas reales","Simular problemes reals");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ", (microdrive_raw_real_life_problems.v ? 'X' : ' ' ));
        menu_add_item_menu_tooltip(array_menu_common,"When using RAW images (RMD) simulate bad behaviour like stretch and bad sectors");
        menu_add_item_menu_ayuda(array_menu_common,"When using RAW images (RMD) simulate bad behaviour like stretch and bad sectors. "
            "It counts the times the microdrive is read/written completely (usage counter) and then:\n"
            "- The first 50 times the microdrive can stretch 2% with a probability of 10%\n"
            "- Every 100 times, a position can be marked as bad with a probability of 5% "
        );
        menu_add_item_menu_es_avanzado(array_menu_common);

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface1_simulate_sound_read,NULL,
            "Simulate sound on read","Simular sonido en lectura","Simular so a lectura");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ", (microdrive_simulate_sound_read.v ? 'X' : ' ' ));
        menu_add_item_menu_tooltip(array_menu_common,"When using RAW images (RMD) simulate sound on read");
        menu_add_item_menu_ayuda(array_menu_common,"When using RAW images (RMD) simulate sound on read\n"
            "The real device don't generate data sound, but ZEsarUX can simulate it just for your enjoyment"
        );
        menu_add_item_menu_es_avanzado(array_menu_common);

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface1_simulate_sound_write,NULL,
            "Simulate sound on write","Simular sonido en escritura","Simular so a escriptura");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ", (microdrive_simulate_sound_write.v ? 'X' : ' ' ));
        menu_add_item_menu_tooltip(array_menu_common,"When using RAW images (RMD) simulate sound on write");
        menu_add_item_menu_ayuda(array_menu_common,"When using RAW images (RMD) simulate sound on write\n"
            "The real device don't generate data sound, but ZEsarUX can simulate it just for your enjoyment"
        );
        menu_add_item_menu_es_avanzado(array_menu_common);


        //De momento soportar hasta 4 microdrives en el menu , aunque se permiten hasta 8

        int i;
        for (i=0;i<MAX_MICRODRIVES_BY_CONFIG;i++) {

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"--Microdrive %d--",i+1);

            char string_microdrive_file_shown[17];


            menu_tape_settings_trunc_name(microdrive_status[i].microdrive_file_name,string_microdrive_file_shown,17);
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_storage_microdrive_file,NULL,
                "Microdrive File","Archivo Microdrive","Arxiu Microdrive");
            menu_add_item_menu_sufijo_format(array_menu_common," [%s]",string_microdrive_file_shown);
            menu_add_item_menu_prefijo(array_menu_common,"    ");
            menu_add_item_menu_valor_opcion(array_menu_common,i);
            menu_add_item_menu_tooltip(array_menu_common,"Microdrive Emulation file");
            menu_add_item_menu_ayuda(array_menu_common,"Microdrive Emulation file");


            //Truco para que parezca que se usa llamada a condicion de los 4 microdrives diferentes, sin
            //tener que crear 4 funciones diferentes de condicion
            if (microdrive_status[i].microdrive_file_name[0]) {
                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_storage_microdrive_enable,NULL,
                    "Microdrive Emulation","Emulación Microdrive","Emulació Microdrive");
            }

            else {
                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_storage_microdrive_enable,menu_interface1_cond_disabled,
                    "Microdrive Emulation","Emulación Microdrive","Emulació Microdrive");
            }

            menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ", (microdrive_status[i].microdrive_enabled ? 'X' : ' '));
            menu_add_item_menu_valor_opcion(array_menu_common,i);
            menu_add_item_menu_tooltip(array_menu_common,"Microdrive Emulation");
            menu_add_item_menu_ayuda(array_menu_common,"Microdrive Emulation");



            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_storage_microdrive_write_protection,NULL,
                "Write Protection","Protección escritura","Protecció escriptura");
            menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ", (microdrive_status[i].microdrive_write_protect ? 'X' : ' '));
            menu_add_item_menu_valor_opcion(array_menu_common,i);
            menu_add_item_menu_tooltip(array_menu_common,"Write Protection");
            menu_add_item_menu_ayuda(array_menu_common,"Write Protection");

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_storage_microdrive_persistent_writes,NULL,
                "Persistent Writes","Escrituras Persistentes","Escriptures Persistents");
            menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(microdrive_status[i].microdrive_persistent_writes ? 'X' : ' ') );
            menu_add_item_menu_valor_opcion(array_menu_common,i);
            menu_add_item_menu_tooltip(array_menu_common,"Tells if Microdrive writes are saved to disk");
            menu_add_item_menu_ayuda(array_menu_common,"Tells if Microdrive writes are saved to disk. "
                "Note: all writing operations to Microdrive are always saved to internal memory (unless you disable write permission), but this setting "
                "tells if these changes are written to disk or not."
                );

            if (microdrive_status[i].microdrive_enabled) {

                if (microdrive_status[i].raw_format==0) {
                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_storage_microdrive_browse,NULL,
                            "Microdrive browse","Explorar microdrive","Explorar microdrive");
                    menu_add_item_menu_prefijo(array_menu_common,"    ");
                    menu_add_item_menu_se_cerrara(array_menu_common);
                    menu_add_item_menu_genera_ventana(array_menu_common);
                    menu_add_item_menu_valor_opcion(array_menu_common,i);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_storage_microdrive_map,NULL,
                            "Microdrive map","Mapa microdrive","Mapa microdrive");
                    menu_add_item_menu_prefijo(array_menu_common,"    ");
                    menu_add_item_menu_se_cerrara(array_menu_common);
                    menu_add_item_menu_genera_ventana(array_menu_common);
                    menu_add_item_menu_valor_opcion(array_menu_common,i);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_storage_microdrive_sectors_info,NULL,
                            "Sectors info","Info Sectores","Info Sectors");
                    menu_add_item_menu_prefijo(array_menu_common,"    ");
                    menu_add_item_menu_se_cerrara(array_menu_common);
                    menu_add_item_menu_genera_ventana(array_menu_common);
                    menu_add_item_menu_valor_opcion(array_menu_common,i);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_storage_microdrive_chkdsk,NULL,
                            "Chkdsk","Chkdsk","Chkdsk");
                    menu_add_item_menu_prefijo(array_menu_common,"    ");
                    menu_add_item_menu_se_cerrara(array_menu_common);
                    menu_add_item_menu_genera_ventana(array_menu_common);
                    menu_add_item_menu_valor_opcion(array_menu_common,i);
                }

                else {
                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_microdrive_raw_map,NULL,
                            "Microdrive raw map","Mapa raw microdrive","Mapa raw microdrive");
                    menu_add_item_menu_prefijo(array_menu_common,"    ");
                    menu_add_item_menu_se_cerrara(array_menu_common);
                    menu_add_item_menu_genera_ventana(array_menu_common);
                    menu_add_item_menu_valor_opcion(array_menu_common,i);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_microdrive_raw_full_erase,NULL,
                            "Full erase","Borrado completo","Borrat complet");
                    menu_add_item_menu_prefijo(array_menu_common,"    ");
                    menu_add_item_menu_se_cerrara(array_menu_common);
                    menu_add_item_menu_genera_ventana(array_menu_common);
                    menu_add_item_menu_valor_opcion(array_menu_common,i);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_microdrive_raw_enlarge,NULL,
                            "Stretch","Estiramiento","Estirament");
                    menu_add_item_menu_prefijo(array_menu_common,"    ");
                    menu_add_item_menu_se_cerrara(array_menu_common);
                    menu_add_item_menu_genera_ventana(array_menu_common);
                    menu_add_item_menu_valor_opcion(array_menu_common,i);
                    menu_add_item_menu_es_avanzado(array_menu_common);
                }


                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_mdv_simulate_bad,NULL,
                        "Emulate bad sectors","Emular sectores erroneos","Emular sectors erronis");
                menu_add_item_menu_prefijo(array_menu_common,"    ");
                menu_add_item_menu_valor_opcion(array_menu_common,i);
                menu_add_item_menu_se_cerrara(array_menu_common);
                menu_add_item_menu_genera_ventana(array_menu_common);
                menu_add_item_menu_es_avanzado(array_menu_common);

            }

			menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_visual_microdrive_previo,NULL,
					"Visual Microdrive","Visual Microdrive","Visual Microdrive");
			menu_add_item_menu_prefijo(array_menu_common,"    ");
			menu_add_item_menu_se_cerrara(array_menu_common);
			menu_add_item_menu_genera_ventana(array_menu_common);
			menu_add_item_menu_valor_opcion(array_menu_common,i);


            menu_add_item_menu_separator(array_menu_common);

        }




        menu_add_ESC_item(array_menu_common);


        //Nota: si no se agrega el nombre del path del indice, se generará uno automáticamente
        menu_add_item_menu_index_full_path(array_menu_common,
            "Main Menu-> Storage-> IF1/Microdrive",
            "Menú Principal-> Almacenamiento-> IF1/Microdrive",
            "Menú Principal-> Emmagatzematge-> IF1/Microdrive");

        retorno_menu=menu_dibuja_menu(&interface1_opcion_seleccionada,&item_seleccionado,array_menu_common,
            "Menu IF1/Microdrive","Menú IF1/Microdrive","Menú IF1/Microdrive" );

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



zxvision_window *menu_visual_floppy_window;

#define MENU_VISUAL_FLOPPY_PISTAS 40
#define MENU_VISUAL_FLOPPY_SECTORES 9
#define MENU_VISUAL_FLOPPY_BYTES_SECTOR 512

//Angulo de rotacion actual que se aumenta si motor activo
int menu_visualfloppy_rotacion_disco=0;

//Si permitida rotacion
int menu_visualfloppy_rotacion_activada=1;

//rotacion con rpm real
int menu_visualfloppy_rotacion_real=1;

int menu_visualfloppy_header_visible=1;


int menu_visualfloppy_coloured_effects=1;

//Retorna en que radio está una pista concreta
int menu_visual_floppy_get_radio_pista(int pista,int radio_exterior_disco,int radio_interior_disco)
{
    int radio_usable=radio_exterior_disco-radio_interior_disco-1; //quitar 1 para no usar justo el radio exterior

    //pista 0 esta en la zona mas externa
    int radio_del_byte=(radio_usable*(MENU_VISUAL_FLOPPY_PISTAS-1-pista))/MENU_VISUAL_FLOPPY_PISTAS;

    radio_del_byte +=radio_interior_disco;

    //quitar 1 para no usar justo el radio exterior
    radio_del_byte--;

    return radio_del_byte;

}

//centro x,y, radios exterior, interior, pista (0..39), sector (0..8), byte en sector (0..511)
void menu_visual_floppy_putpixel_track_sector(int centro_disco_x,int centro_disco_y,
    int radio_interior_disco,int radio_exterior_disco,int pista,int sector,int byte_en_sector,int color)
{

    //si fuera limites, no hacer putpixel
    //No limito byte_en_sector, en caso de sectores de mas de 512 bytes, el pixel simplemente "saltara" al siguiente sector,
    //dado que tampoco voy a hacer una representacion super exacta del entorno, pero al menos, por ejemplo en sectores de 4kb, que se
    //vea bien que lee mas alla de 512 bytes
    if (pista>=MENU_VISUAL_FLOPPY_PISTAS || sector>=MENU_VISUAL_FLOPPY_SECTORES /*|| byte_en_sector>=MENU_VISUAL_FLOPPY_BYTES_SECTOR*/) {
        //printf("Error fuera limite\n");
        return;
    }

    //calcular el incremento de radio en la zona entre radio interior y exterior segun la pista
    int radio_del_byte=menu_visual_floppy_get_radio_pista(pista,radio_exterior_disco,radio_interior_disco);

    /*/

    int radio_usable=radio_exterior_disco-radio_interior_disco-1; //quitar 1 para no usar justo el radio exterior

    //pista 0 esta en la zona mas externa
    int radio_del_byte=(radio_usable*(MENU_VISUAL_FLOPPY_PISTAS-1-pista))/MENU_VISUAL_FLOPPY_PISTAS;

    radio_del_byte +=radio_interior_disco;

    //quitar 1 para no usar justo el radio exterior
    radio_del_byte--;

    */

    //calcular grados. Partiendo que sector 0 es grados 0
    int grados_por_sector=(360/MENU_VISUAL_FLOPPY_SECTORES);



    //Y sumarle lo relativo al byte en sector
    int incremento_sector=(grados_por_sector*byte_en_sector)/MENU_VISUAL_FLOPPY_BYTES_SECTOR;

    //Inicio del sector en:
    int grados_sector=grados_por_sector*sector;

    int grados_final=grados_sector+incremento_sector;

    //sumarle la rotacion disco
    grados_final +=menu_visualfloppy_rotacion_disco;

    //limitar a 360
    grados_final = grados_final % 360;

    //ya tenemos radio y grados. dibujar pixel
    int xdestino=centro_disco_x+((radio_del_byte*util_get_cosine(grados_final))/10000);
    int ydestino=centro_disco_y-((radio_del_byte*util_get_sine(grados_final))/10000);

    //if (byte_en_sector>470) printf("final: %d,%d grados: %d\n",xdestino,ydestino,grados_final);

    zxvision_putpixel(menu_visual_floppy_window,xdestino,ydestino,color);

}


void menu_visual_floppy_draw_header(int pista_actual,int centro_disco_x,int centro_disco_y,int radio_exterior_disco,int radio_fin_datos,int color)
{

    if (pista_actual<0) pista_actual=0;
    if (pista_actual>=MENU_VISUAL_FLOPPY_PISTAS) pista_actual=MENU_VISUAL_FLOPPY_PISTAS-1;

        int ancho_cabezal=radio_exterior_disco/20;

        int alto_cabezal=ancho_cabezal*2;

        int xcabezal=centro_disco_x-ancho_cabezal/2;

        //La zona por donde se mueve el cabezal
        //int recorrido_total_cabezal=radio_exterior_disco-radio_fin_datos-1;



        //int recorrido_actual_cabezal=(pista_actual*recorrido_total_cabezal)/MENU_VISUAL_FLOPPY_PISTAS;
        //pista 0 arriba del todo

        int recorrido_actual_cabezal=menu_visual_floppy_get_radio_pista(pista_actual,radio_exterior_disco,radio_fin_datos);

        int ycabezal=centro_disco_y-recorrido_actual_cabezal;

        /*
        Cabezal asi:

        ----------
          ------
          ------
          ------
          ------
          ------
          ------

        */

        int y;

            for (y=0;y<alto_cabezal;y++) {
                int xinicio=xcabezal;
                int xfinal=xcabezal+ancho_cabezal-1;

                //Si las dos lineas de arriba, hacerla un poco mas larga para marcar exacta la pista
                //Con alto 2, pues en posicion 90 grados, el circulo hace un "piquito" hacia arriba
                if (y==0) {
                    xinicio -=2;
                    xfinal +=2;
                }

                if (y==1) {
                    xinicio -=1;
                    xfinal +=1;
                }


                zxvision_draw_line(menu_visual_floppy_window,xinicio,ycabezal+y,xfinal,ycabezal+y,color,zxvision_putpixel);
            }

}

//Para indicar los sectores leidos, buffer
//buffer de 256kb.
#define MENU_VISUAL_FLOPPY_MAX_LENGTH_BUFFER 262144
int menu_visual_floppy_buffer_length=0;

#define MENU_VISUAL_FLOPPY_ROTATION_SPEED_NORMAL (360/10)
#define MENU_VISUAL_FLOPPY_ROTATION_SPEED_SLOW (360/50/2)


struct s_menu_visual_floppy_buffer {
    int pista;
    int sector;
    int byte_en_sector;
    int intensidad; //intensidad de color en porcentaje: 100%: cuando se agrega. Va bajando hasta 0
    int persistent; //Si no 0, no decrementa el color
};

struct s_menu_visual_floppy_buffer menu_visual_floppy_buffer[MENU_VISUAL_FLOPPY_MAX_LENGTH_BUFFER];

void menu_visual_floppy_buffer_reset(void)
{
    menu_visual_floppy_buffer_length=0;
}

void menu_visual_floppy_buffer_add_common(int pista,int sector,int byte_en_sector,int persistent)
{
    if (menu_visual_floppy_buffer_length>=MENU_VISUAL_FLOPPY_MAX_LENGTH_BUFFER) {
        //printf("Visual floppy buffer is full\n");
        return;
    }
    //printf("add to buffer %d %d %d\n",pista,sector,byte_en_sector);

    menu_visual_floppy_buffer[menu_visual_floppy_buffer_length].pista=pista;
    menu_visual_floppy_buffer[menu_visual_floppy_buffer_length].sector=sector;
    menu_visual_floppy_buffer[menu_visual_floppy_buffer_length].byte_en_sector=byte_en_sector;
    menu_visual_floppy_buffer[menu_visual_floppy_buffer_length].intensidad=100;
    menu_visual_floppy_buffer[menu_visual_floppy_buffer_length].persistent=persistent;

    menu_visual_floppy_buffer_length++;
}

void menu_visual_floppy_buffer_add(int pista,int sector,int byte_en_sector)
{

    menu_visual_floppy_buffer_add_common(pista,sector,byte_en_sector,0);

}

void menu_visual_floppy_buffer_add_persistent(int pista,int sector,int byte_en_sector)
{

    menu_visual_floppy_buffer_add_common(pista,sector,byte_en_sector,1);

}

void menu_visual_floppy_dibujar_index_hole(int centro_disco_x,int centro_disco_y,
    int radio_exterior_disco,int radio_interior_disco,int radio_fin_datos,int color,int grados)
{
            //El Index Hole
        //Entre Interior y principio datos
        int posicion_index_hole=(radio_fin_datos-radio_interior_disco/2);

        //Proporcion como siempre del total
        int radio_index_hole=radio_exterior_disco/20;

       // int index_hole_x=centro_disco_x+posicion_index_hole;
        //int index_hole_y=centro_disco_y;

        int index_hole_x=centro_disco_x+((posicion_index_hole*util_get_cosine(grados))/10000);
        int index_hole_y=centro_disco_y-((posicion_index_hole*util_get_sine(grados))/10000);


        zxvision_draw_ellipse(menu_visual_floppy_window,index_hole_x,index_hole_y,
            radio_index_hole,radio_index_hole,color,
            zxvision_putpixel,360);
}

void menu_visual_floppy_draw_arrow(int centro_disco_x,int centro_disco_y,int radio_exterior_disco,int color)
{
    int indicador_rotacion_radio=radio_exterior_disco/3;
    int indicador_rotacion_x=centro_disco_x-radio_exterior_disco-indicador_rotacion_radio/2;
    int indicador_rotacion_y=centro_disco_y;
    zxvision_draw_arc(menu_visual_floppy_window,indicador_rotacion_x,indicador_rotacion_y,
        indicador_rotacion_radio,indicador_rotacion_radio,color,
        zxvision_putpixel,90,180);

    int final_flecha_x=indicador_rotacion_x-indicador_rotacion_radio;
    int final_flecha_y=indicador_rotacion_y;
    int longitud_flecha=indicador_rotacion_radio/2;
    zxvision_draw_line(menu_visual_floppy_window,final_flecha_x,final_flecha_y,
        final_flecha_x-longitud_flecha/2+1,final_flecha_y-longitud_flecha+1,color,zxvision_putpixel);
    zxvision_draw_line(menu_visual_floppy_window,final_flecha_x,final_flecha_y,
        final_flecha_x+longitud_flecha-1,final_flecha_y-longitud_flecha/2+1,color,zxvision_putpixel);
}

//Contador de segundo para hacer que el overlay solo se redibuje un numero de veces por segundo y no siempre
int menu_visual_floppy_contador_segundo_anterior;

//Si se ha llenado el fondo con espacios del color de fondo esperado
//int menu_visual_floppy_fondo_asignado=0;

//Ultimo valor de RPM de rotacion
int menu_visualfloppy_last_rpm=0;

//Esto se llama desde el timer
void menu_visualfloppy_increment_rotation(void)
{

    //Asumimos por defecto rpm a 0
    menu_visualfloppy_last_rpm=0;

    if (!menu_visualfloppy_rotacion_activada) {
        menu_visualfloppy_rotacion_disco=0;
    }


    else if (pd765_motor_speed) {

        int incremento_grados;

        if (menu_visualfloppy_rotacion_real) {
            //Para que cada 10 frames rote una vez entera
            //Por tanto en 1 segundo gira 5 veces
            incremento_grados=MENU_VISUAL_FLOPPY_ROTATION_SPEED_NORMAL;

            //para que visualmente el ojo siempre perciba movimiento hacia la izquierda (o al menos, que no detecte movimiento de marcas de sectores a la derecha)
            //incremento_grados+=4;
            //porque por una parte tenemos que deberiamos hacer, en 10 frames, una vuelta completa (o sea, incremento_grados=360/10)
            //pero pero otra, hay 9 marcas de sector (y no 10) que al moverse cada frame parece que se mueven a la derecha
            //Al final en 1 segundo habremos dado 5.5 vueltas en vez de 5, o sea, en vez de 300 rpm iremos a 333 rpm
            //incremento_grados=360/MENU_VISUAL_FLOPPY_PISTAS;

        }

        else {

            //rotar bastante menos. 1 vuelta cada 2 segundos
            incremento_grados=MENU_VISUAL_FLOPPY_ROTATION_SPEED_SLOW;
        }

        //Aplicarle la velocidad relativa del motor
        incremento_grados=(incremento_grados*pd765_motor_speed)/100;

        //Sabemos que velocidad no es 0%, por tanto algo se tiene que mover. Si incremento=0, al menos ponemos 1 de incremento
        //if (incremento_grados==0) incremento_grados=1;

        //printf("speed: %d %%\n",pd765_motor_speed);

        menu_visualfloppy_rotacion_disco +=incremento_grados;

        //300 rpm velocidad maxima
        menu_visualfloppy_last_rpm=(300*pd765_motor_speed)/100;

    }

    //rotacion simulada betadisk, sin aceleración
    else {
        if (betadisk_simulated_motor && betadisk_enabled.v) {
            int incremento_grados;
            incremento_grados=MENU_VISUAL_FLOPPY_ROTATION_SPEED_NORMAL;
            menu_visualfloppy_rotacion_disco +=incremento_grados;

            //Valor fijo completamente falso
            menu_visualfloppy_last_rpm=300;
        }
    }

    //limitar siempre a 360
    menu_visualfloppy_rotacion_disco = menu_visualfloppy_rotacion_disco % 360;


    //printf("###Rotacion: %d\n",menu_visualfloppy_rotacion_disco);

}

#define MENU_VISUALFLOPPY_TOTAL_COLORES_MARCAS 12
const int menu_visual_floppy_colores_marcas[MENU_VISUALFLOPPY_TOTAL_COLORES_MARCAS]={
    1, 2, 3, 4, 5, 6,  //evitar negro y blanco
    9,10,11,12,13,14  //evitar negro y blanco de brillo
};

//color de la flecha indicadora de rotacion
int menu_visualfloppy_color_rotacion=1;

int last_menu_visual_floppy_buffer_length=0;

int last_menu_visualfloppy_rotacion_disco=0;

int last_pd765_motor_status=0;

void menu_visual_floppy_overlay(void)
{



    menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (menu_visual_floppy_window->is_minimized) return;



        menu_visual_floppy_contador_segundo_anterior=contador_segundo;




        //Print....
        //Tambien contar si se escribe siempre o se tiene en cuenta contador_segundo...

        int ancho_ventana_pixeles=(menu_visual_floppy_window->visible_width)*menu_char_width;
        int alto_ventana_pixeles=(menu_visual_floppy_window->visible_height-2)*menu_char_height;


        //printf("ancho %d alto %d\n",ancho_ventana_pixeles,alto_ventana_pixeles);

        int centro_disco_x=ancho_ventana_pixeles/2;
        int centro_disco_y=alto_ventana_pixeles/2;

        int color_contorno_disco=7;

        //Elegir el radio como el menor de las dimensiones ancho, alto
        int radio_exterior_disco;

        if (ancho_ventana_pixeles<alto_ventana_pixeles) radio_exterior_disco=ancho_ventana_pixeles/2;
        else radio_exterior_disco=alto_ventana_pixeles/2;

        //quitarle 2 caracter de radio. Fijo a 8 pixeles que es el maximo de un caracter en ancho o alto
        radio_exterior_disco -=(8*2);

        int radio_interior_disco=radio_exterior_disco/6;

        //Resto de dimensiones van relativas a radio_exterior_disco
        //radio exterior entre 6 partes
        //radio interior es 1/6 del exterior
        //margen del interior hasta datos: otro 1/6
        //del interior hasta el exterior quedan 5/6
        //ahi ubicaremos sectores 0..39

        int radio_fin_datos=radio_interior_disco*2;


        //int color_byte_sector=0;

        int byte_en_sector;
        int sector;
        int pista;

        int hay_cambios=1;

        //El bloque de borrado tarda hasta 9 milisegundos (que es mucho, dado que esto se ejecuta en cada frame de video,
        //y el maximo que podemos tardar en un frame de video, contando absolutamente todo, es 20 milisegundos)
        //por tanto es importante hacerlo solo cuando es necesario,
        //o sea, no hacerlo cuando:
        //no hay datos en el buffer a mostrar (ni la vez anterior tampoco), o sea =0,
        //y
        //rotacion disco no ha cambiado desde la vez anterior
        if (menu_visual_floppy_buffer_length==0 &&
            last_menu_visual_floppy_buffer_length==menu_visual_floppy_buffer_length &&
            last_menu_visualfloppy_rotacion_disco==menu_visualfloppy_rotacion_disco &&
            last_pd765_motor_status==pd765_motor_status
            ) {
            //printf("no hay cambios\n");
            hay_cambios=0;
        }

        last_menu_visualfloppy_rotacion_disco=menu_visualfloppy_rotacion_disco;
        last_menu_visual_floppy_buffer_length=menu_visual_floppy_buffer_length;
        last_pd765_motor_status=pd765_motor_status;

        //Si motor moviendose
        //if (pd765_motor_status) {
        //    hay_cambios=1;
        //}

        int i,r;


        if (hay_cambios) {

            //borrar todo el contenido de los circulos
            //alternativa mediante circulos desde interior hasta exterior
            for (r=0;r<radio_exterior_disco;r++) {
                zxvision_draw_ellipse(menu_visual_floppy_window,centro_disco_x,centro_disco_y,
                r,r,HEATMAP_INDEX_FIRST_COLOR,
                zxvision_putpixel,360);
            }

            //borrar posibles posiciones de cabezal
            for (i=0;i<MENU_VISUAL_FLOPPY_PISTAS;i++) {
                menu_visual_floppy_draw_header(i,centro_disco_x,centro_disco_y,radio_exterior_disco,radio_fin_datos,HEATMAP_INDEX_FIRST_COLOR);
            }

            //borrar posiciones de index holes
            for (i=0;i<360;i++) {
                menu_visual_floppy_dibujar_index_hole(centro_disco_x,centro_disco_y,radio_exterior_disco,radio_interior_disco,
                    radio_fin_datos,HEATMAP_INDEX_FIRST_COLOR,i);
            }


            //Borrar flecha indicadora de rotación
            menu_visual_floppy_draw_arrow(centro_disco_x,centro_disco_y,radio_exterior_disco,HEATMAP_INDEX_FIRST_COLOR);

        }



        int total_bytes_mostrados=0;

        int intensidad;

        int persistent;

        int ultimo_color_no_cero=-1;

        //printf("menu_visual_floppy_buffer_length: %d\n",menu_visual_floppy_buffer_length);

        for (i=0;i<menu_visual_floppy_buffer_length;i++) {
            pista=menu_visual_floppy_buffer[i].pista;
            sector=menu_visual_floppy_buffer[i].sector;
            byte_en_sector=menu_visual_floppy_buffer[i].byte_en_sector;
            intensidad=menu_visual_floppy_buffer[i].intensidad;
            persistent=menu_visual_floppy_buffer[i].persistent;

            if (intensidad!=0) {
                total_bytes_mostrados++;
                ultimo_color_no_cero=i;
            }

            //calcular color heatmap
            //#define HEATMAP_INDEX_FIRST_COLOR (TSCONF_INDEX_FIRST_COLOR+TSCONF_TOTAL_PALETTE_COLOURS)
            //#define HEATMAP_TOTAL_PALETTE_COLOURS 256
            int incremento_color=((HEATMAP_TOTAL_PALETTE_COLOURS-1)*intensidad)/100;
            //por si acaso
            if (incremento_color>=HEATMAP_TOTAL_PALETTE_COLOURS-1) incremento_color=HEATMAP_TOTAL_PALETTE_COLOURS-1;
            int color=HEATMAP_INDEX_FIRST_COLOR+incremento_color;


            //printf("%d %d,%d,%d\n",i,pista,sector,byte_en_sector);

            menu_visual_floppy_putpixel_track_sector(centro_disco_x,centro_disco_y,radio_fin_datos,radio_exterior_disco,
            pista,sector,byte_en_sector,color);

            //Y decrementar intensidad, si no esta persistente
            if (!persistent && intensidad>0) intensidad--;
            menu_visual_floppy_buffer[i].intensidad=intensidad;

        }

        //Ajustar el tamaño total al ultimo valor no cero registrado
        menu_visual_floppy_buffer_length=ultimo_color_no_cero+1;

        //if (ultimo_color_no_cero!=-1) printf("ultimo_color_no_cero: %d total: %d\n",ultimo_color_no_cero,menu_visual_floppy_buffer_length);

        //printf("\n");

        //Exterior
        zxvision_draw_ellipse(menu_visual_floppy_window,centro_disco_x,centro_disco_y,
            radio_exterior_disco,radio_exterior_disco,color_contorno_disco,
            zxvision_putpixel,360);



        //Interior
        zxvision_draw_ellipse(menu_visual_floppy_window,centro_disco_x,centro_disco_y,
            radio_interior_disco,radio_interior_disco,color_contorno_disco,
            zxvision_putpixel,360);


        //El Index Hole
        menu_visual_floppy_dibujar_index_hole(centro_disco_x,centro_disco_y,radio_exterior_disco,radio_interior_disco,
            radio_fin_datos,color_contorno_disco,menu_visualfloppy_rotacion_disco);


        //Flecha indicadora de rotación
        if (pd765_motor_status) {
            int color=7;
            if (menu_visualfloppy_coloured_effects) {
                color=menu_visualfloppy_color_rotacion;
                menu_visualfloppy_color_rotacion++;
                if (menu_visualfloppy_color_rotacion>=8) menu_visualfloppy_color_rotacion=1;
            }
            menu_visual_floppy_draw_arrow(centro_disco_x,centro_disco_y,radio_exterior_disco,color);
        }



   //Marcas sectores
        //prueba borrar primero todo
        //alternativa mediante: voy a dibujar todo pista, sector y byte
        for (pista=0;pista<MENU_VISUAL_FLOPPY_PISTAS;pista++) {

            for (sector=0;sector<MENU_VISUAL_FLOPPY_SECTORES;sector++) {


            //centro x,y, radios exterior, interior, pista (0..39), sector (0..8), byte en sector (0..511)
            int color_marca=1;

            if (menu_visualfloppy_coloured_effects) {

                int indice_color=sector % MENU_VISUALFLOPPY_TOTAL_COLORES_MARCAS;

                color_marca=menu_visual_floppy_colores_marcas[indice_color];

            }

            menu_visual_floppy_putpixel_track_sector(centro_disco_x,centro_disco_y,radio_fin_datos,radio_exterior_disco,
                pista,sector,0,color_marca);


            }

        }


        //Dibujar cabezal
        if (menu_visualfloppy_header_visible) {
            //Calcular ancho cabezal
            int pista_actual=pd765_pcn;
            menu_visual_floppy_draw_header(pista_actual,centro_disco_x,centro_disco_y,radio_exterior_disco,radio_fin_datos,6);

        }


            //esto de momento no menu_visual_floppy_buffer_reset();


            //zxvision_draw_window_contents(menu_visual_floppy_window);


    //}

    //la rotacion del disco que se simula en esta ventana se hace desde el timer

    int linea_contador_bytes=2;
    //borrar linea donde hay el contador de total bytes
    zxvision_fill_width_spaces_paper(menu_visual_floppy_window,linea_contador_bytes,HEATMAP_INDEX_FIRST_COLOR);
    if (total_bytes_mostrados) {
        zxvision_print_string_defaults_format(menu_visual_floppy_window,1,linea_contador_bytes,"Bytes shown: %d",total_bytes_mostrados);
    }

    //borrar linea donde hay el contador de rpm
    //printf("%d\n",menu_visualfloppy_last_rpm);
    zxvision_fill_width_spaces_paper(menu_visual_floppy_window,linea_contador_bytes+1,HEATMAP_INDEX_FIRST_COLOR);
    if (menu_visualfloppy_last_rpm!=0) {
        zxvision_print_string_defaults_format(menu_visual_floppy_window,1,linea_contador_bytes+1,"RPM: %d",menu_visualfloppy_last_rpm);
    }

    zxvision_draw_window_contents(menu_visual_floppy_window);

}




//Almacenar la estructura de ventana aqui para que se pueda referenciar desde otros sitios
zxvision_window zxvision_window_visual_floppy;

void menu_visual_floppy_rotation(MENU_ITEM_PARAMETERS)
{
    menu_visualfloppy_rotacion_activada ^=1;
}

void menu_visual_floppy_rotation_real(MENU_ITEM_PARAMETERS)
{
    menu_visualfloppy_rotacion_real ^=1;
}

void menu_visual_floppy_switch_header(MENU_ITEM_PARAMETERS)
{
	menu_visualfloppy_header_visible ^=1;
}

void menu_visualfloppy_switch_coloured_effects(MENU_ITEM_PARAMETERS)
{
    menu_visualfloppy_coloured_effects ^=1;
}

void menu_visual_floppy(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();

    if (!menu_multitarea) {
        menu_warn_message("This window needs multitask enabled");
        return;
    }

    zxvision_window *ventana;
    ventana=&zxvision_window_visual_floppy;

	//IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
	//si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
	//la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
	//zxvision_delete_window_if_exists(ventana);

    //Crear ventana si no existe
    if (!zxvision_if_window_already_exists(ventana)) {


        int xventana,yventana,ancho_ventana,alto_ventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;

        if (!util_find_window_geometry("visualfloppy",&xventana,&yventana,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {
            ancho_ventana=30;
            alto_ventana=20;

            xventana=menu_center_x()-ancho_ventana/2;
            yventana=menu_center_y()-alto_ventana/2;
        }


        zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"Visual Floppy",
            "visualfloppy",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);

        ventana->can_be_backgrounded=1;

        //definir color de papel de fondo
        ventana->default_paper=HEATMAP_INDEX_FIRST_COLOR;
        zxvision_cls(ventana);

    }

    //Si ya existe, activar esta ventana
    else {

        zxvision_activate_this_window(ventana);
    }

	zxvision_draw_window(ventana);




    menu_visual_floppy_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui



    //cambio overlay
    zxvision_set_window_overlay(ventana,menu_visual_floppy_overlay);

    //forzar a escribir el fondo desde overlay
    //menu_visual_floppy_fondo_asignado=0;


    //Toda ventana que este listada en zxvision_known_window_names_array debe permitir poder salir desde aqui
    //Se sale despues de haber inicializado overlay y de cualquier otra variable que necesite el overlay
    if (zxvision_currently_restoring_windows_on_start) {
            //printf ("Saliendo de ventana ya que la estamos restaurando en startup\n");
            return;
    }

    	menu_item *array_menu_debug_new_visualfloppy;
	menu_item item_seleccionado;
	int retorno_menu;
	do {

        //forzar a escribir el fondo desde overlay
        //menu_visual_floppy_fondo_asignado=0;

        //borrar primera linea por si conmuta parametro rotacion
        zxvision_fill_width_spaces_paper(ventana,0,HEATMAP_INDEX_FIRST_COLOR);

		menu_add_item_menu_inicial_format(&array_menu_debug_new_visualfloppy,MENU_OPCION_NORMAL,menu_visual_floppy_rotation,NULL
            ,"[%c] ~~Rotation",(menu_visualfloppy_rotacion_activada ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_debug_new_visualfloppy,'r');
		menu_add_item_menu_ayuda(array_menu_debug_new_visualfloppy,"Disable rotation");
		menu_add_item_menu_tabulado(array_menu_debug_new_visualfloppy,1,0);


        if (menu_visualfloppy_rotacion_activada) {
            menu_add_item_menu_format(array_menu_debug_new_visualfloppy,MENU_OPCION_NORMAL,menu_visual_floppy_rotation_real,NULL
                ,"[%c] R~~eal Rotation",(menu_visualfloppy_rotacion_real ? 'X' : ' '));
            menu_add_item_menu_shortcut(array_menu_debug_new_visualfloppy,'e');
            menu_add_item_menu_ayuda(array_menu_debug_new_visualfloppy,"Show real speed of rotation");
            menu_add_item_menu_tabulado(array_menu_debug_new_visualfloppy,15,0);
        }

		menu_add_item_menu_format(array_menu_debug_new_visualfloppy,MENU_OPCION_NORMAL,menu_visual_floppy_switch_header,NULL
            ,"[%c] He~~ad",(menu_visualfloppy_header_visible ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_debug_new_visualfloppy,'a');
		menu_add_item_menu_ayuda(array_menu_debug_new_visualfloppy,"Show header");
		menu_add_item_menu_tabulado(array_menu_debug_new_visualfloppy,1,1);


		menu_add_item_menu_format(array_menu_debug_new_visualfloppy,MENU_OPCION_NORMAL,menu_visualfloppy_switch_coloured_effects,NULL
            ,"[%c] ~~Color effects",(menu_visualfloppy_coloured_effects ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_debug_new_visualfloppy,'a');
		menu_add_item_menu_ayuda(array_menu_debug_new_visualfloppy,"Show tracks divisions in different colours");
		menu_add_item_menu_tabulado(array_menu_debug_new_visualfloppy,15,1);



		//Nombre de ventana solo aparece en el caso de stdout
		retorno_menu=menu_dibuja_menu_no_title_lang(&visualfloppy_opcion_seleccionada,&item_seleccionado,array_menu_debug_new_visualfloppy,"Visual floppy" );

		if (retorno_menu!=MENU_RETORNO_BACKGROUND) {
            //En caso de menus tabulados, es responsabilidad de este de borrar la ventana
            //Con este cls provoca que se borren todas las otras ventanas en background


            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    if (item_seleccionado.menu_funcion!=NULL) {
                            //printf ("actuamos por funcion\n");
                            item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);


                    }
            }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus && retorno_menu!=MENU_RETORNO_BACKGROUND);





	util_add_window_geometry_compact(ventana);

	if (retorno_menu==MENU_RETORNO_BACKGROUND) {
		zxvision_message_put_window_background();
	}

	else {

		zxvision_destroy_window(ventana);
	}


}




void menu_storage_betadisk_emulation(MENU_ITEM_PARAMETERS)
{
	if (betadisk_enabled.v) betadisk_disable();
	else betadisk_enable();
}

void menu_storage_betadisk_allow_boot(MENU_ITEM_PARAMETERS)
{
	betadisk_allow_boot_48k.v ^=1;
}


void menu_storage_trd_emulation(MENU_ITEM_PARAMETERS)
{
	if (trd_enabled.v) trd_disable();
	else trd_enable();
}

void menu_storage_trd_write_protect(MENU_ITEM_PARAMETERS)
{
	trd_write_protection.v ^=1;
}

int menu_storage_trd_emulation_cond(void)
{
        if (trd_file_name[0]==0) return 0;
        else return 1;
}


void menu_storage_trd_file(MENU_ITEM_PARAMETERS)
{

	trd_disable();

        char *filtros[2];

        filtros[0]="trd";
        filtros[1]=0;

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

              //Obtenemos directorio de trd
        //si no hay directorio, vamos a rutas predefinidas
        if (trd_file_name[0]==0) menu_chdir_sharedfiles();

        else {
                char directorio[PATH_MAX];
                util_get_dir(trd_file_name,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        zvfs_chdir(directorio);
                }
        }


        int ret=menu_filesel("Select TRD File",filtros,trd_file_name);
        //volvemos a directorio inicial
        zvfs_chdir(directorio_actual);

        if (ret==1) {
		if (!si_existe_archivo(trd_file_name)) {
			if (menu_confirm_yesno_texto("File does not exist","Create?")==0) {
                                trd_file_name[0]=0;
                                return;
                        }


			//Crear archivo vacio
		        FILE *ptr_trdfile;
			ptr_trdfile=fopen(trd_file_name,"wb");

		        long long int totalsize=640*1024;

			z80_byte valor_grabar=0;

		        if (ptr_trdfile!=NULL) {
				while (totalsize) {
					fwrite(&valor_grabar,1,1,ptr_trdfile);
					totalsize--;
				}
		                fclose(ptr_trdfile);
		        }

		}

		trd_enable();



        }
        //Sale con ESC
        else {
                //Quitar nombre
                trd_file_name[0]=0;


        }
}


void menu_storage_trd_browser(MENU_ITEM_PARAMETERS)
{
	//menu_file_trd_browser_show(trd_file_name,"TRD");
	menu_file_viewer_read_file("TRD file viewer",trd_file_name);

    //por coherencia cerrar menus al salir de aqui
    salir_todos_menus=1;
}


void menu_storage_trd_persistent_writes(MENU_ITEM_PARAMETERS)
{
	trd_persistent_writes.v ^=1;
}


void menu_betadisk(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_betadisk;
    menu_item item_seleccionado;
    int retorno_menu;
    do {


        char string_trd_file_shown[17];


        menu_tape_settings_trunc_name(trd_file_name,string_trd_file_shown,17);
        menu_add_item_menu_en_es_ca_inicial(&array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_trd_file,NULL,
            "~~TRD File","Archivo ~~TRD","Arxiu ~~TRD");
        menu_add_item_menu_sufijo_format(array_menu_betadisk," [%s]",string_trd_file_shown);
        menu_add_item_menu_prefijo(array_menu_betadisk,"    ");
        menu_add_item_menu_shortcut(array_menu_betadisk,'t');
        menu_add_item_menu_tooltip(array_menu_betadisk,"TRD Emulation file");
        menu_add_item_menu_ayuda(array_menu_betadisk,"TRD Emulation file");


        menu_add_item_menu_en_es_ca(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_trd_emulation,menu_storage_trd_emulation_cond,
            "TRD ~~Emulation","~~Emulación TRD","~~Emulació TRD");
        menu_add_item_menu_prefijo_format(array_menu_betadisk,"[%c] ", (trd_enabled.v ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_betadisk,'e');
        menu_add_item_menu_tooltip(array_menu_betadisk,"TRD Emulation");
        menu_add_item_menu_ayuda(array_menu_betadisk,"TRD Emulation");


        menu_add_item_menu_en_es_ca(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_trd_write_protect,NULL,
            "Wr~~ite protect","Protección escr~~itura","Protecció escr~~iptura");
        menu_add_item_menu_prefijo_format(array_menu_betadisk,"[%c] ", (trd_write_protection.v ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_betadisk,'i');
        menu_add_item_menu_tooltip(array_menu_betadisk,"If TRD disk is write protected");
        menu_add_item_menu_ayuda(array_menu_betadisk,"If TRD disk is write protected");


        menu_add_item_menu_en_es_ca(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_trd_persistent_writes,NULL,
            "Persistent Writes","Escrituras Persistentes","Escriptures Persistents");
        menu_add_item_menu_prefijo_format(array_menu_betadisk,"[%c] ",(trd_persistent_writes.v ? 'X' : ' ') );
        menu_add_item_menu_tooltip(array_menu_betadisk,"Tells if TRD writes are saved to disk");
        menu_add_item_menu_ayuda(array_menu_betadisk,"Tells if TRD writes are saved to disk. "
        "Note: all writing operations to TRD are always saved to internal memory (unless you disable write permission), but this setting "
        "tells if these changes are written to disk or not."
        );



        menu_add_item_menu(array_menu_betadisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);


        menu_add_item_menu_en_es_ca(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_betadisk_emulation,NULL,
            "Betadis~~k Enabled","Betadis~~k Activado","Betadis~~k Activat");
        menu_add_item_menu_prefijo_format(array_menu_betadisk,"[%c] ", (betadisk_enabled.v ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_betadisk,'k');
        menu_add_item_menu_tooltip(array_menu_betadisk,"Enable betadisk");
        menu_add_item_menu_ayuda(array_menu_betadisk,"Enable betadisk");


        menu_add_item_menu_en_es_ca(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_betadisk_allow_boot,NULL,
            "Allow ~~Boot","Permitir ~~Boot","Permetre ~~Boot");
        menu_add_item_menu_prefijo_format(array_menu_betadisk,"[%c] ", (betadisk_allow_boot_48k.v ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_betadisk,'b');
        menu_add_item_menu_tooltip(array_menu_betadisk,"Allow autoboot on 48k machines");
        menu_add_item_menu_ayuda(array_menu_betadisk,"Allow autoboot on 48k machines");

        menu_add_item_menu(array_menu_betadisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_item_menu_en_es_ca(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_trd_browser,menu_storage_trd_emulation_cond,
            "TRD ~~Viewer","~~Visor TRD","~~Visor TRD");
        menu_add_item_menu_prefijo(array_menu_betadisk,"    ");
        menu_add_item_menu_shortcut(array_menu_betadisk,'v');
        menu_add_item_menu_genera_ventana(array_menu_betadisk);
        menu_add_item_menu_tooltip(array_menu_betadisk,"TRD Viewer");
        menu_add_item_menu_ayuda(array_menu_betadisk,"TRD Viewer");

        menu_add_item_menu_en_es_ca(array_menu_betadisk,MENU_OPCION_NORMAL,menu_visual_floppy,NULL,
            "Visual Floppy","Visual Floppy","Visual Floppy");
        menu_add_item_menu_prefijo(array_menu_betadisk,"    ");
        menu_add_item_menu_se_cerrara(array_menu_betadisk);
        menu_add_item_menu_genera_ventana(array_menu_betadisk);

        menu_add_item_menu(array_menu_betadisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_ESC_item(array_menu_betadisk);

        retorno_menu=menu_dibuja_menu_no_title_lang(&betadisk_opcion_seleccionada,&item_seleccionado,array_menu_betadisk,"Betadisk" );


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


void menu_storage_zxpand_enable(MENU_ITEM_PARAMETERS)
{
	if (zxpand_enabled.v) zxpand_disable();
	else zxpand_enable();
}

void menu_storage_zxpand_root_dir(MENU_ITEM_PARAMETERS)
{

	int ret;
	ret=menu_storage_string_root_dir(zxpand_root_dir);

	//Si sale con ESC
	if (ret==0) {
       	//directorio zxpand vacio
        zxpand_cwd[0]=0;
	}

}


void menu_zxpand(MENU_ITEM_PARAMETERS)
{



        //Dado que es una variable local, siempre podemos usar este nombre array_menu_common
        menu_item *array_menu_common;
        menu_item item_seleccionado;
        int retorno_menu;
        do {



			menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,menu_storage_zxpand_enable,NULL,"[%c] ZX~~pand emulation",(zxpand_enabled.v ? 'X' : ' ') );
                        menu_add_item_menu_shortcut(array_menu_common,'p');
			menu_add_item_menu_tooltip(array_menu_common,"Enable ZXpand emulation");
			menu_add_item_menu_ayuda(array_menu_common,"Enable ZXpand emulation");


			if (zxpand_enabled.v) {
				char string_zxpand_root_folder_shown[20];
				menu_tape_settings_trunc_name(zxpand_root_dir,string_zxpand_root_folder_shown,20);

				menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_storage_zxpand_root_dir,NULL,"~~Root dir: %s",string_zxpand_root_folder_shown);
                menu_add_item_menu_prefijo(array_menu_common,"    ");
                        	menu_add_item_menu_shortcut(array_menu_common,'r');
				menu_add_item_menu_tooltip(array_menu_common,"Sets the root directory for ZXpand filesystem");
				menu_add_item_menu_ayuda(array_menu_common,"Sets the root directory for ZXpand filesystem. "
					"Only file and folder names valid for zxpand will be shown:\n"
					"-Maximum 8 characters for name and 3 for extension\n"
					"-Files and folders will be shown always in uppercase. Folders which are not uppercase, are shown but can not be accessed\n"
					);

			}



			menu_add_item_menu_separator(array_menu_common);

            menu_add_ESC_item(array_menu_common);

            retorno_menu=menu_dibuja_menu_no_title_lang(&zxpand_opcion_seleccionada,&item_seleccionado,array_menu_common,"ZXpand" );


                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


}



//Funcion para seleccionar un directorio con filesel
//Solo cambia string_root_dir si se sale de filesel con ESC
//Devuelve mismo valor que retorna menu_filesel
int menu_storage_string_root_dir(char *string_root_dir)
{

        char *filtros[2];

        filtros[0]="nofiles";
        filtros[1]=0;


        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        int ret;


	char nada[PATH_MAX];

        //Obtenemos ultimo directorio visitado
	zvfs_chdir(string_root_dir);


        ret=menu_filesel("Enter dir & press ESC",filtros,nada);


	//Si sale con ESC
	if (ret==0) {
		//Directorio root
		sprintf (string_root_dir,"%s",menu_filesel_last_directory_seen);
		debug_printf (VERBOSE_DEBUG,"Selected directory: %s",string_root_dir);

	}

    //volvemos a directorio inicial
    zvfs_chdir(directorio_actual);

	return ret;


}





void menu_storage_esxdos_traps_emulation(MENU_ITEM_PARAMETERS)
{



	if (esxdos_handler_enabled.v) esxdos_handler_disable();
	else {
		//Si no hay paging, avisar
		if (diviface_enabled.v==0) {
			if (menu_confirm_yesno_texto("No divide/mmc paging","Sure enable?")==0) return;
		}
		esxdos_handler_enable();
	}
}

void menu_esxdos_traps_root_dir(MENU_ITEM_PARAMETERS)
{


	int ret;
	ret=menu_storage_string_root_dir(esxdos_handler_root_dir);

	//Si sale con ESC
	if (ret==0) {
        //directorio esxdos vacio
	    esxdos_handler_cwd[0]=0;
	}

}

void menu_esxdos_traps_readonly(MENU_ITEM_PARAMETERS)
{
    esxdos_handler_readonly.v ^=1;
}


void menu_esxdos_traps(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_esxdos_traps;
    menu_item item_seleccionado;
    int retorno_menu;
    do {

        char string_esxdos_traps_root_dir_shown[18];


        menu_add_item_menu_inicial_format(&array_menu_esxdos_traps,MENU_OPCION_NORMAL,menu_storage_esxdos_traps_emulation,NULL,"[%c] ~~Enabled", (esxdos_handler_enabled.v ? 'X' : ' ' ));
        menu_add_item_menu_shortcut(array_menu_esxdos_traps,'e');
        menu_add_item_menu_tooltip(array_menu_esxdos_traps,"Enable ESXDOS handler");
        menu_add_item_menu_ayuda(array_menu_esxdos_traps,"Enable ESXDOS handler");

        if (esxdos_handler_enabled.v) {
            menu_tape_settings_trunc_name(esxdos_handler_root_dir,string_esxdos_traps_root_dir_shown,18);
            menu_add_item_menu_format(array_menu_esxdos_traps,MENU_OPCION_NORMAL,menu_esxdos_traps_root_dir,NULL,"~~Root dir: %s",string_esxdos_traps_root_dir_shown);
            menu_add_item_menu_prefijo(array_menu_esxdos_traps,"    ");
            menu_add_item_menu_shortcut(array_menu_esxdos_traps,'r');

            menu_add_item_menu_tooltip(array_menu_esxdos_traps,"Sets the root directory for ESXDOS filesystem");
            menu_add_item_menu_ayuda(array_menu_esxdos_traps,"Sets the root directory for ESXDOS filesystem. "
                "Only file and folder names valid for ESXDOS will be shown:\n"
                "-Maximum 8 characters for name and 3 for extension\n"
                "-Files and folders will be shown always in uppercase. Folders which are not uppercase, are shown but can not be accessed\n"
                );

            menu_add_item_menu_format(array_menu_esxdos_traps,MENU_OPCION_NORMAL,menu_esxdos_traps_readonly,NULL,"[%c] Read only",
                (esxdos_handler_readonly.v ? 'X' : ' ' ) );

        }






        menu_add_item_menu(array_menu_esxdos_traps,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_ESC_item(array_menu_esxdos_traps);

        retorno_menu=menu_dibuja_menu_no_title_lang(&esxdos_traps_opcion_seleccionada,&item_seleccionado,array_menu_esxdos_traps,"ESXDOS handler" );


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                    //printf ("actuamos por funcion\n");
                    item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


}




void menu_storage_dskplusthree_file(MENU_ITEM_PARAMETERS)
{

	dskplusthree_disable();

        char *filtros[2];

        filtros[0]="dsk";
        filtros[1]=0;

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

              //Obtenemos directorio de dskplusthree
        //si no hay directorio, vamos a rutas predefinidas
        if (dskplusthree_file_name[0]==0) menu_chdir_sharedfiles();

        else {
                char directorio[PATH_MAX];
                util_get_dir(dskplusthree_file_name,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        zvfs_chdir(directorio);
                }
        }

	char dskfile[PATH_MAX];
	dskfile[0]=0;

        int ret=menu_filesel("Select DSK File",filtros,dskfile);
        //volvemos a directorio inicial
        zvfs_chdir(directorio_actual);

        if (ret==1) {

		if (!si_existe_archivo(dskfile)) {

			//menu_warn_message("File does not exist");
			//return;

            if (menu_confirm_yesno_texto("DSK does not exists","Create?")) {

                //Parece que a +3DOS no le gusta nada discos que se salen del formato estandard
                int tipo=menu_simple_two_choices("DSK type","Image type?","+3DOS compatible","Custom");
                if (tipo<1) return;

                if (tipo==1) {
                    dsk_create(dskfile,40,1,9,512);
                }

                else {

                    char buffer_numeros[5];

                    //Pistas
                    strcpy (buffer_numeros,"40");
                    menu_ventana_scanf("Tracks?",buffer_numeros,3);
                    int pistas=parse_string_to_number(buffer_numeros);

                    //Caras
                    strcpy (buffer_numeros,"1");
                    menu_ventana_scanf("Sides?",buffer_numeros,2);
                    int caras=parse_string_to_number(buffer_numeros);

                    if (caras!=1) {
                        debug_printf(VERBOSE_ERR,"You can only create disks of 1 sides!");

                        //TODO permitir discos de 2 caras
                        return;
                    }


                    if (caras!=1 && caras!=2) {
                        debug_printf(VERBOSE_ERR,"You can only create disks of 1 or 2 sides!");

                        return;
                    }

                    //Sectors/track
                    strcpy (buffer_numeros,"9");
                    menu_ventana_scanf("Sectors per track?",buffer_numeros,2);
                    int sectores_pista=parse_string_to_number(buffer_numeros);
                    if (sectores_pista<1 && sectores_pista>9) {
                        debug_printf(VERBOSE_ERR,"Invalid sectors per track number");
                        return;
                    }




                    int opcion=menu_simple_six_choices("Sector size?","One of:","256","512","1024","2048","4096","8192");
                    if (opcion<1) return;


                    //256   //1
                    //512,  //2
                    //1024, //3
                    //2048, //4
                    //4096, //5
                    //8192, //6


                    int sector_size=128<<opcion;



                    dsk_create(dskfile,pistas,caras,sectores_pista,sector_size);

                }

            }



		}
		dsk_insert_disk(dskfile);

		dskplusthree_enable();

        //Habilitar pd765 a no ser que los traps esten activados
        if (plus3dos_traps.v==0) pd765_enable();
		//plus3dos_traps.v=1;


        }
        //Sale con ESC
        else {
                //Quitar nombre
                dskplusthree_file_name[0]=0;
        }
}

void menu_storage_plusthreedisk_traps(MENU_ITEM_PARAMETERS)
{
	plus3dos_traps.v ^=1;
}


void menu_plusthreedisk_pd765(MENU_ITEM_PARAMETERS)
{
    if (pd765_enabled.v) pd765_disable();
	else pd765_enable();
}

int menu_storage_dskplusthree_emulation_cond(void)
{
        if (dskplusthree_file_name[0]==0) return 0;
        else return 1;
}


void menu_storage_dskplusthree_emulation(MENU_ITEM_PARAMETERS)
{
	if (dskplusthree_emulation.v==0) {
		dskplusthree_enable();

        //Habilitar pd765 a no ser que los traps esten activados
        if (plus3dos_traps.v==0) pd765_enable();

		//plus3dos_traps.v=1;
	}

	else dskplusthree_disable();
}


void menu_storage_dskplusthree_browser(MENU_ITEM_PARAMETERS)
{
	menu_file_dsk_browser_show(dskplusthree_file_name);

    //por coherencia, despues de aqui cerramos todas las ventanas
    salir_todos_menus=1;
}

void menu_storage_dsk_write_protect(MENU_ITEM_PARAMETERS)
{
	dskplusthree_write_protection.v ^=1;
}


void menu_storage_dskplusthree_persistent_writes(MENU_ITEM_PARAMETERS)
{
	dskplusthree_persistent_writes.v ^=1;
}

int menu_storage_dskplusthree_info_cond(void)
{
    return dskplusthree_emulation.v;
}

void menu_plusthreedisk_info_sectors_sector(MENU_ITEM_PARAMETERS)
{
    int pista=valor_opcion & 0xFF;
    int cara=(valor_opcion/256) & 0xFF;
    int sector=(valor_opcion/65536) & 65535;

    int iniciosector=dsk_get_sector_fisico(pista,cara,sector);

    //printf("Pista %d Cara %d Sector %d. Inicio=%XH\n",pista,cara,sector,iniciosector);

    dsk_memory_zone_dsk_sector_start=iniciosector;

    //esto da tamaño segun parametro N,
    //pero en protecciones en que el sector esta escrito varias veces con diferentes valores,
    //queremos ver todas las copias
    //Ver formato DSK:
    /*
    2. Storing Multiple Versions of Weak/Random Sectors.
    Some copy protections have what is described as 'weak/random' data. Each time the sector is read one
    or more bytes will change, the value may be random between consecutive reads of the same sector.
    To support these formats the following extension has been proposed.
    Where a sector has weak/random data, there are multiple copies stored. The actual sector size field
    in the SECTOR INFORMATION LIST describes the size of all the copies. To determine if a sector has multiple
    copies then compare the actual sector size field to the size defined by the N parameter.
    For multiple copies the actual sector size field will have a value which is a multiple of the size
    defined by the N parameter. The emulator should then choose which copy of the sector it should return on each read.
    */
    int sector_size=dsk_get_sector_size_track(pista,cara);

    if (dsk_file_type_extended) {
        //Tamanyo que dice el sector realmente
        sector_size=dsk_get_real_sector_size_extended(pista,cara,sector);
    }

    if (iniciosector<0 || sector_size<0) {
        debug_printf(VERBOSE_ERR,"Can not set memory zone to dsk sector");
        dsk_memory_zone_dsk_sector_enabled.v=0;
    }

    dsk_memory_zone_dsk_sector_size=sector_size;
    dsk_memory_zone_dsk_sector_enabled.v=1;

    menu_set_memzone(MEMORY_ZONE_DSK_SECTOR);

    menu_debug_hexdump(0);
}

void menu_plusthreedisk_info_sectors_list(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

    int pista=valor_opcion & 0xFF;
    int cara=valor_opcion/256;

    char menu_titulo[40];
    sprintf(menu_titulo,"Track %d Head %d Sectors",pista,cara);

    do {


        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

        int sector;


        int total_sectores=dsk_get_total_sectors_track(pista,cara);

        for (sector=0;sector<total_sectores;sector++) {
                z80_byte leido_id_st1 ,leido_id_st2;

                dsk_get_st12(pista,cara,sector,&leido_id_st1,&leido_id_st2);


                menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_plusthreedisk_info_sectors_sector,NULL,"Sector %d ST1: %02X ST2: %02X %s",
                    sector,leido_id_st1,leido_id_st2,(leido_id_st2 & PD765_STATUS_REGISTER_TWO_CM_MASK ? "DELETED" : ""));

                //Codificamos la opcion para el submenu asi
                int valor_opcion_menu=pista+cara*256+sector*65536;
                menu_add_item_menu_valor_opcion(array_menu_common,valor_opcion_menu);
                //menu_add_item_menu_tiene_submenu(array_menu_common);
                menu_add_item_menu_genera_ventana(array_menu_common);

                //Leer chrn para debug
                z80_byte leido_id_c,leido_id_h,leido_id_r,leido_id_n;

                dsk_get_chrn(pista,cara,sector,&leido_id_c,&leido_id_h,&leido_id_r,&leido_id_n);

                menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL," C:%02X H:%02X R:%02X N:%02X",
                    leido_id_c,leido_id_h,leido_id_r,leido_id_n);

                if (dsk_file_type_extended) {
                    int tamanyo_real=dsk_get_real_sector_size_extended(pista,cara,sector);
                    menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL," Actual size: %d",tamanyo_real);
                }

                menu_add_item_menu_separator(array_menu_common);


        }

        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&menu_plusthreedisk_info_sectors_list_opcion_seleccionada,&item_seleccionado,array_menu_common,menu_titulo);


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

void menu_plusthreedisk_info_tracks_list(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;



    do {


        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);
        //menu_add_item_menu_no_es_realmente_un_menu(array_menu_common);

        int pista;
        int cara;

        int total_pistas=dsk_get_total_tracks();
        int total_caras=dsk_get_total_sides();

        for (pista=0;pista<total_pistas;pista++) {
            for (cara=0;cara<total_caras;cara++) {
                /*
                int sinformatear=0;

                if (dsk_file_type_extended) {
                    int track_size=dsk_extended_get_track_size(pista,cara);
                    if (!track_size) sinformatear=1;
                }

                if (sinformatear) {
                    menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Track %02d Side %d. UNFORMATTED",pista,cara);
                }
                */

                if (!dsk_is_track_formatted(pista,cara)) {
                    menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Track %02d Side %d. UNFORMATTED",pista,cara);
                }

                else {
                    menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_plusthreedisk_info_sectors_list,NULL,"Track %02d Side %d",pista,cara);
                    menu_add_item_menu_tiene_submenu(array_menu_common);

                    //Codificamos la opcion para el submenu asi
                    int valor_opcion_menu=pista+cara*256;
                    menu_add_item_menu_valor_opcion(array_menu_common,valor_opcion_menu);

                    int sector_size_track=dsk_get_sector_size_track(pista,cara);
                    int total_sectors_track=dsk_get_total_sectors_track(pista,cara);
                    int gap_length_track=dsk_get_gap_length_track(pista,cara);
                    int filler_byte_track=dsk_get_filler_byte_track(pista,cara);

                    int datarate_track=dsk_get_datarate_track(pista,cara);
                    char *datarates[]={
                        "Single or Double Density",
                        "High Density",
                        "Extended Density"
                    };

                    datarate_track--;
                    char datarate_buffer[100];
                    if (datarate_track<0 || datarate_track>2) {
                        strcpy(datarate_buffer,"Unk");
                    }
                    else {
                        strcpy(datarate_buffer,datarates[datarate_track]);
                    }

                    int recordingmode=dsk_get_recordingmode_track(pista,cara);
                    char *recordings[]={
                        "FM",
                        "MFM"
                    };

                    recordingmode--;
                    char recordingmode_buffer[100];
                    if (recordingmode<0 || recordingmode>1) {
                        strcpy(recordingmode_buffer,"Unk");
                    }
                    else {
                        strcpy(recordingmode_buffer,recordings[recordingmode]);
                    }

                    menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,menu_plusthreedisk_info_sectors_list,NULL," Sector size: %4d Sectors: %d",
                        sector_size_track,total_sectors_track);

                    menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,menu_plusthreedisk_info_sectors_list,NULL," Gap length: %3d Filler: %2XH",
                        gap_length_track,filler_byte_track);

                    menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,menu_plusthreedisk_info_sectors_list,NULL," Datarate: %s. Record mode: %s",
                        datarate_buffer,recordingmode_buffer);

                }

                menu_add_item_menu_separator(array_menu_common);

            }
        }

        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&menu_plusthreedisk_info_tracks_list_opcion_seleccionada,&item_seleccionado,array_menu_common,"Tracks list");


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    //por coherencia, despues de aqui cerramos todas las ventanas
    salir_todos_menus=1;
}


void menu_plusthreedisk_info(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

    char buffer_signature[DSK_SIGNATURE_LENGTH+1];
    char buffer_creator[DSK_CREATOR_LENGTH+1];
    char buffer_esquema_proteccion[DSK_MAX_PROTECTION_SCHEME+1];


    do {

        /*menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,menu_plusthreedisk_info_tracks_list,NULL,"Tracks list");
        menu_add_item_menu_tiene_submenu(array_menu_common);

        menu_add_item_menu_separator(array_menu_common);*/

        dsk_get_signature(buffer_signature);
        dsk_get_creator(buffer_creator);
        dsk_get_protection_scheme(buffer_esquema_proteccion);


        menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Signature:");
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL," %s",buffer_signature);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"Creator:");
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL," %s",buffer_creator);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"Total tracks: %d",dsk_get_total_tracks());
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"Total sides: %d",dsk_get_total_sides());
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"Protection System: %s",buffer_esquema_proteccion);


        menu_add_item_menu_separator(array_menu_common);



        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&menu_plusthreedisk_info_opcion_seleccionada,&item_seleccionado,array_menu_common,"Disk Info");


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    //por coherencia, despues de aqui cerramos todas las ventanas
    salir_todos_menus=1;
}


void menu_storage_dsk_pd765_silent_write_protection(MENU_ITEM_PARAMETERS)
{
    pd765_silent_write_protection.v ^=1;
}

void menu_pcw_boot_cpm(MENU_ITEM_PARAMETERS)
{
    pcw_boot_cpm();
    salir_todos_menus=1;
}

void menu_pcw_boot_locoscript(MENU_ITEM_PARAMETERS)
{
    pcw_boot_locoscript();
    salir_todos_menus=1;
}

void menu_pcw_boot_cpm_reinsert_previous(MENU_ITEM_PARAMETERS)
{
    pcw_boot_reinsert_previous_dsk.v ^=1;
}

void menu_pcw_failback_cpm_when_no_boot(MENU_ITEM_PARAMETERS)
{
    pcw_failback_cpm_when_no_boot.v ^=1;
}

void menu_plusthreedisk(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_plusthreedisk;
    menu_item item_seleccionado;
    int retorno_menu;
    do {


        char string_dskplusthree_file_shown[20];


        menu_tape_settings_trunc_name(dskplusthree_file_name,string_dskplusthree_file_shown,20);
        //menu_add_item_menu_inicial_format(&array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dskplusthree_file,NULL,
        //    "~~DSK File: [%s]",string_dskplusthree_file_shown);
        menu_add_item_menu_en_es_ca_inicial(&array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dskplusthree_file,NULL,
            "~~DSK File","Archivo ~~DSK","Arxiu ~~DSK");
        menu_add_item_menu_sufijo_format(array_menu_plusthreedisk," [%s]",string_dskplusthree_file_shown);
        menu_add_item_menu_prefijo(array_menu_plusthreedisk,"    ");
        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'d');
        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"DSK Emulation file");
        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"DSK Emulation file");


        menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dskplusthree_emulation,
            menu_storage_dskplusthree_emulation_cond,"DSK ~~Emulation","~~Emulación DSK","~~Emulació DSK");
        menu_add_item_menu_prefijo_format(array_menu_plusthreedisk,"[%c] ", (dskplusthree_emulation.v ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'e');
        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"DSK Emulation");
        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"DSK Emulation");



        menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dsk_write_protect,NULL,
            "Write protect","Protección escritura","Protecció escriptura");
        menu_add_item_menu_prefijo_format(array_menu_plusthreedisk,"[%c] ", (dskplusthree_write_protection.v ? 'X' : ' '));
        //menu_add_item_menu_shortcut(array_menu_plusthreedisk,'w');
        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"If DSK disk is write protected");
        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"If DSK disk is write protected");

        if (dskplusthree_write_protection.v) {
            menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dsk_pd765_silent_write_protection,NULL,
                "~~Silent protection","Protección ~~silenciosa","Protecció ~~silenciosa");
            menu_add_item_menu_prefijo_format(array_menu_plusthreedisk,"[%c] ", (pd765_silent_write_protection.v ? 'X' : ' '));
            menu_add_item_menu_shortcut(array_menu_plusthreedisk,'s');
            menu_add_item_menu_tooltip(array_menu_plusthreedisk,"When write protect is enabled, do not notify the cpu, so behave as it is not write protected (but the data is not written)");
            menu_add_item_menu_ayuda(array_menu_plusthreedisk,"When write protect is enabled, do not notify the cpu, so behave as it is not write protected (but the data is not written)");
        }

        menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dskplusthree_persistent_writes,NULL,
            "Persistent Writes","Escrituras Persistentes","Escriptures Persistents");
        menu_add_item_menu_prefijo_format(array_menu_plusthreedisk,"[%c] ",(dskplusthree_persistent_writes.v ? 'X' : ' ') );
        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"Tells if DSK writes are saved to disk");
        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"Tells if DSK writes are saved to disk. "
            "Note: all writing operations to DSK are always saved to internal memory (unless you disable write permission), but this setting "
            "tells if these changes are written to disk or not."
            );



        menu_add_item_menu(array_menu_plusthreedisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        if (MACHINE_IS_PCW) {
            menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_pcw_boot_cpm,
                NULL,"    Boot CP/M now","    Boot CP/M ahora","    Boot CP/M ara");
			menu_add_item_menu_tooltip(array_menu_plusthreedisk,"Boot CP/M");
			menu_add_item_menu_ayuda(array_menu_plusthreedisk,"Boot CP/M");

            menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_pcw_boot_cpm_reinsert_previous,NULL,
            "Reinsert previous dsk after boot","Reinsertar previo dsk después boot","Reinsertar previ dsk després boot");
            menu_add_item_menu_prefijo_format(array_menu_plusthreedisk,"[%c] ",(pcw_boot_reinsert_previous_dsk.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_plusthreedisk,"Reinsert previous dsk after booting CP/M");
			menu_add_item_menu_ayuda(array_menu_plusthreedisk,"Reinsert previous dsk after booting CP/M");

            menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_pcw_failback_cpm_when_no_boot,NULL,
            "Failback to CP/M if no boot","Failback a CP/M si no boot","Failback a CP/M si no boot");
            menu_add_item_menu_prefijo_format(array_menu_plusthreedisk,"[%c] ",(pcw_failback_cpm_when_no_boot.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_plusthreedisk,"Insert CP/M disk if selected disk is not bootable");
			menu_add_item_menu_ayuda(array_menu_plusthreedisk,"Insert CP/M disk if selected disk is not bootable");

            menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_pcw_boot_locoscript,NULL,
                "    Boot LocoScript now","    Boot LocoScript ahora","    Boot LocoScript ara");
			menu_add_item_menu_tooltip(array_menu_plusthreedisk,"Boot LocoScript");
			menu_add_item_menu_ayuda(array_menu_plusthreedisk,"Boot LocoScript");

            menu_add_item_menu(array_menu_plusthreedisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        }




        menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_plusthreedisk_pd765,NULL,
            "~~PD765 enabled","~~PD765 activado","~~PD765 activat");
        menu_add_item_menu_prefijo_format(array_menu_plusthreedisk,"[%c] ",(pd765_enabled.v ? 'X' : ' ') );
        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'p');
        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"Enable PD765 Disk controller used on +3, CPC and PCW machines");
        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"Enable PD765 Disk controller used on +3, CPC and PCW machines");


        menu_add_item_menu(array_menu_plusthreedisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);


        //Esto que no aparezca en cpc ni en pcw
        if (MACHINE_IS_SPECTRUM) {
            menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_plusthreedisk_traps,NULL,
                "+3DOS ~~Traps","+3DOS ~~Traps","+3DOS ~~Traps");
            menu_add_item_menu_prefijo_format(array_menu_plusthreedisk,"[%c] ", (plus3dos_traps.v ? 'X' : ' '));
            menu_add_item_menu_shortcut(array_menu_plusthreedisk,'t');
            menu_add_item_menu_tooltip(array_menu_plusthreedisk,"Enable +3DOS Traps. This is EXPERIMENTAL");
            menu_add_item_menu_ayuda(array_menu_plusthreedisk,"Enable +3DOS Traps. This is EXPERIMENTAL");
            menu_add_item_menu_separator(array_menu_plusthreedisk);
        }



        menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_plusthreedisk_info,menu_storage_dskplusthree_info_cond,
            "Disk ~~Info","Disk ~~Info","Disk ~~Info");
        menu_add_item_menu_prefijo(array_menu_plusthreedisk,"    ");
        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'i');
        menu_add_item_menu_genera_ventana(array_menu_plusthreedisk);

        menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_plusthreedisk_info_tracks_list,menu_storage_dskplusthree_info_cond,
            "Tracks ~~list","~~Lista Pistas","~~Llista Pistes");
        menu_add_item_menu_prefijo(array_menu_plusthreedisk,"    ");
        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'l');
        menu_add_item_menu_genera_ventana(array_menu_plusthreedisk);


        menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dskplusthree_browser,
            menu_storage_dskplusthree_emulation_cond,"Disk ~~Format Viewer","Visor ~~Formato Disco","Visor ~~Format Disc");
        menu_add_item_menu_prefijo(array_menu_plusthreedisk,"    ");
        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'f');
        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"Disk Format Viewer");
        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"Disk Format Viewer");
        menu_add_item_menu_genera_ventana(array_menu_plusthreedisk);

        menu_add_item_menu_separator(array_menu_plusthreedisk);

        menu_add_item_menu_en_es_ca(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_visual_floppy,NULL,
            "~~Visual Floppy","~~Visual Floppy","~~Visual Floppy");
        menu_add_item_menu_prefijo(array_menu_plusthreedisk,"    ");
        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'v');
        menu_add_item_menu_se_cerrara(array_menu_plusthreedisk);
        menu_add_item_menu_genera_ventana(array_menu_plusthreedisk);

        //menu_add_item_menu_tiene_submenu(array_menu_plusthreedisk);


        menu_add_item_menu(array_menu_plusthreedisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_ESC_item(array_menu_plusthreedisk);

        retorno_menu=menu_dibuja_menu_no_title_lang(&plusthreedisk_opcion_seleccionada,&item_seleccionado,array_menu_plusthreedisk,"3\" CF2 Floppy" );


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



}
