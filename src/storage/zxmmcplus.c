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
#include <string.h>



#include "zxmmcplus.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "zesarux.h"



z80_bit zxmmcplus_enabled={0};


//Apunta a memoria rom y ram. Primero rom, luego ram
z80_byte *zxmmcplus_memory_pointer;


int zxmmcplus_nested_id_poke_byte;
int zxmmcplus_nested_id_poke_byte_no_time;
int zxmmcplus_nested_id_peek_byte;
int zxmmcplus_nested_id_peek_byte_no_time;

/*
The PAGING Control Register:

This RD/WR register, built into the CPLD, works exactly as on the ZX-Badaloc clone.
The I/O address on ZXMMC+ is $7F and it's bit layout is as follows:

D7         RAM WR Enable
D6         PAGE-IN Enable
D5         RAM/ROM Select
D4:D0    Page number (0 - 31)

D7: This is a stand-alone bit. When SET, the RAM Chip is WRITE ENABLED, no matter what is enabled on READ.
For example, a simple BASIC program may copy the Sinclair ROM to one RAM bank by just setting this bit then performing a
FOR/NEXT loop which POKEs back what is read by PEEK on the 0 - 16383 address space. The PEEK will read the Sinclair ROM,
while the POKE will write in RAM. When the loop completes, the RAM can be enabled in READ mode in place of the internal ROM.
Being a separate bit, D7 allows write-only operation as well as write protection, turning the nonvolatile RAM into some kind of fast ROM.
This is the key feature used by ResiDOS to work. NOTE: Sinclair basic uses to alter a few locations of the "ROM",
if the zxmmc+ ram is left write-enabled. If the above basic example is tried, it is advisable to write-protect the ram bank
prior to returning to the basic "prompt" (i.e. by including the OUT instruction in the command, after a colon).

D6: When SET, this bit activates the PAGE IN on READ cycles. The internal ROM is disabled and one bank from RAM or ROM is accessed instead.

D5: When LOW, the RAM chip is selected for PAGE IN function. When HIGH, the ROM (FLASH) is selected.

D4:D0    These 5 bits are used to select one of 32 banks, 16KB each, from RAM or ROM (depending on D5).
Their state directly affects the logic level on the 5 upper address lines of bot the RAM and FLASH chips, who are provided by the CPLD.

The power-on status of D5 and D6 is user-selectable by means of dip switch n. 3 and 4 respectively.
When PAGING is OFF, the internal ROM will show up when the system is powered or reset. If RAM paging is enabled and ResiDOS
was previously installed into nonvolatile ram, then ResiDOS will start on power-on. If ROM paging is enabled, then ROM BANK 0
will be selected at power-on.

128K Compatibility:

The CPLD detects the OUT $7FFD instruction and captures data bit D4 to the lower address line of the bank page.
This means that a complete 128K Spectrum ROM SET can be programmed and will work in flashrom, enabling the creation
of NMI-patched rom for snapshot purposes. This feature is disabled when zxmmc+ RAM is paged-in, to avoid problems when
running ResiDOS or bootrom firmware.
*/
z80_byte zxmmcplus_port_7f_value=0;


int zxmmcplus_get_page_number(void)
{
    return zxmmcplus_port_7f_value & 0x1F;
}

z80_byte zxmmcplus_read_rom_byte(z80_int dir)
{
    int pagina=zxmmcplus_get_page_number();

    int offset_pagina=pagina*16384;
    int dir_final=offset_pagina+dir;
    //printf("Getting rom byte offset %X dir %X\n",dir_final,dir);
	return zxmmcplus_memory_pointer[dir_final];
}

int zxmmcplus_get_offset_to_ram_dir(z80_int dir)
{
    int pagina=zxmmcplus_get_page_number();

    int offset_pagina=pagina*16384;

    //Saltar ROM
    offset_pagina +=ZXMMCPLUS_FLASHROM_SIZE;

    int dir_final=offset_pagina+dir;

    return dir_final;
}

z80_byte zxmmcplus_read_ram_byte(z80_int dir)
{
    int dir_final=zxmmcplus_get_offset_to_ram_dir(dir);
	return zxmmcplus_memory_pointer[dir_final];
}

void zxmmcplus_poke_ram(z80_int dir,z80_byte value)
{

    if (zxmmcplus_port_7f_value & 128) {
        int dir_final=zxmmcplus_get_offset_to_ram_dir(dir);
        //printf("Poke to ram address %X value %X\n",dir,value);
        zxmmcplus_memory_pointer[dir_final]=value;
    }

}

void zxmmcplus_poke_rom(z80_int dir,z80_byte value)
{

    printf("Writing rom address %X value %X\n",dir,value);

}

void zxmmcplus_poke(z80_int dir,z80_byte value)
{
    //Si RAM o ROM mapeada
    if (zxmmcplus_port_7f_value & 32) zxmmcplus_poke_rom(dir,value);
    else zxmmcplus_poke_ram(dir,value);
}

