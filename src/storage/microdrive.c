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
#include <string.h>


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



int microdrive_formateando=0;

void microdrive_flush_to_disk_one(int microdrive_seleccionado);


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


void mdr_next_sector(int microdrive_seleccionado)
{
    microdrive_status[microdrive_seleccionado].mdr_current_offset_in_sector=0;
    microdrive_status[microdrive_seleccionado].mdr_current_sector++;
    if (microdrive_status[microdrive_seleccionado].mdr_current_sector>=microdrive_status[microdrive_seleccionado].mdr_total_sectors) {
        microdrive_status[microdrive_seleccionado].mdr_current_sector=0;
    }

    microdrive_status[microdrive_seleccionado].mdr_write_preamble_index=0;

    printf("siguiente sector. actual=%d\n",microdrive_status[microdrive_seleccionado].mdr_current_sector);
}


z80_byte mdr_next_byte(void)
{
    int microdrive_activo=microdrive_primer_motor_activo();
    //Si no hay ninguno activo, nada
    if (microdrive_activo<0) return 0;


    if (microdrive_status[microdrive_activo].microdrive_enabled==0) return 0;

    if (microdrive_status[microdrive_activo].mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
        //Si estamos al final del sector, devolver 0
        //Esto no es lo real en el hardware, pero nos sirve
        //Realmente pasaremos al siguiente sector cuando se lea del puerto ef y hayamos pasado el tiempo final
        return 0;
    }


    int offset_to_sector=microdrive_status[microdrive_activo].mdr_current_sector*MDR_BYTES_PER_SECTOR;

    int offset_efectivo;


    offset_efectivo=microdrive_status[microdrive_activo].mdr_current_offset_in_sector;


    offset_efectivo +=offset_to_sector;



    z80_byte valor=microdrive_status[microdrive_activo].if1_microdrive_buffer[offset_efectivo];



    microdrive_set_visualmem_read(offset_efectivo);

    printf("Retornando byte mdr de offset en PC=%04XH sector %d, offset %d (offset_efectivo=%d), mdr_write_preamble_index=%d =0x%02X\n",
        reg_pc,microdrive_status[microdrive_activo].mdr_current_sector,
        microdrive_status[microdrive_activo].mdr_current_offset_in_sector,
        offset_efectivo,microdrive_status[microdrive_activo].mdr_write_preamble_index,valor);



    microdrive_status[microdrive_activo].mdr_current_offset_in_sector++;

    microdrive_status[microdrive_activo].mdr_write_preamble_index++;






    return valor;

}

//simular sectores erroneos
//Entrada: sector: sector fisico
int microdrive_sector_es_erroneo(int microdrive_activo,int sector,int offset_in_sector)
{
    //Nota: parece que la ROM determina que un sector es erroneo cuando el checksum (byte 14) es incorrecto
    //Alteraremos solo ese byte
    if (offset_in_sector==14) {
        return (microdrive_status[microdrive_activo].bad_sectors_simulated[sector]);
    }

    return 0;

}





