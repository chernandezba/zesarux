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

#include "cpu.h"
#include "zxevo.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "zxvision.h"
#include "screen.h"
#include "ula.h"
#include "operaciones.h"
#include "ide.h"


//Comun zxevo, tanto para baseconf como tsconf
z80_byte zxevo_nvram[ZXEVO_NVRAM_ELEMENTS];


//celda nvram seleccionada
z80_byte zxevo_last_port_dff7;



void zxevo_reset_nvram(void)
{
    int i;

    for (i=0;i<ZXEVO_NVRAM_ELEMENTS;i++) zxevo_nvram[i]=0;
}

/* Presencia de la interfaz Nemo IDE de ZX Evolution. */
z80_bit zxevo_nemo_ide_enabled={0};

// Byte alto retenido durante accesos de dieciséis bits al IDE.
static z80_byte zxevo_nemo_ide_high_latch;
// Byte bajo retenido durante escrituras de dieciséis bits al IDE.
static z80_byte zxevo_nemo_ide_low_latch;
// Indica que falta leer el byte alto de una palabra IDE.
static int zxevo_nemo_ide_read_high_pending;
// Indica que falta completar una palabra de escritura IDE.
static int zxevo_nemo_ide_write_low_pending;
// Indica que el byte alto IDE llegó por el puerto 11.
static int zxevo_nemo_ide_write_high_from_port_11;
// Último valor escrito en el registro de control IDE.
static z80_byte zxevo_nemo_ide_control;
// Comando IDE cuya transferencia de sectores se está contabilizando.
static z80_byte zxevo_nemo_ide_transfer_command;
// Bytes transferidos del sector IDE actual.
static int zxevo_nemo_ide_transfer_bytes;
// Sectores pendientes del comando IDE actual.
static int zxevo_nemo_ide_sectors_remaining;
// Traduce puerto_l IDE al índice del registro ATA; devuelve -1 si no corresponde.
static int zxevo_nemo_ide_register(z80_byte puerto_l)
{
    switch (puerto_l) {
        case 0x30: return 1;
        case 0x50: return 2;
        case 0x70: return 3;
        case 0x90: return 4;
        case 0xb0: return 5;
        case 0xd0: return 6;
        case 0xf0: return 7;
        default: return -1;
    }
}

// Comprueba si la interfaz Nemo IDE atiende puerto_l; devuelve 1 si corresponde.
int zxevo_nemo_ide_port(z80_byte puerto_l)
{
    if (!zxevo_nemo_ide_enabled.v) return 0;
    return puerto_l==0x10 || puerto_l==0x11 || puerto_l==0xc8 ||
           zxevo_nemo_ide_register(puerto_l)>=0;
}

// Descarta las mitades pendientes de una transferencia IDE; sin parámetros ni retorno.
static void zxevo_nemo_ide_sync_data(void)
{
    zxevo_nemo_ide_read_high_pending=0;
    zxevo_nemo_ide_write_low_pending=0;
    zxevo_nemo_ide_write_high_from_port_11=0;
}

// Avanza el LBA de los registros IDE al siguiente sector; sin parámetros ni retorno.
static void zxevo_nemo_ide_increment_lba(void)
{
    unsigned int lba;

    /* Los accesos observados del firmware ZX Evolution usan direccionamiento LBA. */
    if (!(ide_register_drive_head&0x40)) return;

    lba=((unsigned int)(ide_register_drive_head&0x0f)<<24) |
        ((unsigned int)ide_register_cylinder_high<<16) |
        ((unsigned int)ide_register_cylinder_low<<8) |
        ide_register_sector_number;
    lba++;

    ide_register_sector_number=lba&0xff;
    ide_register_cylinder_low=(lba>>8)&0xff;
    ide_register_cylinder_high=(lba>>16)&0xff;
    ide_register_drive_head=(ide_register_drive_head&0xf0) | ((lba>>24)&0x0f);
}

// Inicia el seguimiento de sectores para comando IDE; entrada: comando; sin retorno.
static void zxevo_nemo_ide_begin_transfer(z80_byte comando)
{
    zxevo_nemo_ide_transfer_command=0;
    zxevo_nemo_ide_transfer_bytes=0;
    zxevo_nemo_ide_sectors_remaining=0;

    if (comando==0x20 || comando==0x21 || comando==0x30) {
            zxevo_nemo_ide_transfer_command=comando;
            /* En ATA el valor cero representa una transferencia de 256 sectores. */
            zxevo_nemo_ide_sectors_remaining=ide_register_sector_count ?
                    ide_register_sector_count : 256;
    }
}

