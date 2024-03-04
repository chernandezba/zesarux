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

#ifndef ESXDOS_HANDLER_H
#define ESXDOS_HANDLER_H

#include <dirent.h>
#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif

#include "cpu.h"
#include "utils.h"

//Algunos nombres de flags cambiados segun _DEVELOPMENT/target/zx/config/config_esxdos.m4
/*
define(`__ESXDOS_MODE_READ', 0x01)            # read access
define(`__ESXDOS_MODE_WRITE', 0x02)           # write access

define(`__ESXDOS_MODE_OPEN_EXIST', 0x00)      # open if exists else error; fp = 0
define(`__ESXDOS_MODE_OPEN_CREAT', 0x08)      # open if exists else create; fp = 0
define(`__ESXDOS_MODE_CREAT_NOEXIST', 0x04)   # if file exists error else create; fp = 0
define(`__ESXDOS_MODE_CREAT_TRUNC', 0x0c)     # create or replace an existing file; fp = 0
define(`__ESXDOS_MODE_USE_HEADER', 0x40)      # use +3DOS header passed in DE

*/

#define ESXDOS_RST8_FA_OPEN_EXIST  0x00
// open if exists else error; fp = 0

#define ESXDOS_RST8_FA_READ 0x01
// read access

#define ESXDOS_RST8_FA_WRITE  0x02
// write access

#define ESXDOS_RST8_FA_CREAT_NOEXIST 0x04
// open if exists else create; fp = 0


#define ESXDOS_RST8_FA_OPEN_CREAT  0x08
// # open if exists else create; fp = 0



#define ESXDOS_RST8_FA_CREATE_TRUNC 0x0c
// combinacion de ESXDOS_RST8_FA_OPEN_CREAT y ESXDOS_RST8_FA_CREAT_NOEXIST (8+4)


/*
FA_OPEN_EXIST              equ             %00000000                       ; Open if exists, else error
FA_READ                 equ             %00000001                       ; Read access
FA_WRITE                equ             %00000010                       ; Write access
FA_CREAT_NOEXIST   equ             %00000100                       ; Create if not exists, if exists error
FA_OPEN_CREAT              equ             %00001000                       ; Open if exists, if not create


FA_CREATE_AL    equ             %00001100                       ; Create if not exists, else open and truncate

FA_USE_HEADER   equ             %01000000                       ; Use +3DOS header (passed in DE)
*/

//Este es suma de los anteriores
//#define ESXDOS_RST8_FA_CREATE_AL  0x0C
// $0c  inc c           create if does not
 //                                      exist, else open and
 //                                      truncate
#define ESXDOS_RST8_FA_USE_HEADER  0x40
// $40  ld b,b          use plus3dos header
//                                      (passed in de)



#define ESXDOS_RST8_FA_OVERWRITE   0x0c
#define ESXDOS_RST8_FA_CLOSE      0x00


#define ESXDOS_RST8_DISK_STATUS 0x80

#define ESXDOS_RST8_DISK_READ 0x81
#define ESXDOS_RST8_DISK_WRITE 0x82
#define ESXDOS_RST8_DISK_IOCTL 0x83
#define ESXDOS_RST8_DISK_INFO 0x84
#define ESXDOS_RST8_DISK_FILEMAP 0x85

#define ESXDOS_RST8_M_GETSETDRV   0x89
#define ESXDOS_RST8_M_DRIVEINFO 0x8a

#define ESXDOS_RST8_F_MOUNT 0x98

#define ESXDOS_RST8_F_OPEN      0x9a
#define ESXDOS_RST8_F_CLOSE      0x9b
#define ESXDOS_RST8_F_SYNC      0x9c
#define ESXDOS_RST8_F_READ      0x9d
#define ESXDOS_RST8_F_WRITE      0x9e

#define ESXDOS_RST8_F_SEEK 0x9f

#define ESXDOS_RST8_F_FSTAT 0xa1

#define ESXDOS_RST8_F_OPENDIR 0xa3
#define ESXDOS_RST8_F_READDIR 0xa4

#define ESXDOS_RST8_F_TELLDIR 0xa5
#define ESXDOS_RST8_F_SEEKDIR 0xa6

#define ESXDOS_RST8_F_REWINDDIR 0xa7

#define ESXDOS_RST8_F_GETCWD 0xa8
#define ESXDOS_RST8_F_CHDIR 0xa9

#define ESXDOS_RST8_F_MKDIR 0xaa

