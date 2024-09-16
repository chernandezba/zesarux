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

Sobre la emulacion del microdrive:

Es una emulación muy mejorable:

- No se emula el movimiento real del microdrive: si no se lee o se escribe, el microdrive no se mueve
Solo se avanza el puntero de lectura/escritura cuando se hace una operación de lectura o escritura
- Los bits de lectura/escritura se están ignorando: por ejemplo se puede escribir aunque no se haya metido la controladora en modo escritura
- Se detecta cuando se lanza un comando format: esto es porque se comporta ligeramente diferente al comando de escritura.
Esto no haria falta seguramente si se emulasen los tiempos y el movimiento real del microdrive
- Se deberían guardar los bytes de gap y de preamble: dado que el formato .mdr no lo soporta, los ignoramos, pero no es lo deseable


TODO:

-con un mdr en blanco de 137923 bytes (bytes a 0), si formateamos se obtene 127 kb libres segun el cat. Cosa correcta
Si vuelvo a formatear, obtiene 126 kb libres
Si salgo de ZEsarUX y vuelvo a entrar y selecciono ese mdr ya formateado y reformateo, se vuelven a obtener 126 kb libres

Si salgo de ZEsarUX y vuelvo a escribir todos los bytes a 0, y formateo, se obtienen 127 kb libres
Quizá esto es un fallo de emulacion o del propio interface1. En Fuse por ejemplo, un microdrive en blanco al formatearlo siempre da
126 kb libres, no 127, por lo que quizá Fuse también tiene un error o quizá el error está en la propia rom del interface1

*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>


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


z80_byte *if1_microdrive_buffer;


int mdr_total_sectors=0;




z80_bit microdrive_enabled={0};

z80_bit microdrive_write_protect={0};

z80_bit microdrive_persistent_writes={0};

//int microdrive_must_flush_to_disk=0;

//int puntero_mdr=0;


//prueba

//int mdr_motores[8];

struct s_microdrive_status microdrive_status[MAX_MICRODRIVES];

//-1 si no aplica
int microdrive_get_visualmem_position(unsigned int address)
{
#ifdef EMULATE_VISUALMEM


    //El buffer de visualmem en este caso tiene mismo tamaño que dispositivo microdrive
    int posicion_final=address;

    //por si acaso
    if (posicion_final>=0 && posicion_final<VISUALMEM_MICRODRIVE_BUFFER_SIZE) {
        return posicion_final;
    }


#endif

	return -1;
}

void microdrive_set_visualmem_read(unsigned int address)
{
#ifdef EMULATE_VISUALMEM
	int posicion_final=microdrive_get_visualmem_position(address);
	if (posicion_final>=0) {
		set_visualmemmicrodrive_read_buffer(posicion_final);
	}

#endif
}

void microdrive_set_visualmem_write(unsigned int address)
{
#ifdef EMULATE_VISUALMEM
	int posicion_final=microdrive_get_visualmem_position(address);
	if (posicion_final>=0) {
		set_visualmemmicrodrive_write_buffer(posicion_final);
	}

#endif
}

int mdr_current_sector=0;
int mdr_current_offset_in_sector=0;


//Indica que estamos en la zona de preamble antes de escribir (10 ceros, 2 ff)
int mdr_write_preamble_index=0;

void mdr_next_sector(void)
{
    mdr_current_offset_in_sector=0;
    mdr_current_sector++;
    if (mdr_current_sector>=mdr_total_sectors) mdr_current_sector=0;

    mdr_write_preamble_index=0;

    printf("siguiente sector. actual=%d\n",mdr_current_sector);
}


z80_byte mdr_next_byte(void)
{

    if (microdrive_enabled.v==0) return 0;

    if (mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
        //Si estamos al final del sector, devolver 0
        //Esto no es lo real en el hardware, pero nos sirve
        //Realmente pasaremos al siguiente sector cuando se lea del puerto ef y hayamos pasado el tiempo final
        return 0;
    }


    int offset_to_sector=mdr_current_sector*MDR_BYTES_PER_SECTOR;

    int offset_efectivo;


    offset_efectivo=mdr_current_offset_in_sector;


    offset_efectivo +=offset_to_sector;

    z80_byte valor=if1_microdrive_buffer[offset_efectivo];

    microdrive_set_visualmem_read(offset_efectivo);

    printf("Retornando byte mdr de offset en PC=%04XH sector %d, offset %d (offset_efectivo=%d), mdr_write_preamble_index=%d =0x%02X\n",
        reg_pc,mdr_current_sector,mdr_current_offset_in_sector,offset_efectivo,mdr_write_preamble_index,valor);



    mdr_current_offset_in_sector++;

    mdr_write_preamble_index++;


    /*
    if (mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
        mdr_next_sector();
    }
    */



    return valor;

}

