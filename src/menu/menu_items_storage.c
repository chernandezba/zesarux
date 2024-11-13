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

