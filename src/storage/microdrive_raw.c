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

/*

RAW Microdrive emulation, that can hold any filesystem (not just the interface 1 rom: the one supported for .mdr files).
This can be used formatting from standard interface 1 rom but also from Multiface 128 or from CP/M for LEC memory:
-Multiface 128 reduce gaps and other residual data, having a compatible filesystem with interface 1 rom but get more usable space
-CP/M for LEC memory uses a completely different filesystem format

For standard .MDR emulation, see file "microdrive.c"

Info about how it works:

-Reading or writing to data port E7 blocks the cpu (enables WAIT signal) until the next byte is read/written
-Every 168 t-states the microdrive reads a byte, advances one position and releases WAIT signal
-Reading or writing to status port EF doesn't block the cpu

-The microdrive can write two types of signals:
*regular data: 0 and 1. They are written enabling the erase+write signals
*silence: called also gap. It's written enabling the erase signal only

-About reading GAP or SIGNAL bits from port EF:
*gap signal is enabled when reading gaps from microdrive. Bit 2 or port EF is set when detected gap, or reset when no gap
*sync signal. This is the info from Charles Ingley, who reverse engineered the interface 1 ula and
sells the "vLA1 - Sinclair ZX Interface 1 ULA Replacement":

"
The SYNC signal indicates when a sequence of 8 ones (0xFF) is received (after a minimum number of zeroes)
on either Microdrive channel after a GAP has ended and signals the start of valid data. It stays valid until
the absence of data is detected (i.e. a GAP is detected). There is no requirement for 10 zeroes to be present
before the SYNC byte, this is just how emulator images have standardised. For the hardware to decode properly
at least 3 valid zeroes need to precede the SYNC byte. Due to tape dropouts all 10 leading zeroes after a GAP may not be present.

Sync is bit 1 of port EF and is 0 when sync is detected


An emulator works with an image that contains  fully decoded bytes and luckily has no need for the header
apart from synthesizing the GAP and SYNC signals for IO emulation. The format on a real tape is two streams
of serial bits interleaved across the two channels. The hardware first needs to synchronise to the incoming
data and then determine where the byte boundaries occur. The gaps in the tape format are always followed by a
unique bit sequence (24 or more zero bits followed by 8 one bits) on each channel.
The zeroes determine the bit boundaries for decoding and the 8 ones determine the byte boundary. 

When I mention ones and zeroes I'm talking about bit value (bytes are just an abstraction in hardware).
"

When he says "a unique bit sequence (24 or more zero bits followed by 8 one bits) on each channel. " it means for the two tracks,
so the sync sequence is formed by 00 00 00 00 00 00 FF FF. This byte sequence triggers the sync signal.

HOWEVER, if I emulate this behaviour, it doesn't format well from the interface 1 rom (I mean a normal FORMAT command from basic).
The total available sectors are less than expected.
I assume something in my timmings are not accurate, or some other error I haven't found

So my current implementation of the sync signal is:
-Reading from status port EF blocks the cpu (enables WAIT signal) until the next byte is read/written, like reading/writing E7 port

I have both algorithms available on my code, with the variable:
microdrive_raw_new_sync_algorithm

If it's set to 0, it uses my algorithm that works (reading from status port EF blocks the cpu). This is the default value.
It it's set to 1, it uses the "real" behaviour from the ula, but it doesn't work well: after format from basic,
the total available sectors are less than expected. Do NOT set it unless you know what you are doing
I have kept the two algorithms so maybe in the future I can fix it and use the real algorithm



More info:

https://worldofspectrum.org/faq/reference/formats.htm

https://ia601504.us.archive.org/23/items/spectrum_microdrive_book/spectrum_microdrive_book.pdf

https://sinclair.wiki.zxnet.co.uk/wiki/ZX_Interface_1

https://k1.spdns.de/Vintage/Sinclair/82/Peripherals/Interface%201%20and%20Microdrives%20(Sinclair)/Microdrive-IF1%20Complete%20Rom%20Disassembly.pdf

http://rk.nvg.ntnu.no/sinclair/faq/fileform.html#MDR

https://k1.spdns.de/Vintage/Sinclair/82/Peripherals/Interface%201%20and%20Microdrives%20(Sinclair)/ZX%20Interface%20I%20and%20Interface%20II%20Service%20Manual%20%232.pdf

https://microhobby.speccy.cz/sinclair.htm


*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#include "microdrive_raw.h"
#include "microdrive.h"
#include "if1.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "zxvision.h"
#include "compileoptions.h"
#include "menu_items.h"
#include "screen.h"
#include "audio.h"

//int microdrive_is_raw=1;

int microdrive_raw_pending_read_port=0;

int microdrive_raw_pending_status_port=0;

//Ultimo dato leido por el cabezal
z80_int microdrive_raw_last_read_data;

//Ultimo dato enviado para escribir
z80_byte microdrive_raw_last_byte_to_write;


/*
-definir memoria de 16 bits para el contenido del microdrive:

Byte 0: Dato escrito. Si era un gap, el dato no es relevante (no se habrá escrito nada aqui)

Byte 1:

Bit 0:
-a 0 si el dato se ha escrito solo con señal de Erase. Lo que se interpretará como un gap
-a 1 si el dato se ha escrito con señales Erase+Write
Bit 1: a 1 si es un byte defectuoso (o sea en mal estado y que provocara errores). Por determinar como gestionar esto
Bit 2-7: sin uso de momento
*/