//simular sectores erroneos
//Entrada: sector: sector fisico
int microdrive_sector_es_erroneo(int sector)
{
    //if (sector<100) return 1;

    return 0;

}

//Retornar que motor empezando por el primero esta activo
//Retorna 0...7 si hay alguno
//-1 si no
int microdrive_primer_motor_activo(void)
{
    //Mostrar que motores activos
    int i;
    for (i=0;i<7;i++) {
        if (microdrive_status[i].motor_on) return i;
    }

    return -1;
}

//int mdr_write_beyond_15bytes=0;

//Contador simple para saber si tenemos que devolver gap, sync o datos
int contador_estado_microdrive=0;

//int escrito_byte_info_una_vez=0;

int microdrive_formateando=0;

void mdr_write_byte(z80_byte valor)
{
    if (microdrive_enabled.v==0) return;

    int microdrive_activo=microdrive_primer_motor_activo();
    //Si no hay ninguno activo, asumimos el primero
    if (microdrive_activo<0) microdrive_activo=0;

    microdrive_status[microdrive_activo].microdrive_must_flush_to_disk=1;

    /*if (mdr_write_beyond_15bytes) {
        //printf("Do not write as we are beyond 15 bytes header\n");
        //return;
    }*/

    /*
    Microdrive cartridge
    GAP      PREAMBLE      15 byte      GAP      PREAMBLE      15 byte    512     1
    [-----][00 00 ... ff ff][BLOCK HEAD][-----][00 00 ... ff ff][REC HEAD][ DATA ][CHK]
    Preamble = 10 * 0x00 + 2 * 0xff (12 byte)
    */

    //Preamble:
    //0-11 preamble
    //12-26 header
    //27-29 gap
    //30-41 preamble
    //42-569 datos

    //Esto es un poco chapuza pero funciona
    //La zona de preamble son 10 bytes a 0 y 2 bytes a FF
    if (
        (mdr_write_preamble_index>=0 && mdr_write_preamble_index<=11) ||
        (mdr_write_preamble_index>=27 && mdr_write_preamble_index<=41)
    ) {
        printf("Do not write as we are on the preamble or gap zone (mdr_write_preamble_index=%d)\n",mdr_write_preamble_index);
        mdr_write_preamble_index++;
        return;
    }

    mdr_write_preamble_index++;


    if (mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
        printf("Do not write as we are at the end of sector\n");
        //Si estamos al final del sector, no permitir escribir
        return;
    }


    int offset_to_sector=mdr_current_sector*MDR_BYTES_PER_SECTOR;

    int offset_efectivo;


    offset_efectivo=mdr_current_offset_in_sector;


    offset_efectivo +=offset_to_sector;

    //prueba formateo
    //TODO: detectar formateo


    int escribir=1;

    //Si estamos formateando, solo permitir la primera vez escribir bytes mas alla de la zona de cabecera
    //Si no hicieramos eso, el microdrive resultante tendria 0kb libreas
    //TODO: probablemente esto no pasaria si se emulasen los tiempos y el movimiento real del microdrive
    if (mdr_current_offset_in_sector>=15 && microdrive_formateando) {

        //temp
        valor=0;
        printf("no escribir byte info en formateo\n");

        /*

        if (escrito_byte_info_una_vez) {
            printf("no escribir byte info en formateo\n");

            //En vez de eso, escribir siempre 0
            valor=0;
        }

        else {
            escrito_byte_info_una_vez=1;
        }

        */

    }

    //Simular sectores erroneos
    if (microdrive_sector_es_erroneo(mdr_current_sector)) escribir=0;

    if (escribir) {
        if1_microdrive_buffer[offset_efectivo]=valor;

        microdrive_set_visualmem_write(offset_efectivo);

    }

    printf("Escribiendo byte mdr de offset en PC=%04XH sector %d, offset %d (offset_efectivo=%d) mdr_write_preamble_index=%d : 0x%02X (%c)\n",
        reg_pc,mdr_current_sector,mdr_current_offset_in_sector,offset_efectivo,mdr_write_preamble_index,
        valor,(valor>=32 && valor<=126 ? valor : '.'));



    mdr_current_offset_in_sector++;

    //Si estamos en offset 15, bloqueamos escrituras hasta que haya un cambio de read a write
    /*if (mdr_current_offset_in_sector==15) {
        mdr_write_beyond_15bytes=1;
    }*/

    /*if (mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
        printf("Going beyond sector on write. next sector\n");
        contador_estado_microdrive=0;
        mdr_next_sector();
    }*/


}




