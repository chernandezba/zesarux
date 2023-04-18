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

#ifndef DSK_H
#define DSK_H

#include "cpu.h"

//Ejemplo metal action:  214784 14 sep  2000 Metal Action 1 - Side A.dsk
//Ejemplo Alien Storm: 255232
//Paperboy2-SideB(Spectrum).dsk: 335301
//Monty Python's Flying Circus.dsk 444160
//Narc - Side A.dsk 472832
#define DSK_MAX_BUFFER_DISCO 472832

#define DSK_SIGNATURE_LENGTH 34
#define DSK_CREATOR_LENGTH 14

#define DSK_MAX_PROTECTION_SCHEME 100

//Para poder hacer debug_printf con la clase DSK adecuada
#define DBG_PRINT_DSK debug_printf(VERBOSE_CLASS_DSK|

extern void dsk_insert_disk(char *nombre);

extern char dskplusthree_file_name[];

extern void dskplusthree_flush_contents_to_disk(void);

extern z80_bit dskplusthree_emulation;


extern z80_bit dskplusthree_write_protection;

extern z80_bit dskplusthree_persistent_writes;


extern void dskplusthree_disable(void);
extern void dskplusthree_enable(void);

extern void plus3dsk_put_byte_disk(int offset,z80_byte value);
extern z80_byte plus3dsk_get_byte_disk(int offset);

extern z80_byte p3dsk_buffer_disco[];

extern void dsk_show_activity(void);

extern void dsk_create(char *filename,int tracks,int sides,int sectors_track,int bytes_sector);

extern int dsk_get_sector(int pista,int parametro_r,z80_byte *sector_fisico,int minimo_sector,int search_deleted,int skip_not_match,int check_r_parameter);

extern int dsk_get_physical_sector(int pista,int sector);

extern void dsk_get_chrn(int pista,int cara,int sector_fisico,z80_byte *parametro_c,z80_byte *parametro_h,z80_byte *parametro_r,z80_byte *parametro_n);
extern void dsk_put_chrn(int pista,int cara,int sector_fisico,z80_byte parametro_c,z80_byte parametro_h,z80_byte parametro_r,z80_byte parametro_n);
extern int dsk_get_start_sector_info(int pista,int cara,int sector_fisico);

extern int dsk_get_sector_size_from_n_value(int n_value);

extern void dsk_get_st12(int pista,int cara,int sector_fisico,z80_byte *parametro_st1,z80_byte *parametro_st2);
extern void dsk_put_st12(int pista,int cara,int sector_fisico,z80_byte parametro_st1,z80_byte parametro_st2);

extern int dsk_get_total_tracks(void);

extern int dsk_get_total_sides(void);

extern void dsk_get_signature(char *buffer);
extern void dsk_get_creator(char *buffer);
extern int dsk_get_start_track(int pista,int cara);

extern int dsk_get_track_number(int pista,int cara);
extern int dsk_get_track_side(int pista,int cara);
extern int dsk_get_sector_size_track(int pista,int cara);
extern int dsk_get_total_sectors_track(int pista,int cara);
extern int dsk_get_gap_length_track(int pista,int cara);
extern int dsk_get_filler_byte_track(int pista,int cara);
extern int dsk_file_type_extended;
extern int dsk_extended_get_track_size(int pista,int cara);

extern int dsk_get_protection_scheme(char *buffer);

extern int dsk_memory_zone_dsk_sector_start;
extern int dsk_memory_zone_dsk_sector_size;
extern z80_bit dsk_memory_zone_dsk_sector_enabled;

extern int dsk_get_sector_fisico(int pista,int cara,int sector_fisico);
extern int dsk_get_real_sector_size_extended(int pista,int cara,int sector_fisico);
extern int dsk_get_datarate_track(int pista,int cara);
extern int dsk_get_gap_length_track(int pista,int cara);
extern int dsk_get_recordingmode_track(int pista,int cara);

extern int dsk_is_track_formatted(int pista,int cara);

#endif