//Simular problemas como creacion de sectores erroneos y alargamiento de la cinta
z80_bit microdrive_raw_real_life_problems={0};


z80_int microdrive_raw_return_value_at_pos(int microdrive_activo,int visualmem_update)
{
    z80_int *puntero=microdrive_status[microdrive_activo].raw_microdrive_buffer;

    int microdrive_raw_current_position=microdrive_status[microdrive_activo].raw_current_position;
    if (visualmem_update) microdrive_set_visualmem_read(microdrive_raw_current_position);
    return puntero[microdrive_raw_current_position];
}

void microdrive_raw_write_value_at_pos(int microdrive_activo,z80_int valor,int visualmem_update)
{
    z80_int *puntero=microdrive_status[microdrive_activo].raw_microdrive_buffer;
    int microdrive_raw_current_position=microdrive_status[microdrive_activo].raw_current_position;
    if (visualmem_update) microdrive_set_visualmem_write(microdrive_raw_current_position);
    puntero[microdrive_raw_current_position]=valor;
    //printf("Posicion %d tiene %03XH\n",microdrive_raw_current_position,microdrive_raw_image[microdrive_raw_current_position]);
}

/*
En los primeros 50 usos, la cinta se puede alargar un 2%, random con 10% de probabilidad
Para cada 100 usos, se puede generar una posición errónea (5% de probabilidad)
*/
void microdrive_raw_simulate_real_problems(int microdrive_activo)
{

    if (microdrive_raw_real_life_problems.v==0) return;

    unsigned int usage_counter=microdrive_status[microdrive_activo].raw_usage_counter;

    if (usage_counter<50) {
        int valor_random=util_get_random();
        //printf("valor_random %d\n",valor_random);
        if ((valor_random % 100)<10) {
            DBG_PRINT_MDR VERBOSE_INFO,"MDR: Enlarge microdrive 2%%");
            microdrive_raw_enlarge(microdrive_activo,50);
        }
    }

    if ((usage_counter % 100)==0) {
        int valor_random=util_get_random();
        //printf("valor_random %d\n",valor_random);
        if ((valor_random % 100)<5) {
            DBG_PRINT_MDR VERBOSE_INFO,"MDR: Generate bad position");

            //Por cubrir mas tamaño, pues random estara maximo 65535 aprox
            int posicion=valor_random*4;

            //Y ajustar al tamaño final
            int pos_final=(posicion % microdrive_status[microdrive_activo].raw_total_size);

            microdrive_raw_mark_bad_position(microdrive_activo,pos_final);

        }
    }

}

void microdrive_raw_increment_usage_counter(int microdrive_activo)
{
    microdrive_status[microdrive_activo].raw_usage_counter++;
    microdrive_raw_simulate_real_problems(microdrive_activo);
}

void microdrive_raw_advance_position(int microdrive_activo)
{
    int microdrive_raw_current_position=microdrive_status[microdrive_activo].raw_current_position;

    microdrive_raw_current_position++;
    if (microdrive_raw_current_position>=microdrive_status[microdrive_activo].raw_total_size) {
        microdrive_raw_current_position=0;
        microdrive_raw_increment_usage_counter(microdrive_activo);
    }

    microdrive_status[microdrive_activo].raw_current_position=microdrive_raw_current_position;
}

//Para simular sonido de carga o grabación
int microdrive_mix_audio_output_bit=0;

z80_bit microdrive_simulate_sound_read={0};
z80_bit microdrive_simulate_sound_write={0};

