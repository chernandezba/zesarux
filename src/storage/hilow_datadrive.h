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

#ifndef HILOW_DATADRIVE_H
#define HILOW_DATADRIVE_H

#include "cpu.h"

#define HILOW_ROM_FILE_NAME "hilow_datadrive.rom"


#define HILOW_ROM_SIZE 8192

#define HILOW_RAM_SIZE 2048
//#define HILOW_RAM_SIZE 8192

#define HILOW_SECTOR_SIZE 2048
//#define HILOW_SECTOR_SIZE 1024

/*
una cinta recien formateada acaba los ultimos bytes "útiles" (los de la tabla de sectores libres) hacia la dirección 4E8 hexadecimal
por tanto podria definir el tamaño total de directorio como 500 hexadecimal
Viendo cintas leidas, parece que este valor es 514H = 1300 decimal, que es un valor mas redondo a nivel decimal,
es mas, ese 514H esta en el codigo fuente de la rom
*/
#define HILOW_DIRECTORY_TABLE_SIZE 0x514

//Deduzco por la tabla de sectores libre y como se modifica que el sector mayor es F5H (245)
#define HILOW_MAX_SECTORS 246
//#define HILOW_MAX_SECTORS 256

#define HILOW_DEVICE_SIZE (HILOW_SECTOR_SIZE*HILOW_MAX_SECTORS)

#define HILOW_MAX_SECTORS_PER_FILE 25

#define HILOW_MAX_FILES_DIRECTORY 22

//Lo que ocupa cada entrada de directorio
#define HILOW_DIRECTORY_ENTRY_SIZE 45

extern z80_byte *hilow_device_buffer;

extern void hilow_device_mem_format(int si_escribir_en_ram,int si_escribir_en_device,char *label);

extern void hilow_tapa_action_was_opened(void);

extern void hilow_action_open_tape(void);
extern void hilow_action_close_tape(void);

extern char hilow_file_name[];

extern int hilow_load_device_file(void);

extern void hilow_nmi(void);

//8 KB rom, 2 kb ram
//Creo que son 8 kb ram...
#define HILOW_MEM_SIZE (HILOW_ROM_SIZE+HILOW_RAM_SIZE)

extern z80_byte *hilow_memory_pointer;

extern void hilow_flush_contents_to_disk(void);

extern z80_bit hilow_persistent_writes;

extern z80_bit hilow_write_protection;

//extern void hilow_press_button(void);
extern void hilow_enable(void);
extern void hilow_disable(void);
extern void hilow_reset(void);

extern z80_bit hilow_enabled;

extern z80_bit hilow_mapped_rom;
extern z80_bit hilow_mapped_ram;

extern z80_byte hilow_read_port_ff(z80_int puerto);
extern void hilow_write_port_ff(z80_int port,z80_byte value);

extern z80_bit hilow_cinta_insertada_flag;
extern z80_bit hilow_tapa_has_been_opened;

extern z80_int hilow_util_get_usage_counter(int sector,z80_byte *p);
extern z80_byte hilow_util_get_free_sectors(int sector_dir,z80_byte *p);

extern int hilow_util_get_file_offset(int indice_archivo);
extern int hilow_util_get_total_files(int sector,z80_byte *puntero_memoria);
extern int hilow_get_num_sectors_file(int sector,z80_byte *puntero_memoria,int indice_archivo);
extern int hilow_util_get_sectors_file(int sector,int indice_archivo,z80_byte *puntero_memoria,int *sectores);
extern void hilow_util_get_file_name(int sector,z80_byte *puntero_memoria,int indice_archivo,char *nombre);
extern z80_int hilow_util_get_file_length(int sector,z80_byte *puntero_memoria,int indice_archivo);
extern z80_byte hilow_util_get_file_type(int sector,z80_byte *puntero_memoria,int indice_archivo);
extern void hilow_util_get_file_contents(int sector,z80_byte *puntero_memoria,int indice_archivo,z80_byte *destino_memoria);
extern void hilow_util_get_free_sectors_list(int sector_dir,z80_byte *puntero_memoria,int *sectores);

#endif