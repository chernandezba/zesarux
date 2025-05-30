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
#include <unistd.h>



#include "zxmmcplus.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "zesarux.h"
#include "mmc.h"
#include "mem128.h"
#include "menu_items.h"
#include "screen.h"



z80_bit zxmmcplus_enabled={0};

z80_bit zxmmcplus_flashrom_write_protect={0};

//Si cambios en escritura se hace flush a disco
z80_bit zxmmcplus_flashrom_persistent_writes={0};

int zxmmcplus_flashrom_must_flush_to_disk=0;


//Apunta a memoria rom y ram. Primero rom, luego ram
z80_byte *zxmmcplus_memory_pointer;


//Nombre de la flash. Si "", nombre y ruta por defecto
char zxmmcplus_flash_name[PATH_MAX]="";


//Solo hacemos flush al salir de ZEsarUX o al desactivar zxmmc+,
//dado que guardamos flashrom + ram, los cambios en ram pueden ser continuos y no queremos continuamente que se haga flush desde timer
void zxmmcplus_flashrom_flush_contents_to_disk(void)
{

    if (zxmmcplus_enabled.v==0) return;


    if (zxmmcplus_flashrom_must_flush_to_disk==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush zxmmcplus_flashrom to disk but no changes made");
        return;
    }

    if (zxmmcplus_flashrom_persistent_writes.v==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush zxmmcplus_flashrom to disk but persistent writes disabled");
        return;
    }


    debug_printf (VERBOSE_INFO,"Flushing zxmmcplus_flashrom to disk");


	FILE *ptr_zxmmcplus_flashromfile;

	if (zxmmcplus_flash_name[0]==0) {
		open_sharedfile_write(ZXMMCPLUS_FLASHROM_FILE_NAME,&ptr_zxmmcplus_flashromfile);
	}
	else {
		debug_printf (VERBOSE_INFO,"Opening ZXMMC+ Custom Flash File %s",zxmmcplus_flash_name);
		ptr_zxmmcplus_flashromfile=fopen(zxmmcplus_flash_name,"wb");
	}


	int escritos=0;
	int size=ZXMMCPLUS_FLASHROM_SIZE+ZXMMCPLUS_RAM_SIZE;


	if (ptr_zxmmcplus_flashromfile!=NULL) {
		z80_byte *puntero;
		puntero=zxmmcplus_memory_pointer;

        //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
        //metera flush a 1
		zxmmcplus_flashrom_must_flush_to_disk=0;
		escritos=fwrite(puntero,1,size,ptr_zxmmcplus_flashromfile);

		fclose(ptr_zxmmcplus_flashromfile);


	}

	//printf ("ptr_zxmmcplus_flashromfile: %d\n",ptr_zxmmcplus_flashromfile);
	//printf ("escritos: %d\n",escritos);

	if (escritos!=size || ptr_zxmmcplus_flashromfile==NULL) {
		debug_printf (VERBOSE_ERR,"Error writing to zxmmcplus_flashrom file. Disabling write file operations");
		zxmmcplus_flashrom_persistent_writes.v=0;
	}

}




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

---

OLD INFO from ZX Badaloc:
The non-standard $54DF register allows quick access to ANY ROM/RAM bank (up to 512Kbytes each) that will be
mapped in address space 0-$3FFF, with a single OUT operation:

D7:       RAM Write Enable (standalone);
D6:       ‘1’ = Fast Paging Enabled
D5:       ‘1’ = ROM is paged; ‘0’ = RAM is paged;
D4:D0   5 upper address bit (bank select), up to 32 banks of 16Kbytes each

Setting bit D6 will enable fast paging mode. Bank selected by bits D4:D0 will appear on address space 0-$3FFF.
If D5 is also set, the rom chip is accessed. If D5 is reset, then the 128K/512K ram chip is accessed.

Setting bit D7 will allow the ram bank selected by D4:D0 to be WRITTEN on address space 0-$3FFF, without affecting
the active device for read operation. This allows quick ROM to RAM copy.

*/
z80_byte zxmmcplus_port_7f_value=0;

int zxmmcplus_pending_read_flashrom_status=0;
z80_byte zxmmcplus_flashrom_status=0;

int zxmmcplus_get_page_number(void)
{
    return zxmmcplus_port_7f_value & 0x1F;
}