//Cada byte se escribe cada 168 t-estados, por tanto un bit cada 21 t-estados
//ZEsarUX envia un sample de sonido cada 224 t-estados (15600 hz)
//Realmente tendriamos que tener un sampleado de sonido mucho mayor a 15600 hz para escuchar sonido mas real
//pero de momento esto es la mejor aproximacion que podemos tener
//La frecuencia de sempleado ideal seria : 1 bit cada 21 t-estados, pero son dos pulsos entonces 1 sample cada 10.5 t-estados,
// en un frame de video serian 69888/10.5 =  6656 samples
// en un segundo son 6656*50=332800 hz=332 khz
//esa seria la frecuencia de sampleado minima para poder escuchar bien el sonido del microdrive,
//aunque el oido humano no capta eso ni de lejos)
void microdrive_mix_audio(void)
{
    if (!microdrive_current_is_raw()) return;
    if (microdrive_simulate_sound_read.v==0 && microdrive_simulate_sound_write.v==0) return;


    int sonido_microdrive=microdrive_mix_audio_output_bit*255;
    sonido_microdrive -=128;
    sonido_microdrive /=2;


    reset_silence_detection_counter();
    audio_valor_enviar_sonido_izquierdo /=2;
    audio_valor_enviar_sonido_izquierdo += sonido_microdrive;

    audio_valor_enviar_sonido_derecho /=2;
    audio_valor_enviar_sonido_derecho += sonido_microdrive;


}


void microdrive_mix_audio_generate_next_sample(void)
{

    z80_byte byte_a_samplear;

    //Escribir o leer
    if ((interface1_last_value_port_ef & 0x08)==0) {
        //Escribir
        if (microdrive_simulate_sound_write.v==0) return;

        byte_a_samplear=microdrive_raw_last_byte_to_write;
    }

    else {
        //leer
        if (microdrive_simulate_sound_read.v==0) return;
        byte_a_samplear=microdrive_raw_last_read_data;
    }

    //Microdrive escribe con codificacion Manchester:
    //Si hay bit a 0, en un pulso se invierte el valor una vez
    //Si hay bit a 1, en un pulso se invierte el valor dos veces, por tanto a final de pulso nos quedamos igual

    int i;
    for (i=0;i<8;i++) {
        if ((byte_a_samplear&128)==0) microdrive_mix_audio_output_bit ^=1;

        byte_a_samplear=byte_a_samplear<<1;

    }

}


//Si ultima secuencia leida era sync. Cuando la secuencia esta entera
int microdrive_raw_last_read_data_is_sync=0;


//Si esta a cero, el wait era por puerto e7
//Si esta a uno, el wait era por puerto ef
//Si esta a dos, el wait era por out a puerto de datos
int estado_wait_por_puerto_tipo=0;

z80_byte raw_anterior_leido;

//0 el actual, 1 el anterior, 2 el anterior al 2, etc
//saltara sync si 0 es ff, 1 2 y 3 son 0
z80_byte raw_nuevo_sync_lista[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};


//Si es 0, puerto EF es bloqueante.
//Si es 1, es el comportamiento teorico de la ULA de la interface1. Puerto EF no es bloqueante.
//Aunque formatea a menos capacidad con la rom del interface1
int microdrive_raw_new_sync_algorithm=0;

