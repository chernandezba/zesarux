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

#ifndef SNAP_NEX_H
#define SNAP_NEX_H

extern int load_nex_snapshot_open_esxdos(char *nombre_archivo,int forzar_filehandle_cero);

extern void load_nex_snapshot(char *archivo);

extern void load_nex_snapshot_change_to_next(void);

extern void load_nex_snapshot_if_mount_exdos_folder(char *archivo);

extern void load_snx_snapshot(char *archivo);


#endif