void mdr_write_byte(z80_byte valor)
{

    int microdrive_activo=microdrive_primer_motor_activo();
    //Si no hay ninguno activo, nada
    if (microdrive_activo<0) return;

    if (microdrive_status[microdrive_activo].microdrive_enabled==0) return;


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
        (microdrive_status[microdrive_activo].mdr_write_preamble_index>=0 && microdrive_status[microdrive_activo].mdr_write_preamble_index<=11) ||
        (microdrive_status[microdrive_activo].mdr_write_preamble_index>=27 && microdrive_status[microdrive_activo].mdr_write_preamble_index<=41)
    ) {
        printf("Do not write as we are on the preamble or gap zone (mdr_write_preamble_index=%d)\n",microdrive_status[microdrive_activo].mdr_write_preamble_index);
        microdrive_status[microdrive_activo].mdr_write_preamble_index++;
        return;
    }

    microdrive_status[microdrive_activo].mdr_write_preamble_index++;


    if (microdrive_status[microdrive_activo].mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
        printf("Do not write as we are at the end of sector\n");
        //Si estamos al final del sector, no permitir escribir
        return;
    }


    int offset_to_sector=microdrive_status[microdrive_activo].mdr_current_sector*MDR_BYTES_PER_SECTOR;

    int offset_efectivo;


    offset_efectivo=microdrive_status[microdrive_activo].mdr_current_offset_in_sector;


    offset_efectivo +=offset_to_sector;

    //prueba formateo
    //TODO: detectar formateo


    int escribir=1;

    //Si estamos formateando, solo permitir la primera vez escribir bytes mas alla de la zona de cabecera
    //Si no hicieramos eso, el microdrive resultante tendria 0kb libreas
    //TODO: probablemente esto no pasaria si se emulasen los tiempos y el movimiento real del microdrive
    if (microdrive_status[microdrive_activo].mdr_current_offset_in_sector>=15 && microdrive_formateando) {

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
    if (microdrive_sector_es_erroneo(microdrive_activo,microdrive_status[microdrive_activo].mdr_current_sector,
    microdrive_status[microdrive_activo].mdr_current_offset_in_sector)) {
        //alterar valor porque es incorrecto
        valor ^=255;

    }

    if (escribir) {
        microdrive_status[microdrive_activo].if1_microdrive_buffer[offset_efectivo]=valor;

        microdrive_set_visualmem_write(offset_efectivo);

    }

    printf("Escribiendo byte mdr de offset en PC=%04XH sector %d, offset %d (offset_efectivo=%d) mdr_write_preamble_index=%d : 0x%02X (%c)\n",
        reg_pc,microdrive_status[microdrive_activo].mdr_current_sector,
        microdrive_status[microdrive_activo].mdr_current_offset_in_sector,offset_efectivo,
        microdrive_status[microdrive_activo].mdr_write_preamble_index,
        valor,(valor>=32 && valor<=126 ? valor : '.'));



    microdrive_status[microdrive_activo].mdr_current_offset_in_sector++;




}




void microdrive_insert(int microdrive_seleccionado)
{
    microdrive_status[microdrive_seleccionado].if1_microdrive_buffer=util_malloc(MDR_MAX_FILE_SIZE,"No enough memory for Microdrive buffer");


    FILE *ptr_microdrive_file;
    //Leer archivo mdr
    ptr_microdrive_file=fopen(microdrive_status[microdrive_seleccionado].microdrive_file_name,"rb");

    if (ptr_microdrive_file==NULL) {
        debug_printf (VERBOSE_ERR,"Cannot locate file: %s",microdrive_status[microdrive_seleccionado].microdrive_file_name);
    }

    else {
        //Leer todo el archivo microdrive de prueba
        int leidos=fread(microdrive_status[microdrive_seleccionado].if1_microdrive_buffer,1,MDR_MAX_FILE_SIZE,ptr_microdrive_file);
        printf ("leidos %d bytes de microdrive\n",leidos);

        microdrive_status[microdrive_seleccionado].mdr_total_sectors=leidos/MDR_BYTES_PER_SECTOR;

        fclose(ptr_microdrive_file);

        microdrive_status[microdrive_seleccionado].microdrive_enabled=1;

        //leer byte de write protect

        microdrive_status[microdrive_seleccionado].microdrive_write_protect=0;

        if (microdrive_status[microdrive_seleccionado].if1_microdrive_buffer[leidos-1]) {
            microdrive_status[microdrive_seleccionado].microdrive_write_protect=1;
        }
    }



}

void microdrive_eject(int microdrive_seleccionado)
{
	if (microdrive_status[microdrive_seleccionado].microdrive_enabled==0) return;

	//Hacer flush si hay algun cambio
	microdrive_flush_to_disk_one(microdrive_seleccionado);


	free(microdrive_status[microdrive_seleccionado].if1_microdrive_buffer);


	microdrive_status[microdrive_seleccionado].microdrive_enabled=0;
}


