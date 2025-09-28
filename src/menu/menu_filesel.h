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

#ifndef MENU_FILESEL_H
#define MENU_FILESEL_H

#include "cpu.h"

//Para usar PATH_MAX
#include "zesarux.h"



extern z80_bit menu_filesel_hide_dirs;
extern z80_bit menu_filesel_hide_size;
extern z80_bit menu_filesel_utils_allow_folder_delete;
extern z80_bit menu_filesel_show_previews;
extern z80_bit menu_filesel_show_previews_reduce;
extern z80_bit menu_filesel_show_utils;
extern z80_bit menu_filesel_show_only_read_only_utils;

extern int menu_filesel(char *titulo,char *filtros[],char *archivo);
extern int menu_filesel_if_save(char *titulo,char *filtros[],char *archivo,int si_save);
extern int menu_filesel_save(char *titulo,char *filtros[],char *archivo);
extern int menu_filesel_mkdir(char *directory);
extern int file_utils_mount_mmc_image(char *fullpath);
extern void file_utils_umount_mmc_image(void);
extern char menu_filesel_last_directory_seen[];
extern z80_bit menu_filesel_posicionar_archivo;
extern char menu_filesel_posicionar_archivo_nombre[];
extern void file_utils_info_file(char *archivo);

extern int menu_util_file_is_compressed(char *filename);
extern int menu_filesel_uncompress (char *archivo,char *tmpdir);

extern int menu_filesel_expand(char *archivo,char *tmpdir,char *sufijo_carpeta);

extern int menu_filesel_readdir_mmc_image(const char *directorio, struct dirent ***namelist,
              int (*filter)(const struct dirent *),
              int (*compar)(const struct dirent **, const struct dirent **));

#endif
