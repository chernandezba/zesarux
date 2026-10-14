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
#include <time.h>

#include "cpu.h"
#include "scorpion.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "zxvision.h"
#include "screen.h"
#include "ula.h"
#include "operaciones.h"
#include "start.h"
#include "betadisk.h"
#include "ide.h"

/*

ZS-256	Paginación, ROM, monitor NMI y temporización Scorpion
ZS-256 Turbo+	CPU Turbo, Diferencias de temporización, puertos y lógica
Scorpion GMX	Nueva paginación, ROM ampliada y vídeo adicional

*/

/*

Scorpion ZS 256:

RAM 256 KB

| Puerto | Bits | Función |
|---|---|---|
| `7FFDh` | D0–D2 | Seleccionan una página de 0 a 7. |
| `1FFDh` | D4 | Añade 8 al número de página cuando vale 1. |


ROM 64 KB

| Página | Desplazamiento en la ROM | Contenido | Selección |
|---|---:|---|---|
| 0 | `0000h` | BASIC 128 | `1FFDh.D1=0`, `7FFDh.D4=0` |
| 1 | `4000h` | BASIC 48 | `1FFDh.D1=0`, `7FFDh.D4=1` |
| 2 | `8000h` | Monitor de servicio | `1FFDh.D1=1` |
| 3 | `C000h` | TR-DOS | Se activa mediante la lógica de TR-DOS, no mediante la selección BASIC de `7FFDh`. |

*/


//Donde esta cada pagina de ram
z80_byte *scorpion_ram_mem_table[SCORPIONGMX_TOTAL_RAM_PAGES];

//Donde esta cada pagina de rom
z80_byte *scorpion_rom_mem_table[SCORPIONGMX_TOTAL_ROM_PAGES];

//Direcciones actuales mapeadas
z80_byte *scorpion_memory_paged[4];
static z80_bit scorpion_trdos_active={0};
/* Último valor escrito en 7EFD (la lectura de este puerto devuelve otro estado):
 * D0 EWR: permite escribir en la memoria Flash.
 * D1 Vpp: activa la tensión de programación de la Flash.
 * D2 MagEnable: 0 permite la tecla Magic; 1 bloquea su NMI.
 * D3 Graphic: 0 usa vídeo Spectrum; 1 usa el modo GMX de 640x200.
 * D4 A16, D5 A17, D6 A18: eligen uno de ocho bloques de 64 KiB de Flash.
 * D7 Normal/Turbo: 0 velocidad normal; 1 velocidad doble.
 */
static z80_byte scorpion_gmx_port_7efd;
/* Último valor escrito en DFFD, extensión de RAM de tipo Profi:
 * D0 SM0, D1 SM1, D2 SM2: bits 4, 5 y 6 del banco de 16 KiB en C000h.
 * Los bits 0-2 proceden de 7FFD y el bit 3 de 1FFD.D4.
 * D3-D7: sin función documentada para GMX.
 */
static z80_byte scorpion_gmx_port_dffd;
/* Último valor escrito en 78FD (segunda ventana de RAM, 8000h-BFFFh):
 * D0 SW0, D2 SW2, D3 SW3: bits 0, 2 y 3 del banco de 16 KiB.
 * D1 SW1: bit 1 del banco, invertido físicamente; al reset vale 0
 *          y queda seleccionada la página 2.
 * D4 SW4, D5 SW5, D6 SW6: bits 4, 5 y 6 del banco.
 * D7: sin función documentada en escritura; en lectura refleja el
 *     bit 1 del color del borde.
 */
static z80_byte scorpion_gmx_port_78fd;
/* Desplazamiento vertical del vídeo ampliado: 7AFD aporta D4-D7 del
 * byte bajo y 7CFD aporta D0-D5 del byte alto. D0-D3 del byte bajo
 * siempre valen cero. El resultado es un desplazamiento de 14 bits.
 */
static z80_byte scorpion_gmx_scroll_low;
static z80_byte scorpion_gmx_scroll_high;
/* Último valor escrito en el puerto global xx00:
 * D0 CONF0, D1 CONF1, D2 CONF2: variante de configuración FLEX8000.
 * D3 RECONF: solicita cargar esa configuración y reiniciar GMX.
 * D4 FIXROM: fija A16-A18 de la Flash ante escrituras a 7EFD y reset.
 * D5 BLKEXT: desactiva las ampliaciones GMX.
 * D6 BLKEX2: junto con D5 desactiva los puertos xxFD y cambia la
 *             decodificación de 7FFD al modo compatible.
 * D7 ST/PENT: 0 temporización Scorpion; 1 temporización Pentagon.
 * El reset no borra este registro.
 */
static z80_byte scorpion_gmx_port_00;

/* FFBAh: D0 reinicia ATA, D4 es SDA, D6 es SCL y D7 selecciona
 * los registros alternativos ATA y la escritura en RTC. */
static z80_byte scorpion_gmx_smuc_control=1;
/* 7FBAh: registro de configuración del SMUC; los bits 0-2 y 4-5
 * se leen siempre como unos en esta revisión del controlador. */