void microdrive_footer_operating(void)
{
    char buffer[20];

    int motor_activo=microdrive_primer_motor_activo();
    if (motor_activo>=0) {
        printf("MOTOR ACTIVO: %d\n",motor_activo);
        sprintf(buffer,"MDV%d",motor_activo+1);
    }

    else strcpy(buffer,"MDV");

    //printf("%s\n",buffer);

    generic_footertext_print_operating(buffer);

    //Y poner icono en inverso

    switch (motor_activo) {
        case 0:
            if (!zxdesktop_icon_mdv1_inverse) {
                zxdesktop_icon_mdv1_inverse=1;
                menu_draw_ext_desktop();
            }
        break;

        case 1:
            if (!zxdesktop_icon_mdv2_inverse) {
                zxdesktop_icon_mdv2_inverse=1;
                menu_draw_ext_desktop();
            }
        break;

        case 2:
            if (!zxdesktop_icon_mdv3_inverse) {
                zxdesktop_icon_mdv3_inverse=1;
                menu_draw_ext_desktop();
            }
        break;

        case 3:
            if (!zxdesktop_icon_mdv4_inverse) {
                zxdesktop_icon_mdv4_inverse=1;
                menu_draw_ext_desktop();
            }
        break;
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

    int motor_activo=microdrive_primer_motor_activo();

    if (motor_activo>=0) {
        microdrive_status[motor_activo].contador_estado_microdrive++;
    }



    z80_byte return_value=0;

    //numero arbitrario realmente, cada cuanto incrementamos el contador para pasar de un estado al otro
    //La rom del interface1 por ejemplo cuando está leyendo datos (puerto e7) no está leyendo el puerto de estado (ef)
    //por tanto ese incremento del estado de datos (valor 0) a paso a estado gap lo producimos cuando ha pasado el contadort
    //aunque en dispositivo real esto sucederia justo al dejar de enviar los 543 bytes
    //Logicamente esto no va a la velocidad real ni cuento t-estados ni nada, por ejemplo si lees del puerto
    //de datos, te llegará el siguiente byte, y su vuelves a leer, aunque no haya pasado el tiempo "real" del microdrive
    //para que llegue el siguiente byte, te llegará
    #define MICRODRIVE_PASOS_CAMBIO_ESTADO 20

    if (motor_activo>=0) {

        if      (microdrive_status[motor_activo].contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO)   return_value=MICRODRIVE_STATUS_BIT_GAP; //gap
        else if (microdrive_status[motor_activo].contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*2) return_value=MICRODRIVE_STATUS_BIT_SYNC; //sync
        else if (microdrive_status[motor_activo].contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*3) return_value=0; //datos

        else if (microdrive_status[motor_activo].contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*4) return_value=MICRODRIVE_STATUS_BIT_GAP; //gap
        else if (microdrive_status[motor_activo].contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*5) return_value=MICRODRIVE_STATUS_BIT_SYNC; //sync
        else if (microdrive_status[motor_activo].contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*6) return_value=0; //datos

        if (microdrive_status[motor_activo].contador_estado_microdrive>=MICRODRIVE_PASOS_CAMBIO_ESTADO*6) {
            mdr_next_sector(motor_activo);
            microdrive_status[motor_activo].contador_estado_microdrive=0;
        }

        printf ("In Port ef asked, PC after=0x%x contador_estado_microdrive=%d return_value=0x%x\n",
            reg_pc,microdrive_status[motor_activo].contador_estado_microdrive,return_value);

        if (microdrive_status[motor_activo].microdrive_write_protect==0) return_value |=MICRODRIVE_STATUS_BIT_NOT_WRITE_PROTECT;
    }






    interface1_last_read_status_ef=return_value;

    return return_value;

}



void microdrive_flush_to_disk_one(int microdrive_seleccionado)
{

	if (microdrive_status[microdrive_seleccionado].microdrive_enabled==0) return;

    if (microdrive_status[microdrive_seleccionado].microdrive_must_flush_to_disk==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush microdrive to disk but no changes made");
        return;
    }

	if (microdrive_status[microdrive_seleccionado].microdrive_persistent_writes==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush microdrive to disk but persistent writes disabled");
        return;
    }


    debug_printf (VERBOSE_INFO,"Flushing microdrive to disk");

    printf ("Flushing microdrive %d to disk\n",microdrive_seleccionado);



    FILE *ptr_microdrivefile;

    debug_printf (VERBOSE_INFO,"Opening microdrive File %s",microdrive_status[microdrive_seleccionado].microdrive_file_name);
    ptr_microdrivefile=fopen(microdrive_status[microdrive_seleccionado].microdrive_file_name,"wb");

    int escritos=0;

    int size=microdrive_status[microdrive_seleccionado].mdr_total_sectors*MDR_BYTES_PER_SECTOR;



    if (ptr_microdrivefile!=NULL) {



        z80_byte *puntero;
        puntero=microdrive_status[microdrive_seleccionado].if1_microdrive_buffer;

        //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
        //metera flush a 1
        microdrive_status[microdrive_seleccionado].microdrive_must_flush_to_disk=0;

        escritos=fwrite(puntero,1,size,ptr_microdrivefile);

        //Agregar el byte final que indica proteccion escritura o no
        z80_byte proteccion=microdrive_status[microdrive_seleccionado].microdrive_write_protect;

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
    if (motor_activo>=0) {
        printf("Motor activo %d\n",motor_activo+1);
        //microdrive_footer_operating();
    }

    int microdrive_seleccionado=motor_activo;

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

            if (microdrive_seleccionado>=0) {

                //Saltar a seccion de preamble si conviene (esto cuando se ha acabado de escribir cabecera)
                if (microdrive_status[microdrive_seleccionado].mdr_write_preamble_index<30 && microdrive_status[microdrive_seleccionado].mdr_write_preamble_index>0) {
                    microdrive_status[microdrive_seleccionado].mdr_write_preamble_index=30;
                    printf("Situar mdr_write_preamble_index en %d\n",microdrive_status[microdrive_seleccionado].mdr_write_preamble_index);
                }


                if (microdrive_status[microdrive_seleccionado].mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
                    printf("next sector\n");
                    microdrive_status[microdrive_seleccionado].contador_estado_microdrive=0;
                    mdr_next_sector(microdrive_seleccionado);
                }

            }



        }
    }

}