z80_byte zxmmcplus_peek(z80_int dir)
{
    //Si RAM o ROM mapeada
    if (zxmmcplus_port_7f_value & 32) return zxmmcplus_read_rom_byte(dir);
    else return zxmmcplus_read_ram_byte(dir);
}

z80_byte zxmmcplus_poke_byte(z80_int dir,z80_byte valor)
{

    //Llamar a anterior
    debug_nested_poke_byte_call_previous(zxmmcplus_nested_id_poke_byte,dir,valor);

    if (dir<16384) zxmmcplus_poke(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte zxmmcplus_poke_byte_no_time(z80_int dir,z80_byte valor)
{

	//Llamar a anterior
	debug_nested_poke_byte_no_time_call_previous(zxmmcplus_nested_id_poke_byte_no_time,dir,valor);


	if (dir<16384) zxmmcplus_poke(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte zxmmcplus_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(zxmmcplus_nested_id_peek_byte,dir);


	if (dir<16384 && zxmmcplus_port_7f_value & 64) {
		return zxmmcplus_peek(dir);
	}

	return valor_leido;
}

z80_byte zxmmcplus_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(zxmmcplus_nested_id_peek_byte_no_time,dir);

	if (dir<16384 && zxmmcplus_port_7f_value & 64) {
		return zxmmcplus_peek(dir);
	}

	return valor_leido;
}



void zxmmcplus_nmi(void)
{
    //TODO
}



//Establecer rutinas propias
void zxmmcplus_set_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting zxmmcplus poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	zxmmcplus_nested_id_poke_byte=debug_nested_poke_byte_add(zxmmcplus_poke_byte,"zxmmcplus poke_byte");
	zxmmcplus_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(zxmmcplus_poke_byte_no_time,"zxmmcplus poke_byte_no_time");
	zxmmcplus_nested_id_peek_byte=debug_nested_peek_byte_add(zxmmcplus_peek_byte,"zxmmcplus peek_byte");
	zxmmcplus_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(zxmmcplus_peek_byte_no_time,"zxmmcplus peek_byte_no_time");



}

//Restaurar rutinas de zxmmcplus
void zxmmcplus_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before zxmmcplus");


	debug_nested_poke_byte_del(zxmmcplus_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(zxmmcplus_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(zxmmcplus_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(zxmmcplus_nested_id_peek_byte_no_time);


}



void zxmmcplus_alloc_rom_ram_memory(void)
{
    //memoria de la ram y rom
    int size=ZXMMCPLUS_RAM_SIZE+ZXMMCPLUS_FLASHROM_SIZE;

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for zxmmcplus emulation",size/1024);

    zxmmcplus_memory_pointer=util_malloc(size,"No enough memory for zxmmcplus emulation");


}



int zxmmcplus_load_rom(void)
{

    FILE *ptr_zxmmcplus_romfile;
    int leidos=0;


    debug_printf (VERBOSE_INFO,"Loading zxmmcplus rom %s",ZXMMCPLUS_FLASHROM_FILE_NAME);

    open_sharedfile(ZXMMCPLUS_FLASHROM_FILE_NAME,&ptr_zxmmcplus_romfile);
    if (!ptr_zxmmcplus_romfile) {
        debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_zxmmcplus_romfile!=NULL) {

        leidos=fread(zxmmcplus_memory_pointer,1,ZXMMCPLUS_FLASHROM_SIZE,ptr_zxmmcplus_romfile);
        fclose(ptr_zxmmcplus_romfile);

    }



    if (leidos!=ZXMMCPLUS_FLASHROM_SIZE || ptr_zxmmcplus_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading zxmmcplus rom");
        return 1;
    }

    return 0;
}

void zxmmcplus_reset(void)
{

    if (zxmmcplus_enabled.v==0) {
        return;
    }


    zxmmcplus_port_7f_value=64+32; //page-in + rom select


}

void zxmmcplus_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable zxmmcplus on non Spectrum machine");
        return;
    }

	if (zxmmcplus_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"zxmmcplus Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling zxmmcplus interface");

	zxmmcplus_alloc_rom_ram_memory();


	if (zxmmcplus_load_rom()) return;

	zxmmcplus_set_peek_poke_functions();

	zxmmcplus_enabled.v=1;

	zxmmcplus_reset();

    //Y lo mandamos al boot
    reg_pc=0;

}

void zxmmcplus_disable(void)
{
	if (zxmmcplus_enabled.v==0) return;


	zxmmcplus_restore_peek_poke_functions();

	free(zxmmcplus_memory_pointer);


	zxmmcplus_enabled.v=0;
}



void zxmmcplus_write_port(z80_byte value)
{

    zxmmcplus_port_7f_value=value;
    printf("zxmmcplus_write_port value %02XH pc: %04XH\n",value,reg_pc);

}

z80_byte zxmmcplus_read_port(void)
{
    printf("zxmmcplus_read_port value %02XH (page %d) pc: %04XH\n",zxmmcplus_port_7f_value,zxmmcplus_port_7f_value & 0x1F,reg_pc);
    return zxmmcplus_port_7f_value;


}