static z80_byte scorpion_gmx_smuc_config;
/* D8BEh conserva el byte alto del último dato ATA de 16 bits. */
static z80_byte scorpion_gmx_smuc_data_high;
/* DFBAh: dirección seleccionada y contenido de los registros CMOS. */
static z80_byte scorpion_gmx_smuc_cmos_address;
static z80_byte scorpion_gmx_smuc_cmos[128];
/* Estado de la interrupción de actualización del reloj, registro C. */
static time_t scorpion_gmx_smuc_cmos_last_second;
static int scorpion_gmx_smuc_cmos_update_flag;
/* Sustituye únicamente la palabra 0 de IDENTIFY por un disco ATA fijo. */
static int scorpion_gmx_smuc_identify_first_word;
/* EEPROM 24LC16 y estado de la transferencia serie por SDA/SCL. */
static z80_byte scorpion_gmx_smuc_eeprom[2048];
static int scorpion_gmx_smuc_eeprom_initialized;
static int scorpion_gmx_smuc_eeprom_state;
static int scorpion_gmx_smuc_eeprom_address;
static int scorpion_gmx_smuc_eeprom_input;
static int scorpion_gmx_smuc_eeprom_input_bits;
static int scorpion_gmx_smuc_eeprom_output;
static int scorpion_gmx_smuc_eeprom_output_bits;
static int scorpion_gmx_smuc_eeprom_release;
static z80_byte scorpion_gmx_smuc_eeprom_bus=0xff;

enum {
    SCORPION_SMUC_EEPROM_IDLE,
    SCORPION_SMUC_EEPROM_COMMAND,
    SCORPION_SMUC_EEPROM_ADDRESS,
    SCORPION_SMUC_EEPROM_DATA,
    SCORPION_SMUC_EEPROM_SEND,
    SCORPION_SMUC_EEPROM_ACK
};
/* Registro interno de lectura serie por 78FD: tras RECONF contiene
 * 88h OR CONF0..CONF2. Cada lectura entrega el bit 0 y lo desplaza.
 */
static z80_byte scorpion_gmx_magic_shift;

z80_byte scorpion_read_ay_port_a(z80_byte ay_value)
{
    // En Turbo+ las entradas 0-3 y 5 leen D0-D4 de 7FFD; la entrada 4 lee D4 de 1FFD.
    return (ay_value & 0xc0) | (puerto_32765 & 0x0f) |
           (puerto_8189 & 0x10) | ((puerto_32765 & 0x10) << 1);
}

void scorpion_init_memory_tables(void)
{
    //Primero rom y luego ram

    int i;
    z80_byte *puntero=memoria_spectrum;

    for (i=0;i<SCORPIONGMX_TOTAL_ROM_PAGES;i++) {
        scorpion_rom_mem_table[i]=puntero;
        puntero +=16384;
    }

    for (i=0;i<SCORPIONGMX_TOTAL_RAM_PAGES;i++) {
        scorpion_ram_mem_table[i]=puntero;
        puntero +=16384;
    }


}

void scorpion_set_normal_pages(void)
{
    scorpion_trdos_active.v=0;
    scorpion_memory_paged[0]=scorpion_rom_mem_table[0];
    scorpion_memory_paged[1]=scorpion_ram_mem_table[5];
    scorpion_memory_paged[2]=scorpion_ram_mem_table[2];
    scorpion_memory_paged[3]=scorpion_ram_mem_table[0];


    if (MACHINE_IS_SCORPION_GMX) {
        scorpion_gmx_smuc_control=1;
        scorpion_gmx_smuc_config=0;
        scorpion_gmx_smuc_data_high=0;
        scorpion_gmx_smuc_cmos_address=0;
        scorpion_gmx_smuc_cmos_last_second=0;
        scorpion_gmx_smuc_cmos_update_flag=0;
        scorpion_gmx_smuc_identify_first_word=0;
        scorpion_gmx_smuc_eeprom_state=SCORPION_SMUC_EEPROM_IDLE;
        scorpion_gmx_smuc_eeprom_address=0;
        scorpion_gmx_smuc_eeprom_input=0;
        scorpion_gmx_smuc_eeprom_input_bits=0;
        scorpion_gmx_smuc_eeprom_output=0;
        scorpion_gmx_smuc_eeprom_output_bits=0;
        scorpion_gmx_smuc_eeprom_release=1;
        scorpion_gmx_smuc_eeprom_bus=0xff;
        if (!scorpion_gmx_smuc_eeprom_initialized) {
            int i;
            for (i=0;i<2048;i++) scorpion_gmx_smuc_eeprom[i]=0xff;
            scorpion_gmx_smuc_eeprom_initialized=1;
        }
        scorpion_gmx_port_7efd=0;
        scorpion_gmx_port_dffd=0;
        scorpion_gmx_port_78fd=0;
        scorpion_gmx_scroll_low=0;
        scorpion_gmx_scroll_high=0;
    }

}

/* Procesa el bus serie de la EEPROM 24LC16 del SMUC.
 * Entrada: valor escrito en FFBAh. Salida: ninguna; actualiza SDA leído.
 */