void zxmmcplus_sync_bit_7ffd(void)
{
    //This feature is disabled when zxmmc+ RAM is paged-in
    if ( (zxmmcplus_port_7f_value & 0x60) == 0x40) return;


    z80_byte bit_d4=(puerto_32765 >> 4) & 1;

    zxmmcplus_port_7f_value &=0xFE;

    zxmmcplus_port_7f_value |=bit_d4;

    //printf("Sync bit D4 of 7ffd=%02XH zxmmcplus_port_7f_value=%02XH\n",puerto_32765,zxmmcplus_port_7f_value);
}

z80_byte zxmmcplus_read_rom_byte(z80_int dir)
{

    if (zxmmcplus_pending_read_flashrom_status) {
        //printf("Returning flash rom status byte value %X\n",zxmmcplus_flashrom_status);
        zxmmcplus_pending_read_flashrom_status=0;
        return zxmmcplus_flashrom_status;
    }

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

void zxmmcplus_footer_print_flash_operating(void)
{


	generic_footertext_print_operating("FLASH");
    watermark_tell_device_activity();

    //Y poner icono en inverso

    if (!zxdesktop_icon_zxmmcplusflash_inverse) {
        zxdesktop_icon_zxmmcplusflash_inverse=1;
        menu_draw_ext_desktop();
    }

}

void zxmmcplus_write_flashrom_byte(int dir,z80_byte value)
{
    //Avisar de actividad en la flash. Solo al escribir, al leer no porque si está la rom mapeada se leeria continuamente
    //y continuamente avisaria de actividad
    zxmmcplus_footer_print_flash_operating();

    if (zxmmcplus_flashrom_write_protect.v==0) {
        debug_printf (VERBOSE_PARANOID,"ZXMMC+ Flash ROM Write Address %XH Value %02XH",dir,value);
        zxmmcplus_memory_pointer[dir]=value;
        zxmmcplus_flashrom_must_flush_to_disk=1;
    }


}

void zxmmcplus_poke_ram(z80_int dir,z80_byte value)
{

    if (zxmmcplus_port_7f_value & 128) {
        int dir_final=zxmmcplus_get_offset_to_ram_dir(dir);

        if ((zxmmcplus_port_7f_value & 32)==0) {
            //printf("--Poke to ram address with rom visible for reading %X value %X\n",dir,value);
        }
        zxmmcplus_memory_pointer[dir_final]=value;
        zxmmcplus_flashrom_must_flush_to_disk=1;
    }

}

void zxmmc_flashrom_blockerase(int bloque)
{
    int offset=bloque*65536;

    int i;

    for (i=0;i<65536;i++) {
        zxmmcplus_write_flashrom_byte(offset++,255);
    }
}

/*
Dividir los comandos de flash rom por tipos

Read/Reset
Auto Select
Program
Unlock Bypass
Unlock Bypass Program
Unlock Bypass Reset
Chip Erase
Block Erase
Erase Suspend
Erase Resume

*/

//Definir varios prefijos de comandos
enum zxmmcplus_flashrom_commands_prefixes {
    PREFIX_NONE,
    UNLOCK_BYPASS_PROGRAM, //X A0
    UNLOCK_BYPASS_RESET, //X 90
    COMMON_LONG_ONE, //555 AA, 2AA 55
    PROGRAM, //555 AA, 2AA 55, 555 A0
    COMMON_LONG_TWO, //555 AA, 2AA 55, 555 80, 555 AA, 2AA 55
};

//Que orden de byte estamos escribiendo
int zxmmcplus_romwrite_index=1;

enum zxmmcplus_flashrom_commands_prefixes zxmmcplus_romwrite_current_prefix=PREFIX_NONE;

/*
Emulación de toda la parte de escritura en flash rom del chip M29F040B
Sólo emulamos los comandos que se están usando desde las roms modificadas para zxmmc+, que generan
snapshots en flashrom al pulsar la nmi y tecla R
El resto de comandos no se emulan
*/
void zxmmcplus_poke_rom(z80_int dir,z80_byte value)
{

    //printf("Writing rom address %X value %X pc %X\n",dir,value,reg_pc);
    if ((mmc_last_port_value_1f & 0xF0 )!=0xa0) {
        //printf("Flash is not unlocked for writing (1f port=%02XH)\n",mmc_last_port_value_1f);
        return;
    }

    int pagina=zxmmcplus_get_page_number();

    int dir_final=pagina*16384;

    dir_final +=dir;

    //The Command Interface only uses address bits A0-A10 to verify the commands, the upper address bits are Don’t Care.
    int command=dir_final & 0x7FF;

    //printf("Writing rom. Index command=%d\n",zxmmcplus_romwrite_index);

    int i;

    switch (zxmmcplus_romwrite_index) {
        //Primer byte de comando
        case 1:
            switch(command) {
                case 0x555:
                    if (value==0xAA) {
                        //printf("Setting to preffix COMMON_LONG_ONE\n");
                        zxmmcplus_romwrite_current_prefix=COMMON_LONG_ONE;
                        zxmmcplus_romwrite_index++;
                        return;
                    }
                    else {
                        //printf("Unknown value when command 0x555\n");

                    }
                break;
            }

            switch(value) {
                case 0xA0:
                    //printf("Setting to preffix UNLOCK_BYPASS_PROGRAM\n");
                    zxmmcplus_romwrite_current_prefix=UNLOCK_BYPASS_PROGRAM;
                    zxmmcplus_romwrite_index++;
                    return;
                break;

                case 0x90:
                    //printf("Setting to preffix UNLOCK_BYPASS_RESET\n");
                    zxmmcplus_romwrite_current_prefix=UNLOCK_BYPASS_RESET;
                    zxmmcplus_romwrite_index++;
                    return;
                break;

                case 0xF0:
                    //Fin Comando Read/Reset
                    //printf("Fin Comando Read/Reset (short)\n");
                    zxmmcplus_pending_read_flashrom_status=0;
                break;

                case 0xB0:
                    //Fin Comando Erase Suspend. TODO
                    //printf("Fin Comando Erase Suspend. TODO\n");

                break;

                case 0x30:
                    //Fin Comando Erase Resume. TODO
                    //printf("Fin Comando Erase Resume. TODO\n");

                break;

                default:
                    //printf("Unknown command index %d Addr %02X Data %02X\n",zxmmcplus_romwrite_index,command,value);

                break;
            }
        break;

        //Segundo byte de comando
        case 2:
            if (zxmmcplus_romwrite_current_prefix==UNLOCK_BYPASS_PROGRAM) {
                //Fin comando Unlock Bypass Program. Usar dir_final y value para implementar este comando. TODO
                //printf("Fin Comando Unlock Bypass Program. TODO\n");

                zxmmcplus_romwrite_index=1;
                return;
            }
            else if (zxmmcplus_romwrite_current_prefix==UNLOCK_BYPASS_RESET) {
                if (value==0x00) {
                    //Fin comando Unlock Bypass Reset.TODO
                    //printf("Fin Comando Unlock Bypass Reset. TODO\n");

                    zxmmcplus_romwrite_index=1;
                    return;
                }
                else {
                    //printf("Unknown command index %d Addr %02X Data %02X\n",zxmmcplus_romwrite_index,command,value);
                    zxmmcplus_romwrite_index=1;

                    return;
                }
            }
            else {
                if (command!=0x2AA || value!=0x55) {
                    //printf("Unknown command index %d Addr %02X Data %02X\n",zxmmcplus_romwrite_index,command,value);
                    zxmmcplus_romwrite_index=1;

                    return;
                }
                else {
                    zxmmcplus_romwrite_index++;
                    return;
                }
            }


        break;

        //Tercer byte de comando
        case 3:
            if (value==0xF0) {
                //Fin comando Read/Reset
                zxmmcplus_romwrite_index=1;
                //printf("Fin Comando Read/Reset (long)\n");
                zxmmcplus_pending_read_flashrom_status=0;
                zxmmcplus_romwrite_index=1;
                return;
            }

            else {
                if (command!=0x555) {
                    //printf("Unknown command index %d Addr %02X Data %02X\n",zxmmcplus_romwrite_index,command,value);
                    zxmmcplus_romwrite_index=1;

                    return;
                }
                else {
                    switch(value) {
                        case 0x90:
                            //Fin comando AutoSelect. TODO
                            zxmmcplus_romwrite_index=1;
                            //printf("Fin Comando AutoSelect. TODO\n");

                            return;
                        break;

                        case 0xA0:
                            //printf("Setting to preffix PROGRAM\n");
                            zxmmcplus_romwrite_current_prefix=PROGRAM;
                            zxmmcplus_romwrite_index++;
                            return;
                        break;

                        case 0x20:
                            //Fin comando Unlock bypass. TODO
                            zxmmcplus_romwrite_index=1;
                            //printf("Fin Comando Unlock bypass. TODO\n");

                            return;
                        break;

                        case 0x80:
                            //printf("Setting to preffix COMMON_LONG_TWO\n");
                            zxmmcplus_romwrite_current_prefix=COMMON_LONG_TWO;
                            zxmmcplus_romwrite_index++;
                            return;
                        break;

                        default:
                            //printf("Unknown command index %d Addr %02X Data %02X\n",zxmmcplus_romwrite_index,command,value);
                            zxmmcplus_romwrite_index=1;

                            return;
                        break;
                    }
                }
            }
        break;

        //Cuarto byte de comando
        case 4:
            if (zxmmcplus_romwrite_current_prefix==PROGRAM) {
                //Fin comando Program. Usar dir_final y value para escribir en flash rom
                //printf("Command Program. Writing rom address %X value %X\n",dir_final,value);
                zxmmcplus_write_flashrom_byte(dir_final,value);
                zxmmcplus_romwrite_index=1;
                return;
            }

            else {
                if (command!=0x555 || value!=0xAA) {
                    //printf("Unknown command index %d Addr %02X Data %02X\n",zxmmcplus_romwrite_index,command,value);
                    zxmmcplus_romwrite_index=1;

                    return;
                }
                else {
                    zxmmcplus_romwrite_index++;
                    return;
                }
            }
        break;

        //Quinto byte de comando
        case 5:
                if (command!=0x2AA || value!=0x55) {
                    //printf("Unknown command index %d Addr %02X Data %02X\n",zxmmcplus_romwrite_index,command,value);
                    zxmmcplus_romwrite_index=1;

                    return;
                }
                else {
                    zxmmcplus_romwrite_index++;
                    return;
                }
        break;

        //Sexto byte de comando
        case 6:
            if (command==0x555 && value==0x10) {
                //Fin comando Chip Erase.
                debug_printf (VERBOSE_DEBUG,"ZXMMC+ Flash ROM Chip Erase");
                for (i=0;i<8;i++) zxmmc_flashrom_blockerase(i);
                zxmmcplus_romwrite_index=1;
                zxmmcplus_pending_read_flashrom_status=1;
                zxmmcplus_flashrom_status=255;
                return;
            }
            else if (value==0x30) {
                //Fin comando Block Erase.
                //Bloque se identifica con la dirección
                int bloque=dir_final/65536;
                debug_printf (VERBOSE_DEBUG,"ZXMMC+ Flash ROM Block Erase block %d (address %XH)",bloque,dir_final);
                zxmmc_flashrom_blockerase(bloque);
                zxmmcplus_romwrite_index=1;

                //Y despues de erase la primera lectura de la rom devuelve el status
                //TODO: esto es muy chapucero como implemento el byte de status, de momento es solo para
                //que las operaciones de escritura desde la nmi lo den como valido
                //Ahora lo que hago es que cuando está este estado, la primera lectura retorna el estado y se baja el flag
                //Lo normal seria esperar un tiempo concreto para cambiar ese estado (que creo que es como funciona el chip flashrom)
                zxmmcplus_pending_read_flashrom_status=1;
                zxmmcplus_flashrom_status=255;

                return;
            }
            else {
                //printf("Unknown command index %d Addr %02X Data %02X\n",zxmmcplus_romwrite_index,command,value);
                zxmmcplus_romwrite_index=1;

                return;
            }
        break;
    }

}

void zxmmcplus_poke(z80_int dir,z80_byte value)
{

    if (zxmmcplus_rom_selected() ) zxmmcplus_poke_rom(dir,value);

    zxmmcplus_poke_ram(dir,value);
}

int zxmmcplus_rom_selected(void)
{
    if (zxmmcplus_port_7f_value & 32) return 1;
    else return 0;
}

z80_byte zxmmcplus_peek(z80_int dir)
{
    //Si RAM o ROM mapeada
    if (zxmmcplus_rom_selected() ) return zxmmcplus_read_rom_byte(dir);
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

int zxmmcplus_paged_on_read(void)
{
    if (zxmmcplus_port_7f_value & 64) return 1;
    else return 0;
}

z80_byte zxmmcplus_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(zxmmcplus_nested_id_peek_byte,dir);


	if (dir<16384 && zxmmcplus_paged_on_read() ) {
		return zxmmcplus_peek(dir);
	}

	return valor_leido;
}

z80_byte zxmmcplus_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(zxmmcplus_nested_id_peek_byte_no_time,dir);

	if (dir<16384 && zxmmcplus_paged_on_read() ) {
		return zxmmcplus_peek(dir);
	}

	return valor_leido;
}







