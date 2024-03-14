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

#ifndef SNAP_RZX_H
#define SNAP_RZX_H


extern void load_rzx_snapshot_file(char *archivo);
//extern void load_rzx_snapshot(void);
extern int rzx_reproduciendo;
extern int rzx_lee_puerto(z80_byte *valor_puerto);


extern z80_int rzx_in_fetch_counter_til_next_int;
extern z80_int rzx_in_fetch_counter_til_next_int_counter;

extern void eject_rzx_file(void);
extern void rzx_delete_footer(void);
extern void rzx_print_footer(void);

#endif