static void scorpion_gmx_smuc_eeprom_write(z80_byte value)
{
    int old_clock=scorpion_gmx_smuc_control & 0x40;
    int new_clock=value & 0x40;
    int old_data=scorpion_gmx_smuc_control & 0x10;
    int new_data=value & 0x10;

    if (old_clock!=new_clock) {
        if (new_clock) {
            if (scorpion_gmx_smuc_eeprom_state==SCORPION_SMUC_EEPROM_ACK) {
                if (new_data) scorpion_gmx_smuc_eeprom_state=SCORPION_SMUC_EEPROM_IDLE;
                else {
                    scorpion_gmx_smuc_eeprom_state=SCORPION_SMUC_EEPROM_SEND;
                    scorpion_gmx_smuc_eeprom_output=
                        scorpion_gmx_smuc_eeprom[scorpion_gmx_smuc_eeprom_address];
                    scorpion_gmx_smuc_eeprom_address=(scorpion_gmx_smuc_eeprom_address+1)&0x7ff;
                    scorpion_gmx_smuc_eeprom_output_bits=0;
                }
            }
            else if (scorpion_gmx_smuc_eeprom_release &&
                     scorpion_gmx_smuc_eeprom_state>=SCORPION_SMUC_EEPROM_COMMAND &&
                     scorpion_gmx_smuc_eeprom_state<=SCORPION_SMUC_EEPROM_DATA) {
                scorpion_gmx_smuc_eeprom_input=(scorpion_gmx_smuc_eeprom_input<<1)|!!new_data;
                scorpion_gmx_smuc_eeprom_input_bits++;
            }
        }
        else {
            if (scorpion_gmx_smuc_eeprom_input_bits==8) {
                int data=scorpion_gmx_smuc_eeprom_input & 0xff;
                scorpion_gmx_smuc_eeprom_input=0;
                scorpion_gmx_smuc_eeprom_input_bits=0;
                if (scorpion_gmx_smuc_eeprom_state==SCORPION_SMUC_EEPROM_COMMAND) {
                    if ((data & 0xf0)!=0xa0) {
                        scorpion_gmx_smuc_eeprom_state=SCORPION_SMUC_EEPROM_IDLE;
                    }
                    else {
                        scorpion_gmx_smuc_eeprom_address=
                            (scorpion_gmx_smuc_eeprom_address & 0xff)|((data & 0x0e)<<7);
                        if (data & 1) {
                            scorpion_gmx_smuc_eeprom_state=SCORPION_SMUC_EEPROM_SEND;
                            scorpion_gmx_smuc_eeprom_output=
                                scorpion_gmx_smuc_eeprom[scorpion_gmx_smuc_eeprom_address];
                            scorpion_gmx_smuc_eeprom_address=(scorpion_gmx_smuc_eeprom_address+1)&0x7ff;
                            scorpion_gmx_smuc_eeprom_output_bits=0;
                        }
                        else scorpion_gmx_smuc_eeprom_state=SCORPION_SMUC_EEPROM_ADDRESS;
                    }
                }
                else if (scorpion_gmx_smuc_eeprom_state==SCORPION_SMUC_EEPROM_ADDRESS) {
                    scorpion_gmx_smuc_eeprom_address=
                        (scorpion_gmx_smuc_eeprom_address & 0x700)|data;
                    scorpion_gmx_smuc_eeprom_state=SCORPION_SMUC_EEPROM_DATA;
                }
                else if (scorpion_gmx_smuc_eeprom_state==SCORPION_SMUC_EEPROM_DATA) {
                    if (!(value & 0x20))
                        scorpion_gmx_smuc_eeprom[scorpion_gmx_smuc_eeprom_address]=data;
                    scorpion_gmx_smuc_eeprom_address=
                        (scorpion_gmx_smuc_eeprom_address & 0x7f0)|
                        ((scorpion_gmx_smuc_eeprom_address+1)&0x0f);
                }
                scorpion_gmx_smuc_eeprom_release=0;
                scorpion_gmx_smuc_eeprom_bus=0xbf;
                return;
            }
            if (scorpion_gmx_smuc_eeprom_state==SCORPION_SMUC_EEPROM_SEND) {
                if (scorpion_gmx_smuc_eeprom_output_bits==8) {
                    scorpion_gmx_smuc_eeprom_state=SCORPION_SMUC_EEPROM_ACK;
                    scorpion_gmx_smuc_eeprom_release=1;
                }
                else {
                    scorpion_gmx_smuc_eeprom_bus=
                        (scorpion_gmx_smuc_eeprom_output & 0x80) ? 0xff : 0xbf;
                    scorpion_gmx_smuc_eeprom_output<<=1;
                    scorpion_gmx_smuc_eeprom_output_bits++;
                    scorpion_gmx_smuc_eeprom_release=0;
                    return;
                }
            }
            else scorpion_gmx_smuc_eeprom_release=1;
        }
    }
    else if (new_clock && old_data!=new_data) {
        scorpion_gmx_smuc_eeprom_state=new_data ?
            SCORPION_SMUC_EEPROM_IDLE : SCORPION_SMUC_EEPROM_COMMAND;
        scorpion_gmx_smuc_eeprom_input=0;
        scorpion_gmx_smuc_eeprom_input_bits=0;
        scorpion_gmx_smuc_eeprom_release=1;
    }
    if (scorpion_gmx_smuc_eeprom_release)
        scorpion_gmx_smuc_eeprom_bus=new_data ? 0xff : 0xbf;
}

