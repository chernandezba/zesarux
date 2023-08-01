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

#ifndef REMOTE_H
#define REMOTE_H

extern void init_remote_protocol(void);
extern void end_remote_protocol(void);
extern void enable_and_init_remote_protocol(void);

extern z80_bit remote_ack_enter_cpu_step;
extern z80_bit remote_calling_end_emulator;


//extern int   towindows_remote_cpu_run_misocket;
//extern int towindows_remote_cpu_run_verbose;
//extern int towindows_remote_cpu_run_limite;
//extern int towindows_remote_cpu_run_loop;

extern int zrcp_command_close_all_menus;

#define DEFAULT_REMOTE_PROTOCOL_PORT 10000

#define ZRCP_GET_PUT_SNAPSHOT_MEM MAX_ZSF_SNAPSHOT_SIZE


#define MAX_LENGTH_PROTOCOL_COMMAND ((ZRCP_GET_PUT_SNAPSHOT_MEM*2)+1024)


  //comando de put-snapshot usa hexadecimal sin espacios, asi cada byte ocupa 2 en ZRCP
  //Con el valor de MAX_LENGTH_PROTOCOL_COMMAND, podemos usar hasta 128kb de put-snapshot
  //de momento es poco pero probaremos asi



#endif