#define ESXDOS_RST8_F_STAT 0xac
#define ESXDOS_RST8_F_UNLINK 0xad

#define ESXDOS_RST8_F_RENAME 0xb0


#define ESXDOS_ERROR_EOK                      1          // OK
#define ESXDOS_ERROR_EGENERAL                 2          // Syntax error
#define ESXDOS_ERROR_ESTEND                   3          // Statement lost
#define ESXDOS_ERROR_EWRTYPE                  4          // Wrong file type
#define ESXDOS_ERROR_ENOENT                   5          // No such file or folder
#define ESXDOS_ERROR_EIO                      6          // I/O error
#define ESXDOS_ERROR_EBADFN                   7          // Bad filename
#define ESXDOS_ERROR_EACCES                   8          // Access denied
#define ESXDOS_ERROR_ENOSPC                   9          // Disk full
#define ESXDOS_ERROR_ENXIO                    10         // Bad I/O rest
#define ESXDOS_ERROR_ENODRV                   11         // No such drive
#define ESXDOS_ERROR_ENFILE                   12         // Too many open files
#define ESXDOS_ERROR_EBADF                    13         // Bad file descriptor
#define ESXDOS_ERROR_ENODEV                   14         // No such device
#define ESXDOS_ERROR_EOVERFLOW                15         // File pointer overflow
#define ESXDOS_ERROR_EISDIR                   16         // Is a folder
#define ESXDOS_ERROR_ENOTDIR                  17         // Is not a folder
#define ESXDOS_ERROR_EEXIST                   18         // File exists
#define ESXDOS_ERROR_EPATH                    19         // Bad path
#define ESXDOS_ERROR_ENOSYS                   20         // Missing SYS
#define ESXDOS_ERROR_ENAMETOOLONG             21         // Path too long
#define ESXDOS_ERROR_ENOCMD                   22         // No such command
#define ESXDOS_ERROR_EINUSE                   23         // File in use
#define ESXDOS_ERROR_ERDONLY                  24         // File is read only
#define ESXDOS_ERROR_EVFAIL                   25         // Verify failed
#define ESXDOS_ERROR_EIOFAIL                  26         // Loading .IO failed
#define ESXDOS_ERROR_ENOTEMPTY                27         // Folder not empty
#define ESXDOS_ERROR_EMAPRAM                  28         // MAPRAM active
#define ESXDOS_ERROR_EDRVBUSY                 29         // Drive busy
#define ESXDOS_ERROR_EBADFS                   30         // Unknown file system
#define ESXDOS_ERROR_EDEVBUSY                 31         // Device is BUSY


extern z80_bit esxdos_handler_enabled;
extern void esxdos_handler_run(void);
extern char esxdos_handler_root_dir[];
extern char esxdos_handler_cwd[];

extern z80_bit esxdos_handler_readonly;


extern void esxdos_handler_enable(void);
extern void esxdos_handler_disable(void);
extern void esxdos_handler_reset(void);

#define ESXDOS_MAX_OPEN_FILES 16


//Usados al fopen de archivos y tambien al abrir directorios

struct s_esxdos_fopen {

	/* Para archivos */
	FILE *esxdos_last_open_file_handler_unix;
	//z80_byte temp_esxdos_last_open_file_handler;

	//Usado al hacer fstat
	struct stat last_file_buf_stat;


	/* Para directorios */
	//usados al leer directorio
	//z80_byte esxdos_handler_filinfo_fattrib;
	struct dirent *esxdos_handler_dp;
	DIR *esxdos_handler_dfd; //	=NULL;
	//ultimo directorio leido al listar archivos
	char esxdos_handler_last_dir_open[PATH_MAX];

	//para telldir
	unsigned int contador_directorio;


	z80_byte buffer_plus3dos_header[8];
	z80_bit tiene_plus3dos_header;

	/* Comun */
	//Indica a 1 que el archivo/directorio esta abierto. A 0 si no
	z80_bit open_file;

	z80_bit is_a_directory;

	//Usado solo por debug de ZRCP:
	char debug_name[PATH_MAX];
	char debug_fullpath[PATH_MAX];
};

extern struct s_esxdos_fopen esxdos_fopen_files[];

extern int esxdos_find_free_fopen(void);
extern void esxdos_handler_pre_fileopen(char *nombre_inicial,char *fullpath);
extern void esxdos_handler_call_f_open_post(int handle,char *nombre_archivo,char *fullpath);
extern z80_bit esxdos_umount_on_reset;

#endif