/* Convierte un valor del reloj a BCD si el registro B lo solicita.
 * Entrada: valor binario y registro B del RTC. Salida: valor para el Z80.
 */
static z80_byte scorpion_gmx_smuc_cmos_number(int number,z80_byte rtc_reg_b)
{
    if (rtc_reg_b & 4) return number;
    return (number / 10) * 16 + number % 10;
}

/* Lee el reloj del SMUC y sus registros de estado.
 * Entrada: ninguna. Salida: valor del registro RTC seleccionado.
 */
static z80_byte scorpion_gmx_smuc_cmos_read(void)
{
    time_t now=time(NULL);
    struct tm *clock=localtime(&now);
    z80_byte reg=scorpion_gmx_smuc_cmos_address & 0x7f;
    z80_byte rtc_reg_b=scorpion_gmx_smuc_cmos[11];

    if (scorpion_gmx_smuc_cmos_last_second &&
        now!=scorpion_gmx_smuc_cmos_last_second)
        scorpion_gmx_smuc_cmos_update_flag=1;
    scorpion_gmx_smuc_cmos_last_second=now;

    if (clock) {
        switch (reg) {
            case 0: return scorpion_gmx_smuc_cmos_number(clock->tm_sec,rtc_reg_b);
            case 2: return scorpion_gmx_smuc_cmos_number(clock->tm_min,rtc_reg_b);
            case 4: return scorpion_gmx_smuc_cmos_number(clock->tm_hour,rtc_reg_b);
            case 6: return clock->tm_wday+1;
            case 7: return scorpion_gmx_smuc_cmos_number(clock->tm_mday,rtc_reg_b);
            case 8: return scorpion_gmx_smuc_cmos_number(clock->tm_mon+1,rtc_reg_b);
            case 9: return scorpion_gmx_smuc_cmos_number(clock->tm_year%100,rtc_reg_b);
        }
    }
    if (reg==10) return 0x20 | (scorpion_gmx_smuc_cmos[10] & 0x0f);
    if (reg==11) return (rtc_reg_b & 4) | 2;
    if (reg==12) {
        z80_byte result=scorpion_gmx_smuc_cmos_update_flag ? 0x10 : 0;
        scorpion_gmx_smuc_cmos_update_flag=0;
        return result;
    }
    if (reg==13) return 0x80;
    return scorpion_gmx_smuc_cmos[reg];
}

/* Lee un puerto SMUC conectado al IDE de ZEsarUX.
 * Entrada: puerto Z80 y puntero al resultado. Salida: 1 si el puerto
 * pertenece al SMUC activo; 0 si debe seguir la lectura normal.
 */
int scorpion_gmx_smuc_read(z80_int port,z80_byte *value)
{
    if (!MACHINE_IS_SCORPION_GMX || !ide_enabled.v) return 0;

    if ((port & 0x18a3)!=(0xfffe & 0x18a3)) return 0;
    if ((port & 0xa044)==(0x5fba & 0xa044)) {
        *value=(port & 4) ? 0x57 : 0x3f;
        return 1;
    }
    if ((port & 0xa044)==(0x7fba & 0xa044)) {
        *value=(port & 4) ? 0x57 : (scorpion_gmx_smuc_config | 0x37);
        return 1;
    }
    if ((port & 0xa044)==(0xdfba & 0xa044)) {
        *value=scorpion_gmx_smuc_cmos_read();
        return 1;
    }
    if ((port & 0xa044)==(0xffba & 0xa044)) {
        *value=scorpion_gmx_smuc_eeprom_bus;
        return 1;
    }
    if ((port & 0x8044)==(0xffbe & 0x8044)) {
        if (scorpion_gmx_smuc_control & 0x80) {
            *value=(port & 0x100) ? 0xff : ide_read_command_block_register(7);
            return 1;
        }
        if (!(port & 0x2000)) {
            *value=scorpion_gmx_smuc_data_high;
            return 1;
        }
        int reg=(port>>8)&7;
        if (reg==0) {
            *value=ide_read_command_block_register(0);
            scorpion_gmx_smuc_data_high=ide_read_command_block_register(0);
            if (scorpion_gmx_smuc_identify_first_word) {
                // GMX espera un disco ATA fijo; el IDE genérico anuncia CF.
                *value=0x40;
                scorpion_gmx_smuc_data_high=0;
                scorpion_gmx_smuc_identify_first_word=0;
            }
        }
        else *value=ide_read_command_block_register(reg);
        return 1;
    }
    return 0;
}

/* Escribe un puerto SMUC conectado al IDE de ZEsarUX.
 * Entrada: puerto Z80 y valor escrito. Salida: 1 si el SMUC atendió
 * el puerto; 0 si la escritura debe continuar hacia otros dispositivos.
 */