void microdrive_raw_move(void)
{
    //Primero liberamos la señal wait de la cpu, si es que estaba
    if (z80_wait_signal.v) {
        //printf ("Liberar wait signal en t_estados: %d\n",t_estados);

        if (estado_wait_por_puerto_tipo==1) microdrive_raw_pending_status_port=1;
        if (estado_wait_por_puerto_tipo==0) microdrive_raw_pending_read_port=1;
    }

    /*if (temp_pending_dump) {
        microdrive_raw_dump_values_dump();
    }*/

    z80_wait_signal.v=0;

    //printf ("Avanzar posicion microdrive. t_estados: %d\n",t_estados);

    //Escribir o leer
    if ((interface1_last_value_port_ef & 0x08)==0) {
        //Erase

        z80_int value_to_write=microdrive_raw_last_byte_to_write;

        //Tiene señal de write o solo erase? (si solo es erase, sera un gap)
        if ((interface1_last_value_port_ef & 0x04)==0) {
            //tiene erase+write. Es un dato normal
            //printf("Writing value %04XH at pos %d\n",value_to_write,microdrive_raw_current_position);
            value_to_write |=MICRODRIVE_RAW_INFO_BYTE_MASK_DATA;
            //sleep(1);
        }

        else {
            //Solo Erase, por tanto es un gap
            //printf("Writing GAP %04XH at pos %d\n",value_to_write,microdrive_raw_current_position);
            //sleep(1);
            //El byte como dato sera 0
            //Y gap a 0 tambien
            value_to_write &=0x0000;
        }


   //GAP WRITE if ((interface1_last_value_port_ef & 0x08)==0 && (interface1_last_value_port_ef & 0x04)) {

        int microdrive_activo=microdrive_primer_motor_activo();


        //Si no hay ninguno activo, nada
        if (microdrive_activo>=0 && microdrive_status[microdrive_activo].microdrive_enabled) {
            //printf("Escribiendo %03XH\n",value_to_write);
            //printf("Cabezal en escritura. posicion %d\n",microdrive_status[microdrive_activo].raw_current_position);

            //Preservar del byte de info, bits 1-7.
            z80_int valor_leido=microdrive_raw_return_value_at_pos(microdrive_activo,0);
            valor_leido &=0xFE00;

            //y el valor a escribir preservar asi como el bit de gap
            value_to_write &=0x01FF;

            //Si posicion esta marcado como erroneo, alterar byte
            if (valor_leido & MICRODRIVE_RAW_INFO_BYTE_MASK_BAD_POSITION) {
                //printf("Alterar byte\n");

                //inversion de algunos bits
                value_to_write ^= 0x00EA;
            }

            value_to_write |=valor_leido;

            microdrive_raw_write_value_at_pos(microdrive_activo,value_to_write,1);
            microdrive_status[microdrive_activo].microdrive_must_flush_to_disk=1;
        }



    }


    else if (interface1_last_value_port_ef & 0x04) {
        //LEER

        //printf("Cabezal en lectura. posicion %d\n",microdrive_raw_current_position);
                int microdrive_activo=microdrive_primer_motor_activo();


        //Si no hay ninguno activo, nada
        if (microdrive_activo>=0) {
            //printf("Cabezal en lectura. posicion %d\n",microdrive_status[microdrive_activo].raw_current_position);
            microdrive_raw_last_read_data=microdrive_raw_return_value_at_pos(microdrive_activo,1);
        }



            //Nuevo sync
            //rotar
            //0 el actual, 1 el anterior, 2 el anterior al 2, etc
            raw_nuevo_sync_lista[7]=raw_nuevo_sync_lista[6];
            raw_nuevo_sync_lista[6]=raw_nuevo_sync_lista[5];
            raw_nuevo_sync_lista[5]=raw_nuevo_sync_lista[4];
            raw_nuevo_sync_lista[4]=raw_nuevo_sync_lista[3];
            raw_nuevo_sync_lista[3]=raw_nuevo_sync_lista[2];
            raw_nuevo_sync_lista[2]=raw_nuevo_sync_lista[1];
            raw_nuevo_sync_lista[1]=raw_nuevo_sync_lista[0];
            raw_nuevo_sync_lista[0]=microdrive_raw_last_read_data & 0xFF;




        if ((microdrive_raw_last_read_data & 0x0100)==0) {
            //printf ("En microdrive_raw_move leemos byte gap en posicion %d\n",microdrive_raw_current_position);
            //Si es gap, sera un 0 al final
            microdrive_raw_last_read_data=0x0000;

            //nuevo sync

                microdrive_raw_last_read_data_is_sync=0; //sync reseteado.
                //printf("reset sync en %d\n",microdrive_status[0].raw_current_position);



            //sleep(1);
        }



        else {
            //detectar sync


                if (raw_nuevo_sync_lista[0]==0xFF &&
                    raw_nuevo_sync_lista[1]==0xFF &&
                    raw_nuevo_sync_lista[2]==0x00 &&
                    raw_nuevo_sync_lista[3]==0x00 &&
                    raw_nuevo_sync_lista[4]==0x00 &&
                    raw_nuevo_sync_lista[5]==0x00 &&
                    raw_nuevo_sync_lista[6]==0x00 &&
                    raw_nuevo_sync_lista[7]==0x00

                ) {
                        //printf("nuevo sync en %d\n",microdrive_status[0].raw_current_position);

                        microdrive_raw_last_read_data_is_sync=1; //sync enabled

                }


                //Fin nuevo sync



            raw_anterior_leido=microdrive_raw_last_read_data & 0xFF;

        }

        //Si posicion esta marcado como erroneo, alterar byte
        if (microdrive_raw_last_read_data & MICRODRIVE_RAW_INFO_BYTE_MASK_BAD_POSITION) {
            //invertimos bits arbitrarios
            microdrive_raw_last_read_data ^= 0xCD;
        }

    }



    //Y avanzamos cabezal
    int microdrive_activo=microdrive_primer_motor_activo();


    //Si no hay ninguno activo, nada
    if (microdrive_activo>=0) {
        microdrive_raw_advance_position(microdrive_activo);
    }


    microdrive_mix_audio_generate_next_sample();

}