//Inicializar algunas cosas al principio del todo
void init_microdrives(void)
{
    int i;

    for (i=0;i<MAX_MICRODRIVES;i++) {
        microdrive_status[i].microdrive_enabled=0;
        microdrive_status[i].microdrive_file_name[0]=0;
        microdrive_status[i].microdrive_must_flush_to_disk=0;
        microdrive_status[i].mdr_total_sectors=0;
        microdrive_status[i].mdr_current_sector=0;
        microdrive_status[i].mdr_current_offset_in_sector=0;
        microdrive_status[i].contador_estado_microdrive=0;
        microdrive_status[i].mdr_write_preamble_index=0;
        microdrive_status[i].microdrive_persistent_writes=0;
        microdrive_status[i].microdrive_write_protect=0;


        int j;
        for (j=0;j<MDR_MAX_SECTORS;j++) {
            microdrive_status[i].bad_sectors_simulated[j]=0;
        }
    }
}



//Obtener info de un archivo de zona de memoria mdr
//Para poder soportar duplicados, indicamos un sector de inicio y de ahi en adelante (y puede dar la vuelta una vez)
void mdr_get_info_file(z80_byte *origen,int total_sectors,char *nombre,int tamanyo,struct s_mdr_file_cat_one_file *file,int *p_fragmentados,int *p_no_fragmentados,int sector_inicio)
{
    int i;

    int tamanyo_contando_cabecera=tamanyo+9;

    int total_sectores_a_buscar=tamanyo_contando_cabecera/512;

    //Ver si el ultimo sector esta cortado

    int resto=tamanyo_contando_cabecera % 512;

    if (resto) total_sectores_a_buscar++;

    printf("Sectores a buscar: %d\n",total_sectores_a_buscar);

    file->total_sectors=total_sectores_a_buscar;

    int bloque_buscando;

    //Informacion de fragmentacion
    //Si un sector no es consecutivo al otro, aumenta fragmentacion
    int frag_anterior_sector=-1;

    int frag_sectores_fragmentados=0;
    int frag_sectores_no_fragmentados=1; //Al menos el primero no esta fragmentado logicamente


    //Cada uno de los bloques a buscar
    for (bloque_buscando=0;bloque_buscando<total_sectores_a_buscar;bloque_buscando++) {

        //buscar cada sector cada vez en toda la imagen
        //empieza desde el sector de inicio indicado para poder mostrar correctamente localizacion
        //de archivos duplicados
        int vuelta=0;

        int encontrado=0;

        int sector_primero=sector_inicio;

        for (vuelta=0;vuelta<2 && !encontrado;vuelta++) {
            for (i=sector_primero;i<total_sectors && !encontrado;i++) {
                int offset_sector=i*MDR_BYTES_PER_SECTOR;

                //z80_byte data_recflg=origen[offset_sector+15];
                z80_byte record_segment=origen[offset_sector+16];

                if (record_segment==bloque_buscando /* && (data_recflg & 0x04)==0x04*/) {
                    char nombre_comparar[11];

                    int j;

                    for (j=0;j<10;j++) {
                        z80_byte letra_nombre=origen[offset_sector+19+j];

                        nombre_comparar[j]=letra_nombre;
                    }

                    nombre_comparar[j]=0;

                    if (!strcmp(nombre_comparar,nombre)) {
                        printf("Match nombre [%s] en sector %d\n",nombre,bloque_buscando);

                        //Grabar ese bloque
                        //Si es record 0, saltar 9 bytes de la cabecera de datos
                        int offset_a_grabar=30;
                        //int tamanyo_restar=512;

                        z80_int rec_length=origen[offset_sector+17]+256*origen[offset_sector+18];

                        if (record_segment==0) {
                            offset_a_grabar+=9;
                            rec_length-=9;
                        }

                        //memcpy(destino,&origen[offset_sector+offset_a_grabar],rec_length);
                        //destino +=rec_length;

                        if (bloque_buscando!=0) {
                            //Si sectores consecutivos, no incrementa fragmentación
                            if (i==frag_anterior_sector+1) frag_sectores_no_fragmentados++;
                            //O si anterior sector era el ultimo y este es el primero
                            else if (i==0 && frag_anterior_sector==total_sectors-1) frag_sectores_no_fragmentados++;
                            else frag_sectores_fragmentados++;
                        }

                        file->sectors_list[bloque_buscando]=i;

                        frag_anterior_sector=i;

                        encontrado=1;
                    }

                }
            }

            //la siguiente vuelta empieza desde cero
            sector_primero=0;
        }

    }

    //printf("Info fragmentacion: Fragmentados: %d No fragmentados: %d\n",frag_sectores_fragmentados,frag_sectores_no_fragmentados);

    *p_fragmentados=frag_sectores_fragmentados;
    *p_no_fragmentados=frag_sectores_no_fragmentados;
}


