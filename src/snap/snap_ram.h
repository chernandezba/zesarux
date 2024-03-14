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

#ifndef SNAP_RAM_H
#define SNAP_RAM_H


//total de elementos en el array. El usuario puede cambiar el total que mantiene en ram pero no puede exceder este valor
#define MAX_TOTAL_SNAPSHOTS_IN_RAM 1000

//Estructura para salvado de snapshots en ram

struct s_snapshots_in_ram {
    //en donde esta asignada dicha memoria
    z80_byte *memoria;

    //longitud de ese snapshot
    int longitud;

    //cuando se ha generado
    int hora,minuto,segundo;
};

extern struct s_snapshots_in_ram snapshots_in_ram[];
extern int snapshots_in_ram_total_elements;
extern int snapshot_in_ram_load(int posicion);
//extern int snapshot_in_ram_pending_message_footer;
extern void snapshot_add_in_ram(void);
extern void snapshot_in_ram_rewind(void);
extern void snapshot_in_ram_ffw(void);
extern void snapshot_in_ram_rewind_timer(void);
extern int snapshot_in_ram_interval_seconds;
extern int snapshots_in_ram_maximum;
extern void snapshots_in_ram_reset(void);
extern int snapshot_in_ram_enabled_timer_timeout;

extern z80_bit snapshot_in_ram_enabled;

extern int snapshot_in_ram_get_element(int indice);

#endif