void mdr_raw_write_byte(z80_byte value)
{

    microdrive_raw_last_byte_to_write=value;



    //Activar wait signal hasta que se avance al siguiente byte
    z80_wait_signal.v=1;

    estado_wait_por_puerto_tipo=2;

    //printf ("Activando wait debido a out puerto. t_estados: %d\n",t_estados);


}



z80_byte microdrive_raw_status_ef(void)
{
    //printf ("In Port EF asked, PC after=0x%x\n",reg_pc);
    //sleep(2);


    int motor_activo=microdrive_primer_motor_activo();


    //Este puerto tambien activa wait

    z80_byte return_value=0;



    if (motor_activo>=0) {

        //Tiene gap?
        if ((microdrive_raw_last_read_data & 0x0100)==0) {
            return_value=MICRODRIVE_STATUS_BIT_GAP;
            //printf("----Retornar GAP\n");
            //sleep(1);
        }

        //Ver si habia sync
        else {
            if (microdrive_raw_last_read_data_is_sync) {
                return_value &=(255-MICRODRIVE_STATUS_BIT_SYNC);
                //Si hay sync, bit a 0
                //Si no hay sync, bit a 1

                //printf("----Retornar SYNC\n");

                //sleep(1);
            }
            else {
                //No hay sync. bit a 1
                return_value |=MICRODRIVE_STATUS_BIT_SYNC;
            }
        }



        //return_value=MICRODRIVE_STATUS_BIT_GAP; //gap
        //return_value=MICRODRIVE_STATUS_BIT_SYNC; //sync


        //printf ("MDR: In Port ef asked, PC after=0x%x microdrive_raw_current_position=%d return_value=0x%x\n",
        //    reg_pc,microdrive_raw_current_position,return_value);

        if (microdrive_status[motor_activo].microdrive_write_protect==0) return_value |=MICRODRIVE_STATUS_BIT_NOT_WRITE_PROTECT;
    }

    //resetear señal sync solo desde aqui
    //microdrive_raw_last_read_data_is_sync=0;

    //En principio con esto le dice que si no esta el microdrive habilitado, dira error:
    //Microdrive not present
    //temp if (microdrive_status[motor_activo].microdrive_enabled==0) return_value=MICRODRIVE_STATUS_BIT_GAP | MICRODRIVE_STATUS_BIT_SYNC;

    if ((return_value & MICRODRIVE_STATUS_BIT_SYNC)==0) {
        //printf("No retornar SYNC\n");
        //sleep(1);
    }
    if ((return_value & MICRODRIVE_STATUS_BIT_GAP)==0) {
        //printf("No retornar GAP\n");
    }

    interface1_last_read_status_ef=return_value;

    return return_value;

}


z80_byte microdrive_raw_read_port_e7(void)
{

    //Si volvemos de un in con su wait
    if (microdrive_raw_pending_read_port) {
        microdrive_raw_pending_read_port=0;



        z80_byte value=microdrive_raw_last_read_data & 0xFF;

        //Si era un gap, retornar 0
        if ((microdrive_raw_last_read_data & 0x0100)==0) value=0;

        //interface1_last_read_e7=value;

        //printf("Retornar de lectura puerto E7 valor %02XH pc=%04XH\n",value,reg_pc);
        //sleep(1);
        return value;
    }

    else {
        //En caso contrario, habilitar wait
        //printf("Pasamos a wait al leer puerto E7. PC=%04XH\n",reg_pc);
        estado_wait_por_puerto_tipo=0;
        //sleep(2);
        z80_wait_signal.v=1;
        return 0;
    }



}


z80_byte microdrive_raw_read_port_ef(void)
{

    //Si nuevo algoritmo, puerto no es bloqueante
    if (microdrive_raw_new_sync_algorithm) {
        //printf("Puerto no bloqueante EF\n");
        z80_byte value=microdrive_raw_status_ef();

        //if (value & MICRODRIVE_STATUS_BIT_GAP) printf("Leyendo puerto EF hay GAP. t_estados: %d\n",t_estados);
        //if (value & MICRODRIVE_STATUS_BIT_SYNC) printf("Leyendo puerto EF hay SYNC. t_estados: %d\n",t_estados);

        return value;
    }

    //Si volvemos de un in con su wait
    if (microdrive_raw_pending_status_port) {
        microdrive_raw_pending_status_port=0;


        z80_byte value=microdrive_raw_status_ef();


        //printf("Retornar de lectura puerto EF valor %02XH pc=%04XH\n",value,reg_pc);

        if ((value & MICRODRIVE_STATUS_BIT_SYNC)) {
            //printf("Retornar SYNC a la lectura del puerto\n");
            //sleep(1);
        }

        //sleep(1);
        return value;
    }

    else {
        //En caso contrario, habilitar wait
        z80_wait_signal.v=1;
        //printf("Pasamos a wait al leer puerto EF. PC=%04XH\n",reg_pc);
        estado_wait_por_puerto_tipo=1;
        //sleep(2);
        return 0;
    }
}



