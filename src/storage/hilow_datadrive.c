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
Sobre formato ddh:

Se define de manera simple máximo 256 sectores de 2048 bytes cada sector, por simplificar el acceso (y mínimo 3 sectores)
No se guardan las cabeceras de cada sector, que están presentes en las cintas, solo los datos.
El tamaño es variable, permito desde 3 sectores hasta 256

Sin embargo una cinta hilow al máximo de capacidad
tiene menos espacio usado (menos de 256 sectores). Esto se deduce formateando una cinta en formato crudo (audio raw)
y viendo como asigna la tabla de sectores disponibles.

-Sector 0: cuando se pide acceder a dicho sector, se solicita acceso al sector de directorio 1 o 2,
segun el que esté actualizado mas recientemente
-En una cinta de máxima capacidad, el 7e, 7f, fe,ff no los pone como usables en la tabla de sectores libres
Sospecho que porque el ff se usa realmente como identificador de fin de tabla, o porque no habia
mas espacio en la tabla de sectores libres (esto explicaria porque tampoco aparecen 7e o fe)
El 80h no se usa de la misma manera que el 0 tampoco
Y como el 1 y 2 son de directorio, el 81h y 82h no se usan (porque no hay directorio de la cara B,
quizá pensaron en usarlos también como directorio pero no)

Por tanto tenemos:

-del 03 hasta el fd (251) pero no esta 7e,7f,80,81,82, o sea 251-5=246 para asignar mas los 2 de directorio=248. = 496 total en crudo

01 al 7d  son 125

83 al fd (253) son

131 al 253 son 123

125 cara A+123 cara B=248 sectores tota.es
Creo que no hay feh ni ffh por no tener más espacio en la tabla de directorio. O porque el ff siempre es byte de final


Por tanto en un archivo ddh se tendrán los sectores sin usar (y por tanto ocupando espacio de mas):

00, 7e, 7f,80,81,82,fe, ff

*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

//#if defined(__APPLE__)
//        #include <sys/syslimits.h>
//#endif


#include "hilow_datadrive.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "compileoptions.h"
#include "operaciones.h"
#include "ula.h"
#include "screen.h"
#include "menu_items.h"
#include "mem128.h"
#include "audio.h"


z80_bit hilow_enabled={0};


//8 KB ROM, 2 KB RAM
//RAM mapeada en modo mirror en 2000h, 2800h, 3000h, 3800h
z80_byte *hilow_memory_pointer;



int hilow_nested_id_poke_byte;
int hilow_nested_id_poke_byte_no_time;
int hilow_nested_id_peek_byte;
int hilow_nested_id_peek_byte_no_time;
int hilow_nested_id_core;

z80_bit hilow_mapped_rom={0};
z80_bit hilow_mapped_ram={0};


//Flag que indica si cinta insertada, esto se utiliza por la lectura del puerto de hardware
z80_bit hilow_cinta_insertada_flag={1};

z80_bit hilow_tapa_has_been_opened={0};

int hilow_must_flush_to_disk=0;


//Si cambios en escritura se hace flush a disco
z80_bit hilow_persistent_writes={1};


z80_bit hilow_write_protection={0};

//Si se hacen traps a la rom. Si está desactivado, se usará emulación del puerto FF
z80_bit hilow_rom_traps={1};

z80_bit hilow_hear_load_sound={1};
z80_bit hilow_hear_save_sound={1};

//Para archivo ddh o raw
char hilow_file_name[PATH_MAX]="";

//Para archivos raw
//char hilow_raw_file_name[PATH_MAX]="";


//para guardar la imagen del datadrive ddh
//nota: aunque se usen archivos raw y no ddh, este buffer se asignara igual
z80_byte *hilow_device_buffer=NULL;

//para guardar la imagen en raw del datadrive, caras A y B
z80_byte *hilow_raw_device_buffer_side_a=NULL;
z80_byte *hilow_raw_device_buffer_side_b=NULL;

//Total espacio usado por el buffer raw
int hilow_raw_device_buffer_total_size=0;

//Tal cual emular el boton de casette encendido
z80_bit hilow_reproductor_encendido={1};

//Posicion del cabezal sobre la cinta
int hilow_posicion_cabezal=0;

//Ultimo valor enviado al puerto FF
z80_byte last_hilow_port_value=0;

//Esto solo es para enviar a tarjeta de sonido por si queremos escuchar sonidos de carga o grabacion
z80_byte hilow_ultimo_sample_sonido=0;

//Ultimo sample de sonido leido
z80_byte last_raw_audio_data_read=0;

//
//Inicio gestion movimiento de la cinta
//

//Valor de t_estados anterior
int hilow_estados_anterior=0;

//Cuantos t_estados han pasado desde el ultimo avance del hilow
int hilow_transcurrido_desde_ultimo_movimiento=0;


int speed_hilow_normal=8;
int speed_hilow_rapido=2;
//10x speed for playing (which works out at 8 t-states per sample) and 40x speed for fast forward/rewind (2 t-states per sample).

//tiempo en segundos en que la cinta se queda al principio o al final, antes de detener el motor
#define HILOW_RAW_TIEMPO_EXTREMO 3

int hilow_raw_contador_tiempo_llegado_extremo_cinta=0;

//si la cinta esta en el extremo (inicio o final)
int hilow_raw_cinta_en_extremo=0;

//Si esta en movimiento, o sea:
//motor encendido y cinta no está en uno de los extremos
int hilow_cinta_en_movimiento=0;


//Para debug cuando se usan rom traps
int debug_hilow_last_sector=0;

//Tamaño del archivo ddh leido
int hilow_ddh_file_size=0;

//
//Fin gestion movimiento de la cinta
//


//-1 si no aplica
int hilow_get_visualmem_position(unsigned int address)
{
#ifdef EMULATE_VISUALMEM


    //El buffer de visualmem en este caso tiene mismo tamaño que dispositivo hilow
    int posicion_final=address;

    //por si acaso
    if (posicion_final>=0 && posicion_final<VISUALMEM_HILOW_BUFFER_SIZE) {
        return posicion_final;
            //printf ("add %d hilow_size %ld visualsize: %d final: %ld\n",address,hilow_size,VISUALMEM_hilow_BUFFER_SIZE,posicion_final);

    }


#endif

	return -1;
}

void hilow_set_visualmem_read(unsigned int address)
{
#ifdef EMULATE_VISUALMEM
	int posicion_final=hilow_get_visualmem_position(address);
	if (posicion_final>=0) {
		set_visualmemhilow_read_buffer(posicion_final);
	}

#endif
}

void hilow_set_visualmem_write(unsigned int address)
{
#ifdef EMULATE_VISUALMEM
	int posicion_final=hilow_get_visualmem_position(address);
	if (posicion_final>=0) {
		set_visualmemhilow_write_buffer(posicion_final);
	}

#endif
}


void hilow_flush_contents_to_disk(void)
{

    if (hilow_enabled.v==0) return;

    //Si es archivo raw, no hacer flush aqui
    if (hilow_rom_traps.v==0) return;

    if (hilow_must_flush_to_disk==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush HiLow to disk but no changes made");
        return;
    }

    if (hilow_persistent_writes.v==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush HiLow to disk but persistent writes disabled");
        return;
    }


    debug_printf (VERBOSE_INFO,"Flushing HiLow to disk");


    FILE *ptr_hilowfile;

    debug_printf (VERBOSE_INFO,"Opening HiLow File %s",hilow_file_name);
    ptr_hilowfile=fopen(hilow_file_name,"wb");



    int escritos=0;
    long long int size;
    size=hilow_ddh_file_size; //HILOW_DEVICE_SIZE;



    if (ptr_hilowfile!=NULL) {


        z80_byte *puntero;
        puntero=hilow_device_buffer;

        //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
        //metera flush a 1
        hilow_must_flush_to_disk=0;

        escritos=fwrite(puntero,1,size,ptr_hilowfile);

        fclose(ptr_hilowfile);


    }

    //printf ("ptr_hilowfile: %d\n",ptr_hilowfile);
    //printf ("escritos: %d\n",escritos);

    if (escritos!=size || ptr_hilowfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error writing to HiLow file. Disabling write file operations");
        hilow_persistent_writes.v=0;
    }

}


/*
Escribir contenido de cara A y cara B, uno despues del otro
*/
void hilow_raw_flush_contents_to_disk(void)
{

    if (hilow_enabled.v==0) return;

    //Si no es archivo raw, no hacer flush aqui
    if (hilow_rom_traps.v) return;

    if (hilow_must_flush_to_disk==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush HiLow to disk but no changes made");
        return;
    }

    if (hilow_persistent_writes.v==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush HiLow to disk but persistent writes disabled");
        return;
    }

    debug_printf (VERBOSE_INFO,"Flushing HiLow to disk");


    FILE *ptr_hilowfile;

    debug_printf (VERBOSE_INFO,"Opening HiLow File %s",hilow_file_name);
    ptr_hilowfile=fopen(hilow_file_name,"wb");



    int escritos=0;


    if (ptr_hilowfile!=NULL) {

        escritos=fwrite(hilow_raw_device_buffer_side_a,1,hilow_raw_device_buffer_total_size,ptr_hilowfile);
        if (escritos!=hilow_raw_device_buffer_total_size) {
            debug_printf (VERBOSE_ERR,"Error writing to HiLow file");
        }

        escritos=fwrite(hilow_raw_device_buffer_side_b,1,hilow_raw_device_buffer_total_size,ptr_hilowfile);
        if (escritos!=hilow_raw_device_buffer_total_size) {
            debug_printf (VERBOSE_ERR,"Error writing to HiLow file");
        }

        fclose(ptr_hilowfile);
    }


    if (ptr_hilowfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error writing to HiLow file");
    }


}



int hilow_check_if_rom_area(z80_int dir)
{
    if (dir<8192 && hilow_mapped_rom.v) {
        return 1;
    }
	return 0;
}

int hilow_check_if_ram_area(z80_int dir)
{
    if (dir>=8192 && dir<16384 && hilow_mapped_ram.v) {
        return 1;
    }
	return 0;
}

z80_byte hilow_read_rom_byte(z80_int dir)
{
	//printf ("Read rom byte from %04XH\n",dir);
	return hilow_memory_pointer[dir];
}


z80_byte hilow_read_ram_byte(z80_int dir)
{

	//printf ("Read ram byte from %04XH\n",dir);


    //8kb ram
    dir &= (HILOW_RAM_SIZE-1);


	//La RAM esta despues de los 8kb de rom
	return hilow_memory_pointer[8192+dir];
}