void microdrive_insert(int microdrive_seleccionado)
{
    //Cargar microdrive de prueba
    if1_microdrive_buffer=util_malloc(MDR_MAX_FILE_SIZE,"No enough memory for Microdrive buffer");


    FILE *ptr_microdrive_file;
    //Leer archivo mdr
    ptr_microdrive_file=fopen(microdrive_status[microdrive_seleccionado].microdrive_file_name,"rb");

    if (ptr_microdrive_file==NULL) {
        debug_printf (VERBOSE_ERR,"Cannot locate %s",microdrive_status[microdrive_seleccionado].microdrive_file_name);
    }

    else {
        //Leer todo el archivo microdrive de prueba
        int leidos=fread(if1_microdrive_buffer,1,MDR_MAX_FILE_SIZE,ptr_microdrive_file);
        printf ("leidos %d bytes de microdrive\n",leidos);

        mdr_total_sectors=leidos/MDR_BYTES_PER_SECTOR;

        fclose(ptr_microdrive_file);

        microdrive_enabled.v=1;

        //leer byte de write protect

        microdrive_write_protect.v=0;

        if (if1_microdrive_buffer[leidos-1]) microdrive_write_protect.v=1;
    }



}

void microdrive_eject(int microdrive_seleccionado)
{
	if (microdrive_enabled.v==0) return;

	//Hacer flush si hay algun cambio
	//microdrive_flush_contents_to_disk();


	free(if1_microdrive_buffer);


	microdrive_enabled.v=0;
}


void microdrive_footer_operating(void)
{

    generic_footertext_print_operating("MDR");

    //Y poner icono en inverso
    if (!zxdesktop_icon_mdv1_inverse) {
        zxdesktop_icon_mdv1_inverse=1;
        menu_draw_ext_desktop();
    }
}




z80_byte microdrive_status_ef(void)
{
    //printf ("In Port %x asked, PC after=0x%x\n",puerto_l+256*puerto_h,reg_pc);

    /*
    Microdrive cartridge
    GAP      PREAMBLE      15 byte      GAP      PREAMBLE      15 byte    512     1
    [-----][00 00 ... ff ff][BLOCK HEAD][-----][00 00 ... ff ff][REC HEAD][ DATA ][CHK]
    Preamble = 10 * 0x00 + 2 * 0xff (12 byte)
    */


    contador_estado_microdrive++;



    z80_byte return_value=0;

    //numero arbitrario realmente, cada cuanto incrementamos el contador para pasar de un estado al otro
    //La rom del interface1 por ejemplo cuando está leyendo datos (puerto e7) no está leyendo el puerto de estado (ef)
    //por tanto ese incremento del estado de datos (valor 0) a paso a estado gap lo producimos cuando ha pasado el contadort
    //aunque en dispositivo real esto sucederia justo al dejar de enviar los 543 bytes
    //Logicamente esto no va a la velocidad real ni cuento t-estados ni nada, por ejemplo si lees del puerto
    //de datos, te llegará el siguiente byte, y su vuelves a leer, aunque no haya pasado el tiempo "real" del microdrive
    //para que llegue el siguiente byte, te llegará
    #define MICRODRIVE_PASOS_CAMBIO_ESTADO 20

    if      (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO)   return_value=MICRODRIVE_STATUS_BIT_GAP; //gap
    else if (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*2) return_value=MICRODRIVE_STATUS_BIT_SYNC; //sync
    else if (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*3) return_value=0; //datos

    else if (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*4) return_value=MICRODRIVE_STATUS_BIT_GAP; //gap
    else if (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*5) return_value=MICRODRIVE_STATUS_BIT_SYNC; //sync
    else if (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*6) return_value=0; //datos

    if (contador_estado_microdrive>=MICRODRIVE_PASOS_CAMBIO_ESTADO*6) {
        mdr_next_sector();
        contador_estado_microdrive=0;
    }

    printf ("In Port ef asked, PC after=0x%x contador_estado_microdrive=%d return_value=0x%x\n",reg_pc,contador_estado_microdrive,return_value);


    if (microdrive_write_protect.v==0) return_value |=MICRODRIVE_STATUS_BIT_NOT_WRITE_PROTECT;

    interface1_last_read_status_ef=return_value;

    return return_value;

}