void microdrive_raw_insert(int microdrive_seleccionado)
{

    DBG_PRINT_MDR VERBOSE_INFO,"MDR: Inserting RAW microdrive file %s on MDV%d",
        microdrive_status[microdrive_seleccionado].microdrive_file_name,
        microdrive_seleccionado+1
    );

    if (!si_existe_archivo(microdrive_status[microdrive_seleccionado].microdrive_file_name)) {
        DBG_PRINT_MDR VERBOSE_ERR,"MDR: Microdrive file %s does not exist",microdrive_status[microdrive_seleccionado].microdrive_file_name);
        return;
    }

    //Leer tamanyo
    int tamanyo_archivo=get_file_size(microdrive_status[microdrive_seleccionado].microdrive_file_name);

    //restar cabecera
    tamanyo_archivo -=MICRODRIVE_RAW_HEADER_SIZE;

    //Si es tamaño invalido o impar (porque son registros de 16 bits y por tanto par)
    if (tamanyo_archivo<=0 || (tamanyo_archivo &1) ) {
        DBG_PRINT_MDR VERBOSE_ERR,"MDR: Invalid size for RAW microdrive file");
        return;
    }

    microdrive_status[microdrive_seleccionado].raw_microdrive_buffer=util_malloc(tamanyo_archivo,"No enough memory for Microdrive buffer");

    //Leer cabecera y comparar si valida signature
    z80_byte header[MICRODRIVE_RAW_HEADER_SIZE];

    FILE *ptr_microdrive_file;
    //Leer archivo mdr
    ptr_microdrive_file=fopen(microdrive_status[microdrive_seleccionado].microdrive_file_name,"rb");

    if (ptr_microdrive_file==NULL) {
        DBG_PRINT_MDR VERBOSE_ERR,"MDR: Cannot locate file: %s",microdrive_status[microdrive_seleccionado].microdrive_file_name);
    }

    else {
        fread(header,1,MICRODRIVE_RAW_HEADER_SIZE,ptr_microdrive_file);

        char expected_signature[7];
        strcpy(expected_signature,MICRODRIVE_RAW_SIGNATURE);

        char read_signature[7];
        memcpy(read_signature,header,6);
        read_signature[6]=0;

        if (strcmp(read_signature,expected_signature)) {
            DBG_PRINT_MDR VERBOSE_ERR,"MDR: Invalid RAW microdrive signature file");
            return;
        }

  /*
  0       “RAWMDV” 6 bytes
6       -Version. byte. actual: 0
7       -Creator: 30 bytes. 0 del final opcional. Ejemplo “ZEsarUX 11.1-SN”
37      -Machine: para que máquina se está usando. 0 del final opcional. 20 bytes:
        “Spectrum”
        “QL”
57      19 bytes. 0 del final opcional
        -Fecha: 19 bytes. 0 del final opcional
        “DD/MM/AAAA HH:MM:SS”
  */

        //bloques de char con 1 byte mas para poder meter los ceros del final, porque el 0 es opcional en la cabecera
        //Mostrar los otros datos a nivel de info

        z80_byte header_version=header[6];
        char header_creator[31];
        memcpy(header_creator,&header[7],30);
        header_creator[30]=0;

        char header_machine[21];
        memcpy(header_machine,&header[37],20);
        header_machine[20]=0;

        char header_date[20];
        memcpy(header_date,&header[57],19);
        header_date[19]=0;

        microdrive_status[microdrive_seleccionado].raw_usage_counter=header[76]|(header[77]<<8)|(header[78]<<16)|(header[79]<<24);

        DBG_PRINT_MDR VERBOSE_INFO,"MDR: File version %d created on %s from machine %s on date %s usage counter %u",
            header_version,header_creator,header_machine,header_date,microdrive_status[microdrive_seleccionado].raw_usage_counter);


        int total_microdrive=tamanyo_archivo/2;

        microdrive_status[microdrive_seleccionado].raw_total_size=total_microdrive;

        int i;

        //Leer primero datos
        for (i=0;i<total_microdrive;i++) {

            z80_byte valor_leido1;
            //,valor_leido2;

            fread(&valor_leido1,1,1,ptr_microdrive_file);


            z80_int word_leido;

            word_leido=valor_leido1;


            microdrive_status[microdrive_seleccionado].raw_microdrive_buffer[i]=word_leido;
        }

        //Y luego info de cada dato (gap, etc)
        for (i=0;i<total_microdrive;i++) {

            z80_byte valor_leido1;
            //,valor_leido2;

            fread(&valor_leido1,1,1,ptr_microdrive_file);


            z80_int word_leido=microdrive_status[microdrive_seleccionado].raw_microdrive_buffer[i];

            //Modificar solo 8 bits altos
            word_leido &=0x00FF;

            word_leido |=(valor_leido1<<8);


            microdrive_status[microdrive_seleccionado].raw_microdrive_buffer[i]=word_leido;
        }


        //DBG_PRINT_MDR VERBOSE_INFO,"MDR: Read %d bytes from microdrive image",leidos);



        fclose(ptr_microdrive_file);



        microdrive_status[microdrive_seleccionado].microdrive_enabled=1;
        microdrive_status[microdrive_seleccionado].raw_format=1;
        microdrive_status[microdrive_seleccionado].raw_current_position=0;


    }

}

