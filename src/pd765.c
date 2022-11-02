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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>


#include "pd765.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "dsk.h"

z80_bit pd765_enabled={0};



/*
Main status register

Bit Number  Name                Symbol      Description
----------  -----------         ------      --------------
DB0         FDD 0 Busy          D0B         FDD Number 0 is in seek mode. If any of the bits is set FDC will not accept read or write command
DB1         FDD 1 Busy          D1B         FDD Number 1 is in seek mode. If any of the bits is set FDC will not accept read or write command
DB2         FDD 2 Busy          D2B         FDD Number 2 is in seek mode. If any of the bits is set FDC will not accept read or write command
DB3         FDD 3 Busy          D3B         FDD Number 3 is in seek mode. If any of the bits is set FDC will not accept read or write command
DB4         FDC Busy            CB          A read or write command is in process. FDC will not accept any other command
DB5         Execution Mode      EXM         This bit is set only during execution phase in non-DMA mode. When DB5 goes low, execution phase has ended,
                                            and result phase was started. It operates only during NON-DMA mode of operation
DB6         Data Input/Output   DIO         Indicates direction of data transfer between FDC and Data Register. If DIO = "1" then transfer is from
                                            Data Register to the Processor. If DIO = "0", then transfer is from the Processor to Data Register
DB7         Request for Master  RQM         Indicates Data Register is ready to send or receive data to or from the Processor. Both bits DIO and RQM
                                            should be used to perform the hand-shaking functions of "ready" and "direction" to the processor

*/

#define PD765_STATUS_REGISTER_D0B_MASK 0x01
#define PD765_STATUS_REGISTER_D1B_MASK 0x02
#define PD765_STATUS_REGISTER_D2B_MASK 0x04
#define PD765_STATUS_REGISTER_D3B_MASK 0x08
#define PD765_STATUS_REGISTER_CB_MASK  0x10
#define PD765_STATUS_REGISTER_EXM_MASK 0x20
#define PD765_STATUS_REGISTER_DIO_MASK 0x40
#define PD765_STATUS_REGISTER_RQM_MASK 0x80

#define PD765_STATUS_REGISTER_ON_BOOT (PD765_STATUS_REGISTER_RQM_MASK)

z80_byte pd765_main_status_register=PD765_STATUS_REGISTER_ON_BOOT;


void pd765_reset(void)
{
    pd765_main_status_register=PD765_STATUS_REGISTER_ON_BOOT;
}

z80_bit pd765_enabled;
void pd765_enable(void)
{
    if (pd765_enabled.v) return;

    debug_printf (VERBOSE_INFO,"Enabling PD765");
    pd765_enabled.v=1;

}

void pd765_disable(void)
{
    if (pd765_enabled.v==0) return;

    debug_printf (VERBOSE_INFO,"Disabling PD765");
    pd765_enabled.v=0;
}

void pd765_motor_on(void)
{

}
void pd765_motor_off(void)
{

}
void pd765_write_command(z80_byte value)
{
    printf("PD765: Write command on pc %04XH: %02XH\n",reg_pc,value);
}
z80_byte pd765_read_command(void)
{
    printf("PD765: Read command on pc %04XH\n",reg_pc);

    return 255;
}
z80_byte pd765_read_status_register(void)
{
    printf("PD765: Reading main status register on pc %04XH: %02XH\n",reg_pc,pd765_main_status_register);
    //sleep(1);
    return pd765_main_status_register;
}

void pd765_out_port_1ffd(z80_byte value)
{
    //0x1ffd: Setting bit 3 high will turn the drive motor (or motors, if you have more than one drive attached) on. 
    //Setting bit 3 low will turn them off again. (0x1ffd is also used for memory control).

    if (value&8) pd765_motor_on();
    else pd765_motor_off();
 
}

void pd765_out_port_3ffd(z80_byte value)
{

    //Puertos disco +3
    pd765_write_command(value);

}

