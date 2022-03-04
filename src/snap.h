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

#ifndef SNAP_H
#define SNAP_H

#define CURRENT_ZX_VERSION 6


extern void snapshot_load(void);
extern void snapshot_load_name(char *nombre);
extern void snapshot_save(char *archivo);

extern z80_bit snap_zx_permitir_versiones_desconocidas;

extern void load_spectrum_simulate_loading(z80_byte *buffer_lectura,z80_int destino,int leidos,z80_byte flag);

extern char *snapfile;
extern z80_byte snap_zx_version_save;
extern void snap_simulate_load_espera_no_tecla(void);



extern void autosave_snapshot(void);

extern void autoload_snapshot(void);

extern z80_bit snapshot_contautosave_interval_enabled;

extern char snapshot_autosave_interval_quicksave_name[];

extern int snapshot_autosave_interval_seconds;

extern int snapshot_autosave_interval_current_counter;

extern char snapshot_autosave_interval_quicksave_directory[];

extern void autosave_snapshot_at_fixed_interval(void);

extern void load_z80_snapshot(char *archivo);

extern void load_nex_snapshot(char *archivo);

extern void snapshot_quick_save(char *nombre);

extern z80_bit sna_setting_no_change_machine;

extern char *zxfile_machines_id[];

extern char *z80file_machines_id[];

#define AUTOSAVE_NAME "zesarux_autosave.zsf"

extern void snapshot_get_date_time_string(char *texto);

extern void snap_dump_zsf_on_cpu_panic(void);

extern void snapshot_get_date_time_string_human(char *texto);

extern void load_z80_snapshot_bytes(z80_byte *buffer_lectura,int leidos,z80_int direccion_destino,int comprimido,z80_byte *puntero_memoria);

extern int load_nex_snapshot_open_esxdos(char *nombre_archivo,int forzar_filehandle_cero);

#define SNA_48K_HEADER_SIZE 27
#define SNA_128K_HEADER_SIZE 4

#define SP_HEADER_SIZE 38

#define Z80_MAIN_HEADER_SIZE 30
#define Z80_AUX_HEADER_SIZE 57

#define NEX_HEADER_SIZE 512

#endif