int scorpion_gmx_smuc_write(z80_int port,z80_byte value)
{
    if (!MACHINE_IS_SCORPION_GMX || !ide_enabled.v) return 0;

    if ((port & 0x18a3)!=(0xfffe & 0x18a3)) return 0;
    if ((port & 0xa044)==(0xffba & 0xa044)) {
        if (value & 1) ide_reset();
        scorpion_gmx_smuc_eeprom_write(value);
        scorpion_gmx_smuc_control=value;
        return 1;
    }
    if ((port & 0xa044)==(0xdfba & 0xa044)) {
        if (scorpion_gmx_smuc_control & 0x80)
            scorpion_gmx_smuc_cmos[scorpion_gmx_smuc_cmos_address & 0x7f]=value;
        else scorpion_gmx_smuc_cmos_address=value;
        return 1;
    }
    if ((port & 0xa044)==(0x7fba & 0xa044)) {
        scorpion_gmx_smuc_config=value;
        return 1;
    }
    if ((port & 0x8044)==(0xffbe & 0x8044)) {
        if (scorpion_gmx_smuc_control & 0x80) return 1;
        if (!(port & 0x2000)) {
            scorpion_gmx_smuc_data_high=value;
            return 1;
        }
        int reg=(port>>8)&7;
        if (reg==0) {
            ide_write_command_block_register(0,value);
            ide_write_command_block_register(0,scorpion_gmx_smuc_data_high);
        }
        else {
            if (reg==7) scorpion_gmx_smuc_identify_first_word=(value==0xec);
            ide_write_command_block_register(reg,value);
        }
        return 1;
    }
    return 0;
}

void scorpion_malloc_mem_machine(void)
{
    //64 KB ROM, 256 KB RAM en scorpion 256
    //512 KB ROM, 2048 KB RAM en scorpion gmx
    //asignamos el maximo

    int total_ram=(SCORPIONGMX_ROM_SIZE+SCORPIONGMX_RAM_SIZE)*1024;

    //malloc_machine((SCORPION_ROM_SIZE+SCORPION_RAM_SIZE)*1024);
    //random_ram(memoria_spectrum+SCORPION_ROM_SIZE*1024,SCORPION_RAM_SIZE*1024);

    malloc_machine(total_ram);
    //random_ram(memoria_spectrum+SCORPION_ROM_SIZE*1024,SCORPION_RAM_SIZE*1024);

    scorpion_init_memory_tables();
    scorpion_set_normal_pages();
}

void scorpion_mem_page_ram(void)
{

    z80_byte page_entra=puerto_32765 & 7;

//| `7FFDh` | D0–D2 | Seleccionan una página de 0 a 7. |
//| `1FFDh` | D4 | Añade 8 al número de página cuando vale 1. |

    if (puerto_8189 & 16) page_entra +=8;

    if (MACHINE_IS_SCORPION_GMX) {
        page_entra |= (scorpion_gmx_port_dffd & 7) << 4;
    }

    scorpion_memory_paged[3]=scorpion_ram_mem_table[page_entra];
}

void scorpion_mem_page_rom(void)
{

    z80_byte page_entra;

    /*

| Página | Desplazamiento en la ROM | Contenido | Selección |
|---|---:|---|---|
| 0 | `0000h` | BASIC 128 | `1FFDh.D1=0`, `7FFDh.D4=0` |
| 1 | `4000h` | BASIC 48 | `1FFDh.D1=0`, `7FFDh.D4=1` |
| 2 | `8000h` | Monitor de servicio | `1FFDh.D1=1` |
| 3 | `C000h` | TR-DOS | Se activa mediante la lógica de TR-DOS, no mediante la selección BASIC de `7FFDh`. |
    */
    if ((puerto_8189 & 2)==0) {
        page_entra=(puerto_32765 & 16) >> 4;
    }

    else {
        page_entra=2;
    }

    if (MACHINE_IS_SCORPION_GMX) {
        page_entra |= (scorpion_gmx_port_7efd & 0x70) >> 2;
    }

    if (scorpion_trdos_active.v || (MACHINE_IS_SCORPION_GMX && (puerto_8189 & 4)))
        scorpion_memory_paged[0]=scorpion_rom_mem_table[(page_entra & ~3) | 3];
    else if (puerto_8189 & 1) scorpion_memory_paged[0]=scorpion_ram_mem_table[0];
    else scorpion_memory_paged[0]=scorpion_rom_mem_table[page_entra];
}

void scorpion_trdos_update(z80_int dir)
{
    //TR-DOS se activa al ejecutar en 3D00h-3DFFh desde BASIC 48.
    //Se desactiva al ejecutar fuera de la zona de ROM.
    if (scorpion_trdos_active.v) {
        if (dir>=0x4000) {
            scorpion_trdos_active.v=0;
            scorpion_mem_page_rom();
        }
    }
    else if (dir>=0x3D00 && dir<=0x3DFF &&
             (puerto_8189 & 3)==0 && (puerto_32765 & 16)) {
        scorpion_trdos_active.v=1;
        scorpion_mem_page_rom();
    }
}

int scorpion_trdos_is_active(void)
{
    return scorpion_trdos_active.v || (MACHINE_IS_SCORPION_GMX && (puerto_8189 & 4));
}