z80_byte mdr_get_file_catalogue_get_byte(z80_byte *puntero,int sector,int sector_offset)
{

    int offset=sector*MDR_BYTES_PER_SECTOR;

    offset +=sector_offset;

    return puntero[offset];
}


void mdr_get_file_from_catalogue(z80_byte *origen,struct s_mdr_file_cat_one_file *archivo,z80_byte *destino)
{

    int tamanyo=archivo->file_size;

    //Ocupa 0. no busquemos nada
    //if (tamanyo==0) return;

    int tamanyo_contando_cabecera=tamanyo+9;

    int total_sectores_a_buscar=tamanyo_contando_cabecera/512;

    //Ver si el ultimo sector esta cortado

    int resto=tamanyo_contando_cabecera % 512;

    if (resto) total_sectores_a_buscar++;

    printf("Sectores a buscar: %d\n",total_sectores_a_buscar);

    int bloque_buscando;



    //Cada uno de los bloques a buscar
    for (bloque_buscando=0;bloque_buscando<total_sectores_a_buscar;bloque_buscando++) {


        int i=archivo->sectors_list[bloque_buscando];


        int offset_sector=i*MDR_BYTES_PER_SECTOR;


        //Grabar ese bloque
        //Si es record 0, saltar 9 bytes de la cabecera de datos
        int offset_a_grabar=30;
        //int tamanyo_restar=512;

        int rec_length=origen[offset_sector+17]+256*origen[offset_sector+18];


        if (bloque_buscando==0) {
            offset_a_grabar+=9;
            rec_length-=9;
        }

        printf("Copiando %d bytes\n",rec_length);

        if (rec_length>0) {
            memcpy(destino,&origen[offset_sector+offset_a_grabar],rec_length);
        }

        destino +=rec_length;


    }


}