int microdrive_current_is_raw(void)
{

    int microdrive_activo=microdrive_primer_motor_activo();
    //Si no hay ninguno activo, nada
    if (microdrive_activo<0) return 0;

    return microdrive_status[microdrive_activo].raw_format;
}

void microdrive_raw_flush_to_disk_one(int microdrive_seleccionado)
{


    FILE *ptr_microdrivefile;

    DBG_PRINT_MDR VERBOSE_INFO,"MDR: Opening microdrive File %s",microdrive_status[microdrive_seleccionado].microdrive_file_name);
    ptr_microdrivefile=fopen(microdrive_status[microdrive_seleccionado].microdrive_file_name,"wb");

    //int escritos=0;


    if (ptr_microdrivefile!=NULL) {



        //Escribir cabecera
        z80_byte header[MICRODRIVE_RAW_HEADER_SIZE];
        microdrive_raw_create_header(header,microdrive_status[microdrive_seleccionado].raw_usage_counter);

        //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
        //metera flush a 1
        microdrive_status[microdrive_seleccionado].microdrive_must_flush_to_disk=0;

        //Cabecera
        fwrite(header,1,MICRODRIVE_RAW_HEADER_SIZE,ptr_microdrivefile);

        //escritos=fwrite(puntero,1,size,ptr_microdrivefile);

        //Escribir datos
        int i;
        for (i=0;i<microdrive_status[microdrive_seleccionado].raw_total_size;i++) {
            z80_byte byte1;

            z80_int word=microdrive_status[microdrive_seleccionado].raw_microdrive_buffer[i];


            byte1=word & 0xFF;


            fwrite(&byte1,1,1,ptr_microdrivefile);

        }

        //Escribir info de cada dato (gap, etc)
        for (i=0;i<microdrive_status[microdrive_seleccionado].raw_total_size;i++) {
            z80_byte byte2;

            z80_int word=microdrive_status[microdrive_seleccionado].raw_microdrive_buffer[i];



            byte2=(word>>8) & 0xFF;


            fwrite(&byte2,1,1,ptr_microdrivefile);
        }

        fclose(ptr_microdrivefile);


    }

    //printf ("ptr_microdrivefile: %d\n",ptr_microdrivefile);
    //printf ("escritos: %lld\n",escritos);

    if (ptr_microdrivefile==NULL) {
        DBG_PRINT_MDR VERBOSE_ERR,"MDR: Error writing to microdrive file");
        //microdrive_persistent_writes.v=0;
    }
}