void hilow_poke_ram(z80_int dir,z80_byte value)
{

	if (hilow_check_if_ram_area(dir) ) {
		//printf ("Poke ram byte to %04XH with value %02XH\n",dir,value);

        //8kb ram
        dir &= (HILOW_RAM_SIZE-1);

		//La RAM esta despues de los 8kb de rom
		hilow_memory_pointer[8192+dir]=value;
	}

}


z80_byte hilow_poke_byte(z80_int dir,z80_byte valor)
{

    //Llamar a anterior
    debug_nested_poke_byte_call_previous(hilow_nested_id_poke_byte,dir,valor);

	hilow_poke_ram(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte hilow_poke_byte_no_time(z80_int dir,z80_byte valor)
{

	//Llamar a anterior
	debug_nested_poke_byte_no_time_call_previous(hilow_nested_id_poke_byte_no_time,dir,valor);


	hilow_poke_ram(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte hilow_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(hilow_nested_id_peek_byte,dir);


	if (hilow_check_if_rom_area(dir)) {
		return hilow_read_rom_byte(dir);
	}

	if (hilow_check_if_ram_area(dir)) {
		return hilow_read_ram_byte(dir);
	}

	return valor_leido;
}

z80_byte hilow_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(hilow_nested_id_peek_byte_no_time,dir);


	if (hilow_check_if_rom_area(dir)) {
        return hilow_read_rom_byte(dir);
    }

	if (hilow_check_if_ram_area(dir)) {
		return hilow_read_ram_byte(dir);
	}

	return valor_leido;
}

int hilow_if_rom_basic_enabled(void)
{
    //Dice si esta activada la rom que contiene el basic
    //en 48k-> siempre
    //en maquinas 128k: rom3 en el caso de +2A/+3, rom 1 en caso de 128k
    if (MACHINE_IS_SPECTRUM_16_48) {

        //maquina 16k, inves ,48k o tk
        return 1;
    }

    if (MACHINE_IS_SPECTRUM_128_P2)  {

        //maquina 128k. rom 1 mapeada
        if ((puerto_32765 & 16) ==16)
        return 1;
    }

    if (MACHINE_IS_SPECTRUM_P2A_P3) {
        //maquina +2A
        if ((puerto_32765 & 16) ==16   && ((puerto_8189&4) ==4  ))
        return 1;
    }

    return 0;
}

void hilow_automap_unmap_memory(z80_int dir)
{
	//Si hay que mapear/desmapear memorias
	//printf ("test dir %04XH\n",dir);

	//Puntos de mapeo rom
	//Si no estaba mapeada
	if (hilow_mapped_rom.v==0) {
        if (hilow_if_rom_basic_enabled()) {
		    if (dir==0x04C2 || dir==0x0556 || dir==0x0976) {
    			//printf ("Mapeando rom y ram en %04XH\n",dir);
	    		hilow_mapped_rom.v=1;
                hilow_mapped_ram.v=1;
		    }
        }
	}

	//Puntos de desmapeo rom
	//Si estaba mapeada
	if (hilow_mapped_rom.v==1) {
		if (dir==0x0052) {
			hilow_mapped_rom.v=0;
            hilow_mapped_ram.v=0;
			//printf ("Desmapeando rom y ram en %04XH\n",dir);
		}
	}

}

void hilow_nmi(void)
{
    if (hilow_mapped_rom.v==0) {
        debug_printf(VERBOSE_DEBUG,"Enabling hilow memory from nmi triggered");
        hilow_mapped_rom.v=1;
        hilow_mapped_ram.v=1;
    }
}

void hilow_footer_operating(void)
{
    generic_footertext_print_operating("HILOW");
    watermark_tell_device_activity();

    //Y poner icono en inverso
    if (!zxdesktop_icon_hilow_inverse) {
        //printf("icon activity\n");
        zxdesktop_icon_hilow_inverse=1;
        menu_draw_ext_desktop();
    }
}

void hilow_tapa_action_was_opened(void)
{
   hilow_tapa_has_been_opened.v=1;
}

void hilow_tapa_reset_was_opened(void)
{
   hilow_tapa_has_been_opened.v=0;
}



void hilow_action_open_tape(void)
{
    if (hilow_cinta_insertada_flag.v) {
        //quitamos cinta. ademas indicamos tapa se ha abierto
        hilow_tapa_action_was_opened();
        hilow_cinta_insertada_flag.v=0;
    }

}

void hilow_action_close_tape(void)
{
    if (hilow_cinta_insertada_flag.v==0) {
        hilow_cinta_insertada_flag.v=1;
    }
}

//Sector desde 0,1,2,3,4.... 7b,7c, 82,83,84
int old_hilow_get_offset_sector(int sector,int offset)
{
    if (sector==0x7f || sector==0x80 || sector==0x81) {
        printf("Invalid sector %02XH\n",sector);
        return -1;
    }

    int cara_b=sector & 0x80;

    int sector_sin_cara=sector & 0x7f;

    if (cara_b) sector_sin_cara -=2; //El primero sera 82h

    /*
    Sectores cara A: del 01 al 7d  son 125
    Sectores cara B: 83(131) al fd (253) son 123
    */

    int offset_final=sector_sin_cara*HILOW_SECTOR_SIZE;

    if (cara_b) offset_final +=(125*HILOW_SECTOR_SIZE);

    offset_final +=offset;

    return offset_final;


}


int hilow_get_offset_sector(int sector,int offset)
{

    int offset_final=sector*HILOW_SECTOR_SIZE;

    offset_final +=offset;

    return offset_final;


}

int hilow_write_byte_device(int sector,int offset,z80_byte valor)
{
    hilow_footer_operating();

    //autocerrar tapa
    hilow_tapa_reset_was_opened();

    if (hilow_write_protection.v) return 0;


    offset=hilow_get_offset_sector(sector,offset);

    if (offset>=HILOW_DEVICE_SIZE || offset<0) {
        debug_printf (VERBOSE_DEBUG,"Error. Trying to write beyond HiLow Data Drive. Size: %ld Asked sector: %d Offset: %d",
                        HILOW_DEVICE_SIZE,sector,offset);

        //no desactivamos porque esto implica quitar funciones de peek/poke anidadas del core y petaria
		//hilow_disable();
		return 1;
	}

    hilow_device_buffer[offset]=valor;

    hilow_set_visualmem_write(offset);

    hilow_must_flush_to_disk=1;

    return 0;
}

z80_byte hilow_read_byte_device(int sector,int offset)
{
    hilow_footer_operating();

    //autocerrar tapa
    hilow_tapa_reset_was_opened();


    offset=hilow_get_offset_sector(sector,offset);

    if (offset>=HILOW_DEVICE_SIZE || offset<0) {
        debug_printf (VERBOSE_INFO,"Error. Trying to read beyond HiLow Data Drive. Size: %ld Asked sector: %d Offset: %d",
                        HILOW_DEVICE_SIZE,sector,offset);
        //no desactivamos porque esto implica quitar funciones de peek/poke anidadas del core y petaria
		//hilow_disable();
		return 0;
	}

    hilow_set_visualmem_read(offset);

    return hilow_device_buffer[offset];
}

void temp_directorio_falso(z80_int inicio_datos)
{

    int i;

     if (reg_a==0) { //Sector 0 directorio
         //Sector 0 directorio

                for (i=0;i<2048;i++) {
                    //poke_byte_no_time(reg_ix+i,'!');
                    //reg_de?
                    //poke_byte_no_time(inicio_datos+i,'!');

                        z80_int destino=inicio_datos+i;
                        destino &= (HILOW_RAM_SIZE-1);
                        destino +=8192;

                        z80_int temp_sp=reg_sp;
                        temp_sp &= (HILOW_RAM_SIZE-1);
                        temp_sp +=8192;

                        //Chapuza para no sobreescribir stack. Temporal
                        if (destino!=temp_sp && destino!=temp_sp+1) {
                    poke_byte_no_time(inicio_datos+i,255);
                        }
                }

                //TODO: en algun punto dice los KB libres de la cinta...



                //inicio_datos=8192+reg_de;

                //numero entradas en cinta?
                poke_byte_no_time(inicio_datos+0,2);
                poke_byte_no_time(inicio_datos+1,10);

                //nombre cinta
                poke_byte_no_time(inicio_datos+2,'Z');
                poke_byte_no_time(inicio_datos+3,'E');
                poke_byte_no_time(inicio_datos+4,'s');
                poke_byte_no_time(inicio_datos+5,'a');
                poke_byte_no_time(inicio_datos+6,'r');
                poke_byte_no_time(inicio_datos+7,'U');
                poke_byte_no_time(inicio_datos+8,'X');
                poke_byte_no_time(inicio_datos+9,'D');
                poke_byte_no_time(inicio_datos+10,'D');




                //Maximo 22 archivos
                //primera entrada cinta
                //tipo archivo
                //0=bas
                //1=num
                //2=chr
                //3=cod
                //4=nmi
                //FF=borrado? fin de directorio?
                poke_byte_no_time(inicio_datos+11,3);
                poke_byte_no_time(inicio_datos+11+1,'A');
                poke_byte_no_time(inicio_datos+11+2,'1');
                poke_byte_no_time(inicio_datos+11+3,'2');
                poke_byte_no_time(inicio_datos+11+4,'3');
                poke_byte_no_time(inicio_datos+11+5,'4');
                poke_byte_no_time(inicio_datos+11+6,'5');
                poke_byte_no_time(inicio_datos+11+7,'6');
                poke_byte_no_time(inicio_datos+11+8,'7');
                poke_byte_no_time(inicio_datos+11+9,'8');
                poke_byte_no_time(inicio_datos+11+10,' ');

                //tamaño archivo.  -> screen$
                poke_byte_no_time(inicio_datos+11+11,0x00);
                poke_byte_no_time(inicio_datos+11+12,0x1b);

                //primer parametro de cabecera de cinta: direccion, line, etc
                poke_byte_no_time(inicio_datos+11+13,4);
                poke_byte_no_time(inicio_datos+11+14,2);

                //byte 15?
                //byte 16?

                //byte 17: numero de sectores
                //byte 18: primer sector
                //byte 19: segundo sector
                //etc

                //atributo? valor 3 (o bits 0 y 1 activos)= directorio?
                //for (i=11+15;i<11+45;i++) {
                //    poke_byte_no_time(inicio_datos+i,0);
                //}


                //for (i=inicio_datos+12+10+4;i<inicio_datos+2048;i++) {
                //    poke_byte_no_time(i,0);
                //}

                //segunda entrada. cada entrada 45 bytes aparentemente
                poke_byte_no_time(inicio_datos+11+45,0);
                poke_byte_no_time(inicio_datos+11+45+1,'B');
                poke_byte_no_time(inicio_datos+11+45+2,' ');
                poke_byte_no_time(inicio_datos+11+45+3,' ');
                poke_byte_no_time(inicio_datos+11+45+4,' ');
                poke_byte_no_time(inicio_datos+11+45+5,' ');
                poke_byte_no_time(inicio_datos+11+45+6,' ');
                poke_byte_no_time(inicio_datos+11+45+7,' ');
                poke_byte_no_time(inicio_datos+11+45+8,' ');
                poke_byte_no_time(inicio_datos+11+45+9,' ');
                poke_byte_no_time(inicio_datos+11+45+10,' ');

                //tercera entrada. cada entrada 45 bytes aparentemente
                poke_byte_no_time(inicio_datos+11+45+45,0);
                poke_byte_no_time(inicio_datos+11+45+45+1,'C');
                poke_byte_no_time(inicio_datos+11+45+45+2,' ');
                poke_byte_no_time(inicio_datos+11+45+45+3,' ');
                poke_byte_no_time(inicio_datos+11+45+45+4,' ');
                poke_byte_no_time(inicio_datos+11+45+45+5,' ');
                poke_byte_no_time(inicio_datos+11+45+45+6,' ');
                poke_byte_no_time(inicio_datos+11+45+45+7,' ');
                poke_byte_no_time(inicio_datos+11+45+45+8,' ');
                poke_byte_no_time(inicio_datos+11+45+45+9,' ');
                poke_byte_no_time(inicio_datos+11+45+45+10,' ');


                //cuarta entrada. cada entrada 45 bytes aparentemente. Indicamos con FF final de entradas
                //La FF es de final y por tanto no se ve
                poke_byte_no_time(inicio_datos+11+45+45+45,0xFF);
                poke_byte_no_time(inicio_datos+11+45+45+46,'D');

            }

            else {
                //sector no 0
                poke_byte_no_time(reg_ix,34);

                //un solo load code de un archivo de 769 bytes, cargado en 16384 hace:

                //Entering READ_SECTOR. A=03H IX=4000H DE=0800H HL=0800H BC=0003H
                //PC=186d SP=3fd4 AF=0301 BC=0003 HL=0800 DE=0800 IX=4000 IY=5c3a AF'=0301 BC'=1721 HL'=ffff DE'=369b I=3f R=5d  F=-------C F'=-------C MEMPTR=186d IM1 IFF-- VPS: 0 MMU=00000000000000000000000000000000
                //..ZEsarUXDD.A12345678 ...................................B         .................................

                //Returning to address 1D52H
                //Entering READ_SECTOR. A=03H IX=4000H DE=0800H HL=0800H BC=0003H
                //PC=186d SP=3fd8 AF=0301 BC=0003 HL=0800 DE=0800 IX=4000 IY=5c3a AF'=0301 BC'=1721 HL'=ffff DE'=369b I=3f R=6a  F=-------C F'=-------C MEMPTR=186d IM1 IFF-- VPS: 0 MMU=00000000000000000000000000000000
                //".ZEsarUXDD.A12345678 ...................................B         .................................

                //Returning to address 1D52H
                //Entering READ_SECTOR. A=03H IX=4000H DE=0301H HL=0800H BC=0003H
                //PC=186d SP=3fdc AF=0301 BC=0003 HL=0800 DE=0301 IX=4000 IY=5c3a AF'=0301 BC'=1721 HL'=ffff DE'=369b I=3f R=77  F=-------C F'=-------C MEMPTR=186d IM1 IFF-- VPS: 0 MMU=00000000000000000000000000000000

            }
}



/*
void temp_debug_registers(void)
{
    char buffer[HILOW_SECTOR_SIZE];
    print_registers(buffer);

    printf ("%s\n",buffer);

}

void temp_debug_mem_registers(void)
{
    temp_debug_registers();

    //mostrar algunos caracteres
    int i;
    for (i=0;i<100;i++) {
        z80_byte c=hilow_read_ram_byte(i);
        printf("%c",(c>=32 && c<=126 ? c : '.'));
    }
    printf("\n");
}

void temp_dump_from_addr(z80_int dir)
{

    //mostrar algunos caracteres
    int i;
    for (i=0;i<100;i++) {
        z80_byte c=peek_byte_no_time(dir+i);
        printf("%c",(c>=32 && c<=126 ? c : '.'));
    }
    printf("\n");
}
*/

int hilow_write_mem_to_device(z80_int dir,int sector,int longitud,int offset_destination)
{
    int i;



    for (i=0;i<longitud;i++) {
        z80_byte c=peek_byte_no_time(dir+i);
        if (hilow_write_byte_device(sector,i+offset_destination,c)) {
            return 1;
        }
    }

    return 0;
}

void hilow_device_set_sectores_disponible(int si_escribir_en_ram,int si_escribir_en_device,z80_byte sectores_disponibles)
{


    int offset=1011; //1011 decimal =3f3H
    z80_byte value_to_write=sectores_disponibles; //HILOW_MAX_DATA_USABLE_SECTORS;

    debug_printf(VERBOSE_DEBUG,"Setting %d sectors available",value_to_write);

    if (si_escribir_en_ram) poke_byte_no_time(8192+offset,value_to_write);
    if (si_escribir_en_device) {
        //En ambas copias de directorio
        hilow_write_byte_device(1,offset,value_to_write);
        hilow_write_byte_device(2,offset,value_to_write);
    }
    //en las rutinas de la rom se suele acceder por la direccion 3BF3
    //(3BF3 AND 2047) = 1011
    //Dado que la ram es de 2kb y se repite desde 8192 hasta 16383, se puede acceder a misma memoria
    //desde varios sitios




}

void hilow_device_initialize_sector_zero(int si_escribir_en_ram,int si_escribir_en_device)
{


    int offset=0;
    int i;

    for (i=0;i<HILOW_SECTOR_SIZE;i++,offset++) {
        if (si_escribir_en_device) {
            //En ambas copias de directorio
            hilow_write_byte_device(1,offset,255);
            hilow_write_byte_device(2,offset,255);
        }
        if (si_escribir_en_ram) poke_byte_no_time(8192+offset,255);
    }


}

void hilow_device_set_usage_counter(int si_escribir_en_ram,int si_escribir_en_device)
{


    int offset=0;
    int i;

    for (i=0;i<2;i++,offset++) {
        if (si_escribir_en_device) {
            //En ambas copias de directorio
            hilow_write_byte_device(1,offset,0);
            hilow_write_byte_device(2,offset,0);
        }
        if (si_escribir_en_ram) poke_byte_no_time(8192+offset,0);
    }


}

int hilow_create_sector_table(int si_escribir_en_ram,int si_escribir_en_device,int lados,int *p_total_sectors)
{

    debug_printf(VERBOSE_DEBUG,"HiLow: Creating free sectors table");
    //int i;

    int id_sector_tabla;

    int offset=0x3f4;

    //Nota: el sector 0 como tal no se usa para archivos logicamente
    //pero en la tabla de sectores usados debe estar, pues al borrar archivos, en rutina L06D1, utiliza
    //ese 00 inicial para saber donde empieza la tabla y de alguna manera indica donde finalizar con el movimiento de sectores de la tabla


    //El 00 inicial de la tabla
    if (si_escribir_en_device) {
        //En ambas copias de directorio
        hilow_write_byte_device(1,offset,0);
        hilow_write_byte_device(2,offset,0);
    }
    if (si_escribir_en_ram) poke_byte_no_time(8192+offset,0);

    offset++;

    int sectores_disponibles=0;

    int total_sectores=hilow_ddh_file_size/HILOW_SECTOR_SIZE;

    if (lados==1) {
        if (total_sectores>128) total_sectores=128;
    }

    *p_total_sectors=total_sectores;

    //Es <= dado que el id de sector siempre lo decrementamos
    for (id_sector_tabla=0;id_sector_tabla<total_sectores;id_sector_tabla++) {
        //sectores 00,01,02,7e,7f, 80,81,82,fe,ff  no lo metemos en tabla
        int id_sector_sin_cara=id_sector_tabla & 0x7f;
        //Sin considerar cara es sector 0,1,2,7e,7f, asi la comparacion nos vale para las dos caras

        if (id_sector_sin_cara!=0 && id_sector_sin_cara!=1 && id_sector_sin_cara!=2 &&
            id_sector_sin_cara!=0x7e &&  id_sector_sin_cara!=0x7f) {

            sectores_disponibles++;

            if (si_escribir_en_device) {
                //En ambas copias de directorio
                hilow_write_byte_device(1,offset,id_sector_tabla);
                hilow_write_byte_device(2,offset,id_sector_tabla);
            }
            if (si_escribir_en_ram) poke_byte_no_time(8192+offset,id_sector_tabla);

            offset++;
        }

    }



    //Y byte 0 para el final. No estoy seguro que sea necesario
    //poke_byte_no_time(8192+offset,0);

    return sectores_disponibles;

}

void hilow_set_tapelabel(int si_escribir_en_ram,int si_escribir_en_device,char *label)
{

    int i;

    //9 espacios por defecto 123456789
    char buffer_destino[10]="         ";


    //guardarlo en buffer temporal con espacios
    for (i=0;i<9 && *label;i++) {
        buffer_destino[i]=*label;
        label++;
    }

    //Y pasarlo a memoria y/o dispositivo
    int offset=2;

    for (i=0;i<9;i++,offset++) {
        if (si_escribir_en_device) {
            //En ambas copias de directorio
            hilow_write_byte_device(1,offset,buffer_destino[i]);
            hilow_write_byte_device(2,offset,buffer_destino[i]);
        }
        if (si_escribir_en_ram) poke_byte_no_time(8192+offset,buffer_destino[i]);
    }


}

void hilow_device_clear_sectors(int total_sectors)
{
    int sector;

    debug_printf(VERBOSE_INFO,"Clearing total sectors of %d",total_sectors);

    //ignorar sectores no usables
    //desde el sector 3 en adelante
    for (sector=0;sector<total_sectors;sector++) {

        int id_sector_sin_cara=sector & 0x7f;
        //Sin considerar cara es sector 0,1,2,7e,7f, asi la comparacion nos vale para las dos caras

        if (id_sector_sin_cara!=0 && id_sector_sin_cara!=1 && id_sector_sin_cara!=2 &&
            id_sector_sin_cara!=0x7e &&  id_sector_sin_cara!=0x7f) {
            debug_printf(VERBOSE_DEBUG,"Clearing sector %02XH",sector);
            int offset;
            for (offset=0;offset<HILOW_SECTOR_SIZE;offset++) {
                hilow_write_byte_device(sector,offset,0);
            }
        }
    }

}


void hilow_device_mem_format(int si_escribir_en_ram,int si_escribir_en_device,char *label,int lados,int con_borrado)
{

    /*
    Formato de un directorio HiLow:

    - Directorio se guarda alternativamente en sector 1 y 2
    - Contenido directorio:

    * Offset 0: 16 bits: usage counter. Indica cuantas escrituras se han hecho en ese directorio. Dado que se alternan las escrituras del sector 1 y 2,
    lo habitual es que este usage counter en un sector sea igual al otro sector +1

    * Offset 2: 9 bytes. Etiqueta de la cinta

    * Offset 11: Primera entrada de archivo. Cada entrada ocupa 45 bytes. Los primeros 17 bytes son los mismos utilizados en cabeceras de ZX Spectrum:
         11: Tipo archivo: 0 basic, 1 matriz numerica, 2 matriz chr, 3 code. El valor 4 se usa para snapshots de nmi. Se asume valor 255 si esta es la última entrada de directorio
         12: Nombre. Empieza por caracter punto (.) excepto los snapshots nmi que empiezan por asterisco (*)
         22: Tamaño archivo
         24: Inicio, Linea autoarranque, etc
         26: Auxiliar: tamaño variables, etc

         28: Número de sectores usados en este archivo
         29: Número del primer sector usado en este archivo
         30: Número del segundo sector usado en este archivo
         etc...

    * Offset 56: Segunda entrada de archivo

    ...

    * Offset 956 : Entrada 22 de archivo

    * Offset 1009: Posibles datos de significado desconocido

    * Offset 1010: Posibles datos de significado desconocido

    * Offset 1011 (3F3H): 1 byte: numero de sectores disponibles

    * Offset 1012 (3F4H): 1 byte a 0. Marcador de inicio de la tabla de sectores disponibles

    * Offset 1013 (3F5H): X bytes. Tabla de sectores disponibles. En una cinta vacia empezará con 3, 4, 5, etc...
    Ni el 0 ni el 1 pueden estar en la lista

    */

    hilow_device_initialize_sector_zero(si_escribir_en_ram,si_escribir_en_device);

    hilow_device_set_usage_counter(si_escribir_en_ram,si_escribir_en_device);

    hilow_set_tapelabel(si_escribir_en_ram,si_escribir_en_device,label);

    int total_sectors;

    int sectores_disponibles=hilow_create_sector_table(si_escribir_en_ram,si_escribir_en_device,lados,&total_sectors);

    hilow_device_set_sectores_disponible(si_escribir_en_ram,si_escribir_en_device,sectores_disponibles);

    if (con_borrado) {
        hilow_device_clear_sectors(total_sectors);
    }

}


//Cual es el sector de la tabla de directorio leido. Al escribir se va alternando 1 y 2
int hilow_sector_tabla_directorio=1;

void hilow_write_directory_sector(void)
{
    hilow_sector_tabla_directorio++;
    if (hilow_sector_tabla_directorio==3) hilow_sector_tabla_directorio=1;

    z80_int dir_inicio=8192;
    z80_int longitud=HILOW_DIRECTORY_TABLE_SIZE;

    //printf("Writing directory (size: %d) to sector %d\n",longitud,hilow_sector_tabla_directorio);

    debug_printf(VERBOSE_INFO,"HiLow: Write from cache memory to directory sector (size: %d sector: %d)",longitud,hilow_sector_tabla_directorio);

    hilow_write_mem_to_device(dir_inicio,hilow_sector_tabla_directorio,longitud,0);
}

void hilow_trap_write_verify(void)
{
    //printf("VERIFY or WRITE probably\n");


    //printf("Retornando porque no carry. Posible escritura?\n\n");
    //temp_debug_registers();
    //temp_dump_from_addr(reg_ix);

    z80_byte retorno_error=0;

    if (!(Z80_FLAGS & FLAG_Z)) {
        debug_printf(VERBOSE_INFO,"HiLow: Verify action. Just return ok");
        //printf("VERIFY probably\n");
        //No hacer nada y retornar todo ok
    }

    else {

        //printf("WRITE probably\n");

        z80_int dir_inicio=reg_ix;
        z80_int longitud=reg_de;
        z80_byte sector=reg_a;


        //if (reg_de>HILOW_SECTOR_SIZE) {
        if (sector==0) {
            //printf("Writing from cache memory to directory sector\n");



            //directamente copiar lo de la cache hacia aqui
            //esto soluciona la escritura
            //hilow_write_mem_to_device(8192,0,HILOW_SECTOR_SIZE,0);
            hilow_write_directory_sector();
        }

        else {

            //Sectores 1 y 2 son de directorio.

            //Lo logico seria no permitir esto. Sector 0 para llamadas a la rom es el directorio (sea el 1 o el 2),
            //y luego se empieza a usar sectores de la rom a partir del 3
            if (sector==1 || sector==2) {
                debug_printf(VERBOSE_ERR,"HiLow: Trying to write to invalid sector %02XH",sector);
                return;
            }

            debug_printf(VERBOSE_INFO,"HiLow: Write from %04XH length %04XH sector %02XH",dir_inicio,longitud,sector);

            //sector--;

            debug_hilow_last_sector=sector;

            if (hilow_write_mem_to_device(dir_inicio,sector,longitud,0)) {
                //Error al escribir, sector mas alla del rango
                retorno_error=1; //Error en la cinta
            }

        }

    }

    //Retorno de verify o write

    reg_a=retorno_error;


    //No seguro de esto
    reg_ix +=reg_de;

    reg_pc=pop_valor();

    //printf("Returning from WRITE/VERIFY SECTOR to address %04XH\n",reg_pc);
}




int hilow_read_device_to_mem(z80_int dir,int sector,int longitud)
{
    int retorno_error=0;

    //no estoy seguro de esto
    if (longitud>HILOW_SECTOR_SIZE) {
        debug_printf (VERBOSE_DEBUG,"HiLow: Error. Size to read %d exceeds maximum %d",longitud,HILOW_SECTOR_SIZE);
        longitud=HILOW_SECTOR_SIZE;
    }

    //no estoy seguro de esto
    if (longitud==0) {
        debug_printf (VERBOSE_DEBUG,"HiLow: Size to read is zero");
        longitud=HILOW_SECTOR_SIZE;
    }



    //printf("Reading data from sector %d length %04XH to address %04XH\n",sector,longitud,dir);


    if (sector<0) {
        //printf("Sector beyond maximum. Do nothing\n");

        debug_printf (VERBOSE_DEBUG,"Error. Trying to read beyond max HiLow Data Drive sectors. Size: %ld Asked sector: %d",
                HILOW_DEVICE_SIZE,sector);
        retorno_error=1;
    }
    else {
        int i;
        for (i=0;i<longitud;i++) {
            poke_byte_no_time(dir+i,hilow_read_byte_device(sector,i));
        }
    }

    return retorno_error;

}

void hilow_read_directory_sector(void)
{
    //sector de directorio
    z80_int inicio_datos=8192;
    z80_int leer_datos=HILOW_DIRECTORY_TABLE_SIZE;

    //Usar el sector 1/2 dependiendo de que tenga el valor de contador mas alto

    z80_int contador_sector_cero;
    z80_int contador_sector_uno;


    contador_sector_cero=value_8_to_16(hilow_read_byte_device(1,1),hilow_read_byte_device(1,0));
    contador_sector_uno= value_8_to_16(hilow_read_byte_device(2,1),hilow_read_byte_device(2,0));

    //printf("Directory usage counters. Sector zero: %d Sector one: %d\n",contador_sector_cero,contador_sector_uno);


    if (contador_sector_cero>contador_sector_uno) {
        hilow_sector_tabla_directorio=1;
    }
    else {
        hilow_sector_tabla_directorio=2;
    }

    //printf("Reading directory (size: %d) from sector %d as it has the highest usage counter (or they are both the same)\n",leer_datos,hilow_sector_tabla_directorio);


    /* if (reg_a==0 && reg_de==0xFFFF && reg_sp<16384) {
        //leer sector 0 desde rutina de copia de archivos de una cinta a otra
        //Esto es una chapucilla pero funciona
        //leemos algo menos para no sobrescribir stack, pues SP probablemente estara sobre direccion 3FE2 aprox
        leer_datos=0x600;

    }    */

    debug_printf(VERBOSE_INFO,"HiLow: Read from directory sector (size: %d sector: %d) to cache memory",leer_datos,hilow_sector_tabla_directorio);

    reg_a=hilow_read_device_to_mem(inicio_datos,hilow_sector_tabla_directorio,leer_datos);
}

void hilow_trap_read(void)
{

    //int i;
    //temp_debug_mem_registers();

    //printf("READ probably\n");

    z80_int inicio_datos;
    z80_int leer_datos;

    //int offset_device=0;

    if (reg_a==0) {

        hilow_read_directory_sector();

    }

    else {
        inicio_datos=reg_ix;
        leer_datos=reg_de;

        int sector=reg_a;


        if (sector==1 || sector==2) {
            debug_printf(VERBOSE_ERR,"HiLow: Trying to read from invalid sector %d",sector);
            return;
        }

        debug_printf(VERBOSE_INFO,"HiLow: Read from sector %02XH to %04XH length %04XH",sector,inicio_datos,leer_datos);

        debug_hilow_last_sector=sector;

        reg_a=hilow_read_device_to_mem(inicio_datos,sector,leer_datos);
    }



    //No seguro de esto
    reg_ix +=reg_de;

    reg_pc=pop_valor();

    //printf("Returning from READ_SECTOR to address %04XH\n",reg_pc);

}


void hilow_trap_format(void)
{

    /*
    ; IX=inicio datos??  (quiza siempre direccion 8192)
    ; DE=longitud datos?? (quiza siempre escribe tamaño de HILOW_SECTOR_SIZE)
    ; A= sector
    ???
    */

    /*temp_debug_mem_registers();

    //mostrar algunos caracteres
    int i;
    for (i=0;i<2048;i++) {
        z80_byte c=peek_byte_no_time(8192+i);
        if (c>=32 && c<=126) printf("%c",c);
        else printf(" %02X ",c);
    }
    printf("\n");
    */



/*

                LD      A,$01           ;1=con borrado
                JR      Z,L08A9
                XOR     A               ;0=sin borrado

L08A9:          LD      (L3EF9),A
                LD      A,$11           ;Pregunta: [SIMPLE LADO  DOBLE LADO]
                CALL    L0B21           ;BIT 1 de $3EF9=1 SIMPLE
                LD      A,(L3EF9)
                JR      Z,L08BB         ;BIT 1 de $3EF9=0 DOBLE*/

    z80_byte byte_flags=peek_byte_no_time(0x3EF9);
    //printf("lado: %02XH\n",lado);

    int lados=(byte_flags & 0x02 ? 2 : 1);
    int con_borrado=byte_flags & 0x01;

    debug_printf(VERBOSE_INFO,"HiLow: Formatting device, total sides: %d, with%s erasing",lados,(con_borrado ? "" : "out"));

    //Asumimos siempre sector 0, pues rutina de formateo no llega a avanzar a siguientes sectores y da error  (error que interceptamos)

    //Dado que no finaliza el formateo, tenemos que indicar nosotros la tabla de sectores
    int total_sectors;
    int sectores_disponibles=hilow_create_sector_table(1,0,lados,&total_sectors);

    //Rellenamos parte restante del sector 0
    //Dado que no finaliza el formateo, tenemos que indicar nosotros el total de sectores disponibles
    hilow_device_set_sectores_disponible(1,0,sectores_disponibles);

    if (con_borrado) {
        hilow_device_clear_sectors(total_sectors);
    }


    //Finalmente escribimos tal cual el contenido de la memoria HiLow al dispositivo, en ambas copias de directorio
    hilow_write_mem_to_device(8192,1,HILOW_SECTOR_SIZE,0);
    hilow_write_mem_to_device(8192,2,HILOW_SECTOR_SIZE,0);

    //La rom posteriormente escribira una copia del directorio desde direccion L08C4

    //no error
    reg_a=0;

    reg_pc=pop_valor();

    //Decimos que el ultimo sector de directorio leido es el 1
    //esto realmente daria un poco igual, pero es para indicar despues de un formateo que dado que ambos sectores 1 y 2 son iguales,
    //los dos tienen contador a 0, y por la logica de escritura, habremos leido del 1 pero escrito en el 2
    hilow_sector_tabla_directorio=1;

    //printf("Returning from FORMAT to address %04XH\n",reg_pc);


}


z80_byte cpu_core_loop_spectrum_hilow(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

    //Solo contar estados cuando el core no está detenido esperando a siguiente interrupcion
    if (esperando_tiempo_final_t_estados.v==0) hilow_count_tstates();

    //Llamar a anterior
    debug_nested_core_call_previous(hilow_nested_id_core);


    hilow_automap_unmap_memory(reg_pc);

    if (hilow_mapped_rom.v==0) {
        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;
    }

    if (hilow_rom_traps.v==0) return 0;

    //Traps a la rom

    //debug de rutinas
    if (reg_pc==0x186D) {

        debug_printf(VERBOSE_DEBUG,"HiLow: Entering READ_WRITE_VERIFY_SECTOR. PC=%04XH return=%04XH A=%02XH Carry=%d Z=%d IX=%04XH DE=%04XH HL=%04XH BC=%04XH SP=%04XH",
            reg_pc,peek_word(reg_sp),reg_a,Z80_FLAGS & FLAG_C,(Z80_FLAGS & FLAG_Z ? 1 : 0),reg_ix,reg_de,reg_hl,reg_bc,reg_sp);


        //printf("(3F31)=%04XH\n",peek_word(0x3f31));
        //printf("Sector: %d\n",reg_a);


        if (!(Z80_FLAGS & FLAG_C)) {
            hilow_trap_write_verify();
        }

        //Read
        else {
            hilow_trap_read();
        }
    }

    //debug de rutinas
    if (reg_pc==0x16D0) {
        debug_printf(VERBOSE_DEBUG,"HiLow: Entering FORMAT_SECTOR. PC=%04XH",reg_pc);

        hilow_trap_format();
    }

    if (reg_pc==0x1BEF) {
        debug_printf(VERBOSE_DEBUG,"HiLow: Entering COMMAND_HILOW_PORT PC=%04XH. Returning",reg_pc);
        reg_pc=pop_valor();
        //printf("\nExiting to %04XH\n",reg_pc);
    }

    //No saltamos este retardo. Asi permite poder ver algo del texto de inicio de formateo. Si no, seria demasiado rapido
    /*if (reg_pc==0x17D5) {
        printf("\nEntering RETARDO PC=%04XH. Exiting\n",reg_pc);

        reg_a=1;
        reg_hl=0;

        reg_pc=pop_valor();
        printf("\nExiting to %04XH\n",reg_pc);
    }*/

    /*if (reg_pc==0x17BB) {
        printf("\nEntering DETECT ERROR PC=%04XH. Skipping\n",reg_pc);

        reg_pc+=3;


        printf("\nExiting to %04XH\n",reg_pc);
    }

    if (reg_pc==0x17B4) {
        printf("\nEntering DETECT Z PC=%04XH. Forcing Z\n",reg_pc);
        //L17B4:          JR      Z,L17D0

        Z80_FLAGS |=FLAG_Z;

    }


    if (reg_pc==0x1B4C) {
        printf("\nEntering Cinta no sirve PC=%04XH. Forcing C\n",reg_pc);
        //L1B4C:          JP      NC,L1A47  ;Cinta no sirve

        Z80_FLAGS |=FLAG_C;

    }

    if (reg_pc==0x1BBC || reg_pc==0x1BCB) {
        printf("\nEntering More detect NC PC=%04XH. Forcing C\n",reg_pc);

        Z80_FLAGS |=FLAG_C;

    }
    */



    if (reg_pc==0x1A9E) {

        z80_int next_pc=0x1acf;

        debug_printf(VERBOSE_DEBUG,"HiLow: Entering PRE_FORMAT. PC=%04XH. Skipping to %04XH",reg_pc,next_pc);

        //saltar adelante en codigo. feo....

        reg_pc=next_pc;

        //printf("Skipping to address %04XH\n",reg_pc);
    }

    if (reg_pc==0x1AC0) {

        z80_int next_pc=0x1acf;

        debug_printf(VERBOSE_DEBUG,"HiLow: Entering PRE_FORMAT2. PC=%04XH. Skipping to %04XH",reg_pc,next_pc);


        //saltar adelante en codigo. feo....

        reg_pc=next_pc;

        //printf("Skipping to address %04XH\n",reg_pc);
    }

    //Evitar error del formateo
    if (reg_pc==0x08C4 && (!(Z80_FLAGS & FLAG_Z)) ) {
        //L08C4:          JP      NZ,ERR_IO

        //Saltar instruccion
        debug_printf(VERBOSE_DEBUG,"HiLow: Entering POST_FORMAT_ERROR. PC=%04XH. Avoiding it",reg_pc);

        reg_pc +=3;

        //Y hacemos un CALL para hacer sonido. Para que se vea momentaneamente texto de Formateando...
        //push_valor(reg_pc,PUSH_VALUE_TYPE_CALL);
        //reg_pc=0x106F;

        //printf("Skipping to address %04XH\n",reg_pc);
    }


    /*if (reg_pc==0x1AF1) {

        printf("\nEntering POST_FORMAT3. A=%02XH IX=%04XH DE=%04XH\n",reg_a,reg_ix,reg_de);

        //engañar... para saltar una condicion que hace cancelar el bucle de sectores 1,2,3,...
        //Z80_FLAGS |=FLAG_Z;
    }*/


    /*if (reg_pc==0x08FB) {
        printf("\nEntering L08FB. A=%02XH IX=%04XH DE=%04XH\n",reg_a,reg_ix,reg_de);

        //saltar opcode JP      Z,BREAKCONT
        reg_pc +=3;

        printf("Skipping to address %04XH\n",reg_pc);

        //Esto al hacer un SAVE al final parece ir a la direccion 0 y se resetea...
    }*/
    //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
    return 0;

}



//Establecer rutinas propias
void hilow_set_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting hilow poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	hilow_nested_id_poke_byte=debug_nested_poke_byte_add(hilow_poke_byte,"HiLow poke_byte");
	hilow_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(hilow_poke_byte_no_time,"HiLow poke_byte_no_time");
	hilow_nested_id_peek_byte=debug_nested_peek_byte_add(hilow_peek_byte,"HiLow peek_byte");
	hilow_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(hilow_peek_byte_no_time,"HiLow peek_byte_no_time");


	hilow_nested_id_core=debug_nested_core_add(cpu_core_loop_spectrum_hilow,"HiLow Spectrum core");


}

//Restaurar rutinas de hilow
void hilow_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before hilow");


	debug_nested_poke_byte_del(hilow_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(hilow_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(hilow_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(hilow_nested_id_peek_byte_no_time);


	debug_nested_core_del(hilow_nested_id_core);
}



void hilow_alloc_rom_ram_memory(void)
{
    //memoria de la ram y rom
    int size=HILOW_MEM_SIZE;

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for hilow emulation",size/1024);

    hilow_memory_pointer=malloc(size);
    if (hilow_memory_pointer==NULL) {
        cpu_panic ("No enough memory for hilow emulation");
    }

    //vaciar esa memoria, para no dejar rastros en el dispositivo de cosas de la ram de ZEsarUX
    memset(hilow_memory_pointer,0,size);


}

void hilow_alloc_device_memory(void)
{

    //z80_byte hilow_device_buffer[HILOW_DEVICE_SIZE];

    int size=HILOW_DEVICE_SIZE;

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for hilow device emulation",size/1024);


    hilow_device_buffer=malloc(size);
    if (hilow_device_buffer==NULL) {
        cpu_panic ("No enough memory for hilow emulation");
    }


}

int hilow_load_rom(void)
{

    FILE *ptr_hilow_romfile;
    int leidos=0;

    debug_printf (VERBOSE_INFO,"Loading hilow rom %s",HILOW_ROM_FILE_NAME);

    open_sharedfile(HILOW_ROM_FILE_NAME,&ptr_hilow_romfile);
    if (!ptr_hilow_romfile) {
            debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_hilow_romfile!=NULL) {

        leidos=fread(hilow_memory_pointer,1,HILOW_ROM_SIZE,ptr_hilow_romfile);
        fclose(ptr_hilow_romfile);

    }



    if (leidos!=HILOW_ROM_SIZE || ptr_hilow_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading hilow rom");
        return 1;
    }

    return 0;
}



int hilow_load_ddh_device_file(void)
{
    if (hilow_device_buffer==NULL) {
        debug_printf(VERBOSE_ERR,"HiLow is not enabled");
        return 1;
    }

    FILE *ptr_hilowfile;
    //unsigned int leidos=0;

    debug_printf (VERBOSE_INFO,"Opening HiLow Data Drive File %s",hilow_file_name);
    ptr_hilowfile=fopen(hilow_file_name,"rb");

    //Maximo a leer
    //Nota: si se usa un archivo de mas capacidad, no se leera el resto
    //y cuando se guarde, se grabara con el maximo tamaño, no con el tamaño que tenia antes
    unsigned int bytes_a_leer=HILOW_DEVICE_SIZE;


    if (ptr_hilowfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error opening HiLow Data Drive file %s",hilow_file_name);
        return 1;
    }

    //leidos=fread(hilow_device_buffer,1,bytes_a_leer,ptr_hilowfile);
    unsigned int leidos=fread(hilow_device_buffer,1,bytes_a_leer,ptr_hilowfile);
    fclose(ptr_hilowfile);

    //Al menos 3 sectores
    int sectores=leidos/HILOW_SECTOR_SIZE;
    if (sectores<3) {
        debug_printf(VERBOSE_ERR,"Invalid file size");
        return 1;
    }

    hilow_ddh_file_size=leidos;

    //De momento no comprobamos tamaño leido, por si en el futuro cambiamos el formato
    /*
    if (leidos!=bytes_a_leer) {
        debug_printf (VERBOSE_ERR,"Error reading hilow. Asked: %ld Read: %d",bytes_a_leer,leidos);
        return 1;
    }
    */

    //Indicar que la tapa se ha abierto para hacer que recargue el sector de directorio
    hilow_tapa_action_was_opened();

    return 0;

}



int hilow_load_raw_device_file(void)
{


    if (!si_existe_archivo(hilow_file_name)) {
        debug_printf (VERBOSE_ERR,"Error opening HiLow RAW Data Drive file %s",hilow_file_name);
        return 1;
    }

    int bytes_a_leer=get_file_size(hilow_file_name);

    hilow_raw_device_buffer_total_size=bytes_a_leer/2;

    hilow_raw_device_buffer_side_a=util_malloc(hilow_raw_device_buffer_total_size,"Can not allocate memory for HiLow RAW Data Drive");
    hilow_raw_device_buffer_side_b=util_malloc(hilow_raw_device_buffer_total_size,"Can not allocate memory for HiLow RAW Data Drive");


    FILE *ptr_hilowfile;

    debug_printf (VERBOSE_INFO,"Opening HiLow RAW Data Drive File %s",hilow_file_name);
    ptr_hilowfile=fopen(hilow_file_name,"rb");


    if (ptr_hilowfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error opening HiLow RAW Data Drive file %s",hilow_file_name);
        return 1;
    }


    fread(hilow_raw_device_buffer_side_a,1,hilow_raw_device_buffer_total_size,ptr_hilowfile);
    fread(hilow_raw_device_buffer_side_b,1,hilow_raw_device_buffer_total_size,ptr_hilowfile);


    fclose(ptr_hilowfile);

    hilow_posicion_cabezal=0;
    hilow_raw_set_motor_off();


    //Indicar que la tapa se ha abierto para hacer que recargue el sector de directorio
    hilow_tapa_action_was_opened();

    return 0;

}

int hilow_load_device_file(void)
{

    if (!util_compare_file_extension(hilow_file_name,"ddh")) {
        hilow_rom_traps.v=1;
    }
    else {
        hilow_rom_traps.v=0;
    }


    if (hilow_rom_traps.v) {
        return hilow_load_ddh_device_file();
    }
    else {
        return hilow_load_raw_device_file();
    }
}

void hilow_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable HiLow on non Spectrum machine");
        return;
    }

	if (hilow_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"HiLow Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling HiLow interface");

	hilow_alloc_rom_ram_memory();

    hilow_alloc_device_memory();

    if (hilow_load_device_file()) return;


	if (hilow_load_rom()) return;

	hilow_set_peek_poke_functions();

	hilow_enabled.v=1;

	hilow_reset();

}

void hilow_disable(void)
{
	if (hilow_enabled.v==0) return;

	//Hacer flush si hay algun cambio
	hilow_flush_contents_to_disk();

    //Hacer flush si son imagenes raw
    if (hilow_rom_traps.v==0) {
        hilow_raw_flush_contents_to_disk();
        hilow_posicion_cabezal=0;
        hilow_raw_set_motor_off();
    }

	hilow_restore_peek_poke_functions();

	free(hilow_memory_pointer);

    free(hilow_device_buffer);

	hilow_enabled.v=0;
}



void hilow_reset(void)
{

    if (hilow_enabled.v==0) {
        return;
    }

	hilow_mapped_rom.v=0;
	hilow_mapped_ram.v=0;

}

//Funcion antes de las pruebas con puerto i/o real
void hilow_write_port_ff_ddh(z80_int port GCC_UNUSED,z80_byte value GCC_UNUSED)
{
    hilow_footer_operating();
/*
Escritura:

Parece que van controlados mediante valores de comando:

00H: ??
22H: ??
26H: leer sector??
28H: cancelacion?? fin de operacion?? pasar a modo "inicial"?

80H: Format?
A8H: ??


Bit de valor 08H tambien parece tener algo que ver
Puede que esos comandos sea combinacion de bits
*/
	//printf ("Writing hilow port %04XH value %02XH from PC=%04XH\n",port,value,reg_pc);
    last_hilow_port_value=value;
}

//Funcion antes de las pruebas con puerto i/o real
z80_byte hilow_read_port_ff_ddh(z80_int puerto GCC_UNUSED)
{



/*
Lectura:

Bit 7: ??
Bit 6: A 1 si grabador encendido
Bit 5: ??
Bit 4: ??
Bit 3: A 0 si se ha abierto la tapa en algun momento
Bit 2: A 1 si hay cinta insertada
Bit 1: ??
Bit 0: A 1 cuando esta listo para leer?


*/

	//printf ("Reading hilow port %04XH value from PC=%04XH\n",puerto,reg_pc);


    z80_byte valor_retorno=0;

    if (hilow_cinta_insertada_flag.v) valor_retorno |=4; //Hay cinta insertada

    if (hilow_tapa_has_been_opened.v==0) valor_retorno |=8; //No se ha abierto la tapa en algun momento

    valor_retorno |=64; //grabador encendido


    valor_retorno |=1; //listo

    return valor_retorno;


}

z80_int hilow_util_get_usage_counter(int sector_dir,z80_byte *p)
{
    p +=hilow_get_offset_sector(sector_dir,0);

    return value_8_to_16(p[1],p[0]);
}

z80_byte hilow_util_get_free_sectors(int sector_dir,z80_byte *p)
{
    p +=hilow_get_offset_sector(sector_dir,0);

    return p[0x3F3];
}

//Rellena una tabla de sectores[256] dice a 1 si el sector esta ocupado
void hilow_util_get_free_sectors_list(int sector_dir,z80_byte *puntero_memoria,int *sectores)
{
    int total_free_sectores=hilow_util_get_free_sectors(sector_dir,puntero_memoria);

    puntero_memoria +=hilow_get_offset_sector(sector_dir,0x3F5);



    int i;
    for (i=0;i<256;i++) {
       sectores[i]=0;
    }

    for (i=0;i<total_free_sectores;i++) {
        z80_byte s=*puntero_memoria;

        sectores[s]=1;

        puntero_memoria++;
    }

}

int hilow_util_get_file_offset(int indice_archivo)
{
    return indice_archivo*HILOW_DIRECTORY_ENTRY_SIZE+11;
}


int hilow_util_get_total_files(int sector_dir,z80_byte *puntero_memoria)
{

    puntero_memoria +=hilow_get_offset_sector(sector_dir,0);

    int i;

    //aunque el maximo es HILOW_MAX_FILES_DIRECTORY, obtenemos el maximo que diria el filesystem, puede que este corrupto
    for (i=0;i<=255;i++) {
        //Obtener archivos
        int offset_archivo=hilow_util_get_file_offset(i);

        z80_byte tipo_archivo=puntero_memoria[offset_archivo];

        if (tipo_archivo==255) {
            return i;
        }

    }

    return i;


}

int hilow_get_num_sectors_file(int sector_dir,z80_byte *puntero_memoria,int indice_archivo)
{

    puntero_memoria +=hilow_get_offset_sector(sector_dir,0);

    int offset_archivo=hilow_util_get_file_offset(indice_archivo);

    return puntero_memoria[offset_archivo+17];
}


int hilow_util_get_sectors_file(int sector_dir,int indice_archivo,z80_byte *puntero_memoria,int *sectores)
{
    puntero_memoria +=hilow_get_offset_sector(sector_dir,0);

    int offset_archivo=hilow_util_get_file_offset(indice_archivo);

    int i;

    z80_byte total_sectores=puntero_memoria[offset_archivo+17];
    if (total_sectores>HILOW_MAX_SECTORS_PER_FILE) total_sectores=HILOW_MAX_SECTORS_PER_FILE;

    int offset_inicio_sectores=offset_archivo+18;
    for (i=0;i<total_sectores;i++,offset_inicio_sectores++) {
        sectores[i]=puntero_memoria[offset_inicio_sectores];
    }

    return total_sectores;
}

void hilow_util_get_file_name(int sector_dir,z80_byte *puntero_memoria,int indice_archivo,char *nombre)
{
    puntero_memoria +=hilow_get_offset_sector(sector_dir,0);

    int offset_archivo=hilow_util_get_file_offset(indice_archivo);


    util_tape_get_info_tapeblock(&puntero_memoria[offset_archivo],0,19,nombre);


    //Eliminar posibles / del nombre
    int i;
    for (i=0;nombre[i];i++) {
            if (nombre[i]=='/') nombre[i]=' ';
    }


}

//Retorna el primer byte de la cabecera: 0 program, 3 bytes, etc
z80_byte hilow_util_get_file_type(int sector_dir,z80_byte *puntero_memoria,int indice_archivo)
{
    puntero_memoria +=hilow_get_offset_sector(sector_dir,0);

    int offset_archivo=hilow_util_get_file_offset(indice_archivo);


    return puntero_memoria[offset_archivo];

}

z80_int hilow_util_get_file_length(int sector_dir,z80_byte *puntero_memoria,int indice_archivo)
{

    //printf("hilow_util_get_file_length sector: %d puntero_memoria %p indice_archivo %d\n",sector_dir,puntero_memoria,indice_archivo);

    puntero_memoria +=hilow_get_offset_sector(sector_dir,0);

    int offset_archivo=hilow_util_get_file_offset(indice_archivo);


    return value_8_to_16(puntero_memoria[offset_archivo+12],puntero_memoria[offset_archivo+11]);


}

z80_byte hilow_util_get_sector_byte(int sector,z80_byte *puntero_memoria,int offset_origen)
{
    int offset=hilow_get_offset_sector(sector,offset_origen);

    if (offset>=HILOW_DEVICE_SIZE || offset<0) return 0;

    return puntero_memoria[offset];
}


void hilow_util_get_sector(int sector,z80_byte *puntero_memoria,z80_byte *destino,int total_leer)
{

    int i;

    for (i=0;i<total_leer;i++) {
        z80_byte byte_leido=hilow_util_get_sector_byte(sector,puntero_memoria,i);
        *destino=byte_leido;
        destino++;
    }
}

void hilow_util_get_file_contents(int sector_dir,z80_byte *puntero_memoria,int indice_archivo,z80_byte *destino_memoria)
{

    //int offset_archivo=hilow_util_get_file_offset(indice_archivo);

    z80_int longitud=hilow_util_get_file_length(sector_dir,puntero_memoria,indice_archivo);


    int sectores[HILOW_MAX_SECTORS_PER_FILE];

    hilow_util_get_sectors_file(sector_dir,indice_archivo,puntero_memoria,sectores);

    int indice_sector=0;

    //printf("Extracting %d bytes\n",longitud);

    while (longitud>0) {
        int leer=longitud;
        if (leer>HILOW_SECTOR_SIZE) leer=HILOW_SECTOR_SIZE;

        int sector_leer=sectores[indice_sector++];

        //printf("Reading sector %d\n",sector_leer);

        hilow_util_get_sector(sector_leer,puntero_memoria,destino_memoria,leer);

        destino_memoria +=leer;

        longitud -=leer;
    }

}



//
// Inicio gestionar hilow en formato raw
//



/*
Info from Jane McKay

OUT FFh
Bit 7 - Reset the Cassette Change bit (1 = reset, 0 = do nothing) (writing a 1 here will set bit 3 of IN FFh back to a 1)
Bit 6 - (Not used?)
Bit 5 - Motor On (1 = On, 0 = Stop)
Bit 4 - Write Gate (1 = Write Enabled, 0 = Write Disabled)
Bit 3 - Fast (1 = Fast, 0 = Slow)
Bit 2 - Track Select (1 = Side 1, 0 = Side 2)
Bit 1 - Forward (1 = Forward, 0 = Reverse)
Bit 0 - Data Bit Out (saving)

IN FFh
Bit 7 - (Not used?)
Bit 6 - Data Bit In (loading)
Bit 5 - (Not used?)
Bit 4 - (Not used?)
Bit 3 - Cassette Change (0 = Cassette Changed, 1 = Not Changed) (it is "changed" if the Cassette Sense bit has ever gone low)
Bit 2 - Cassette Sense (1 = Cassette in place, 0 = No cassette)
Bit 1 - Reverse (1 = Reverse, 0 = Forward) (Inverted last bit written to bit 1 of OUT FFh, used in "Start the tape" routine)
Bit 0 - Cassette Motion (0 = Moving, 1 = Stopped)

Note: "Data Bit In" is always 1 when Cassette is "stopped"
Note: Bit 1 is confusing, it seems to be the last bit written to bit 1, but inverted. I have no idea why, but the "Start the tape"
routine doesn't work properly without it.
Note: Cassette Motion can register as "stopped" when the motor is off, or when the start or end of the tape has been reached.
It is always a 1 when the drive is switched off.
*/



void hilow_mix_audio(void)
{
    if (hilow_hear_load_sound.v==0 && hilow_hear_save_sound.v==0) return;

    if (hilow_rom_traps.v) return;

    //Si esta motor en marcha
    if ((last_hilow_port_value & 0x20)==0) return;

    int sonido_hilow=hilow_ultimo_sample_sonido;
    sonido_hilow -=128;


    reset_silence_detection_counter();
    audio_valor_enviar_sonido_izquierdo /=2;
    audio_valor_enviar_sonido_izquierdo += sonido_hilow/2;

    audio_valor_enviar_sonido_derecho /=2;
    audio_valor_enviar_sonido_derecho += sonido_hilow/2;



}



void hilow_write_port_ff_raw(z80_int port GCC_UNUSED,z80_byte value)
{
    hilow_footer_operating();

/*
OUT FFh
Bit 7 - Reset the Cassette Change bit (1 = reset, 0 = do nothing) (writing a 1 here will set bit 3 of IN FFh back to a 1)
Bit 6 - (Not used?)
Bit 5 - Motor On (1 = On, 0 = Stop)
Bit 4 - Write Gate (1 = Write Enabled, 0 = Write Disabled)
Bit 3 - Fast (1 = Fast, 0 = Slow)
Bit 2 - Track Select (1 = Side 1, 0 = Side 2)
Bit 1 - Forward (1 = Forward, 0 = Reverse)
Bit 0 - Data Bit Out (saving)
*/

    last_hilow_port_value=value;

    if (value & 0x80) hilow_tapa_has_been_opened.v=0;

    if (value & 0x20) {
        //hilow_cinta_en_movimiento=1;
    }
    else {
        hilow_cinta_en_movimiento=0;
    }
}




z80_byte *hilow_get_audio_buffer(void)
{
    //Bit 2 - Track Select (1 = Side 1, 0 = Side 2)
    if (last_hilow_port_value & 0x04) return hilow_raw_device_buffer_side_a;
    else return hilow_raw_device_buffer_side_b;
}

z80_bit hilow_diffencial_algorithm_enabled={0};
int hilow_diffencial_algorithm_last_bit=0;
z80_bit hilow_invert_bit={0};
int hilow_diffencial_algorithm_volume_range=10;



//z80_byte anterior_last_raw_audio_data_read;
//int last_raw_audio_data_read_previous_return_value;

z80_byte hilow_read_port_ff_raw(z80_int puerto GCC_UNUSED)
{

/*
Lectura:


IN FFh
Bit 7 - (Not used?)
Bit 6 - Data Bit In (loading)
Bit 5 - (Not used?)
Bit 4 - (Not used?)

Bit 3 - Cassette Change (0 = Cassette Changed, 1 = Not Changed) (it is "changed" if the Cassette Sense bit has ever gone low).
O sea, A 0 si se ha abierto la tapa en algun momento

Bit 2 - Cassette Sense (1 = Cassette in place, 0 = No cassette)
Bit 1 - Reverse (1 = Reverse, 0 = Forward) (Inverted last bit written to bit 1 of OUT FFh, used in "Start the tape" routine)
Bit 0 - Cassette Motion (0 = Moving, 1 = Stopped)

Note: "Data Bit In" is always 1 when Cassette is "stopped"
Note: Bit 1 is confusing, it seems to be the last bit written to bit 1, but inverted. I have no idea why, but the "Start the tape" routine doesn't work properly without it.
Note: Cassette Motion can register as "stopped" when the motor is off, or when the start or end of the tape has been reached. It is always a 1 when the drive is switched off.

*/

	//printf ("Reading hilow port %04XH value from PC=%04XH\n",puerto,reg_pc);


    z80_byte valor_retorno=0;

    if (hilow_cinta_insertada_flag.v) valor_retorno |=4; //Hay cinta insertada

    if (hilow_tapa_has_been_opened.v==0) valor_retorno |=8; //No se ha abierto la tapa en algun momento


    if (last_raw_audio_data_read>128) {
        //printf("1\n");
        valor_retorno |=64;
    }
    else {
        //printf("0\n");
    }


    //Algoritmo basado en que si es algo mayor que 128, sera un 1.
    //Si es algo menor que 128 sera un 0
    //Y si esta cerca de 128, sera el mismo valor anterior
    //Esto deberia dar mejor resultado con señal proveniente de una cinta real, pues hay variaciones pequeñas que no queremos que altere el bit
    if (hilow_diffencial_algorithm_enabled.v) {
        int vol=hilow_diffencial_algorithm_volume_range;
        if (last_raw_audio_data_read>128+vol) hilow_diffencial_algorithm_last_bit=1;
        else if (last_raw_audio_data_read<128-vol) hilow_diffencial_algorithm_last_bit=0;
        else {
            //sin cambio en el bit
        }

        if (hilow_diffencial_algorithm_last_bit) valor_retorno |=64;
        else valor_retorno &=(255-64);

    }


    //Otro algoritmo. Mirando si la cresta del valor sube o baja, teniendo en cuenta un margen donde no se cambia el valor anterior
    /*
    if (1) {


        //realmente no hace falta inicializarlo a 0 pues siempre retornara valor,
        //solo es para el compilador para que no se queje
        char return_value=0;

        //sacar diferencia valor anterior con actual
        int diferencia=last_raw_audio_data_read-anterior_last_raw_audio_data_read;

        if (diferencia<0) diferencia=-diferencia;

        //Si la onda esta mas o menos igual, damos valor anterior
        if (diferencia<=hilow_diffencial_algorithm_volume_range) {
            //printf("igual\n");
            return_value=last_raw_audio_data_read_previous_return_value;
        }

        else {

            //Si la onda "sube", es +1
            if (last_raw_audio_data_read>anterior_last_raw_audio_data_read) {
                //printf ("superior\n");
                return_value=1;
            }
            //Si la onda "baja", es -1
            else if (last_raw_audio_data_read<anterior_last_raw_audio_data_read) {
                return_value=0;
                //printf("inferior\n");
            }


        }

        anterior_last_raw_audio_data_read=last_raw_audio_data_read;
        last_raw_audio_data_read_previous_return_value=return_value;

        //printf("retornar %d\n",return_value);

        if (return_value) valor_retorno |=64;
        else valor_retorno &=(255-64);

    }
    //Fin //Otro algoritmo
    */





    if (hilow_invert_bit.v) valor_retorno ^=64;


    //Parece que bit 6 tambien se activa cuando cinta detenida
    if (!hilow_cinta_en_movimiento) {
        valor_retorno |=1;
        valor_retorno |=64;
    }

    /*
    * Hilow simular reproductor encendido o no:
        -si encendido y motor detenido, lectura bit de audio siempre es 1
        -si apagado y motor detenido, lectura bit de audio siempre es 0
    */

    if (hilow_reproductor_encendido.v==0) valor_retorno &=(255-64);

    //Bit 1 - Reverse (1 = Reverse, 0 = Forward) (Inverted last bit written to bit 1 of OUT FFh, used in "Start the tape" routine)
    if ((last_hilow_port_value & 0x02)==0) valor_retorno |=0x02;

    return valor_retorno;


}

void hilow_raw_set_motor_off(void)
{
    //Bit 5 - Motor On (1 = On, 0 = Stop)
    last_hilow_port_value &=(255-HILOW_PORT_MASK_MOTOR_ON);

    hilow_cinta_en_movimiento=0;

}

void hilow_raw_power_off_player(void)
{
    hilow_reproductor_encendido.v=0;

    //Motor off y cinta en movimiento off
    hilow_raw_set_motor_off();
}

void hilow_raw_power_on_player(void)
{
    hilow_reproductor_encendido.v=1;

    //Motor off y cinta en movimiento off
    hilow_raw_set_motor_off();
}

z80_byte hilow_read_port_ff(z80_int puerto)
{
    hilow_footer_operating();

    z80_byte value;

    if (hilow_rom_traps.v) value=hilow_read_port_ff_ddh(puerto);
    else value=hilow_read_port_ff_raw(puerto);

    return value;
}


void hilow_write_port_ff(z80_int port,z80_byte value)
{
    if (hilow_rom_traps.v) hilow_write_port_ff_ddh(port,value);
    else return hilow_write_port_ff_raw(port,value);
}

//Usado en visual tape
int hilow_visual_rodillo_arrastre_grados=0;

int hilow_visual_slow_movement=0;

int hilow_timer_events_counter=0;

//Eventos de timer
//Para ver si cinta en extremo, para mover rodillos de arrastre
void hilow_timer_events(void)
{
    if (hilow_rom_traps.v) return;

    //ver si esta motor on y no esta atascado (o sea ver variable movimiento)
    if (hilow_cinta_en_movimiento) {
        //Mover rodillo de arrastre para visual tape
        //en 1 segundo avanza 360 grados, en velocidad normal de cinta 1x
        //O sea 360/50=7 grados cada frame de video
        int incremento_grados=7;


        //10x o 40x
        if (last_hilow_port_value & HILOW_PORT_MASK_FAST) incremento_grados *=40;
        else incremento_grados *=10;

        if (hilow_visual_slow_movement) incremento_grados=1;

        //Adelante o atras
        if ((last_hilow_port_value & HILOW_PORT_MASK_FORWARD)==0) incremento_grados=-incremento_grados;

        hilow_visual_rodillo_arrastre_grados +=incremento_grados;
        hilow_visual_rodillo_arrastre_grados=(hilow_visual_rodillo_arrastre_grados % 360);
    }


    //Lo siguiente una vez por segundo
    hilow_timer_events_counter+=20;

    if (hilow_timer_events_counter!=1000) return;

    hilow_timer_events_counter=0;

    if (!hilow_raw_cinta_en_extremo) return;

    //Si motor en marcha
    if ((last_hilow_port_value & HILOW_PORT_MASK_MOTOR_ON)==0) return;

    if (hilow_raw_contador_tiempo_llegado_extremo_cinta==HILOW_RAW_TIEMPO_EXTREMO) {
        //detener motor
        //printf("Deteniendo motor\n");

        hilow_raw_set_motor_off();
    }

    else {
        hilow_raw_contador_tiempo_llegado_extremo_cinta++;
    }

}

//Si esta al principio o final de cinta que no se puede escribir (esa parte en blanco sin zona magnetica)
int hilow_raw_en_zona_blanca(int posicion)
{
    int espacio_samples=HILOW_RAW_SAMPLE_FREQ*HILOW_RAW_ZONA_BLANCA_SEC;
    if (posicion<espacio_samples || posicion>hilow_raw_device_buffer_total_size-espacio_samples) return 1;
    else return 0;
}

void hilow_raw_write_byte(int posicion,z80_byte valor)
{
    if (hilow_raw_en_zona_blanca(posicion)) return;

    z80_byte *puntero_audio=hilow_get_audio_buffer();

    puntero_audio[posicion]=valor;
}

z80_byte hilow_raw_read_byte(int posicion)
{
    if (hilow_raw_en_zona_blanca(posicion)) return 0;

    z80_byte *puntero_audio=hilow_get_audio_buffer();

    return puntero_audio[posicion];
}

void hilow_raw_move(void)
{
    //printf("Mover posicion hilow en t_estados %d\n",t_estados);

    //Direccion
    int direccion=+1;

    //Bit 1 - Forward (1 = Forward, 0 = Reverse)
    if ((last_hilow_port_value & 0x02)==0) {
        //printf("moviendose hacia atras\n");
        direccion=-1;
    }
    else {
        //printf("moviendose hacia adelante\n");
    }

    //asumimos
    hilow_cinta_en_movimiento=1;


    if (direccion>=0) {

        if (hilow_posicion_cabezal==hilow_raw_device_buffer_total_size-1) {
            //printf("llegado al final\n");
            hilow_cinta_en_movimiento=0;
            hilow_raw_cinta_en_extremo=1;
        }

        else {
            hilow_posicion_cabezal++;

            hilow_raw_cinta_en_extremo=0;
            hilow_raw_contador_tiempo_llegado_extremo_cinta=0;

        }

    }
    else {
        if (hilow_posicion_cabezal==0) {
            //printf("llegado al principio\n");
            hilow_cinta_en_movimiento=0;
            hilow_raw_cinta_en_extremo=1;
        }
        else {
            hilow_posicion_cabezal--;

            hilow_raw_cinta_en_extremo=0;
            hilow_raw_contador_tiempo_llegado_extremo_cinta=0;
        }
    }



    //Si escribir o leer
    //Bit 4 - Write Gate (1 = Write Enabled, 0 = Write Disabled)
    if (last_hilow_port_value & 0x10) {
        //printf("Escribiendo\n");
        //f0 y 10 para hacer igual que emulador x128
        z80_byte valor_escribir=0x10;
        //Bit 0 - Data Bit Out (saving)
        if (last_hilow_port_value & 1) valor_escribir=0xf0;

        if (hilow_write_protection.v==0) {
            hilow_raw_write_byte(hilow_posicion_cabezal,valor_escribir);
            hilow_must_flush_to_disk=1;
        }

        if (hilow_hear_save_sound.v) hilow_ultimo_sample_sonido=valor_escribir;
    }


    else {
        //leyendo
        last_raw_audio_data_read=hilow_raw_read_byte(hilow_posicion_cabezal);
        if (hilow_hear_load_sound.v) hilow_ultimo_sample_sonido=last_raw_audio_data_read;
    }

    if (hilow_posicion_cabezal%5000 ==1) {

        if (last_hilow_port_value & 0x08) {
            //printf("rapido cara %c pos %d\n",(last_hilow_port_value & 0x04 ? 'A' : 'B'),hilow_posicion_cabezal);
        }
        else {
            //printf("lento cara %c pos %d\n",(last_hilow_port_value & 0x04 ? 'A' : 'B'),hilow_posicion_cabezal);
        }
    }



}

void hilow_count_tstates(void)
{

    //Si reproductor apagado, no hacer nada
    if (hilow_reproductor_encendido.v==0) return;

    //Si motor detenido, no hacer nada
    //Bit 5 - Motor On (1 = On, 0 = Stop)
    if ((last_hilow_port_value & 0x20)==0) return;
    //printf("Motor moviendose en t_estados %d\n",t_estados);


    int delta_testados;

    //Cada x t-estados, avanzar un byte
    if (t_estados>=hilow_estados_anterior) {
        delta_testados=t_estados-hilow_estados_anterior;
    }

    else {
        //ha dado la vuelta
        //printf("ha dado la vuelta. t_estados=%d hilow_estados_anterior=%d\n",t_estados,hilow_estados_anterior);
        delta_testados=screen_testados_total-hilow_estados_anterior;
        //printf("parcial delta: %5d t_estados: %d screen_testados_total: %d\n",delta_testados,t_estados,screen_testados_total);
        delta_testados +=t_estados;
    }

    int velocidad=speed_hilow_normal;

    //Bit 3 - Fast (1 = Fast, 0 = Slow)
    if (last_hilow_port_value & 0x08) {
        velocidad=speed_hilow_rapido;
        //printf("rapido\n");
    }
    else {
        //printf("lento\n");
    }

    //if (t_estados<100 || t_estados>70000) printf("delta: %5d t_estados: %d\n",delta_testados,t_estados);

    hilow_transcurrido_desde_ultimo_movimiento +=delta_testados;

    while (hilow_transcurrido_desde_ultimo_movimiento>=velocidad) {
        hilow_transcurrido_desde_ultimo_movimiento -=velocidad;
        //printf("mover cabezal en testados: %d testados_parc: %d hilow_transcurrido_desde_ultimo_movimiento: %d screen_testados_total: %d\n",
        //    t_estados,debug_t_estados_parcial,hilow_transcurrido_desde_ultimo_movimiento,screen_testados_total);
        hilow_raw_move();
    }



    hilow_estados_anterior=t_estados;
}

//Retorna el total de minutos, de ambas caras
int hilow_raw_get_minutes_tape(void)
{
    long long int minutos_total_cinta_long=(hilow_raw_device_buffer_total_size*2L)/HILOW_RAW_SAMPLE_FREQ/60;
    int minutos_total_cinta=minutos_total_cinta_long;

    return minutos_total_cinta;
}

int hilow_raw_transcurrido_cinta_porc(void)
{
    //de 64 bits porque al multiplicar por 100 se puede salir de rango
    long long int porcentaje_long=0;

    if (hilow_raw_device_buffer_total_size!=0) {
        porcentaje_long=(hilow_posicion_cabezal*100L)/hilow_raw_device_buffer_total_size;
    }

    int porcentaje=porcentaje_long;

    return porcentaje;

}

//
// Fin gestionar hilow en formato raw
//