void mdr_get_file_catalogue_get_label(char *texto,z80_byte *origen,int sector)
{
    int i;

    for (i=0;i<10;i++) {
        z80_byte caracter=mdr_get_file_catalogue_get_byte(origen,sector,4+i);

        if (caracter<32 || caracter>126) caracter='.';

        texto[i]=caracter;

    }

    texto[i]=0;
}

int mdr_get_file_catalogue_get_valor_max_copias(struct s_mdr_file_cat *catalogo,char *nombre)
{
    int copias=1;

    int i;

    //Buscar valor maximo
    for (i=0;i<catalogo->total_files;i++) {
        if (!strcmp(nombre,catalogo->file[i].name)) {
            if (catalogo->file[i].numero_copias>copias) copias=catalogo->file[i].numero_copias;
        }
    }

    return copias;

}

void mdr_set_max_copias_todos_archivos(struct s_mdr_file_cat *catalogo)
{
    int i;

    //Establecer valor copias
    for (i=0;i<catalogo->total_files;i++) {
        int max_copias=mdr_get_file_catalogue_get_valor_max_copias(catalogo,catalogo->file[i].name);
        //solo alterarlo si es >1
        if (max_copias>1) {
            printf("Readjusting file [%s] to %d copies\n",catalogo->file[i].name,max_copias);
            catalogo->file[i].numero_copias=max_copias;
        }
    }
}

//Dice el numero de copias de un archivo
//Y si tiene mas de una copia, retorna el id que se genera del catalogo
int mdr_get_file_catalogue_get_copias(struct s_mdr_file_cat *catalogo,char *nombre,int *p_id_file)
{
    int copias=1;

    int id_file=-1;

    int i;

    for (i=0;i<catalogo->total_files;i++) {
        if (!strcmp(nombre,catalogo->file[i].name)) {
            copias++;
            id_file=catalogo->file[i].id_file;
        }

    }

    *p_id_file=id_file;

    return copias;

}

//devuelve el listado de archivos de un mdr
//usado por multiples funciones para facilitar el acceso a los archivos
struct s_mdr_file_cat *mdr_get_file_catalogue(z80_byte *origen,int total_sectors)
{

    //Asignar memoria
    struct s_mdr_file_cat *catalogo=util_malloc(sizeof(struct s_mdr_file_cat),"Can not allocate memory for catalogue");

    int i;

    catalogo->total_files=0;
    catalogo->used_sectors=0;


    int frag_sectores_fragmentados=0;
    int frag_sectores_no_fragmentados=0;

    int escrito_microdrive_label=0;

    //Sacamos el label del sector 0 primero. Si esta erroneo, ya se corregira cuando se detecte el primer archivo
    mdr_get_file_catalogue_get_label(catalogo->label,origen,0);

    int id_file=0;