/* Indica si el joystick Kempston puede responder a un puerto en GMX.
 * Entrada: dirección del puerto. Salida: 1 si el joystick puede responder;
 * 0 si TR-DOS está activo o la ROM de servicio consulta el puerto 1Fh.
 */
int scorpion_gmx_kempston_port_enabled(z80_int port)
{
    int page;

    if (scorpion_trdos_is_active()) return 0;
    if ((port & 0xff)!=0x1f) return 1;

    for (page=0;page<SCORPIONGMX_TOTAL_ROM_PAGES;page++) {
        if (scorpion_memory_paged[0]==scorpion_rom_mem_table[page]) {
            return (page & 3)!=2;
        }
    }

    return 1;
}

void scorpion_nmi(void)
{
    // MAGIC selecciona el monitor antes de leer la instrucción en 0066h.
    scorpion_trdos_active.v=0;
    puerto_8189=(puerto_8189 & ~1) | 2;
    scorpion_mem_page_rom();
}

void scorpion_write_port_1ffd(z80_byte value)
{
    puerto_8189=value;

    //asignar ram
    scorpion_mem_page_ram();

    //asignar rom
    scorpion_mem_page_rom();
}

/*
 * Actualiza los registros de paginado y configuración de GMX.
 * Entrada: port es la dirección del puerto; value es el byte escrito.
 * Salida: ninguna. Puede cambiar el mapa de memoria o reiniciar la CPU.
 */
void scorpion_gmx_write_port(z80_int port,z80_byte value)
{
    if ((port & 0xff)==0) {
        scorpion_gmx_port_00=value;
        if (value & 8) {
            // La reconfiguración conserva este puerto y reinicia el procesador.
            scorpion_gmx_magic_shift=0x88 | (value & 7);
            if (!(value & 0x10)) reset_cpu();
        }
        return;
    }

    switch (port & 0xff00) {
        case 0xdf00:
            scorpion_gmx_port_dffd=value;
            scorpion_mem_page_ram();
            break;
        case 0x7800:
            scorpion_gmx_port_78fd=value;
            scorpion_memory_paged[2]=scorpion_ram_mem_table[(value ^ 2) & 0x7f];
            break;
        case 0x7a00:
            scorpion_gmx_scroll_low=value & 0xf0;
            break;
        case 0x7c00:
            scorpion_gmx_scroll_high=value & 0x3f;
            break;
        case 0x7e00:
        {
            z80_byte video_anterior=scorpion_gmx_port_7efd & 8;
            scorpion_gmx_port_7efd=(scorpion_gmx_port_00 & 0x10) ?
                ((value & 0x8f) | (scorpion_gmx_port_7efd & 0x70)) : value;
            if (video_anterior != (scorpion_gmx_port_7efd & 8)) {
                printf("Scorpion GMX: video %s (7EFDh=%02Xh, PC=%04Xh)\n",
                       (scorpion_gmx_port_7efd & 8) ? "640x200" : "256x192",
                       scorpion_gmx_port_7efd,reg_pc);
                fflush(stdout);
            }
            scorpion_mem_page_rom();
            scorpion_set_cpu_speed((value & 0x80) ? 2 : 1);
            break;
        }
    }
}

/*
 * Lee los registros de estado de paginado de GMX.
 * Entrada: port es la dirección del puerto solicitado.
 * Salida: byte de estado; 78FD desplaza un bit de la configuración latente.
 */
z80_byte scorpion_gmx_read_port(z80_int port)
{
    switch (port & 0xff00) {
        case 0x7a00:
            return (puerto_32765 & 7) | ((puerto_8189 & 0x10) >> 1) |
                   ((scorpion_gmx_port_dffd & 7) << 4);
        case 0x7800:
        {
            z80_byte result=(scorpion_gmx_port_78fd & 0x7f) |
                            (scorpion_gmx_magic_shift & 1);
            scorpion_gmx_magic_shift >>= 1;
            return result;
        }
        case 0x7e00:
            return ((puerto_32765 & 0x20) >> 5) |
                   ((puerto_32765 & 8) >> 2) |
                   ((scorpion_gmx_port_7efd & 0x80) >> 5) |
                   (scorpion_gmx_port_7efd & 8) |
                   ((scorpion_gmx_port_00 & 0x80) >> 3) |
                   (scorpion_gmx_port_00 & 0x20) |
                   ((puerto_8189 & 1) << 6);
    }
    return 0xff;
}



void scorpion_set_cpu_speed(int turbo)
{

    cpu_turbo_speed=turbo;
    cpu_set_turbo_speed();
}

/*
 * Simula seleccionar la velocidad normal con el control físico de GMX.
 * Entrada: ninguna.
 * Salida: ninguna; deja la CPU a x1 y actualiza el estado de 7EFD.D7.
 */
void scorpion_gmx_set_normal_speed(void)
{
    if (!MACHINE_IS_SCORPION_GMX) return;

    scorpion_gmx_port_7efd &= ~0x80;
    scorpion_set_cpu_speed(1);
}

/*
 * Simula seleccionar la velocidad turbo con el control físico de GMX.
 * Entrada: ninguna.
 * Salida: ninguna; deja la CPU a x2 y actualiza el estado de 7EFD.D7.
 */