//Escribe la cabecera de un archivo raw microdrive (.RMD).
//256 bytes
void microdrive_raw_create_header(z80_byte *destino,unsigned int usagecounter)
{
    //Inicializar a 0 todo
    int i;

    for (i=0;i<MICRODRIVE_RAW_HEADER_SIZE;i++) destino[i]=0;

/*
-Firma:
offset  Description
0       “RAWMDV” 6 bytes
6       -Version. byte. current: 0
7       -Creator: 30 bytes. String ended with 0 byte (0 final optional). Example “ZEsarUX 11.1-SN”
37      -Machine: What machine uses this microdrive. String ended with 0 byte  (0 final optional). 20 bytes: examples:
        “Spectrum”
        “QL”
57      19 bytes.
        -Date: 19 bytes. String ended with 0 byte (0 final optional)
        “DD/MM/AAAA HH:MM:SS”

76-79   Usage counter, times the microdrive has been full read (times the header position has passed position 0). Unsigned integer 32 bits, little endian
80-255  Reserved. Set to 0

After the header comes the data and then the type of all data. For example, for a 90 KB raw microdrive, here it comes:

90 KB of data: just the raw data
90 KB of type for all the data: every position of data can be: gap or data.
And also, every position can be marked as "bad position" (emulating a microdrive with sections of data that can't be written or read).
Every one of this data type is a mask of bits, currently only bits 0 and 1 are used:

Bit 0: Data or Gap. If set bit, it's a data. If not, it's a gap
Bit 1: Bad position. If set, it's a bad positiion. If not, it's a "good" position

For example if the first 5 bytes of data are: Gap, Gap, Data, Data, Data+Bad, the data types will be:

00, 00, 01, 01, 03

Following the same 90 KB example, the total size of the RMD file will be: 256+92160+92160=184576 bytes

*/
    strcpy((char *)destino,MICRODRIVE_RAW_SIGNATURE);

    destino[6]=0; //Version

    strcpy((char *)&destino[7],"ZEsarUX " EMULATOR_VERSION);

    if (MACHINE_IS_QL) strcpy((char *)&destino[37],"QL");
    else strcpy((char *)&destino[37],"Spectrum");

    char time_string[40];
    snapshot_get_date_time_string_human(time_string);
    strcpy((char *)&destino[57],time_string);

    destino[76]=usagecounter & 0xFF;
    destino[77]=(usagecounter>>8) & 0xFF;
    destino[78]=(usagecounter>>16) & 0xFF;
    destino[79]=(usagecounter>>24) & 0xFF;

}


void microdrive_raw_mark_bad_position(int microdrive_seleccionado,int position)
{
    z80_int *puntero=microdrive_status[microdrive_seleccionado].raw_microdrive_buffer;
    z80_int valor_leido=puntero[position];

    valor_leido |=MICRODRIVE_RAW_INFO_BYTE_MASK_BAD_POSITION;

    puntero[position]=valor_leido;

    microdrive_status[microdrive_seleccionado].microdrive_must_flush_to_disk=1;
}


void microdrive_raw_unmark_bad_position(int microdrive_seleccionado,int position)
{
    z80_int *puntero=microdrive_status[microdrive_seleccionado].raw_microdrive_buffer;
    z80_int valor_leido=puntero[position];

    valor_leido &=(0xFFFF - MICRODRIVE_RAW_INFO_BYTE_MASK_BAD_POSITION);

    puntero[position]=valor_leido;

    microdrive_status[microdrive_seleccionado].microdrive_must_flush_to_disk=1;
}

void microdrive_raw_full_erase(int microdrive_seleccionado)
{
    z80_int *puntero=microdrive_status[microdrive_seleccionado].raw_microdrive_buffer;

    int i;

    for (i=0;i<microdrive_status[microdrive_seleccionado].raw_total_size;i++) {
        //Conservar bit de defectuoso y el resto a 0
        z80_int valor_leido=puntero[i] & MICRODRIVE_RAW_INFO_BYTE_MASK_BAD_POSITION;
        puntero[i]=valor_leido;
    }

    microdrive_status[microdrive_seleccionado].microdrive_must_flush_to_disk=1;

}

//ampliacion dice: cada cuantos X posiciones se le agregara 1 mas
void microdrive_raw_enlarge(int microdrive_seleccionado,int ampliacion)
{

    if (ampliacion<=0) {
        DBG_PRINT_MDR VERBOSE_ERR,"MDR: Invalid stretch parameter");
        return;
    }

    int current_size=microdrive_status[microdrive_seleccionado].raw_total_size;


    //si ampliacion=1, es el doble de ampliacion, que sera el doble del original
    //Y el espacio necesario es de 16 bits
    z80_int *new_microdrive_buffer=util_malloc(current_size*2*2,"No enough memory for Microdrive buffer");


    int i;
    int destino=0;
    int sub_pos=0;

    for (i=0;i<current_size;i++) {
        //printf("%d\n",i);
        new_microdrive_buffer[destino++]=microdrive_status[microdrive_seleccionado].raw_microdrive_buffer[i];

        //Si ampliar
        sub_pos++;
        if (sub_pos==ampliacion) {
            new_microdrive_buffer[destino++]=0;
            sub_pos=0;
        }
    }



    microdrive_status[microdrive_seleccionado].raw_total_size=destino;

    //printf("before free\n");

    free(microdrive_status[microdrive_seleccionado].raw_microdrive_buffer);

    microdrive_status[microdrive_seleccionado].raw_microdrive_buffer=new_microdrive_buffer;

    microdrive_status[microdrive_seleccionado].microdrive_must_flush_to_disk=1;


}