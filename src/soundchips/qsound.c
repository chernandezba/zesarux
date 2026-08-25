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
#include "qsound.h"
#include "audio.h"
#include "debug.h"
#include "ay38912.h"
#include "utils.h"



//Habilitado qsound a nivel global
int ql_qsound_is_enabled=0;

//Si se carga la rom del qsound
int ql_qsound_rom_enabled=1;

//Si se capturan las llamadas a la rom de qsound
int ql_qsound_handle_traps=0;

//Si el PIA esta habilitado
int ql_qsound_pia_enabled=1;

void ql_set_qsound_settings_on_enabled(void)
{
    if (ql_qsound_is_enabled) {
        //Lo habitual
        ql_qsound_rom_enabled=1;
        ql_qsound_handle_traps=0;
        ql_qsound_pia_enabled=1;
        ay_chip_present.v=1;
        qsound_load_rom();
    }
}

void ql_qsound_enable(void)
{
    ql_qsound_is_enabled=1;
    ql_set_qsound_settings_on_enabled();
}

void ql_qsound_disable(void)
{
    ql_qsound_is_enabled=0;
}


//Esto no lo uso, lo capturo por si en el futuro hace falta
moto_long qsound_sv_aybas=0xFFFFFFFF;

//Valor habitual para sv.ayjmp
moto_long qsound_sv_ayjmp=0x29B10;

void ql_traps_qsound(void)
{
    if (!ql_qsound_is_enabled) return;
    if (!ql_qsound_handle_traps) return;

    int i;
    moto_long puntero;

    //sale de leer direccion 0x28164
    //if (get_pc_register()==0x29C30) {
    if (get_pc_register()==qsound_sv_ayjmp) {
        printf("qsound sv.ayjmp (%X) D0=%X\n",qsound_sv_ayjmp,m68k_get_reg(NULL,M68K_REG_D0));

        switch (m68k_get_reg(NULL,M68K_REG_D0)) {
            case 1:
                printf("write ay\n");
            break;

            case 2:
                printf("read ay\n");
            break;

            case 3:
                printf("writeall ay. Address=%X\n",m68k_get_reg(NULL,M68K_REG_A1));
                puntero=m68k_get_reg(NULL,M68K_REG_A1);

                for (i=0;i<14;i++) {
                    out_port_ay(65533,i);
	                out_port_ay(49149,ql_readbyte(puntero+i));
                }
            break;

            case 4:
                printf("readall ay\n");
            break;

            default:
                printf("Unknown qsound function D0=%X\n",m68k_get_reg(NULL,M68K_REG_D0));
            break;
        }
    }
}



void ql_writebyte_qsound(unsigned int Address, unsigned char Data)
{




    //Cuando se genera el puntero que apunta a sv.ayjmp
    if (Address>=0x28164 && Address<=0x28167) {

        qsound_sv_ayjmp=(memoria_ql[0x28164] << 24) | (memoria_ql[0x28165] << 16) | (memoria_ql[0x28166] << 8) | memoria_ql[0x28167];

        //printf("Generating qsound_sv_ayjmp=%XH (Pointer Address Written=%XH)\n",qsound_sv_ayjmp,Address);
    }

    //Cuando se genera el puntero que apunta a sv.aybas
    if (Address>=0x28160 && Address<=0x28163) {

        qsound_sv_aybas=(memoria_ql[0x28160] << 24) | (memoria_ql[0x28161] << 16) | (memoria_ql[0x28162] << 8) | memoria_ql[0x28163];

        //printf("Generating qsound_sv_aybas=%XH (Pointer Address Written=%XH)\n",qsound_sv_aybas,Address);
    }



    if (!ql_qsound_is_enabled) return;


    //Temporal qsound
    /* Esto para el clon de alvaro alea

    Write 0xC3000 -> Write Address Register

Read 0xC3000 -> Read Status

Write 0xC3002 -> Write Register Value

Read 0xC3002 -> Read Register Value
    */

    if (Address>=0xC3000 && Address<=0xC3002) {
        //printf("Write Qsound Address %X Value %02X\n",Address,Data);
    }

}


unsigned char qsound_pia_data_port_a=0;

//de momento no usado
unsigned char qsound_pia_control_port_a=0;

unsigned char qsound_pia_data_port_b=0;

//de momento no usado
unsigned char qsound_pia_control_port_b=0;

void ql_writebyte_qsound_pia(unsigned int Address, unsigned char Data)
{

    if (!ql_qsound_is_enabled) return;

    if (ay_chip_present.v==0) return;

    if (!ql_qsound_pia_enabled) return;

    //The 6821 PIO is decoded using A0,A1 and A13 to A19, so will be available in the top 8K of the card (0b 1100 001xxxxx xxxxxxAB or 0xC2000 to 0xC3FFF)
    if ((Address & 0xFE000) == 0xC2000) {
    //if ((Address & 0x8003)>=0x8000 && (Address & 0x8003)<=0x8003) {
        int registro=Address&3;
        //printf("Write Qsound PIA Address %X Register %d Value %02X\n",Address,registro,Data);

        int bc1,bdir;

        switch (registro) {
            case 0:
                qsound_pia_data_port_a=Data;
            break;

            case 1:
                qsound_pia_control_port_a=Data;
            break;

            case 2:
                qsound_pia_data_port_b=Data;
                bc1=Data &1;
                bdir=Data & 8;

                if (bdir) {
                    if (bc1) {
                        //seleccionar registro
                        out_port_ay(65533,qsound_pia_data_port_a);
                    }
                    else {
                        //enviar valor a registro
	                    out_port_ay(49149,qsound_pia_data_port_a);
                    }
                }
            break;

            case 3:
                qsound_pia_control_port_b=Data;
            break;
        }
    }


}


void qsound_load_rom(void)
{

    moto_long direccion=0xc0000;
    int longitud_rom=8192;

    //Primero, por si se ha quitado la rom, vaciar esa zona de memoria
    memset(&memoria_ql[direccion],0,longitud_rom);


    if (!ql_qsound_is_enabled) return;

    if (!ql_qsound_rom_enabled) return;

    printf("Loading qsound rom\n");

    FILE *ptr_qsound_rom;

    open_sharedfile("Qsound_V1.94.rom",&ptr_qsound_rom);

    if (!ptr_qsound_rom) {
        debug_printf (VERBOSE_ERR,"Unable to load qsound rom");
    }
    else {

        fread(&memoria_ql[direccion],1,longitud_rom,ptr_qsound_rom);

        fclose(ptr_qsound_rom);
    }


}