void scorpion_gmx_set_turbo_speed(void)
{
    if (!MACHINE_IS_SCORPION_GMX) return;

    scorpion_gmx_port_7efd |= 0x80;
    scorpion_set_cpu_speed(2);
}

z80_byte fetch_opcode_scorpion(void)
{

    scorpion_trdos_update(reg_pc);
    if (scorpion_trdos_is_active()) betadisk_handle_trdos_traps();

#ifdef EMULATE_VISUALMEM
	set_visualmemopcodebuffer(reg_pc);
#endif
	return peek_byte_no_time (reg_pc);
}

void poke_byte_no_time_scorpion(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

    if (dir>16383 || (puerto_8189 & 1)) {
		poke_byte_spectrum_sync_rainbow(dir);
#ifdef EMULATE_VISUALMEM

        set_visualmembuffer(dir);

#endif
        dir = dir & 16383;
        puntero=scorpion_memory_paged[segmento]+dir;

        *puntero=valor;
    }
}


void poke_byte_scorpion(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

#ifdef EMULATE_CONTEND
    if (contend_pages_actual[segmento]) {
        //printf ("t_estados: %d\n",t_estados);
        t_estados += contend_table[ t_estados ];
    }
#endif
    t_estados += 3;



    if (dir>16383 || (puerto_8189 & 1)) {
		poke_byte_spectrum_sync_rainbow(dir);
#ifdef EMULATE_VISUALMEM

        set_visualmembuffer(dir);

#endif
		dir = dir & 16383;
		puntero=scorpion_memory_paged[segmento]+dir;

		//printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
		*puntero=valor;
	}
}




z80_byte peek_byte_no_time_scorpion(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

	dir = dir & 16383;
	puntero=scorpion_memory_paged[segmento]+dir;

	return *puntero;
}


z80_byte peek_byte_scorpion(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

#ifdef EMULATE_CONTEND
    if (contend_pages_actual[segmento]) {
            t_estados += contend_table[ t_estados ];
    }
#endif
    t_estados += 3;



	dir = dir & 16383;
	puntero=scorpion_memory_paged[segmento]+dir;

	return *puntero;
}



// Dibuja colour en (x,y), con escalas scale_x/scale_y y offsets offset_x/offset_y; sin retorno.
static void scorpiongmx_putpixel_scaled(int x,int y,z80_int colour,
                                    int scale_x,int scale_y,
                                    int offset_x,int offset_y)
{
    int dx,dy;
    int output_x=offset_x+x*scale_x;
    int output_y=offset_y+y*scale_y;

    for (dy=0;dy<scale_y;dy++)
            for (dx=0;dx<scale_x;dx++)
                    scr_putpixel_zoom(output_x+dx,output_y+dy,colour);
}

//Refresco pantalla sin rainbow
// Dibuja la pantalla ZX estándar sin rainbow; sin parámetros ni retorno.
void scorpiongmx_refresca_pantalla_no_rainbow_standard_48k(void)
{
    int x,y,bit;
    z80_int direccion,dir_atributo;
    z80_byte byte_leido;
    int color=0;
    int fila;
    //int zx,zy;

    z80_byte attribute,ink,paper,bright,flash,aux;



    z80_byte *screen=get_base_mem_pantalla();

    //printf ("dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
    z80_byte x_hi;

    for (y=0;y<192;y++) {

        direccion=screen_addr_table[(y<<5)];


        fila=y/8;
        dir_atributo=6144+(fila*32);
        for (x=0,x_hi=0;x<32;x++,x_hi +=8) {



            byte_leido=screen[direccion];
            attribute=screen[dir_atributo];



            if (scr_refresca_sin_colores.v) {
                attribute=56;


            }

            else {
                if (scr_refresca_show_attribute_grid.v) {
                    int cuad=(fila+x)%2;
                    if (cuad) attribute=56+64;
                    else attribute=56;
                }
            }


            ink=attribute &7;
            paper=(attribute>>3) &7;
            bright=(attribute) &64;
            flash=(attribute)&128;
            if (flash) {
                //intercambiar si conviene
                if (estado_parpadeo.v) {
                        aux=paper;
                        paper=ink;
                        ink=aux;
                }
            }

            if (bright) {
                ink +=8;
                paper +=8;
            }

            for (bit=0;bit<8;bit++) {

                color= ( byte_leido & 128 ? ink : paper );


                scorpiongmx_putpixel_scaled(x_hi+bit,y,
                                         color,
                                         2,2,64,8);

                byte_leido=byte_leido<<1;
            }



            direccion++;
            dir_atributo++;
        }

    }

}




/* Dibuja el modo GMX de 640x200 desde sus páginas físicas de píxeles y
 * atributos. Entrada: ninguna. Salida: ninguna; actualiza la imagen visible.
 */
