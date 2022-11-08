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

#ifndef PD765_H
#define PD765_H

#include "cpu.h"

#define PD765_STATUS_REGISTER_D0B_MASK 0x01
#define PD765_STATUS_REGISTER_D1B_MASK 0x02
#define PD765_STATUS_REGISTER_D2B_MASK 0x04
#define PD765_STATUS_REGISTER_D3B_MASK 0x08
#define PD765_STATUS_REGISTER_CB_MASK  0x10
#define PD765_STATUS_REGISTER_EXM_MASK 0x20
#define PD765_STATUS_REGISTER_DIO_MASK 0x40
#define PD765_STATUS_REGISTER_RQM_MASK 0x80

extern z80_byte pd765_main_status_register;

extern z80_bit pd765_enabled;
extern void pd765_enable(void);
extern void pd765_disable(void);

extern z80_byte pd765_read(void);
extern z80_byte pd765_read_status_register(void);
extern void pd765_out_port_1ffd(z80_byte value);
extern void pd765_out_port_3ffd(z80_byte value);

extern void pd765_reset(void);

extern int pd765_interrupt_pending;
extern int pd765_pcn;
extern int pd765_motor_status;

#endif
