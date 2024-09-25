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

#ifndef MICRODRIVE_H
#define MICRODRIVE_H

#include "cpu.h"

#include "zesarux.h"

//bytes en crudo por sector, contando cabeceras
#define MDR_BYTES_PER_SECTOR 543

#define MDR_MAX_SECTORS 254
#define MDR_MAX_FILE_SIZE ((MDR_BYTES_PER_SECTOR*MDR_MAX_SECTORS)+1)

extern z80_byte mdr_next_byte(void);

extern void mdr_next_sector(int microdrive_seleccionado);


extern void microdrive_insert(int microdrive_seleccionado);

extern void microdrive_footer_operating(void);

extern void microdrive_eject(int microdrive_seleccionado);
extern z80_byte microdrive_status_ef(void);


extern void mdr_write_byte(z80_byte valor);

extern void microdrive_flush_to_disk(void);

extern void microdrive_write_port_ef(z80_byte value);

extern int microdrive_formateando;

//extern int escrito_byte_info_una_vez;


#define MICRODRIVE_STATUS_BIT_GAP  4
#define MICRODRIVE_STATUS_BIT_SYNC 2

//Si vale 1, es que no esta protegido
#define MICRODRIVE_STATUS_BIT_NOT_WRITE_PROTECT 1

#define MAX_MICRODRIVES 8

struct s_microdrive_status {
    int microdrive_enabled;
    int motor_on;

    char microdrive_file_name[PATH_MAX];
    int microdrive_must_flush_to_disk;

    int microdrive_persistent_writes;
    int microdrive_write_protect;

    z80_byte *if1_microdrive_buffer;

    int mdr_total_sectors;
    int mdr_current_sector;
    int mdr_current_offset_in_sector;

    //Contador simple para saber si tenemos que devolver gap, sync o datos
    int contador_estado_microdrive;

    //Indica que estamos en la zona de preamble antes de escribir (10 ceros, 2 ff)
    int mdr_write_preamble_index;

    int bad_sectors_simulated[MDR_MAX_SECTORS];
};

extern struct s_microdrive_status microdrive_status[];

extern void init_microdrives(void);


struct s_mdr_file_cat_one_file {
    //nombre tal cual esta en el microdrive
    char name[11];

    //nombre descriptivo con la info del archivo etc
    char name_extended[200];

    //bytes de cabecera
    //tal cual vendrian en una cabecera de 17 bytes, pero sin incluir el nombre
    z80_byte header_info[7];

    int file_size;
    int total_sectors;

    //<0 si no encuentra un sector
    int sectors_list[MDR_MAX_SECTORS];

    int porcentaje_fragmentacion;

    int numero_copias;

    //Id que se genera con el catalogo: archivos iguales (que son copias uno de otros) tienen mismo id
    int id_file;

    //Indica que a un archivo le faltan bloques. Usado en chkdsk
    //O sea que tienen al menos el primer bloque 0 y le falta alguno de los siguientes
    int faltan_bloques;
};

struct s_mdr_file_cat {
    int total_files;
    int used_sectors;
    int porcentaje_fragmentacion;

    struct s_mdr_file_cat_one_file file[MDR_MAX_SECTORS];

    char label[11];
};

extern struct s_mdr_file_cat *mdr_get_file_catalogue(z80_byte *origen,int total_sectors);

extern void mdr_get_file_from_catalogue(z80_byte *origen,struct s_mdr_file_cat_one_file *archivo,z80_byte *destino);

extern void microdrive_switch_write_protection(int microdrive_seleccionado);

extern int mdr_if_file_exists_catalogue(struct s_mdr_file_cat *catalogo,char *nombre);

extern int mdr_chkdsk_get_files_no_block_zero(struct s_mdr_file_cat *catalogo,z80_byte *origen,int total_sectors);

extern void mdr_get_file_name_escaped(char *origen,char *destino);

#endif
