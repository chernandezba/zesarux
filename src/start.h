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

#ifndef START_H
#define START_H

#include "cpu.h"

extern int zesarux_main (int main_argc,char *main_argv[]);
extern void set_last_dimensiones_ventana(void);
extern void random_ram(z80_byte *puntero,int longitud);
extern void dump_ram_file_on_exit(void);
extern void end_emulator_saveornot_config(int saveconfig);

extern void end_emulator(void);

extern void end_emulator_autosave_snapshot(void);

#endif