// Cuenta bytes transferidos y actualiza sectores y LBA; entrada: bytes; sin retorno.
static void zxevo_nemo_ide_data_transferred(int bytes)
{
    if (!zxevo_nemo_ide_transfer_command) return;

    zxevo_nemo_ide_transfer_bytes+=bytes;
    if (zxevo_nemo_ide_transfer_bytes<512) return;

    zxevo_nemo_ide_transfer_bytes=0;
    zxevo_nemo_ide_sectors_remaining--;
    ide_register_sector_count=(z80_byte)zxevo_nemo_ide_sectors_remaining;

    if (zxevo_nemo_ide_sectors_remaining>0) {
            /* ide.c procesa un sector por orden. ZX Evolution mantiene DRQ y avanza
               al siguiente sector repitiendo internamente la misma orden. */
            zxevo_nemo_ide_increment_lba();
            ide_write_command_block_register(7,zxevo_nemo_ide_transfer_command);
    }
    else zxevo_nemo_ide_transfer_command=0;
}

// Lee el registro IDE de puerto_l y devuelve su byte, incluida la división de palabras.
z80_byte zxevo_nemo_ide_read(z80_byte puerto_l)
{
    int registro=zxevo_nemo_ide_register(puerto_l);

    if (puerto_l==0x10) {
            /* En modo extendido dos lecturas consecutivas de #10 entregan
               primero el byte bajo y después el alto. En modo NemoIDE el
               segundo byte se recoge mediante #11. */
            if (zxevo_nemo_ide_read_high_pending) {
                    zxevo_nemo_ide_read_high_pending=0;
                    return zxevo_nemo_ide_high_latch;
            }

            z80_byte bajo=ide_read_command_block_register(0);
            zxevo_nemo_ide_high_latch=ide_read_command_block_register(0);
            zxevo_nemo_ide_data_transferred(2);
            zxevo_nemo_ide_read_high_pending=1;
            return bajo;
    }

    if (puerto_l==0x11) {
            zxevo_nemo_ide_read_high_pending=0;
            return zxevo_nemo_ide_high_latch;
    }

    zxevo_nemo_ide_sync_data();
    if (puerto_l==0xc8) return ide_read_command_block_register(7);
    return ide_read_command_block_register(registro);
}

// Escribe valor en el registro IDE de puerto_l; sin retorno.
void zxevo_nemo_ide_write(z80_byte puerto_l,z80_byte valor)
{
    int registro=zxevo_nemo_ide_register(puerto_l);

    if (puerto_l==0x11) {
            /* NemoIDE escribe primero el byte alto en #11 y después el
               byte bajo en #10, que completa la palabra ATA. */
            zxevo_nemo_ide_high_latch=valor;
            zxevo_nemo_ide_write_high_from_port_11=1;
            zxevo_nemo_ide_write_low_pending=0;
            return;
    }

    if (puerto_l==0x10) {
            if (zxevo_nemo_ide_write_high_from_port_11) {
                    ide_write_command_block_register(0,valor);
                    ide_write_command_block_register(0,zxevo_nemo_ide_high_latch);
                    zxevo_nemo_ide_data_transferred(2);
                    zxevo_nemo_ide_write_high_from_port_11=0;
                    return;
            }

            /* El modo extendido recibe por #10 los bytes bajo y alto de
               cada palabra, en ese orden. */
            if (!zxevo_nemo_ide_write_low_pending) {
                    zxevo_nemo_ide_low_latch=valor;
                    zxevo_nemo_ide_write_low_pending=1;
            }
            else {
                    ide_write_command_block_register(0,zxevo_nemo_ide_low_latch);
                    ide_write_command_block_register(0,valor);
                    zxevo_nemo_ide_data_transferred(2);
                    zxevo_nemo_ide_write_low_pending=0;
            }
            return;
    }

    zxevo_nemo_ide_sync_data();
    if (puerto_l==0xc8) {
            /* Bit SRST del registro Device Control. */
            if ((valor&4) && !(zxevo_nemo_ide_control&4)) ide_reset();
            zxevo_nemo_ide_control=valor;
            return;
    }
    ide_write_command_block_register(registro,valor);
    if (registro==7) {
        // Nemo acaba de recibir un comando ATA. Si TSConf tiene también SMUC
        // habilitado, la siguiente DMA IDE debe usar Nemo, no el último SMUC.
        if (MACHINE_IS_TSCONF) ide_smuc_dma_selected=0;
        zxevo_nemo_ide_begin_transfer(valor);
    }
}


// Reinicia las mitades pendientes y el seguimiento de sectores de Nemo IDE.
void zxevo_nemo_ide_reset(void)
{
    zxevo_nemo_ide_high_latch=0xff;
    zxevo_nemo_ide_low_latch=0;
    zxevo_nemo_ide_read_high_pending=0;
    zxevo_nemo_ide_write_low_pending=0;
    zxevo_nemo_ide_write_high_from_port_11=0;
    zxevo_nemo_ide_control=0;
    zxevo_nemo_ide_transfer_command=0;
    zxevo_nemo_ide_transfer_bytes=0;
    zxevo_nemo_ide_sectors_remaining=0;
}
