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
#include <string.h>

#include "pcw.h"
#include "cpu.h"
#include "screen.h"
#include "debug.h"
#include "contend.h"
#include "joystick.h"
#include "zxvision.h"
#include "operaciones.h"
#include "utils.h"
#include "audio.h"

#include "dsk.h"
#include "pd765.h"


//Direcciones donde estan cada pagina de ram. 16 paginas de 16 kb cada una
z80_byte *pcw_ram_mem_table[16];


//Registros de bancos para F0,F1,F2,F3
z80_byte pcw_bank_registers[4];


//Direcciones actuales mapeadas para lectura, bloques de 16 kb
z80_byte *pcw_memory_paged_read[4];


z80_byte *pcw_get_memory_offset(z80_int dir)
{
    int segmento=dir/16384;

    int offset=dir & 16383;

    z80_byte *puntero;

    puntero=pcw_memory_paged_read[segmento];

    puntero +=offset;

    return puntero;
}

void pcw_set_memory_pages()
{


    z80_byte bank;

    bank=pcw_bank_registers[0];
    pcw_memory_paged_read[0]=pcw_ram_mem_table[bank];

    bank=pcw_bank_registers[1];
    pcw_memory_paged_read[1]=pcw_ram_mem_table[bank];

    bank=pcw_bank_registers[2];
    pcw_memory_paged_read[2]=pcw_ram_mem_table[bank];

    bank=pcw_bank_registers[3];
    pcw_memory_paged_read[3]=pcw_ram_mem_table[bank];


}

void pcw_reset(void)
{
    pcw_bank_registers[0]=pcw_bank_registers[1]=pcw_bank_registers[2]=pcw_bank_registers[3]=0;
    pcw_set_memory_pages();
}

void pcw_init_memory_tables()
{
	debug_printf (VERBOSE_DEBUG,"Initializing pcw memory tables");

        z80_byte *puntero;
        puntero=memoria_spectrum;




        int i;
        for (i=0;i<16;i++) {
                pcw_ram_mem_table[i]=puntero;
                puntero +=16384;
        }

}

void pcw_out_port_bank(z80_byte puerto_l,z80_byte value)
{
/*

&F0	O	Select bank for &0000
&F1	O	Select bank for &4000
&F2	O	Select bank for &8000
&F3	O	Select bank for &C000. Usually &87.
    */

    int bank=puerto_l-0xF0;

    //TODO: numeracion bancos??

    bank &=15;

    pcw_bank_registers[bank]=value;

    printf("PCW set bank %d value %02XH\n",bank,value);

    pcw_set_memory_pages();


}