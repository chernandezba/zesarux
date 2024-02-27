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

#ifndef QL_QDOS_HANDLER_H
#define QL_QDOS_HANDLER_H

#include "ql.h"

#include <dirent.h>
#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif

#include "utils.h"


#define QLTRAPS_MAX_OPEN_FILES 20
#define QLTRAPS_START_FILE_NUMBER 32


//operation not complete
#define QDOS_ERROR_CODE_NC -1


//buffer overflow
#define QDOS_ERROR_CODE_BO -5

//channel not open
#define QDOS_ERROR_CODE_NO -6


//file or device not found
#define QDOS_ERROR_CODE_NF -7

//end of file
#define QDOS_ERROR_CODE_EF -10

//file error
#define QDOS_ERROR_CODE_FE -16

extern void ql_rom_traps(void);

extern char ql_mdv1_root_dir[];
extern char ql_mdv2_root_dir[];
extern char ql_flp1_root_dir[];

extern int ql_device_mdv1_readonly;
extern int ql_device_mdv2_readonly;
extern int ql_device_flp1_readonly;

extern int ql_microdrive_floppy_emulation;

extern z80_byte ql_last_trap;

extern int ql_previous_trap_was_4;

extern moto_long ql_task_default_data_size;

#define QL_POSSIBLE_HEADER_LENGTH_ONE 30
#define QL_POSSIBLE_HEADER_LENGTH_TWO 44

#define QL_MAX_FILE_HEADER_LENGTH QL_POSSIBLE_HEADER_LENGTH_TWO





//64-6 cuando no hay magic
//#define QL_MAX_FILE_HEADER_LENGTH QL_POSSIBLE_HEADER_LENGTH_NO_MAGIC

struct s_qltraps_fopen {

        /* Para archivos */
        FILE *qltraps_last_open_file_handler_unix;
        //z80_byte temp_qltraps_last_open_file_handler;

        //Usado al hacer fstat
        struct stat last_file_buf_stat;


        /* Para directorios */
        //usados al leer directorio
        //z80_byte qltraps_handler_filinfo_fattrib;
        struct dirent *qltraps_handler_dp;
        DIR *qltraps_handler_dfd; //    =NULL;
        //ultimo directorio leido al listar archivos
        char qltraps_handler_last_dir_open[PATH_MAX];

        //para telldir
        unsigned int contador_directorio;

        //para io.file. indica que la siguiente lectura debe retornar eof
        int next_eof_ptr_io_fline;

        //Indica que se ha abierto el dispositivo entero "mdv1_", "mdv2_",etc. Se usa en dir mdv1_
        int es_dispositivo;

        char ql_file_name[1024];


        /* Comun */
        //Indica a 1 que el archivo/directorio esta abierto. A 0 si no
        moto_bit open_file;

        //moto_bit is_a_directory;

        //Usado solo por debug de ZRCP:
        char debug_name[PATH_MAX];
        char debug_fullpath[PATH_MAX];

        //Si tiene cabecera al abrirlo. se guardara en file_header
        int has_header_on_read;

        //Si tiene cabecera sin magic abrirlo. se guardara en file_header_nomagic
        int has_header_no_magic_on_read;

        //The headers ZEsarUX supports can be 30 bytes or 44 bytes long
        moto_byte file_header[QL_MAX_FILE_HEADER_LENGTH];


};

extern struct s_qltraps_fopen qltraps_fopen_files[];

enum ql_qdos_unidades {
    QL_QDOS_UNIT_MDV1,
    QL_QDOS_UNIT_MDV2,
    QL_QDOS_UNIT_FLP1
};

extern void ql_insert_mdv_flp(enum ql_qdos_unidades unidad,char *dir_to_mount);

#endif
