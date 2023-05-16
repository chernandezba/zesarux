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
   Menu settings entries
*/

//
// Archivo para entradas de menu de settings, excluyendo funciones auxiliares de soporte de menu
// Las funciones auxiliares de menu estan en menu.c
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
#include "menu_items.h"
#include "menu_items_settings.h"
#include "menu_filesel.h"
#include "menu_file_viewer_browser.h"
#include "menu_debug_cpu.h"
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
#include "snap_zx8081.h"
#include "menu_bitmaps.h"
#include "pcw.h"


#ifdef COMPILE_ALSA
#include "audioalsa.h"
#endif


#ifdef COMPILE_ONEBITSPEAKER
#include "audioonebitspeaker.h"
#endif

 
#if defined(__APPLE__)
	#include <sys/syslimits.h>

	#include <sys/resource.h>

#endif


#ifdef COMPILE_CURSES
	#include "scrcurses.h"
#endif

#ifdef COMPILE_AA
	#include "scraa.h"
#endif

#ifdef COMPILE_STDOUT
	#include "scrstdout.h"
#endif


#ifdef COMPILE_XWINDOWS
	#include "scrxwindows.h"
#endif

#ifdef COMPILE_CURSESW
	#include "cursesw_ext.h"
#endif



//Opciones seleccionadas para cada menu
int settings_opcion_seleccionada=0;
int settings_config_file_opcion_seleccionada=0;
int change_video_driver_opcion_seleccionada=0;
int window_settings_opcion_seleccionada=0;
int zxvision_settings_opcion_seleccionada=0;
int settings_snapshot_opcion_seleccionada=0;
int hardware_realjoystick_opcion_seleccionada=0;
int hardware_realjoystick_event_opcion_seleccionada=0;
int hardware_realjoystick_keys_opcion_seleccionada=0;
int special_fx_settings_opcion_seleccionada=0;
int osd_settings_opcion_seleccionada=0;
int external_tools_config_opcion_seleccionada=0;
int hardware_set_f_functions_opcion_seleccionada=0;
int hardware_set_f_func_action_opcion_seleccionada=0;
int settings_debug_opcion_seleccionada=0;
int settings_audio_opcion_seleccionada=0;
int change_audio_driver_opcion_seleccionada=0;
int hardware_advanced_opcion_seleccionada=0;
int ula_settings_opcion_seleccionada=0;
int hardware_memory_settings_opcion_seleccionada=0;
int hardware_printers_opcion_seleccionada=0;
int hardware_settings_opcion_seleccionada=0;
int keyboard_settings_opcion_seleccionada=0;
int hardware_redefine_keys_opcion_seleccionada=0;
int settings_tape_opcion_seleccionada=0;
int settings_storage_opcion_seleccionada=0;
int textdrivers_settings_opcion_seleccionada=0;
int colour_settings_opcion_seleccionada=0;
int settings_display_opcion_seleccionada=0;
int accessibility_settings_opcion_seleccionada=0;
int chardetection_settings_opcion_seleccionada=0;
int textspeech_opcion_seleccionada=0;
int accessibility_menu_opcion_seleccionada=0;
int userdef_button_func_action_opcion_seleccionada=0;
int zxdesktop_set_userdef_buttons_functions_opcion_seleccionada=0;
int ext_desktop_settings_opcion_seleccionada=0;
int cpu_settings_opcion_seleccionada=0;
int zxdesktop_set_configurable_icons_opcion_seleccionada=0;
int fileselector_settings_opcion_seleccionada=0;
int debug_verbose_filter_opcion_seleccionada=0;
int settings_statistics_opcion_seleccionada=0;

//Fin opciones seleccionadas para cada menu


//void menu_hardware_realjoystick(MENU_ITEM_PARAMETERS);
//void menu_settings_tape(MENU_ITEM_PARAMETERS);

//aofile. aofilename apuntara aqui
char aofilename_file[PATH_MAX];

//archivo zxprinter bitmap
char zxprinter_bitmap_filename_buffer[PATH_MAX];
//archivo zxprinter texto ocr
char zxprinter_ocr_filename_buffer[PATH_MAX];

//vofile. vofilename apuntara aqui
char vofilename_file[PATH_MAX];



int menu_cond_zx81(void)
{
        if (MACHINE_IS_ZX81_TYPE) return 1;
        return 0;
}

int menu_cond_zx81_realvideo(void)
{
        if (menu_cond_zx81()==0) return 0;
        return rainbow_enabled.v;

}

int menu_cond_realvideo(void)
{
	return rainbow_enabled.v;

}




int menu_cond_zx8081(void)
{
	if (MACHINE_IS_ZX8081) return 1;
	return 0;
}

int menu_cond_zx8081_realvideo(void)
{
	if (menu_cond_zx8081()==0) return 0;
	return rainbow_enabled.v;
}

int menu_cond_zx8081_wrx(void)
{
        if (menu_cond_zx8081()==0) return 0;
        return wrx_present.v;
}

int menu_cond_zx8081_wrx_no_stabilization(void)
{
	if (menu_cond_zx8081_wrx()==0) return 0;
	return !video_zx8081_estabilizador_imagen.v;
}

int menu_cond_zx8081_no_realvideo(void)
{
        if (menu_cond_zx8081()==0) return 0;
        return !rainbow_enabled.v;
}

int menu_cond_curses(void)
{
	if (!strcmp(scr_new_driver_name,"curses")) return 1;
	return 0;
}

int menu_cond_stdout(void)
{
        if (!strcmp(scr_new_driver_name,"stdout")) return 1;

        return 0;
}

int menu_cond_simpletext(void)
{
        if (!strcmp(scr_new_driver_name,"simpletext")) return 1;

        return 0;
}



/*
int menu_cond_no_stdout(void)
{
        //esto solo se permite en drivers xwindows, caca, aa, curses. NO en stdout
        if (!strcmp(scr_new_driver_name,"stdout")) return 0;
        return 1;
}
*/

int menu_cond_no_curses_no_stdout(void)
{
        //esto solo se permite en drivers xwindows, caca, aa. NO en curses ni stdout
        if (!strcmp(scr_new_driver_name,"curses")) return 0;
        if (!strcmp(scr_new_driver_name,"stdout")) return 0;
	return 1;
}




int menu_cond_zx8081_no_curses_no_stdout(void)
{
	if (!menu_cond_zx8081()) return 0;
	return menu_cond_no_curses_no_stdout();

}

int menu_cond_wrx(void)
{
	return wrx_present.v;
}



int menu_display_rainbow_cond(void)
{
	//if (MACHINE_IS_Z88) return 0;
	return 1;
}

void menu_settings_config_file_save_config(MENU_ITEM_PARAMETERS)
{
	if (util_write_configfile()) {
		menu_generic_message_splash("Save configuration","OK. Configuration saved");
	};
}

void menu_settings_config_file_save_on_exit(MENU_ITEM_PARAMETERS)
{
	if (save_configuration_file_on_exit.v) {
		if (menu_confirm_yesno_texto("Write configuration","To disable setting saveconf")==0) return;
		save_configuration_file_on_exit.v=0;
		util_write_configfile();
		menu_generic_message_splash("Save configuration","OK. Configuration saved");
	}
	else save_configuration_file_on_exit.v=1;
}

void menu_settings_config_file_show(MENU_ITEM_PARAMETERS)
{
                          char configfile[PATH_MAX];

                                if (util_get_configfile_name(configfile)==0)  {
					menu_warn_message("Unknown configuration file");
                                }
	else {
		menu_file_viewer_read_text_file("Config file",configfile);
	}
}

void menu_settings_config_file_show_location(MENU_ITEM_PARAMETERS)
{

    char configfile[PATH_MAX];

    if (util_get_configfile_name(configfile)==0)  {
        sprintf(configfile,"Unknown");
    }

	menu_generic_message_format("Config Path",configfile);    
}


void menu_settings_config_file_reset(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno_texto("Reset defaults","Need to exit. Sure?")==0) return;

	util_create_sample_configfile(1);
	menu_warn_message("Configuration settings reset to defaults. Press enter to close ZEsarUX. You should start ZEsarUX again to read default configuration");

	//Y nos aseguramos que al salir no se guarde configuración con lo que tenemos en memoria
	save_configuration_file_on_exit.v=0;
	end_emulator_autosave_snapshot();

}


//menu config_file settings
void menu_settings_config_file(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_config_file;
        menu_item item_seleccionado;
	int retorno_menu;
        do {



		menu_add_item_menu_inicial_format(&array_menu_settings_config_file,MENU_OPCION_NORMAL,menu_settings_config_file_save_on_exit,NULL,"~~Autosave on exit");
        menu_add_item_menu_spanish_catalan(array_menu_settings_config_file,"~~Autosalvar al salir","~~Autosalvar al sortir");
        menu_add_item_menu_prefijo_format(array_menu_settings_config_file,"[%c] ",(save_configuration_file_on_exit.v ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_config_file,'a');
		menu_add_item_menu_tooltip(array_menu_settings_config_file,"Auto save configuration on exit emulator");
		menu_add_item_menu_ayuda(array_menu_settings_config_file,"Auto save configuration on exit emulator and overwrite it. Note: not all settings are saved");

		menu_add_item_menu_en_es_ca(array_menu_settings_config_file,MENU_OPCION_NORMAL,menu_settings_config_file_save_config,NULL,
            "    ~~Save configuration","    ~~Salvar configuración","    ~~Salvar configuració");
		menu_add_item_menu_shortcut(array_menu_settings_config_file,'s');
		menu_add_item_menu_tooltip(array_menu_settings_config_file,"Overwrite your configuration file with current settings");
		menu_add_item_menu_ayuda(array_menu_settings_config_file,"Overwrite your configuration file with current settings");
        menu_add_item_menu_es_avanzado(array_menu_settings_config_file);


		menu_add_item_menu_en_es_ca(array_menu_settings_config_file,MENU_OPCION_NORMAL,menu_settings_config_file_show,NULL,
            "    ~~View config file","    ~~Ver archivo configuración","    ~~Veure arxiu configuració");
		menu_add_item_menu_shortcut(array_menu_settings_config_file,'v');
		menu_add_item_menu_tooltip(array_menu_settings_config_file,"View configuration file");
		menu_add_item_menu_ayuda(array_menu_settings_config_file,"View configuration file");
        menu_add_item_menu_es_avanzado(array_menu_settings_config_file);

		menu_add_item_menu_en_es_ca(array_menu_settings_config_file,MENU_OPCION_NORMAL,menu_settings_config_file_show_location,NULL,
            "    Show config file ~~path","    Ver ~~path archivo configuración","    Veure ~~path arxiu configuració");
		menu_add_item_menu_shortcut(array_menu_settings_config_file,'p');
		menu_add_item_menu_tooltip(array_menu_settings_config_file,"Show config file location");
		menu_add_item_menu_ayuda(array_menu_settings_config_file,"Show config file location");
        menu_add_item_menu_es_avanzado(array_menu_settings_config_file);        


		menu_add_item_menu_en_es_ca(array_menu_settings_config_file,MENU_OPCION_NORMAL,menu_settings_config_file_reset,NULL,
            "    ~~Reset config file","    ~~Resetear archivo config","    ~~Resetejar arxiu config");
		menu_add_item_menu_shortcut(array_menu_settings_config_file,'r');
		menu_add_item_menu_tooltip(array_menu_settings_config_file,"Reset configuration file to default values");
		menu_add_item_menu_ayuda(array_menu_settings_config_file,"Reset configuration file to default values");	
        menu_add_item_menu_es_avanzado(array_menu_settings_config_file);	



                menu_add_item_menu(array_menu_settings_config_file,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                
		menu_add_ESC_item(array_menu_settings_config_file);

                retorno_menu=menu_dibuja_menu(&settings_config_file_opcion_seleccionada,&item_seleccionado,array_menu_settings_config_file,"Configuration file" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
                        }
                }

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

int menu_interface_border_cond(void)
{
	if (ventana_fullscreen) return 0;
	return 1;
}

int menu_interface_zoom_cond(void)
{
        if (ventana_fullscreen) return 0;
        return 1;
}

void menu_interface_frameskip(MENU_ITEM_PARAMETERS)
{

	menu_ventana_scanf_numero_enhanced("Frameskip",&frameskip,3,+1,0,49,0);


}

void menu_interface_autoframeskip(MENU_ITEM_PARAMETERS)
{
	autoframeskip.v ^=1;
}

void menu_interface_ignore_click_open_menu(MENU_ITEM_PARAMETERS)
{
    mouse_menu_ignore_click_open.v ^=1;    
}

void menu_interface_border(MENU_ITEM_PARAMETERS)
{

	//Esperar a que no estemos redibujando pantalla
	//while (sem_screen_refresh_reallocate_layers) {
	//	printf ("-----Waiting until redraw and realloc functions finish\n");
	//}

        debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);



	if (border_enabled.v) disable_border();
	else enable_border();

        //scr_init_pantalla();

	//printf ("--antes de init pantalla\n");

	screen_init_pantalla_and_others_and_realjoystick();

	//printf ("--despues de init pantalla\n");

    debug_printf(VERBOSE_INFO,"Creating Screen");

	//printf ("--antes de init footer\n");
	menu_init_footer();
	//printf ("--despues de init footer\n");

	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);	

	//printf ("--despues de restore overlay\n");

	debug_printf (VERBOSE_DEBUG,"Rearrange zxvision windows after changing border settings");
	zxvision_rearrange_background_windows();

    zxvision_check_all_configurable_icons_positions();
	
}



int zxdesktop_estado_antes_fullscreen=0;

void menu_interface_fullscreen(MENU_ITEM_PARAMETERS)
{

	if (ventana_fullscreen==0) {
        //Desactivar ZX Desktop al pasar a full screen
        zxdesktop_estado_antes_fullscreen=screen_ext_desktop_enabled;
        if (zxdesktop_disable_on_full_screen) {
            if (screen_ext_desktop_enabled) {
                menu_ext_desk_settings_enable(0);
            }
        }

		scr_set_fullscreen();
	}

	else {
		scr_reset_fullscreen();

        //Retornar ZX Desktop estado al volver de full screen
        if (zxdesktop_disable_on_full_screen && zxdesktop_estado_antes_fullscreen) {
            if (!screen_ext_desktop_enabled) menu_ext_desk_settings_enable(0);
        }

	}

	clear_putpixel_cache();
	menu_init_footer();

}

void menu_setting_limit_menu_open(MENU_ITEM_PARAMETERS)
{
	menu_limit_menu_open.v ^=1;
}


void menu_interface_language(MENU_ITEM_PARAMETERS)
{
    gui_language++;

    if (gui_language>GUI_LANGUAGE_CATALAN) gui_language=0;

    menu_first_aid("language");
}

void menu_interface_zoom(MENU_ITEM_PARAMETERS)
{
        char string_zoom[2];
	int temp_zoom;


	//comprobaciones previas para no petar el sprintf
	if (zoom_x>9 || zoom_x<1) zoom_x=1;

        sprintf (string_zoom,"%d",zoom_x);


        //menu_ventana_scanf_numero("Window Zoom",string_zoom,2);
		//menu_ventana_scanf("Window Zoom",string_zoom,2);

		int retorno=menu_ventana_scanf_numero("Window Zoom",string_zoom,2,+1,1,9,0);
		if (retorno>=0) {
	        temp_zoom=parse_string_to_number(string_zoom);


			screen_set_window_zoom(temp_zoom);
		}

		//else {
		//	printf("pulsado ESC\n");
		//}

}

void menu_window_settings_reduce_075(MENU_ITEM_PARAMETERS)
{
	screen_reduce_075.v ^=1;
	enable_rainbow();
}

void menu_window_settings_reduce_075_antialias(MENU_ITEM_PARAMETERS)
{
	screen_reduce_075_antialias.v ^=1;
}

void menu_window_settings_reduce_075_ofx(MENU_ITEM_PARAMETERS)
{
        char string_offset[3];
        sprintf (string_offset,"%d",screen_reduce_offset_x);
        menu_ventana_scanf("Offset x",string_offset,3);
        screen_reduce_offset_x=parse_string_to_number(string_offset);
}

void menu_window_settings_reduce_075_ofy(MENU_ITEM_PARAMETERS)
{
        char string_offset[3];
        sprintf (string_offset,"%d",screen_reduce_offset_y);
        menu_ventana_scanf("Offset y",string_offset,3);
        screen_reduce_offset_y=parse_string_to_number(string_offset);
}

void menu_interface_footer(MENU_ITEM_PARAMETERS)
{



        debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;
	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);


        if (menu_footer==0) {
		enable_footer();
	}

        else {
                disable_footer();
                
        }


        modificado_border.v=1;
        debug_printf(VERBOSE_INFO,"Creating Screen");
        //scr_init_pantalla();
	screen_init_pantalla_and_others_and_realjoystick();


	if (menu_footer) menu_init_footer();

	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);

}

void menu_interface_show_cpu_usage(MENU_ITEM_PARAMETERS)
{
	screen_show_cpu_usage.v ^=1;
	clear_putpixel_cache();
	if (!screen_show_cpu_usage.v) menu_init_footer();
}

void menu_interface_show_fps(MENU_ITEM_PARAMETERS)
{
	screen_show_fps.v ^=1;
	clear_putpixel_cache();
	if (!screen_show_fps.v) menu_init_footer();
}

void menu_interface_show_cpu_temp(MENU_ITEM_PARAMETERS)
{
	screen_show_cpu_temp.v ^=1;
	clear_putpixel_cache();
	if (!screen_show_cpu_temp.v) menu_init_footer();
}


int num_menu_scr_driver;
int num_previo_menu_scr_driver;


//Determina cual es el video driver actual
void menu_change_video_driver_get(void)
{
	int i;
        for (i=0;i<num_scr_driver_array;i++) {
		if (!strcmp(scr_new_driver_name,scr_driver_array[i].driver_name)) {
			num_menu_scr_driver=i;
			num_previo_menu_scr_driver=i;
			return;
		}

        }

}

void menu_change_video_driver_change(MENU_ITEM_PARAMETERS)
{
	num_menu_scr_driver++;
	if (num_menu_scr_driver==num_scr_driver_array) num_menu_scr_driver=0;
}

void menu_change_video_driver_apply(MENU_ITEM_PARAMETERS)
{

	//Si driver null, avisar
	if (!strcmp(scr_driver_array[num_menu_scr_driver].driver_name,"null")) {
		if (menu_confirm_yesno_texto("Driver is null","Sure?")==0) return;
	}

	//Si driver es cocoa, no dejar cambiar a cocoa
	if (!strcmp(scr_driver_array[num_menu_scr_driver].driver_name,"cocoa")) {
		debug_printf(VERBOSE_ERR,"You can not set cocoa driver from menu. "
				"You must start emulator with cocoa driver (with --vo cocoa or without any --vo setting)");
		return;
	}


	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);



	screen_reset_scr_driver_params();

        int (*funcion_init) ();
        int (*funcion_set) ();

        funcion_init=scr_driver_array[num_menu_scr_driver].funcion_init;
        funcion_set=scr_driver_array[num_menu_scr_driver].funcion_set;

	int resultado=funcion_init();
	set_menu_gui_zoom();
	clear_putpixel_cache();

screen_restart_pantalla_restore_overlay(previous_function,menu_antes);


                if ( resultado == 0 ) {
                        funcion_set();
			menu_generic_message_splash("Apply Driver","OK. Driver applied");
	                //Y salimos de todos los menus
	                salir_todos_menus=1;

                }

		else {
			debug_printf(VERBOSE_ERR,"Can not set video driver. Restoring to previous driver %s",scr_new_driver_name);
			menu_change_video_driver_get();


			screen_end_pantalla_save_overlay(&previous_function,&menu_antes);


			//Restaurar video driver
			screen_reset_scr_driver_params();
		        funcion_init=scr_driver_array[num_previo_menu_scr_driver].funcion_init;
			set_menu_gui_zoom();

		        funcion_set=scr_driver_array[num_previo_menu_scr_driver].funcion_set;

			funcion_init();
			clear_putpixel_cache();
			funcion_set();


			screen_restart_pantalla_restore_overlay(previous_function,menu_antes);
		}

        //scr_init_pantalla();

	modificado_border.v=1;

	menu_init_footer();


	if (!strcmp(scr_new_driver_name,"aa")) {
		menu_generic_message_format("Warning","Remember that on aa video driver, menu is opened with %s",openmenu_key_message);
	}

	//TODO
	//Para aalib, tanto aqui como en cambio de border, no se ve el cursor del menu .... en cuanto se redimensiona la ventana, se arregla


}

void menu_change_video_driver(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_change_video_driver;
        menu_item item_seleccionado;
        int retorno_menu;

	menu_change_video_driver_get();

        do {

                menu_add_item_menu_inicial_format(&array_menu_change_video_driver,MENU_OPCION_NORMAL,menu_change_video_driver_change,NULL,"Video Driver: %s",scr_driver_array[num_menu_scr_driver].driver_name );

                menu_add_item_menu_format(array_menu_change_video_driver,MENU_OPCION_NORMAL,menu_change_video_driver_apply,NULL,"Apply Driver" );

                menu_add_item_menu(array_menu_change_video_driver,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_change_video_driver,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_change_video_driver);

                retorno_menu=menu_dibuja_menu(&change_video_driver_opcion_seleccionada,&item_seleccionado,array_menu_change_video_driver,"Change Video Driver" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

int menu_change_video_driver_cond(void)
{
	if (ventana_fullscreen) return 0;
	else return 1;
}


void menu_interface_flash(MENU_ITEM_PARAMETERS)
{
    disable_change_flash.v ^=1;    
}

void menu_interface_no_autoframeskip_move_windows(MENU_ITEM_PARAMETERS)
{
    auto_frameskip_even_when_movin_windows.v ^=1;
}

void menu_change_online_download_path(MENU_ITEM_PARAMETERS)
{
    menu_storage_string_root_dir(online_download_path);
}

void menu_interface_frameskip_draw_zxdesktop_background(MENU_ITEM_PARAMETERS)
{
    frameskip_draw_zxdesktop_background.v ^=1;
}

void menu_interface_zoom_autochange_big_display(MENU_ITEM_PARAMETERS)
{
    autochange_zoom_big_display.v ^=1;
}

void menu_interface_rpi_performance_params(MENU_ITEM_PARAMETERS)
{
    cambio_parametros_maquinas_lentas.v ^=1;
}

void menu_general_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_window_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


			//hotkeys usadas: fbmzropcilna

        menu_add_item_menu_inicial_format(&array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_fullscreen,NULL,"[%c] ~~Full Screen",(ventana_fullscreen ? 'X' : ' ' ) );
		menu_add_item_menu_shortcut(array_menu_window_settings,'f');

		if (!MACHINE_IS_Z88 && !MACHINE_IS_TSCONF && !MACHINE_IS_TBBLUE && !MACHINE_IS_CPC && !MACHINE_IS_PCW) {
	        menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_border,menu_interface_border_cond,
                "~~Border enabled","~~Borde activado","~~Border activat");
            menu_add_item_menu_prefijo_format(array_menu_window_settings,"[%c] ", (border_enabled.v==1 ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_window_settings,'b');
		}

		int fps;
		int divisor=frameskip+1;
		if (divisor==0) {
			fps=50; //Esto no deberia suceder nunca. Pero lo hacemos por una posible division por 0 (si frameskip fuera -1)
		}
		else {
			fps=50/divisor;
		}

		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_frameskip,NULL,"[%d] F~~rameskip (%d FPS)",frameskip,fps);
        menu_add_item_menu_spanish_format(array_menu_window_settings,"[%d] Saltar F~~rames (%d FPS)",frameskip,fps);
		menu_add_item_menu_shortcut(array_menu_window_settings,'r');
        menu_add_item_menu_tooltip(array_menu_window_settings,"Sets the number of frames to skip every time the screen needs to be refreshed");
        menu_add_item_menu_ayuda(array_menu_window_settings,"Sets the number of frames to skip every time the screen needs to be refreshed");
        menu_add_item_menu_es_avanzado(array_menu_window_settings);

        menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_frameskip_draw_zxdesktop_background,NULL,
            "Frameskip to ZX Desktop background","Frameskip en fondo ZX Desktop","Frameskip al fons del ZX Desktop");
        menu_add_item_menu_prefijo_format(array_menu_window_settings,"[%c]  ",(frameskip_draw_zxdesktop_background.v ? 'X' : ' ')  );
        menu_add_item_menu_tooltip(array_menu_window_settings,"Apply frameskip when drawing ZX Desktop background");
        menu_add_item_menu_ayuda(array_menu_window_settings,"Apply frameskip when drawing ZX Desktop background");        
        menu_add_item_menu_es_avanzado(array_menu_window_settings);


		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_autoframeskip,NULL,"[%c] ~~%s",        
				(autoframeskip.v ? 'X' : ' '),
                menu_get_string_language("Auto Frameskip")
        );

		menu_add_item_menu_shortcut(array_menu_window_settings,'a');	
        menu_add_item_menu_tooltip(array_menu_window_settings,"Let ZEsarUX decide when to skip frames");
        menu_add_item_menu_ayuda(array_menu_window_settings,"ZEsarUX skips frames when the host cpu use is too high. Then skiping frames the cpu use decreases");



        if (autoframeskip.v) {
            menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_no_autoframeskip_move_windows,NULL,
                "Also when moving objects","También al mover objetos","També al moure objectes");
            menu_add_item_menu_prefijo_format(array_menu_window_settings,"[%c]  ",(auto_frameskip_even_when_movin_windows.v ? 'X' : ' ')                
            );
            menu_add_item_menu_tooltip(array_menu_window_settings,"Autoframeskip even when moving icons or windows or resizing windows");
            menu_add_item_menu_ayuda(array_menu_window_settings,"Autoframeskip even when moving icons or windows or resizing windows. Enabling it uses less cpu when moving or resizing objects but "
                "can make objects disappear or not refresh quickly. Disabling it enhances refreshing objects when moving but uses more cpu and may slow down emulation");
            menu_add_item_menu_es_avanzado(array_menu_window_settings);
        }


#ifdef EMULATE_RASPBERRY
        menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_rpi_performance_params,NULL,
            "RPi slow improve performance","RPi lenta mejorar rendimiento","RPi lenta millorar rendiment");
        menu_add_item_menu_prefijo_format(array_menu_window_settings,"[%c] ",(cambio_parametros_maquinas_lentas.v ? 'X' : ' '));
        menu_add_item_menu_tooltip(array_menu_window_settings,"Change some performance parameters (frameskip, realvideo, etc) on slow machines like Rpi 1");
        menu_add_item_menu_ayuda(array_menu_window_settings,"Change some performance parameters (frameskip, realvideo, etc) on slow machines like Rpi 1");
        menu_add_item_menu_es_avanzado(array_menu_window_settings);

#endif        

		menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_flash,NULL,
            "Flash enabled","Parpadeo activado","Parpelleig activat");
        menu_add_item_menu_prefijo_format(array_menu_window_settings,"[%c] ",(disable_change_flash.v==0 ? 'X' : ' '));
        menu_add_item_menu_tooltip(array_menu_window_settings,"Disables flash for emulated machines and also for menu interface");
        menu_add_item_menu_ayuda(array_menu_window_settings,"Disables flash for emulated machines and also for menu interface");
        menu_add_item_menu_es_avanzado(array_menu_window_settings);


        if (mouse_menu_disabled.v==0) {
            menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_ignore_click_open_menu,NULL,
                "Cl~~icking mouse opens menu","Cl~~ick raton abre menú","Cl~~ick ratolí obre menú");
            menu_add_item_menu_prefijo_format(array_menu_window_settings,"[%c] ", (mouse_menu_ignore_click_open.v==0 ? 'X' : ' ') );

            
            menu_add_item_menu_tooltip(array_menu_window_settings,"Ignore mouse clicking to open menu or ZX Desktop buttons");
            menu_add_item_menu_shortcut(array_menu_window_settings,'i');
            menu_add_item_menu_ayuda(array_menu_window_settings,"Disabling this will make mouse be ignored when clicking on "
                "the window to open menu or pressing ZX Desktop buttons. The mouse can still be used when the menu is open");
            menu_add_item_menu_es_avanzado(array_menu_window_settings);
        }

		menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_setting_limit_menu_open,NULL,
            "Li~~mit menu opening","Li~~mitar apertura menú","Li~~mitar apertura menú");
		menu_add_item_menu_prefijo_format(array_menu_window_settings,"[%c] ",(menu_limit_menu_open.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_window_settings,'m');	
		menu_add_item_menu_tooltip(array_menu_window_settings,"Limit the action to open menu (F5 by default, joystick button)");			
		menu_add_item_menu_ayuda(array_menu_window_settings,"Limit the action to open menu (F5 by default, joystick button). To open it, you must press the key 3 times in one second");
        menu_add_item_menu_es_avanzado(array_menu_window_settings);



        if (si_complete_video_driver() ) {
                menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_zoom,menu_interface_zoom_cond,"[%d] Window Size ~~Zoom",zoom_x);
                menu_add_item_menu_shortcut(array_menu_window_settings,'z');
                menu_add_item_menu_tooltip(array_menu_window_settings,"Change Window Zoom");
                menu_add_item_menu_ayuda(array_menu_window_settings,"Changes Window Size Zoom (width and height)");

                menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_zoom_autochange_big_display,NULL,
                    "[%c] Autochange Zoom big display",(autochange_zoom_big_display.v ? 'X' : ' ' )); 
                menu_add_item_menu_tooltip(array_menu_window_settings,"Autochange to zoom 1 when switching to machine with big display (Next, QL, CPC, ...)");
                menu_add_item_menu_ayuda(array_menu_window_settings,"Autochange to zoom 1 when switching to machine with big display (Next, QL, CPC, ...)");
        }


		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_window_settings_reduce_075,NULL,"[%c] R~~educe to 0.75",(screen_reduce_075.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_window_settings,'e');
		menu_add_item_menu_tooltip(array_menu_window_settings,"Reduce machine display output by 0.75. Enables realvideo and forces watermark");
		menu_add_item_menu_ayuda(array_menu_window_settings,"Reduce machine display output by 0.75. Enables realvideo and forces watermark. This feature has been used on a large bulb display for the RunZX 2018 event");
        menu_add_item_menu_es_avanzado(array_menu_window_settings);

		if (screen_reduce_075.v) {
			menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_window_settings_reduce_075_antialias,NULL,"[%c]  Antialias",(screen_reduce_075_antialias.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_window_settings,"Antialias is only applied to the standard 16 Spectrum colors");
			menu_add_item_menu_ayuda(array_menu_window_settings,"Antialias is only applied to the standard 16 Spectrum colors");
            menu_add_item_menu_es_avanzado(array_menu_window_settings);

			menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_window_settings_reduce_075_ofx,NULL,"[%d]  Offset x",screen_reduce_offset_x);
            menu_add_item_menu_es_avanzado(array_menu_window_settings);

			menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_window_settings_reduce_075_ofy,NULL,"[%d]  Offset y",screen_reduce_offset_y);
            menu_add_item_menu_es_avanzado(array_menu_window_settings);
		}
		

		/*"--reduce-075               Reduce display size 4/3 (divide by 4, multiply by 3)\n"
		"--reduce-075-offset-x n    Destination offset x on reduced display\n"
		"--reduce-075-offset-y n    Destination offset y on reduced display\n"*/



		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_footer,NULL,"[%c] Window F~~ooter",(menu_footer ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_window_settings,'o');
		menu_add_item_menu_tooltip(array_menu_window_settings,"Show on footer some machine information");
		menu_add_item_menu_ayuda(array_menu_window_settings,"Show on footer some machine information, like tape loading");
        menu_add_item_menu_es_avanzado(array_menu_window_settings);


		if (menu_footer) {
			menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_show_fps,NULL,"[%c] Show F~~PS",(screen_show_fps.v ? 'X' : ' ') );
            menu_add_item_menu_spanish_format(array_menu_window_settings,"[%c] Mostrar F~~PS",(screen_show_fps.v ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_window_settings,'p');
			menu_add_item_menu_tooltip(array_menu_window_settings,"Show FPS on footer");
			menu_add_item_menu_ayuda(array_menu_window_settings,"It tells the current FPS");
            menu_add_item_menu_es_avanzado(array_menu_window_settings);
												
		}


		//Uso cpu no se ve en windows
#ifndef MINGW
		if (menu_footer) {
			menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_show_cpu_usage,NULL,
                "Show ~~CPU usage","Mostrar uso ~~CPU","Mostrar us ~~CPU");
            menu_add_item_menu_prefijo_format(array_menu_window_settings,"[%c] ",(screen_show_cpu_usage.v ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_window_settings,'c');
			menu_add_item_menu_tooltip(array_menu_window_settings,"Show CPU usage on footer");
			menu_add_item_menu_ayuda(array_menu_window_settings,"It tells you how much host cpu machine is using ZEsarUX. So it's better to have it low. "
															"Higher values mean you need a faster host machine to use ZEsarUX");
            menu_add_item_menu_es_avanzado(array_menu_window_settings);
		}
#endif

		//temperatura cpu solo se ve en Linux
#ifdef __linux__
		if (menu_footer) {
			menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_show_cpu_temp,NULL,"[%c] Show CPU temperature",(screen_show_cpu_temp.v ? 'X' : ' ') );
            menu_add_item_menu_spanish_format(array_menu_window_settings,"[%c] Mostrar temperatura CPU",(screen_show_cpu_temp.v ? 'X' : ' ') );
			//menu_add_item_menu_shortcut(array_menu_window_settings,'c');
			menu_add_item_menu_tooltip(array_menu_window_settings,"Show CPU temperature on footer");
			menu_add_item_menu_ayuda(array_menu_window_settings,"It tells the temperature of the main CPU");
            menu_add_item_menu_es_avanzado(array_menu_window_settings);
												
		}
#endif

        char idioma[32];
        strcpy(idioma,"Default");
        if (gui_language==GUI_LANGUAGE_SPANISH) strcpy(idioma,"Español");
        if (gui_language==GUI_LANGUAGE_CATALAN) strcpy(idioma,"Català");

       	menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_language,NULL,
            "~~Language","~~Lenguaje","~~Llenguatge");
        menu_add_item_menu_prefijo_format(array_menu_window_settings,"[%s] ",idioma);
        menu_add_item_menu_shortcut(array_menu_window_settings,'l');


        char string_online_download_path[16];
        menu_tape_settings_trunc_name(online_download_path,string_online_download_path,16);

        menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_change_online_download_path,NULL,
        "Online Download Path","Ruta descargas online","Ruta descarregues online");
        menu_add_item_menu_prefijo_format(array_menu_window_settings,"[%s] ",string_online_download_path);
        menu_add_item_menu_tooltip(array_menu_window_settings,"Where to download files from the speccy and zx81 online browser");
        menu_add_item_menu_ayuda(array_menu_window_settings,"Where to download files from the speccy and zx81 online browser. If not set, they are download to a temporary folder");


        
        //Con driver cocoa, no permitimos cambiar a otro driver
		if (strcmp(scr_new_driver_name,"cocoa")) {
            menu_add_item_menu(array_menu_window_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

			menu_add_item_menu_en_es_ca(array_menu_window_settings,MENU_OPCION_NORMAL,menu_change_video_driver,menu_change_video_driver_cond,
                "Change Video Driver","Cambiar Driver Video","Canviar Driver Video");
            menu_add_item_menu_es_avanzado(array_menu_window_settings);
		}


	
        menu_add_item_menu(array_menu_window_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        
		menu_add_ESC_item(array_menu_window_settings);

        //retorno_menu=menu_dibuja_menu(&window_settings_opcion_seleccionada,&item_seleccionado,array_menu_window_settings,"ZEsarUX Window Settings" );
        retorno_menu=menu_dibuja_menu(&window_settings_opcion_seleccionada,&item_seleccionado,array_menu_window_settings,"General Settings" );

        

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                        
                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}


void menu_interface_change_gui_style_apply(MENU_ITEM_PARAMETERS)
{

    //Si se pulsa Enter
    estilo_gui_activo=valor_opcion;

    set_charset_from_gui();

    menu_init_footer();

    zxvision_restart_all_background_windows();
}

void menu_interface_change_gui_style_test(MENU_ITEM_PARAMETERS)
{
    menu_espera_no_tecla();
    menu_reset_counters_tecla_repeticion();


    int ancho=32;
    int alto=24;
    int x=menu_center_x()-ancho/2;
    int y=menu_center_y()-alto/2;


    zxvision_window ventana;

    zxvision_new_window(&ventana,x,y,ancho,alto,ancho-1,alto-2,"Test style");

    zxvision_draw_window(&ventana);

    int linea=0;

    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,"Normal Text");

    zxvision_print_string(&ventana,1,linea,ESTILO_GUI_TINTA_SELECCIONADO,ESTILO_GUI_PAPEL_SELECCIONADO,0,"Selected Text"); 
    if (ESTILO_GUI_MUESTRA_CURSOR) zxvision_print_string(&ventana,0,linea,ESTILO_GUI_TINTA_SELECCIONADO,ESTILO_GUI_PAPEL_SELECCIONADO,0,">");
    linea++;

    zxvision_print_string(&ventana,1,linea,ESTILO_GUI_TINTA_NO_DISPONIBLE,ESTILO_GUI_PAPEL_NO_DISPONIBLE,0,"Unavailable Text");
    if (ESTILO_GUI_MUESTRA_CURSOR) zxvision_print_string(&ventana,0,linea,ESTILO_GUI_TINTA_NO_DISPONIBLE,ESTILO_GUI_PAPEL_NO_DISPONIBLE,0,"x");
    linea++;


    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_SEL_NO_DISPONIBLE,ESTILO_GUI_PAPEL_SEL_NO_DISPONIBLE,0,"Unavailable Selected Text");

    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_OPCION_MARCADA,ESTILO_GUI_PAPEL_OPCION_MARCADA,0,"Marked Text");

    //Así es tal como lo muestra en texto de volumen
    char warn_colour='0'+ESTILO_GUI_COLOR_AVISO;
    char buffer_text[64];
    sprintf(buffer_text,"$$%cWarning text",warn_colour);
    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,buffer_text);

    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,"~~Hotkey");


    //El parpadeo es igual en todos los temas (de momento) pero también lo mostramos
    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,1,"Flashing Text");

    z80_byte caracter_titulo=menu_retorna_caracter_espacio_titulo();
    char buffer_title_text[32];
    sprintf(buffer_title_text,"Title text %c%c%c",caracter_titulo,caracter_titulo,caracter_titulo);
    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_TITULO,ESTILO_GUI_PAPEL_TITULO,0,buffer_title_text);

    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_TITULO_INACTIVA,ESTILO_GUI_PAPEL_TITULO_INACTIVA,0,"Inactive Title Text");

    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_COLOR_RECUADRO,0,"Window Box colour (this paper)");

    //tinta waveform y tinta normal se usan a la vez en widget tipo speaker. por tanto interesa que no sean iguales
    //asi este item lo mostramos combinando los dos colores
    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_COLOR_WAVEFORM,0,"Waveform colour (this paper)");

    //tinta 0 - negro es la de intensidad mas baja. papel el del unused
    //Poner una tinta negra normalmente a no ser que el papel vaya a ser negro tambien
    //Aunque no debería indicar color de zona usada visualmem en negro, pues en la ventana es el color al que se llegara (dependiendo intensidad),
    //segun la cantidad de accesos a memoria
    int tinta_visualmem_text=0;
    if (ESTILO_GUI_COLOR_UNUSED_VISUALMEM==0) {
        tinta_visualmem_text=7;
    }

    zxvision_print_string(&ventana,1,linea++,tinta_visualmem_text,ESTILO_GUI_COLOR_UNUSED_VISUALMEM,0,"Unused visualmem (this paper)");

    //En visual tape se deben poder distinguir ESTILO_GUI_COLOR_WAVEFORM de ESTILO_GUI_COLOR_BLOCK_VISUALTAPE
    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_COLOR_WAVEFORM,ESTILO_GUI_COLOR_BLOCK_VISUALTAPE,0,"Visual tape block (this paper)");

    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,"Ascii table:");
    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,"!\"#$%&\'()*+,-./0123456789:;<=>");
    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,"?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\");
    zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,"]^_`abcdefghijklmnopqrstuvwxyz");



    if (si_complete_video_driver()) {
        zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,"{|}~\x7f"); 
        //el 127 en teoria no es ascii
        //aunque en mis tipos de letras si que esta y lo pongo

        zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,"Extra characters:");
        int i;

        //Aunque este buffer esta pensado para maximo 32 caracteres de la linea
        char buffer_extra[64];
        int posicion=0;
        for (i=128;i<128+ancho-2;i++,posicion++) {
            buffer_extra[posicion]=i;
        }

        buffer_extra[posicion]=0;
        zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,buffer_extra);

        posicion=0;
        for (;i<=MAX_CHARSET_GRAPHIC;i++,posicion++) {
            buffer_extra[posicion]=i;
        }

        buffer_extra[posicion]=0;
        zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,buffer_extra);        
    }

    else {
        //Sin el 127 porque no es ascii y en drivers de texto no se veria
        zxvision_print_string(&ventana,1,linea++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,"{|}~"); 
    }


    zxvision_draw_window_contents(&ventana);

    zxvision_wait_until_esc(&ventana);

    cls_menu_overlay();    

    zxvision_destroy_window(&ventana);
}

void menu_interface_change_gui_style(MENU_ITEM_PARAMETERS)
{
    int common_opcion_seleccionada=estilo_gui_activo;


    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

    do {

        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

        int i;
        for (i=0;i<ESTILOS_GUI;i++) {

            if (!si_complete_video_driver() && definiciones_estilos_gui[i].require_complete_video_driver) {
                //El estilo requiere video driver completo. Siguiente
                //printf ("no puedo seleccionar: %s\n",definiciones_estilos_gui[i].nombre_estilo);
                //Y ademas movemos el cursor al principio, pues hemos quitado uno al menos de la lista y el cursor no correspondera
                common_opcion_seleccionada=0;
            }            
            
            else {
                menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_interface_change_gui_style_apply,NULL,definiciones_estilos_gui[i].nombre_estilo);
                menu_add_item_menu_valor_opcion(array_menu_common,i);
            }

        }

        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        //Y opcion para probar estilo
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_interface_change_gui_style_test,NULL,"~~Test style");
        menu_add_item_menu_shortcut(array_menu_common,'t');



        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&common_opcion_seleccionada,&item_seleccionado,array_menu_common,"Style" );

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }        


    
    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

void menu_interface_hidemouse(MENU_ITEM_PARAMETERS)
{
    debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;
	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);



	mouse_pointer_shown.v ^=1;

	screen_init_pantalla_and_others_and_realjoystick();

    debug_printf(VERBOSE_INFO,"Creating Screen");

	menu_init_footer();


	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);	
	
}

void menu_interface_restore_windows_geometry(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Restore windows geometry")) {
		util_clear_all_windows_geometry();
		menu_generic_message("Restore windows geometry","OK. All windows restored to their default positions and sizes");
	}
}

void menu_interface_reopen_background_windows_on_start(MENU_ITEM_PARAMETERS)
{
	menu_reopen_background_windows_on_start.v ^=1;
}


void menu_interface_allow_background_windows_always_force(MENU_ITEM_PARAMETERS)
{
    always_force_overlay_visible_when_menu_closed ^=1;
}

void menu_interface_allow_background_windows_delete_windows(void)
{
    zxvision_window_delete_all_windows();
    cls_menu_overlay();    
}

void menu_interface_allow_background_windows(MENU_ITEM_PARAMETERS)
{

	//Borrar todas si vamos a desactivarlo
	if (menu_allow_background_windows) {
		menu_interface_allow_background_windows_delete_windows();
	}

	menu_allow_background_windows ^=1;

	/*if (menu_allow_background_windows) {
		menu_warn_message("DANGER! This is very EXPERIMENTAL! Put a windows in background (not menu window) by pressing F6");
	}*/

}

void menu_interface_multitask(MENU_ITEM_PARAMETERS)
{

	menu_multitarea=menu_multitarea^1;
	if (menu_multitarea==0) {
		audio_playing.v=0;
	}
	timer_reset();

}

void menu_interface_menu_emulation_paused(MENU_ITEM_PARAMETERS)
{
    menu_emulation_paused_on_menu ^=1;

	if (menu_emulation_paused_on_menu) {
		audio_playing.v=0;
	}    

    timer_reset();
}

void menu_interface_hide_vertical_perc_bar(MENU_ITEM_PARAMETERS)
{
		menu_hide_vertical_percentaje_bar.v ^=1;
}



void menu_interface_hide_minimize_button(MENU_ITEM_PARAMETERS)
{
	menu_hide_minimize_button.v ^=1;
}

void menu_interface_hide_maximize_button(MENU_ITEM_PARAMETERS)
{
	menu_hide_maximize_button.v ^=1;
}

void menu_interface_hide_close_button(MENU_ITEM_PARAMETERS)
{
	menu_hide_close_button.v ^=1;
}

void menu_interface_invert_mouse_scroll(MENU_ITEM_PARAMETERS)
{
	menu_invert_mouse_scroll.v ^=1;
}

/*void menu_bw_no_multitask(MENU_ITEM_PARAMETERS)
{
	screen_bw_no_multitask_menu.v ^=1;
}*/

void menu_interface_force_confirm_yes(MENU_ITEM_PARAMETERS)
{
	force_confirm_yes.v ^=1;
}

void menu_interface_force_atajo(MENU_ITEM_PARAMETERS)
{
        menu_force_writing_inverse_color.v ^=1;
}

void menu_interface_tooltip(MENU_ITEM_PARAMETERS)
{
	tooltip_enabled.v ^=1;
	menu_tooltip_counter=0;
}


void menu_interface_disable_menu_mouse(MENU_ITEM_PARAMETERS)
{
    mouse_menu_disabled.v ^=1;
}

void menu_setting_quickexit(MENU_ITEM_PARAMETERS)
{
	quickexit.v ^=1;
}

void menu_setting_select_machine_by_name(MENU_ITEM_PARAMETERS)
{
	setting_machine_selection_by_name.v ^=1;
}

void menu_interface_first_aid(MENU_ITEM_PARAMETERS)
{
	menu_disable_first_aid.v ^=1;
}

void menu_interface_restore_first_aid(MENU_ITEM_PARAMETERS)
{
	menu_first_aid_restore_all();

	menu_generic_message("Restore messages","OK. Restored all first aid messages");
}

void menu_interface_charwidth_after_width_change(void)
{
	//Reorganizar ventanas en background segun nuevo tamaño caracter
	if (menu_allow_background_windows) zxvision_rearrange_background_windows();	
}

void menu_interface_charwidth(MENU_ITEM_PARAMETERS)
{
	menu_char_width--;

	if (menu_char_width==4) menu_char_width=8;

    menu_interface_charwidth_after_width_change();


}

void menu_interface_charheight(MENU_ITEM_PARAMETERS)
{
	menu_char_height--;

	if (menu_char_height==5) menu_char_height=8;

    menu_interface_charwidth_after_width_change();


}


void menu_interface_hide_submenu_indicator(MENU_ITEM_PARAMETERS)
{
    menu_hide_submenu_indicator.v ^=1;
}

void menu_interface_charset(MENU_ITEM_PARAMETERS)
{

    user_charset++;
    if (charset_list[user_charset].puntero==NULL) user_charset=-1;

    set_user_charset();

}

void menu_interface_hide_background_button_on_inactive(MENU_ITEM_PARAMETERS)
{
    menu_hide_background_button_on_inactive.v ^=1;
}

void menu_interface_bw_no_multitask(MENU_ITEM_PARAMETERS)
{
	screen_machine_bw_no_multitask.v ^=1;
}

void menu_interface_mix_menu(MENU_ITEM_PARAMETERS)
{
	screen_menu_mix_method++;
	if (screen_menu_mix_method==MAX_MENU_MIX_METHODS) screen_menu_mix_method=0;
}

void menu_interface_mix_tranparency(MENU_ITEM_PARAMETERS)
{


	char string_trans[3];

        sprintf (string_trans,"%d",screen_menu_mix_transparency);

        menu_ventana_scanf("Transparency? (0-95)",string_trans,3);

        int valor=parse_string_to_number(string_trans);
	if (valor<0 || valor>95) {
		debug_printf (VERBOSE_ERR,"Invalid value");
	}

	else {
		screen_menu_mix_transparency=valor;
	}


}


void menu_interface_reduce_bright_menu(MENU_ITEM_PARAMETERS)
{
	screen_menu_reduce_bright_machine.v ^=1;
}

void menu_special_fx_settings(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_special_fx_settings;
    menu_item item_seleccionado;
    int retorno_menu;
    do {


/*
0=Menu por encima de maquina, si no es transparente
1=Menu por encima de maquina, si no es transparente. Y Color Blanco con brillo es transparente
2=Mix de los dos colores, con control de transparecnai
*/


        menu_add_item_menu_inicial_format(&array_menu_special_fx_settings,MENU_OPCION_NORMAL,menu_interface_mix_menu,NULL,"Menu Mix Method");
        menu_add_item_menu_spanish_catalan(array_menu_special_fx_settings,"Metodo mezclado menu","Metode barreja menu");
        menu_add_item_menu_prefijo_format(array_menu_special_fx_settings,"[%s] ",screen_menu_mix_methods_strings[screen_menu_mix_method] );
        menu_add_item_menu_tooltip(array_menu_special_fx_settings,"How to mix menu and the layer below");
        menu_add_item_menu_ayuda(array_menu_special_fx_settings,"How to mix menu and the layer below");

        if (screen_menu_mix_method==2) {
            menu_add_item_menu_en_es_ca(array_menu_special_fx_settings,MENU_OPCION_NORMAL,menu_interface_mix_tranparency,NULL,
                "Transparency","Transparencia","Transparencia");
            menu_add_item_menu_prefijo_format(array_menu_special_fx_settings,"[%d%%] ",screen_menu_mix_transparency );
            menu_add_item_menu_tooltip(array_menu_special_fx_settings,"Transparency percentage to apply to menu");
            menu_add_item_menu_ayuda(array_menu_special_fx_settings,"Transparency percentage to apply to menu");
        }

        if (screen_menu_mix_method==0 || screen_menu_mix_method==1) {
            //Lo desactivo. Esto da problemas con footer
            /*
            menu_add_item_menu_format(array_menu_special_fx_settings,MENU_OPCION_NORMAL,menu_interface_reduce_bright_menu,NULL,"[%c] Darken when menu",(screen_menu_reduce_bright_machine.v ? 'X' : ' ' ) );
            menu_add_item_menu_tooltip(array_menu_special_fx_settings,"Darken layer below menu when menu open");
            menu_add_item_menu_ayuda(array_menu_special_fx_settings,"Darken layer below menu when menu open");
            */
        }
    


        menu_add_item_menu_en_es_ca(array_menu_special_fx_settings,MENU_OPCION_NORMAL,menu_interface_bw_no_multitask,NULL,
            "B&W on menu+no multitask","B&N en menu+no multitarea","B&N al menu+no multitasca");
        menu_add_item_menu_prefijo_format(array_menu_special_fx_settings,"[%c] ",(screen_machine_bw_no_multitask.v ? 'X' : ' ' ) );
        menu_add_item_menu_tooltip(array_menu_special_fx_settings,"Grayscale layer below menu when menu opened and multitask is disabled");
        menu_add_item_menu_ayuda(array_menu_special_fx_settings,"Grayscale layer below menu when menu opened and multitask is disabled");


                    

        menu_add_item_menu(array_menu_special_fx_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        
        menu_add_ESC_item(array_menu_special_fx_settings);

        retorno_menu=menu_dibuja_menu(&special_fx_settings_opcion_seleccionada,&item_seleccionado,array_menu_special_fx_settings,"Special FX Settings" );

            

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                
            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

void menu_interface_mouse_right_esc(MENU_ITEM_PARAMETERS)
{
    menu_mouse_right_send_esc.v ^=1;
}



void menu_zxvision_settings(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

    do {

	
		menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,menu_interface_charwidth,NULL,"Menu char width");
        menu_add_item_menu_spanish_catalan(array_menu_common,"Ancho de caracter de menú","Ample de caracter de menú");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%d] ",menu_char_width);
		//menu_add_item_menu_shortcut(array_menu_common,'i');	
		menu_add_item_menu_tooltip(array_menu_common,"Menu character width");
		menu_add_item_menu_ayuda(array_menu_common,"Menu character width. You can reduce it so allowing more text columns in a window");


		menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_interface_charheight,NULL,"Menu char height");
        menu_add_item_menu_spanish_catalan(array_menu_common,"Altura de caracter de menú","Alçada de caracter de menú");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%d] ",menu_char_height);
		menu_add_item_menu_tooltip(array_menu_common,"Menu character height");
		menu_add_item_menu_ayuda(array_menu_common,"Menu character height. You can reduce it so allowing more text rows in a window");

		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_first_aid,NULL,
            "First aid help","Ayuda de primeros auxilios","Ajuda de primers auxilis");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_disable_first_aid.v==0 ? 'X' : ' ') );	
		menu_add_item_menu_tooltip(array_menu_common,"Enable or disable First Aid help");
		menu_add_item_menu_ayuda(array_menu_common,"Enable or disable First Aid help");		
        

        if (menu_disable_first_aid.v==0) {

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_restore_first_aid,NULL,
                "    Restore all 1st aid mess.","    Restaurar todos mens. 1r auxi.","    Restaurar tots miss. 1r auxi.");
            menu_add_item_menu_tooltip(array_menu_common,"Restore all First Aid help messages");
            menu_add_item_menu_ayuda(array_menu_common,"Restore all First Aid help messages");
            menu_add_item_menu_es_avanzado(array_menu_common);

        }


		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_setting_select_machine_by_name,NULL,
            "Select machine by name","Seleccionar maquina por nombre","Escollir maquina pel nom");
		menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(setting_machine_selection_by_name.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_common,"Select machine by name instead of manufacturer on menu Machine");
		menu_add_item_menu_ayuda(array_menu_common,"Select machine by name instead of manufacturer on menu Machine");
        menu_add_item_menu_es_avanzado(array_menu_common);



		menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_setting_quickexit,NULL,"[%c] ~~Quick exit",
			(quickexit.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_common,'q');	
		menu_add_item_menu_tooltip(array_menu_common,"Exit emulator quickly: no yes/no confirmation and no fadeout");			
		menu_add_item_menu_ayuda(array_menu_common,"Exit emulator quickly: no yes/no confirmation and no fadeout");
        menu_add_item_menu_es_avanzado(array_menu_common);


		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_change_gui_style,NULL,
            "    ~~Style","    E~~stilo","    E~~stil");

		menu_add_item_menu_sufijo_format(array_menu_common," [%s]",definiciones_estilos_gui[estilo_gui_activo].nombre_estilo);


		menu_add_item_menu_shortcut(array_menu_common,'s');
		menu_add_item_menu_tooltip(array_menu_common,"Change GUI Style");
                menu_add_item_menu_ayuda(array_menu_common,"You can switch between:\n"
					"- ZEsarUX: default style\n"
                    "- ZEsarUX matte: same as default but less shiny\n"
					"- ZXSpectr: my first emulator created on 1996, that worked on MS-DOS and Windows\n"
                    "- ManSoftware: style using my own font I created when I was a child ;)\n"
					"- ZX80/81: ZX80&81 style\n"
                    "- QL: Sinclair QL style\n"
					"- Z88: Z88 style\n"
                    "- Sam: Sam Coupe style\n"
					"- CPC: Amstrad CPC style\n"
					"- MSX: MSX style\n"
					"- RetroMac: MacOS classic style\n"
                    "- AmigaOS: AmigaOS 1.3 style\n"
                    "- AtariTOS: Atari TOS style\n"
                    "- BeOS: BeOS operating system style\n"
					"- Borland: Borland MS-DOS programs style\n"
                    "- Turbovision: Borland Turbovision GUI style\n"
                    "- Ocean: Blue style\n"
                    "- Bloody: Red style\n"
                    "- Panther: Pink style\n"
                    "- Grass: Green style\n"
                    "- Sky: Cyan style\n"
                    "- Sunny: Yellow style\n"
					"- Clean: Simple style with black & white menus\n"
					"- CleanInverse: Same style as previous but using inverted colours\n"
                    "- Solarized Dark/Light: Solarized styles\n"
					"\nNote: Some styles (like Solarized) need a full video driver, can't be set on curses or aalib for example"
					
					);
        menu_add_item_menu_tiene_submenu(array_menu_common);

        char temp_charset[MAX_CHARSET_NAME];

        if (user_charset>=0) {
            sprintf(temp_charset," [%s]",charset_list[user_charset].nombre);
        }
        else {
            strcpy(temp_charset," [ ]");
        }
        
       
        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_charset,NULL,
            "    Custom charset","    Charset personalizado","    Charset personalitzat");
        menu_add_item_menu_sufijo(array_menu_common,temp_charset);
        menu_add_item_menu_es_avanzado(array_menu_common);
    


        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);


        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_disable_menu_mouse,NULL,
            "Use mouse on menu","Usar ratón en el menu","Usar ratolí al menu");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ", (mouse_menu_disabled.v==0 ? 'X' : ' ') );
        menu_add_item_menu_es_avanzado(array_menu_common);
        //menu_add_item_menu_shortcut(array_menu_common,'u');      

        if (mouse_menu_disabled.v==0) {
            if (!strcmp(scr_new_driver_name,"xwindows")  || !strcmp(scr_new_driver_name,"sdl") || !strcmp(scr_new_driver_name,"cocoa") ) {
                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_hidemouse,NULL,
                    "Mouse pointer","Puntero del raton","Punter del ratolí");
                menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ", (mouse_pointer_shown.v==1 ? 'X' : ' ') );
                //menu_add_item_menu_shortcut(array_menu_common,'m');
                menu_add_item_menu_es_avanzado(array_menu_common);
            }
        }

		menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_interface_tooltip,NULL,"[%c] ~~Tooltips",(tooltip_enabled.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_common,'t');	
		menu_add_item_menu_tooltip(array_menu_common,"Enable or disable tooltips");
		menu_add_item_menu_ayuda(array_menu_common,"Enable or disable tooltips");          
        
		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_force_atajo,NULL,
            "Force visible ~~hotkeys","Forzar visibilidad ~~hotkeys","Forçar visibilitat ~~hotkeys");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_force_writing_inverse_color.v ? 'X' : ' ') );
        menu_add_item_menu_shortcut(array_menu_common,'h');
		menu_add_item_menu_tooltip(array_menu_common,"Force always show hotkeys");
		menu_add_item_menu_ayuda(array_menu_common,"Force always show hotkeys. By default it will only be shown after a timeout or wrong key pressed");
        menu_add_item_menu_es_avanzado(array_menu_common);

		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_force_confirm_yes,NULL,
            "Force confirm yes","Forzar confirmaciones a si","Forçar confirmacions a si");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(force_confirm_yes.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_common,"Force confirmation dialogs yes/no always to yes");
		menu_add_item_menu_ayuda(array_menu_common,"Force confirmation dialogs yes/no always to yes");
        menu_add_item_menu_es_avanzado(array_menu_common);



		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_hide_vertical_perc_bar,NULL,
            "Percentage bar","Barra de porcentaje","Barra de percentatge");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_hide_vertical_percentaje_bar.v==0 ? 'X' : ' ') );
		//menu_add_item_menu_shortcut(array_menu_common,'p');
		menu_add_item_menu_tooltip(array_menu_common,"Shows vertical percentage bar on the right of text windows and file browser");
		menu_add_item_menu_ayuda(array_menu_common,"Shows vertical percentage bar on the right of text windows and file browser");
        menu_add_item_menu_es_avanzado(array_menu_common);


		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_hide_submenu_indicator,NULL,
            "Submenu indicator","Indicador de submenu","Indicador de submenu");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_hide_submenu_indicator.v==0 ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_common,"Shows submenu indicator character (>) on menu items with submenus");
        menu_add_item_menu_ayuda(array_menu_common,"Shows submenu indicator character (>) on menu items with submenus");
        menu_add_item_menu_es_avanzado(array_menu_common);


		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_hide_minimize_button,NULL,
            "Minimize button","Boton de minimizar","Boto de minimitzar");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_hide_minimize_button.v ? ' ' : 'X') );
        menu_add_item_menu_es_avanzado(array_menu_common);

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_hide_maximize_button,NULL,
            "Maximize button","Boton de maximizar","Boto de maximitzar");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_hide_maximize_button.v ? ' ' : 'X') );
        menu_add_item_menu_es_avanzado(array_menu_common);
		
		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_hide_close_button,NULL,
            "Close button","Boton de cerrar","Boto de tancar");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_hide_close_button.v ? ' ' : 'X') );
        menu_add_item_menu_es_avanzado(array_menu_common);


        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_hide_background_button_on_inactive,NULL,
            "Background button on inactive","Boton de background en inactivo","Boto de background a inactiu");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_hide_background_button_on_inactive.v ? ' ' : 'X') );
        menu_add_item_menu_tooltip(array_menu_common,"Shows background button flashing on inactive windows");
        menu_add_item_menu_ayuda(array_menu_common,"Shows background button flashing on inactive windows");
        menu_add_item_menu_es_avanzado(array_menu_common);

		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_invert_mouse_scroll,NULL,
            "Invert mouse scroll","Invertir scroll raton","Invertir scroll ratoli");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_invert_mouse_scroll.v ? 'X' : ' ') );
        menu_add_item_menu_es_avanzado(array_menu_common);


		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_mouse_right_esc,NULL,
            "Right mouse sends ESC","Boton derecho envia ESC","Botó dret envia ESC");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_mouse_right_send_esc.v ? 'X' : ' ') ); 
        menu_add_item_menu_tooltip(array_menu_common,"Right button mouse simulate ESC key or secondary actions");
        menu_add_item_menu_ayuda(array_menu_common,"Right button mouse simulate ESC key or secondary actions");
        menu_add_item_menu_es_avanzado(array_menu_common);

        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);


        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_multitask,NULL,
            "M~~ultitask menu","Menu m~~ultitarea","Menu m~~ultitasca");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ", (menu_multitarea==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_common,'u');
		menu_add_item_menu_tooltip(array_menu_common,"When multitask is disabled, both emulation, background windows and other menu features are stopped when opening the menu");
        menu_add_item_menu_ayuda(array_menu_common,"When multitask is disabled, both emulation, background windows and other menu features are stopped when opening the menu");


        if (menu_multitarea) {
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_menu_emulation_paused,NULL,
                "Sto~~p emulation on menu","Sto~~p emulación en menu","Sto~~p emulació al menu");
            menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_emulation_paused_on_menu ? 'X' : ' ' ));

            menu_add_item_menu_shortcut(array_menu_common,'p');
            menu_add_item_menu_tooltip(array_menu_common,"When multitask is enabled, you can disable emulation when opening the menu");
            menu_add_item_menu_ayuda(array_menu_common,"When multitask is enabled, you can disable emulation when opening the menu");
        }



        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_allow_background_windows,NULL,
            "~~Background windows","Ventanas en ~~background","Finestres a ~~background");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_allow_background_windows ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_common,'b');
        menu_add_item_menu_tooltip(array_menu_common,"You can allow some menu windows to be put on the background. See Help-> Background Windows Help for more info");
		menu_add_item_menu_ayuda(array_menu_common,"You can allow some menu windows to be put on the background. See Help-> Background Windows Help for more info");


        if (menu_allow_background_windows && menu_multitarea) {
           menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_allow_background_windows_always_force,NULL,
            "Even when menu closed","Incluso con menu cerrado","Inclús amb menu tancat");
            menu_add_item_menu_prefijo_format(array_menu_common,"[%c]  ",(always_force_overlay_visible_when_menu_closed ? 'X' : ' ') ); 
           menu_add_item_menu_tooltip(array_menu_common,"Shows background window even when menu closed");
           menu_add_item_menu_ayuda(array_menu_common,"Shows background window even when menu closed");
        }

		if (menu_allow_background_windows && menu_multitarea && save_configuration_file_on_exit.v) {
			menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_reopen_background_windows_on_start,NULL,
                "Reopen windows on start","Reabrir ventanas al inicio","Reobrir finestres al inici");
            menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_reopen_background_windows_on_start.v ? 'X' : ' ') );
		}

		menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_interface_restore_windows_geometry,NULL,
            "    Restore windows geometry","    Restaurar geometria ventanas","    Restaurar geometria finestres");
		menu_add_item_menu_tooltip(array_menu_common,"Restore all windows positions and sizes to their default values");
		menu_add_item_menu_ayuda(array_menu_common,"Restore all windows positions and sizes to their default values");
        menu_add_item_menu_es_avanzado(array_menu_common);

        

        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		if (si_complete_video_driver() ) {
			menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_special_fx_settings,NULL,
                "Special ~~FX","E~~fectos especiales","E~~fectes especials");

            menu_add_item_menu_shortcut(array_menu_common,'f');
            menu_add_item_menu_tiene_submenu(array_menu_common);
        }

						

        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        //menu_add_item_menu(array_menu_common,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&zxvision_settings_opcion_seleccionada,&item_seleccionado,array_menu_common,"ZX Vision Settings" );

                

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                        
                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}




void menu_osd_settings_watermark(MENU_ITEM_PARAMETERS)
{
	if (screen_watermark_enabled.v==0) {
		//Ya se permite watermark con o sin realvideo
		//enable_rainbow();
		screen_watermark_enabled.v=1;
	}

	else screen_watermark_enabled.v=0;
}

void menu_osd_settings_watermark_position(MENU_ITEM_PARAMETERS)
{
	screen_watermark_position++;
	if (screen_watermark_position>3) screen_watermark_position=0;
}

void menu_interface_show_splash_texts(MENU_ITEM_PARAMETERS)
{
	screen_show_splash_texts.v ^=1;
}


void menu_osd_settings(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_osd_settings;
    menu_item item_seleccionado;
    int retorno_menu;
    do {



    menu_add_item_menu_en_es_ca_inicial(&array_menu_osd_settings,MENU_OPCION_NORMAL,menu_interface_show_splash_texts,NULL,
        "~~Show splash texts","Mo~~strar textos splash","Mo~~strar textos splash");
    menu_add_item_menu_prefijo_format(array_menu_osd_settings,"[%c] ",(screen_show_splash_texts.v ? 'X' : ' ' ) );
    menu_add_item_menu_tooltip(array_menu_osd_settings,"Show on display some splash texts, like display mode change or watches");
    menu_add_item_menu_ayuda(array_menu_osd_settings,"Show on display some splash texts, like display mode change or watches");
    menu_add_item_menu_shortcut(array_menu_osd_settings,'s');


    menu_add_item_menu_en_es_ca(array_menu_osd_settings,MENU_OPCION_NORMAL,menu_osd_settings_watermark,NULL,
        "W~~atermark","Marca de ~~agua","Marca d'~~aigüa");
    menu_add_item_menu_prefijo_format(array_menu_osd_settings,"[%c] ",(screen_watermark_enabled.v ? 'X' : ' ' ) );
    menu_add_item_menu_tooltip(array_menu_osd_settings,"Adds a watermark to the display");
    menu_add_item_menu_ayuda(array_menu_osd_settings,"Adds a watermark to the display. May produce flickering if not enabled realvideo. If using reduce window setting, it will be forced enabled");
    menu_add_item_menu_shortcut(array_menu_osd_settings,'a');

    //Esta posicion afecta tanto al watermark normal como al forzado de 0.75
    menu_add_item_menu_en_es_ca(array_menu_osd_settings,MENU_OPCION_NORMAL,menu_osd_settings_watermark_position,NULL,
        "Watermark ~~position","~~Posición marca de agua","~~Posició marca d'aigüa");
    menu_add_item_menu_prefijo_format(array_menu_osd_settings,"[%d] ",screen_watermark_position);
    menu_add_item_menu_shortcut(array_menu_osd_settings,'p');
		

    menu_add_item_menu(array_menu_osd_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
    
    menu_add_ESC_item(array_menu_osd_settings);

    retorno_menu=menu_dibuja_menu(&osd_settings_opcion_seleccionada,&item_seleccionado,array_menu_osd_settings,"OSD Settings");

                
    if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                    //printf ("actuamos por funcion\n");
                    item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                    
            }
    }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}




void menu_tool_path(char *tool_path,char *name)
{

    char *filtros[2];

    filtros[0]="";
    filtros[1]=0;

	char buffer_tool_path[PATH_MAX];

    //guardamos directorio actual
    char directorio_actual[PATH_MAX];
    getcwd(directorio_actual,PATH_MAX);

    //Obtenemos directorio de la tool

    char directorio[PATH_MAX];
    util_get_dir(tool_path,directorio);
    debug_printf (VERBOSE_INFO,"Last directory: %s",directorio);

    //cambiamos a ese directorio, siempre que no sea nulo
    if (directorio[0]!=0) {
		debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
		zvfs_chdir(directorio);
    }


    int ret;

    char ventana_titulo[40];
    sprintf (ventana_titulo,"Select %s tool",name);

    ret=menu_filesel(ventana_titulo,filtros,buffer_tool_path);
    //volvemos a directorio inicial
    zvfs_chdir(directorio_actual);


    if (ret==1) {
		sprintf (tool_path,"%s",buffer_tool_path);
    }

}


void menu_external_tool_sox(MENU_ITEM_PARAMETERS)
{
    menu_tool_path(external_tool_sox,"sox");
}

/*void menu_external_tool_unzip(MENU_ITEM_PARAMETERS)
{
	menu_tool_path(external_tool_unzip,"unzip");
}*/

void menu_external_tool_gunzip(MENU_ITEM_PARAMETERS)
{
	menu_tool_path(external_tool_gunzip,"gunzip");
}

void menu_external_tool_tar(MENU_ITEM_PARAMETERS)
{
	menu_tool_path(external_tool_tar,"tar");
}

void menu_external_tool_unrar(MENU_ITEM_PARAMETERS)
{
	menu_tool_path(external_tool_unrar,"unrar");
}



void menu_external_tools_config(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_external_tools_config;
    menu_item item_seleccionado;
    int retorno_menu;


	char string_sox[20];
	//char string_unzip[20];
	char string_gunzip[20];
	char string_tar[20];
	char string_unrar[20];


    do {

		menu_tape_settings_trunc_name(external_tool_sox,string_sox,20);
		//menu_tape_settings_trunc_name(external_tool_unzip,string_unzip,20);
		menu_tape_settings_trunc_name(external_tool_gunzip,string_gunzip,20);
		menu_tape_settings_trunc_name(external_tool_tar,string_tar,20);
		menu_tape_settings_trunc_name(external_tool_unrar,string_unrar,20);

        menu_add_item_menu_inicial_format(&array_menu_external_tools_config,MENU_OPCION_NORMAL,menu_external_tool_sox,NULL,"~~Sox [%s]",string_sox);
		menu_add_item_menu_shortcut(array_menu_external_tools_config,'s');
        menu_add_item_menu_tooltip(array_menu_external_tools_config,"Change Sox Path");
        menu_add_item_menu_ayuda(array_menu_external_tools_config,"Change Sox Path. Path can not include spaces");


        /*menu_add_item_menu_format(array_menu_external_tools_config,MENU_OPCION_NORMAL,menu_external_tool_unzip,NULL,"Un~~zip [%s]",string_unzip);
		menu_add_item_menu_shortcut(array_menu_external_tools_config,'z');
        menu_add_item_menu_tooltip(array_menu_external_tools_config,"Change Unzip Path");
        menu_add_item_menu_ayuda(array_menu_external_tools_config,"Change Unzip Path. Path can not include spaces");*/



        menu_add_item_menu_format(array_menu_external_tools_config,MENU_OPCION_NORMAL,menu_external_tool_gunzip,NULL,"~~Gunzip [%s]",string_gunzip);
		menu_add_item_menu_shortcut(array_menu_external_tools_config,'g');
        menu_add_item_menu_tooltip(array_menu_external_tools_config,"Change Gunzip Path");
        menu_add_item_menu_ayuda(array_menu_external_tools_config,"Change Gunzip Path. Path can not include spaces");



        menu_add_item_menu_format(array_menu_external_tools_config,MENU_OPCION_NORMAL,menu_external_tool_tar,NULL,"~~Tar [%s]",string_tar);
		menu_add_item_menu_shortcut(array_menu_external_tools_config,'t');
        menu_add_item_menu_tooltip(array_menu_external_tools_config,"Change Tar Path");
        menu_add_item_menu_ayuda(array_menu_external_tools_config,"Change Tar Path. Path can not include spaces");


        menu_add_item_menu_format(array_menu_external_tools_config,MENU_OPCION_NORMAL,menu_external_tool_unrar,NULL,"Un~~rar [%s]",string_unrar);
		menu_add_item_menu_shortcut(array_menu_external_tools_config,'r');
        menu_add_item_menu_tooltip(array_menu_external_tools_config,"Change Unrar Path");
        menu_add_item_menu_ayuda(array_menu_external_tools_config,"Change Unrar Path. Path can not include spaces");



        menu_add_item_menu(array_menu_external_tools_config,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        //menu_add_item_menu(array_menu_external_tools_config,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
        menu_add_ESC_item(array_menu_external_tools_config);

        retorno_menu=menu_dibuja_menu(&external_tools_config_opcion_seleccionada,&item_seleccionado,array_menu_external_tools_config,"External tools paths" );

                
        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);  
            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}







void menu_debug_registers_console(MENU_ITEM_PARAMETERS) {
	debug_registers ^=1;
}

void menu_debug_configuration_stepover(MENU_ITEM_PARAMETERS)
{
	//debug_core_evitamos_inter.v ^=1;
	remote_debug_settings ^=32;
}


void menu_breakpoints_condition_behaviour(MENU_ITEM_PARAMETERS)
{
	debug_breakpoints_cond_behaviour.v ^=1;
}

void menu_debug_configuration_remoteproto_port(MENU_ITEM_PARAMETERS)
{
	char string_port[6];
	int port;

	sprintf (string_port,"%d",remote_protocol_port);

	menu_ventana_scanf("Port",string_port,6);

	if (string_port[0]==0) return;

	else {
			port=parse_string_to_number(string_port);

			if (port<1 || port>65535) {
								debug_printf (VERBOSE_ERR,"Invalid port %d",port);
								return;
			}


			end_remote_protocol();
			remote_protocol_port=port;
			init_remote_protocol();
	}

}

void menu_debug_configuration_remoteproto_prompt(MENU_ITEM_PARAMETERS)
{
    menu_ventana_scanf("ZRCP prompt",remote_prompt_command_string,REMOTE_MAX_PROMPT_LENGTH);

    //Si se deja en blanco, restaurar por defecto
    if (remote_prompt_command_string[0]==0) strcpy(remote_prompt_command_string,"command");
}

void menu_debug_shows_invalid_opcode(MENU_ITEM_PARAMETERS)
{
	debug_shows_invalid_opcode.v ^=1;
}

void menu_debug_settings_show_fired_breakpoint(MENU_ITEM_PARAMETERS)
{
	debug_show_fired_breakpoints_type++;
	if (debug_show_fired_breakpoints_type==3) debug_show_fired_breakpoints_type=0;
}

void menu_debug_settings_show_screen(MENU_ITEM_PARAMETERS)
{
	debug_settings_show_screen.v ^=1;
}
void menu_debug_settings_show_scanline(MENU_ITEM_PARAMETERS)
{
	menu_debug_registers_if_showscan.v ^=1;
}

void menu_debug_configuration_remoteproto(MENU_ITEM_PARAMETERS)
{
	if (remote_protocol_enabled.v) {
		end_remote_protocol();
		remote_protocol_enabled.v=0;
	}

	else {
		enable_and_init_remote_protocol();
	}
}


void menu_debug_verbose(MENU_ITEM_PARAMETERS)
{
	verbose_level++;
	if (verbose_level>4) verbose_level=0;
}

void menu_zesarux_zxi_hardware_debug_file(MENU_ITEM_PARAMETERS)
{

	char *filtros[2];

    filtros[0]="";
    filtros[1]=0;


    if (menu_filesel("Select Debug File",filtros,zesarux_zxi_hardware_debug_file)==1) {
    	//Ver si archivo existe y preguntar
		if (si_existe_archivo(zesarux_zxi_hardware_debug_file)) {
            if (menu_confirm_yesno_texto("File exists","Append?")==0) {
				zesarux_zxi_hardware_debug_file[0]=0;
				return;
			}
        }

    }

	else zesarux_zxi_hardware_debug_file[0]=0;

}

void menu_hardware_debug_port(MENU_ITEM_PARAMETERS)
{
	hardware_debug_port.v ^=1;
}


void menu_debug_settings_dump_snap_panic(MENU_ITEM_PARAMETERS)
{
	debug_dump_zsf_on_cpu_panic.v ^=1;
}

void menu_debug_verbose_always_console(MENU_ITEM_PARAMETERS)
{
	debug_always_show_messages_in_console.v ^=1;
}

void menu_debug_settings_visualmem_grafico(MENU_ITEM_PARAMETERS)
{
	setting_mostrar_visualmem_grafico.v ^=1;
}

void menu_debug_unnamed_console_enable(MENU_ITEM_PARAMETERS)
{

    if (debug_unnamed_console_enabled.v) {
        debug_unnamed_console_end();
        debug_unnamed_console_enabled.v=0;
    }

    else {
        debug_unnamed_console_enabled.v=1;
        debug_unnamed_console_init();
    }

}

void menu_debug_settings_sourcecode_lprefix(MENU_ITEM_PARAMETERS)
{
    remote_debug_settings ^=4;    
}

void menu_debug_settings_sourcecode_skipcols(MENU_ITEM_PARAMETERS)
{
    menu_ventana_scanf_numero_enhanced("Skip Columns",&debug_load_source_code_skip_columns,3,+1,0,99,0);
}

void menu_debug_settings_max_history(MENU_ITEM_PARAMETERS)
{

    int max_items=cpu_history_get_max_size();

    int ret=menu_ventana_scanf_numero_enhanced("Maximum items",&max_items,9,+100000,1,CPU_HISTORY_MAX_ALLOWED_ELEMENTS,0);

    //Si pulsado ESC no cambiar nada
    if (ret<0) {
        //printf("Pulsado ESC\n");
        return;
    }

    //si no esta habilitado, solo cambiamos el valor sin rehacer el history
    if (cpu_history_enabled.v==0) {
        cpu_history_max_elements=max_items;
    }

    else {
        cpu_history_set_max_size(max_items);
    }
}

void menu_debug_settings_show_address_basic(MENU_ITEM_PARAMETERS)
{
    debug_view_basic_show_address.v ^=1;
}

void menu_debug_settings_show_fired_halt(MENU_ITEM_PARAMETERS)
{
    debug_settings_show_fired_halt.v ^=1;
}

void menu_debug_verbose_excludeinclude(MENU_ITEM_PARAMETERS)
{
    if (debug_mascara_modo_exclude_include==VERBOSE_MASK_CLASS_TYPE_EXCLUDE) {
        debug_mascara_modo_exclude_include=VERBOSE_MASK_CLASS_TYPE_INCLUDE;
    }
    else {
        debug_mascara_modo_exclude_include=VERBOSE_MASK_CLASS_TYPE_EXCLUDE;
    }
    
}

void menu_debug_verbose_filter_item(MENU_ITEM_PARAMETERS)
{
    //Obtener bit de mascara que vamos a cambiar
    int class_mask=debug_get_class_mask_value(valor_opcion);

    //Conmutar valor
    if (debug_mascara_modo_exclude_include==VERBOSE_MASK_CLASS_TYPE_EXCLUDE) {
        debug_mascara_clase_exclude ^= class_mask;
    }
    else {
        debug_mascara_clase_include ^= class_mask;
    }
}

void menu_debug_verbose_filter(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;


    do {

        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

        int valor_mascara_config=(debug_mascara_modo_exclude_include==VERBOSE_MASK_CLASS_TYPE_EXCLUDE ? debug_mascara_clase_exclude : debug_mascara_clase_include);


        int total_masks=debug_get_total_class_masks();
        int i;
        for (i=0;i<total_masks;i++) {
            int class_mask=debug_get_class_mask_value(i);

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_verbose_filter_item,NULL,"[%c] %s",
                (valor_mascara_config & class_mask ? 'X' : ' '),
                debug_get_class_mask_name(i)
            );

            menu_add_item_menu_valor_opcion(array_menu_common,i);
        }

        
        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&debug_verbose_filter_opcion_seleccionada,&item_seleccionado,array_menu_common,"Filter mask");



        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}    
    

//menu debug settings
void menu_settings_debug(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_settings_debug;
    menu_item item_seleccionado;
    int retorno_menu;
    do {


        char string_zesarux_zxi_hardware_debug_file_shown[18];
      
        menu_add_item_menu_inicial(&array_menu_settings_debug,"",MENU_OPCION_UNASSIGNED,NULL,NULL);


		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_sourcecode_lprefix,NULL,
            "Source code L Prefix","Código fuente prefijo L","Codi font prefix L");
		menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",( remote_debug_settings & 4 ? ' ' : 'X') );
        menu_add_item_menu_tooltip(array_menu_settings_debug,"Consider a L preffix when searching source code labels");
        menu_add_item_menu_ayuda(array_menu_settings_debug,"Consider a L preffix when searching source code labels");

		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_sourcecode_skipcols,NULL,
            "Source code skip Cols","Código fuente saltar cols","Codi font saltar cols");
		menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%d] ",debug_load_source_code_skip_columns);
        menu_add_item_menu_tooltip(array_menu_settings_debug,"Skip columns when searching for label from the beginning of line");
        menu_add_item_menu_ayuda(array_menu_settings_debug,"Skip columns when searching for label from the beginning of line");   

		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_configuration_stepover,NULL,
            "Step ~~over interrupt","Pas~~o a paso salta interrupción","Pas a pas salta interrupci~~ó");
        menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",(remote_debug_settings&32 ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Avoid step to step or continuous execution of nmi or maskable interrupt routines on debug cpu menu");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Avoid step to step or continuous execution of nmi or maskable interrupt routines on debug cpu menu");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'o');         

		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_screen,NULL,
            "Show display on debug","Ver pantalla al debugar","Veure pantalla al debugar");
		menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",( debug_settings_show_screen.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"If shows emulated screen on every key action on debug registers menu");	
		menu_add_item_menu_ayuda(array_menu_settings_debug,"If shows emulated screen on every key action on debug registers menu");	        

		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_scanline,NULL,
            "Show electron on debug","Ver electrón al debugar","Veure electró al debugar");
		menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",( menu_debug_registers_if_showscan.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Shows TV electron position when debugging, using a coloured line. Requires real video");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Shows TV electron position when debugging, using a coloured line. Requires real video");

        
        if (MACHINE_IS_SPECTRUM) {
            menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_fired_halt,NULL,
                "Show on border fired Halt","Mostrar en border Halt ejecutado","Mostrar al border Halt executat");
            menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",( debug_settings_show_fired_halt.v ? 'X' : ' ') );
            menu_add_item_menu_tooltip(array_menu_settings_debug,"Shows on which scanline has been executed a Halt, inverting border color. Requires real video");
            menu_add_item_menu_ayuda(array_menu_settings_debug,"Shows on which scanline has been executed a Halt, inverting border color. Requires real video");
        }


		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_breakpoints_condition_behaviour,NULL,
            "~~Breakp. behaviour","Comportamiento ~~Breakp.","Comportament ~~Breakp.");
        menu_add_item_menu_sufijo_format(array_menu_settings_debug," [%s]",(debug_breakpoints_cond_behaviour.v ? "On Change" : "Always") );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Indicates whether breakpoints are fired always or only on change from false to true");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Indicates whether breakpoints are fired always or only on change from false to true");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'b');


		char show_fired_breakpoint_type[30];
		if (debug_show_fired_breakpoints_type==0) strcpy(show_fired_breakpoint_type,"Always");
		else if (debug_show_fired_breakpoints_type==1) strcpy(show_fired_breakpoint_type,"NoPC");
		else strcpy(show_fired_breakpoint_type,"Never");																	//						   OnlyNonPC
																															//  01234567890123456789012345678901
		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_fired_breakpoint,NULL,
            "Show fired breakpoint","Ver breakpoint disparado","Veure breakpoint disparat");
        menu_add_item_menu_sufijo_format(array_menu_settings_debug," [%s]",show_fired_breakpoint_type);
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Tells to show the breakpoint condition when it is fired");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Tells to show the breakpoint condition when it is fired. "
								"Possible values:\n"
								"Always: always shows the condition\n"
								"NoPC: only shows conditions that are not like PC=XXXX\n"
								"Never: never shows conditions\n" );	


        menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_address_basic,NULL,
            "Show address on View Basic","Ver dirección en Ver Basic","Veure adreça a Veure Basic");
		menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",( debug_view_basic_show_address.v ? 'X' : ' ') );

		menu_add_item_menu_tooltip(array_menu_settings_debug,"Shows location address of every basic line on menu View Basic");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Shows location address of every basic line on menu View Basic");



		if (si_complete_video_driver() ) {
			menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_settings_visualmem_grafico,NULL,
                "    Show Visualmem","    Ver Memoria Visual","    Veure Memoria Visual");
			menu_add_item_menu_sufijo_format(array_menu_settings_debug," [%s]",(setting_mostrar_visualmem_grafico.v ? "Graphic" : "Text") );


			menu_add_item_menu_tooltip(array_menu_settings_debug,"Shows Visualmem menu with graphic or with text");
			menu_add_item_menu_ayuda(array_menu_settings_debug,"Shows Visualmem menu with graphic or with text");

		}

        menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_verbose,NULL,
            "Verbose ~~level","Nive~~l Verbose","Nive~~ll Verbose");
        menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%d] ",verbose_level);
		menu_add_item_menu_shortcut(array_menu_settings_debug,'l');	
        menu_add_item_menu_tooltip(array_menu_settings_debug,"Verbose level for debug messages. Usually shown on terminal console or on debug console window");
        menu_add_item_menu_ayuda(array_menu_settings_debug,"Verbose level for debug messages. Usually shown on terminal console or on debug console window");

		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_verbose_excludeinclude,NULL,
            "Message filter","Filtro mensajes","Filtre missatges");
        menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%s] ",
            (debug_mascara_modo_exclude_include==VERBOSE_MASK_CLASS_TYPE_EXCLUDE ? "Exclude" : "Include") );
		menu_add_item_menu_shortcut(array_menu_settings_debug,'l');	
        menu_add_item_menu_tooltip(array_menu_settings_debug,"Filter type for debug messages");
        menu_add_item_menu_ayuda(array_menu_settings_debug,"Filter type for debug messages");

        int valor_mascara=(debug_mascara_modo_exclude_include==VERBOSE_MASK_CLASS_TYPE_EXCLUDE ? debug_mascara_clase_exclude : debug_mascara_clase_include);

		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_verbose_filter,NULL,
            "Filter mask","Máscara filtro","Màscara filtre");
        menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%08X] ",valor_mascara);
		menu_add_item_menu_shortcut(array_menu_settings_debug,'l');	
        menu_add_item_menu_tooltip(array_menu_settings_debug,"Filter mask for debug messages");
        menu_add_item_menu_ayuda(array_menu_settings_debug,"Filter mask for debug messages");



		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_unnamed_console_enable,NULL,
            "Debug console window","Ventana de consola depuración","Finestra de consola depuració");
		menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",( debug_unnamed_console_enabled.v ? 'X' : ' ') );    
        menu_add_item_menu_tooltip(array_menu_settings_debug,"Enables debug console window, it will be visible on Debug->Debug console menu");
        menu_add_item_menu_ayuda(array_menu_settings_debug,"Enables debug console window, it will be visible on Debug->Debug console menu. "
            "There it shows the same messages as the ones shown on terminal console");

		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_verbose_always_console,NULL,
            "Always debug in terminal","Siempre debug a terminal","Sempre debug a terminal");
		menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",( debug_always_show_messages_in_console.v ? 'X' : ' ') );

		menu_add_item_menu_tooltip(array_menu_settings_debug,"Always show messages in terminal console (using simple printf) additionally to the default video driver");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Always show messages in terminal console (using simple printf) additionally to the default video driver. Interesting in some cases as curses, aa or caca video drivers");
				


		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_dump_snap_panic,NULL,
            "Dump snapshot on panic","Volcar snapshot cuando panic","Volcar snapshot quan panic");
		menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",( debug_dump_zsf_on_cpu_panic.v ? 'X' : ' ') );	
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Dump .zsf snapshot when a cpu panic is fired");	
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Dump .zsf snapshot when a cpu panic is fired");	

		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_registers_console,NULL,
            "View r~~egisters in terminal","Ver r~~egistros en terminal","Veure r~~egistres a la terminal");
        menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",(debug_registers==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_debug,'e');

		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_shows_invalid_opcode,NULL,
            "Show ~~invalid opcode","Mostrar opcode ~~inválido","Mostrar opcode ~~invàlid");
		menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",(debug_shows_invalid_opcode.v ? 'X' : ' ') ); 
		menu_add_item_menu_shortcut(array_menu_settings_debug,'i');
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Show which opcodes are invalid (considering ED, DD, FD prefixes)");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Show which opcodes are invalid (considering ED, DD, FD prefixes). "
								"A message will be shown on console, when verbose level is 2 or higher");



        char ayuda_leyenda[32*10]; // para 10 lineas de ayuda, mas que suficiente
        sprintf(ayuda_leyenda,"Maximum items allowed on cpu history feature. Each item uses %d bytes of memory",CPU_HISTORY_REGISTERS_SIZE);

        menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_max_history,NULL,
            "Max history items","Max items en historial","Max items a l'historial");
        menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%d] ",cpu_history_get_max_size() );
        menu_add_item_menu_tooltip(array_menu_settings_debug,ayuda_leyenda);
        menu_add_item_menu_ayuda(array_menu_settings_debug,ayuda_leyenda);


		menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);		


#ifndef NETWORKING_DISABLED
		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_configuration_remoteproto,NULL,
            "ZRCP Remote protocol","ZRCP protocolo Remoto","ZRCP protocol Remot");
        menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",(remote_protocol_enabled.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Enables or disables ZEsarUX remote command protocol (ZRCP)");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Enables or disables ZEsarUX remote command protocol (ZRCP)");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'r');

		if (remote_protocol_enabled.v) {
			menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_configuration_remoteproto_port,NULL,
                "ZRCP ~~port","~~Puerto ZRCP","~~Port ZRCP");
            menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%d] ",remote_protocol_port );
			menu_add_item_menu_tooltip(array_menu_settings_debug,"Changes remote command protocol port");
			menu_add_item_menu_ayuda(array_menu_settings_debug,"Changes remote command protocol port");
			menu_add_item_menu_shortcut(array_menu_settings_debug,'p');

            char string_prompt[20];
            menu_tape_settings_trunc_name(remote_prompt_command_string,string_prompt,20);
			menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_configuration_remoteproto_prompt,NULL,
                "ZRCP pro~~mpt","ZRCP pro~~mpt","ZRCP pro~~mpt");
            menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%s] ",string_prompt );
			menu_add_item_menu_tooltip(array_menu_settings_debug,"Changes remote command protocol prompt");
			menu_add_item_menu_ayuda(array_menu_settings_debug,"Changes remote command protocol prompt");
			menu_add_item_menu_shortcut(array_menu_settings_debug,'m');            
		}

#endif


		menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_hardware_debug_port,NULL,
            "Hardware ~~debug ports","Puertos ~~debug hardware","Ports ~~debug hardware");
        menu_add_item_menu_prefijo_format(array_menu_settings_debug,"[%c] ",(hardware_debug_port.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"If hardware debug ports are enabled");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"These ports are used to interact with ZEsarUX, for example showing a ASCII character on console, read ZEsarUX version, etc. "
														"Read file extras/docs/zesarux_zxi_registers.txt for more information");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'d');


		if (hardware_debug_port.v) {
			menu_tape_settings_trunc_name(zesarux_zxi_hardware_debug_file,string_zesarux_zxi_hardware_debug_file_shown,18);
        	menu_add_item_menu_en_es_ca(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_zesarux_zxi_hardware_debug_file,NULL,
                "Byte ~~file","~~Fichero byte","~~Fitxer byte");
            menu_add_item_menu_sufijo_format(array_menu_settings_debug," [%s]",string_zesarux_zxi_hardware_debug_file_shown);
			menu_add_item_menu_tooltip(array_menu_settings_debug,"File used on using register 6 (HARDWARE_DEBUG_BYTE_FILE)");		
			menu_add_item_menu_ayuda(array_menu_settings_debug,"File used on using register 6 (HARDWARE_DEBUG_BYTE_FILE)");	
			menu_add_item_menu_shortcut(array_menu_settings_debug,'f');							
		}


		
        menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        //menu_add_item_menu(array_menu_settings_debug,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings_debug);

        retorno_menu=menu_dibuja_menu(&settings_debug_opcion_seleccionada,&item_seleccionado,array_menu_settings_debug,"Debug Settings" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}


int num_menu_audio_driver;
int num_previo_menu_audio_driver;


//Determina cual es el audio driver actual
void menu_change_audio_driver_get(void)
{
        int i;
        for (i=0;i<num_audio_driver_array;i++) {
		//printf ("actual: %s buscado: %s indice: %d\n",audio_new_driver_name,audio_driver_array[i].driver_name,i);
                if (!strcmp(audio_new_driver_name,audio_driver_array[i].driver_name)) {
                        num_menu_audio_driver=i;
                        num_previo_menu_audio_driver=i;
			return;
                }

        }

}


void menu_change_audio_driver_change(MENU_ITEM_PARAMETERS)
{
        num_menu_audio_driver++;
        if (num_menu_audio_driver==num_audio_driver_array) num_menu_audio_driver=0;
}

void menu_change_audio_driver_apply(MENU_ITEM_PARAMETERS)
{

	audio_end();

        int (*funcion_init) ();
        int (*funcion_set) ();

        funcion_init=audio_driver_array[num_menu_audio_driver].funcion_init;
        funcion_set=audio_driver_array[num_menu_audio_driver].funcion_set;
                if ( (funcion_init()) ==0) {
                        funcion_set();
			menu_generic_message_splash("Apply Driver","OK. Driver applied");
			salir_todos_menus=1;
                }

                else {
                        debug_printf(VERBOSE_ERR,"Can not set audio driver. Restoring to previous driver %s",audio_new_driver_name);
			menu_change_audio_driver_get();

                        //Restaurar audio driver
                        funcion_init=audio_driver_array[num_previo_menu_audio_driver].funcion_init;
                        funcion_set=audio_driver_array[num_previo_menu_audio_driver].funcion_set;

                        funcion_init();
                        funcion_set();
                }



}


void menu_change_audio_driver(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_change_audio_driver;
        menu_item item_seleccionado;
        int retorno_menu;

       	menu_change_audio_driver_get();

        do {

                menu_add_item_menu_inicial_format(&array_menu_change_audio_driver,MENU_OPCION_NORMAL,menu_change_audio_driver_change,NULL,"Audio Driver: %s",audio_driver_array[num_menu_audio_driver].driver_name );

                menu_add_item_menu_format(array_menu_change_audio_driver,MENU_OPCION_NORMAL,menu_change_audio_driver_apply,NULL,"Apply Driver" );

                menu_add_item_menu(array_menu_change_audio_driver,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_change_audio_driver,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_change_audio_driver);

                retorno_menu=menu_dibuja_menu(&change_audio_driver_opcion_seleccionada,&item_seleccionado,array_menu_change_audio_driver,"Change Audio Driver" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}






int menu_cond_ay_chip(void)
{
	return ay_chip_present.v;
}

int menu_cond_i8049_chip(void)
{
	return i8049_chip_present;
}



void menu_audio_beep_filter_on_rom_save(MENU_ITEM_PARAMETERS)
{
	output_beep_filter_on_rom_save.v ^=1;
}


void menu_audio_beep_alter_volume(MENU_ITEM_PARAMETERS)
{
	output_beep_filter_alter_volume.v ^=1;
}


void menu_audio_beep_volume(MENU_ITEM_PARAMETERS)
{

        char string_vol[4];

        sprintf (string_vol,"%d",output_beep_filter_volume);


        menu_ventana_scanf("Volume (0-127)",string_vol,4);

        int v=parse_string_to_number(string_vol);

        if (v>127 || v<0) {
                debug_printf (VERBOSE_ERR,"Invalid volume value");
                return;
        }

        output_beep_filter_volume=v;
}

void menu_audio_beeper_real (MENU_ITEM_PARAMETERS)
{
	beeper_real_enabled ^=1;
}

void menu_audio_volume(MENU_ITEM_PARAMETERS)
{
	menu_ventana_scanf_numero_enhanced("Volume in %",&audiovolume,4,+20,0,100,0);

/*
        char string_perc[4];

        sprintf (string_perc,"%d",audiovolume);


        //menu_ventana_scanf("Volume in %",string_perc,4);
		int retorno=menu_ventana_scanf_numero("Volume in %",string_perc,4,+20,0,100,0);

		if (retorno<0) return;

        int v=parse_string_to_number(string_perc);

	if (v>100 || v<0) {
		debug_printf (VERBOSE_ERR,"Invalid volume value");
		return;
	}

	audiovolume=v;
*/
}

void menu_audio_ay_chip(MENU_ITEM_PARAMETERS)
{
	ay_chip_present.v^=1;
}

void menu_audio_ay_chip_autoenable(MENU_ITEM_PARAMETERS)
{
	autoenable_ay_chip.v^=1;
}



void menu_audio_sound_zx8081(MENU_ITEM_PARAMETERS)
{
	zx8081_vsync_sound.v^=1;
}

void menu_audio_zx8081_detect_vsync_sound(MENU_ITEM_PARAMETERS)
{
	zx8081_detect_vsync_sound.v ^=1;
}



void menu_setting_ay_piano_grafico(MENU_ITEM_PARAMETERS)
{
	setting_mostrar_ay_piano_grafico.v ^=1;
}


void menu_aofile_insert(MENU_ITEM_PARAMETERS)
{

	if (aofile_inserted.v==0) {
		init_aofile();

		//Si todo ha ido bien
		if (aofile_inserted.v) {
			menu_generic_message_format("File information","%s\n%s\n\n%s",
			last_message_helper_aofile_vofile_file_format,last_message_helper_aofile_vofile_bytes_minute_audio,last_message_helper_aofile_vofile_util);
		}

	}

        else if (aofile_inserted.v==1) {
                close_aofile();
        }

}

int menu_aofile_cond(void)
{
	if (aofilename!=NULL) return 1;
	else return 0;
}

void menu_aofile(MENU_ITEM_PARAMETERS)
{

	aofile_inserted.v=0;


        char *filtros[3];

#ifdef USE_SNDFILE
        filtros[0]="rwa";
        filtros[1]="wav";
        filtros[2]=0;
#else
        filtros[0]="rwa";
        filtros[1]=0;
#endif


        if (menu_filesel("Select Audio File",filtros,aofilename_file)==1) {

       	        if (si_existe_archivo(aofilename_file)) {

               	        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) {
				aofilename=NULL;
				return;
			}

       	        }

                aofilename=aofilename_file;


        }

	else {
		aofilename=NULL;
	}


}




/*void menu_audio_audiodac(MENU_ITEM_PARAMETERS)
{
	audiodac_enabled.v ^=1;
}*/

void menu_audio_audiodac_type(MENU_ITEM_PARAMETERS)
{
	if (audiodac_enabled.v==0) {
		audiodac_enabled.v=1;
		audiodac_selected_type=0;
	}

	else {
		audiodac_selected_type++;
		if (audiodac_selected_type==MAX_AUDIODAC_TYPES) {
			audiodac_selected_type=0;
			audiodac_enabled.v=0;
		}
	}
}

void menu_audio_audiodac_set_port(MENU_ITEM_PARAMETERS)
{
	char string_port[4];

	sprintf (string_port,"%02XH",audiodac_types[MAX_AUDIODAC_TYPES-1].port);

	menu_ventana_scanf("Port Value",string_port,4);

	int valor_port=parse_string_to_number(string_port);

	if (valor_port<0 || valor_port>255) {
					debug_printf (VERBOSE_ERR,"Invalid value %d",valor_port);
					return;
	}

	audiodac_set_custom_port(valor_port);
	//audiodac_types[MAX_AUDIODAC_TYPES-1].port=valor_port;
	//audiodac_selected_type=MAX_AUDIODAC_TYPES-1;

}

void menu_audio_beeper(MENU_ITEM_PARAMETERS)
{
	beeper_enabled.v ^=1;
}

void menu_audio_change_ay_chips(MENU_ITEM_PARAMETERS)
{
	if (total_ay_chips==MAX_AY_CHIPS) total_ay_chips=1;
	else total_ay_chips++;

	ay_chip_selected=0;
}


/*
void menu_audio_ay_stereo_custom(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_A++;
	if (ay3_custom_stereo_A==3) {
		ay3_custom_stereo_A=2;

		ay3_custom_stereo_B++;
		if (ay3_custom_stereo_B==3) {
			ay3_custom_stereo_B=2;

			ay3_custom_stereo_C++;
			if (ay3_custom_stereo_C==3) {
				ay3_custom_stereo_A=0;
				ay3_custom_stereo_B=0;
				ay3_custom_stereo_C=0;
			}
		}
	}	
}
*/



void menu_silence_detector(MENU_ITEM_PARAMETERS)
{
	silence_detector_setting.v ^=1;
}

void menu_audio_resample_1bit(MENU_ITEM_PARAMETERS)
{
	audio_resample_1bit.v ^=1;
}






void menu_onebitspeaker_intensive_cpu(MENU_ITEM_PARAMETERS)
{

    audioonebitspeaker_intensive_cpu_usage ^=1;

}

void menu_onebitspeaker_agudo_filtro(MENU_ITEM_PARAMETERS)
{
    audioonebitspeaker_agudo_filtro ^=1;
}

void menu_onebitspeaker_agudo_filtro_limite(MENU_ITEM_PARAMETERS)
{
    audioonebitspeaker_agudo_filtro_limite--;

    if (audioonebitspeaker_agudo_filtro_limite==0) audioonebitspeaker_agudo_filtro_limite=15;
}

void menu_onebitspeaker_tipo_speaker(MENU_ITEM_PARAMETERS)
{

    audio_end();

    if (audioonebitspeaker_tipo_altavoz==TIPO_ALTAVOZ_ONEBITSPEAKER_PCSPEAKER) {
        audioonebitspeaker_tipo_altavoz=TIPO_ALTAVOZ_ONEBITSPEAKER_RPI_GPIO;
    }
    else {
        audioonebitspeaker_tipo_altavoz=TIPO_ALTAVOZ_ONEBITSPEAKER_PCSPEAKER;
    }

    audio_init();
}



void menu_onebitspeaker_gpio_pin(MENU_ITEM_PARAMETERS)
{

    int valor=audioonebitspeaker_rpi_gpio_pin;

    menu_ventana_scanf_numero_enhanced("Raspberry GPIO Pin number",&valor,3,+1,0,99,0);

    audio_end();

    audioonebitspeaker_rpi_gpio_pin=valor;

    audio_init();
}

void menu_audio_i8049_chip_present(MENU_ITEM_PARAMETERS)
{
    i8049_chip_present ^= 1;
}

void menu_audio_general_sound_enable(MENU_ITEM_PARAMETERS)
{
    if (gs_enabled.v) gs_disable();
    else gs_enable();
}

void menu_audio_general_sound_mem(MENU_ITEM_PARAMETERS)
{
    /*
//3=128 kb
//7=256 kb
//15=512 kb
//31=1024 kb
z80_byte gs_memory_mapping_mask_pages=15;    
    */
   
   //Desactivar y activar GS para que note el cambio de paginas
   gs_disable();
   

   gs_memory_mapping_mask_pages *=2;

   gs_memory_mapping_mask_pages |=1;

   if (gs_memory_mapping_mask_pages>31) gs_memory_mapping_mask_pages=3;

   //printf("mask page: %d\n",gs_memory_mapping_mask_pages);

   gs_enable();   

}

void menu_audiosdl_callback_type(MENU_ITEM_PARAMETERS)
{
    audiosdl_use_new_callback.v ^=1;
}

void menu_settings_audio(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_audio;
	menu_item item_seleccionado;
	int retorno_menu;

        do {

		//hotkeys usadas: vuacpdrbfoilh

		menu_add_item_menu_inicial_format(&array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_volume,NULL,"    Output ~~Volume");
        menu_add_item_menu_spanish_catalan(array_menu_settings_audio,"    Volumen de salida","    Volum de sortida");
        menu_add_item_menu_sufijo_format(array_menu_settings_audio," [%d%%]", audiovolume);
		menu_add_item_menu_shortcut(array_menu_settings_audio,'v');

        if (!MACHINE_IS_QL && sn_chip_present.v==0) {
            menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_chip_autoenable,NULL,"A~~utoenable AY Chip");
            menu_add_item_menu_spanish_catalan(array_menu_settings_audio,"A~~utohabilitar Chip AY","A~~utohabilitar Xip AY");
            menu_add_item_menu_prefijo_format(array_menu_settings_audio,"[%c] ",(autoenable_ay_chip.v==1 ? 'X' : ' '));
            menu_add_item_menu_shortcut(array_menu_settings_audio,'u');
            menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable AY Chip automatically when it is needed");
            menu_add_item_menu_ayuda(array_menu_settings_audio,"This option is usefor for example on Spectrum 48k games that uses AY Chip "
                        "and for some ZX80/81 games that also uses it (Bi-Pak ZON-X81, but not Quicksilva QS Sound board)");		

            menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_chip,NULL,"~~AY Chip");
            menu_add_item_menu_spanish_catalan(array_menu_settings_audio,"Chip ~~AY","Xip ~~AY");
            menu_add_item_menu_prefijo_format(array_menu_settings_audio,"[%c] ", (ay_chip_present.v==1 ? 'X' : ' '));
            menu_add_item_menu_shortcut(array_menu_settings_audio,'a');
            menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable AY Chip on this machine");
            menu_add_item_menu_ayuda(array_menu_settings_audio,"It enables the AY Chip for the machine, by activating the following hardware:\n"
                        "-Normal AY Chip for Spectrum\n"
                        "-Fuller audio box for Spectrum\n"
                        "-Quicksilva QS Sound board on ZX80/81\n"
                        "-Bi-Pak ZON-X81 Sound on ZX80/81\n"
                );




			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_change_ay_chips,menu_cond_ay_chip,"[%d] AY ~~Chips %s",total_ay_chips,
				(total_ay_chips>1 ? "(Turbosound)" : "") );
			menu_add_item_menu_shortcut(array_menu_settings_audio,'c');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Total number of AY Chips");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Total number of AY Chips");
            menu_add_item_menu_es_avanzado(array_menu_settings_audio);

        }

            else if (MACHINE_IS_QL) {
                menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_i8049_chip_present,NULL,"[%c] i8049 sound chip", (i8049_chip_present ? 'X' : ' '));
            }




		if (si_complete_video_driver() ) {
			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_setting_ay_piano_grafico,NULL,"    ~~Piano Type");
            menu_add_item_menu_spanish_catalan(array_menu_settings_audio,"    Tipo ~~Piano","    Tipus ~~Piano");
			menu_add_item_menu_sufijo_format(array_menu_settings_audio," [%s]",(setting_mostrar_ay_piano_grafico.v ? "Graphic" : "Text") );
			menu_add_item_menu_shortcut(array_menu_settings_audio,'p');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Shows AY/Beeper Piano menu with graphic or with text");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Shows AY/Beeper Piano menu with graphic or with text");
            menu_add_item_menu_es_avanzado(array_menu_settings_audio);

		}


        if (MACHINE_IS_SPECTRUM) {
            menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_general_sound_enable,NULL,"[%c] General Sound", (gs_enabled.v ? 'X' : ' '));
            menu_add_item_menu_es_avanzado(array_menu_settings_audio);

            if (gs_enabled.v) {
                menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_general_sound_mem,NULL,"[%4d KB] General Sound RAM",
                (gs_memory_mapping_mask_pages+1)*32 );
                menu_add_item_menu_es_avanzado(array_menu_settings_audio);
            }
        }


		



		if (MACHINE_IS_SPECTRUM) {

			menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);
            menu_add_item_menu_es_avanzado(array_menu_settings_audio);

			char string_audiodac[32];

				if (audiodac_enabled.v) {
					sprintf (string_audiodac,": %s",audiodac_types[audiodac_selected_type].name);
				}
				else {
					strcpy(string_audiodac,"");
				}

				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_audiodac_type,NULL,"[%c] ~~DAC%s",(audiodac_enabled.v ? 'X' : ' ' ),
						string_audiodac);
				menu_add_item_menu_shortcut(array_menu_settings_audio,'d');
                menu_add_item_menu_es_avanzado(array_menu_settings_audio);

				if (audiodac_enabled.v) {
					menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_audiodac_set_port,NULL,"[%02XH] DAC port",audiodac_types[audiodac_selected_type].port);
                    menu_add_item_menu_es_avanzado(array_menu_settings_audio);
				}



		}


    menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		if (!MACHINE_IS_ZX8081 && !MACHINE_IS_QL) {

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beeper,NULL,"[%c] Beepe~~r",(beeper_enabled.v==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_audio,'r');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable beeper output");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Enable or disable beeper output");

		}



		if (MACHINE_IS_ZX8081) {
			//sound on zx80/81

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_zx8081_detect_vsync_sound,menu_cond_zx8081,"[%c] Detect VSYNC Sound",(zx8081_detect_vsync_sound.v ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Tries to detect when vsync sound is played. This feature is experimental");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Tries to detect when vsync sound is played. This feature is experimental");


			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_sound_zx8081,menu_cond_zx8081,"[%c] VSYNC Sound on zx80/81", (zx8081_vsync_sound.v==1 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enables or disables VSYNC sound on ZX80 and ZX81");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"This method uses the VSYNC signal on the TV to make sound");


		}




		int mostrar_real_beeper=0;

		if (MACHINE_IS_ZX8081) {
			if (zx8081_vsync_sound.v) mostrar_real_beeper=1;
		}

		else {
			if (beeper_enabled.v) mostrar_real_beeper=1;
		}

		if (mostrar_real_beeper && !MACHINE_IS_QL) {

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beeper_real,NULL,"[%c] Real ~~Beeper",(beeper_real_enabled==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_audio,'b');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable Real Beeper enhanced sound. ");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Real beeper produces beeper sound more realistic but uses a bit more cpu. Needs beeper enabled (or vsync sound on zx80/81)");
		}

		menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_filter_on_rom_save,NULL,"ROM SAVE filter");
            menu_add_item_menu_spanish_format(array_menu_settings_audio,"Filtro SAVE en ROM");
            menu_add_item_menu_catalan_format(array_menu_settings_audio,"Filtre SAVE a ROM");
            menu_add_item_menu_prefijo_format(array_menu_settings_audio,"[%c] ",(output_beep_filter_on_rom_save.v ? 'X' : ' '));            
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Apply filter on ROM save routines");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"It detects when on ROM save routines and alter audio output to use only "
					"the MIC bit of the FEH port");
            menu_add_item_menu_es_avanzado(array_menu_settings_audio);

//extern z80_bit output_beep_filter_alter_volume;
//extern char output_beep_filter_volume;

			if (output_beep_filter_on_rom_save.v) {
				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_alter_volume,NULL,"[%c] Alter beeper volume",
				(output_beep_filter_alter_volume.v ? 'X' : ' ') );

				menu_add_item_menu_tooltip(array_menu_settings_audio,"Alter output beeper volume");
				menu_add_item_menu_ayuda(array_menu_settings_audio,"Alter output beeper volume. You can set to a maximum to "
							"send the audio to a real spectrum to load it");
                menu_add_item_menu_es_avanzado(array_menu_settings_audio);


				if (output_beep_filter_alter_volume.v) {
					menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_volume,NULL,"[%d] Beeper volume",output_beep_filter_volume);
                    menu_add_item_menu_es_avanzado(array_menu_settings_audio);
				}
			}

		}


		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_resample_1bit,NULL,"[%c] 1 bit filter",(audio_resample_1bit.v ? 'X' : ' '));
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Resample audio output to 1 bit only");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"Resample audio output to 1 bit only");
        menu_add_item_menu_es_avanzado(array_menu_settings_audio);
		
	
		menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        menu_add_item_menu_es_avanzado(array_menu_settings_audio);


		char string_aofile_shown[10];
		menu_tape_settings_trunc_name(aofilename,string_aofile_shown,10);
		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_aofile,NULL,"    Audio ~~out to file");
        menu_add_item_menu_spanish_catalan(array_menu_settings_audio,"    Audio ~~out a archivo","    Audio ~~out a arxiu");
        menu_add_item_menu_sufijo_format(array_menu_settings_audio," [%s]",string_aofile_shown);
		menu_add_item_menu_shortcut(array_menu_settings_audio,'o');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Saves the generated sound to a file");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"You can save .raw format and if compiled with sndfile, to .wav format. "
					"You can see the file parameters on the console enabling verbose debug level to 2 minimum");
        menu_add_item_menu_es_avanzado(array_menu_settings_audio);



		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_aofile_insert,menu_aofile_cond,"Audio file ~~inserted");
        menu_add_item_menu_spanish_catalan(array_menu_settings_audio,"Archivo audio ~~insertado","Arxiu audio ~~insertat");
        menu_add_item_menu_prefijo_format(array_menu_settings_audio,"[%c] ",(aofile_inserted.v ? 'X' : ' ' ));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'i');
        menu_add_item_menu_es_avanzado(array_menu_settings_audio);


        menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_silence_detector,NULL,"Si~~lence detector");
        menu_add_item_menu_spanish_catalan(array_menu_settings_audio,"Detector de silencio","Detector de silenci");
        menu_add_item_menu_prefijo_format(array_menu_settings_audio,"[%c] ",(silence_detector_setting.v ? 'X' : ' ' ));
        menu_add_item_menu_shortcut(array_menu_settings_audio,'l');
        menu_add_item_menu_tooltip(array_menu_settings_audio,"Change this setting if you are listening some audio 'clicks'");
        menu_add_item_menu_ayuda(array_menu_settings_audio,"Change this setting if you are listening some audio 'clicks'");
        menu_add_item_menu_es_avanzado(array_menu_settings_audio);

        menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_change_audio_driver,NULL,"    Change Audio Driv~~er");
        menu_add_item_menu_spanish_catalan(array_menu_settings_audio,"    Cambiar Driv~~er Audio","    Canviar Driv~~er Audio");
        menu_add_item_menu_shortcut(array_menu_settings_audio,'e');
        menu_add_item_menu_tiene_submenu(array_menu_settings_audio);
        menu_add_item_menu_es_avanzado(array_menu_settings_audio);


			if (!strcmp(audio_new_driver_name,"sdl")) {
                menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_item_menu(array_menu_settings_audio,"--Audio SDL settings--",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_item_menu_en_es_ca(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audiosdl_callback_type,NULL,
                    "Callback Type","Tipo Callback","Tipus Callback");
                menu_add_item_menu_prefijo_format(array_menu_settings_audio,"[%s] ",
                    (audiosdl_use_new_callback.v ? "New" : "Old"));
                menu_add_item_menu_tooltip(array_menu_settings_audio,"Use Old Callback or New. New Callback is usually better on Windows\n");
                menu_add_item_menu_ayuda(array_menu_settings_audio,"Use Old Callback or New. New Callback is usually better on Windows\n");

                menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);         
                           
			}	


			if (!strcmp(audio_new_driver_name,"onebitspeaker")) {
                menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_item_menu(array_menu_settings_audio,"--One Bit Speaker settings--",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_item_menu_en_es_ca(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_onebitspeaker_tipo_speaker,NULL,
                    "Speaker Type","Tipo Speaker","Tipus Speaker");
                menu_add_item_menu_prefijo_format(array_menu_settings_audio,"[%s] ",
                    (audioonebitspeaker_tipo_altavoz==TIPO_ALTAVOZ_ONEBITSPEAKER_PCSPEAKER ? "PC Speaker" : "Raspberry GPIO"));

                if (audioonebitspeaker_tipo_altavoz==TIPO_ALTAVOZ_ONEBITSPEAKER_RPI_GPIO) {
                    menu_add_item_menu_en_es_ca(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_onebitspeaker_gpio_pin,NULL,
                        "GPIO Pinout number","GPIO Pinout number","GPIO Pinout number");
                    menu_add_item_menu_prefijo_format(array_menu_settings_audio,"[%d] ",audioonebitspeaker_rpi_gpio_pin);
                    menu_add_item_menu_tooltip(array_menu_settings_audio,"Which GPIO port is the speaker connected to");
                    menu_add_item_menu_ayuda(array_menu_settings_audio,"Which GPIO port is the speaker connected to");
                }


                menu_add_item_menu_en_es_ca(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_onebitspeaker_intensive_cpu,NULL,
                    "Improved sound","Sonido mejorado","So millorat");
                menu_add_item_menu_prefijo_format(array_menu_settings_audio,"[%c] ",(audioonebitspeaker_intensive_cpu_usage ? 'X' : ' ' ));
                menu_add_item_menu_tooltip(array_menu_settings_audio,"Improved sound but uses more cpu");
                menu_add_item_menu_ayuda(array_menu_settings_audio,"Improved sound but uses more cpu");

                
                menu_add_item_menu_en_es_ca(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_onebitspeaker_agudo_filtro,NULL,
                    "Hi Freq filter","Hi Freq filtro","Hi Freq filtre");
                menu_add_item_menu_prefijo_format(array_menu_settings_audio,"[%c] ",(audioonebitspeaker_agudo_filtro ? 'X' : ' ' ));
                menu_add_item_menu_tooltip(array_menu_settings_audio,"Filter to avoid high frequency sounds");
                menu_add_item_menu_ayuda(array_menu_settings_audio,"Filter to avoid high frequency sounds");     

                if (audioonebitspeaker_agudo_filtro) {
                    menu_add_item_menu_en_es_ca(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_onebitspeaker_agudo_filtro_limite,NULL,
                    "Limit","Límite","Límit");
                    menu_add_item_menu_prefijo_format(array_menu_settings_audio," [%d Hz] ",
                        FRECUENCIA_CONSTANTE_NORMAL_SONIDO/2/audioonebitspeaker_agudo_filtro_limite);
                    menu_add_item_menu_tooltip(array_menu_settings_audio,"Any sound with a frequency higher than this will not be heard");
                    menu_add_item_menu_ayuda(array_menu_settings_audio,"Any sound with a frequency higher than this will not be heard");
                }   

                menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);         
                           
			}				
	



            if (ay_chip_present.v || sn_chip_present.v || i8049_chip_present) {
					menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_direct_midi_output,audio_midi_available,"Audio Chip to ~~MIDI Output");
                    menu_add_item_menu_spanish_catalan(array_menu_settings_audio,"Audio Chip a salida ~~MIDI","Audio Xip a sortida ~~MIDI");
					menu_add_item_menu_tooltip(array_menu_settings_audio,"Direct Audio Chip (AY, SN or i8049) music output to a real MIDI device. Supported on Linux, Mac and Windows. On Linux, needs alsa driver compiled.");
            


#ifdef COMPILE_ALSA

					menu_add_item_menu_ayuda(array_menu_settings_audio,"Direct Audio Chip music output to a real MIDI device. Supported on Linux, Mac and Windows. On Linux, needs alsa driver compiled.\n"
						"On Linux you can simulate an external midi device by using timidity. If you have it installed, it may probably be running in memory as "
						"an alsa sequencer client. If not, run it with the command line:\n"
						"timidity -iA -Os -B2,8 -EFreverb=0\n"
						"Running timidity that way, would probably require that you use another audio driver in ZEsarUX different than alsa, "
						"unless you have alsa software mixing enabled"
					);

#else
					menu_add_item_menu_ayuda(array_menu_settings_audio,"Direct Audio Chip music output to a real MIDI device. Supported on Linux, Mac and Windows. On Linux, needs alsa driver compiled.");
#endif

					menu_add_item_menu_shortcut(array_menu_settings_audio,'m');
                    menu_add_item_menu_tiene_submenu(array_menu_settings_audio);
		
            }







        menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_ESC_item(array_menu_settings_audio);

        retorno_menu=menu_dibuja_menu(&settings_audio_opcion_seleccionada,&item_seleccionado,array_menu_settings_audio,"Audio Settings" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	                //llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}




void menu_ula_disable_rom_paging(MENU_ITEM_PARAMETERS)
{
	ula_disabled_rom_paging.v ^=1;
}

void menu_ula_disable_ram_paging(MENU_ITEM_PARAMETERS)
{
        ula_disabled_ram_paging.v ^=1;
}


void menu_ula_databus_value(MENU_ITEM_PARAMETERS)
{
    int valor=ula_databus_value;
    if (menu_ventana_scanf_numero_enhanced("ULA Databus value",&valor,4,+1,0,255,1)>=0) {
        ula_databus_value=valor;
    }
}

void menu_ula_contend(MENU_ITEM_PARAMETERS)
{
	contend_enabled.v ^=1;

	inicializa_tabla_contend();

}


void menu_ula_late_timings(MENU_ITEM_PARAMETERS)
{
	ula_late_timings.v ^=1;
	inicializa_tabla_contend();
}


void menu_ula_im2_slow(MENU_ITEM_PARAMETERS)
{
        ula_im2_slow.v ^=1;
}


void menu_ula_pentagon_timing(MENU_ITEM_PARAMETERS)
{
	if (pentagon_timing.v) {
		contend_enabled.v=1;
		ula_disable_pentagon_timing();
	}

	else {
		contend_enabled.v=0;
		ula_enable_pentagon_timing();
	}


}


//Retorna 0 si ok
//Retorna -1 si no hay cambio de variable
//Modifica valor de variable
int menu_hardware_advanced_input_value(int minimum,int maximum,char *texto,int *variable)
{
	
	int variable_copia;
	variable_copia=*variable;

	menu_ventana_scanf_numero_enhanced(texto,&variable_copia,4,+1,minimum,maximum,1);

	if (variable_copia==(*variable)) {
		//printf("no hay cambios\n");
		return -1;
	}

	else {
		*variable=variable_copia;
		return 0;
	}
	/*

	int valor;

        char string_value[4];

        sprintf (string_value,"%d",*variable);


        menu_ventana_scanf(texto,string_value,4);

        valor=parse_string_to_number(string_value);

	if (valor<minimum || valor>maximum) {
		debug_printf (VERBOSE_ERR,"Value out of range. Minimum: %d Maximum: %d",minimum,maximum);
		return -1;
	}

	*variable=valor;
	return 0;
	*/


}

void menu_hardware_advanced_reload_display(void)
{

		screen_testados_linea=screen_total_borde_izquierdo/2+128+screen_total_borde_derecho/2+screen_invisible_borde_derecho/2;

	        //Recalcular algunos valores cacheados
	        recalcular_get_total_ancho_rainbow();
        	recalcular_get_total_alto_rainbow();

                screen_set_video_params_indices();
                inicializa_tabla_contend();

                init_rainbow();
                init_cache_putpixel();
}



void menu_hardware_advanced_hidden_top_border(MENU_ITEM_PARAMETERS)
{
	int max,min;
	min=7;
	max=16;
	if (MACHINE_IS_PRISM) {
		min=32;
		max=45;
	}

	if (menu_hardware_advanced_input_value(min,max,"Hidden top Border",&screen_invisible_borde_superior)==0) {
		menu_hardware_advanced_reload_display();
	}
}

void menu_hardware_advanced_visible_top_border(MENU_ITEM_PARAMETERS)
{

	int max,min;
	min=32;
	max=56;
	if (MACHINE_IS_PRISM) {
		min=32;
		max=48;
	}

        if (menu_hardware_advanced_input_value(min,max,"Visible top Border",&screen_borde_superior)==0) {
                menu_hardware_advanced_reload_display();
        }
}

void menu_hardware_advanced_visible_bottom_border(MENU_ITEM_PARAMETERS)
{
	int max,min;
	min=48;
	max=56;
	if (MACHINE_IS_PRISM) {
		min=32;
                max=48;
        }



        if (menu_hardware_advanced_input_value(min,max,"Visible bottom Border",&screen_total_borde_inferior)==0) {
                menu_hardware_advanced_reload_display();
        }
}

void menu_hardware_advanced_borde_izquierdo(MENU_ITEM_PARAMETERS)
{

	int valor_pixeles;

	valor_pixeles=screen_total_borde_izquierdo/2;

        int max,min;
	min=12;
	max=24;
	if (MACHINE_IS_PRISM) {
                min=20;
		max=32;
	}

	if (menu_hardware_advanced_input_value(min,max,"Left Border TLength",&valor_pixeles)==0) {
		screen_total_borde_izquierdo=valor_pixeles*2;
		 menu_hardware_advanced_reload_display();
        }
}

void menu_hardware_advanced_borde_derecho(MENU_ITEM_PARAMETERS)
{

        int valor_pixeles;

	valor_pixeles=screen_total_borde_derecho/2;

        int max,min;
	min=12;
        max=24;
	if (MACHINE_IS_PRISM) {
                min=20;
                max=32;
        }

        if (menu_hardware_advanced_input_value(min,max,"Right Border TLength",&valor_pixeles)==0) {
                screen_total_borde_derecho=valor_pixeles*2;
                 menu_hardware_advanced_reload_display();
        }
}

void menu_hardware_advanced_hidden_borde_derecho(MENU_ITEM_PARAMETERS)
{

        int valor_pixeles;

	valor_pixeles=screen_invisible_borde_derecho/2;

        int max,min;
	min=24;
	max=52;

	if (MACHINE_IS_PRISM) {
                min=60;
                max=79;
        }

        if (menu_hardware_advanced_input_value(min,max,"Right Hidden B.TLength",&valor_pixeles)==0) {
                screen_invisible_borde_derecho=valor_pixeles*2;
                 menu_hardware_advanced_reload_display();
        }
}


void menu_ula_advanced(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_hardware_advanced;
    menu_item item_seleccionado;
    int retorno_menu;
    do {
        menu_add_item_menu_en_es_ca_inicial(&array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_hidden_top_border,NULL,
            "Hidden Top Border","Borde Superior Oculto","Vora Superior Oculta");
        menu_add_item_menu_prefijo_format(array_menu_hardware_advanced,"[%2d] ",screen_invisible_borde_superior);

        menu_add_item_menu_en_es_ca(array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_visible_top_border,NULL,
            "Visible Top Border","Borde Superior Visible","Vora Superior Visible");
        menu_add_item_menu_prefijo_format(array_menu_hardware_advanced,"[%d] ",screen_borde_superior);

        menu_add_item_menu_en_es_ca(array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_visible_bottom_border,NULL,
            "Visible Bottom Border","Borde Inferior Visible","Vora Inferior Visible");
        menu_add_item_menu_prefijo_format(array_menu_hardware_advanced,"[%d] ",screen_total_borde_inferior);

        menu_add_item_menu_en_es_ca(array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_borde_izquierdo,NULL,
            "Left Border TLength","Borde Izquierdo TLongitud","Vora Esquerra TLongitut");
        menu_add_item_menu_prefijo_format(array_menu_hardware_advanced,"[%d] ",screen_total_borde_izquierdo/2);

        menu_add_item_menu_en_es_ca(array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_borde_derecho,NULL,
            "Right Border TLength","Borde Derecho TLongitud","Vora Dreta TLongitut");
        menu_add_item_menu_prefijo_format(array_menu_hardware_advanced,"[%d] ",screen_total_borde_derecho/2);

        menu_add_item_menu_en_es_ca(array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_hidden_borde_derecho,NULL,
            "Right Hidden B. TLength","B. Derecho Oculto TLongitud","V. Dreta Oculta TLongitut");
        menu_add_item_menu_prefijo_format(array_menu_hardware_advanced,"[%d] ",screen_invisible_borde_derecho/2);


        menu_add_item_menu(array_menu_hardware_advanced,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,NULL,NULL,"Info:");
        menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,NULL,NULL,"Total line TLength: %d",screen_testados_linea);
        menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,NULL,NULL,"Total scanlines: %d",screen_scanlines);
        menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,NULL,NULL,"Total T-states: %d",screen_testados_total);
        menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,NULL,NULL,"Total Hz: %d",screen_testados_total*50);

        menu_add_item_menu(array_menu_hardware_advanced,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        //menu_add_item_menu(array_menu_hardware_advanced,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
        menu_add_ESC_item(array_menu_hardware_advanced);

        retorno_menu=menu_dibuja_menu(&hardware_advanced_opcion_seleccionada,&item_seleccionado,array_menu_hardware_advanced,"Advanced Timing Settings" );

            

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                        
                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



//menu ula settings
void menu_ula_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_ula_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial(&array_menu_ula_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);


#ifdef EMULATE_CONTEND

        if (MACHINE_IS_SPECTRUM) {
            menu_add_item_menu_en_es_ca(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_late_timings,NULL,
                "ULA ~~timing","~~Temporización ULA","~~Temporització ULA");
            menu_add_item_menu_prefijo_format(array_menu_ula_settings,"[%s] ",(ula_late_timings.v ? "Late" : "Early"));
            menu_add_item_menu_shortcut(array_menu_ula_settings,'t');
            menu_add_item_menu_tooltip(array_menu_ula_settings,"Use ULA early or late timings");
            menu_add_item_menu_ayuda(array_menu_ula_settings,"Late timings have the contended memory table start one t-state later");

            menu_add_item_menu_en_es_ca(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_contend,NULL,
                "~~Contended memory","Memoria ~~Contended","Memoria ~~Contended");
            menu_add_item_menu_prefijo_format(array_menu_ula_settings,"[%c] ", (contend_enabled.v==1 ? 'X' : ' '));
            menu_add_item_menu_shortcut(array_menu_ula_settings,'c');
            menu_add_item_menu_tooltip(array_menu_ula_settings,"Enable contended memory & ports emulation");
            menu_add_item_menu_ayuda(array_menu_ula_settings,"Contended memory & ports is the native way of some of the emulated machines");

		}

#endif

        if (MACHINE_IS_SPECTRUM) {

			menu_add_item_menu_en_es_ca(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_im2_slow,NULL,
                "ULA IM2 s~~low","ULA IM2 ~~lenta","ULA IM2 ~~lenta");
            menu_add_item_menu_prefijo_format(array_menu_ula_settings,"[%c] ",(ula_im2_slow.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_ula_settings,'l');
			menu_add_item_menu_tooltip(array_menu_ula_settings,"Add one t-state when an IM2 is fired");
			menu_add_item_menu_ayuda(array_menu_ula_settings,"It improves visualization on some demos, like overscan, ula128 and scroll2017");
        }




		if (MACHINE_IS_SPECTRUM) {

            menu_add_item_menu_en_es_ca(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_pentagon_timing,NULL,
                "~~Pentagon timing","Temporización ~~Pentagon","Temporització ~~Pentagon");
            menu_add_item_menu_prefijo_format(array_menu_ula_settings,"[%c] ",(pentagon_timing.v ? 'X' : ' '));
            menu_add_item_menu_shortcut(array_menu_ula_settings,'p');
            menu_add_item_menu_tooltip(array_menu_ula_settings,"Enable Pentagon timings");
            menu_add_item_menu_ayuda(array_menu_ula_settings,"Pentagon does not have contended memory/ports and have different display timings");

		}


		if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
			menu_add_item_menu_en_es_ca(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_disable_rom_paging,NULL,
                "Allow ROM Paging","Permitir paginar ROM","Permetre Paginar ROM");
            menu_add_item_menu_prefijo_format(array_menu_ula_settings,"[%c] ",(ula_disabled_rom_paging.v==0 ? 'X' : ' '));

			menu_add_item_menu_en_es_ca(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_disable_ram_paging,NULL,
                "Allow RAM Paging","Permitir paginar RAM","Permetre Paginar RAM");
            menu_add_item_menu_prefijo_format(array_menu_ula_settings,"[%c] ",(ula_disabled_ram_paging.v==0 ? 'X' : ' '));
		}

        if (CPU_IS_Z80) {
            menu_add_item_menu_en_es_ca(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_databus_value,NULL,
                "~~ULA Databus value","Valor ~~ULA Databus","Valor ~~ULA Databus");
            menu_add_item_menu_prefijo_format(array_menu_ula_settings,"[%d] ",ula_databus_value);
            menu_add_item_menu_shortcut(array_menu_ula_settings,'u');
        }

        menu_add_item_menu(array_menu_ula_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_item_menu_en_es_ca(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_advanced,menu_cond_realvideo,
            "~~Advanced timing settings","Ajustes ~~avanzados temporización","Ajustaments ~~avançats temporització");
        menu_add_item_menu_shortcut(array_menu_ula_settings,'a');
        menu_add_item_menu_tooltip(array_menu_ula_settings,"Advanced timing settings. Requires realvideo");
        menu_add_item_menu_ayuda(array_menu_ula_settings,"Change and view some timings for the machine. Requires realvideo");
        menu_add_item_menu_tiene_submenu(array_menu_ula_settings);


        menu_add_item_menu(array_menu_ula_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        //menu_add_item_menu(array_menu_ula_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
        menu_add_ESC_item(array_menu_ula_settings);

        retorno_menu=menu_dibuja_menu(&ula_settings_opcion_seleccionada,&item_seleccionado,array_menu_ula_settings,"ULA Settings" );

        
        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                        
                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



void menu_hardware_datagear_dma(MENU_ITEM_PARAMETERS)
{
	if (datagear_dma_emulation.v) datagear_dma_disable();
	else datagear_dma_enable();
}

void menu_hardware_kempston_mouse_sensibilidad(MENU_ITEM_PARAMETERS)
{
	//kempston_mouse_factor_sensibilidad++;
	//if (kempston_mouse_factor_sensibilidad==6) kempston_mouse_factor_sensibilidad=1;
	char titulo_ventana[33];
	sprintf (titulo_ventana,"Sensitivity (1-%d)",MAX_KMOUSE_SENSITIVITY);

	menu_hardware_advanced_input_value(1,MAX_KMOUSE_SENSITIVITY,titulo_ventana,&kempston_mouse_factor_sensibilidad);
}

void menu_tbblue_fast_boot_mode(MENU_ITEM_PARAMETERS)
{
	tbblue_fast_boot_mode.v ^=1;
}

void menu_tbblue_rtc_traps(MENU_ITEM_PARAMETERS)
{
	tbblue_use_rtc_traps ^=1;
}

void menu_hardware_tbblue_core_version(MENU_ITEM_PARAMETERS)
{
	char string_value[4];

	int valor;

	sprintf (string_value,"%d",tbblue_core_current_version_major);
	//Entre 0 y 255
	menu_ventana_scanf("Major",string_value,4);
	valor=parse_string_to_number(string_value);
	if (valor<0 || valor>255) {
		debug_printf (VERBOSE_ERR,"Invalid value");
		return;
	}

	tbblue_core_current_version_major=valor;


	sprintf (string_value,"%d",tbblue_core_current_version_minor);
	//Entre 0 y 15
	menu_ventana_scanf("Minor",string_value,3);
	valor=parse_string_to_number(string_value);
	if (valor<0 || valor>15) {
		debug_printf (VERBOSE_ERR,"Invalid value");
		return;
	}

	tbblue_core_current_version_minor=valor;

	sprintf (string_value,"%d",tbblue_core_current_version_subminor);
	//Entre 0 y 15
	menu_ventana_scanf("Subminor",string_value,3);
	valor=parse_string_to_number(string_value);
	if (valor<0 || valor>15) {
		debug_printf (VERBOSE_ERR,"Invalid value");
		return;
	}
	
	tbblue_core_current_version_subminor=valor;	


}

void menu_hardware_joystick_fire_key(MENU_ITEM_PARAMETERS)
{
    joystick_defined_key_fire++;
    if (joystick_defined_key_fire==4) joystick_defined_key_fire=0;
}



//OLD: Solo permitimos autofire para Kempston, Fuller ,Zebra y mikrogen, para evitar que con cursor o con sinclair se este mandando una tecla y dificulte moverse por el menu
int menu_hardware_autofire_cond(void)
{
	//if (joystick_emulation==JOYSTICK_FULLER || joystick_emulation==JOYSTICK_KEMPSTON || joystick_emulation==JOYSTICK_ZEBRA || joystick_emulation==JOYSTICK_MIKROGEN) return 1;
	//return 0;

	return 1;
}




void menu_hardware_joystick(MENU_ITEM_PARAMETERS)
{


    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    int opcion_seleccionada=joystick_emulation;


    menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

    int i;

    for (i=0;i<=JOYSTICK_TOTAL;i++) {

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,joystick_texto[i]);

    }

    menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

    menu_add_ESC_item(array_menu_common);

    retorno_menu=menu_dibuja_menu(&opcion_seleccionada,&item_seleccionado,array_menu_common,"Joystick");

     

    if (retorno_menu==MENU_RETORNO_NORMAL && (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0) {
        joystick_emulation=opcion_seleccionada;
        joystick_cycle_next_type_autofire();
    }
    
}

void menu_hardware_gunstick(MENU_ITEM_PARAMETERS)
{
        if (gunstick_emulation==GUNSTICK_TOTAL) gunstick_emulation=0;
	else gunstick_emulation++;
}

void menu_hardware_gunstick_range_x(MENU_ITEM_PARAMETERS)
{
	if (gunstick_range_x==256) gunstick_range_x=1;
	else gunstick_range_x *=2;
}

void menu_hardware_gunstick_range_y(MENU_ITEM_PARAMETERS)
{
        if (gunstick_range_y==64) gunstick_range_y=1;
        else gunstick_range_y *=2;
}

void menu_hardware_gunstick_y_offset(MENU_ITEM_PARAMETERS)
{
	if (gunstick_y_offset==32) gunstick_y_offset=0;
	else gunstick_y_offset +=4;
}

void menu_hardware_gunstick_solo_brillo(MENU_ITEM_PARAMETERS)
{
	gunstick_solo_brillo ^=1;
}

int menu_hardware_gunstick_aychip_cond(void)
{
	if (gunstick_emulation==GUNSTICK_AYCHIP) return 1;
	else return 0;
}

void menu_hardware_autofire(MENU_ITEM_PARAMETERS)
{
	if (joystick_autofire_frequency==0) joystick_autofire_frequency=1;
	else if (joystick_autofire_frequency==1) joystick_autofire_frequency=2;
	else if (joystick_autofire_frequency==2) joystick_autofire_frequency=5;
	else if (joystick_autofire_frequency==5) joystick_autofire_frequency=10;
	else if (joystick_autofire_frequency==10) joystick_autofire_frequency=25;
	else if (joystick_autofire_frequency==25) joystick_autofire_frequency=50;

	else joystick_autofire_frequency=0;
}

void menu_hardware_autoleftright(MENU_ITEM_PARAMETERS)
{
	if (joystick_autoleftright_frequency==1) joystick_autoleftright_frequency=2;
	else if (joystick_autoleftright_frequency==2) joystick_autoleftright_frequency=5;
	else if (joystick_autoleftright_frequency==5) joystick_autoleftright_frequency=10;
	else if (joystick_autoleftright_frequency==10) joystick_autoleftright_frequency=25;
	else if (joystick_autoleftright_frequency==25) joystick_autoleftright_frequency=50;

	else joystick_autoleftright_frequency=1;    
}

void menu_hardware_kempston_mouse(MENU_ITEM_PARAMETERS)
{
	kempston_mouse_emulation.v ^=1;
}


int menu_hardware_realjoystick_cond(void)
{
	return realjoystick_present.v;
}





void menu_hardware_memory_refresh(MENU_ITEM_PARAMETERS)
{
	if (machine_emulate_memory_refresh==0) {
		set_peek_byte_function_ram_refresh();
		machine_emulate_memory_refresh=1;
	}

	else {
		reset_peek_byte_function_ram_refresh();
                machine_emulate_memory_refresh=0;
	}
}



int menu_cond_allow_write_rom(void)
{

	if (superupgrade_enabled.v) return 0;
	if (dandanator_enabled.v) return 0;
	if (kartusho_enabled.v) return 0;
	if (samram_enabled.v) return 0;
	if (ifrom_enabled.v) return 0;
	if (betadisk_enabled.v) return 0;

	if (MACHINE_IS_INVES) return 0;
	if (MACHINE_IS_SPECTRUM_16_48) return 1;
	if (MACHINE_IS_ZX8081) return 1;
	if (MACHINE_IS_ACE) return 1;
	if (MACHINE_IS_SAM) return 1;

	return 0;

}

void menu_hardware_allow_write_rom(MENU_ITEM_PARAMETERS)
{
	if (allow_write_rom.v) {
		reset_poke_byte_function_writerom();
                allow_write_rom.v=0;

	}

	else {
		set_poke_byte_function_writerom();
		allow_write_rom.v=1;
	}
}



void menu_hardware_memory_128k_multiplier(MENU_ITEM_PARAMETERS)
{

	z80_byte valor=mem128_multiplicador;

	if (valor==8) valor=1;
	else valor <<=1;

	mem_set_multiplicador_128(valor);
}

void menu_hardware_tbblue_ram(MENU_ITEM_PARAMETERS)
{
	if (tbblue_extra_512kb_blocks==3) tbblue_extra_512kb_blocks=0;
	else tbblue_extra_512kb_blocks++;
}



void menu_hardware_ace_ramtop(MENU_ITEM_PARAMETERS)
{


        char string_ramtop[3];

        //int valor_antes_ramtop=((ramtop_ace-16383)/1024)+3;
				int valor_antes_ramtop=get_ram_ace();

	//printf ("ramtop: %d\n",valor_antes_ramtop);

        sprintf (string_ramtop,"%d",valor_antes_ramtop);


        menu_ventana_scanf("RAM? (3, 19, 35 or 51)",string_ramtop,3);

        int valor_leido_ramtop=parse_string_to_number(string_ramtop);

        //si mismo valor volver
        if (valor_leido_ramtop==valor_antes_ramtop) return;

	if (valor_leido_ramtop!=3 && valor_leido_ramtop!=19 && valor_leido_ramtop!=35 && valor_leido_ramtop!=51) {
		debug_printf (VERBOSE_ERR,"Invalid RAM value");
		return;
	}


	set_ace_ramtop(valor_leido_ramtop);
        reset_cpu();

}



void menu_hardware_zx8081_ramtop(MENU_ITEM_PARAMETERS)
{


        char string_ramtop[3];

	//int valor_antes_ramtop=(ramtop_zx8081-16383)/1024;
	int valor_antes_ramtop=zx8081_get_standard_ram();

        sprintf (string_ramtop,"%d",valor_antes_ramtop);


        menu_ventana_scanf("Standard RAM",string_ramtop,3);

        int valor_leido_ramtop=parse_string_to_number(string_ramtop);

	//si mismo valor volver
	if (valor_leido_ramtop==valor_antes_ramtop) return;


	if (valor_leido_ramtop>0 && valor_leido_ramtop<17) {
		set_zx8081_ramtop(valor_leido_ramtop);
		//ramtop_zx8081=16383+1024*valor_leido_ramtop;
		reset_cpu();
	}

}

void menu_hardware_zx8081_ram_in_8192(MENU_ITEM_PARAMETERS)
{
	ram_in_8192.v ^=1;
}

void menu_hardware_zx8081_ram_in_49152(MENU_ITEM_PARAMETERS)
{
        if (ram_in_49152.v==0) enable_ram_in_49152();
        else ram_in_49152.v=0;

}

void menu_hardware_zx8081_ram_in_32768(MENU_ITEM_PARAMETERS)
{

	if (ram_in_32768.v==0) enable_ram_in_32768();
	else {
		ram_in_32768.v=0;
		ram_in_49152.v=0;
	}

}




void menu_hardware_sam_ram(MENU_ITEM_PARAMETERS)
{
	if (sam_memoria_total_mascara==15) sam_memoria_total_mascara=31;
	else sam_memoria_total_mascara=15;
}


void menu_hardware_inves_poke(MENU_ITEM_PARAMETERS)
{


	char string_poke[4];

	sprintf (string_poke,"%d",last_inves_low_ram_poke_menu);


	menu_ventana_scanf("Poke low RAM with",string_poke,4);

	last_inves_low_ram_poke_menu=parse_string_to_number(string_poke);

	modificado_border.v=1;

	poke_inves_rom(last_inves_low_ram_poke_menu);

}


int menu_inves_cond(void)
{
        if (MACHINE_IS_INVES) return 1;
        else return 0;
}


void menu_hardware_pcw_ram(MENU_ITEM_PARAMETERS)
{

    if (pcw_total_ram==2*1024*1024) pcw_total_ram=256*1024;
    else pcw_total_ram *=2;

}
//menu hardware settings
void menu_hardware_memory_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_memory_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_en_es_ca_inicial(&array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_allow_write_rom,
            menu_cond_allow_write_rom,"Allow ~~write in ROM","Permitir ~~write en ROM","Permetre ~~write a ROM");
		menu_add_item_menu_prefijo_format(array_menu_hardware_memory_settings,"[%c] ",(allow_write_rom.v ? 'X' : ' ') );            
		menu_add_item_menu_shortcut(array_menu_hardware_memory_settings,'w');
		menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"Allow write in ROM");
		menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"Allow write in ROM. Only allowed on Spectrum 48k/16k models, ZX80, ZX81, Sam Coupe and Jupiter Ace (and not on Inves)");

		if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
			menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_memory_128k_multiplier,NULL,"RAM size [%4d KB]",128*mem128_multiplicador);
			menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"Allows setting more than 128k RAM on a 128k type machine");
			menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"Allows setting more than 128k RAM on a 128k type machine");
		}

		if (MACHINE_IS_TBBLUE) {
			menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_tbblue_ram,NULL,"RAM size [%d KB]",tbblue_get_current_ram() );
		}

		if (MACHINE_IS_PCW) {
			menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_pcw_ram,NULL,"RAM size [%d KB]",pcw_total_ram/1024 );
		}        

		if (menu_cond_zx8081() ) {

                        //int ram_zx8081=(ramtop_zx8081-16383)/1024;
												int ram_zx8081=zx8081_get_standard_ram();
                        menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_zx8081_ramtop,menu_cond_zx8081,"ZX80/81 Standard RAM [%d KB]",ram_zx8081);
                        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"Standard RAM for the ZX80/81");
                        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"Standard RAM for the ZX80/81");


                        menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_zx8081_ram_in_8192,menu_cond_zx8081,"[%c] ZX80/81 8K RAM in 2000H", (ram_in_8192.v ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"8KB RAM at address 2000H");
                        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"8KB RAM at address 2000H. Used on some wrx games");

                        menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_zx8081_ram_in_32768,menu_cond_zx8081,"[%c] ZX80/81 16K RAM in 8000H", (ram_in_32768.v ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"16KB RAM at address 8000H");
                        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"16KB RAM at address 8000H");
                        menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_zx8081_ram_in_49152,menu_cond_zx8081,"[%c] ZX80/81 16K RAM in C000H", (ram_in_49152.v ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"16KB RAM at address C000H");
                        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"16KB RAM at address C000H. It requires the previous RAM at 8000H");


                }


		if (MACHINE_IS_SAM) {
			menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_sam_ram,NULL,"Sam Coupe RAM [%d KB]",get_sam_ram() );
		}


		if (MACHINE_IS_ACE) {
			//int ram_ace=((ramtop_ace-16383)/1024)+3;
			int ram_ace=get_ram_ace();
			menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_ace_ramtop,NULL,"Jupiter Ace RAM [%d KB]",ram_ace);
		}



      if (MACHINE_IS_SPECTRUM_48) {
        menu_add_item_menu_en_es_ca(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_memory_refresh,NULL,
            "RAM Refresh emulation","Emulación Refresco RAM","Emulació Refresc RAM");
        menu_add_item_menu_prefijo_format(array_menu_hardware_memory_settings,"[%c] ", (machine_emulate_memory_refresh==1 ? 'X' : ' '));
        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"Enable RAM R~~efresh emulation");
        menu_add_item_menu_shortcut(array_menu_hardware_memory_settings,'e');
        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"RAM Refresh emulation consists, in a real Spectrum 48k, "
                        "to refresh the upper 32kb RAM using the R register. On a real Spectrum 48k, if you modify "
                        "the R register very fast, you can lose RAM contents.\n"
                        "This option emulates this behaviour, and sure you don't need to enable it on the 99.99 percent of the "
                        "situations ;) ");
        }


	  if (MACHINE_IS_INVES) {
            menu_add_item_menu_en_es_ca(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_inves_poke,menu_inves_cond,
                "Poke Inves Low RAM","Pokear Inves RAM Baja","Pokear Inves RAM Baixa");
            menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"Poke Inves low RAM");
            menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"You can alter the way Inves work with ULA port (sound & border). "
                            "You change here the contents of the low (hidden) RAM of the Inves (addresses 0-16383). Choosing this option "
                            "is the same as poke at the whole low RAM addresses (0 until 16383). I suggest to poke with value 15 or 23 "
                            "on games that you can not hear well the music: Lemmings, ATV, Batman caped crusader...");

        }




		menu_add_item_menu(array_menu_hardware_memory_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_ESC_item(array_menu_hardware_memory_settings);

        retorno_menu=menu_dibuja_menu(&hardware_memory_settings_opcion_seleccionada,&item_seleccionado,array_menu_hardware_memory_settings,"Memory Settings" );

        
        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                        
                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}





void menu_hardware_printers_zxprinter_enable(MENU_ITEM_PARAMETERS)
{
	zxprinter_enabled.v ^=1;

	if (zxprinter_enabled.v==0) {
	        close_zxprinter_bitmap_file();
        	close_zxprinter_ocr_file();
	}

}

int menu_hardware_zxprinter_cond(void)
{
	return zxprinter_enabled.v;
}

void menu_hardware_zxprinter_bitmapfile(MENU_ITEM_PARAMETERS)
{


	close_zxprinter_bitmap_file();

        char *filtros[3];

        filtros[0]="txt";
        filtros[1]="pbm";
        filtros[2]=0;


        if (menu_filesel("Select Bitmap File",filtros,zxprinter_bitmap_filename_buffer)==1) {
                //Ver si archivo existe y preguntar
                struct stat buf_stat;

                if (stat(zxprinter_bitmap_filename_buffer, &buf_stat)==0) {

                        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

                }

                zxprinter_bitmap_filename=zxprinter_bitmap_filename_buffer;

		zxprinter_file_bitmap_init();


        }

//        else {
//		close_zxprinter_file();
//        }
}

void menu_hardware_zxprinter_ocrfile(MENU_ITEM_PARAMETERS)
{


        close_zxprinter_ocr_file();

        char *filtros[2];

        filtros[0]="txt";
        filtros[1]=0;


        if (menu_filesel("Select OCR File",filtros,zxprinter_ocr_filename_buffer)==1) {
                //Ver si archivo existe y preguntar
                struct stat buf_stat;

                if (stat(zxprinter_ocr_filename_buffer, &buf_stat)==0) {

                        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

                }

                zxprinter_ocr_filename=zxprinter_ocr_filename_buffer;

                zxprinter_file_ocr_init();


        }

//        else {
//              close_zxprinter_file();
//        }
}


void menu_hardware_zxprinter_copy(MENU_ITEM_PARAMETERS)
{
        push_valor(reg_pc,PUSH_VALUE_TYPE_CALL);

	if (MACHINE_IS_SPECTRUM) {
	        reg_pc=0x0eac;
	}

	if (MACHINE_IS_ZX81_TYPE) {
		reg_pc=0x0869;
	}


	if (menu_multitarea) menu_generic_message("COPY","OK. COPY executed");
	else menu_generic_message("COPY","Register PC set to the COPY routine. Return to the emulator to let the COPY routine to be run");


}



void menu_hardware_printers(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_hardware_printers;
    menu_item item_seleccionado;
    int retorno_menu;
    do {
                menu_add_item_menu_inicial_format(&array_menu_hardware_printers,MENU_OPCION_NORMAL,menu_hardware_printers_zxprinter_enable,NULL,"[%c] ZX Printer",(zxprinter_enabled.v==1 ? 'X' : ' ' ));
		menu_add_item_menu_tooltip(array_menu_hardware_printers,"Enables or disables ZX Printer emulation");
		menu_add_item_menu_ayuda(array_menu_hardware_printers,"You must set it to off when finishing printing to close generated files");

        char string_bitmapfile_shown[16];
        menu_tape_settings_trunc_name(zxprinter_bitmap_filename,string_bitmapfile_shown,16);

        menu_add_item_menu_en_es_ca(array_menu_hardware_printers,MENU_OPCION_NORMAL,menu_hardware_zxprinter_bitmapfile,
            menu_hardware_zxprinter_cond,"Bitmap file","Archivo bitmap","Arxiu bitmap");
        menu_add_item_menu_sufijo_format(array_menu_hardware_printers," [%s]",string_bitmapfile_shown);

        menu_add_item_menu_tooltip(array_menu_hardware_printers,"Sends printer output to image file");
        menu_add_item_menu_ayuda(array_menu_hardware_printers,"Printer output is saved to a image file. Supports pbm file format, and "
            "also supports text file, "
            "where every pixel is a character on text. "
            "It is recommended to close the image file when finishing printing, so its header is updated");


		char string_ocrfile_shown[19];
		menu_tape_settings_trunc_name(zxprinter_ocr_filename,string_ocrfile_shown,19);

        menu_add_item_menu_en_es_ca(array_menu_hardware_printers,MENU_OPCION_NORMAL,menu_hardware_zxprinter_ocrfile,
            menu_hardware_zxprinter_cond,"OCR file","Archivo OCR","Arxiu OCR");
        menu_add_item_menu_sufijo_format(array_menu_hardware_printers," [%s]",string_ocrfile_shown);

        menu_add_item_menu_tooltip(array_menu_hardware_printers,"Sends printer output to text file using OCR method");
        menu_add_item_menu_ayuda(array_menu_hardware_printers,"Printer output is saved to a text file using OCR method to guess text. "
            "If you cancel a printing with SHIFT+SPACE on Basic, you have to re-select the ocr file to reset some "
            "internal counters. If you don't do that, OCR will not work");


		menu_add_item_menu_en_es_ca(array_menu_hardware_printers,MENU_OPCION_NORMAL,menu_hardware_zxprinter_copy,
            menu_hardware_zxprinter_cond,"Run COPY routine","Ejecutar rutina COPY","Executar rutina COPY");

        menu_add_item_menu_tooltip(array_menu_hardware_printers,"Runs ROM COPY routine");
		menu_add_item_menu_ayuda(array_menu_hardware_printers,"It calls ROM copy routine on Spectrum and ZX-81, like the COPY command on BASIC. \n"
            "I did not guarantee that the call will always work, because this function will probably "
            "use some structures and variables needed in BASIC and if you are running some game, maybe it "
            "has not these variables correct");



        menu_add_item_menu(array_menu_hardware_printers,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        //menu_add_item_menu(array_menu_hardware_printers,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
        menu_add_ESC_item(array_menu_hardware_printers);

        retorno_menu=menu_dibuja_menu(&hardware_printers_opcion_seleccionada,&item_seleccionado,array_menu_hardware_printers,"Printing emulation" );

                

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                        
                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



void menu_cpu_speed(MENU_ITEM_PARAMETERS)
{
	menu_ventana_scanf_numero_enhanced("Emulator Speed (%)",&porcentaje_velocidad_emulador,5,+25,1,9999,0);


	set_emulator_speed();

}





void menu_hardware_keyboard_issue(MENU_ITEM_PARAMETERS)
{
	keyboard_issue2.v^=1;
}

void menu_hardware_azerty(MENU_ITEM_PARAMETERS)
{
	azerty_keyboard.v ^=1;
}


void menu_chloe_keyboard(MENU_ITEM_PARAMETERS)
{
	chloe_keyboard.v ^=1;
	scr_z88_cpc_load_keymap();
}

void menu_hardware_spectrum_keyboard_matrix_error(MENU_ITEM_PARAMETERS)
{
	keyboard_matrix_error.v ^=1;
}

void menu_hardware_recreated_keyboard(MENU_ITEM_PARAMETERS)
{
	recreated_zx_keyboard_support.v ^=1;
}

void menu_hardware_sdl_raw_read(MENU_ITEM_PARAMETERS)
{
	sdl_raw_keyboard_read.v ^=1;
}

int menu_hardware_keyboard_issue_cond(void)
{
	if (MACHINE_IS_SPECTRUM) return 1;
	return 0;
}

void menu_hardware_keymap_z88_cpc(MENU_ITEM_PARAMETERS) {
	//solo hay dos tipos
	z88_cpc_keymap_type ^=1;
	scr_z88_cpc_load_keymap();
}


void menu_hardware_redefine_keys_set_keys(MENU_ITEM_PARAMETERS)
{


        z80_byte tecla_original,tecla_redefinida;
	tecla_original=lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_original;
	tecla_redefinida=lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_redefinida;

        char buffer_caracter_original[2];
	char buffer_caracter_redefinida[2];

	if (tecla_original==0) {
		buffer_caracter_original[0]=0;
		buffer_caracter_redefinida[0]=0;
	}


	else {
		buffer_caracter_original[0]=(tecla_original>=32 && tecla_original <=127 ? tecla_original : '?');
		buffer_caracter_redefinida[0]=(tecla_redefinida>=32 && tecla_redefinida <=127 ? tecla_redefinida : '?');
	}

        buffer_caracter_original[1]=0;
        buffer_caracter_redefinida[1]=0;




        menu_ventana_scanf("Original key",buffer_caracter_original,2);
        tecla_original=buffer_caracter_original[0];

	if (tecla_original==0) {
		lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_original=0;
		return;
	}


        menu_ventana_scanf("Destination key",buffer_caracter_redefinida,2);
        tecla_redefinida=buffer_caracter_redefinida[0];

	if (tecla_redefinida==0) {
                lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_original=0;
                return;
        }


	lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_original=tecla_original;
	lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_redefinida=tecla_redefinida;

}




void menu_hardware_redefine_keys(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_redefine_keys;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char buffer_texto[40];

                int i;
                for (i=0;i<MAX_TECLAS_REDEFINIDAS;i++) {
				z80_byte tecla_original=lista_teclas_redefinidas[i].tecla_original;
				z80_byte tecla_redefinida=lista_teclas_redefinidas[i].tecla_redefinida;
                        if (tecla_original) {
					sprintf(buffer_texto,"Key %c to %c",(tecla_original>=32 && tecla_original <=127 ? tecla_original : '?'),
					(tecla_redefinida>=32 && tecla_redefinida <=127 ? tecla_redefinida : '?') );

								}

                        else {
                                sprintf(buffer_texto,"Unused entry");
                        }



                        if (i==0) menu_add_item_menu_inicial_format(&array_menu_hardware_redefine_keys,MENU_OPCION_NORMAL,menu_hardware_redefine_keys_set_keys,NULL,buffer_texto);
                        else menu_add_item_menu_format(array_menu_hardware_redefine_keys,MENU_OPCION_NORMAL,menu_hardware_redefine_keys_set_keys,NULL,buffer_texto);


                        menu_add_item_menu_tooltip(array_menu_hardware_redefine_keys,"Redefine the key");
                        menu_add_item_menu_ayuda(array_menu_hardware_redefine_keys,"Indicates which key on the Spectrum keyboard is sent when "
                                                "pressed the original key");
                }



                menu_add_item_menu(array_menu_hardware_redefine_keys,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_redefine_keys,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_redefine_keys);

                retorno_menu=menu_dibuja_menu(&hardware_redefine_keys_opcion_seleccionada,&item_seleccionado,array_menu_hardware_redefine_keys,"Redefine keys" );

                


if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


void menu_hardware_keyboard_enter_return(MENU_ITEM_PARAMETERS)
{
    keyboard_swap_enter_return.v ^=1;
}


//menu keyboard settings
void menu_keyboard_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_keyboard_settings;
	menu_item item_seleccionado;
	int retorno_menu;
        do {

		menu_add_item_menu_inicial(&array_menu_keyboard_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);	




		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_en_es_ca(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_keyboard_issue,
                menu_hardware_keyboard_issue_cond,"Keyboard ~~Issue","Teclado ~~Issue","Teclat ~~Issue");
            menu_add_item_menu_prefijo_format(array_menu_keyboard_settings,"[%c] ", (keyboard_issue2.v==1 ? '2' : '3'));
			menu_add_item_menu_shortcut(array_menu_keyboard_settings,'i');
			menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Type of Spectrum keyboard emulated");
			menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Changes the way the Spectrum keyboard port returns its value: Issue 3 returns bit 6 off, and Issue 2 has bit 6 on");
		}


		//Soporte para Azerty keyboard

		if (!strcmp(scr_new_driver_name,"xwindows")) {
			menu_add_item_menu_en_es_ca(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_azerty,NULL,
                "~~Azerty keyboard","Teclado ~~Azerty","Teclat ~~Azerty");
            menu_add_item_menu_prefijo_format(array_menu_keyboard_settings,"[%c] ",(azerty_keyboard.v ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_keyboard_settings,'a');
			menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Enables azerty keyboard");
			menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Only used on xwindows driver by now. Enables to use numeric keys on Azerty keyboard, without having "
						"to press Shift. Note we are referring to the numeric keys (up to letter A, Z, etc) and not to the numeric keypad.");
		}


		menu_add_item_menu_en_es_ca(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_recreated_keyboard,NULL,
            "ZX Recreated support","Soporta ZX Recreated","Suporta ZX Recreated");
		menu_add_item_menu_prefijo_format(array_menu_keyboard_settings,"[%c] ",(recreated_zx_keyboard_support.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Enables ZX Recreated support. Press F1 to see details");
		menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Enables ZX Recreated support. You have to consider the following:\n"
						"- It supports Game Mode/Layer A on ZX Recreated. QWERTY mode/Layer B works like a normal keyboard\n"
						"- You must use the ZX Recreated only on the machine emulated, not on the menu\n"
						"- You must use your normal PC keyboard on the menu\n"
						"- I can't distinguish between normal keyboard and ZX Recreated keyboard key press. "
						"So if you enable ZX Recreated support and press keys on your normal PC keyboard, out of the menu, will produce strange combination of keys. "
						"If you press keys on the ZX Recreated on the menu, will produce strange combination of keys too. "
						"On the other hand, if you have ZX Recreated support disabled, and press keys on the ZX Recreated, will produce strange combination of keys too.\n"
						"- If you use Mac OS X, you are probably using the Cocoa driver, so it will work only by enabling this setting\n"
						"- If you use Linux, you should use the SDL1 or SDL2 video driver, and also enable the SDL Raw keyboard setting. "
						"It won't work well using other video drivers (last row of keys will fail)\n"
						"- If you use Windows, you are probably using the SDL1 or SDL2 video driver, so same behaviour as Linux: you must also enable the SDL Raw keyboard setting"
		);

		menu_add_item_menu_en_es_ca(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_keyboard_enter_return,NULL,
            "Swap Enter - Return","Intercambiar Enter-Return","Intercanviar Enter-Return");
		menu_add_item_menu_prefijo_format(array_menu_keyboard_settings,"[%c] ",(keyboard_swap_enter_return.v ? 'X' : ' ') );
        menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Swap keys Enter and Return, used on CPC and PCW");
        menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Swap keys Enter and Return, used on CPC and PCW");

#ifdef COMPILE_SDL
		if (!strcmp(scr_new_driver_name,"sdl")) {
			menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_sdl_raw_read,NULL,"[%c] SDL Raw keyboard",
				(sdl_raw_keyboard_read.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Read the keyboard using raw mode. Needed for ZX Recreated to work");
				menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Read the keyboard using raw mode. Needed for ZX Recreated to work");
		}
#endif


		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_en_es_ca(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_chloe_keyboard,NULL,
                "Chloe Keyboard","Teclado Chloe","Teclat Chloe");
            menu_add_item_menu_prefijo_format(array_menu_keyboard_settings,"[%c] ",(chloe_keyboard.v ? 'X' : ' ') );
		}

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_en_es_ca(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_spectrum_keyboard_matrix_error,NULL,
                "Speccy keyb. ~~ghosting","Speccy tecl. ~~ghosting","Speccy tecl. ~~ghosting");
			menu_add_item_menu_prefijo_format(array_menu_keyboard_settings,"[%c] ",(keyboard_matrix_error.v ? 'X' : ' ') );                    
			menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Enables real keyboard emulation, even with the keyboard matrix error");
			menu_add_item_menu_shortcut(array_menu_keyboard_settings,'g');
			menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Enables real keyboard emulation, even with the keyboard matrix error.\n"
						"This is the error/feature that returns more key pressed than the real situation, for example, "
						"pressing keys ASQ, will return ASQW. Using a pc keyboard is difficult to test that effect, because "
						"that most of them can return more than two or three keys pressed at a time. But using the on-screen keyboard "
						"and also the Recreated Keyboard, you can test it");

		}

		if (MACHINE_IS_Z88 || MACHINE_IS_CPC || MACHINE_IS_PCW || chloe_keyboard.v || MACHINE_IS_SAM || MACHINE_IS_QL || MACHINE_IS_MSX || MACHINE_IS_SVI || MACHINE_IS_PCW)  {
			//keymap solo hace falta con xwindows y sdl. fbdev y cocoa siempre leen en raw como teclado english
			if (!strcmp(scr_new_driver_name,"xwindows")  || !strcmp(scr_new_driver_name,"sdl") ) {
				
				menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_keymap_z88_cpc,NULL,"K~~eymap [%s]",realmachine_keymap_strings_types[z88_cpc_keymap_type]);

				menu_add_item_menu_shortcut(array_menu_keyboard_settings,'e');
				menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Keyboard Layout");
				menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Used on Z88, CPC, Sam, QL, MSX, SVI and Chloe machines, needed to map symbol keys. "
						"You must indicate here which kind of physical keyboard you have. Your physical keyboard will "
						"be mapped always to the English keyboard on the emulated machine, to the absolute positions of the keys. "
						"You have two physical keyboard choices: Default (English) and Spanish"
						"\n"
						"Note: Seems Windows version need this setting (usually) to be set to Default (please don't ask me why)"						
						);
			}
		}


                menu_add_item_menu(array_menu_keyboard_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        	//Redefine keys
		menu_add_item_menu_en_es_ca(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_redefine_keys,NULL,
            "~~Redefine keys","~~Redefinir teclas","~~Redefinir tecles");
		menu_add_item_menu_shortcut(array_menu_keyboard_settings,'r');
		menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Redefine one key to another");
		menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Redefine one key to another");
        menu_add_item_menu_tiene_submenu(array_menu_keyboard_settings);





                menu_add_item_menu(array_menu_keyboard_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_ESC_item(array_menu_keyboard_settings);

                retorno_menu=menu_dibuja_menu(&keyboard_settings_opcion_seleccionada,&item_seleccionado,array_menu_keyboard_settings,"Keyboard Settings" );

                
		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
			//llamamos por valor de funcion
	                if (item_seleccionado.menu_funcion!=NULL) {
        	                //printf ("actuamos por funcion\n");
                	        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
	                }
		}

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}

void menu_hardware_settings_set_z88_clock(MENU_ITEM_PARAMETERS)
{
    z88_set_system_clock_to_z88();    

    menu_generic_message_splash("Set Z88 clock","OK. Clock synchronized");
}

void menu_hardware_dinamic_sd1(MENU_ITEM_PARAMETERS)
{
    dinamic_sd1.v ^=1;
}

//menu hardware settings
void menu_hardware_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_settings;
	menu_item item_seleccionado;
	int retorno_menu;
        do {

			menu_add_item_menu_inicial(&array_menu_hardware_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);





		if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081 || MACHINE_IS_SAM || MACHINE_IS_CPC || MACHINE_IS_MSX || MACHINE_IS_SVI || MACHINE_IS_PCW) {
            menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_joystick,NULL,
                "~~Joystick type","Tipo ~~Joystick","Tipus ~~Joystick");
			menu_add_item_menu_sufijo_format(array_menu_hardware_settings," [%s]",joystick_texto[joystick_emulation]);
			menu_add_item_menu_shortcut(array_menu_hardware_settings,'j');
        	        menu_add_item_menu_tooltip(array_menu_hardware_settings,"Decide which joystick type is emulated");
                	menu_add_item_menu_ayuda(array_menu_hardware_settings,"Joystick is emulated with:\n"
					"-A real joystick connected to an USB port\n"
					"-Cursor keys on the keyboard for the directions and Home key for fire"
			);
        }

        if (joystick_autofire_frequency==0) {
            menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_autofire,menu_hardware_autofire_cond,
                "[ ] Joystick ~~Autofire","[ ] Joystick ~~Autodisparo","[ ] Joystick ~~Autofoc");
        }
        else {
            menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_autofire,NULL,
            "Joystick ~~Autofire","Joystick ~~Autodisparo","Joystick ~~Autofoc");
            menu_add_item_menu_prefijo_format(array_menu_hardware_settings,"[%d Hz] ",50/joystick_autofire_frequency);
        }
        menu_add_item_menu_shortcut(array_menu_hardware_settings,'a');
        menu_add_item_menu_tooltip(array_menu_hardware_settings,"Frequency for the joystick autofire");
        menu_add_item_menu_ayuda(array_menu_hardware_settings,"Times per second (Hz) the joystick fire is auto-switched from pressed to not pressed and viceversa. "
                                        "Autofire can only be enabled on Kempston, Fuller, Zebra and Mikrogen; Sinclair, Cursor, and OPQA can not have "
                                        "autofire because this function can interfiere with the menu (it might think a key is pressed)");
        menu_add_item_menu_es_avanzado(array_menu_hardware_settings);

        menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_autoleftright,NULL,
            "Joystick AutoLeftRight","Joystick AutoIzqDer","Joystick AutoEsqDreta");
        menu_add_item_menu_prefijo_format(array_menu_hardware_settings,"[%d Hz] ",50/joystick_autoleftright_frequency);
        menu_add_item_menu_tooltip(array_menu_hardware_settings,"You have to define a F-key or a button to trigger the action: JoyLeftRight");
        menu_add_item_menu_ayuda(array_menu_hardware_settings,"You have to define a F-key or a button to trigger the action: JoyLeftRight");
        menu_add_item_menu_es_avanzado(array_menu_hardware_settings);
		
		


        //Aunque no todas las máquinas tienen joystick, es importante que esta opción siempre aparezca
        //pues al activar una tecla como fire (por ejemplo right shift) hace que esa tecla ya no se comporte como right shift, sino solo fire
        //esto en Z88 podria ser critico, pues no funcionaria el right shift y ademas el usuario no podria reasignar el fire a home y dejar 
        //right shift como right shift del Z88
        menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_joystick_fire_key,NULL,
            "Fire key","Tecla disparo","Tecla foc");
        menu_add_item_menu_prefijo_format(array_menu_hardware_settings,"[%s] ",joystick_defined_fire_texto[joystick_defined_key_fire]);
        menu_add_item_menu_tooltip(array_menu_hardware_settings,"Define which key triggers the fire function for the joystick");
        menu_add_item_menu_ayuda(array_menu_hardware_settings,"Define which key triggers the fire function for the joystick. "
            "Not all video drivers support reading all keys");



		if (MACHINE_IS_SPECTRUM) {

			if (gunstick_emulation==0) menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick,NULL,"[ ] ~~Lightgun");
			else menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick,NULL,"[%s] ~~Lightgun",gunstick_texto[gunstick_emulation]);
			menu_add_item_menu_shortcut(array_menu_hardware_settings,'l');
			menu_add_item_menu_tooltip(array_menu_hardware_settings,"Decide which kind of lightgun is emulated with the mouse");
			menu_add_item_menu_ayuda(array_menu_hardware_settings,"Lightgun emulation supports the following two models:\n\n"
					"Gunstick from MHT Ingenieros S.L: all types except AYChip\n\n"
					"Magnum Light Phaser (experimental): supported by AYChip type");
            menu_add_item_menu_es_avanzado(array_menu_hardware_settings);


			if (menu_hardware_gunstick_aychip_cond()) {
				menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick_range_x,NULL," X Range: %d",gunstick_range_x);
                menu_add_item_menu_es_avanzado(array_menu_hardware_settings);
				menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick_range_y,NULL," Y Range: %d",gunstick_range_y);
                menu_add_item_menu_es_avanzado(array_menu_hardware_settings);
				menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick_y_offset,NULL," Y Offset: %s%d",(gunstick_y_offset ? "-" : "" ), gunstick_y_offset);
                menu_add_item_menu_es_avanzado(array_menu_hardware_settings);
				menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick_solo_brillo,NULL," Detect only white bright: %s",(gunstick_solo_brillo ? "On" : "Off"));
                menu_add_item_menu_es_avanzado(array_menu_hardware_settings);
		}


			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_kempston_mouse,NULL,"[%c] Kempston Mou~~se emulation",(kempston_mouse_emulation.v==1 ? 'X' : ' '));

			menu_add_item_menu_shortcut(array_menu_hardware_settings,'s');

			if (kempston_mouse_emulation.v) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_kempston_mouse_sensibilidad,NULL,"[%2d] Mouse Sensitivity",kempston_mouse_factor_sensibilidad);
            menu_add_item_menu_es_avanzado(array_menu_hardware_settings);
			}

		}

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_datagear_dma,NULL,"[%c] Datagear DMA emulation",(datagear_dma_emulation.v==1 ? 'X' : ' '));
            menu_add_item_menu_es_avanzado(array_menu_hardware_settings);

            menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_dinamic_sd1,NULL,"[%c] Dinamic SD1 emulation",(dinamic_sd1.v ? 'X' : ' '));
            menu_add_item_menu_es_avanzado(array_menu_hardware_settings);
		}		


  		if (MACHINE_IS_TBBLUE) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_tbblue_fast_boot_mode,NULL,"[%c] Next fast boot mode",
			(tbblue_fast_boot_mode.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_hardware_settings,"Boots tbblue directly to a 48 rom but with all the Next features enabled (except divmmc)");
			menu_add_item_menu_ayuda(array_menu_hardware_settings,"Boots tbblue directly to a 48 rom but with all the Next features enabled (except divmmc)");
            menu_add_item_menu_es_avanzado(array_menu_hardware_settings);

			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_tbblue_machine_id,NULL,"[%d] Next machine id",tbblue_machine_id); 
            menu_add_item_menu_es_avanzado(array_menu_hardware_settings);

			//menu_hardware_tbblue_core_version
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_tbblue_core_version,NULL,"[%d.%d.%d] Next core version",
									tbblue_core_current_version_major,tbblue_core_current_version_minor,tbblue_core_current_version_subminor);
            menu_add_item_menu_es_avanzado(array_menu_hardware_settings);


			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_tbblue_rtc_traps,NULL,"[%c] Next RTC traps",(tbblue_use_rtc_traps ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_hardware_settings,"Allows RTC trap for NextOS ROM");
			menu_add_item_menu_ayuda(array_menu_hardware_settings,"Allows RTC trap for NextOS ROM and any program that uses RTC.SYS");
            menu_add_item_menu_es_avanzado(array_menu_hardware_settings);

		}

        
        menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_cpu_speed,NULL,
            "Emulator Spee~~d","Veloci~~dad Emulador","Velocitat Emula~~dor");
		menu_add_item_menu_prefijo_format(array_menu_hardware_settings,"[%3d%%] ",porcentaje_velocidad_emulador);
		menu_add_item_menu_shortcut(array_menu_hardware_settings,'d');
		menu_add_item_menu_tooltip(array_menu_hardware_settings,"Change the emulator Speed");
		menu_add_item_menu_ayuda(array_menu_hardware_settings,"Changes all the emulator speed by setting a different interval between display frames. "
		"Also changes audio frequency");		

		menu_add_item_menu(array_menu_hardware_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		//Keyboard settings
		menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_keyboard_settings,NULL,
            "~~Keyboard settings","Opciones te~~klado","Opcions te~~klat");
		menu_add_item_menu_shortcut(array_menu_hardware_settings,'k');
		menu_add_item_menu_tooltip(array_menu_hardware_settings,"Hardware settings");
		menu_add_item_menu_ayuda(array_menu_hardware_settings,"Hardware settings");
        menu_add_item_menu_tiene_submenu(array_menu_hardware_settings);






		menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_memory_settings,NULL,
            "~~Memory Settings","Opciones ~~Memoria","Opcions ~~Memoria");
		menu_add_item_menu_shortcut(array_menu_hardware_settings,'m');
        menu_add_item_menu_tiene_submenu(array_menu_hardware_settings);



		

        if (MACHINE_IS_Z88) {
            menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_settings_set_z88_clock,NULL,
                "Sync Z88 clock","Sincronizar reloj Z88","Sincronitzar rellotge Z88");
            menu_add_item_menu_tooltip(array_menu_hardware_settings,"Sync Z88 clock to the current time");
            menu_add_item_menu_ayuda(array_menu_hardware_settings,"Sync Z88 clock to the current time");
            menu_add_item_menu_es_avanzado(array_menu_hardware_settings);
        }



		if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX81_TYPE) {
			menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_printers,NULL,
                "~~Printing emulation","Emulación im~~presora","Emulació im~~pressora");
			menu_add_item_menu_shortcut(array_menu_hardware_settings,'p');
            menu_add_item_menu_tiene_submenu(array_menu_hardware_settings);
		}

        //Aqui permito que el menu aparezca en cualquier maquina, aunque en algunas no este implementado (por ejemplo QL)
	    
	
        menu_add_item_menu_en_es_ca(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_realjoystick,menu_hardware_realjoystick_cond,
            "~~Real joystick settings","Opciones joystick ~~real","Opcions joystick ~~real");
        menu_add_item_menu_shortcut(array_menu_hardware_settings,'r');
        menu_add_item_menu_tooltip(array_menu_hardware_settings,"Settings for the real joystick");
        menu_add_item_menu_ayuda(array_menu_hardware_settings,"Settings for the real joystick");
        menu_add_item_menu_tiene_submenu(array_menu_hardware_settings);

		

		// De momento esto desactivado
        /*
		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_if1_settings,NULL,"Interface 1: %s",(if1_enabled.v ? "Yes" : "No") );
		}
        */
		




        menu_add_item_menu(array_menu_hardware_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_ESC_item(array_menu_hardware_settings);

        retorno_menu=menu_dibuja_menu(&hardware_settings_opcion_seleccionada,&item_seleccionado,array_menu_hardware_settings,"Hardware Settings" );

                
		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
			//llamamos por valor de funcion
	                if (item_seleccionado.menu_funcion!=NULL) {
        	                //printf ("actuamos por funcion\n");
                	        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
	                }
		}

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}

void menu_storage_settings_fast_autoload(MENU_ITEM_PARAMETERS)
{
	fast_autoload.v ^=1;
}


void menu_tape_autoloadtape(MENU_ITEM_PARAMETERS)
{
        noautoload.v ^=1;
}

void menu_tape_autoselectfileopt(MENU_ITEM_PARAMETERS)
{
        autoselect_snaptape_options.v ^=1;
}


//menu storage settings
void menu_settings_storage(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_settings_storage;
    menu_item item_seleccionado;
    int retorno_menu;
    do {


        menu_add_item_menu_en_es_ca_inicial(&array_menu_settings_storage,MENU_OPCION_NORMAL,menu_tape_autoloadtape,NULL,
            "~~Autoload medium","~~Autocargar medio","~~Autocarregar medi");
        menu_add_item_menu_prefijo_format(array_menu_settings_storage,"[%c] ", (noautoload.v==0 ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_settings_storage,'a');
        menu_add_item_menu_tooltip(array_menu_settings_storage,"Autoload medium and set machine");
        menu_add_item_menu_ayuda(array_menu_settings_storage,"This option first change to the machine that handles the medium file type selected (tape, cartridge, etc), resets it, set some default machine values, and then, it sends "
            "a LOAD sentence to load the medium\n"
            "Note: The machine is changed only using smartload. Inserting a medium only resets the machine but does not change it");


        if (noautoload.v==0) {
            menu_add_item_menu_en_es_ca(array_menu_settings_storage,MENU_OPCION_NORMAL,menu_storage_settings_fast_autoload,NULL,
                "Fast autoloa~~d","Autocargar rápi~~do","Autocarregar ràpi~~d");
            menu_add_item_menu_prefijo_format(array_menu_settings_storage,"[%c] ",(fast_autoload.v ? 'X' : ' ' ) );                
            menu_add_item_menu_shortcut(array_menu_settings_storage,'d');
            menu_add_item_menu_tooltip(array_menu_settings_storage,"Do the autoload process at top speed");
            menu_add_item_menu_ayuda(array_menu_settings_storage,"Do the autoload process at top speed");
        }


        menu_add_item_menu_en_es_ca(array_menu_settings_storage,MENU_OPCION_NORMAL,menu_tape_autoselectfileopt,NULL,
            "A~~utoselect medium opts","A~~utoseleccionar opcs. medio","A~~utoseleccionar opcs. medi");
        menu_add_item_menu_prefijo_format(array_menu_settings_storage,"[%c] ", (autoselect_snaptape_options.v==1 ? 'X' : ' ' ));
        menu_add_item_menu_shortcut(array_menu_settings_storage,'u');
        menu_add_item_menu_tooltip(array_menu_settings_storage,"Detect options for the selected medium file and the needed machine");
        menu_add_item_menu_ayuda(array_menu_settings_storage,"The emulator uses a database for different included programs "
            "(and some other not included) and reads .config files to select emulator settings and the needed machine "
            "to run them. If you disable this, the database nor the .config files are read");


        if (!MACHINE_IS_Z88 && !MACHINE_IS_CHLOE && !MACHINE_IS_QL) {
            menu_add_item_menu(array_menu_settings_storage,"",MENU_OPCION_SEPARADOR,NULL,NULL);

            menu_add_item_menu_en_es_ca(array_menu_settings_storage,MENU_OPCION_NORMAL,menu_settings_tape,NULL,
                "~~Tape","Cin~~ta","Cin~~ta");
            menu_add_item_menu_shortcut(array_menu_settings_storage,'t');
            menu_add_item_menu_tiene_submenu(array_menu_settings_storage);
        }


																			
					
        menu_add_item_menu(array_menu_settings_storage,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        
        menu_add_ESC_item(array_menu_settings_storage);

        retorno_menu=menu_dibuja_menu(&settings_storage_opcion_seleccionada,&item_seleccionado,array_menu_settings_storage,"Storage Settings" );

        
        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                        
                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



void menu_display_snow_effect(MENU_ITEM_PARAMETERS)
{
	snow_effect_enabled.v ^=1;
}


void menu_display_inves_ula_bright_error(MENU_ITEM_PARAMETERS)
{
	inves_ula_bright_error.v ^=1;
}


void menu_display_slow_adjust(MENU_ITEM_PARAMETERS)
{
	video_zx8081_lnctr_adjust.v ^=1;
}



void menu_display_estabilizador_imagen(MENU_ITEM_PARAMETERS)
{
	video_zx8081_estabilizador_imagen.v ^=1;
}

void menu_display_interlace(MENU_ITEM_PARAMETERS)
{
	if (video_interlaced_mode.v) disable_interlace();
	else enable_interlace();
}


void menu_display_interlace_scanlines(MENU_ITEM_PARAMETERS)
{
	if (video_interlaced_scanlines.v) disable_scanlines();
	else enable_scanlines();
}

void menu_display_gigascreen(MENU_ITEM_PARAMETERS)
{
        if (gigascreen_enabled.v) disable_gigascreen();
        else enable_gigascreen();
}

void menu_display_chroma81(MENU_ITEM_PARAMETERS)
{
        if (chroma81.v) disable_chroma81();
        else enable_chroma81();
}

void menu_display_ulaplus(MENU_ITEM_PARAMETERS)
{
        if (ulaplus_presente.v) disable_ulaplus();
        else enable_ulaplus();
}


void menu_display_autodetect_chroma81(MENU_ITEM_PARAMETERS)
{
	autodetect_chroma81.v ^=1;
}


void menu_display_spectra(MENU_ITEM_PARAMETERS)
{
	if (spectra_enabled.v) spectra_disable();
	else spectra_enable();
}

void menu_display_snow_effect_margin(MENU_ITEM_PARAMETERS)
{
	snow_effect_min_value++;
	if (snow_effect_min_value==8) snow_effect_min_value=1;
}

void menu_display_timex_video(MENU_ITEM_PARAMETERS)
{
	if (timex_video_emulation.v) disable_timex_video();
	else enable_timex_video();
}

void menu_display_minimo_vsync(MENU_ITEM_PARAMETERS)
{

        menu_hardware_advanced_input_value(100,999,"Minimum vsync length",&minimo_duracion_vsync);
}

void menu_display_timex_video_512192(MENU_ITEM_PARAMETERS)
{

	timex_mode_512192_real.v ^=1;
}

void menu_display_cpc_force_mode(MENU_ITEM_PARAMETERS)
{
	if (cpc_forzar_modo_video.v==0) {
		cpc_forzar_modo_video.v=1;
		cpc_forzar_modo_video_modo=0;
	}
	else {
		cpc_forzar_modo_video_modo++;
		if (cpc_forzar_modo_video_modo==4) {
			cpc_forzar_modo_video_modo=0;
			cpc_forzar_modo_video.v=0;
		}
	}
}

void menu_display_refresca_sin_colores(MENU_ITEM_PARAMETERS)
{
	scr_refresca_sin_colores.v ^=1;
	modificado_border.v=1;
}


void menu_display_timex_force_line_512192(MENU_ITEM_PARAMETERS)
{
	if (timex_ugly_hack_last_hires==0) timex_ugly_hack_last_hires=198;

	        char string_num[4];

        sprintf (string_num,"%d",timex_ugly_hack_last_hires);

        menu_ventana_scanf("Scanline",string_num,4);

        timex_ugly_hack_last_hires=parse_string_to_number(string_num);
}

void menu_display_timex_ugly_hack(MENU_ITEM_PARAMETERS)
{
	timex_ugly_hack_enabled ^=1;
}

void menu_spritechip(MENU_ITEM_PARAMETERS)
{
	if (spritechip_enabled.v) spritechip_disable();
	else spritechip_enable();
}


void menu_display_emulate_fast_zx8081(MENU_ITEM_PARAMETERS)
{
	video_fast_mode_emulation.v ^=1;
	modificado_border.v=1;
}



void menu_display_emulate_zx8081display_spec(MENU_ITEM_PARAMETERS)
{
	if (simulate_screen_zx8081.v==1) simulate_screen_zx8081.v=0;
	else {
		simulate_screen_zx8081.v=1;
		umbral_simulate_screen_zx8081=4;
	}
	modificado_border.v=1;
}


void menu_display_osd_word_kb_length(MENU_ITEM_PARAMETERS)
{
	menu_ventana_scanf_numero_enhanced("Length? (10-100)",&adventure_keyboard_key_length,4,+10,10,100,0);

/*
	char string_length[4];

        sprintf (string_length,"%d",adventure_keyboard_key_length);

        //menu_ventana_scanf("Length? (10-100)",string_length,4);

		int ret=menu_ventana_scanf_numero("Length? (10-100)",string_length,4,+10,10,100,0);

		if (ret<0) return;

        int valor=parse_string_to_number(string_length);
	if (valor<10 || valor>100) {
		debug_printf (VERBOSE_ERR,"Invalid value");
	}

	else {
		adventure_keyboard_key_length=valor;
	}
	*/

}


void menu_display_osd_word_kb_finalspc(MENU_ITEM_PARAMETERS)
{
	adventure_keyboard_send_final_spc ^=1;
}


void menu_display_emulate_zx8081_thres(MENU_ITEM_PARAMETERS)
{

/*
        char string_thres[3];

        sprintf (string_thres,"%d",umbral_simulate_screen_zx8081);

        menu_ventana_scanf("Pixel Threshold",string_thres,3);

	umbral_simulate_screen_zx8081=parse_string_to_number(string_thres);
	if (umbral_simulate_screen_zx8081<1 || umbral_simulate_screen_zx8081>16) umbral_simulate_screen_zx8081=4;
*/

	menu_ventana_scanf_numero_enhanced("Pixel Threshold",&umbral_simulate_screen_zx8081,3,+1,1,16,0);


}


int menu_display_settings_disp_zx8081_spectrum(void)
{

	//esto solo en spectrum y si el driver no es curses y si no hay rainbow
	if (!strcmp(scr_new_driver_name,"curses")) return 0;
	if (rainbow_enabled.v==1) return 0;

	return !menu_cond_zx8081();
}


void menu_display_arttext(MENU_ITEM_PARAMETERS)
{
	texto_artistico.v ^=1;
}



#ifdef COMPILE_AA
void menu_display_slowaa(MENU_ITEM_PARAMETERS)
{
	scraa_fast ^=1;
}
#else
void menu_display_slowaa(MENU_ITEM_PARAMETERS){}
#endif



void menu_display_zx8081_wrx(MENU_ITEM_PARAMETERS)
{
	if (wrx_present.v) {
		disable_wrx();
	}

	else {
		enable_wrx();
	}
	//wrx_present.v ^=1;
}





void menu_display_x_offset(MENU_ITEM_PARAMETERS)
{

	//offset_zx8081_t_coordx +=8;
    //    if (offset_zx8081_t_coordx>=30*8) offset_zx8081_t_coordx=-30*8;

	menu_ventana_scanf_numero_enhanced("X offset",&offset_zx8081_t_coordx,5,+8,-30*8,30*8,1);

}


int menu_display_emulate_zx8081_cond(void)
{
	return simulate_screen_zx8081.v;
}


void menu_display_autodetect_rainbow(MENU_ITEM_PARAMETERS)
{
	autodetect_rainbow.v ^=1;
}

void menu_display_autodetect_wrx(MENU_ITEM_PARAMETERS)
{
        autodetect_wrx.v ^=1;
}

int menu_display_aa_cond(void)
{
        if (!strcmp(scr_new_driver_name,"aa")) return 1;

        else return 0;
}


void menu_display_tsconf_vdac(MENU_ITEM_PARAMETERS)
{
	tsconf_vdac_with_pwm.v ^=1;

	menu_interface_rgb_inverse_common();
}

void menu_display_tsconf_pal_depth(MENU_ITEM_PARAMETERS)
{
	tsconf_palette_depth--;
	if (tsconf_palette_depth<2) tsconf_palette_depth=5;

	menu_interface_rgb_inverse_common();

}

void menu_display_rainbow(MENU_ITEM_PARAMETERS)
{
	if (rainbow_enabled.v==0) enable_rainbow();
	else disable_rainbow();


}

void menu_vofile_insert(MENU_ITEM_PARAMETERS)
{

        if (vofile_inserted.v==0) {
                init_vofile();
                //Si todo ha ido bien
                if (vofile_inserted.v) {
                        menu_generic_message_format("File information","%s\n%s\n\n%s",
												last_message_helper_aofile_vofile_file_format,last_message_helper_aofile_vofile_bytes_minute_video,last_message_helper_aofile_vofile_util);
                }

        }

        else if (vofile_inserted.v==1) {
                close_vofile();
        }

}


int menu_vofile_cond(void)
{
        if (vofilename!=NULL) return 1;
        else return 0;
}

void menu_vofile(MENU_ITEM_PARAMETERS)
{

        vofile_inserted.v=0;


        char *filtros[2];

        filtros[0]="rwv";
        filtros[1]=0;


        if (menu_filesel("Select Video File",filtros,vofilename_file)==1) {

                 //Ver si archivo existe y preguntar
                struct stat buf_stat;

                if (stat(vofilename_file, &buf_stat)==0) {

                        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) {
                                vofilename=NULL;
                                return;
                        }

                }



                vofilename=vofilename_file;
        }

        else {
                vofilename=NULL;
        }


}

void menu_vofile_fps(MENU_ITEM_PARAMETERS)
{
	if (vofile_fps==1) {
		vofile_fps=50;
		return;
	}

        if (vofile_fps==2) {
                vofile_fps=1;
                return;
        }


        if (vofile_fps==5) {
                vofile_fps=2;
                return;
        }

        if (vofile_fps==10) {
                vofile_fps=5;
                return;
        }

        if (vofile_fps==25) {
                vofile_fps=10;
                return;
        }


        if (vofile_fps==50) {
                vofile_fps=25;
                return;
        }

}

int menu_display_curses_cond(void)
{
	if (!strcmp(scr_new_driver_name,"curses")) return 1;

	else return 0;
}


int menu_display_cursesstdout_cond(void)
{
	if (menu_display_curses_cond() ) return 1;
	if (menu_cond_stdout() ) return 1;

	return 0;
}



int menu_display_cursesstdoutsimpletext_cond(void)
{
	if (menu_display_cursesstdout_cond() ) return 1;
	if (menu_cond_simpletext() ) return 1;

	return 0;
}



int menu_display_arttext_cond(void)
{

	if (!menu_display_cursesstdout_cond()) return 0;

	//en zx80 y 81 no hay umbral, no tiene sentido. ahora si. hay rainbow de zx8081
	//if (machine_type>=20 && machine_type<=21) return 0;
	if (use_scrcursesw.v) return 1;
	if (texto_artistico.v) return 1;

    return 0;
}

int menu_cond_stdout_simpletext(void)
{
	if (menu_cond_stdout() || menu_cond_simpletext() ) return 1;
	return 0;
}


//En curses y stdout solo se permite para zx8081
int menu_cond_realvideo_curses_stdout_zx8081(void)
{
	if (menu_cond_stdout() || menu_cond_curses() ) {
		if (MACHINE_IS_SPECTRUM ) return 0;
	}

	return 1;
}


void menu_display_stdout_simpletext_automatic_redraw(MENU_ITEM_PARAMETERS)
{
	stdout_simpletext_automatic_redraw.v ^=1;
}



void menu_display_send_ansi(MENU_ITEM_PARAMETERS)
{
	screen_text_accept_ansi ^=1;
}

void menu_display_arttext_thres(MENU_ITEM_PARAMETERS)
{

        char string_thres[3];

        sprintf (string_thres,"%d",umbral_arttext);

        menu_ventana_scanf("Pixel Threshold",string_thres,3);

        umbral_arttext=parse_string_to_number(string_thres);
        if (umbral_arttext<1 || umbral_arttext>16) umbral_arttext=4;

}


void menu_display_text_brightness(MENU_ITEM_PARAMETERS)
{

        char string_bri[4];

        sprintf (string_bri,"%d",screen_text_brightness);

        menu_ventana_scanf("Brightness? (0-100)",string_bri,4);

	int valor=parse_string_to_number(string_bri);
	if (valor<0 || valor>100) debug_printf (VERBOSE_ERR,"Invalid brightness value %d",valor);

	else screen_text_brightness=valor;

}






void menu_display_stdout_simpletext_fps(MENU_ITEM_PARAMETERS)
{
	    char string_fps[3];

        sprintf (string_fps,"%d",50/scrstdout_simpletext_refresh_factor);

        menu_ventana_scanf("FPS? (1-50)",string_fps,3);

        int valor=parse_string_to_number(string_fps);
		scr_set_fps_stdout_simpletext(valor);

}


void menu_display_ocr_23606(MENU_ITEM_PARAMETERS)
{

ocr_settings_not_look_23606.v ^=1;

}

void menu_display_text_all_refresh_pixel(MENU_ITEM_PARAMETERS)
{

screen_text_all_refresh_pixel.v ^=1;

}

void menu_display_text_all_refresh_pixel_invert(MENU_ITEM_PARAMETERS)
{

screen_text_all_refresh_pixel_invert.v ^=1;

}


void menu_display_text_all_refresh_pixel_scale(MENU_ITEM_PARAMETERS)
{
// screen_text_all_refresh_pixel_scale
char string_bri[3];

        sprintf (string_bri,"%d",screen_text_all_refresh_pixel_scale);

        menu_ventana_scanf("Scale? (1-99)",string_bri,3);

	int valor=parse_string_to_number(string_bri);
	if (valor<1 || valor>99) debug_printf (VERBOSE_ERR,"Invalid scale value %d",valor);

	else screen_text_all_refresh_pixel_scale=valor;
	
}

void menu_display_text_all_refresh_pixel_max_ancho(MENU_ITEM_PARAMETERS)
{
    char string_bri[5];

    sprintf (string_bri,"%d",scr_refresca_pantalla_tsconf_text_max_ancho);

    menu_ventana_scanf("Max width? (1-9999)",string_bri,5);

	int valor=parse_string_to_number(string_bri);
	if (valor<1 || valor>9999) debug_printf (VERBOSE_ERR,"Invalid max width value %d",valor);

	else scr_refresca_pantalla_tsconf_text_max_ancho=valor;    
}          

void menu_display_text_all_refresh_pixel_offset_x(MENU_ITEM_PARAMETERS)
{
    char string_bri[5];

    sprintf (string_bri,"%d",scr_refresca_pantalla_tsconf_text_offset_x);

    menu_ventana_scanf("X-Offset? (0-9999)",string_bri,5);

	int valor=parse_string_to_number(string_bri);
	if (valor<0 || valor>9999) debug_printf (VERBOSE_ERR,"Invalid X-Offset value %d",valor);

	else scr_refresca_pantalla_tsconf_text_offset_x=valor;    
}       


void menu_display_text_all_refresh_pixel_max_alto(MENU_ITEM_PARAMETERS)
{
    char string_bri[5];

    sprintf (string_bri,"%d",scr_refresca_pantalla_tsconf_text_max_alto);

    menu_ventana_scanf("Max height? (1-9999)",string_bri,5);

	int valor=parse_string_to_number(string_bri);
	if (valor<1 || valor>9999) debug_printf (VERBOSE_ERR,"Invalid max height value %d",valor);

	else scr_refresca_pantalla_tsconf_text_max_alto=valor;    
}          

void menu_display_text_all_refresh_pixel_offset_y(MENU_ITEM_PARAMETERS)
{
    char string_bri[5];

    sprintf (string_bri,"%d",scr_refresca_pantalla_tsconf_text_offset_y);

    menu_ventana_scanf("Y-Offset? (0-9999)",string_bri,5);

	int valor=parse_string_to_number(string_bri);
	if (valor<0 || valor>9999) debug_printf (VERBOSE_ERR,"Invalid Y-Offset value %d",valor);

	else scr_refresca_pantalla_tsconf_text_offset_y=valor;    
}   


#ifdef COMPILE_CURSESW
void menu_display_cursesw_ext(MENU_ITEM_PARAMETERS)
{
	use_scrcursesw.v ^=1;

	if (use_scrcursesw.v) {
		//Reiniciar locale
		cursesw_ext_init();
	}
}
#endif
						

void menu_textdrivers_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_textdrivers_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

		char buffer_string[50];


		//Como no sabemos cual sera el item inicial, metemos este sin asignar, que se sobreescribe en el siguiente menu_add_item_menu
		//menu_add_item_menu_inicial(&array_menu_textdrivers_settings,"---Text Driver Settings--",MENU_OPCION_UNASSIGNED,NULL,NULL);
		menu_add_item_menu_inicial(&array_menu_textdrivers_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);


                //para stdout y simpletext
                if (menu_cond_stdout_simpletext() ) {
                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_stdout_simpletext_automatic_redraw,NULL,"[%c]   Stdout automatic redraw", (stdout_simpletext_automatic_redraw.v==1 ? 'X' : ' ') );
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"It enables automatic display redraw");
                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"It enables automatic display redraw");


                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_send_ansi,NULL,"[%c]   Send ANSI Ctrl Sequence",(screen_text_accept_ansi==1 ? 'X' : ' ') );

						if (stdout_simpletext_automatic_redraw.v) {
							menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_stdout_simpletext_fps,NULL,"[%2d]  Redraw fps", 50/scrstdout_simpletext_refresh_factor);
						}

                }

		if (menu_display_cursesstdout_cond() ) {
                        //solo en caso de curses o stdout

						
                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_arttext,menu_display_cursesstdout_cond,"[%c]   Text artistic emulation", (texto_artistico.v==1 ? 'X' : ' ') );
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Write different artistic characters for unknown 4x4 rectangles, "
                                        "on stdout and curses drivers");

                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Write different artistic characters for unknown 4x4 rectangles, "
                                        "on curses, stdout and simpletext drivers. "
                                        "If disabled, unknown characters are written with ?");

#ifdef COMPILE_CURSESW
						menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_cursesw_ext,NULL,"[%c]   Extended utf blocky", (use_scrcursesw.v ? 'X' : ' ') );
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Use extended utf characters to have 64x48 display, only for curses driver and machines: Spectrum (realvideo enabled or not) and ZX80/81 (only for realvideo enabled)");
						menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Use extended utf characters to have 64x48 display, only for curses driver and machines: Spectrum (realvideo enabled or not) and ZX80/81 (only for realvideo enabled)");
#endif
								


                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_arttext_thres,menu_display_arttext_cond,"[%2d]  Pixel threshold",umbral_arttext);
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Pixel Threshold to decide which artistic character write in a 4x4 rectangle, "
                                        "on curses, stdout and simpletext drivers with text artistic emulation or utf enabled");
                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Pixel Threshold to decide which artistic character write in a 4x4 rectangle, "
                                        "on curses, stdout and simpletext drivers with text artistic emulation or utf enabled");
                                        
                                        





                        if (rainbow_enabled.v) {
                                menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_text_brightness,NULL,"[%3d] Text brightness",screen_text_brightness);
                                menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Text brightness used on some machines and text drivers, like tsconf");
                                menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Text brightness used on some machines and text drivers, like tsconf");
                                
                                menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_text_all_refresh_pixel,NULL,"[%c] All pixel to text",(screen_text_all_refresh_pixel.v ? 'X' : ' ' ));
                                
                                if (screen_text_all_refresh_pixel.v) {
                                
                                menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_text_all_refresh_pixel_scale,NULL,"[1:%d] Scale",screen_text_all_refresh_pixel_scale );
                                
                                menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_text_all_refresh_pixel_invert,NULL,"[%c] Invert text",(screen_text_all_refresh_pixel_invert.v ? 'X' : ' ' ));

                                menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_text_all_refresh_pixel_max_ancho,NULL,"[%d] Max width (in chars)",scr_refresca_pantalla_tsconf_text_max_ancho);
                                
                                menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_text_all_refresh_pixel_offset_x,NULL,"[%d] X-Offset (in chars)",scr_refresca_pantalla_tsconf_text_offset_x);

                                menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_text_all_refresh_pixel_max_alto,NULL,"[%d] Max height (in chars)",scr_refresca_pantalla_tsconf_text_max_alto);
                                
                                menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_text_all_refresh_pixel_offset_y,NULL,"[%d] Y-Offset (in chars)",scr_refresca_pantalla_tsconf_text_offset_y);


                                }
                                
                                
                                
                                
                                
                                
                                
                                
                        }
                        
                        
                        
                        

                }






if (menu_display_aa_cond() ) {

#ifdef COMPILE_AA
                        sprintf (buffer_string,"Slow AAlib emulation: %s", (scraa_fast==0 ? "On" : "Off"));
#else
                        sprintf (buffer_string,"Slow AAlib emulation: Off");
#endif
                        menu_add_item_menu(array_menu_textdrivers_settings,buffer_string,MENU_OPCION_NORMAL,menu_display_slowaa,menu_display_aa_cond);

                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Enable slow aalib emulation; slow is a little better");
                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Enable slow aalib emulation; slow is a little better");

                }



                menu_add_item_menu(array_menu_textdrivers_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_textdrivers_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_textdrivers_settings);

                retorno_menu=menu_dibuja_menu(&textdrivers_settings_opcion_seleccionada,&item_seleccionado,array_menu_textdrivers_settings,"Text Driver Settings" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



//void menu_display_cpc_double_vsync(MENU_ITEM_PARAMETERS)
//{
//	cpc_send_double_vsync.v ^=1;
//}



void menu_display_16c_mode(MENU_ITEM_PARAMETERS)
{
    if (pentagon_16c_mode_available.v) disable_16c_mode();
    else enable_16c_mode();
}

void menu_display_tbblue_store_scanlines(MENU_ITEM_PARAMETERS)
{
	tbblue_store_scanlines.v ^=1;
}

void menu_display_tbblue_store_scanlines_border(MENU_ITEM_PARAMETERS)
{
	tbblue_store_scanlines_border.v ^=1;
}

void menu_display_vdp_9918a_unlimited_sprites_line(MENU_ITEM_PARAMETERS)
{
	vdp_9918a_unlimited_sprites_line.v ^=1;
}

void menu_display_sms_disable_raster_interrupt(MENU_ITEM_PARAMETERS)
{
	sms_disable_raster_interrupt.v ^=1;
}

void menu_display_sms_only_one_raster_int_frame(MENU_ITEM_PARAMETERS)
{
    sms_only_one_raster_int_frame.v ^=1;
}

void menu_display_msx_loading_stripes(MENU_ITEM_PARAMETERS)
{
	msx_loading_stripes.v ^=1;
}

/*
void menu_display_ql_simular_parpadeo(MENU_ITEM_PARAMETERS)
{
	ql_simular_parpadeo_cursor.v ^=1;
}
*/

void menu_display_tbblue_optimized_sprite_render(MENU_ITEM_PARAMETERS)
{
    tbblue_disable_optimized_sprites.v ^=1;
}

//void menu_display_cpc_end_frame_workaround(MENU_ITEM_PARAMETERS)
//{
//    cpc_endframe_workaround.v ^=1;
//}

void menu_display_sms_wonderboy_scroll_hack(MENU_ITEM_PARAMETERS)
{
    sms_wonderboy_scroll_hack.v ^=1;
}

void menu_interface_rgb_inverse_common(void)
{
	modificado_border.v=1;
	screen_init_colour_table();

        //Dado que se han cambiado la paleta de colores, hay que vaciar la putpixel cache
        clear_putpixel_cache();

	menu_init_footer();
}

void menu_interface_red(MENU_ITEM_PARAMETERS)
{
	screen_gray_mode ^= 4;
	menu_interface_rgb_inverse_common();
}

void menu_interface_green(MENU_ITEM_PARAMETERS)
{
        screen_gray_mode ^= 2;
	menu_interface_rgb_inverse_common();
}

void menu_interface_blue(MENU_ITEM_PARAMETERS)
{
        screen_gray_mode ^= 1;
	menu_interface_rgb_inverse_common();
}

void menu_interface_inverse_video(MENU_ITEM_PARAMETERS)
{
        inverse_video.v ^= 1;
	menu_interface_rgb_inverse_common();
}

void menu_interface_real_1648_palette(MENU_ITEM_PARAMETERS)
{
	spectrum_1648_use_real_palette.v ^=1;
	//screen_set_spectrum_palette_offset();
	menu_interface_rgb_inverse_common();
}

void menu_colour_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_colour_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {



		menu_add_item_menu_inicial_format(&array_menu_colour_settings,MENU_OPCION_NORMAL,menu_interface_red,NULL,"[%c] ~~Red display",(screen_gray_mode & 4 ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_colour_settings,'r');

		menu_add_item_menu_format(array_menu_colour_settings,MENU_OPCION_NORMAL,menu_interface_green,NULL,"[%c] ~~Green display",(screen_gray_mode & 2 ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_colour_settings,'g');
		
		menu_add_item_menu_format(array_menu_colour_settings,MENU_OPCION_NORMAL,menu_interface_blue,NULL,"[%c] ~~Blue display",(screen_gray_mode & 1 ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_colour_settings,'b');

		menu_add_item_menu_format(array_menu_colour_settings,MENU_OPCION_NORMAL,menu_interface_inverse_video,NULL,"[%c] ~~Inverse colours",(inverse_video.v==1 ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_colour_settings,'i');
		menu_add_item_menu_tooltip(array_menu_colour_settings,"Inverse Color Palette");
		menu_add_item_menu_ayuda(array_menu_colour_settings,"Inverses all the colours used on the emulator, including menu");


		if (MACHINE_IS_SPECTRUM_16 || MACHINE_IS_SPECTRUM_48) {
			menu_add_item_menu_format(array_menu_colour_settings,MENU_OPCION_NORMAL,menu_interface_real_1648_palette,NULL,"[%c] R~~eal palette",(spectrum_1648_use_real_palette.v ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_colour_settings,'e');
			menu_add_item_menu_tooltip(array_menu_colour_settings,"Use real Spectrum 16/48/+ colour palette");
			menu_add_item_menu_ayuda(array_menu_colour_settings,"Use real Spectrum 16/48/+ colour palette. "
				"In fact, this palette is the same as a Spectrum issue 3, and almost the same as issue 1 and 2");
		}

        menu_add_item_menu(array_menu_colour_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_ESC_item(array_menu_colour_settings);

                retorno_menu=menu_dibuja_menu(&colour_settings_opcion_seleccionada,&item_seleccionado,array_menu_colour_settings,"Colour Settings" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

void menu_settings_display_z88_shortcuts(MENU_ITEM_PARAMETERS)
{
    z88_hide_keys_shortcuts.v ^=1;

    screen_z88_draw_lower_screen();
}

void menu_display_pcw_black_white(MENU_ITEM_PARAMETERS)
{
    pcw_black_white_display.v ^=1;
}

void menu_display_pcw_always_on(MENU_ITEM_PARAMETERS)
{
    pcw_always_on_display.v ^=1;
}

void menu_display_pcw_do_not_inverse(MENU_ITEM_PARAMETERS)
{
    pcw_do_not_inverse_display.v ^=1;
}

void menu_display_pcw_do_not_scroll(MENU_ITEM_PARAMETERS)
{
    pcw_do_not_scroll.v ^=1;
}

//menu display settings
void menu_settings_display(MENU_ITEM_PARAMETERS)
{

	menu_item *array_menu_settings_display;
	menu_item item_seleccionado;
	int retorno_menu;
	do {

		//hotkeys usadas: ricglwmptez


		char string_vofile_shown[10];
		menu_tape_settings_trunc_name(vofilename,string_vofile_shown,10);


		menu_add_item_menu_inicial_format(&array_menu_settings_display,MENU_OPCION_NORMAL,menu_vofile,NULL,"Video out to file:");
        menu_add_item_menu_spanish_catalan(array_menu_settings_display,"Salida video a archivo:","Sortida video a arxiu:");
        menu_add_item_menu_sufijo_format(array_menu_settings_display," %s",string_vofile_shown);
		menu_add_item_menu_tooltip(array_menu_settings_display,"Saves the video output to a file");
		menu_add_item_menu_ayuda(array_menu_settings_display,"The generated file have raw format. You can see the file parameters "
			"on the console enabling verbose debug level to 2 minimum.\n"
			"A watermark is added to the final video, as you may see when you activate it\n"
			"Note: Gigascreen, Interlaced effects or menu windows are not saved to file."
		);

		if (menu_vofile_cond() ) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_vofile_fps,menu_vofile_cond,"[%d] FPS Video file",50/vofile_fps);
			menu_add_item_menu_en_es_ca(array_menu_settings_display,MENU_OPCION_NORMAL,menu_vofile_insert,menu_vofile_cond,
                "Video file enabled","Archivo video activado","Arxiu video activat");
            menu_add_item_menu_prefijo_format(array_menu_settings_display,"[%c] ",(vofile_inserted.v ? 'X' : ' ' ));
		}

		else {
					menu_add_item_menu(array_menu_settings_display,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		}



		if (!MACHINE_IS_Z88) {


				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_autodetect_rainbow,NULL,"[%c] Autodetect Real Video",(autodetect_rainbow.v==1 ? 'X' : ' '));
				menu_add_item_menu_tooltip(array_menu_settings_display,"Autodetect the need to enable Real Video");
				menu_add_item_menu_ayuda(array_menu_settings_display,"This option detects whenever is needed to enable Real Video. "
								"On Spectrum, it detects the reading of idle bus or repeated border changes. "
								"On ZX80/81, it detects the I register on a non-normal value when executing video display. "
			"On all machines, it also detects when loading a real tape. "
								);
		}




		menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_rainbow,menu_display_rainbow_cond,"[%c] ~~Real Video",(rainbow_enabled.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_display,'r');

		menu_add_item_menu_tooltip(array_menu_settings_display,"Enable Real Video. Enabling it makes display as a real machine");
		menu_add_item_menu_ayuda(array_menu_settings_display,"Real Video makes display works as in the real machine. It uses a bit more CPU than disabling it.\n\n"
				"On Spectrum, display is drawn every scanline. "
				"It enables hi-res colour (rainbow) on the screen and on the border, Gigascreen, Interlaced, ULAplus, Spectra, Timex Video, snow effect, idle bus reading and some other advanced features. "
				"Also enables all the Inves effects.\n"
				"Disabling it, the screen is drawn once per frame (1/50) and the previous effects "
				"are not supported.\n\n"
				"On ZX80/ZX81, enables hi-res display and loading/saving stripes on the screen, and the screen is drawn every scanline.\n"
				"By disabling it, the screen is drawn once per frame, no hi-res display, and only text mode is supported.\n\n"
				"On Z88, display is drawn the same way as disabling it; it is only used when enabling Video out to file.\n\n"
				"Real Video can be enabled on all the video drivers, but on curses, stdout and simpletext (in Spectrum and Z88 machines), the display drawn is the same "
				"as on non-Real Video, but you can have idle bus support on these drivers. "
				"Curses, stdout and simpletext drivers on ZX80/81 machines do have Real Video display."
				);

		if (MACHINE_IS_TBBLUE && rainbow_enabled.v) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_tbblue_store_scanlines,NULL,"[%c] Legacy hi-color effects",(tbblue_store_scanlines.v ? 'X' : ' '));	
			menu_add_item_menu_tooltip(array_menu_settings_display,"Allow legacy hi-color effects on pixel/attribute display zone on TBBlue");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Allows you to make hi-res effects on pixel/attribute display zone on TBBlue, like overscan demo for example. "
										"It is not needed for Spectrum Next games, but needed for Timex 8x1 mode. Disabling it reduces cpu usage");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_tbblue_store_scanlines_border,NULL,"[%c] Legacy border effects",(tbblue_store_scanlines_border.v ? 'X' : ' '));	
			menu_add_item_menu_tooltip(array_menu_settings_display,"Allow legacy border effects on TBBlue");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Allows you to make hi-res effects on border zone on TBBlue, like overscan demo or load/save border stripes for example. "
										"It is not needed for Spectrum Next games. Disabling it reduces cpu usage");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);
			/*
			Benchmark of this: compiled without optimization O2, with:
			./configure --enable-memptr --enable-visualmem --enable-cpustats --enable-ssl 
			on Mac OS X 10.15.4 iMac late 2013 2,9 GHz Intel Core i5 4 cores

			*cpu turbo x1: overscan demo.    OFF hi-color OFF border: 50 % cpu 50 FPS
			 cpu turbo x1: overscan demo.    ON  hi-color OFF border: 52 % cpu 50 FPS
			 cpu turbo x1: overscan demo.    ON  hi-color ON  border: 55 % cpu 50 FPS

			*cpu turbo x1: 48k basic prompt. OFF hi-color OFF border: 42 % cpu 50 FPS
			 cpu turbo x1: 48k basic prompt. ON  hi-color OFF border: 44 % cpu 50 FPS
			 cpu turbo x1: 48k basic prompt. ON  hi-color ON  border: 45 % cpu 50 FPS

			*cpu turbo x4: overscan demo.    OFF hi-color OFF border: 84 % cpu 50 FPS
			 cpu turbo x4: overscan demo.    ON  hi-color OFF border: 78 % cpu 43 FPS
			 cpu turbo x4: overscan demo.    ON  hi-color ON  border: 71 % cpu 35 FPS

			*cpu turbo x4: 48k basic prompt. OFF hi-color OFF border: 73 % cpu 50 FPS
			 cpu turbo x4: 48k basic prompt. ON  hi-color OFF border: 80 % cpu 50 FPS
			 cpu turbo x4: 48k basic prompt. ON  hi-color ON  border: 86 % cpu 50 FPS


			*cpu turbo x4: Poogie. OFF hi-color OFF border: 83 % cpu 50 FPS
			 cpu turbo x4: Poogie. ON hi-color  OFF border: 70 % cpu 38 FPS
			 cpu turbo x4: Poogie. ON hi-color  ON  border: 69 % cpu 33 FPS


			*/

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_tbblue_optimized_sprite_render,NULL,"[%c] Optimized render sprite",(tbblue_disable_optimized_sprites.v==0 ? 'X' : ' '));	
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enable optimized sprite rendering. Usually you don't need to disable this");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Optimized render walks on the sprite list until the last visible sprite. "
                                    "Besides, non-optimized rendering walk on the whole sprite list all the time, no matter the last visible sprite. "
                                    "Usually don't want to disable this optimization");    
            menu_add_item_menu_es_avanzado(array_menu_settings_display);        

		}

		if (MACHINE_IS_TSCONF) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_tsconf_vdac,NULL,"[%c] TSConf VDAC PWM",
				(tsconf_vdac_with_pwm.v ? 'X' : ' ')     );

			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables full vdac colour palette or PWM style");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Full vdac colour palette gives you different colour levels for every 5 bit colour component.\n"
					"With PWM mode it gives you 5 bit values different from 0..23, but from 24 to 31 are all set to value 255");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_tsconf_pal_depth,NULL,
					 "[%d] TSConf palette depth",tsconf_palette_depth);
            menu_add_item_menu_es_avanzado(array_menu_settings_display);




        }


		if (MACHINE_IS_CPC) {
				if (cpc_forzar_modo_video.v==0)
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_cpc_force_mode,NULL,"[ ] Force Video Mode");
				else
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_cpc_force_mode,NULL,"[%d] Force Video Mode",
						cpc_forzar_modo_video_modo);
                menu_add_item_menu_es_avanzado(array_menu_settings_display);
				//menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_cpc_double_vsync,NULL,"[%c] Double Vsync",(cpc_send_double_vsync.v==1 ? 'X' : ' ') );
				//menu_add_item_menu_tooltip(array_menu_settings_display,"Workaround to avoid hang on some games");
				//menu_add_item_menu_ayuda(array_menu_settings_display,"Workaround to avoid hang on some games");

                //menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_cpc_end_frame_workaround,NULL,"[%c] End frame workaround",(cpc_endframe_workaround.v==1 ? 'X' : ' ') );


		}


		if (!MACHINE_IS_Z88) {


			if (menu_cond_realvideo() ) {
                if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081) {
                    menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_interlace,menu_cond_realvideo,"[%c] ~~Interlaced mode", (video_interlaced_mode.v==1 ? 'X' : ' '));
                    menu_add_item_menu_shortcut(array_menu_settings_display,'i');
                    menu_add_item_menu_tooltip(array_menu_settings_display,"Enable interlaced mode");
                    menu_add_item_menu_ayuda(array_menu_settings_display,"Interlaced mode draws the screen like the machine on a real TV: "
                        "Every odd frame, odd lines on TV are drawn; every even frame, even lines on TV are drawn. It can be used "
                        "to emulate twice the vertical resolution of the machine (384) or simulate different colours. "
                        "This effect is only emulated with vertical zoom multiple of two: 2,4,6... etc");
                    menu_add_item_menu_es_avanzado(array_menu_settings_display);
                }


				if (video_interlaced_mode.v) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_interlace_scanlines,NULL,"[%c] S~~canlines", (video_interlaced_scanlines.v==1 ? 'X' : ' '));
					menu_add_item_menu_shortcut(array_menu_settings_display,'c');
					menu_add_item_menu_tooltip(array_menu_settings_display,"Enable scanlines on interlaced mode");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Scanlines draws odd lines a bit darker than even lines");
                    menu_add_item_menu_es_avanzado(array_menu_settings_display);
				}


				if (MACHINE_IS_SPECTRUM) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_gigascreen,NULL,"[%c] ~~Gigascreen",(gigascreen_enabled.v==1 ? 'X' : ' '));
					menu_add_item_menu_shortcut(array_menu_settings_display,'g');
					menu_add_item_menu_tooltip(array_menu_settings_display,"Enable gigascreen colours");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Gigascreen enables more than 15 colours by combining pixels "
							"of even and odd frames. The total number of different colours is 102");
                    menu_add_item_menu_es_avanzado(array_menu_settings_display);
				}



				if (MACHINE_IS_SPECTRUM && !MACHINE_IS_ZXEVO && !MACHINE_IS_TBBLUE)  {

					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_snow_effect,NULL,"[%c] Snow effect support", (snow_effect_enabled.v==1 ? 'X' : ' '));
					menu_add_item_menu_tooltip(array_menu_settings_display,"Enable snow effect on Spectrum");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Snow effect is a bug on some Spectrum models "
						"(models except +2A and +3) that draws corrupted pixels when I register is pointed to "
						"slow RAM.");
						// Even on 48k models it resets the machine after some seconds drawing corrupted pixels");
                    menu_add_item_menu_es_avanzado(array_menu_settings_display);

					if (snow_effect_enabled.v==1) {
						menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_snow_effect_margin,NULL,"[%d] Snow effect threshold",snow_effect_min_value);
                        menu_add_item_menu_es_avanzado(array_menu_settings_display);
					}
				}


				if (MACHINE_IS_INVES) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_inves_ula_bright_error,NULL,"[%c] Inves bright error",(inves_ula_bright_error.v ? 'X' : ' '));
					menu_add_item_menu_tooltip(array_menu_settings_display,"Emulate Inves oddity when black colour and change from bright 0 to bright 1");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Emulate Inves oddity when black colour and change from bright 0 to bright 1. Seems it only happens with RF or RGB connection");
                    menu_add_item_menu_es_avanzado(array_menu_settings_display);

				}

			}

            if (MACHINE_IS_PCW) {
                menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_pcw_black_white,NULL,
                    "[%c] Black & White monitor",(pcw_black_white_display.v ? 'X' : ' '));

                menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_pcw_always_on,NULL,
                    "[%c] Always on monitor",(pcw_always_on_display.v ? 'X' : ' '));

                menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_pcw_do_not_inverse,NULL,
                    "[%c] Do not allow inverse",(pcw_do_not_inverse_display.v ? 'X' : ' '));

                menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_pcw_do_not_scroll,NULL,
                    "[%c] Do not allow scroll",(pcw_do_not_scroll.v ? 'X' : ' '));
            }
		}

		//para stdout

		/*
#ifdef COMPILE_STDOUT
		if (menu_cond_stdout() ) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_stdout_simpletext_automatic_redraw,NULL,"Stdout automatic redraw: %s", (stdout_simpletext_automatic_redraw.v==1 ? "On" : "Off"));
			menu_add_item_menu_tooltip(array_menu_settings_display,"It enables automatic display redraw");
			menu_add_item_menu_ayuda(array_menu_settings_display,"It enables automatic display redraw");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_send_ansi,NULL,"Send ANSI Control Sequence: %s",(screen_text_accept_ansi==1 ? "On" : "Off"));

		}

#endif

		*/


		if (menu_cond_zx8081_realvideo()) {

		//z80_bit video_zx8081_estabilizador_imagen;

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_estabilizador_imagen,menu_cond_zx8081_realvideo,"[%c] Horizontal stabilization", (video_zx8081_estabilizador_imagen.v==1 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Horizontal image stabilization");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Horizontal image stabilization. Usually enabled.");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_slow_adjust,menu_cond_zx8081_realvideo,"[%c] ~~LNCTR video adjust", (video_zx8081_lnctr_adjust.v==1 ? 'X' : ' '));
			//l repetida con load screen, pero como esa es de spectrum, no coinciden
			menu_add_item_menu_shortcut(array_menu_settings_display,'l');
			menu_add_item_menu_tooltip(array_menu_settings_display,"LNCTR video adjust");
			menu_add_item_menu_ayuda(array_menu_settings_display,"LNCTR video adjust change sprite offset when drawing video images. "
				"If you see your hi-res image is not displayed well, try changing it");
            




			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_x_offset,menu_cond_zx8081_realvideo,"[%d] Video x_offset",offset_zx8081_t_coordx);
			menu_add_item_menu_tooltip(array_menu_settings_display,"Video horizontal image offset");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Video horizontal image offset, usually you don't need to change this");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_minimo_vsync,menu_cond_zx8081_realvideo,"[%d] Video min. vsync length",minimo_duracion_vsync);
			menu_add_item_menu_tooltip(array_menu_settings_display,"Video minimum vsync length in t-states");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Video minimum vsync length in t-states");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_autodetect_wrx,NULL,"[%c] Autodetect WRX",(autodetect_wrx.v==1 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Autodetect the need to enable WRX mode on ZX80/81");
			menu_add_item_menu_ayuda(array_menu_settings_display,"This option detects whenever is needed to enable WRX. "
									"On ZX80/81, it detects the I register on a non-normal value when executing video display. "
			"In some cases, chr$128 and udg modes are detected incorrectly as WRX");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_zx8081_wrx,menu_cond_zx8081_realvideo,"[%c] ~~WRX", (wrx_present.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'w');
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables WRX hi-res mode");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Enables WRX hi-res mode");



		}

		else {

			if (menu_cond_zx8081() ) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_emulate_fast_zx8081,menu_cond_zx8081_no_realvideo,"[%c] ZX80/81 detect fast mode", (video_fast_mode_emulation.v==1 ? 'X' : ' '));
				menu_add_item_menu_tooltip(array_menu_settings_display,"Detect fast mode and simulate it, on non-realvideo mode");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Detect fast mode and simulate it, on non-realvideo mode");
                menu_add_item_menu_es_avanzado(array_menu_settings_display);
			}

		}

		if (MACHINE_IS_ZX8081) {

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_autodetect_chroma81,NULL,"[%c] Autodetect Chroma81",(autodetect_chroma81.v ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Autodetect Chroma81");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Detects when Chroma81 video mode is needed and enable it");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_chroma81,NULL,"[%c] Chro~~ma81 support",(chroma81.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'m');
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables Chroma81 colour video mode");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Enables Chroma81 colour video mode");

		}


		if (MACHINE_IS_SPECTRUM && !MACHINE_IS_TBBLUE) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_ulaplus,NULL,"[%c] ULA~~plus support",(ulaplus_presente.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'p');
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables ULAplus support");
			menu_add_item_menu_ayuda(array_menu_settings_display,"The following ULAplus modes are supported:\n"
						"Mode 1: Standard 256x192 64 colours\n"
						"Mode 3: Linear mode 128x96, 16 colours per pixel (radastan mode)\n"
						"Mode 5: Linear mode 256x96, 16 colours per pixel (ZEsarUX mode 0)\n"
						"Mode 7: Linear mode 128x192, 16 colours per pixel (ZEsarUX mode 1)\n"
						"Mode 9: Linear mode 256x192, 16 colours per pixel (ZEsarUX mode 2)\n"
			);
            menu_add_item_menu_es_avanzado(array_menu_settings_display);
		}

		if (MACHINE_IS_PENTAGON) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_16c_mode,NULL,"[%c] 16C mode support",(pentagon_16c_mode_available.v ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables 16C video mode support");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Enables 16C video mode support. That brings you mode 256x192x16 colour on Pentagon");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);
		}

		if (MACHINE_IS_SPECTRUM) {


			if (!MACHINE_IS_PENTAGON) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_video,NULL,"[%c] ~~Timex video support",(timex_video_emulation.v ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_settings_display,'t');
				menu_add_item_menu_tooltip(array_menu_settings_display,"Enables Timex Video modes");
				menu_add_item_menu_ayuda(array_menu_settings_display,"The following Timex Video modes are emulated:\n"
				"Mode 0: Video data at address 16384 and 8x8 color attributes at address 22528 (like on ordinary Spectrum)\n"
				"Mode 1: Video data at address 24576 and 8x8 color attributes at address 30720\n"
				"Mode 2: Multicolor mode: video data at address 16384 and 8x1 color attributes at address 24576\n"
				"Mode 6: Hi-res mode 512x192, monochrome.");
                menu_add_item_menu_es_avanzado(array_menu_settings_display);

				if (timex_video_emulation.v && !MACHINE_IS_TBBLUE) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_video_512192,NULL,"[%c] Timex Real 512x192",(timex_mode_512192_real.v ? 'X' : ' '));
					menu_add_item_menu_tooltip(array_menu_settings_display,"Selects between real 512x192 or scaled 256x192");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Real 512x192 does not support scanline effects (it draws the display at once). "
								"If not enabled real, it draws scaled 256x192 but does support scanline effects");
                    menu_add_item_menu_es_avanzado(array_menu_settings_display);



					if (timex_mode_512192_real.v==0) {

						menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_ugly_hack,NULL,"[%c] Ugly hack",(timex_ugly_hack_enabled ? 'X' : ' ') );
						menu_add_item_menu_tooltip(array_menu_settings_display,"EXPERIMENTAL feature");
						menu_add_item_menu_ayuda(array_menu_settings_display,"EXPERIMENTAL feature");
                        menu_add_item_menu_es_avanzado(array_menu_settings_display);

						if (timex_ugly_hack_enabled) {
						menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_force_line_512192,NULL,"[%d] Force 512x192 at",timex_ugly_hack_last_hires);
						menu_add_item_menu_tooltip(array_menu_settings_display,"EXPERIMENTAL feature");
						menu_add_item_menu_ayuda(array_menu_settings_display,"EXPERIMENTAL feature");
                        menu_add_item_menu_es_avanzado(array_menu_settings_display);
						}
					}

				}
			}


			if (!MACHINE_IS_ZXEVO) {

				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_spectra,NULL,"[%c] Sp~~ectra support",(spectra_enabled.v ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_settings_display,'e');
				menu_add_item_menu_tooltip(array_menu_settings_display,"Enables Spectra video modes");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Enables Spectra video modes. All video modes are fully emulated");
                menu_add_item_menu_es_avanzado(array_menu_settings_display);


				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_spritechip,NULL,"[%c] ~~ZGX Sprite Chip",(spritechip_enabled.v ? 'X' : ' ') );
				menu_add_item_menu_shortcut(array_menu_settings_display,'z');
				menu_add_item_menu_tooltip(array_menu_settings_display,"Enables ZGX Sprite Chip");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Enables ZGX Sprite Chip");
                menu_add_item_menu_es_avanzado(array_menu_settings_display);
			}


		}




		if (MACHINE_IS_SPECTRUM && rainbow_enabled.v==0) {
			menu_add_item_menu(array_menu_settings_display,"",MENU_OPCION_SEPARADOR,NULL,NULL);


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_emulate_zx8081display_spec,menu_display_settings_disp_zx8081_spectrum,"[%c] ZX80/81 Display on Speccy", (simulate_screen_zx8081.v==1 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Simulates the resolution of ZX80/81 on the Spectrum");
			menu_add_item_menu_ayuda(array_menu_settings_display,"It makes the resolution of display on Spectrum like a ZX80/81, with no colour. "
					"This mode is not supported with real video enabled");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);


			if (menu_display_emulate_zx8081_cond() ){
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_emulate_zx8081_thres,menu_display_emulate_zx8081_cond,"[%d] Pixel threshold",umbral_simulate_screen_zx8081);
				menu_add_item_menu_tooltip(array_menu_settings_display,"Pixel Threshold to draw black or white in a 4x4 rectangle, "
						"when ZX80/81 Display on Speccy enabled");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Pixel Threshold to draw black or white in a 4x4 rectangle, "
						"when ZX80/81 Display on Speccy enabled");
                menu_add_item_menu_es_avanzado(array_menu_settings_display);
			}


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_refresca_sin_colores,NULL,"[%c] Colours enabled",(scr_refresca_sin_colores.v==0 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Disables colours for Spectrum display");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Disables colours for Spectrum display");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);



		}



		if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081 || MACHINE_IS_CPC) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_osd_word_kb_length,NULL,"[%d] OSD Adventure KB length",adventure_keyboard_key_length);
			menu_add_item_menu_tooltip(array_menu_settings_display,"Define the duration for every key press on the Adventure Text OSD Keyboard");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Define the duration for every key press on the Adventure Text OSD Keyboard, in 1/50 seconds (default 50)");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_osd_word_kb_finalspc,NULL,"[%c] OSD Adv. final space",
				(adventure_keyboard_send_final_spc ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Sends a space after every word on the Adventure Text OSD Keyboard");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Sends a space after every word on the Adventure Text OSD Keyboard");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);
			

		}

		if (MACHINE_HAS_VDP_9918A) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_vdp_9918a_unlimited_sprites_line,NULL,"[%c] Unlimited sprites per line", (vdp_9918a_unlimited_sprites_line.v ? 'X' : ' ') );	
            menu_add_item_menu_es_avanzado(array_menu_settings_display);
		}

        if (MACHINE_IS_SMS) {
            //Corrige tambien el cuelgue del space harrier al iniciar
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_sms_disable_raster_interrupt,NULL,"[%c] Disable raster interrupt", (sms_disable_raster_interrupt.v ? 'X' : ' ') );	
            menu_add_item_menu_es_avanzado(array_menu_settings_display);

            if (sms_disable_raster_interrupt.v==0) {
                menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_sms_only_one_raster_int_frame,NULL,
                    "[%c] One interrupt / frame", (sms_only_one_raster_int_frame.v ? 'X' : ' ') );	
                menu_add_item_menu_es_avanzado(array_menu_settings_display);
            }

            //wonder boy, astro flash
            //Esto corrige tambien el cuelgue del space harrier al iniciar
            menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_sms_wonderboy_scroll_hack,NULL,"[%c] Fix some scrolls",
                (sms_wonderboy_scroll_hack.v ? 'X' : ' ') );	
            menu_add_item_menu_tooltip(array_menu_settings_display,"Fix scroll in some games, like Astro Flash or Wonder Boy in Monster World");
            menu_add_item_menu_ayuda(array_menu_settings_display,"Fix scroll in some games, like Astro Flash or Wonder Boy in Monster World");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);
		}

		if (MACHINE_IS_MSX) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_msx_loading_stripes,NULL,"[%c] Loading stripes", (msx_loading_stripes.v ? 'X' : ' ') );	
			menu_add_item_menu_tooltip(array_menu_settings_display,"Simulates loading border stripes when loading from real tape");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Simulates loading border stripes when loading from real tape");
            menu_add_item_menu_es_avanzado(array_menu_settings_display);
		}		

        /*
		if (MACHINE_IS_QL) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_ql_simular_parpadeo,NULL,"[%c] QL cursor flashing", (ql_simular_parpadeo_cursor.v ? 'X' : ' ') );	
			menu_add_item_menu_tooltip(array_menu_settings_display,"Simulates QL cursor flashing");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Simulates QL cursor flashing");
		}
        */

        if (MACHINE_IS_Z88 && si_complete_video_driver() ) {
            menu_add_item_menu_en_es_ca(array_menu_settings_display,MENU_OPCION_NORMAL,menu_settings_display_z88_shortcuts,NULL,
            "Show Z88 shortcuts","Mostrar atajos del Z88","Veure dreceres del Z88");
            menu_add_item_menu_prefijo_format(array_menu_settings_display,"[%c] ", (z88_hide_keys_shortcuts.v==0 ? 'X' : ' ') );
            menu_add_item_menu_tooltip(array_menu_settings_display,"Show Z88 shortcuts below the display");
            menu_add_item_menu_ayuda(array_menu_settings_display,"Show Z88 shortcuts below the display");
        }

		menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_ocr_23606,NULL,"[%c] OCR Alternate chars", (ocr_settings_not_look_23606.v==0 ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_display,"Tells to look for an alternate character set other than the ROM default on OCR functions");
		menu_add_item_menu_ayuda(array_menu_settings_display,"Tells to look for an alternate character set other than the ROM default on OCR functions. "
							"It will look also for another character set which table is set on sysvar 23606/7. It may generate false positives "
							"on some games. It's used on text drivers (curses, stdout, simpletext) but also on OCR function");
        menu_add_item_menu_es_avanzado(array_menu_settings_display);


		if (menu_display_cursesstdoutsimpletext_cond() || menu_display_aa_cond() ) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_textdrivers_settings,NULL,"Text driver settings");
		}

		
        menu_add_item_menu(array_menu_settings_display,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_item_menu_en_es_ca(array_menu_settings_display,MENU_OPCION_NORMAL,menu_colour_settings,NULL,
            "Colour settings","Opciones de colores","Opcions de colors");
        menu_add_item_menu_tiene_submenu(array_menu_settings_display);
		//menu_add_item_menu_shortcut(array_menu_settings_display,'c');		


		menu_add_item_menu(array_menu_settings_display,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		//menu_add_item_menu(array_menu_settings_display,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings_display);

		retorno_menu=menu_dibuja_menu(&settings_display_opcion_seleccionada,&item_seleccionado,array_menu_settings_display,"Display Settings" );

                

		//NOTA: no llamar por numero de opcion dado que hay opciones que ocultamos (relacionadas con real video)

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

			//llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


void menu_textspeech_filter_program(MENU_ITEM_PARAMETERS)
{

	char *filtros[2];

        filtros[0]="";
        filtros[1]=0;

/*
	char string_program[PATH_MAX];
	if (textspeech_filter_program!=NULL) {
		sprintf (string_program,"%s",textspeech_filter_program);
	}

	else {
		string_program[0]=0;
	}



	int ret=menu_filesel("Select Speech Program",filtros,string_program);


	if (ret==1) {
		sprintf (menu_buffer_textspeech_filter_program,"%s",string_program);
		textspeech_filter_program=menu_buffer_textspeech_filter_program;
		textspeech_filter_program_check_spaces();
	}

	else {
		textspeech_filter_program=NULL;
	}
*/

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de speech program
        //si no hay directorio, vamos a rutas predefinidas
        if (textspeech_filter_program==NULL) menu_chdir_sharedfiles();
        else {
                char directorio[PATH_MAX];
                util_get_dir(textspeech_filter_program,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        zvfs_chdir(directorio);
                }
        }

        int ret;

        ret=menu_filesel("Select Speech Program",filtros,menu_buffer_textspeech_filter_program);
        //volvemos a directorio inicial
        zvfs_chdir(directorio_actual);


        if (ret==1) {

                textspeech_filter_program=menu_buffer_textspeech_filter_program;
					textspeech_filter_program_check_spaces();
			}

		else {
			textspeech_filter_program=NULL;
        }



}

void menu_textspeech_stop_filter_program(MENU_ITEM_PARAMETERS)
{

        char *filtros[2];

        filtros[0]="";
        filtros[1]=0;


/*
        char string_program[PATH_MAX];
        if (textspeech_stop_filter_program!=NULL) {
                sprintf (string_program,"%s",textspeech_stop_filter_program);
        }

        else {
                string_program[0]=0;
        }



        int ret=menu_filesel("Select Stop Speech Prg",filtros,string_program);


        if (ret==1) {
                sprintf (menu_buffer_textspeech_stop_filter_program,"%s",string_program);
                textspeech_stop_filter_program=menu_buffer_textspeech_stop_filter_program;
                textspeech_stop_filter_program_check_spaces();
        }

        else {
                textspeech_stop_filter_program=NULL;
        }
*/

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de speech program
        //si no hay directorio, vamos a rutas predefinidas
        if (textspeech_stop_filter_program==NULL) menu_chdir_sharedfiles();
        else {
                char directorio[PATH_MAX];
                util_get_dir(textspeech_stop_filter_program,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        zvfs_chdir(directorio);
                }
        }

        int ret;

        ret=menu_filesel("Select Stop Speech Prg",filtros,menu_buffer_textspeech_stop_filter_program);
        //volvemos a directorio inicial
        zvfs_chdir(directorio_actual);


        if (ret==1) {

                textspeech_stop_filter_program=menu_buffer_textspeech_stop_filter_program;
                                        textspeech_stop_filter_program_check_spaces();
                        }

                else {
                        textspeech_stop_filter_program=NULL;
        }




}

void menu_textspeech_filter_timeout(MENU_ITEM_PARAMETERS)
{

       int valor;

        char string_value[3];

        sprintf (string_value,"%d",textspeech_timeout_no_enter);


        menu_ventana_scanf("Timeout (0=never)",string_value,3);

        valor=parse_string_to_number(string_value);

	if (valor<0) debug_printf (VERBOSE_ERR,"Timeout must be 0 minimum");

	else textspeech_timeout_no_enter=valor;


}

void menu_textspeech_program_wait(MENU_ITEM_PARAMETERS)
{
	textspeech_filter_program_wait.v ^=1;
}

void menu_textspeech_send_menu(MENU_ITEM_PARAMETERS)
{
        textspeech_also_send_menu.v ^=1;
}

void menu_chardetection_settings_stdout_line_witdh_space(MENU_ITEM_PARAMETERS)
{
        chardetect_line_width_wait_space.v ^=1;
}

void menu_chardetection_settings_stdout_line_witdh_dot(MENU_ITEM_PARAMETERS)
{
        chardetect_line_width_wait_dot.v ^=1;
}

void menu_chardetection_settings_stdout_line_width(MENU_ITEM_PARAMETERS)
{

        char string_width[3];

        int width;


        sprintf (string_width,"%d",chardetect_line_width);

        menu_ventana_scanf("Line width 0=no limit",string_width,3);

        width=parse_string_to_number(string_width);

        //if (width>999) {
        //        debug_printf (VERBOSE_ERR,"Invalid width %d",width);
        //        return;
        //}
        chardetect_line_width=width;

}


void menu_chardetection_settings_send_consolewindow(MENU_ITEM_PARAMETERS)
{
    textspeech_get_stdout.v ^=1;
}


#ifdef COMPILE_STDOUT
void menu_display_stdout_send_speech_debug(MENU_ITEM_PARAMETERS)
{
	scrstdout_also_send_speech_debug_messages.v ^=1;
}
#endif


void menu_textspeech(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_textspeech;
        menu_item item_seleccionado;
        int retorno_menu;

        do {



                char string_filterprogram_shown[14];
		char string_stop_filterprogram_shown[14];

		if (textspeech_filter_program!=NULL) {
	                menu_tape_settings_trunc_name(textspeech_filter_program,string_filterprogram_shown,14);
		}

		else {
		sprintf (string_filterprogram_shown,"None");
		}



                if (textspeech_stop_filter_program!=NULL) {
                        menu_tape_settings_trunc_name(textspeech_stop_filter_program,string_stop_filterprogram_shown,14);
                }

                else {
                sprintf (string_stop_filterprogram_shown,"None");
                }


                        menu_add_item_menu_inicial_format(&array_menu_textspeech,MENU_OPCION_NORMAL,menu_textspeech_filter_program,NULL,"~~Speech program [%s]",string_filterprogram_shown);
			menu_add_item_menu_shortcut(array_menu_textspeech,'s');
        	        menu_add_item_menu_tooltip(array_menu_textspeech,"Specify which program to send generated text");
        	        menu_add_item_menu_ayuda(array_menu_textspeech,"Specify which program to send generated text. Text is send to the program "
						"to its standard input on Unix versions (Linux, Mac, etc) or sent as the first parameter on "
						"Windows (MINGW) version\n"
                        "There are some script examples on speech_filters folder to run text-to-speech programs but also translation programs\n"
						"Pressing a key on the menu (or ESC with menu closed) forces the following queded text entries to flush, and running the "
						"Stop Program to stop the current text script.\n");


			if (textspeech_filter_program!=NULL) {

				menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_textspeech_stop_filter_program,NULL,"Stop program [%s]",string_stop_filterprogram_shown);

        	                menu_add_item_menu_tooltip(array_menu_textspeech,"Specify a path to a program or script in charge of stopping the running speech program");
                	        menu_add_item_menu_ayuda(array_menu_textspeech,"Specify a path to a program or script in charge of stopping the running speech program. If not specified, the current speech script can't be stopped");


				menu_add_item_menu(array_menu_textspeech,"",MENU_OPCION_SEPARADOR,NULL,NULL);




				menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_textspeech_program_wait,NULL,"[%c] ~~Wait program to exit",(textspeech_filter_program_wait.v ? 'X' : ' ' ) );
				menu_add_item_menu_shortcut(array_menu_textspeech,'w');
                	        menu_add_item_menu_tooltip(array_menu_textspeech,"Wait and pause the emulator until the Speech program returns");
                        	menu_add_item_menu_ayuda(array_menu_textspeech,"Wait and pause the emulator until the Speech program returns");


				menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_textspeech_send_menu,NULL,"[%c] Also send ~~menu",(textspeech_also_send_menu.v ? 'X' : ' ' ));
				menu_add_item_menu_shortcut(array_menu_textspeech,'m');
				menu_add_item_menu_tooltip(array_menu_textspeech,"Also send text menu entries to Speech program");
				menu_add_item_menu_ayuda(array_menu_textspeech,"Also send text menu entries to Speech program");

#ifdef COMPILE_STDOUT
				if (menu_cond_stdout() ) {
							menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_display_stdout_send_speech_debug,NULL,"[%c] Also send debug messages", (scrstdout_also_send_speech_debug_messages.v==1 ? 'X' : ' '));
							menu_add_item_menu_tooltip(array_menu_textspeech,"Also send debug messages to speech");
							menu_add_item_menu_ayuda(array_menu_textspeech,"Also send debug messages to speech");

				}

#endif

			

            if (chardetect_printchar_enabled.v) {
                menu_add_item_menu(array_menu_textspeech,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,NULL,NULL,"--Trap print to speech--");
                menu_add_item_menu_tooltip(array_menu_textspeech,"Settings for texto coming from trap print and sent to speech");
                menu_add_item_menu_ayuda(array_menu_textspeech,"Settings for texto coming from trap print and sent to speech");

                    menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_chardetection_settings_stdout_line_width,NULL,"[%d] Line w~~idth",chardetect_line_width);
                menu_add_item_menu_shortcut(array_menu_textspeech,'i');
                menu_add_item_menu_tooltip(array_menu_textspeech,"The minimum characters to detect as a line text");
                menu_add_item_menu_ayuda(array_menu_textspeech,"The minimum characters to detect as a line text. Setting 0 means no limit, so "
                            "even when a carriage return is received, the text will not be sent unless a Enter "
                            "key is pressed or when timeout no enter is reached\n");


                if (chardetect_line_width!=0) {
                    menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_chardetection_settings_stdout_line_witdh_space,NULL,"[%c] End line with s~~pace",(chardetect_line_width_wait_space.v==1 ? 'X' : ' '));
                    menu_add_item_menu_shortcut(array_menu_textspeech,'p');
                    menu_add_item_menu_tooltip(array_menu_textspeech,"Text will be sent to speech when line is larger than line width and a space, comma or semicolon is detected");
                    menu_add_item_menu_ayuda(array_menu_textspeech,"Text will be sent to speech when line is larger than line width and a space, comma or semicolon is detected");


                    menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_chardetection_settings_stdout_line_witdh_dot,NULL,"[%c] End line with d~~ot",(chardetect_line_width_wait_dot.v==1 ? 'X' : ' '));
                    menu_add_item_menu_shortcut(array_menu_textspeech,'o');
                    menu_add_item_menu_tooltip(array_menu_textspeech,"Text will be sent to speech when line is larger than line width and a dot is detected");
                    menu_add_item_menu_ayuda(array_menu_textspeech,"Text will be sent to speech when line is larger than line width and a dot is detected");

                }

                menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_textspeech_filter_timeout,NULL,"[%d] ~~Timeout no enter",textspeech_timeout_no_enter);
                menu_add_item_menu_shortcut(array_menu_textspeech,'t');
                menu_add_item_menu_tooltip(array_menu_textspeech,"After some seconds the text will be sent to the Speech program when no "
                        "new line is sent");
                menu_add_item_menu_ayuda(array_menu_textspeech,"After some seconds the text will be sent to the Speech program when no "
                        "new line is sent. 0=never. A new line could also be detected by a space, comma, semicolon or dot depending on previous choices in this menu");


                
                menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_chardetection_settings_send_consolewindow,NULL,"[%c] Get stdout from script",(textspeech_get_stdout.v==1 ? 'X' : ' '));                    
                menu_add_item_menu_tooltip(array_menu_textspeech,"Send stdout from script to debug console window");
                menu_add_item_menu_ayuda(array_menu_textspeech,"Send stdout from script to debug console window");
                
            }

        }

          menu_add_item_menu(array_menu_textspeech,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_textspeech,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_textspeech);

                retorno_menu=menu_dibuja_menu(&textspeech_opcion_seleccionada,&item_seleccionado,array_menu_textspeech,"Text to Speech" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}




void menu_chardetection_settings_trap_rst16(MENU_ITEM_PARAMETERS)
{
        chardetect_printchar_enabled.v ^=1;
}



void menu_chardetection_settings_second_trap(MENU_ITEM_PARAMETERS)
{

        char string_dir[6];

        int dir;


        sprintf (string_dir,"%d",chardetect_second_trap_char_dir);

        menu_ventana_scanf("Address (0=none)",string_dir,6);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }

        chardetect_second_trap_char_dir=dir;

}

void menu_chardetection_settings_third_trap(MENU_ITEM_PARAMETERS)
{

        char string_dir[6];

        int dir;


        sprintf (string_dir,"%d",chardetect_third_trap_char_dir);

        menu_ventana_scanf("Address (0=none)",string_dir,6);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }

        chardetect_third_trap_char_dir=dir;

}

void menu_chardetection_settings_stdout_trap_detection(MENU_ITEM_PARAMETERS)
{


        trap_char_detection_routine_number++;
        if (trap_char_detection_routine_number==TRAP_CHAR_DETECTION_ROUTINES_TOTAL) trap_char_detection_routine_number=0;

        chardetect_init_trap_detection_routine();

}

void menu_chardetection_settings_chardetect_char_filter(MENU_ITEM_PARAMETERS)
{
        chardetect_char_filter++;
        if (chardetect_char_filter==CHAR_FILTER_TOTAL) chardetect_char_filter=0;
}



void menu_chardetection_settings_second_trap_sum32(MENU_ITEM_PARAMETERS)
{

        chardetect_second_trap_sum32.v ^=1;

        //y ponemos el contador al maximo para que no se cambie por si solo
        chardetect_second_trap_sum32_counter=MAX_STDOUT_SUM32_COUNTER;


}


void menu_chardetection_settings_second_trap_range_min(MENU_ITEM_PARAMETERS)
{

        char string_dir[6];

        int dir;


        sprintf (string_dir,"%d",chardetect_second_trap_detect_pc_min);

        menu_ventana_scanf("Address",string_dir,6);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }

        chardetect_second_trap_detect_pc_min=dir;

}

void menu_chardetection_settings_second_trap_range_max(MENU_ITEM_PARAMETERS)
{

        char string_dir[6];

        int dir;


        sprintf (string_dir,"%d",chardetect_second_trap_detect_pc_max);

        menu_ventana_scanf("Address",string_dir,6);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }

        chardetect_second_trap_detect_pc_max=dir;

}




void menu_chardetection_settings_enable(MENU_ITEM_PARAMETERS)
{
	chardetect_detect_char_enabled.v ^=1;
}

void menu_chardetection_chardetect_ignore_newline(MENU_ITEM_PARAMETERS)
{
    chardetect_ignore_newline.v ^=1;
}

void menu_chardetection_chardetect_rom_number_compat(MENU_ITEM_PARAMETERS)
{
    chardetect_rom_compat_numbers.v ^=1;
}

//menu chardetection settings
void menu_chardetection_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_chardetection_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                        menu_add_item_menu_inicial_format(&array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_trap_rst16,NULL,"[%c] ~~Trap print", (chardetect_printchar_enabled.v==1 ? 'X' : ' ' ));
			menu_add_item_menu_shortcut(array_menu_chardetection_settings,'t');
                        menu_add_item_menu_tooltip(array_menu_chardetection_settings,"It enables the emulator to show and send to speech the text sent to standard rom print call routines and non standard, generated from some games, specially text adventures");
                        menu_add_item_menu_ayuda(array_menu_chardetection_settings,"It enables the emulator to show and send to speech the text sent to standard rom print call routines and generated from some games, specially text adventures. "
                                                "On Spectrum, ZX80, ZX81 machines, standard rom calls are RST 10H. On Z88, it traps OS_OUT and some other calls. Non standard calls are the ones indicated on Second and Third trap");


			if (chardetect_printchar_enabled.v) {

                menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_chardetect_rom_number_compat,NULL,"[%c] ROM number print compat", (chardetect_rom_compat_numbers.v==1 ? 'X' : ' ' ));     
                menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Enable ROM trap compatibility for printing numbers (but not good for printing from games, like PAWS)");
                menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Enable ROM trap compatibility for printing numbers (but not good for printing from games, like PAWS)");


	                        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_second_trap,NULL,"~~Second trap address [%d]",chardetect_second_trap_char_dir);
				menu_add_item_menu_shortcut(array_menu_chardetection_settings,'s');
        	                menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Address of the second print routine");
                	        menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Address of the second print routine");

	                        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_second_trap_sum32,NULL,"[%c] Second trap s~~um 32",(chardetect_second_trap_sum32.v ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_chardetection_settings,'u');
				menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Sums 32 to the ASCII value read");
				menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Sums 32 to the ASCII value read");


        	                menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_third_trap,NULL,"T~~hird trap address [%d]",chardetect_third_trap_char_dir);
				menu_add_item_menu_shortcut(array_menu_chardetection_settings,'h');
                	        menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Address of the third print routine");
                        	menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Address of the third print routine");

       menu_add_item_menu(array_menu_chardetection_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);			



      


                        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_chardetect_char_filter,NULL,"Char ~~filter [%s]",chardetect_char_filter_names[chardetect_char_filter]);
			menu_add_item_menu_shortcut(array_menu_chardetection_settings,'f');
			menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Send characters to an internal filter");
			menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Send characters to an internal filter");

               

            menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_chardetect_ignore_newline,NULL,"[%c] ~~Ignore new line character", (chardetect_ignore_newline.v==1 ? 'X' : ' ' ));     
            menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Just ignore new line characters");
            menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Just ignore new line characters");

			}

                menu_add_item_menu(array_menu_chardetection_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);



			menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_enable,NULL,"[%c] Enable 2nd trap ~~detection",(chardetect_detect_char_enabled.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_chardetection_settings,'d');
			menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Enable char detection method to guess Second Trap address");
			menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Enable char detection method to guess Second Trap address");





			if (chardetect_detect_char_enabled.v) {


	                        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_stdout_trap_detection,NULL,"Detect ~~routine [%s]",trap_char_detection_routines_texto[trap_char_detection_routine_number]);
				menu_add_item_menu_shortcut(array_menu_chardetection_settings,'r');
        			 menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Selects method for second trap character routine detection");
	                        menu_add_item_menu_ayuda(array_menu_chardetection_settings,"This function enables second trap character routine detection for programs "
                                                "that does not use RST16 calls to ROM for printing characters, on Spectrum models. "
                                                "It tries to guess where the printing "
                                                "routine is located and set Second Trap address when it finds it. This function has some pre-defined known "
                                                "detection call printing routines (for example AD Adventures) and one other totally automatic method, "
                                        	"which first tries to find automatically an aproximate range where the routine is, and then, "
						"it finds which routine is, trying all on this list. "
						"This automatic method "
                                                "makes writing operations a bit slower (only while running the detection routine)");


        	                if (trap_char_detection_routine_number!=TRAP_CHAR_DETECTION_ROUTINE_AUTOMATIC && trap_char_detection_routine_number!=TRAP_CHAR_DETECTION_ROUTINE_NONE)  {
                        	        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_second_trap_range_min,NULL,"Detection routine mi~~n [%d]",chardetect_second_trap_detect_pc_min);
					menu_add_item_menu_shortcut(array_menu_chardetection_settings,'n');
					menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Lower address limit to find character routine");
					menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Lower address limit to find character routine");


                	                menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_second_trap_range_max,NULL,"Detection routine ma~~x [%d]",chardetect_second_trap_detect_pc_max);
					menu_add_item_menu_shortcut(array_menu_chardetection_settings,'x');
					menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Higher address limit to find character routine");
					menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Higher address limit to find character routine");
	                        }


			}






                menu_add_item_menu(array_menu_chardetection_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_chardetection_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_chardetection_settings);

                retorno_menu=menu_dibuja_menu(&chardetection_settings_opcion_seleccionada,&item_seleccionado,array_menu_chardetection_settings,"Print char traps" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

void menu_accessibility_menu_high_contrast(MENU_ITEM_PARAMETERS)
{

    int indice=menu_get_gui_index_by_name("Clean");
    if (indice<0) {
        debug_printf(VERBOSE_ERR,"Invalid gui style");
        return;
    }

    menu_interface_change_gui_style_apply(indice);

    menu_generic_message_splash("Set high contrast style","OK. Style applied");


}

void menu_accessibility_menu_big_font(MENU_ITEM_PARAMETERS)
{
    menu_char_width=8;

	menu_interface_charwidth_after_width_change();

    menu_generic_message_splash("Set big font","OK. Big font applied");
}

void menu_accessibility_menu_zxdesktop_clean(MENU_ITEM_PARAMETERS)
{
    //color solido
    menu_ext_desktop_fill=0;
    //y color blanco
    menu_ext_desktop_fill_first_color=7;
    //y sin fondo scr
    zxdesktop_draw_scrfile_enabled=0;


    menu_generic_message_splash("ZX Desktop clean fill","OK. Clean ZX Desktop applied");
}


void menu_accessibility_menu_zxdesktop_button_boxes(MENU_ITEM_PARAMETERS)
{
    menu_ext_desktop_disable_box_upper_icons.v=0;
    menu_ext_desktop_disable_box_lower_icons.v=0;

    //Y no transparentes botones e iconos
    menu_ext_desktop_transparent_upper_icons.v=0;
    menu_ext_desktop_transparent_lower_icons.v=0;
    menu_ext_desktop_transparent_configurable_icons.v=0;
    

    menu_generic_message_splash("ZX Desktop buttons","OK. Enabled boxes and disable transparency on ZX Desktop buttons and icons");
}

void menu_accessibility_menu_hotkeys(MENU_ITEM_PARAMETERS)
{
    menu_force_writing_inverse_color.v=1;

    menu_generic_message_splash("Force hotkeys","OK. Forced hotkeys");
}

void menu_accessibility_menu_disable_back(MENU_ITEM_PARAMETERS)
{
	menu_interface_allow_background_windows_delete_windows();

	menu_allow_background_windows=0;

    menu_generic_message_splash("Disable background windows","OK. Disabled background windows");
}

void menu_accessibility_gray_scale(MENU_ITEM_PARAMETERS)
{
    if (screen_gray_mode) screen_gray_mode=0;
    else screen_gray_mode=7;

	menu_interface_rgb_inverse_common();    
}

void menu_accessibility_menu(MENU_ITEM_PARAMETERS)
{

    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

    do {

        menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,menu_accessibility_menu_high_contrast,NULL,"Set high ~~contrast style");
        menu_add_item_menu_shortcut(array_menu_common,'c');
        menu_add_item_menu_tooltip(array_menu_common,"This setting can be also be enabled/disabled from Settings-> ZX Vision menu");
        menu_add_item_menu_ayuda(array_menu_common,"This setting can be also be enabled/disabled from Settings-> ZX Vision menu");

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_accessibility_menu_big_font,NULL,"Set big ~~font");
        menu_add_item_menu_shortcut(array_menu_common,'f');
        menu_add_item_menu_tooltip(array_menu_common,"This setting can be also be enabled/disabled from Settings-> ZX Vision menu");
        menu_add_item_menu_ayuda(array_menu_common,"This setting can be also be enabled/disabled from Settings-> ZX Vision menu");


        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_accessibility_menu_hotkeys,NULL,"Force ~~hotkeys");
        menu_add_item_menu_shortcut(array_menu_common,'h');
        menu_add_item_menu_tooltip(array_menu_common,"This setting can be also be enabled/disabled from Settings-> ZX Vision menu");
        menu_add_item_menu_ayuda(array_menu_common,"This setting can be also be enabled/disabled from Settings-> ZX Vision menu");


        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_accessibility_menu_disable_back,NULL,"Disable background ~~windows");
        menu_add_item_menu_shortcut(array_menu_common,'w');
        menu_add_item_menu_tooltip(array_menu_common,"This setting can be also be enabled/disabled from Settings-> ZX Vision menu");
        menu_add_item_menu_ayuda(array_menu_common,"This setting can be also be enabled/disabled from Settings-> ZX Vision menu"); 

        if (screen_ext_desktop_enabled) {
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_accessibility_menu_zxdesktop_clean,NULL,"ZX Desktop c~~lean fill");
            menu_add_item_menu_shortcut(array_menu_common,'l');
            menu_add_item_menu_tooltip(array_menu_common,"This setting can be also be enabled/disabled from Settings-> ZX Desktop menu");
            menu_add_item_menu_ayuda(array_menu_common,"This setting can be also be enabled/disabled from Settings-> ZX Desktop menu");

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_accessibility_menu_zxdesktop_button_boxes,NULL,"ZX Desktop ~~objects visibility");
            menu_add_item_menu_shortcut(array_menu_common,'o');
            menu_add_item_menu_tooltip(array_menu_common,"Affects buttons and icons. This setting can be also be enabled/disabled from Settings-> ZX Desktop menu");
            menu_add_item_menu_ayuda(array_menu_common,"Affects buttons and icons. This setting can be also be enabled/disabled from Settings-> ZX Desktop menu");
        }

        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_interface_inverse_video,NULL,"[%c] Inverse colours",(inverse_video.v==1 ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_common,"Inverse Color Palette");
		menu_add_item_menu_ayuda(array_menu_common,"Inverses all the colours used on the emulator, including menu");    

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_accessibility_gray_scale,NULL,"[%c] Gray mode",(screen_gray_mode ? 'X' : ' ' ));
		menu_add_item_menu_tooltip(array_menu_common,"Set Gray Palette");
		menu_add_item_menu_ayuda(array_menu_common,"Set Gray Palette to all the colours used on the emulator, including menu");    

		menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_interface_flash,NULL,"[%c] Flash enabled",(disable_change_flash.v==0 ? 'X' : ' '));
        menu_add_item_menu_spanish_format(array_menu_common,"[%c] Parpadeo activado",(disable_change_flash.v==0 ? 'X' : ' '));
        menu_add_item_menu_tooltip(array_menu_common,"Disables flash for emulated machines and also for menu interface");
        menu_add_item_menu_ayuda(array_menu_common,"Disables flash for emulated machines and also for menu interface");


 
        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&accessibility_menu_opcion_seleccionada,&item_seleccionado,array_menu_common,"GUI accessibility");



        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}                        


void menu_accessibility_settings(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_accessibility_settings;
    menu_item item_seleccionado;
    int retorno_menu;
    do {



        menu_add_item_menu_inicial_format(&array_menu_accessibility_settings,MENU_OPCION_NORMAL,menu_chardetection_settings,NULL,"~~Print char traps");
        menu_add_item_menu_spanish_catalan(array_menu_accessibility_settings,"Traps de im~~presión de caracteres","Traps d'im~~pressió de caràcters");
        menu_add_item_menu_shortcut(array_menu_accessibility_settings,'p');
        menu_add_item_menu_tooltip(array_menu_accessibility_settings,"Settings on capture print character routines");
        menu_add_item_menu_ayuda(array_menu_accessibility_settings,"Settings on capture print character routines");
        menu_add_item_menu_tiene_submenu(array_menu_accessibility_settings);


        menu_add_item_menu_en_es_ca(array_menu_accessibility_settings,MENU_OPCION_NORMAL,menu_textspeech,NULL,
            "~~Text to speech","~~Texto a voz","~~Text a veu");
        menu_add_item_menu_shortcut(array_menu_accessibility_settings,'t');
        menu_add_item_menu_tooltip(array_menu_accessibility_settings,"Specify a script or program to send all text generated, "
                                "from Spectrum display or emulator menu, "
                                "usually used on text to speech");
        menu_add_item_menu_ayuda(array_menu_accessibility_settings,"Specify a script or program to send all text generated, "
                                "from Spectrum display or emulator menu, "
                                "usually used on text to speech. "
                                "When running the script: \n"
                                "ESC means abort next executions on queue.\n"
                                "Enter means run pending execution.\n");
        menu_add_item_menu_tiene_submenu(array_menu_accessibility_settings);

        menu_add_item_menu_format(array_menu_accessibility_settings,MENU_OPCION_NORMAL,menu_accessibility_menu,NULL,"~~GUI");
        menu_add_item_menu_shortcut(array_menu_accessibility_settings,'g');
        menu_add_item_menu_tooltip(array_menu_accessibility_settings,"Settings to improve accessibility on ZEsarUX GUI");
        menu_add_item_menu_ayuda(array_menu_accessibility_settings,"Settings to improve accessibility on ZEsarUX GUI");
        menu_add_item_menu_tiene_submenu(array_menu_accessibility_settings);



        menu_add_item_menu(array_menu_accessibility_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        //menu_add_item_menu(array_menu_accessibility_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
        menu_add_ESC_item(array_menu_accessibility_settings);

        retorno_menu=menu_dibuja_menu(&accessibility_settings_opcion_seleccionada,&item_seleccionado,array_menu_accessibility_settings,"Accessibility Settings");

    

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                        
                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

void menu_zxvision_settings_advanced_enable(MENU_ITEM_PARAMETERS)
{
    menu_show_advanced_items.v ^=1;
}


//menu settings
void menu_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings;
	menu_item item_seleccionado;
	int retorno_menu;

        do {


		menu_add_item_menu_inicial(&array_menu_settings,"~~Accessibility",MENU_OPCION_NORMAL,menu_accessibility_settings,NULL);
        menu_add_item_menu_spanish_catalan(array_menu_settings,"~~Accesibilidad","~~Accessibilitat");
		menu_add_item_menu_shortcut(array_menu_settings,'a');
		menu_add_item_menu_tooltip(array_menu_settings,"Accessibility settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Accessibility settings, to use text-to-speech facilities on ZEsarUX menu and games");
        menu_add_item_menu_tiene_submenu(array_menu_settings);

		menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_settings_audio,NULL,"A~~udio");
		menu_add_item_menu_shortcut(array_menu_settings,'u');
		menu_add_item_menu_tooltip(array_menu_settings,"Audio settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Audio settings");
        menu_add_item_menu_tiene_submenu(array_menu_settings);

		menu_add_item_menu_en_es_ca(array_menu_settings,MENU_OPCION_NORMAL,menu_settings_config_file,NULL,
            "Configu~~ration file","Archivo configu~~ración","Arxiu configu~~ració");
		menu_add_item_menu_shortcut(array_menu_settings,'r');
		menu_add_item_menu_tooltip(array_menu_settings,"Configuration file");
		menu_add_item_menu_ayuda(array_menu_settings,"Configuration file");
        menu_add_item_menu_tiene_submenu(array_menu_settings);

		menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_cpu_settings,NULL,"~~CPU");
		menu_add_item_menu_shortcut(array_menu_settings,'c');
	    	menu_add_item_menu_tooltip(array_menu_settings,"Change some CPU settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Change some CPU settings");		
        menu_add_item_menu_tiene_submenu(array_menu_settings);
        menu_add_item_menu_es_avanzado(array_menu_settings);

		menu_add_item_menu(array_menu_settings,"D~~ebug",MENU_OPCION_NORMAL,menu_settings_debug,NULL);
		menu_add_item_menu_shortcut(array_menu_settings,'e');
		menu_add_item_menu_tooltip(array_menu_settings,"Debug settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Debug settings");
        menu_add_item_menu_tiene_submenu(array_menu_settings);
        menu_add_item_menu_es_avanzado(array_menu_settings);

		menu_add_item_menu(array_menu_settings,"~~Display",MENU_OPCION_NORMAL,menu_settings_display,NULL);
		menu_add_item_menu_shortcut(array_menu_settings,'d');
		menu_add_item_menu_tooltip(array_menu_settings,"Display settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Display settings");
        menu_add_item_menu_tiene_submenu(array_menu_settings);

		menu_add_item_menu_en_es_ca(array_menu_settings,MENU_OPCION_NORMAL,menu_external_tools_config,NULL,
            "E~~xternal tools paths","Rutas utilidades e~~xternas","Rutes utilitats e~~xternes");
		menu_add_item_menu_shortcut(array_menu_settings,'x');
        menu_add_item_menu_tooltip(array_menu_settings,"External tools paths settings");
        menu_add_item_menu_ayuda(array_menu_settings,"External tools paths settings");
        menu_add_item_menu_tiene_submenu(array_menu_settings);
        menu_add_item_menu_es_avanzado(array_menu_settings);

		menu_add_item_menu_en_es_ca(array_menu_settings,MENU_OPCION_NORMAL,menu_fileselector_settings,NULL,
            "~~File Browser","Navegador de ~~ficheros","Navegador de ~~fitxers");
		menu_add_item_menu_shortcut(array_menu_settings,'f');
		menu_add_item_menu_tooltip(array_menu_settings,"Settings for the File browser");
		menu_add_item_menu_ayuda(array_menu_settings,"These settings are related to the File Browser");   
        menu_add_item_menu_tiene_submenu(array_menu_settings);
        menu_add_item_menu_es_avanzado(array_menu_settings);

		//Set F keys functions
		menu_add_item_menu_en_es_ca(array_menu_settings,MENU_OPCION_NORMAL,menu_hardware_set_f_functions,NULL,
            "Fu~~nction keys","Teclas de fu~~nciones","Tecles de fu~~ncions");
		menu_add_item_menu_tooltip(array_menu_settings,"Assign actions to F keys");
		menu_add_item_menu_ayuda(array_menu_settings,"Assign actions to F keys");
        menu_add_item_menu_shortcut(array_menu_settings,'n'); 
        menu_add_item_menu_tiene_submenu(array_menu_settings);       

		//menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_window_settings,NULL,"ZEsarUX ~~Window");
        menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_general_settings,NULL,"~~General");
		menu_add_item_menu_shortcut(array_menu_settings,'g');
		menu_add_item_menu_tooltip(array_menu_settings,"These settings are related to the ZEsarUX Window");
		menu_add_item_menu_ayuda(array_menu_settings,"These settings are related to the ZEsarUX Window");
        menu_add_item_menu_tiene_submenu(array_menu_settings);


		menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_hardware_settings,NULL,"~~Hardware");
		menu_add_item_menu_shortcut(array_menu_settings,'h');
		menu_add_item_menu_tooltip(array_menu_settings,"Other hardware settings for the running machine (not CPU or ULA)");
		menu_add_item_menu_ayuda(array_menu_settings,"Select different settings for the machine and change its behaviour (not CPU or ULA)");
        menu_add_item_menu_tiene_submenu(array_menu_settings);

		menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_osd_settings,NULL,"~~OSD");
		menu_add_item_menu_shortcut(array_menu_settings,'o');
        menu_add_item_menu_tiene_submenu(array_menu_settings);
        menu_add_item_menu_es_avanzado(array_menu_settings);	        

		menu_add_item_menu_en_es_ca(array_menu_settings,MENU_OPCION_NORMAL,menu_settings_snapshot,NULL,
            "~~Snapshot","In~~stantánea","Instantània");
		menu_add_item_menu_shortcut(array_menu_settings,'s');
		menu_add_item_menu_tooltip(array_menu_settings,"Snapshot settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Snapshot settings");
        menu_add_item_menu_tiene_submenu(array_menu_settings);
        menu_add_item_menu_es_avanzado(array_menu_settings);

#ifndef NETWORKING_DISABLED	

		//De momento todo lo que hay en el menu de Statistics requiere red, y este requiere pthreads
		//Si no hay threads, tampoco este menu

		menu_add_item_menu_en_es_ca(array_menu_settings,MENU_OPCION_NORMAL,menu_settings_statistics,NULL,
            "Stat~~istics","Estad~~ísticas","Estad~~ístiques");
		menu_add_item_menu_shortcut(array_menu_settings,'i');
		menu_add_item_menu_tooltip(array_menu_settings,"Statistics settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Statistics settings");
        menu_add_item_menu_tiene_submenu(array_menu_settings);

#endif

		menu_add_item_menu_en_es_ca(array_menu_settings,MENU_OPCION_NORMAL,menu_settings_storage,NULL,
            "S~~torage","Almacenamien~~to","Emmagatzema~~tge");
		menu_add_item_menu_shortcut(array_menu_settings,'t');
		menu_add_item_menu_tooltip(array_menu_settings,"Storage settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Storage settings");
        menu_add_item_menu_tiene_submenu(array_menu_settings);

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_ula_settings,NULL,"U~~LA");
			menu_add_item_menu_shortcut(array_menu_settings,'l');
            menu_add_item_menu_tooltip(array_menu_settings,"Change some ULA settings");
            menu_add_item_menu_ayuda(array_menu_settings,"Change some ULA settings");
            menu_add_item_menu_tiene_submenu(array_menu_settings);
            menu_add_item_menu_es_avanzado(array_menu_settings);


		}	

		if (scr_driver_can_ext_desktop() ) {
			menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_ext_desktop_settings,NULL,"ZX Des~~ktop");
			menu_add_item_menu_shortcut(array_menu_settings,'k');
			menu_add_item_menu_tooltip(array_menu_settings,"Expand the program window having a ZX Desktop space to the right and on the bottom");
			menu_add_item_menu_ayuda(array_menu_settings,"ZX Desktop enables you to have a space on the right and on the bottom to place "
				"zxvision windows, menus or other widgets");
            menu_add_item_menu_tiene_submenu(array_menu_settings);
		}


		menu_add_item_menu(array_menu_settings,"ZX ~~Vision",MENU_OPCION_NORMAL,menu_zxvision_settings,NULL);
		menu_add_item_menu_shortcut(array_menu_settings,'v');
		menu_add_item_menu_tooltip(array_menu_settings,"These settings are related to the GUI interface: ZX Vision");
		menu_add_item_menu_ayuda(array_menu_settings,"These settings are related to the GUI interface: ZX Vision");
        menu_add_item_menu_tiene_submenu(array_menu_settings);


        menu_add_item_menu_separator(array_menu_settings);

        menu_add_item_menu_en_es_ca(array_menu_settings,MENU_OPCION_NORMAL,menu_zxvision_settings_advanced_enable,NULL,
            "Advanced menu items","Items de menú avanzados","Items de menú avançats");
        menu_add_item_menu_prefijo_format(array_menu_settings,"[%c] ",(menu_show_advanced_items.v ? 'X' : ' ') );
        menu_add_item_menu_tooltip(array_menu_settings,"Shows advanced menu items");
        menu_add_item_menu_ayuda(array_menu_settings,"Shows advanced menu items");


        menu_add_item_menu_separator(array_menu_settings);


                //menu_add_item_menu(array_menu_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings);

                retorno_menu=menu_dibuja_menu(&settings_opcion_seleccionada,&item_seleccionado,array_menu_settings,"Settings" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	                //llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

void menu_snapshot_permitir_versiones_desconocidas(MENU_ITEM_PARAMETERS)
{
	snap_zx_permitir_versiones_desconocidas.v ^=1;
}


void menu_snapshot_save_version(MENU_ITEM_PARAMETERS)
{
	snap_zx_version_save++;
	if (snap_zx_version_save>CURRENT_ZX_VERSION) snap_zx_version_save=1;
}

void menu_snapshot_autosave_at_interval(MENU_ITEM_PARAMETERS)
{
	snapshot_contautosave_interval_enabled.v ^=1;

	//resetear contador
	snapshot_autosave_interval_current_counter=0;
}

void menu_snapshot_close_menu_after_smartload(MENU_ITEM_PARAMETERS)
{
	no_close_menu_after_smartload.v ^=1;
}

void menu_snapshot_autosave_at_interval_directory(MENU_ITEM_PARAMETERS)
{
	menu_storage_string_root_dir(snapshot_autosave_interval_quicksave_directory);
}

void menu_snapshot_autosave_at_interval_seconds(MENU_ITEM_PARAMETERS)
{
	char string_segundos[3];

sprintf (string_segundos,"%d",snapshot_autosave_interval_seconds);

	menu_ventana_scanf("Seconds: ",string_segundos,3);

	int valor_leido=parse_string_to_number(string_segundos);

	if (valor_leido<0 || valor_leido>999) debug_printf(VERBOSE_ERR,"Invalid interval");

	else snapshot_autosave_interval_seconds=valor_leido;
}

void menu_snapshot_autosave_at_interval_prefix(MENU_ITEM_PARAMETERS)
{
	char string_prefix[30];
	//Aunque el limite real es PATH_MAX, lo limito a 30

	sprintf (string_prefix,"%s",snapshot_autosave_interval_quicksave_name);

	menu_ventana_scanf("Name prefix: ",string_prefix,30);

	if (string_prefix[0]==0) return;

	strcpy(snapshot_autosave_interval_quicksave_name,string_prefix);
}

void menu_snapshot_sna_set_machine(MENU_ITEM_PARAMETERS)
{
	sna_setting_no_change_machine.v ^=1;
}

void menu_snapshot_settings_compressed_zsf(MENU_ITEM_PARAMETERS)
{
	zsf_force_uncompressed ^=1;
}

void menu_snapshot_autosave_exit(MENU_ITEM_PARAMETERS)
{
	autosave_snapshot_on_exit.v ^=1;
}

void menu_snapshot_autoload_start(MENU_ITEM_PARAMETERS)
{
        autoload_snapshot_on_start.v ^=1;
}


void menu_snapshot_autosnap_path(MENU_ITEM_PARAMETERS)
{
	menu_storage_string_root_dir(autosave_snapshot_path_buffer);
}

void menu_settings_snapshot_sync_to_z88_clock(MENU_ITEM_PARAMETERS)
{
    sync_clock_to_z88.v ^=1;
}

void menu_snapshot_settings_rom_zsf(MENU_ITEM_PARAMETERS)
{
    zsf_snap_save_rom.v ^=1;
}

void menu_settings_snapshot(MENU_ITEM_PARAMETERS)
{

        menu_item *array_menu_settings_snapshot;
        menu_item item_seleccionado;
        int retorno_menu;

        do {


			//hotkeys usados: uvctslpinrh
					char string_autosave_interval_prefix[16];
					menu_tape_settings_trunc_name(snapshot_autosave_interval_quicksave_name,string_autosave_interval_prefix,16);

					char string_autosave_interval_path[16];
					menu_tape_settings_trunc_name(snapshot_autosave_interval_quicksave_directory,string_autosave_interval_path,16);


		menu_add_item_menu_en_es_ca_inicial(&array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_permitir_versiones_desconocidas,
            NULL,"Allow U~~nknown .ZX versions","Permitir versiones .ZX desco~~nocidas","Permetre versions .ZX desco~~negudes");
        menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%c] ",(snap_zx_permitir_versiones_desconocidas.v ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_snapshot,'n');
		menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Allow loading ZX Snapshots of unknown versions");
		menu_add_item_menu_ayuda(array_menu_settings_snapshot,"This setting permits loading of ZX Snapshots files of unknown versions. "
            "It can be used to load snapshots saved on higher emulator versions than this one");


		menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_save_version,NULL,
            "Save ZX Snapshot ~~version","Grabar ZX Snapshot ~~versión","Gravar ZX Snapshot ~~versió");
        menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%d] ",snap_zx_version_save);
		menu_add_item_menu_shortcut(array_menu_settings_snapshot,'v');
        menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Decide which kind of .ZX version file is saved");
        menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Version 1,2,3 works on ZEsarUX and ZXSpectr\n"
            "Version 4 works on ZEsarUX V1.3 and higher\n"
            "Version 5 works on ZEsarUX V2 and higher\n"
        );

        menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_settings_compressed_zsf,NULL,
            "~~Compressed ZSF","ZSF ~~Comprimido","ZSF ~~Comprimit");
        menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%c] ",(zsf_force_uncompressed ? ' ' : 'X') );
        menu_add_item_menu_shortcut(array_menu_settings_snapshot,'c');
        menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Setting to save compressed ZSF files or not"); 
        menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Setting to save compressed ZSF files or not");

        menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_settings_rom_zsf,NULL,
            "ZSF save R~~OM","ZSF grabar R~~OM","ZSF gravar R~~OM");
        menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%c] ",(zsf_snap_save_rom.v ? 'X' : ' ') );
        menu_add_item_menu_shortcut(array_menu_settings_snapshot,'o');
        menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Include ROM contents in saved ZSF snapshot"); 
        menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Include ROM contents in saved ZSF snapshot. Useful when running custom roms. "
            "Only available for Spectrum/Clones models 16k/48k/128k/+2/+2A/+3");

        menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_sna_set_machine,NULL,
            "Set ~~machine snap load","Cambio ~~máquina al cargar snap","Canvi ~~màquina al carregar snap");
        menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%c] ",(sna_setting_no_change_machine.v ? ' ' : 'X'));
        menu_add_item_menu_shortcut(array_menu_settings_snapshot,'m');
        menu_add_item_menu_tooltip(array_menu_settings_snapshot,"If machine is reset to 48k/128k when loading a .sna or .z80 snapshot file");
        menu_add_item_menu_ayuda(array_menu_settings_snapshot,"If machine is reset to 48k/128k when loading a .sna or .z80 snapshot file.\n"
            "Disabling it, the .sna snapshot is loaded but the machine is not changed, so it allows to load, for example, a 48k snapshot on a Prism machine, or TBBlue, or any Spectrum machine different than 48/128.\n"
            "If current machine is not a Spectrum, loading a .sna snapshot will always switch to 48k/128k.\n"
            "This setting only applies to .sna snapshots, but not to .z80, .zx, or any other snapshot type."
        );

        menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_close_menu_after_smartload,NULL,
            "Close menu after smartload","Cerrar menú después smartload","Tancar menú després smartload");
        menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%c] ",(no_close_menu_after_smartload.v ? ' ' : 'X'));
        menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Closes the menu after Smartload");
        menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Closes the menu after Smartload");

        if (MACHINE_IS_Z88) {
            menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_settings_snapshot_sync_to_z88_clock,NULL,
            "Sync PC->Z88 clock","Sinc PC->reloj Z88","Sinc PC->rellotge Z88");
            menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%c] ", (sync_clock_to_z88.v ? 'X' : ' ') );
            menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Sync PC clock to Z88 clock after loading a snapshot");
            menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Sync PC clock to Z88 clock after loading a snapshot");
        }
        

        menu_add_item_menu(array_menu_settings_snapshot,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosave_exit,NULL,
            "Auto~~save on exit","Auto~~salvar al salir","Auto~~salvar al sortir");
		menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%c] ",(autosave_snapshot_on_exit.v ? 'X' : ' ' ) );            
		menu_add_item_menu_shortcut(array_menu_settings_snapshot,'s');
		 menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Saves a snapshot with the machine state when exiting ZEsarUX. Saved file is " AUTOSAVE_NAME);
		 menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Saves a snapshot with the machine state when exiting ZEsarUX. Saved file is " AUTOSAVE_NAME);



		menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autoload_start,NULL,
            "Auto~~load on start","Auto~~load al inicio","Auto~~load a l'inici");
		menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%c] ",(autoload_snapshot_on_start.v ? 'X' : ' ') );            
		menu_add_item_menu_shortcut(array_menu_settings_snapshot,'l');
		menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Loads the snapshot saved when starting ZEsarUX (previous menu item)");
		menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Loads the snapshot saved when starting ZEsarUX (previous menu item)");



		if (autosave_snapshot_on_exit.v || autoload_snapshot_on_start.v) {
            char string_autosnap_path[14];
            menu_tape_settings_trunc_name(autosave_snapshot_path_buffer,string_autosnap_path,14);
			menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosnap_path,NULL,
                "Autosnap ~~path","Autosnap car~~peta","Autosnap car~~peta");
            menu_add_item_menu_sufijo_format(array_menu_settings_snapshot," [%s]",string_autosnap_path);
			menu_add_item_menu_shortcut(array_menu_settings_snapshot,'p');
			menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Where to save/load automatic snapshot. If not set, uses current directory");
			menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Where to save/load automatic snapshot. If not set, uses current directory");
		}

		

        menu_add_item_menu(array_menu_settings_snapshot,"",MENU_OPCION_SEPARADOR,NULL,NULL);



        menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosave_at_interval,NULL,
            "Contsave at ~~interval","Contsave a ~~intervalo","Contsave a ~~interval");
        menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%c] ",(snapshot_contautosave_interval_enabled.v ? 'X' : ' ' ) );            
        menu_add_item_menu_shortcut(array_menu_settings_snapshot,'i');
        menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Enable continuous autosave snapshot every fixed interval");
        menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Enable continuous autosave snapshot every fixed interval");


        if (snapshot_contautosave_interval_enabled.v) {
            menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosave_at_interval_seconds,NULL,
                "Contsave S~~econds","Contsave S~~egundos","Contsave S~~egons");
            menu_add_item_menu_prefijo_format(array_menu_settings_snapshot,"[%d] ",snapshot_autosave_interval_seconds);
            menu_add_item_menu_shortcut(array_menu_settings_snapshot,'e');
            menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Save snapshot every desired interval");
            menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Save snapshot every desired interval");
        }


		menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosave_at_interval_prefix,NULL,
            "QS&CA P~~refix","QS&CA P~~refijo","QS&CA P~~refix");
        menu_add_item_menu_sufijo_format(array_menu_settings_snapshot," [%s]",string_autosave_interval_prefix);
		menu_add_item_menu_shortcut(array_menu_settings_snapshot,'r');
        menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Name prefix for quicksave and continous autosave snapshots");
        menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Name prefix for quicksave and continous autosave snapshots. The final name will be: prefix-date-hour.zsf");

        menu_add_item_menu_en_es_ca(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosave_at_interval_directory,NULL,
            "QS&CA P~~ath","QS&CA C~~arpeta","QS&CA C~~arpeta");
        menu_add_item_menu_sufijo_format(array_menu_settings_snapshot," [%s]",string_autosave_interval_path);
        menu_add_item_menu_shortcut(array_menu_settings_snapshot,'a');
        menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Path to save quicksave & continous autosave");
        menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Path to save quicksave & continous autosave. If not set, will use current directory");



        menu_add_item_menu(array_menu_settings_snapshot,"",MENU_OPCION_SEPARADOR,NULL,NULL);



		menu_add_ESC_item(array_menu_settings_snapshot);

        retorno_menu=menu_dibuja_menu(&settings_snapshot_opcion_seleccionada,&item_seleccionado,array_menu_settings_snapshot,"Snapshot Settings" );

                

            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    if (item_seleccionado.menu_funcion!=NULL) {
                            //printf ("actuamos por funcion\n");
                            item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
            
                    }
            }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}

void menu_print_text_axis(char *buffer,int button_type,int button_number)
{

	char buffer_axis[2];

	if (button_type==0) sprintf (buffer_axis,"%s","");
                                        //este sprintf se hace asi para evitar warnings al compilar

	if (button_type<0) sprintf (buffer_axis,"-");
	if (button_type>0) sprintf (buffer_axis,"+");

	sprintf(buffer,"%s%d",buffer_axis,button_number);

}

/*
void old_menu_hardware_realjoystick_event_button(MENU_ITEM_PARAMETERS)
{

    menu_simple_ventana("Redefine event","Please press the button/axis");
	menu_refresca_pantalla();

	//Para xwindows hace falta esto, sino no refresca
	 scr_actualiza_tablas_teclado();

        //redefinir evento
        if (!realjoystick_redefine_event(hardware_realjoystick_event_opcion_seleccionada)) {
		//se ha salido con tecla. ver si es ESC
		if ((puerto_especial1&1)==0) {
			//desasignar evento
			realjoystick_events_array[hardware_realjoystick_event_opcion_seleccionada].asignado.v=0;
		}
	}

}
*/

//Retorna tecla pulsada
int menu_common_wait_realjoystick_press(zxvision_window *ventana)
{
	z80_byte acumulado;



	int valor_contador_segundo_anterior;

	valor_contador_segundo_anterior=contador_segundo;

	menu_hardware_realjoystick_test_reset_last_values();

	int salir_por_boton=0;

    int tecla=0;

	do {

		menu_cpu_core_loop();
                acumulado=menu_da_todas_teclas();

		//si no hay multitarea, pausar
		if (menu_multitarea==0) {
			usleep(20000); //20 ms
		}

        
    

		//Si pulsado boton
		if (menu_info_joystick_last_button>=0 && menu_info_joystick_last_value!=0) {
			//printf ("Salir por boton\n");
			salir_por_boton=1;
		}

        //Escribir esto a cada frame
        if ( (  valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0 || salir_por_boton) {
            valor_contador_segundo_anterior=contador_segundo;
			//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);
			if (menu_multitarea==0) menu_refresca_pantalla();


			//char buffer_texto_medio[40];

			/*

            No escribimos nada. Creo que no es necesario mostrar esto
			
			zxvision_print_string_defaults_fillspc(ventana,1,linea,"Last joystick button/axis:");
			



			char buffer_type[40];

			
			
			if (menu_info_joystick_last_type==REALJOYSTICK_INPUT_EVENT_BUTTON) {
				strcpy(buffer_type,"Button");
			}
			else if (menu_info_joystick_last_type==REALJOYSTICK_INPUT_EVENT_AXIS) {
				strcpy(buffer_type,"Axis");
			}
			else strcpy(buffer_type,"Unknown");

		
			if (menu_info_joystick_last_button<0) strcpy(buffer_texto_medio,"Button: None");
			else sprintf (buffer_texto_medio,"Button: %d",menu_info_joystick_last_button);
			zxvision_print_string_defaults_fillspc(ventana,1,linea+1,buffer_texto_medio);

			if (menu_info_joystick_last_type<0) strcpy(buffer_texto_medio,"Type: None");
			else sprintf (buffer_texto_medio,"Type: %d (%s)",menu_info_joystick_last_type,buffer_type);
			zxvision_print_string_defaults_fillspc(ventana,1,linea+2,buffer_texto_medio);



			char buffer_event[40];
			if (menu_info_joystick_last_index>=0 && menu_info_joystick_last_index<MAX_EVENTS_JOYSTICK) {
				strcpy(buffer_event,realjoystick_event_names[menu_info_joystick_last_index]);
			}
			else {
				strcpy(buffer_event,"None");
			}


			sprintf (buffer_texto_medio,"Value: %6d",menu_info_joystick_last_raw_value);
			zxvision_print_string_defaults_fillspc(ventana,1,linea+3,buffer_texto_medio);

            */



			zxvision_draw_window_contents(ventana);


			
        }

		

        //Hay tecla pulsada
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA ) {
            tecla=menu_get_pressed_key();

                                                                                                                            
            //Si tecla no es ESC, no salir
                            
            if (tecla!=2) {
                acumulado = MENU_PUERTO_TECLADO_NINGUNA;
            }


            //Si ha salido por boton de joystick, esperar evento
            /*if (salir_por_boton) {
                if (menu_multitarea==0) menu_refresca_pantalla();
                menu_espera_no_tecla();
            }*/
            
                                                                    
        }


    } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && !salir_por_boton);

    menu_espera_no_tecla();

    return tecla;

}

void menu_hardware_realjoystick_event_button(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();
    

	zxvision_window ventana;

	int alto_ventana=3;
	int ancho_ventana=30;	
	int x_ventana=menu_center_x()-ancho_ventana/2; 
	int y_ventana=menu_center_y()-alto_ventana/2; 	

	zxvision_new_window(&ventana,x_ventana,y_ventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,"Redefine event");
	zxvision_draw_window(&ventana);		

    int linea=0;	

    zxvision_print_string_defaults(&ventana,1,linea,"Please press the button/axis");

    //simulador_joystick_forzado=1;

    int tecla=menu_common_wait_realjoystick_press(&ventana);



    //printf("Valor opcion: %d\n",valor_opcion);


    if (tecla==2) { 
        //Desasignar si se sale con ESC
        realjoystick_events_array[valor_opcion].asignado.v=0;
    }

    else {


        int button=menu_info_joystick_last_button;

        int type=menu_info_joystick_last_type;
        int value=menu_info_joystick_last_value;

        //printf("--Button %d type %d value %d\n",button,type,value);

        if (button>=0 && type>=0) {
            realjoystick_redefine_event_no_wait(valor_opcion,button,type,value);
        }
    }

    //menu_espera_no_joystick();

	
	zxvision_destroy_window(&ventana);    
    
}



//Retorna <0 si salir con ESC
int menu_joystick_event_list(void)
{
      
        menu_item *array_menu_joystick_event_list;
        menu_item item_seleccionado;
        int retorno_menu;

        int joystick_event_list_opcion_seleccionada=0;
       

                char buffer_texto[40];

                int i;
                for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {

                  //enum defined_f_function_ids accion=defined_f_functions_keys_array[i];

                  sprintf (buffer_texto,"%s",realjoystick_event_names[i]);


                    if (i==0) menu_add_item_menu_inicial_format(&array_menu_joystick_event_list,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);
                     else menu_add_item_menu_format(array_menu_joystick_event_list,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);

				}



                menu_add_item_menu(array_menu_joystick_event_list,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_joystick_event_list);

                retorno_menu=menu_dibuja_menu(&joystick_event_list_opcion_seleccionada,&item_seleccionado,array_menu_joystick_event_list,"Select event" );

                


								if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

												//Si se pulsa Enter
												return joystick_event_list_opcion_seleccionada;
			
                                }

                                else return -1;

}

/*
void old_menu_hardware_realjoystick_keys_button_by_button(int indice,z80_byte caracter)
{
    //Uso una simple ventana dado que por zxvision_window no puedo dejarla ahi de fondo y leer el joystick
    //TODO: probar esto porque quiza si se puede...
    menu_simple_ventana("Redefine key","Please press the button/axis");




    menu_refresca_pantalla();

    //Para xwindows hace falta esto, sino no refresca
    scr_actualiza_tablas_teclado();

    //redefinir boton a tecla
    if (!realjoystick_redefine_key(indice,caracter)) {
    //se ha salido con tecla. ver si es ESC
            if ((puerto_especial1&1)==0) {
                    //desasignar evento
        realjoystick_keys_array[indice].asignado.v=0;
            }
    }
}
*/

void menu_hardware_realjoystick_keys_button_by_button(int indice,z80_byte caracter)
{


	zxvision_window ventana;

	int alto_ventana=3;
	int ancho_ventana=30;	
	int x_ventana=menu_center_x()-ancho_ventana/2; 
	int y_ventana=menu_center_y()-alto_ventana/2; 	

	zxvision_new_window(&ventana,x_ventana,y_ventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,"Redefine key");
	zxvision_draw_window(&ventana);		

    int linea=0;	

    zxvision_print_string_defaults(&ventana,1,linea,"Please press the button/axis");

    //simulador_joystick_forzado=1;

    int tecla=menu_common_wait_realjoystick_press(&ventana);



    //printf("Valor opcion: %d\n",indice);


    if (tecla==2) { 
        //Desasignar si se sale con ESC
        realjoystick_keys_array[indice].asignado.v=0;
    }

    else {


        int button=menu_info_joystick_last_button;

        int type=menu_info_joystick_last_type;
        int value=menu_info_joystick_last_value;

        if (button>=0 && type>=0) {
            realjoystick_redefine_key_no_wait(indice,caracter,button,type,value);
        }
    }

    //menu_espera_no_joystick();

	
	zxvision_destroy_window(&ventana);    


}


void menu_hardware_realjoystick_keys_button(MENU_ITEM_PARAMETERS)
{

    //printf("valor opcion: %d\n",valor_opcion);

	//int menu_simple_two_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2)
	int tipo=menu_simple_two_choices("Selection type","You want to set by","Button","Event");

	if (tipo==0) return; //ESC


    z80_byte caracter;

    char buffer_caracter[2];
    buffer_caracter[0]=0;

    menu_ventana_scanf("Please write the key",buffer_caracter,2);


    caracter=buffer_caracter[0];

    if (caracter==0) {
		//desasignamos
		realjoystick_keys_array[valor_opcion].asignado.v=0;
		return;
	}



	

	if (tipo==1) { //Definir por boton

            menu_hardware_realjoystick_keys_button_by_button(valor_opcion,caracter);


        }

        if (tipo==2) { //Definir por evento
        	int evento=menu_joystick_event_list();
        	 realjoystick_copy_event_button_key(evento,valor_opcion,caracter);
        	//printf ("evento: %d\n",evento);
        }



}




void menu_hardware_realjoystick_clear_keys(MENU_ITEM_PARAMETERS)
{
        if (menu_confirm_yesno_texto("Clear list","Sure?")==1) {
                realjoystick_clear_keys_array();
        }
}


void menu_hardware_realjoystick_keys(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_realjoystick_keys;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char buffer_texto[40];
                char buffer_texto_boton[10];

                int i;
                for (i=0;i<MAX_KEYS_JOYSTICK;i++) {
                        if (realjoystick_keys_array[i].asignado.v) {
				menu_print_text_axis(buffer_texto_boton,realjoystick_keys_array[i].button_type,realjoystick_keys_array[i].button);

				z80_byte c=realjoystick_keys_array[i].caracter;
				if (c>=32 && c<=126) sprintf (buffer_texto,"Button %s sends [%c]",buffer_texto_boton,c);
				else sprintf (buffer_texto,"Button %s sends [(%d)]",buffer_texto_boton,c);
                        }

                        else {
                                sprintf(buffer_texto,"Unused entry");
			}



                        if (i==0) menu_add_item_menu_inicial_format(&array_menu_hardware_realjoystick_keys,MENU_OPCION_NORMAL,menu_hardware_realjoystick_keys_button,NULL,buffer_texto);
                        else menu_add_item_menu_format(array_menu_hardware_realjoystick_keys,MENU_OPCION_NORMAL,menu_hardware_realjoystick_keys_button,NULL,buffer_texto);

                        menu_add_item_menu_valor_opcion(array_menu_hardware_realjoystick_keys,i);


                        menu_add_item_menu_tooltip(array_menu_hardware_realjoystick_keys,"Redefine the button");
                        menu_add_item_menu_ayuda(array_menu_hardware_realjoystick_keys,"Indicates which key on the Spectrum keyboard is sent when "
						"pressed the button/axis on the real joystick");
                }

                menu_add_item_menu(array_menu_hardware_realjoystick_keys,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_item_menu_format(array_menu_hardware_realjoystick_keys,MENU_OPCION_NORMAL,menu_hardware_realjoystick_clear_keys,NULL,"Clear list");


                menu_add_item_menu(array_menu_hardware_realjoystick_keys,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_realjoystick_keys,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_realjoystick_keys);

                retorno_menu=menu_dibuja_menu(&hardware_realjoystick_keys_opcion_seleccionada,&item_seleccionado,array_menu_hardware_realjoystick_keys,"Joystick to keys" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



void menu_hardware_realjoystick_clear_events(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno_texto("Clear list","Sure?")==1) {
		realjoystick_clear_events_array();
	}
}


void menu_hardware_realjoystick_event(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_realjoystick_event;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char buffer_texto[40];
                char buffer_texto_boton[10];

                int i;
                for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
                        if (realjoystick_events_array[i].asignado.v) {
				menu_print_text_axis(buffer_texto_boton,realjoystick_events_array[i].button_type,realjoystick_events_array[i].button);
                        }

                        else {
                                sprintf(buffer_texto_boton,"None");
                        }

                        sprintf (buffer_texto,"Button for %s [%s]",realjoystick_event_names[i],buffer_texto_boton);


                        if (i==0) menu_add_item_menu_inicial_format(&array_menu_hardware_realjoystick_event,MENU_OPCION_NORMAL,menu_hardware_realjoystick_event_button,NULL,buffer_texto);
                        else menu_add_item_menu_format(array_menu_hardware_realjoystick_event,MENU_OPCION_NORMAL,menu_hardware_realjoystick_event_button,NULL,buffer_texto);

                        menu_add_item_menu_valor_opcion(array_menu_hardware_realjoystick_event,i);


                        menu_add_item_menu_tooltip(array_menu_hardware_realjoystick_event,"Redefine the action");
                        menu_add_item_menu_ayuda(array_menu_hardware_realjoystick_event,"Redefine the action");
                }

                menu_add_item_menu(array_menu_hardware_realjoystick_event,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_item_menu_format(array_menu_hardware_realjoystick_event,MENU_OPCION_NORMAL,menu_hardware_realjoystick_clear_events,NULL,"Clear list");


                menu_add_item_menu(array_menu_hardware_realjoystick_event,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_realjoystick_event,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_realjoystick_event);

                retorno_menu=menu_dibuja_menu(&hardware_realjoystick_event_opcion_seleccionada,&item_seleccionado,array_menu_hardware_realjoystick_event,"Joystick to events" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


//Variables leidas desde menu para el comprobador de joystick
int menu_info_joystick_last_button;
int menu_info_joystick_last_type;
int menu_info_joystick_last_value;
int menu_info_joystick_last_index;
int menu_info_joystick_last_raw_value;


void menu_hardware_realjoystick_test_reset_last_values(void)
{
	menu_info_joystick_last_button=-1;
	menu_info_joystick_last_type=-1;
	menu_info_joystick_last_value=-1;
	menu_info_joystick_last_index=-1;
	menu_info_joystick_last_raw_value=-1;
}


//Llena string de texto con barras =====|===== segun si valor es entre -32767  y 32768
//Valor 0:      -----|-----
//Valor 32767:  -----|=====
//Valor -32767: =====|-----
//limite_barras dice cuantas barras muestra hacia la derecha o izquierda
void menu_hardware_realjoystick_test_fill_bars(int valor,char *string,int limite_barras)
{
	//Limitar valor entre -32767 y 32767
	if (valor>32767) valor=32767;
	if (valor<-32767) valor=-32767;

	//Cuantas barras hay que hacer
	int barras=(valor*limite_barras)/32767;

	if (barras<0) barras=-barras;

	//String inicial
	int i;
	for (i=0;i<limite_barras;i++) {
		string[i]='-';
		string[i+limite_barras+1]='-';
	}

	//Medio
	string[i]='|';

	//Final
	string[i+limite_barras+1]=0;


	//Y ahora llenar hacia la izquierda o derecha
	int signo=+1;
	if (valor<0) signo=-1;
	int indice=limite_barras+signo; //Nos posicionamos a la derecha o izquierda de la barra central
	for (;barras;barras--) {
		string[indice]='=';
		indice +=signo;
	}

}



#define REALJOYSTICK_TEST_ANCHO 36
#define REALJOYSTICK_TEST_ALTO 16

void menu_hardware_realjoystick_test(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();
    

	zxvision_window ventana;

	int alto_ventana=REALJOYSTICK_TEST_ALTO;
	int ancho_ventana=REALJOYSTICK_TEST_ANCHO;	
	int x_ventana=menu_center_x()-ancho_ventana/2; 
	int y_ventana=menu_center_y()-alto_ventana/2; 	

	zxvision_new_window(&ventana,x_ventana,y_ventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,"Joystick Information");
	zxvision_draw_window(&ventana);			


	z80_byte acumulado;



	int valor_contador_segundo_anterior;

	valor_contador_segundo_anterior=contador_segundo;

	menu_hardware_realjoystick_test_reset_last_values();

	int salir_por_boton=0;


	do {

		menu_cpu_core_loop();
                acumulado=menu_da_todas_teclas();

		//si no hay multitarea, pausar
		if (menu_multitarea==0) {
			usleep(20000); //20 ms
		}


		//Si es evento de salir, forzar el mostrar la info y luego salir
		if (menu_info_joystick_last_button>=0 && menu_info_joystick_last_index==REALJOYSTICK_EVENT_ESC_MENU) {
			//printf ("Salir por boton\n");
			salir_por_boton=1;
		}

        if ( ((contador_segundo%50) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0 || salir_por_boton) {
            valor_contador_segundo_anterior=contador_segundo;
			//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);
			if (menu_multitarea==0) menu_refresca_pantalla();


			char buffer_texto_medio[40];

			int linea=0;
			//int menu_info_joystick_last_button,menu_info_joystick_last_type,menu_info_joystick_last_value,menu_info_joystick_last_index;
			
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Last joystick button/axis:");
			linea++;

		

			//printf ("nuevo evento test joystick\n");

			char buffer_type[40];
#define LONGITUD_BARRAS 14
			char fill_bars[(LONGITUD_BARRAS*2)+2];
			fill_bars[0]=0;
			if (menu_info_joystick_last_type==REALJOYSTICK_INPUT_EVENT_BUTTON) {
				strcpy(buffer_type,"Button");
			}
			else if (menu_info_joystick_last_type==REALJOYSTICK_INPUT_EVENT_AXIS) {
				strcpy(buffer_type,"Axis");
				menu_hardware_realjoystick_test_fill_bars(menu_info_joystick_last_raw_value,fill_bars,LONGITUD_BARRAS);
			}
			else strcpy(buffer_type,"Unknown");

		
			if (menu_info_joystick_last_button<0) strcpy(buffer_texto_medio,"Button: None");
			else sprintf (buffer_texto_medio,"Button: %d",menu_info_joystick_last_button);
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);

			if (menu_info_joystick_last_type<0) strcpy(buffer_texto_medio,"Type: None");
			else sprintf (buffer_texto_medio,"Type: %d (%s)",menu_info_joystick_last_type,buffer_type);
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);



			char buffer_event[40];
			if (menu_info_joystick_last_index>=0 && menu_info_joystick_last_index<MAX_EVENTS_JOYSTICK) {
				strcpy(buffer_event,realjoystick_event_names[menu_info_joystick_last_index]);
			}
			else {
				strcpy(buffer_event,"None");
			}


			sprintf (buffer_texto_medio,"Value: %6d",menu_info_joystick_last_raw_value);
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);

			sprintf (buffer_texto_medio,"%s",fill_bars);
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);


			sprintf (buffer_texto_medio,"Index: %d Event: %s",menu_info_joystick_last_index,buffer_event);
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);

			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");	

			sprintf (buffer_texto_medio,"Driver: %s",realjoystick_driver_name);
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);	

			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Name:");	
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,realjoystick_joy_name);	

			sprintf (buffer_texto_medio,"Total buttons: %d",realjoystick_total_buttons);
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);	

			sprintf (buffer_texto_medio,"Total axis: %d",realjoystick_total_axes);
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);	

 			if (!realjoystick_is_linux_native() ) {
				sprintf (buffer_texto_medio,"Autocalibrate value: %d",realjoystick_autocalibrate_value);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);			
			}



			//realjoystick_ultimo_indice=-1;
			//menu_hardware_realjoystick_test_reset_last_values();
			//menu_info_joystick_last_button=-1;
			//menu_info_joystick_last_type=-1;
			//menu_info_joystick_last_value=-1;
			//menu_info_joystick_last_index=-1;
			//menu_info_joystick_last_raw_value=-1;

			zxvision_draw_window_contents(&ventana);


			
        }

		

        //Hay tecla pulsada
            if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA ) {
                                int tecla=menu_get_pressed_key();

                                                                                                                                
				//Si tecla no es ESC, no salir
                                
				if (tecla!=2) {
					acumulado = MENU_PUERTO_TECLADO_NINGUNA;
				}


				//Si ha salido por boton de joystick, esperar evento
				if (salir_por_boton) {
                    if (menu_multitarea==0) menu_refresca_pantalla();
					menu_espera_no_tecla();
				}
				
                                                                        
			}


        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA);


	
	zxvision_destroy_window(&ventana);

}


void menu_hardware_realjoystick_autocalibrate(MENU_ITEM_PARAMETERS)
{
    char string_calibrate[6];
	int valor;


    sprintf (string_calibrate,"%d",realjoystick_autocalibrate_value);

	menu_ventana_scanf("Autocalibrate value",string_calibrate,6);

	valor=parse_string_to_number(string_calibrate);

	
	if (valor<0 || valor>32000) {
		debug_printf (VERBOSE_ERR,"Value out of range. Minimum: 0 Maximum: 32000");
        return;
    }

	realjoystick_autocalibrate_value=valor;


}

void menu_hardware_realjoystick_set_defaults(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno_texto("Set to defaults","Sure?")==1) {
        realjoystick_new_set_default_functions();
		menu_generic_message("Set to defaults","OK. Events and keys tables set to default values");
    }
}


void menu_hardware_realjoystick_native(MENU_ITEM_PARAMETERS)
{
	no_native_linux_realjoystick.v ^=1;
	menu_generic_message("Linux native driver","OK. You must reopen ZEsarUX to apply this setting");
}





void menu_hardware_realjoystick(MENU_ITEM_PARAMETERS)
{
	menu_item *array_menu_hardware_realjoystick;
	menu_item item_seleccionado;
	int retorno_menu;
	do {

		menu_add_item_menu_en_es_ca_inicial(&array_menu_hardware_realjoystick,MENU_OPCION_NORMAL,menu_hardware_realjoystick_event,NULL,
            "Joystick to ~~events","Joystick a ~~eventos","Joystick a ~~events");
		menu_add_item_menu_shortcut(array_menu_hardware_realjoystick,'e');
		menu_add_item_menu_tooltip(array_menu_hardware_realjoystick,"Define which events generate every button/movement of the joystick");
		menu_add_item_menu_ayuda(array_menu_hardware_realjoystick,"Define which events generate every button/movement of the joystick");
        menu_add_item_menu_tiene_submenu(array_menu_hardware_realjoystick);



		menu_add_item_menu_en_es_ca(array_menu_hardware_realjoystick,MENU_OPCION_NORMAL,menu_hardware_realjoystick_keys,NULL,
            "Joystick to ~~keys","Joystick a te~~klas","Joystick a te~~kles");
		menu_add_item_menu_shortcut(array_menu_hardware_realjoystick,'k');
		menu_add_item_menu_tooltip(array_menu_hardware_realjoystick,"Define which press key generate every button/movement of the joystick");
		menu_add_item_menu_ayuda(array_menu_hardware_realjoystick,"Define which press key generate every button/movement of the joystick");
        menu_add_item_menu_tiene_submenu(array_menu_hardware_realjoystick);


		menu_add_item_menu_en_es_ca(array_menu_hardware_realjoystick,MENU_OPCION_NORMAL,menu_hardware_realjoystick_test,NULL,
            "Joystick ~~information","~~Información del joystick","~~Informació del joystick");
		menu_add_item_menu_shortcut(array_menu_hardware_realjoystick,'i');
		menu_add_item_menu_tooltip(array_menu_hardware_realjoystick,"Joystick information");
		menu_add_item_menu_ayuda(array_menu_hardware_realjoystick,"Joystick information and test tool");

		if (!realjoystick_is_linux_native() ) {
			menu_add_item_menu(array_menu_hardware_realjoystick,"",MENU_OPCION_SEPARADOR,NULL,NULL);
			menu_add_item_menu_en_es_ca(array_menu_hardware_realjoystick,MENU_OPCION_NORMAL,menu_hardware_realjoystick_autocalibrate,NULL,
                "Auto~~calibrate value","Valor Auto~~calibrado","Valor Auto~~calibrat");
            menu_add_item_menu_prefijo_format(array_menu_hardware_realjoystick,"[%5d] ",realjoystick_autocalibrate_value);
			menu_add_item_menu_shortcut(array_menu_hardware_realjoystick,'c');
			menu_add_item_menu_tooltip(array_menu_hardware_realjoystick,"Autocalibrate value");
			menu_add_item_menu_ayuda(array_menu_hardware_realjoystick,"Parameter to autocalibrate joystick axis. "
										"Axis values read from joystick less than n and greater than -n are considered as 0. "
										" Default: 16384. Not used on native linux real joystick");
		}


		//En linux, poder decir si usamos driver nativo o no

#ifdef USE_LINUXREALJOYSTICK

	menu_add_item_menu_en_es_ca(array_menu_hardware_realjoystick,MENU_OPCION_NORMAL,menu_hardware_realjoystick_native,NULL,
        "Linux native driver","Linux driver nativo","Linux driver nadiu");
    menu_add_item_menu_prefijo_format(array_menu_hardware_realjoystick,"[%c] ",(no_native_linux_realjoystick.v ? ' ' : 'X'));
	menu_add_item_menu_tooltip(array_menu_hardware_realjoystick,"Use or not the native linux real joystick support. Instead use the video driver joystick support (currently only SDL)");
	menu_add_item_menu_ayuda(array_menu_hardware_realjoystick,"Use or not the native linux real joystick support. Instead use the video driver joystick support (currently only SDL)");

#endif		


		menu_add_item_menu(array_menu_hardware_realjoystick,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_item_menu_en_es_ca(array_menu_hardware_realjoystick,MENU_OPCION_NORMAL,menu_hardware_realjoystick_set_defaults,NULL,
            "Set events&keys to default","Cambio eventos&teclas a defecto","Canvi events&tecles a defecte");
		menu_add_item_menu_tooltip(array_menu_hardware_realjoystick,"Reset events & keys table to default values");
		menu_add_item_menu_ayuda(array_menu_hardware_realjoystick,"Reset events & keys table to default values");


		menu_add_item_menu(array_menu_hardware_realjoystick,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		
		menu_add_ESC_item(array_menu_hardware_realjoystick);

		retorno_menu=menu_dibuja_menu(&hardware_realjoystick_opcion_seleccionada,&item_seleccionado,array_menu_hardware_realjoystick,"Real joystick support" );

			

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
				//llamamos por valor de funcion
				if (item_seleccionado.menu_funcion!=NULL) {
						//printf ("actuamos por funcion\n");
						item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
						
				}
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


void menu_tape_simulate_real_load(MENU_ITEM_PARAMETERS)
{
	tape_loading_simulate.v ^=1;

	//Al activar carga real, tambien activamos realvideo
	if (tape_loading_simulate.v==1) rainbow_enabled.v=1;
}

void menu_tape_simulate_real_load_fast(MENU_ITEM_PARAMETERS)
{
        tape_loading_simulate_fast.v ^=1;
}


int menu_tape_simulate_real_load_cond(void)
{
	return tape_loading_simulate.v==1;
}

void menu_realtape_volumen(MENU_ITEM_PARAMETERS)
{
	realtape_volumen++;
	if (realtape_volumen==16) realtape_volumen=-16;
}



void menu_tape_any_flag(MENU_ITEM_PARAMETERS)
{
	tape_any_flag_loading.v^=1;
}


void menu_standard_to_real_tape_fallback(MENU_ITEM_PARAMETERS)
{
	standard_to_real_tape_fallback.v ^=1;

}

void menu_realtape_accelerate_loaders(MENU_ITEM_PARAMETERS)
{
	accelerate_loaders.v ^=1;

}

void menu_realtape_loading_sound(MENU_ITEM_PARAMETERS)
{
	realtape_loading_sound.v ^=1;
}

void menu_msx_loading_noise_reduction(MENU_ITEM_PARAMETERS)
{
	msx_loading_noise_reduction.v ^=1;
}

void menu_tape_tzx_suppress_pause(MENU_ITEM_PARAMETERS)
{
    tzx_suppress_pause.v ^=1;
}

void menu_realtape_algorithm_new(MENU_ITEM_PARAMETERS)
{
    realtape_algorithm_new.v ^=1;
}

void menu_realtape_algorithm_new_noise_reduction(MENU_ITEM_PARAMETERS)
{
    menu_ventana_scanf_numero_enhanced("Noise reduction",&realtape_algorithm_new_noise_reduction,4,+1,0,127,0);  
}


void menu_tape_zx8081_disable_tape_traps(MENU_ITEM_PARAMETERS)
{
    zx8081_disable_tape_traps.v ^=1;
}



void menu_realtape_wave_offset(MENU_ITEM_PARAMETERS)
{
        int valor_offset;

        char string_offset[5];


        sprintf (string_offset,"%d",realtape_wave_offset);

        menu_ventana_scanf("Offset",string_offset,5);

        valor_offset=parse_string_to_number(string_offset);

	if (valor_offset<-128 || valor_offset>127) {
		debug_printf (VERBOSE_ERR,"Invalid offset");
		return;
	}

	realtape_wave_offset=valor_offset;

}

void menu_tape_autorewind(MENU_ITEM_PARAMETERS)
{
    tape_auto_rewind.v ^=1;
}

//menu settings tape 
void menu_settings_tape(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_tape;
	menu_item item_seleccionado;
	int retorno_menu;

        do {
                //char string_tape_load_shown[20],string_tape_load_inserted[50],string_tape_save_shown[20],string_tape_save_inserted[50];
		//char string_realtape_shown[23];

		menu_add_item_menu_en_es_ca_inicial(&array_menu_settings_tape,MENU_OPCION_NORMAL,NULL,NULL,
            "--Standard Tape--","--Cinta Estándar--","--Cinta Estàndard");


		menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_standard_to_real_tape_fallback,NULL,
            "Fa~~llback to real tape","Fa~~llback a cinta real","Fa~~llback a cinta real");
        menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ",(standard_to_real_tape_fallback.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_settings_tape,'l');
		menu_add_item_menu_tooltip(array_menu_settings_tape,"If this standard tape is detected as real tape, reinsert tape as real tape");
		menu_add_item_menu_ayuda(array_menu_settings_tape,"While loading the standard tape, if a custom loading routine is detected, "
					"the tape will be ejected from standard tape and inserted it as real tape. If autoload tape is enabled, "
					"the machine will be resetted and loaded the tape from the beginning");


		menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_tape_any_flag,NULL,
            "~~Any flag loading","Carga cu~~alquier flag","Carregar qu~~alsevol flag");
        menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ", (tape_any_flag_loading.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_tape,'a');
		menu_add_item_menu_tooltip(array_menu_settings_tape,"Enables tape load routine to load without knowing block flag");
		menu_add_item_menu_ayuda(array_menu_settings_tape,"Enables tape load routine to load without knowing block flag. You must enable it on Tape Copy programs and also on Rocman game");
        menu_add_item_menu_es_avanzado(array_menu_settings_tape);

		menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_tape_autorewind,NULL,
            "Autorew~~ind","Autorebob~~inar","Autorebob~~inar");
        menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ", (tape_auto_rewind.v ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_tape,'i');
		menu_add_item_menu_tooltip(array_menu_settings_tape,"Autorewind standard tape when reaching end of tape");
        menu_add_item_menu_ayuda(array_menu_settings_tape,"Autorewind standard tape when reaching end of tape");
        menu_add_item_menu_es_avanzado(array_menu_settings_tape);


        menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_tape_simulate_real_load,NULL,
            "~~Simulate real load","~~Simular carga real","~~Simular càrrega real");
        menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ", (tape_loading_simulate.v==1 ? 'X' : ' '));
        menu_add_item_menu_shortcut(array_menu_settings_tape,'s');
        menu_add_item_menu_tooltip(array_menu_settings_tape,"Simulate sound and loading stripes");
        menu_add_item_menu_ayuda(array_menu_settings_tape,"Simulate sound and loading stripes. You can skip simulation pressing any key (and the data is loaded)");
        menu_add_item_menu_es_avanzado(array_menu_settings_tape);

        menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_tape_simulate_real_load_fast,menu_tape_simulate_real_load_cond,
            "Fast Simulate real load","Carga real simulada rápida","Càrrega real simulada ràpida");
        menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ", (tape_loading_simulate_fast.v==1 ? 'X' : ' '));
        menu_add_item_menu_tooltip(array_menu_settings_tape,"Simulate sound and loading stripes at faster speed");
        menu_add_item_menu_ayuda(array_menu_settings_tape,"Simulate sound and loading stripes at faster speed");
        menu_add_item_menu_es_avanzado(array_menu_settings_tape);

        menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_tape_tzx_suppress_pause,NULL,
            "TZX delete pause","TZX eliminar pausa","TZX eliminar pausa");
        menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ", (tzx_suppress_pause.v==1 ? 'X' : ' '));
        menu_add_item_menu_tooltip(array_menu_settings_tape,"Do not follow pauses on TZX tapes");
        menu_add_item_menu_ayuda(array_menu_settings_tape,"Do not follow pauses on TZX tapes");
        menu_add_item_menu_es_avanzado(array_menu_settings_tape);

        if (MACHINE_IS_ZX8081) {
            menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_tape_zx8081_disable_tape_traps,NULL,
                "Tape traps","Traps de cinta","Traps de cinta");
            menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ", (zx8081_disable_tape_traps.v==1 ? ' ' : 'X'));
            menu_add_item_menu_tooltip(array_menu_settings_tape,"Enable tape traps on ZX80/81");
            menu_add_item_menu_ayuda(array_menu_settings_tape,"Enable tape traps on ZX80/81");
        }



        menu_add_item_menu(array_menu_settings_tape,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,NULL,NULL,
            "--Input Real Tape--","--Cinta Real de Entrada--","--Cinta Real d'Entrada--");

		menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_realtape_loading_sound,NULL,
            "Loading sound","Sonido de carga","So de càrrega");
        menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ", (realtape_loading_sound.v==1 ? 'X' : ' '));
		menu_add_item_menu_tooltip(array_menu_settings_tape,"Enable loading sound");
		menu_add_item_menu_ayuda(array_menu_settings_tape,"Enable loading sound. With sound disabled, the tape is also loaded");

		menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_realtape_algorithm_new,NULL,
            "Improved algorithm","Algoritmo mejorado","Algoritme millorat");
        menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ", (realtape_algorithm_new.v==1 ? 'X' : ' '));
		menu_add_item_menu_tooltip(array_menu_settings_tape,"Use improved loading algorithm");
		menu_add_item_menu_ayuda(array_menu_settings_tape,"Use improved loading algorithm. Gives better results with non-zero centered audio tapes but without noise");
        menu_add_item_menu_es_avanzado(array_menu_settings_tape);

        if (realtape_algorithm_new.v) {
            menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_realtape_algorithm_new_noise_reduction,NULL,
                "Noise reduction","Reducción de ruido","Reducció de soroll");
            menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%d] ",realtape_algorithm_new_noise_reduction);
            menu_add_item_menu_tooltip(array_menu_settings_tape,"Noise reduction value");
            menu_add_item_menu_ayuda(array_menu_settings_tape,"Noise reduction value. Set a value >0 when you need to reduce noise");
            menu_add_item_menu_es_avanzado(array_menu_settings_tape);
        }


        else {

            menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_realtape_volumen,NULL,
                "Volume bit 1 range","Rango Volumen bit 1","Rang Volum bit 1");
            menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%s%d] ",(realtape_volumen>0 ? "+" : ""),realtape_volumen);
            menu_add_item_menu_tooltip(array_menu_settings_tape,"Volume bit 1 starting range value");
            menu_add_item_menu_ayuda(array_menu_settings_tape,"The input audio value read (considering range from -128 to +127) is treated "
                        "normally as 1 if the value is in range 0...+127, and 0 if it is in range -127...-1. This setting "
                        "increases this 0 (of range 0...+127) to consider it is a bit 1. I have found this value is better to be 0 "
                        "on Spectrum, and 2 on ZX80/81");
            menu_add_item_menu_es_avanzado(array_menu_settings_tape);
        }



		menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_realtape_wave_offset,NULL,
            "Level Offset","Desplazamiento nivel","Desplaçament nivell");
        menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%d] ",realtape_wave_offset);
		menu_add_item_menu_tooltip(array_menu_settings_tape,"Apply offset to sound value read");
		menu_add_item_menu_ayuda(array_menu_settings_tape,"Indicates some value (positive or negative) to sum to the raw value read "
					"(considering range from -128 to +127) to the input audio value read");
        menu_add_item_menu_es_avanzado(array_menu_settings_tape);


		if (MACHINE_IS_MSX) {
			menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_msx_loading_noise_reduction,NULL,
                "MSX Loading noise reduction","Reducción ruido carga MSX","Reducció soroll càrrega MSX");
			menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ",(msx_loading_noise_reduction.v==1 ? 'X' : ' '));
            menu_add_item_menu_es_avanzado(array_menu_settings_tape);
		}

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_en_es_ca(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_realtape_accelerate_loaders,NULL,
                "A~~ccelerate loaders","A~~celerar cargadores","A~~ccelerar carregadors");
			menu_add_item_menu_prefijo_format(array_menu_settings_tape,"[%c] ",(accelerate_loaders.v==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_tape,'c');
			menu_add_item_menu_tooltip(array_menu_settings_tape,"Set top speed setting when loading a real tape");
			menu_add_item_menu_ayuda(array_menu_settings_tape,"Set top speed setting when loading a real tape");
            menu_add_item_menu_es_avanzado(array_menu_settings_tape);
		}


        menu_add_item_menu(array_menu_settings_tape,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_ESC_item(array_menu_settings_tape);

        retorno_menu=menu_dibuja_menu(&settings_tape_opcion_seleccionada,&item_seleccionado,array_menu_settings_tape,"Tape Settings" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
			//llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


}






zxvision_window menu_zxdesktop_set_userdef_button_func_action_ventana;



//Para hacer un preview del boton
void menu_zxdesktop_set_userdef_button_func_action_putpixel(z80_int *destino GCC_UNUSED,int x,int y,int ancho GCC_UNUSED,int color)
{
	//scr_putpixel(x,y,color);
    //zxvision_window *ventana;
    //ventana=&menu_zxdesktop_set_userdef_button_func_action_ventana;


    zxvision_putpixel(&menu_zxdesktop_set_userdef_button_func_action_ventana,x,y,color);
}


#define ZXDESKTOP_DEFINE_CUSTOM_BUTTONS_ANCHO_VENTANA 29
#define ZXDESKTOP_DEFINE_CUSTOM_BUTTONS_ALTO_VENTANA 20

//Ubicar el boton hacia la derecha de la ventana
#define ZXDESKTOP_DEFINE_CUSTOM_BUTTONS_OFFSET_BUTTON (ZXDESKTOP_DEFINE_CUSTOM_BUTTONS_ANCHO_VENTANA-(ZESARUX_ASCII_LOGO_ANCHO/menu_char_width)-2)


    
void menu_zxdesktop_set_userdef_button_func_action_overlay(void)
{
    

    zxvision_window *ventana;
    ventana=&menu_zxdesktop_set_userdef_button_func_action_ventana;




    menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (ventana->is_minimized) return;


    char **puntero_bitmap;

    int numero_boton=userdef_button_func_action_opcion_seleccionada;

    //Que el número del botón esté dentro del rango total y ademas evitamos el 0 (default)
    if (numero_boton>0 && numero_boton<MAX_F_FUNCTIONS) {
        puntero_bitmap=defined_direct_functions_array[numero_boton].bitmap_button;
        puntero_bitmap=alter_zesarux_ascii_logo(puntero_bitmap);
    
        int offset_x=ZXDESKTOP_DEFINE_CUSTOM_BUTTONS_OFFSET_BUTTON*menu_char_width;
        int offset_y=ventana->offset_y;

        //Desplazar putpixel segun el offset de scroll
        offset_y *=menu_char_height;

        //Primero poner todo el fondo del botón en color blanco
        int x,y;

        for (x=0;x<ZESARUX_ASCII_LOGO_ANCHO;x++) {
            for (y=0;y<ZESARUX_ASCII_LOGO_ALTO;y++) {
                zxvision_putpixel(ventana,offset_x+x,offset_y+y,7);
            }
        }

        //Y dibujar dicho botón
        int nivel_zoom=1;
        screen_put_asciibitmap_generic(puntero_bitmap,NULL,offset_x,offset_y,ZESARUX_ASCII_LOGO_ANCHO,ZESARUX_ASCII_LOGO_ALTO, 
            0,menu_zxdesktop_set_userdef_button_func_action_putpixel,nivel_zoom,0);
    }

    //Siempre hará el dibujado de contenido para evitar que cuando esta en background, otra ventana por debajo escriba algo,
    //y entonces como esta no redibuja siempre, al no escribir encima, se sobreescribe este contenido con el de otra ventana
    //En ventanas que no escriben siempre su contenido, siempre deberia estar zxvision_draw_window_contents que lo haga siempre
    zxvision_draw_window_contents(ventana);        
}


//Seleccionar accion para un boton o para una tecla F, funcion comun a las dos
int menu_zxdesktop_set_userdef_button_func_action(int accion_inicial_seleccionada)
{

    int alto_ventana=ZXDESKTOP_DEFINE_CUSTOM_BUTTONS_ALTO_VENTANA;
    int ancho_ventana=ZXDESKTOP_DEFINE_CUSTOM_BUTTONS_ANCHO_VENTANA;

    int x_ventana=menu_center_x()-ancho_ventana/2;
    int y_ventana=menu_center_y()-alto_ventana/2;


    //En este caso creamos un menu tabulado porque necesitamos crear nosotros la ventana antes para 
    //poderla hacer mas ancha para ubicar el dibujo del boton seleccionado
	zxvision_window *ventana;
    
    ventana=&menu_zxdesktop_set_userdef_button_func_action_ventana;

	zxvision_new_window(ventana,x_ventana,y_ventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,MAX_F_FUNCTIONS+2,"Set Action");

    //Decir que siempre hay que borrar cache al refrescar, especial en el caso de accion por defecto y que no tiene dibujo
    ventana->must_clear_cache_on_draw=1;
	zxvision_draw_window(ventana);		


    //userdef_button_func_action_opcion_seleccionada=defined_buttons_functions_array[valor_opcion];
    userdef_button_func_action_opcion_seleccionada=accion_inicial_seleccionada;


    //cambio overlay
    zxvision_set_window_overlay(ventana,menu_zxdesktop_set_userdef_button_func_action_overlay);    

    menu_item *array_menu_zxdesktop_set_userdef_button_func_action;
    menu_item item_seleccionado;
    int retorno_menu;
    

    char buffer_texto[40];

    int i;
    for (i=0;i<MAX_F_FUNCTIONS;i++) {

        //enum defined_f_function_ids accion=defined_buttons_functions_array[i];

        sprintf (buffer_texto,"%s",defined_direct_functions_array[i].texto_funcion);


        if (i==0) menu_add_item_menu_inicial_format(&array_menu_zxdesktop_set_userdef_button_func_action,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);
        else menu_add_item_menu_format(array_menu_zxdesktop_set_userdef_button_func_action,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);

        menu_add_item_menu_tabulado(array_menu_zxdesktop_set_userdef_button_func_action,1,i);

    }


    //menu_add_item_menu(array_menu_zxdesktop_set_userdef_button_func_action,"",MENU_OPCION_SEPARADOR,NULL,NULL); 
    menu_add_ESC_item(array_menu_zxdesktop_set_userdef_button_func_action);
    menu_add_item_menu_tabulado(array_menu_zxdesktop_set_userdef_button_func_action,1,i+1);

    retorno_menu=menu_dibuja_menu(&userdef_button_func_action_opcion_seleccionada,&item_seleccionado,array_menu_zxdesktop_set_userdef_button_func_action,"Set Action" );

    //restauramos modo normal de texto de menu


    //En caso de menus tabulados, suele ser necesario esto. Si no, la ventana se quedaria visible
    

    //Asumimos que se pulsa ESC
    int indice_retorno=-1;


    if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
        //Si se pulsa Enter
        indice_retorno=userdef_button_func_action_opcion_seleccionada;
        //defined_buttons_functions_array[valor_opcion]=indice;
    }


    //En caso de menus tabulados, es responsabilidad de este de liberar ventana
    zxvision_destroy_window(ventana);

    return indice_retorno;

}


//Definir boton de zx desktop a accion
void menu_zxdesktop_set_userdef_buttons_functions(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_zxdesktop_set_userdef_buttons_functions;
    menu_item item_seleccionado;
    int retorno_menu;
    do {

        menu_add_item_menu_inicial(&array_menu_zxdesktop_set_userdef_buttons_functions,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

        char buffer_texto[40];

        int i;
        for (i=0;i<MAX_USERDEF_BUTTONS;i++) {

            int indice_tabla=defined_buttons_functions_array[i];

            //tabulado todo a misma columna, agregamos un espacio con F entre 1 y 9
            sprintf (buffer_texto,"Button %d %s[%s]",i+1,(i+1<=9 ? " " : ""),defined_direct_functions_array[indice_tabla].texto_funcion);


            //if (i==0) menu_add_item_menu_inicial_format(&array_menu_zxdesktop_set_userdef_buttons_functions,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);
            menu_add_item_menu_format(array_menu_zxdesktop_set_userdef_buttons_functions,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);

            menu_add_item_menu_valor_opcion(array_menu_zxdesktop_set_userdef_buttons_functions,i);

        
        }



        menu_add_item_menu(array_menu_zxdesktop_set_userdef_buttons_functions,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        
        menu_add_ESC_item(array_menu_zxdesktop_set_userdef_buttons_functions);

        retorno_menu=menu_dibuja_menu(&zxdesktop_set_userdef_buttons_functions_opcion_seleccionada,&item_seleccionado,array_menu_zxdesktop_set_userdef_buttons_functions,"Set Buttons" );

        


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion. Se llama a la funcion de elegir accion siempre
            
            //printf ("actuamos por funcion\n");

            int accion_seleccionada=defined_buttons_functions_array[item_seleccionado.valor_opcion];

            int indice_retorno=menu_zxdesktop_set_userdef_button_func_action(accion_seleccionada);

            if (indice_retorno>=0) {
                //printf("definimos boton. boton %d accion %d\n",item_seleccionado.valor_opcion,indice_retorno);
                defined_buttons_functions_array[item_seleccionado.valor_opcion]=indice_retorno;
            }
            
            
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



void menu_hardware_set_f_functions(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_hardware_set_f_functions;
    menu_item item_seleccionado;
    int retorno_menu;

    do {

        menu_add_item_menu_inicial(&array_menu_hardware_set_f_functions,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

        //char buffer_texto[40];

        int i;
        for (i=0;i<MAX_F_FUNCTIONS_KEYS;i++) {

            int indice_tabla=defined_f_functions_keys_array[i];

            //tabulado todo a misma columna, agregamos un espacio con F entre 1 y 9
            //sprintf (buffer_texto,"Key F%d %s[%s]",i+1,(i+1<=9 ? " " : ""),defined_direct_functions_array[indice_tabla].texto_funcion);

            //menu_add_item_menu_format(array_menu_hardware_set_f_functions,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);

            menu_add_item_menu_en_es_ca(array_menu_hardware_set_f_functions,MENU_OPCION_NORMAL,NULL,NULL,
                "Key F","Tecla F","Tecla F");

            //tabulado todo a misma columna, agregamos un espacio con F entre 1 y 9
            menu_add_item_menu_sufijo_format(array_menu_hardware_set_f_functions,
                "%d %s[%s]",i+1,(i+1<=9 ? " " : ""),defined_direct_functions_array[indice_tabla].texto_funcion);

            menu_add_item_menu_valor_opcion(array_menu_hardware_set_f_functions,i);

            //Algunas acciones que permiten extra info
            if (
                defined_direct_functions_array[indice_tabla].id_funcion==F_FUNCION_OPEN_WINDOW ||
                defined_direct_functions_array[indice_tabla].id_funcion==F_FUNCION_DESKTOP_SNAPSHOT ||
                defined_direct_functions_array[indice_tabla].id_funcion==F_FUNCION_DESKTOP_TAPE ||
                defined_direct_functions_array[indice_tabla].id_funcion==F_FUNCION_DESKTOP_GENERIC_SMARTLOAD 
            ) {
                char string_extra_info[16];
                menu_tape_settings_trunc_name(defined_f_functions_keys_array_parameters[i],string_extra_info,16);

                menu_add_item_menu_en_es_ca(array_menu_hardware_set_f_functions,MENU_OPCION_NORMAL,NULL,NULL,
                    " Parameters"," Parámetros"," Paràmetres");
                menu_add_item_menu_sufijo_format(array_menu_hardware_set_f_functions,": %s",string_extra_info);
                menu_add_item_menu_tooltip(array_menu_hardware_set_f_functions,"Parameters for some actions, like window name for OpenWindow action");
                menu_add_item_menu_ayuda(array_menu_hardware_set_f_functions,"Parameters for some actions, like window name for OpenWindow action");

                //Esto es un poco chapuza... para indicar que es Parameters, la opcion tiene bit 8 alzado
                menu_add_item_menu_valor_opcion(array_menu_hardware_set_f_functions,i | 256);
            }
        }



        menu_add_item_menu(array_menu_hardware_set_f_functions,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        
        menu_add_ESC_item(array_menu_hardware_set_f_functions);

        retorno_menu=menu_dibuja_menu(&hardware_set_f_functions_opcion_seleccionada,&item_seleccionado,array_menu_hardware_set_f_functions,"Set Function keys");

                


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //Se llama a la funcion de elegir accion o establecer extra info
            if (item_seleccionado.valor_opcion>=256) {
                //Establecer extra info
                //char defined_f_functions_keys_array_parameters[MAX_F_FUNCTIONS_KEYS][PATH_MAX]={

                int indice=item_seleccionado.valor_opcion & 0xFF;

                menu_ventana_scanf("Extra info",defined_f_functions_keys_array_parameters[indice],PATH_MAX);
	            
            }

            else {
            
                //printf ("actuamos por funcion\n");

                int accion_seleccionada=defined_f_functions_keys_array[item_seleccionado.valor_opcion];

                //hardware_set_f_func_action_opcion_seleccionada=defined_f_functions_keys_array[valor_opcion];

                int indice_retorno=menu_zxdesktop_set_userdef_button_func_action(accion_seleccionada);

                if (indice_retorno>=0) {
                    //printf("definimos fkey. tecla f %d accion %d\n",item_seleccionado.valor_opcion,indice_retorno);
                    defined_f_functions_keys_array[item_seleccionado.valor_opcion]=indice_retorno;
                }
            }
            
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}




void menu_ext_desk_settings_enable(MENU_ITEM_PARAMETERS)
{
	
	debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);



	screen_ext_desktop_enabled ^=1;

        

	screen_init_pantalla_and_others_and_realjoystick();


    debug_printf(VERBOSE_INFO,"Creating Screen");

	menu_init_footer();

	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);	


	//Cerrar ventamas y olvidar geometria ventanas
	zxvision_window_delete_all_windows_and_clear_geometry();

    if (screen_ext_desktop_enabled) {
        //Reordenar iconos por si habia alguno antes
        //zxvision_reorder_configurable_icons();

        //Comprobar posiciones iconos y reajustar
        zxvision_check_all_configurable_icons_positions();

        //Crear iconos por defecto si hace falta
        create_default_zxdesktop_configurable_icons();
    }

	cls_menu_overlay();


}

void menu_ext_desk_settings_custom_width_height(int reorganize_windows)
{

	debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);
       

	screen_init_pantalla_and_others_and_realjoystick();

    debug_printf(VERBOSE_INFO,"Creating Screen");

	menu_init_footer();

	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);	

	//Cerrar ventamas y olvidar geometria ventanas
	//zxvision_window_delete_all_windows_and_clear_geometry();

	//Reorganizar ventanas solo si conviene (cuando tamaño pasa a ser menor)
	if (reorganize_windows)	{
        zxvision_rearrange_background_windows();

        //Comprobar posiciones iconos y reajustar
        zxvision_check_all_configurable_icons_positions();        
    }

	//Conveniente esto para borrar "restos" de ventanas
	cls_menu_overlay();


}



void menu_ext_desk_settings_custom_width(MENU_ITEM_PARAMETERS)
{

	int reorganize_windows=0;

	char string_width[5];

	sprintf (string_width,"%d",zxdesktop_width);


	menu_ventana_scanf("Width",string_width,5);

	int valor=parse_string_to_number(string_width);

	if (valor<128 || valor>9999) {
		debug_printf (VERBOSE_ERR,"Invalid value");
		return;
	}


	if (valor<zxdesktop_width) reorganize_windows=1;

	zxdesktop_width=valor;


    menu_ext_desk_settings_custom_width_height(reorganize_windows);


}


void menu_ext_desk_settings_custom_height(MENU_ITEM_PARAMETERS)
{

	int reorganize_windows=0;

	char string_height[5];

	sprintf (string_height,"%d",zxdesktop_height);


	menu_ventana_scanf("Height",string_height,5);

	int valor=parse_string_to_number(string_height);

	if (valor<0 || valor>9999) {
		debug_printf (VERBOSE_ERR,"Invalid value");
		return;
	}

    //Valor tiene que ser multiple de 8, si no, el footer no se ve bien
    valor &=(65535-7);


	if (valor<zxdesktop_height) reorganize_windows=1;

	zxdesktop_height=valor;


    menu_ext_desk_settings_custom_width_height(reorganize_windows);


}

void menu_ext_desk_settings_width(MENU_ITEM_PARAMETERS)
{

    menu_ext_desk_settings_width_enlarge_reduce(1);
}

void menu_ext_desk_settings_height(MENU_ITEM_PARAMETERS)
{

    menu_ext_desk_settings_height_enlarge_reduce(1);
}

void menu_ext_desk_settings_filltype(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_fill++;
	if (menu_ext_desktop_fill>MENU_MAX_EXT_DESKTOP_FILL_NUMBER) menu_ext_desktop_fill=0;
}

void menu_ext_desk_settings_fillcolor(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_fill_first_color++;
	if (menu_ext_desktop_fill_first_color==16) menu_ext_desktop_fill_first_color=0;
}

void menu_ext_desk_settings_fillcolor_second(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_fill_second_color++;
	if (menu_ext_desktop_fill_second_color==16) menu_ext_desktop_fill_second_color=0;
}

/*
int menu_ext_desktop_fill=1;
int menu_ext_desktop_fill_first_color=1;
*/

void menu_ext_desk_settings_placemenu(MENU_ITEM_PARAMETERS)
{
	screen_ext_desktop_place_menu ^=1;	
}

void menu_ext_desk_settings_direct_buttons(MENU_ITEM_PARAMETERS)
{
	menu_zxdesktop_buttons_enabled.v ^=1;
}

void menu_ext_desk_settings_upper_transparent(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_transparent_upper_icons.v ^=1;
}

void menu_ext_desk_settings_lower_transparent(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_transparent_lower_icons.v ^=1;
}

void menu_ext_desk_settings_configurable_icons_transparent(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_transparent_configurable_icons.v ^=1;
}

void menu_ext_desk_settings_configurable_icons_text_background(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_configurable_icons_text_background.v ^=1;
}


void menu_ext_desk_settings_upper_box(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_disable_box_upper_icons.v ^=1;
}

void menu_ext_desk_settings_lower_box(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_disable_box_lower_icons.v ^=1;
}

int menu_ext_desktop_cond(void)
{
	if (ventana_fullscreen) return 0;
	return 1;
}

void menu_ext_desk_settings_switch_button(MENU_ITEM_PARAMETERS)
{
    zxdesktop_switch_button_enabled.v ^=1;
}

void menu_zxdesktop_scrfile(MENU_ITEM_PARAMETERS)
{
    char *filtros[13];

    filtros[0]="scr";
    filtros[1]="tap";
    filtros[2]="tzx";
    filtros[3]="pzx";
    filtros[4]="trd";
    filtros[5]="ddh";
    filtros[6]="dsk";
    filtros[7]="zsf";
    filtros[8]="sna";
    filtros[9]="sp";
    filtros[10]="z80";
    filtros[11]="p";
    filtros[12]=0;


    //guardamos directorio actual
    char directorio_actual[PATH_MAX];
    getcwd(directorio_actual,PATH_MAX);

    //Obtenemos directorio de cinta
    //si no hay directorio, vamos a rutas predefinidas
    if (zxdesktop_draw_scrfile_name[0]==0) menu_chdir_sharedfiles();

    else {
        char directorio[PATH_MAX];
        util_get_dir(zxdesktop_draw_scrfile_name,directorio);
        //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

        //cambiamos a ese directorio, siempre que no sea nulo
        if (directorio[0]!=0) {
            debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
            zvfs_chdir(directorio);
        }
    }



    //int ret;

    //ret=menu_filesel("Select file",filtros,zxdesktop_draw_scrfile_name);
    menu_filesel("Select file",filtros,zxdesktop_draw_scrfile_name);
    //volvemos a directorio inicial
    zvfs_chdir(directorio_actual);

    //Si esta activo, lo cargamos
    if (zxdesktop_draw_scrfile_enabled) zxdesktop_draw_scrfile_load();

 
}

void menu_zxdesktop_scrfile_enable(MENU_ITEM_PARAMETERS)
{
    zxdesktop_draw_scrfile_enabled ^=1;

    if (zxdesktop_draw_scrfile_enabled) zxdesktop_draw_scrfile_load();
}

void menu_zxdesktop_scrfile_centered(MENU_ITEM_PARAMETERS)
{
    zxdesktop_draw_scrfile_centered ^=1;
}

void menu_zxdesktop_scrfile_fillscale(MENU_ITEM_PARAMETERS)
{
    zxdesktop_draw_scrfile_fill_scale ^=1;
}

void menu_zxdesktop_scrfile_mix_background(MENU_ITEM_PARAMETERS)
{
    zxdesktop_draw_scrfile_mix_background ^=1;
}

void menu_zxdesktop_scrfile_scalefactor(MENU_ITEM_PARAMETERS)
{
    menu_ventana_scanf_numero_enhanced("Scale factor",&zxdesktop_draw_scrfile_scale_factor,2,+1,1,5,0);
}

void menu_ext_desk_settings_disable_on_fullscreen(MENU_ITEM_PARAMETERS)
{
    zxdesktop_disable_on_full_screen ^=1;
}

void menu_zxdesktop_scrfile_disable_flash(MENU_ITEM_PARAMETERS)
{
    zxdesktop_draw_scrfile_disable_flash ^=1;

    zxdesktop_draw_scrfile_load();
}

void menu_ext_desk_settings_frame_emulated_display(MENU_ITEM_PARAMETERS)
{
    zxdesktop_disable_show_frame_around_display ^=1;
}

void menu_zxdesktop_add_configurable_icons(MENU_ITEM_PARAMETERS)
{
    int indice_retorno=menu_zxdesktop_set_userdef_button_func_action(0);

    if (indice_retorno>=0) {    
        int indice_icono=zxvision_add_configurable_icon(indice_retorno);
    

        if (indice_icono>=0) {
            //Si es icono de machine selection, indicarle como parametro la maquina actual
            enum defined_f_function_ids accion=menu_da_accion_direct_functions_indice(indice_retorno);

            if (accion==F_FUNCION_SET_MACHINE) {
                char buffer_maquina[100];

                get_machine_config_name_by_number(buffer_maquina,current_machine_type);
                if (buffer_maquina[0]!=0) {
                    strcpy(zxdesktop_configurable_icons_list[indice_icono].extra_info,buffer_maquina);

                    //Cambiar nombre icono con la maquina en cuestión
                    sprintf(zxdesktop_configurable_icons_list[indice_icono].text_icon,"Set %s",get_machine_name(current_machine_type));

                }
            }
        }
    }
}

void menu_zxdesktop_set_configurable_icons_choose(MENU_ITEM_PARAMETERS)
{

    int icono_seleccionado=valor_opcion;

    int indice_seleccionada=zxdesktop_configurable_icons_list[icono_seleccionado].indice_funcion;

    int indice_retorno=menu_zxdesktop_set_userdef_button_func_action(indice_seleccionada);

    if (indice_retorno>=0) {
        //printf("definimos boton. accion %d\n",indice_retorno);
        //int id_funcion=defined_direct_functions_array[accion_seleccionada].id_funcion;
        //Asignar funcion
        zxdesktop_configurable_icons_list[icono_seleccionado].indice_funcion=indice_retorno;

        //Asignar nombre accion
        strcpy(zxdesktop_configurable_icons_list[icono_seleccionado].text_icon,defined_direct_functions_array[indice_retorno].texto_funcion);

        //Si ya existia, conservar posicion. Si no, poner una nueva
        if (zxdesktop_configurable_icons_list[icono_seleccionado].status==ZXDESKTOP_CUSTOM_ICON_NOT_EXISTS) {
            zxvision_set_configurable_icon_position(icono_seleccionado,430,110);
            zxdesktop_configurable_icons_list[icono_seleccionado].status=ZXDESKTOP_CUSTOM_ICON_EXISTS; 
        }
    }    


}

void menu_zxdesktop_set_configurable_icons_rename(MENU_ITEM_PARAMETERS)
{
    menu_ventana_scanf("Rename",zxdesktop_configurable_icons_list[valor_opcion].text_icon,MAX_LENGTH_TEXT_ICON);    
}

void menu_zxdesktop_set_configurable_icons_change_parameters(MENU_ITEM_PARAMETERS)
{
    menu_ventana_scanf("Parameters",zxdesktop_configurable_icons_list[valor_opcion].extra_info,PATH_MAX);
}

void menu_zxdesktop_set_configurable_icons_move_trash(MENU_ITEM_PARAMETERS)
{
    zxvision_move_configurable_icon_to_trash(valor_opcion);
}

void menu_zxdesktop_set_configurable_icons_view(MENU_ITEM_PARAMETERS)
{
    menu_file_viewer_read_file("Text file view",zxdesktop_configurable_icons_list[valor_opcion].extra_info);    
}

void menu_zxdesktop_set_configurable_icons_info(MENU_ITEM_PARAMETERS)
{
    file_utils_info_file(zxdesktop_configurable_icons_list[valor_opcion].extra_info);
}


void menu_zxdesktop_set_configurable_icons_modify(MENU_ITEM_PARAMETERS)
{

    int indice_funcion=zxdesktop_configurable_icons_list[valor_opcion].indice_funcion;
    enum defined_f_function_ids id_funcion=menu_da_accion_direct_functions_indice(indice_funcion);


    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int opcion_seleccionada=0;
    int retorno_menu;

    //No estar en bucle
    //do {

        menu_add_item_menu_inicial(&array_menu_common,"Change type",MENU_OPCION_NORMAL,menu_zxdesktop_set_configurable_icons_choose,NULL);
        menu_add_item_menu_spanish_catalan(array_menu_common,"Cambiar tipo","Canviar tipus");
        menu_add_item_menu_valor_opcion(array_menu_common,valor_opcion);
        //menu_add_item_menu_shortcut(array_menu_common,'t');


        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zxdesktop_set_configurable_icons_rename,NULL,
            "R~~ename","R~~enombrar","R~~enombrar");
        menu_add_item_menu_valor_opcion(array_menu_common,valor_opcion);
        menu_add_item_menu_shortcut(array_menu_common,'e');        

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zxdesktop_set_configurable_icons_change_parameters,NULL,
            "~~Parameters","~~Parámetros","~~Paràmetres");
        menu_add_item_menu_valor_opcion(array_menu_common,valor_opcion);
        menu_add_item_menu_shortcut(array_menu_common,'p');   


        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zxdesktop_set_configurable_icons_move_trash,NULL,
            "Move to Trash","Mover a Papelera","Moure a Paperera");
        menu_add_item_menu_valor_opcion(array_menu_common,valor_opcion);
        //menu_add_item_menu_shortcut(array_menu_common,'m');   


        //Si el icono es enlace a archivo
        if (id_funcion==F_FUNCION_DESKTOP_SNAPSHOT ||
            id_funcion==F_FUNCION_DESKTOP_TAPE ||
            id_funcion==F_FUNCION_DESKTOP_GENERIC_SMARTLOAD) {

            menu_add_item_menu_separator(array_menu_common);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zxdesktop_set_configurable_icons_view,NULL,
                "~~View","~~Ver","~~Veure");
            menu_add_item_menu_valor_opcion(array_menu_common,valor_opcion);
            menu_add_item_menu_shortcut(array_menu_common,'v');            

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zxdesktop_set_configurable_icons_info,NULL,
                "~~Info","~~Info","~~Info");
            menu_add_item_menu_valor_opcion(array_menu_common,valor_opcion);
            menu_add_item_menu_shortcut(array_menu_common,'i');              
            
        }  

        //Si icono es my machine
        if (id_funcion==F_FUNCION_DESKTOP_MY_MACHINE) {

            menu_add_item_menu_separator(array_menu_common);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_machine_selection,NULL,
                "Select ~~Machine","Elige ~~Máquina","Escull ~~Màquina");
            menu_add_item_menu_shortcut(array_menu_common,'m');     

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_storage,NULL,
                "S~~torage","Almacenamien~~to","Emmagatzemamen~~t");
            menu_add_item_menu_shortcut(array_menu_common,'t');      

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_hardware_settings,NULL,
                "~~Hardware settings","Opciones ~~Hardware","Opcions ~~Hardware");
            menu_add_item_menu_shortcut(array_menu_common,'h');            

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_debug_registers,NULL,
                "Debug ~~CPU","Debug ~~CPU","Debug ~~CPU");
            menu_add_item_menu_shortcut(array_menu_common,'c');   

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_debug_reset,NULL,
                "~~Reset CPU","~~Reset CPU","~~Reset CPU");
            menu_add_item_menu_valor_opcion(array_menu_common,valor_opcion);
            menu_add_item_menu_shortcut(array_menu_common,'r');    

        }

        //Si icono es Papelera
        if (id_funcion==F_FUNCION_DESKTOP_TRASH) {

            menu_add_item_menu_separator(array_menu_common);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zxdesktop_trash_empty,NULL,
                "Empty Trash","Vaciar Papelera","Buidar Paperera");

        }

        menu_add_item_menu_separator(array_menu_common);
        menu_add_ESC_item(array_menu_common);
        retorno_menu=menu_dibuja_menu(&opcion_seleccionada,&item_seleccionado,array_menu_common,"Properties" );

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                
            }
        }

    //No estar en bucle
    //} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    //Y decir que el siguiente menu ya no se abre desde boton y por tanto no se posiciona debajo del boton
    //Antes se quitaba el flag tambien en menu_dibuja_menu, pero ya no. Asi conseguimos que todos los menus
    //que se abran dependiendo del boton, queden debajo de dicho boton
    force_next_menu_position.v=0;    


}    

//Definir icono de zx desktop a accion
void menu_zxdesktop_set_configurable_icons(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    do {

        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zxdesktop_add_configurable_icons,NULL,"Add next");

        char buffer_texto[100];

        int i;
        for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {

            //int indice_tabla=defined_buttons_functions_array[i];

            int indice_funcion=zxdesktop_configurable_icons_list[i].indice_funcion;
            char estado_icono[30];

            if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_NOT_EXISTS) {
                strcpy(estado_icono,"Not Exists");
            }
            else if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_EXISTS) {
                strcpy(estado_icono,"Exists ");
            }
            else if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_DELETED) {
                strcpy(estado_icono,"Deleted");
            }
            else {
                //otros casos??
                strcpy(estado_icono,"Unknown");
            }

            sprintf (buffer_texto,"Icon %2d %4d,%4d %s [%s]",i,zxdesktop_configurable_icons_list[i].pos_x,zxdesktop_configurable_icons_list[i].pos_y,
                    estado_icono,defined_direct_functions_array[indice_funcion].texto_funcion);


            //if (i==0) menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zxdesktop_set_configurable_icons_modify,NULL,buffer_texto);

            menu_add_item_menu_valor_opcion(array_menu_common,i);

        
        }



        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        
        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&zxdesktop_set_configurable_icons_opcion_seleccionada,&item_seleccionado,array_menu_common,"Set Icons" );

        



        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

void menu_ext_desk_settings_configurable_icons_enabled(MENU_ITEM_PARAMETERS)
{
    zxdesktop_configurable_icons_enabled.v ^=1;

    if (zxdesktop_configurable_icons_enabled_and_visible()) {
        //Crear iconos por defecto si hace falta
        create_default_zxdesktop_configurable_icons();    
    }
}

void menu_zxdesktop_degraded_inverted(MENU_ITEM_PARAMETERS)
{
    menu_ext_desktop_degraded_inverted.v ^=1;
}

void menu_ext_desk_settings_empty_trash_exit(MENU_ITEM_PARAMETERS)
{
    zxdesktop_empty_trash_on_exit.v ^=1;
}

void menu_ext_desk_settings_show_app_open(MENU_ITEM_PARAMETERS)
{
    zxdesktop_icon_show_app_open.v ^=1;
}

void menu_ext_desktop_settings(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_ext_desktop_settings;
    menu_item item_seleccionado;
    int retorno_menu;

    do {

        menu_add_item_menu_inicial_format(&array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_enable,menu_ext_desktop_cond,"Enabled");
        menu_add_item_menu_spanish_catalan(array_menu_ext_desktop_settings,"Activado","Activat");
        menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(screen_ext_desktop_enabled ? 'X' : ' ' ) );

		if (screen_ext_desktop_enabled) {
			menu_add_item_menu_format(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_width,menu_ext_desktop_cond,"~~Width");
            menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%4d] ",zxdesktop_width);
            menu_add_item_menu_shortcut(array_menu_ext_desktop_settings,'w');
			menu_add_item_menu_tooltip(array_menu_ext_desktop_settings,"Tells the width of the ZX Desktop space");
			menu_add_item_menu_ayuda(array_menu_ext_desktop_settings,"Final width is this value in pixels X current horizontal zoom");
            menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);

			menu_add_item_menu_format(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_custom_width,menu_ext_desktop_cond,"~~Custom Width");
            menu_add_item_menu_shortcut(array_menu_ext_desktop_settings,'c');
            menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);      

			menu_add_item_menu_format(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_height,menu_ext_desktop_cond,"~~Height");
            menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%4d] ",zxdesktop_height);
            menu_add_item_menu_shortcut(array_menu_ext_desktop_settings,'h');
			menu_add_item_menu_tooltip(array_menu_ext_desktop_settings,"Tells the height of the ZX Desktop space");
			menu_add_item_menu_ayuda(array_menu_ext_desktop_settings,"Final height is this value in pixels X current vertical zoom");
            menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);            

			menu_add_item_menu_format(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_custom_height,menu_ext_desktop_cond,"C~~ustom Height");
            menu_add_item_menu_shortcut(array_menu_ext_desktop_settings,'u');
            menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);

            menu_add_item_menu_separator(array_menu_ext_desktop_settings);      
            menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);
		
            menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_disable_on_fullscreen,NULL,
                "Disable on Full Screen","Desactivar en pantalla completa","Desactivar a pantalla completa");
            menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(zxdesktop_disable_on_full_screen ? 'X' : ' ' ));
            menu_add_item_menu_tooltip(array_menu_ext_desktop_settings,"Disable ZX Desktop when going to full screen");
            menu_add_item_menu_ayuda(array_menu_ext_desktop_settings,"Disable ZX Desktop when going to full screen. It will be enabled again going back from full screen");
            menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);
	
			menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_placemenu,NULL,
                "Open Menu on ZX Desktop","Abrir menu en ZX Desktop","Obrir menu al ZX Desktop");
            menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(screen_ext_desktop_place_menu ? 'X' : ' ' ) );
			menu_add_item_menu_tooltip(array_menu_ext_desktop_settings,"Try to place new menu items on the ZX Desktop space");
			menu_add_item_menu_ayuda(array_menu_ext_desktop_settings,"Try to place new menu items on the ZX Desktop space");
            menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);
        }

        menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_switch_button,NULL,
            "Footer +/- buttons","Botones +/- en pie de pagina","Botons +/- al peu de pagina");
        menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(zxdesktop_switch_button_enabled.v ? 'X' : ' ' ) );
        menu_add_item_menu_tooltip(array_menu_ext_desktop_settings,"Enable buttons on footer to enlarge/reduce ZX Desktop (visible when menu closed)");
        menu_add_item_menu_ayuda(array_menu_ext_desktop_settings,"Enable buttons on footer to enlarge/reduce ZX Desktop (visible when menu closed)");        
        menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);


        if (screen_get_ext_desktop_height_zoom()>=ZXDESKTOP_MINIMUM_HEIGHT_SHOW_FRAME) {
            menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_frame_emulated_display,NULL,
                "Frame on emulated display","Recuadro en pantalla emulada","Requadre a la pantalla emulada");
            menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(!zxdesktop_disable_show_frame_around_display ? 'X' : ' ' ) );
            menu_add_item_menu_tooltip(array_menu_ext_desktop_settings,"Enable showing a frame around the emulated machine display");
            menu_add_item_menu_ayuda(array_menu_ext_desktop_settings,"Enable showing a frame around the emulated machine display");
            menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);
        }

        if (screen_ext_desktop_enabled) {

            menu_add_item_menu_separator(array_menu_ext_desktop_settings);

			menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_direct_buttons,NULL,
                "~~Direct access buttons","Botones de acceso ~~directo","Botons d'access ~~directe");
            menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(menu_zxdesktop_buttons_enabled.v ? 'X' : ' ' ) );
            menu_add_item_menu_shortcut(array_menu_ext_desktop_settings,'d');

			if (menu_zxdesktop_buttons_enabled.v) {
				menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_upper_transparent,NULL,
                    "Transparent upper buttons","Botones superiores transparentes","Botons superiors transparents");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(menu_ext_desktop_transparent_upper_icons.v ? 'X' : ' ' ) );
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);

                menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_upper_box,NULL,
                    "Box on upper buttons","Caja en botones superiores","Caixa als botons superiors");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(menu_ext_desktop_disable_box_upper_icons.v ? ' ' : 'X' ) );
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);

                menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_zxdesktop_set_userdef_buttons_functions,NULL,
                    "    Customize ~~buttons","    Personalizar ~~botones","    Personalitzar ~~botons");
                menu_add_item_menu_shortcut(array_menu_ext_desktop_settings,'b');
                menu_add_item_menu_tiene_submenu(array_menu_ext_desktop_settings);
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);

                menu_add_item_menu_separator(array_menu_ext_desktop_settings);
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);




				menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_lower_transparent,NULL,
                    "Transparent lower buttons","Botones inferiores transparentes","Botons inferiors transparents");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(menu_ext_desktop_transparent_lower_icons.v ? 'X' : ' ' ) );
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);

                menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_lower_box,NULL,
                    "Box on lower buttons","Caja en botones inferiores","Caixa als botons inferiors");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(menu_ext_desktop_disable_box_lower_icons.v ? ' ' : 'X' ) );
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);

			}

			menu_add_item_menu_separator(array_menu_ext_desktop_settings);


            menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_configurable_icons_enabled,NULL,
                "Icons on ZX Desktop","Iconos en ZX Desktop","Icones al ZX Desktop");
            menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(zxdesktop_configurable_icons_enabled.v ? 'X' : ' ' ) );

                
            if (zxdesktop_configurable_icons_enabled.v) {

                menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_empty_trash_exit,NULL,
                    "Empty trash on exit","Vaciar papelera al salir","Buidar paperera al sortir");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(zxdesktop_empty_trash_on_exit.v ? 'X' : ' ' ) );
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);                

                menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_configurable_icons_transparent,NULL,
                    "Transparent icons","Iconos transparentes","Icones transparents");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(menu_ext_desktop_transparent_configurable_icons.v ? 'X' : ' ' ) );                              
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);

                menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_configurable_icons_text_background,NULL,
                    "Icon text background","Fondo de texto de iconos","Fons de text de icones");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(menu_ext_desktop_configurable_icons_text_background.v ? 'X' : ' ' ) );                              
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);

                menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_show_app_open,NULL,
                    "Show indicators for open apps","Mostrar indicadores en apps abiertas","Mostrar indicadors en apps obertes");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(zxdesktop_icon_show_app_open.v ? 'X' : ' ' ) );                              
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);


                menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_zxdesktop_set_configurable_icons,NULL,
                    "    Modify icons","    Modificar iconos","    Modificar icones");
                menu_add_item_menu_tiene_submenu(array_menu_ext_desktop_settings);  
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);

            }

            menu_add_item_menu_separator(array_menu_ext_desktop_settings);            
			
			menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_SEPARADOR,NULL,NULL,
                "--Background--","--Fondo--","--Fons--");
			
            char fill_type_name[32];
			int seleccion_primary=0;
			int seleccion_secondary=0;
			switch (menu_ext_desktop_fill) {
				//solid, rainbow, punteado, ajedrez
				case 0:
					strcpy(fill_type_name,"Solid");
					seleccion_primary=1;
				break;

				case 1:
					strcpy(fill_type_name,"Rainbow");
				break;

				case 2:
					strcpy(fill_type_name,"RainbowAlive");
				break;				

				case 3:
					strcpy(fill_type_name,"Dots");
					seleccion_primary=1;
					seleccion_secondary=1;
				break;

				case 4:
					strcpy(fill_type_name,"Chess");
					seleccion_primary=1;
					seleccion_secondary=1;					
				break;

				case 5:
					strcpy(fill_type_name,"Grid");
					seleccion_primary=1;
					seleccion_secondary=1;					
				break;				

				case 6:
					strcpy(fill_type_name,"Random");
				break;		

				case 7:
					strcpy(fill_type_name,"Degraded");
                    seleccion_primary=1;
				break;


				default:
					strcpy(fill_type_name,"Unknown");
				break;

			}

            menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_filltype,NULL,
                "~~Fill type","Tipo ~~fill","Tipus ~~farcit");
            menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%s] ",fill_type_name);
            menu_add_item_menu_shortcut(array_menu_ext_desktop_settings,'f');
			
			if (seleccion_primary) {
                //en tipo degraded, no tiene sentido mostrar los colores bright
                int color_primario=menu_ext_desktop_fill_first_color;
                if (menu_ext_desktop_fill==7) color_primario &=7;
				menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_fillcolor,NULL,
                        "Primary Fill Color","Color primario relleno","Color primari de farciment");
				menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%s] ",spectrum_colour_names[color_primario]);
			}

            if (menu_ext_desktop_fill==7) {
                menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_zxdesktop_degraded_inverted,NULL,
                    "Inverted Degraded","Degradado invertido","Degradat invertit");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(menu_ext_desktop_degraded_inverted.v ? 'X' : ' ' ));
                menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);                
            }

			if (seleccion_secondary) {
				menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_fillcolor_second,NULL,
                    "Secondary Fill Color","Color secundario relleno","Color secundari de farciment");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%s] ",spectrum_colour_names[menu_ext_desktop_fill_second_color]);
			}


            char string_back_scr_shown[20];
            menu_tape_settings_trunc_name(zxdesktop_draw_scrfile_name,string_back_scr_shown,20);
            menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_zxdesktop_scrfile,NULL,
                "~~SCR file","archivo ~~SCR","arxiu ~~SCR");
            menu_add_item_menu_sufijo_format(array_menu_ext_desktop_settings," [%s]",string_back_scr_shown);
            menu_add_item_menu_shortcut(array_menu_ext_desktop_settings,'s');         
            menu_add_item_menu_tooltip(array_menu_ext_desktop_settings,"Sets a SCR file for ZX Desktop background. Flash attributes are not used");
            menu_add_item_menu_ayuda(array_menu_ext_desktop_settings,"Sets a SCR file for ZX Desktop background. Flash attributes are not used");
          
			if (zxdesktop_draw_scrfile_name[0]!=0) {
                menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_zxdesktop_scrfile_enable,NULL,
                    "SCR active","SCR activo","SCR actiu");
                menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(zxdesktop_draw_scrfile_enabled ? 'X' : ' ' ));


                if (zxdesktop_draw_scrfile_enabled) {

                    menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_zxdesktop_scrfile_centered,NULL,
                        "SCR centered","SCR centrado","SCR centrat");
                    menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(zxdesktop_draw_scrfile_centered ? 'X' : ' ' ));


                    menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_zxdesktop_scrfile_fillscale,NULL,
                        "SCR autoscale","SCR autoescalar","SCR autoescalar");
                    menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(zxdesktop_draw_scrfile_fill_scale ? 'X' : ' ' ));


                    if (!zxdesktop_draw_scrfile_fill_scale) {
                        menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_zxdesktop_scrfile_scalefactor,NULL,
                            "SCR scale factor","SCR factor de escala","SCR factor d'escala");
                        menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%d] ",zxdesktop_draw_scrfile_scale_factor);

                    }

                    menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_zxdesktop_scrfile_mix_background,NULL,
                        "SCR background mix","SCR mezclar con fondo","SCR mesclar amb fons");
                    menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(zxdesktop_draw_scrfile_mix_background ? 'X' : ' ' ));
                    menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);


                    menu_add_item_menu_en_es_ca(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_zxdesktop_scrfile_disable_flash,NULL,
                        "SCR allow flash","SCR permitir parpadeo","SCR permetre parpelleig");
                    menu_add_item_menu_prefijo_format(array_menu_ext_desktop_settings,"[%c] ",(zxdesktop_draw_scrfile_disable_flash==0 ? 'X' : ' ' ));
                    menu_add_item_menu_es_avanzado(array_menu_ext_desktop_settings);


                }

                
            }

		}


        menu_add_item_menu_separator(array_menu_ext_desktop_settings);

		menu_add_ESC_item(array_menu_ext_desktop_settings);

        retorno_menu=menu_dibuja_menu(&ext_desktop_settings_opcion_seleccionada,&item_seleccionado,array_menu_ext_desktop_settings,"ZX Desktop Settings");

                

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                    
            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}


void menu_spectrum_core_reduced(MENU_ITEM_PARAMETERS)
{
	core_spectrum_uses_reduced.v ^=1;

	set_cpu_core_loop();

}



void menu_tbblue_deny_turbo_rom(MENU_ITEM_PARAMETERS)
{

	tbblue_deny_turbo_rom.v ^=1;
}

void menu_hardware_top_speed(MENU_ITEM_PARAMETERS)
{
	timer_toggle_top_speed_timer();
}

void menu_turbo_mode(MENU_ITEM_PARAMETERS)
{
	if (cpu_turbo_speed==MAX_CPU_TURBO_SPEED) cpu_turbo_speed=1;
	else cpu_turbo_speed *=2;

	cpu_set_turbo_speed();
}

void menu_zxuno_deny_turbo_bios_boot(MENU_ITEM_PARAMETERS)
{
	zxuno_deny_turbo_bios_boot.v ^=1;
}

void menu_cpu_type(MENU_ITEM_PARAMETERS)
{
	z80_cpu_current_type++;
	if (z80_cpu_current_type>=TOTAL_Z80_CPU_TYPES) z80_cpu_current_type=0;
}


void menu_cpu_ldir_hack(MENU_ITEM_PARAMETERS)
{
	cpu_ldir_lddr_hack_optimized.v ^=1;
}


void menu_tbblue_deny_turbo_rom_max_allowed(MENU_ITEM_PARAMETERS)
{
	tbblue_deny_turbo_rom_max_allowed *=2;

	if (tbblue_deny_turbo_rom_max_allowed>8) tbblue_deny_turbo_rom_max_allowed=1;
}





//menu cpu settings
void menu_cpu_settings(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_cpu_settings;
    menu_item item_seleccionado;
    int retorno_menu;
    do {

		//hotkeys usadas: todc

		char buffer_velocidad[16];

		if (CPU_IS_Z80 && !MACHINE_IS_Z88) {
			int cpu_hz=get_cpu_frequency();
			int cpu_khz=cpu_hz/1000;

			//Obtener decimales
			int mhz_enteros=cpu_khz/1000;
			int decimal_mhz=cpu_khz-(mhz_enteros*1000);

								//01234567890123456789012345678901
								//           1234567890
								//Turbo: 16X 99.999 MHz
			sprintf(buffer_velocidad,"%d.%d MHz",mhz_enteros,decimal_mhz);
		}
		else {
			buffer_velocidad[0]=0;
		}

		menu_add_item_menu_inicial_format(&array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_turbo_mode,NULL,"~~Turbo [%dX %s]",cpu_turbo_speed,buffer_velocidad);
		menu_add_item_menu_shortcut(array_menu_cpu_settings,'t');
		menu_add_item_menu_tooltip(array_menu_cpu_settings,"Changes only the Z80 speed");
		menu_add_item_menu_ayuda(array_menu_cpu_settings,"Changes only the Z80 speed. Do not modify FPS, interrupts or any other parameter. "
					"Some machines, like ZX-Uno or Chloe, change this setting");





		if (MACHINE_IS_ZXUNO) {
            //menu_add_item_menu(array_menu_cpu_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
            menu_add_item_menu_en_es_ca(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_zxuno_deny_turbo_bios_boot,NULL,
                "~~Deny turbo on boot","~~Denegar turbo al inicio","~~Denegar turbo a l'inici");
            menu_add_item_menu_prefijo_format(array_menu_cpu_settings,"[%c] ",(zxuno_deny_turbo_bios_boot.v ? 'X' : ' ') );                            
            menu_add_item_menu_shortcut(array_menu_cpu_settings,'d');
            menu_add_item_menu_tooltip(array_menu_cpu_settings,"Denies changing turbo mode when booting ZX-Uno and on bios");
            menu_add_item_menu_ayuda(array_menu_cpu_settings,"Denies changing turbo mode when booting ZX-Uno and on bios");
	  }

		if (MACHINE_IS_TBBLUE) {
            //menu_add_item_menu(array_menu_cpu_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
            menu_add_item_menu_en_es_ca(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_tbblue_deny_turbo_rom,NULL,
                "Limit turbo on ROM","Limitar turbo en la ROM","Limitar turbo a la ROM");
            menu_add_item_menu_prefijo_format(array_menu_cpu_settings,"[%c] ",(tbblue_deny_turbo_rom.v ? 'X' : ' ') );

            //menu_add_item_menu_shortcut(array_menu_cpu_settings,'d');
            menu_add_item_menu_tooltip(array_menu_cpu_settings,"Limit changing turbo mode on Next ROM. Useful on slow machines. Can make the boot process to fail");
            menu_add_item_menu_ayuda(array_menu_cpu_settings,"Limit changing turbo mode on Next ROM. Useful on slow machines. Can make the boot process to fail");

            if (tbblue_deny_turbo_rom.v) {
                menu_add_item_menu_en_es_ca(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_tbblue_deny_turbo_rom_max_allowed,NULL,
                    "Max turbo allowed","Max turbo permitido","Max turbo permès");
                menu_add_item_menu_prefijo_format(array_menu_cpu_settings,"[%d] ",tbblue_deny_turbo_rom_max_allowed);
                menu_add_item_menu_tooltip(array_menu_cpu_settings,"Max turbo value allowed on Next ROM.");
                menu_add_item_menu_ayuda(array_menu_cpu_settings,"Max turbo value allowed on Next ROM.");
            }
	  }	  

		if (!MACHINE_IS_Z88) {
			menu_add_item_menu_format(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_hardware_top_speed,NULL,"[%c] T~~op Speed",(top_speed_timer.v ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_cpu_settings,'o');
			menu_add_item_menu_tooltip(array_menu_cpu_settings,"Runs at maximum speed, when menu closed. Not available on Z88");
			menu_add_item_menu_ayuda(array_menu_cpu_settings,"Runs at maximum speed, using 100% of CPU of host machine, when menu closed. "
						"The display is refreshed 1 time per second. This mode is also entered when loading a real tape and "
						"accelerate loaders setting is enabled. Not available on Z88");

		}	  

		if (CPU_IS_Z80) {
            menu_add_item_menu(array_menu_cpu_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

			menu_add_item_menu_en_es_ca(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_cpu_type,NULL,
                "Z80 CPU Type","Tipo CPU Z80","Tipus CPU Z80");
            menu_add_item_menu_sufijo_format(array_menu_cpu_settings," [%s]",z80_cpu_types_strings[z80_cpu_current_type]);
			menu_add_item_menu_tooltip(array_menu_cpu_settings,"Chooses the cpu type");
			menu_add_item_menu_ayuda(array_menu_cpu_settings,"CPU type modifies the way the CPU fires an IM0 interrupt, or the behaviour of opcode OUT (C),0, for example");


			menu_add_item_menu_en_es_ca(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_cpu_ldir_hack,NULL,
                "Z80 LDIR/LDDR Hack","Truco Z80 LDIR/LDDR","Truc Z80 LDIR/LDDR");
            menu_add_item_menu_prefijo_format(array_menu_cpu_settings,"[%c] ",(cpu_ldir_lddr_hack_optimized.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_cpu_settings,"EXPERIMENTAL feature! It makes a fast data transference without taking care of timings. Speeds up cpu core");
			menu_add_item_menu_ayuda(array_menu_cpu_settings,"EXPERIMENTAL feature! It makes a fast data transference without taking care of timings. Speeds up cpu core");
		}		

		if (MACHINE_IS_SPECTRUM) {
            menu_add_item_menu(array_menu_cpu_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

			menu_add_item_menu_en_es_ca(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_spectrum_core_reduced,NULL,
                "Spectrum ~~core","~~Core Spectrum","~~Core Spectrum");
			menu_add_item_menu_sufijo_format(array_menu_cpu_settings," [%s]",(core_spectrum_uses_reduced.v ? "Reduced" : "Normal") );            
			menu_add_item_menu_shortcut(array_menu_cpu_settings,'c');
			menu_add_item_menu_tooltip(array_menu_cpu_settings,"Switches between the normal Spectrum core or the reduced core");
			menu_add_item_menu_ayuda(array_menu_cpu_settings,"When using the Spectrum reduced core, the following features are NOT available or are NOT properly emulated:\n"
				"Debug t-states, Char detection, +3 Disk, Save to tape, Divide, Divmmc, Multiface, RZX, Raster interrupts, ZX-Uno DMA, TBBlue DMA, Datagear DMA, TBBlue Copper, Audio DAC, Stereo AY, Video out to file, Last core frame statistics");
		}



        menu_add_item_menu(array_menu_cpu_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_ESC_item(array_menu_cpu_settings);

        retorno_menu=menu_dibuja_menu(&cpu_settings_opcion_seleccionada,&item_seleccionado,array_menu_cpu_settings,"CPU Settings" );

        
        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                        
                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



}




void menu_setting_filesel_no_show_dirs(MENU_ITEM_PARAMETERS)
{
	menu_filesel_hide_dirs.v ^=1;
}

void menu_setting_filesel_no_show_size(MENU_ITEM_PARAMETERS)
{
	menu_filesel_hide_size.v ^=1;
}

void menu_setting_filesel_previews(MENU_ITEM_PARAMETERS)
{
	menu_filesel_show_previews.v ^=1;
}

void menu_setting_fileviewer_hex(MENU_ITEM_PARAMETERS)
{
    menu_file_viewer_always_hex.v ^=1;
}

void menu_setting_filesel_allow_delete_folders(MENU_ITEM_PARAMETERS)
{
    menu_filesel_utils_allow_folder_delete.v ^=1;
}

void menu_setting_filesel_previews_reduce(MENU_ITEM_PARAMETERS)
{
    menu_filesel_show_previews_reduce.v ^=1;
}

void menu_fileselector_settings(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    do {

		menu_add_item_menu_en_es_ca_inicial(&array_menu_common,MENU_OPCION_NORMAL,menu_setting_filesel_no_show_dirs,NULL,
            "Show ~~directories","Mostrar ~~directorios","Veure ~~directoris");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_filesel_hide_dirs.v==0 ? 'X' : ' ') );
        menu_add_item_menu_shortcut(array_menu_common,'d');	
        menu_add_item_menu_tooltip(array_menu_common,"Hide directories from file browser menus");
        menu_add_item_menu_ayuda(array_menu_common,"Hide directories from file browser menus. "
                                "Useful on demo environments and you don't want the user to be able to navigate the filesystem");

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_setting_filesel_no_show_size,NULL,
            "Show file ~~size","Mostrar e~~spacio archivos","Veure e~~spai arxius");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_filesel_hide_size.v==0 ? 'X' : ' ') );
        menu_add_item_menu_shortcut(array_menu_common,'s');    
        menu_add_item_menu_tooltip(array_menu_common,"Hide file size from file selector menus");
        menu_add_item_menu_ayuda(array_menu_common,"Hide file size from file browser menus");      

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_setting_filesel_previews,NULL,
            "Show file ~~previews","Mostrar ~~previews archivos","Veure ~~previews arxius");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_filesel_show_previews.v ? 'X' : ' ') );
        menu_add_item_menu_shortcut(array_menu_common,'p');
        menu_add_item_menu_tooltip(array_menu_common,"Show file previews in the file selector");
        menu_add_item_menu_ayuda(array_menu_common,"Show file previews for .scr, .tap, .tzx, etc...\n"
                            "Note that the fileselector window must be big enough to hold that preview, if not, it will not be shown");

        if (menu_filesel_show_previews.v) {
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_setting_filesel_previews_reduce,NULL,
                "Red~~uce previews to half size","Red~~ucir previews a la mitad","Red~~uir previews a la meitat");
            menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_filesel_show_previews_reduce.v ? 'X' : ' ') );
            menu_add_item_menu_shortcut(array_menu_common,'u');
            menu_add_item_menu_tooltip(array_menu_common,"Reduce previews to half size instead of full size");
            menu_add_item_menu_ayuda(array_menu_common,"Reduce previews to half size instead of full size");
        }

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_setting_fileviewer_hex,NULL,
            "~~Hexadecimal file viewer","Visor archivos ~~Hexadecimal","Visor arxius ~~Hexadecimal");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_file_viewer_always_hex.v ? 'X' : ' ') );
        menu_add_item_menu_shortcut(array_menu_common,'h');
        menu_add_item_menu_tooltip(array_menu_common,"File viewer always shows file contents in hexadecimal+ascii");
        menu_add_item_menu_ayuda(array_menu_common,"File viewer always shows file contents in hexadecimal+ascii");

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_setting_filesel_allow_delete_folders,NULL,
            "Allow fold~~ers delete","Permitir borrar dir~~ectorios","Permetre esborrar dir~~ectoris");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(menu_filesel_utils_allow_folder_delete.v ? 'X' : ' ') );
        menu_add_item_menu_shortcut(array_menu_common,'e');
        menu_add_item_menu_tooltip(array_menu_common,"Allows deleting folders on the file utilities browser. Enable it AT YOUR OWN RISK");
        menu_add_item_menu_ayuda(array_menu_common,"Allows deleting folders on the file utilities browser. Enable it AT YOUR OWN RISK");

                   

        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&fileselector_settings_opcion_seleccionada,&item_seleccionado,array_menu_common,"File Browser Settings" );

                

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                
            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}




void menu_settings_enable_statistics(MENU_ITEM_PARAMETERS)
{
	if (stats_enabled.v) stats_disable();
	else stats_enable();
}

void menu_settings_enable_check_updates(MENU_ITEM_PARAMETERS)
{
	stats_check_updates_enabled.v ^=1;	
}

void menu_settings_enable_check_yesterday_users(MENU_ITEM_PARAMETERS)
{
	stats_check_yesterday_users_enabled.v ^=1;
}

void menu_settings_statistics(MENU_ITEM_PARAMETERS)
{
        //Dado que es una variable local, siempre podemos usar este nombre array_menu_common
        menu_item *array_menu_common;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

			menu_add_item_menu_en_es_ca_inicial(&array_menu_common,MENU_OPCION_NORMAL,menu_settings_enable_check_updates,NULL,
                "Check updates","Comprobar actualizaciones","Comprovar actualitzacions");
			menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(stats_check_updates_enabled.v ? 'X' : ' ') );    
			menu_add_item_menu_tooltip(array_menu_common,"Check ZEsarUX updates");
			menu_add_item_menu_ayuda(array_menu_common,"Check ZEsarUX updates");                       


			menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_settings_enable_check_yesterday_users,NULL,
                "Check yesterday users","Comprobar usuarios de ayer","Comprovar usuaris d'ahir");
			menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(stats_check_yesterday_users_enabled.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_common,"Retrieve ZEsarUX yesterday users");
			menu_add_item_menu_ayuda(array_menu_common,"Retrieve ZEsarUX yesterday users");


                
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_settings_enable_statistics,NULL,
                "Send Statistics","Enviar estadísticas","Enviar estadístiques");
            menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",(stats_enabled.v ? 'X' : ' ') );                    
			menu_add_item_menu_tooltip(array_menu_common,"Send anonymous statistics to a remote server, every time ZEsarUX starts");
			menu_add_item_menu_ayuda(array_menu_common,"Send anonymous statistics to a remote server, every time ZEsarUX starts");

			if (stats_enabled.v) {
				menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
                    "The following data is sent:","Se envían los siguientes datos:","S'envien les següents dades:");
	
	            menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    Public IP address");
				
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    UUID: %s",stats_uuid);
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    System: %s",COMPILATION_SYSTEM);
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    Minutes: %d",stats_get_current_total_minutes_use() );
				
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    Speccy queries: %d",stats_total_speccy_browser_queries);
				
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    ZX81 queries: %d",stats_total_zx81_browser_queries);
				
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    Emulator version: %s",EMULATOR_VERSION);
				
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    Build Number: %s",BUILDNUMBER);
				

			}

              
						
			menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

            menu_add_ESC_item(array_menu_common);

            retorno_menu=menu_dibuja_menu(&settings_statistics_opcion_seleccionada,&item_seleccionado,array_menu_common,"Statistics Settings" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


