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

#ifndef SNAP_ZX8081_H
#define SNAP_ZX8081_H

extern int new_tap_load_detect_zx8081(void);
extern int new_tap_save_detect_zx8081(void);
extern void new_tape_load_zx80(void);
extern void new_tape_load_zx81(void);
extern void new_tape_save_zx80(void);
extern void new_tape_save_zx81();
extern void new_load_zx80_o_snapshot(char *archivo);
extern void new_load_zx81_p_snapshot(char *archivo);
extern void new_save_zx80_o_snapshot(char *filename);
extern void new_save_zx81_p_snapshot(char *filename);
extern int new_tap_save_detect_zx81(void);
extern int new_tap_save_detect_zx80(void);
extern int new_tap_load_detect_zx81(void);
extern int new_tap_load_detect_zx80(void);
extern void new_snap_load_zx8081_simulate_cpuloop(void);
extern void new_snap_load_zx8081_simulate_bit(z80_bit valor);
extern void new_snap_load_zx8081_simulate_byte(z80_byte valor);
extern void new_snap_load_zx80_zx81_simulate_loading(z80_byte *puntero_inicio,z80_byte *buffer_lectura,int leidos);
extern void new_load_zx80_o_snapshot_in_mem(char *archivo);
extern void new_load_zx81_p_snapshot_in_mem(char *archivo) ;
extern void new_set_return_saveload_zx80(void);
extern void new_set_return_saveload_zx81(void);
extern void new_load_zx80_set_common_registers(void);
extern void new_load_zx81_set_common_registers(z80_int ramtopvalue);
extern void new_snap_load_zx80_smp(char *archivo);
extern void new_snap_load_zx81_smp(char *archivo);

extern z80_bit zx8081_disable_tape_traps;

#endif