void microdrive_flush_to_disk_one(int microdrive_seleccionado)
{

	if (microdrive_enabled.v==0) return;

    if (microdrive_status[microdrive_seleccionado].microdrive_must_flush_to_disk==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush microdrive to disk but no changes made");
        return;
    }

	if (microdrive_persistent_writes.v==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush microdrive to disk but persistent writes disabled");
        return;
    }


    debug_printf (VERBOSE_INFO,"Flushing microdrive to disk");

    printf ("Flushing microdrive %d to disk\n",microdrive_seleccionado);



    FILE *ptr_microdrivefile;

    debug_printf (VERBOSE_INFO,"Opening microdrive File %s",microdrive_status[microdrive_seleccionado].microdrive_file_name);
    ptr_microdrivefile=fopen(microdrive_status[microdrive_seleccionado].microdrive_file_name,"wb");

    int escritos=0;

    int size=mdr_total_sectors*MDR_BYTES_PER_SECTOR;



    if (ptr_microdrivefile!=NULL) {



        z80_byte *puntero;
        puntero=if1_microdrive_buffer;

        //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
        //metera flush a 1
        microdrive_status[microdrive_seleccionado].microdrive_must_flush_to_disk=0;

        escritos=fwrite(puntero,1,size,ptr_microdrivefile);

        //Agregar el byte final que indica proteccion escritura o no
        z80_byte proteccion=microdrive_write_protect.v;

        fwrite(&proteccion,1,1,ptr_microdrivefile);

        fclose(ptr_microdrivefile);


    }

    //printf ("ptr_microdrivefile: %d\n",ptr_microdrivefile);
    //printf ("escritos: %lld\n",escritos);

    if (escritos!=size || ptr_microdrivefile==NULL) {
        debug_printf (VERBOSE_ERR,"Error writing to microdrive file");
        //microdrive_persistent_writes.v=0;
    }

}

void microdrive_flush_to_disk(void)
{
    int i;

    for (i=0;i<MAX_MICRODRIVES;i++) {
        microdrive_flush_to_disk_one(i);
    }

}



void microdrive_write_port_ef(z80_byte value)
{

    z80_byte antes_interface1_last_value_port_ef=interface1_last_value_port_ef;


    interface1_last_value_port_ef=value;
    printf("Write to port EF value %02XH\n",value);
    microdrive_footer_operating();


    //Si alterar motores
    if( !( value & 0x02 ) && ( antes_interface1_last_value_port_ef & 0x02 ) ) {      /* ~~\__ */
        int i;

        for( i = 7; i > 0; i-- ) {
        /* Rotate one drive */
        microdrive_status[i].motor_on = microdrive_status[i - 1].motor_on;
        }
        microdrive_status[0].motor_on = (value & 0x01) ? 0 : 1;

    }

    //Mostrar que motores activos
    int motor_activo=microdrive_primer_motor_activo();
    if (motor_activo>=0) printf("Motor activo %d\n",motor_activo+1);

    //if1_ula.comms_clk = ( val & 0x02 ) ? 1 : 0;


    //Si pasamos de lectura a escritura, inicializar contadores
    if (antes_interface1_last_value_port_ef &4) {
        if ((interface1_last_value_port_ef&4)==0) {
            printf("pasamos a write. PC=%04XH\n",reg_pc);

            //Preamble:
            //0-11 preamble
            //12-26 header
            //27-29 gap
            //30-41 preamble
            //42-569 datos

            //Saltar a seccion de preamble si conviene (esto cuando se ha acabado de escribir cabecera)
            if (mdr_write_preamble_index<30 && mdr_write_preamble_index>0) {
                mdr_write_preamble_index=30;
                printf("Situar mdr_write_preamble_index en %d\n",mdr_write_preamble_index);
            }


            if (mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
                printf("next sector\n");
                contador_estado_microdrive=0;
                mdr_next_sector();
            }

            //liberar al siguiente sector. Esto es para format
            /*if (mdr_write_beyond_15bytes) {
                printf("Allowing write again to next sector header 15 bytes probably from FORMAT command\n");
                mdr_write_beyond_15bytes=0;

                contador_estado_microdrive=0;
                mdr_next_sector();

                //mdr_write_preamble_index=0;
            }*/

        }
    }

}

//Inicializar algunas cosas al principio del todo
void init_microdrives(void)
{
    int i;

    for (i=0;i<MAX_MICRODRIVES;i++) {
        microdrive_status[i].microdrive_file_name[0]=0;
        microdrive_status[i].microdrive_must_flush_to_disk=0;
    }
}