//Establecer rutinas propias
void zxmmcplus_set_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting zxmmc+ poke / peek functions");

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

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for zxmmc+ emulation",size/1024);

    //Asigno y relleno con 0, no es que sea estrictamente necesario, pero asi queda la memoria asignada mas "limpia" desde el principio
    zxmmcplus_memory_pointer=util_malloc_fill(size,"No enough memory for zxmmc+ emulation",0);


}

void zxmmcplus_clear_ram(void)
{
    //Saltar ROM
    int offset_ram=ZXMMCPLUS_FLASHROM_SIZE;

    int i;

    for (i=0;i<ZXMMCPLUS_RAM_SIZE;i++) {
        zxmmcplus_memory_pointer[offset_ram+i]=0;
    }

    zxmmcplus_flashrom_must_flush_to_disk=1;

}


//En el archivo de flash guardamos la ROM y la RAM
int zxmmcplus_load_flash(void)
{

    FILE *ptr_zxmmcplus_flashfile;
    int leidos=0;

    int size=ZXMMCPLUS_FLASHROM_SIZE+ZXMMCPLUS_RAM_SIZE;

	if (zxmmcplus_flash_name[0]==0) {
		open_sharedfile(ZXMMCPLUS_FLASHROM_FILE_NAME,&ptr_zxmmcplus_flashfile);
	}
	else {
		debug_printf (VERBOSE_INFO,"Opening ZXMMC+ Custom Flash File %s",zxmmcplus_flash_name);
		ptr_zxmmcplus_flashfile=fopen(zxmmcplus_flash_name,"rb");
	}

    if (ptr_zxmmcplus_flashfile!=NULL) {

        leidos=fread(zxmmcplus_memory_pointer,1,size,ptr_zxmmcplus_flashfile);
        fclose(ptr_zxmmcplus_flashfile);

    }



    if (leidos!=size || ptr_zxmmcplus_flashfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading zxmmc+ flash rom");
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

    //printf("ZXMMC+ reset\n");
}

void zxmmcplus_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable zxmmc+ on non Spectrum machine");
        return;
    }

	if (zxmmcplus_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"zxmmc+ Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling zxmmc+ interface");

	zxmmcplus_alloc_rom_ram_memory();


	if (zxmmcplus_load_flash()) return;

	zxmmcplus_set_peek_poke_functions();

	zxmmcplus_enabled.v=1;

	zxmmcplus_reset();

    //Y lo mandamos al boot
    reg_pc=0;

}

void zxmmcplus_disable(void)
{
	if (zxmmcplus_enabled.v==0) return;

    zxmmcplus_flashrom_flush_contents_to_disk();


	zxmmcplus_restore_peek_poke_functions();

	free(zxmmcplus_memory_pointer);


	zxmmcplus_enabled.v=0;
}



void zxmmcplus_write_port(z80_byte value)
{

    zxmmcplus_port_7f_value=value;
    //printf("zxmmcplus_write_port value %02XH (page %d) pc: %04XH\n",value,zxmmcplus_port_7f_value & 0x1F,reg_pc);

}

z80_byte zxmmcplus_read_port(void)
{
    //printf("zxmmcplus_read_port value %02XH (page %d) pc: %04XH\n",zxmmcplus_port_7f_value,zxmmcplus_port_7f_value & 0x1F,reg_pc);
    return zxmmcplus_port_7f_value;


}


//Card select. Solo guardar el valor cuando mmc esta desactivado, ese mmc_last_port_value_1f se usa en la proteccion de rom
void zxmmcplus_mmc_cs(z80_byte value)
{
    mmc_last_port_value_1f=value;
}