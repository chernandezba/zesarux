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
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "ql.h"
#include "m68k.h"
#include "debug.h"
#include "utils.h"
#include "zxvision.h"
#include "operaciones.h"
#include "screen.h"
#include "settings.h"
#include "ay38912.h"
#include "ql_i8049.h"
#include "ql_zx8302.h"
#include "compileoptions.h"


#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif




unsigned char *memoria_ql;





void ql_writebyte(unsigned int Address, unsigned char Data)
{
    Address&=QL_MEM_LIMIT;

    if (Address>=0x18000 && Address<=0x1BFFF) {
        ql_zx8032_write(Address,Data);


        #ifdef EMULATE_VISUALMEM

        //Escribimos en visualmem a partir de direccion 18000H
        set_visualmembuffer(Address);

        #endif

        return; //Espacio i/o
    }

    if (Address<0x18000 || Address>QL_MEM_LIMIT) return;


    unsigned char valor=Data;

    memoria_ql[Address]=valor;

    #ifdef EMULATE_VISUALMEM

    //Escribimos en visualmem a partir de direccion 18000H
    set_visualmembuffer(Address);

    #endif

}

unsigned char ql_readbyte(unsigned int Address)
{
    Address&=QL_MEM_LIMIT;

    if (Address>=0x18000 && Address<=0x1BFFF) {


        unsigned char valor=ql_zx8032_readbyte(Address);

        #ifdef EMULATE_VISUALMEM

        //Escribimos en visualmem a partir de direccion 18000H
        set_visualmemreadbuffer(Address);

        #endif
        return valor;
    }


    if (Address>QL_MEM_LIMIT) return(0);

    #ifdef EMULATE_VISUALMEM

    //Escribimos en visualmem a partir de direccion 18000H
    set_visualmemreadbuffer(Address);

    #endif


    unsigned char valor=memoria_ql[Address];
    return valor;
}

//Puntero a la funcion final que se modifica cuando se asigna maquina QL. Al inicio, se apunta a funcion vacia para
//que el parser de breakpoints desde configfile no pete
//Podia petar con --machine QL --set-breakpoint 1 "OPCODE1=207"
unsigned char (*ql_readbyte_no_ports_function)(unsigned int Address);

unsigned char ql_readbyte_no_ports_vacio(unsigned int Address GCC_UNUSED)
{
	return 0;
}

unsigned char ql_readbyte_no_ports(unsigned int Address)
{
	Address&=QL_MEM_LIMIT;
	unsigned char valor=memoria_ql[Address];
	return valor;

}

void ql_writebyte_no_ports(unsigned int Address,unsigned char valor)
{
	Address&=QL_MEM_LIMIT;
	memoria_ql[Address]=valor;

}





unsigned int GetMemB(unsigned int address)
{
        return(ql_readbyte(address));
}


/* Fetch word, address may not be word-aligned */
unsigned int  GetMemW(unsigned int address)
{
#ifdef CHKADDRESSERR
    if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 1);
#endif
        return((ql_readbyte(address)<<8)|ql_readbyte(address+1));
}


/* Fetch dword, address may not be dword-aligned */
unsigned int GetMemL(unsigned int address)
{
#ifdef CHKADDRESSERR
    if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 1);
#endif
        return((GetMemW(address)<<16) | GetMemW(address+2));
}


/* Write byte to address */
void SetMemB (unsigned int address, unsigned int value)
{
    ql_writebyte(address,value);
}


/* Write word, address may not be word-aligned */
void SetMemW(unsigned int address, unsigned int value)
{
#ifdef CHKADDRESSERR
if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 0);
#endif
        ql_writebyte(address,(value>>8)&255);
        ql_writebyte(address+1, (value&255));
}
/* Write dword, address may not be dword-aligned */
void SetMemL(unsigned int address, unsigned int value)
{
#ifdef CHKADDRESSERR
    if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 0);
#endif
        SetMemW(address, (value>>16)&65535);
        SetMemW(address+2, (value&65535));
}


unsigned int m68k_read_disassembler_16 (unsigned int address)
{
	return GetMemW(address);
}


unsigned int m68k_read_disassembler_32 (unsigned int address)
{
	return GetMemL(address);
}




//Funciones legacy solo para interceptar posibles llamadas a poke, peek etc en caso de motorola
//la mayoria de estas vienen del menu, lo ideal es que en el menu se usen peek_byte_z80_moto , etc

void poke_byte_legacy_ql(z80_int dir GCC_UNUSED,z80_byte valor GCC_UNUSED)
{
	debug_printf(VERBOSE_ERR,"Calling poke_byte function on a QL machine. TODO fix it!");
}

void poke_byte_no_time_legacy_ql(z80_int dir GCC_UNUSED,z80_byte valor GCC_UNUSED)
{
	debug_printf(VERBOSE_ERR,"Calling poke_byte_no_time function on a QL machine. TODO fix it!");
}

z80_byte peek_byte_legacy_ql(z80_int dir GCC_UNUSED)
{
	debug_printf(VERBOSE_ERR,"Calling peek_byte function on a QL machine. TODO fix it!");
	return 0;
}

z80_byte peek_byte_no_time_legacy_ql(z80_int dir GCC_UNUSED)
{
	//debug_printf(VERBOSE_ERR,"Calling peek_byte_no_time function on a QL machine. TODO fix it!");
	return 0;
}

z80_byte lee_puerto_legacy_ql(z80_byte h GCC_UNUSED,z80_byte l GCC_UNUSED)
{
	debug_printf(VERBOSE_ERR,"Calling lee_puerto function on a QL machine. TODO fix it!");
	return 0;
}

void out_port_legacy_ql(z80_int puerto GCC_UNUSED,z80_byte value GCC_UNUSED)
{
	debug_printf(VERBOSE_ERR,"Calling out_port function on a QL machine. TODO fix it!");
}

z80_byte fetch_opcode_legacy_ql(void)
{
	debug_printf(VERBOSE_ERR,"Calling fetch_opcode function on a QL machine. TODO fix it!");
	return 0;
}





void motorola_get_flags_string(char *texto)
{

    unsigned int registro_sr=m68k_get_reg(NULL, M68K_REG_SR);

    sprintf (texto,"%c%c%c%c%c%c%c%c%c%c",
        (registro_sr&32768 ? 'T' : '-'),
        (registro_sr&8192  ? 'S' : '-'),
        (registro_sr&1024  ? '2' : '-'),
        (registro_sr&512   ? '1' : '-'),
        (registro_sr&256   ? '0' : '-'),
        (registro_sr&16 ? 'X' : '-'),
        (registro_sr&8  ? 'N' : '-'),
        (registro_sr&4  ? 'Z' : '-'),
        (registro_sr&2  ? 'V' : '-'),
        (registro_sr&1  ? 'C' : '-')  );
}
