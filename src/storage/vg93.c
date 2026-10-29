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

#include "vg93.h"
#include "cpu.h"
#include "debug.h"
#include "betadisk.h"


/*

Emulación parcial de la controladora de disquete VG93 usada en Scorpion y Pentagon.
Actualmente solo se conecta a los puertos de Scorpion.

*/

/* Registro de sistema de la interfaz Beta Disk: unidad y cara seleccionadas. */
static z80_byte vg93_system;

/* Cilindro seleccionado por el registro de pista del VG93. */
static z80_byte vg93_track;

/* Número de sector del VG93, comenzando en uno. */
static z80_byte vg93_sector;

/* Último byte del registro de datos; sirve también como destino de SEEK. */
static z80_byte vg93_data;

/* Posición del siguiente byte durante una lectura física de sector. */
static int vg93_read_offset;

/* Estado de la transferencia, usado por los bits BUSY y DRQ. */
static int vg93_read_active;

/* Indica que el VG93 ha terminado el comando y ha generado INTRQ. */
static int vg93_intrq;

/* Conserva el estado de error del último comando del VG93. */
static z80_byte vg93_error;

/* Distingue los bits de estado de comandos de posicionamiento y lectura. */
static int vg93_type1;

/* Alterna el pulso INDEX en el estado de los comandos de tipo I. */
static int vg93_index;

/* Reinicia los registros del VG93 y de la interfaz Beta Disk.
   Entrada: ninguna. Salida: controlador sin transferencia pendiente. */
void vg93_reset(void)
{
    vg93_system=0x3c;
    vg93_track=0;
    vg93_sector=1;
    vg93_data=0;
    vg93_read_offset=0;
    vg93_read_active=0;
    vg93_intrq=0;
    vg93_error=0;
    vg93_type1=1;
    vg93_index=0;
}

/* Lee los registros físicos del VG93 y de la interfaz Beta Disk.
   Entrada: puerto 1Fh, 3Fh, 5Fh, 7Fh o FFh. Salida: valor del registro.
   En FFh, el bit 6 indica DRQ y el bit 7 indica fin de comando. */
z80_byte vg93_read_port(z80_int port)
{
    switch (port & 0xff) {
        case 0xff:
            return (vg93_read_active ? 0x40 : 0) |
                   (vg93_intrq ? 0x80 : 0);

        case 0x1f: {
            z80_byte status=vg93_error;
            if (vg93_read_active) status |= 0x03;
            else if (vg93_type1) {
                vg93_index ^=2;
                status |= vg93_index;
                if (vg93_track==0) status |= 0x04;
            }
            vg93_intrq=0;
            return status;
        }

        case 0x3f: return vg93_track;
        case 0x5f: return vg93_sector;

        case 0x7f:
            if (vg93_read_active) {
                int side=(vg93_system & 0x10) ? 0 : 1;
                int pista=(vg93_track << 1) | side;
                vg93_data=betadisk_get_byte_disk(pista,
                    vg93_sector-1,vg93_read_offset++);
                if (vg93_read_offset==256) {
                    vg93_read_active=0;
                    vg93_intrq=1;
                }
            }
            return vg93_data;
    }
    return 0xff;
}

/* Escribe los registros físicos del VG93 y de la interfaz Beta Disk.
   Entrada: puerto y valor. Salida: actualiza selección de cara, registros o
   inicia una transferencia de lectura de un sector TRD de 256 bytes. */
void vg93_write_port(z80_int port,z80_byte value)
{
    switch (port & 0xff) {
        case 0xff:
            vg93_system=value;
            break;

        case 0x3f:
            vg93_track=value;
            break;

        case 0x5f:
            vg93_sector=value;
            break;

        case 0x7f:
            vg93_data=value;
            break;

        case 0x1f:
            vg93_read_active=0;
            vg93_read_offset=0;
            vg93_intrq=0;
            vg93_error=0;
            vg93_type1=(value & 0x80)==0;

            if ((value & 0xf0)==0x80 || (value & 0xf0)==0x90) {
                if (vg93_sector>=1 && vg93_sector<=16 &&
                    vg93_track<80 && (vg93_system & 3)==0) {
                    vg93_read_active=1;
                }
                else {
                    vg93_error=0x10;
                    vg93_intrq=1;
                }
            }
            else if ((value & 0xf0)==0x10) {
                vg93_track=vg93_data;
                vg93_intrq=1;
            }
            else if ((value & 0xf0)==0x00) {
                vg93_track=0;
                vg93_intrq=1;
            }
            else if ((value & 0xf0)==0xd0) {
                vg93_intrq=1;
            }
            else {
                vg93_error=0x10;
                vg93_intrq=1;
            }
            break;
    }
}
