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

//This atomlite emulation code is derived from SimCoupe source code and help from its author, Simon Owen
//http://www.simcoupe.org

#include <stdio.h>
#include <string.h>

#include "atomlite.h"
#include "cpu.h"
#include "screen.h"
#include "debug.h"
#include "menu.h"
#include "operaciones.h"
#include "utils.h"
#include "ide.h"


z80_bit atomlite_enabled={0};

/*
z80_byte m_bAddressLatch = 0;

#define ATOM_LITE_ADDR_MASK 0x1f
// Chip select mask

//temp
//#define ATOM_LITE_ADDR_MASK 0x07




#define ATOM_LITE_REG_MASK  0x07  
// Device address mask

int m_uActive = 0;

const unsigned int HDD_ACTIVE_FRAMES = 2;

void atomlite_out (z80_int wPort_, z80_byte bVal_)
{
    switch (wPort_ & ATOM_LITE_REG_MASK)
    {
        // Address select
        case 5:
            // Bits 5-7 are unused, so strip them
            m_bAddressLatch = (bVal_ & ATOM_LITE_ADDR_MASK);
		//printf ("Selecting m_bAddressLatch: %02XH\n",m_bAddressLatch);
            break;

        // Both data ports behave the same
        case 6:
        case 7:
            switch (m_bAddressLatch & ATOM_LITE_ADDR_MASK)
            {
                // Dallas clock
                case 0x1d:
                    //m_Dallas.Out(wPort_ << 8, bVal_);
                    break;

                // ATA device
                default:
                    m_uActive = HDD_ACTIVE_FRAMES;
                    //CAtaAdapter::Out(m_bAddressLatch & ATOM_LITE_ADDR_MASK, bVal_);
			//printf ("ide_write_command_block_register: %02XH %02XH\n",m_bAddressLatch & ATOM_LITE_ADDR_MASK, bVal_);
			//ide_write_command_block_register(m_bAddressLatch & ATOM_LITE_ADDR_MASK, bVal_);
                    break;
            }
            break;

        default:
            //TRACE("AtomLite: Unhandled write to %#04x with %#02x\n", wPort_, bVal_);
            break;
    }
}



z80_byte atomlite_in (z80_int wPort_)
{
    z80_byte bRet = 0xff;

    switch (wPort_ & ATOM_LITE_REG_MASK)
    {
        // Both data ports behave the same
        case 6:
        case 7:
            switch (m_bAddressLatch & ATOM_LITE_ADDR_MASK)
            {
                // Dallas clock
                case 0x1d:
                    //bRet = m_Dallas.In(wPort_ << 8);
			bRet=255;
                    break;

                // ATA device
                default:
                    //bRet = CAtaAdapter::InWord(m_bAddressLatch & ATOM_LITE_ADDR_MASK) & 0xff;
			//printf ("ide_read_command_block_register %02XH\n",m_bAddressLatch & ATOM_LITE_ADDR_MASK);
			//bRet=ide_read_command_block_register(m_bAddressLatch & ATOM_LITE_ADDR_MASK);
                    break;
            }
            break;

        default:
            //TRACE("AtomLite: Unrecognised read from %#04x\n", wPort_);
            break;
    }

    return bRet;
}



*/

void atomlite_print_binary(z80_byte value)
{
	int i;

	for (i=0;i<8;i++) {
		if (value&128) printf ("1");
		else printf ("0");
		value=value<<1;
	}
}

z80_byte atomlite_ide_register=0;

//temp
			extern z80_byte ide_status_register;

void atomlite_out (z80_byte puerto_l, z80_byte value)
{
	//printf ("Port: ");
	//atomlite_print_binary(puerto_l);
	//printf (" value: ");
	//atomlite_print_binary(value);
	//printf ("\n");


	//sleep(1);

	//TODO controlar disco 0 o 1
	switch (puerto_l &7) {
		case 5:
			atomlite_ide_register=value&7;
			//printf ("Selecting IDE register :%d\n",atomlite_ide_register);


		break;

		case 6:
		case 7:

			//printf ("Writing to IDE register %d value %02XH\n",atomlite_ide_register,value);
			ide_write_command_block_register(atomlite_ide_register,value);

			//TODO ver si esto es solo para atom o es un fallo de mi emulacion IDE
			ide_status_register=0x50;

		break;

		default:
			//printf ("Out write no reconocido\n");
			//sleep(1);
		break;
        }
}

z80_byte atomlite_in (z80_byte puerto_l)
{
	//sleep(1);

	z80_byte value=255;

        //TODO controlar disco 0 o 1

	switch (puerto_l &7) {
		//case 5:
                //printf ("Returning selected IDE register :%d\n",atomlite_ide_register);
		//value=atomlite_ide_register;
        	//break;

		case 6:
		case 7:
			value=ide_read_command_block_register(atomlite_ide_register);
        	        //printf ("Returning IDE register %d value %02XH\n",atomlite_ide_register,value);


			//TODO ver si esto es solo para atom o es un fallo de mi emulacion IDE
			ide_status_register ^=0x08;

		break;

		default:
			printf ("In read no reconocido\n");
			//sleep(1);
		break;
        }

	return value;
}