    for (i=0;i<total_sectors;i++) {

        z80_byte data_recflg=mdr_get_file_catalogue_get_byte(origen,i,15);
        z80_byte record_segment=mdr_get_file_catalogue_get_byte(origen,i,16);

        if ((data_recflg & 0x04)==0x04) catalogo->used_sectors++;

        //Mostrar nombre archivo

            if (record_segment==0 && (data_recflg & 0x04)==0x04) {
                z80_int tamanyo=mdr_get_file_catalogue_get_byte(origen,i,31)+256*mdr_get_file_catalogue_get_byte(origen,i,32);

                //char nombre[11];




                //printf(" %s %d bytes\n",nombre,tamanyo);

                //char buffer_info_tape[32*4]; //4 lineas mas que suficiente

                z80_byte buffer_tap_temp[36];
                //primer byte cabecera
                buffer_tap_temp[0]=mdr_get_file_catalogue_get_byte(origen,i,30);
                catalogo->file[catalogo->total_files].header_info[0]=buffer_tap_temp[0];

                int j;


                //nombre
                for (j=0;j<10;j++) {
                    z80_byte letra_nombre=mdr_get_file_catalogue_get_byte(origen,i,19+j);

                    buffer_tap_temp[1+j]=letra_nombre;

                    catalogo->file[catalogo->total_files].name[j]=letra_nombre;
                    //nombre[j]=letra_nombre;
                }

                catalogo->file[catalogo->total_files].name[j]=0;

                //Ver si ese archivo ya existia, para considerar duplicados
                int id_file_con_copias;
                int copias_archivo=mdr_get_file_catalogue_get_copias(catalogo,catalogo->file[catalogo->total_files].name,&id_file_con_copias);

                if (copias_archivo>1) {
                    //A medida que se van leyendo archivos, si por ejemplo hay 3 archivos duplicados de nombre "run",
                    //el primero dirá que no tiene duplicados, el segundo dirá que tiene 1 duplicado, y el tercero dirá que
                    //tiene 2 duplicados
                    //Luego se reajusta al final el valor maximo para todos
                    //printf("Archivo [%s] tiene %d copias\n",catalogo->file[catalogo->total_files].name,copias_archivo);

                    //metemos mismo id
                    catalogo->file[catalogo->total_files].id_file=id_file_con_copias;
                }
                else {
                    //id_file uno generado
                    catalogo->file[catalogo->total_files].id_file=id_file;
                    id_file++;
                }

                printf("--Archivo [%s] con id [%d] tiene %d copias\n",
                    catalogo->file[catalogo->total_files].name,
                    catalogo->file[catalogo->total_files].id_file,
                    copias_archivo);

                catalogo->file[catalogo->total_files].numero_copias=copias_archivo;

                //parametros cabecera
                for (j=0;j<6;j++) {
                    buffer_tap_temp[11+j]=mdr_get_file_catalogue_get_byte(origen,i,31+j);
                    catalogo->file[catalogo->total_files].header_info[1+j]=buffer_tap_temp[11+j];
                }

                //excepcion en basic. esta diferente en cabecera de microdrive y de cinta
                //linea autorun
                if (buffer_tap_temp[0]==0) {
                    buffer_tap_temp[13]=mdr_get_file_catalogue_get_byte(origen,i,37);
                    buffer_tap_temp[14]=mdr_get_file_catalogue_get_byte(origen,i,38);
                }

                //excepcion en arrays. nombre variable
                if (buffer_tap_temp[0]==1 || buffer_tap_temp[0]==2) {
                    buffer_tap_temp[14]=mdr_get_file_catalogue_get_byte(origen,i,35);
                }



                //util_tape_tap_get_info(buffer_tap_temp,buffer_info_tape,0);

                z80_byte flag=0;
                z80_int longitud=19;

                util_tape_get_info_tapeblock((z80_byte *)buffer_tap_temp,flag,longitud,catalogo->file[catalogo->total_files].name_extended);





                //printf("Nombre: [%s] (file_length=%d)\n",buffer_info_tape,tamanyo);



                int frag,nofrag;
                mdr_get_info_file(origen,total_sectors,catalogo->file[catalogo->total_files].name,tamanyo,&catalogo->file[catalogo->total_files],&frag,&nofrag,i);

                catalogo->file[catalogo->total_files].file_size=tamanyo;

                //fragmentacion de ese archivo
                int file_frag_total_suma=frag+nofrag;

                int file_porc_frag;

                if (file_frag_total_suma==0) file_porc_frag=0;

                else file_porc_frag=(frag*100)/file_frag_total_suma;

                catalogo->file[catalogo->total_files].porcentaje_fragmentacion=file_porc_frag;


                //fragmentacion total
                frag_sectores_fragmentados +=frag;
                frag_sectores_no_fragmentados +=nofrag;


                catalogo->total_files++;


                //Corregir label, si sector 0 no se usaba, corregir desde primer sector usado
                if (!escrito_microdrive_label) {
                    escrito_microdrive_label=1;
                     mdr_get_file_catalogue_get_label(catalogo->label,origen,i);
                }
            }





    }


    mdr_set_max_copias_todos_archivos(catalogo);

    int total=frag_sectores_fragmentados+frag_sectores_no_fragmentados;

    int porc_frag;

    if (total==0) porc_frag=0;

    else porc_frag=(frag_sectores_fragmentados*100)/total;

    catalogo->porcentaje_fragmentacion=porc_frag;

    return catalogo;

}



void microdrive_switch_write_protection(int microdrive_seleccionado)
{
    microdrive_status[microdrive_seleccionado].microdrive_write_protect ^=1;

    microdrive_status[microdrive_seleccionado].microdrive_must_flush_to_disk=1;
}