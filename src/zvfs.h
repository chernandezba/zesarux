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

#ifndef ZVFS_H
#define ZVFS_H

#include "ff.h"

extern void zvfs_fclose(int in_fatfs,FILE *ptr_file_name,FIL *fil);
extern int zvfs_feof(int in_fatfs,FILE *ptr_file_name,FIL *fil);
extern int zvfs_fopen_read(char *file_name,int *in_fatfs,FILE **ptr_file_name,FIL *fil);
extern int zvfs_fopen_write(char *file_name,int *in_fatfs,FILE **ptr_file_name,FIL *fil);
extern int zvfs_fread(int in_fatfs,z80_byte *puntero_memoria,int bytes_to_load,FILE *ptr_file_hexdump_browser,FIL *fil);
extern int zvfs_fwrite(int in_fatfs,z80_byte *puntero_memoria,int bytes_to_save,FILE *ptr_file_hexdump_browser,FIL *fil);
extern void zvfs_getcwd(char *dir,int len);
extern void zvfs_chdir(char *dir);
extern void zvfs_rename(char *old,char *new);
extern int zvfs_delete(char *filename);
extern void zvfs_mkdir(char *directory);
extern const char *zvfs_get_strerror(FRESULT error);
extern void zvfs_fseek(int in_fatfs,FILE *ptr_file, long offset, int whence,FIL *fil);
extern long zvfs_ftell(int in_fatfs,FILE *ptr_file, FIL *fil);
extern z80_byte zvfs_fgetc(int in_fatfs,FILE *ptr_file_hexdump_browser,FIL *fil);
extern long long int zvfs_get_file_size(char *name);

#endif
