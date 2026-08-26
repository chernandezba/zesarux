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

#ifndef QSOUND_H
#define QSOUND_H


extern void ql_traps_qsound(void);

extern void qsound_load_rom(void);

extern int ql_qsound_is_enabled;
extern int ql_qsound_rom_enabled;
extern int ql_qsound_handle_traps;
extern int ql_qsound_pia_enabled;

extern void ql_qsound_enable(void);
extern void ql_qsound_disable(void);
extern void ql_set_qsound_settings_on_enabled(void);

extern unsigned char qsound_pia_data_port_a;
extern unsigned char qsound_pia_control_port_a;
extern unsigned char qsound_pia_data_port_b;
extern unsigned char qsound_pia_control_port_b;

#define QSOUND_ROM_FILE "qsound_V1.94.rom"

extern char qsound_rom_name[];


#endif
