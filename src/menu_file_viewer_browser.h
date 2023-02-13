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

#ifndef MENU_FILE_VIEWER_BROWSER_H
#define MENU_FILE_VIEWER_BROWSER_H

#include <dirent.h>
#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "cpu.h"


//Por el tema de usar PATH_MAX en windows
#ifdef MINGW
#include <stdlib.h>
#define PATH_MAX MAX_PATH
#define NAME_MAX MAX_PATH
#endif


extern void menu_file_viewer_read_file(char *title,char *file_name);
extern void menu_tape_browser_show(char *filename);
extern void menu_file_hexdump_browser_show(char *filename);
extern void menu_file_mmc_browser_show_file(z80_byte *origen,char *destino,int sipuntoextension,int longitud);
extern void menu_file_mmc_browser_show(char *filename,char *tipo_imagen);
extern void menu_file_viewer_read_text_file(char *title,char *file_name);
extern void menu_file_dsk_browser_show(char *filename);
extern void menu_hilow_datadrive_browser(z80_byte *puntero_memoria);
extern void menu_dsk_getoff_block(z80_byte *dsk_file_memory,int longitud_dsk,int bloque,int *offset1,int *offset2,int incremento_pista);


#endif