static void scorpiongmx_refresca_pantalla_no_rainbow_640x200(void)
{
    int x,y,bit;
    unsigned int scroll=((unsigned int)scorpion_gmx_scroll_high << 8) |
                        scorpion_gmx_scroll_low;
    int pagina=(puerto_32765 & 8) ? 0x3b : 0x39;
    z80_byte *pixels=scorpion_ram_mem_table[pagina];
    z80_byte *attributes=scorpion_ram_mem_table[pagina+0x40];

    for (y=0;y<200;y++) {
        unsigned int direccion=(scroll+y*80)%16000;

        for (x=0;x<80;x++) {
            z80_byte pixel=pixels[direccion];
            z80_byte attribute=attributes[direccion];
            z80_byte ink,paper;

            if (scr_refresca_sin_colores.v) attribute=56;
            else if (scr_refresca_show_attribute_grid.v)
                attribute=((y/8+x)&1) ? 120 : 56;

            ink=(attribute & 7) | ((attribute & 0x40) >> 3);
            paper=((attribute >> 3) & 7) | ((attribute & 0x40) >> 3);

            if ((attribute & 0x80) && estado_parpadeo.v) {
                z80_byte aux=ink;
                ink=paper;
                paper=aux;
            }

            for (bit=0;bit<8;bit++) {
                scorpiongmx_putpixel_scaled(x*8+bit,y,
                                            (pixel & 0x80) ? ink : paper,
                                            1,2,0,0);
                pixel <<= 1;
            }

            direccion++;
            if (direccion==16000) direccion=0;
        }
    }
}


int if_scorpiongmx_mode_640_200(void)
{
    if (scorpion_gmx_port_7efd & 8) return 1;
    else return 0;
}

void scorpiongmx_refresca_pantalla_no_rainbow(void)
{
    if (scorpion_gmx_port_7efd & 8)
        scorpiongmx_refresca_pantalla_no_rainbow_640x200();
    else
        scorpiongmx_refresca_pantalla_no_rainbow_standard_48k();
}

// Dibuja colour en (x,y) usando el zoom global; sin retorno.
static void scorpiongmx_putpixel_absolute(int x,int y,z80_int colour)
{
    int dx,dy;
    int px=x*zoom_x;
    int py=y*zoom_y;

    for (dy=0;dy<zoom_y;dy++)
        for (dx=0;dx<zoom_x;dx++)
            scr_putpixel(px+dx,py+dy,colour);
}

// Escribe en offset_x/offset_y y width/height la geometría del modo actual; sin retorno.
static void scorpiongmx_get_mode_geometry(int *offset_x,int *offset_y,
                                       int *width,int *height)
{
    if (scorpion_gmx_port_7efd & 8) {
        *width=SCORPIONGMX_DISPLAY_WIDTH;
        *height=SCORPIONGMX_DISPLAY_HEIGHT;
        *offset_x=0;
        *offset_y=0;
        return;
    }

    // La pantalla Spectrum 256x192 se dibuja a escala 2 dentro de 640x400.
    *width=512;
    *height=384;
    *offset_x=(SCORPIONGMX_DISPLAY_WIDTH-*width)/2;
    *offset_y=(SCORPIONGMX_DISPLAY_HEIGHT-*height)/2;
}

// Dibuja el borde del modo GMX actual; sin parámetros ni retorno.
static void screen_scorpiongmx_refresca_border(void)
{
    int x,y,offset_x,offset_y,width,height;
    int left,top,right,bottom;
    int total_width,total_height;
    z80_int colour=out_254 & 7;

    scorpiongmx_get_mode_geometry(&offset_x,&offset_y,&width,&height);
    total_width=SCORPIONGMX_DISPLAY_WIDTH+
            (SCORPIONGMX_LEFT_BORDER_NO_ZOOM+SCORPIONGMX_RIGHT_BORDER_NO_ZOOM)*border_enabled.v;
    total_height=SCORPIONGMX_DISPLAY_HEIGHT+
            (SCORPIONGMX_TOP_BORDER_NO_ZOOM+SCORPIONGMX_BOTTOM_BORDER_NO_ZOOM)*border_enabled.v;

    left=SCORPIONGMX_LEFT_BORDER_NO_ZOOM*border_enabled.v+offset_x;
    top=SCORPIONGMX_TOP_BORDER_NO_ZOOM*border_enabled.v+offset_y;
    right=left+width;
    bottom=top+height;

    for (y=0;y<top;y++)
        for (x=0;x<total_width;x++)
                scorpiongmx_putpixel_absolute(x,y,colour);

    for (y=bottom;y<total_height;y++)
        for (x=0;x<total_width;x++)
                scorpiongmx_putpixel_absolute(x,y,colour);

    for (y=top;y<bottom;y++) {
        for (x=0;x<left;x++)
                scorpiongmx_putpixel_absolute(x,y,colour);
        for (x=right;x<total_width;x++)
                scorpiongmx_putpixel_absolute(x,y,colour);
        }
}

// Refresca pantalla y borde usando rainbow o dibujo completo; sin parámetros ni retorno.
void scorpiongmx_refresca_pantalla(void)
{
    //De momento no refrescar por rainbow

    //if (rainbow_enabled.v) {
    //    scr_refresca_pantalla_rainbow_comun();
    //}
    //else {
        scorpiongmx_refresca_pantalla_no_rainbow();
        screen_scorpiongmx_refresca_border();
        modificado_border.v=0;
    //}
}
