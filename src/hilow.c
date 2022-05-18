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

//#if defined(__APPLE__)
//        #include <sys/syslimits.h>
//#endif


#include "hilow.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"
#include "screen.h"
#include "menu_items.h"
#include "mem128.h"


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


//Esto de momento se puede conmutar pero luego ira asociado a una cinta real
z80_bit hilow_cinta_insertada={1};

z80_bit hilow_tapa_has_been_opened={0};

int hilow_must_flush_to_disk=0;


//Si cambios en escritura se hace flush a disco
z80_bit hilow_persistent_writes={1};


z80_bit hilow_write_protection={0};

void hilow_flush_contents_to_disk(void)
{

    if (hilow_enabled.v==0) return;

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
    long int size;
    size=HILOW_DEVICE_SIZE;



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
        printf("Enabling hilow memory from nmi triggered\n");
        hilow_mapped_rom.v=1;
        hilow_mapped_ram.v=1;
    }   
}

void hilow_footer_operating(void)
{
    generic_footertext_print_operating("HILOW");

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


//para guardar la imagen del datadrive
z80_byte *hilow_device_buffer=NULL;

int hilow_write_byte_device(int sector,int offset,z80_byte valor)
{
    hilow_footer_operating();

    //autocerrar tapa 
    hilow_tapa_reset_was_opened();

    if (hilow_write_protection.v) return 0;

    offset +=(sector*HILOW_SECTOR_SIZE);

    //if (sector==255) printf("Sector 255 offset: %d max: %d\n",offset,HILOW_DEVICE_SIZE);

    if (offset>=HILOW_DEVICE_SIZE) {
        debug_printf (VERBOSE_DEBUG,"Error. Trying to write beyond HiLow Data Drive. Size: %ld Asked sector: %d Offset: %d",
                        HILOW_DEVICE_SIZE,sector,offset);

        //no desactivamos porque esto implica quitar funciones de peek/poke anidadas del core y petaria
		//hilow_disable();
		return 1;
	}    

    hilow_device_buffer[offset]=valor;

    hilow_must_flush_to_disk=1;

    return 0;
}

z80_byte hilow_read_byte_device(int sector,int offset)
{
    hilow_footer_operating();

    //autocerrar tapa 
    hilow_tapa_reset_was_opened();

    offset +=(sector*HILOW_SECTOR_SIZE);

    if (offset>=HILOW_DEVICE_SIZE) {
        debug_printf (VERBOSE_DEBUG,"Error. Trying to read beyond HiLow Data Drive. Size: %ld Asked sector: %d Offset: %d",
                        HILOW_DEVICE_SIZE,sector,offset);
        //no desactivamos porque esto implica quitar funciones de peek/poke anidadas del core y petaria
		//hilow_disable();
		return 0;
	}        

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


void temp_chapuza_espacio_disponible(z80_int inicio_datos)
{

    poke_byte_no_time(inicio_datos+1011,HILOW_MAX_SECTORS-1);
    //no tengo claro este segundo byte para que sirve. si no lo escribo, le pone un 0
    poke_byte_no_time(inicio_datos+1012,HILOW_MAX_SECTORS-1);

                                 
}

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

int hilow_write_mem_to_device(z80_int dir,int sector,int longitud,int offset_destination)
{
    int i;

    printf("Writing memory to hilow device. dir=%04XH sector=%d length=%04XH offset_destination=%04XH\n",
        dir,sector,longitud,offset_destination);

    if (sector>=HILOW_MAX_SECTORS) {
        //printf("Sector beyond maximum. Do nothing\n");

        debug_printf (VERBOSE_DEBUG,"Error. Trying to write beyond max HiLow Data Drive sectors. Size: %ld Asked sector: %d Offset: %d",
                        HILOW_DEVICE_SIZE,sector,offset_destination);

        //no desactivamos porque esto implica quitar funciones de peek/poke anidadas del core y petaria
		//hilow_disable();

        return 1;
    }

    for (i=0;i<longitud;i++) {
        z80_byte c=peek_byte_no_time(dir+i);
        if (hilow_write_byte_device(sector,i+offset_destination,c)) {
            return 1;
        }
    }  

    return 0;      
}

void hilow_create_sector_table(void)
{
    /*
    Empieza en dirección 400h del sector 0 (esto es 1024-mitad de sector)
    Lo metemos tanto en el dispositivo como en RAM
    */

    printf("Creating free sectors table\n");
    int i;

    int id_sector_tabla;

    int offset=0x400;

    for (id_sector_tabla=1;id_sector_tabla<HILOW_MAX_SECTORS;id_sector_tabla++,offset++) {
        hilow_write_byte_device(0,offset,id_sector_tabla);
        poke_byte_no_time(8192+offset,id_sector_tabla);
    }

}


void hilow_trap_write_verify(void)
{
    printf("VERIFY or WRITE probably\n");


    //printf("Retornando porque no carry. Posible escritura?\n\n");
    temp_debug_registers();
    temp_dump_from_addr(reg_ix);

    z80_byte retorno_error=0;

    if (!(Z80_FLAGS & FLAG_Z)) {
        printf("VERIFY probably\n");
        //No hacer nada y retornar todo ok
    }   

    else {    

        printf("WRITE probably\n");  

        z80_int dir_inicio=reg_ix;   
        z80_int longitud=reg_de;  
        z80_byte sector=reg_a;  

        //if (reg_de>HILOW_SECTOR_SIZE) {
        if (sector==0) {
            //printf("WARN. DE > %d. Writing only to maximum\n",HILOW_SECTOR_SIZE);
            //printf("WARN. DE > %d. Probably dir entry\n",HILOW_SECTOR_SIZE);
            printf("Writing from cache memory to sector 0\n");
            //hilow_write_mem_to_device(reg_ix,reg_a,17,11);
            //Y meter valor a 1 despues... esto con 1 archivo, que sucede con 2??
            //hilow_write_byte_device(0,17+11,1);


            //directamente copiar lo de la cache hacia aqui
            //esto soluciona la escritura
            //hilow_write_mem_to_device(8192,0,HILOW_SECTOR_SIZE,0);
            dir_inicio=8192;
            longitud=HILOW_SECTOR_SIZE;
        }
        //else {
        //    hilow_write_mem_to_device(dir_inicio,sector,longitud,0);
        //}

        if (hilow_write_mem_to_device(dir_inicio,sector,longitud,0)) {
            //Error al escribir, sector mas alla del rango
            retorno_error=1; //Error en la cinta
        }

    }

    //Retorno de verify o write
    
    reg_a=retorno_error;

    //reg_a++;

    //valor distinto de 0 retorna el error "Error en la cinta"
    //reg_a=1;

    

    //No seguro de esto
    reg_ix +=reg_de;

    //siguiente sector???
    //reg_bc++;

    //siguiente sector??
    reg_a_shadow++;

    reg_pc=pop_valor();  

    printf("Returning from WRITE/VERIFY SECTOR to address %04XH\n",reg_pc);       
}

void hilow_trap_read(void)
{

    int i;
    temp_debug_mem_registers();   

    printf("READ probably\n");

    z80_int inicio_datos;
    z80_int leer_datos;

    //int offset_device=0;

    if (reg_a==0) {
        //sector 0
        inicio_datos=8192;
        leer_datos=HILOW_SECTOR_SIZE;                    
    }

    else {
        inicio_datos=reg_ix;
        leer_datos=reg_de;
    }


    /*else {
        //Sector 0
        //leer_datos=100; //al azar
        if (reg_de==17) {
            
            //lectura entrada directorio??
            printf("Leyendo entrada directorio\n");

            //offset_device=0;

            //descomentando el leer_datos, intenta leer pero con direccion y longitud incorrectas y peta
            //si lo dejo comentado, suele quedarse en un bucle cerrado o dar tape loading error
            //leer_datos=500; //80 //11+45; //17; //valor al azar

            //si subo algo mas de 17, suele resetearse
            //leer_datos=100;

            //si dejo en 17, carga solo si la grabacion ha sido justo ahora (porque probablemente lo tenga en cache)
            leer_datos=17;

            //leer_datos=1;
            //leer_datos=45+11;
            //inicio_datos=reg_ix;

            //offset_device=11;

            //despues_directorio=1;
        }
    }*/




    z80_byte retorno_error=0;

    //no estoy seguro de esto
    if (leer_datos>HILOW_SECTOR_SIZE) leer_datos=HILOW_SECTOR_SIZE;

    //no estoy seguro de esto
    if (leer_datos==0) leer_datos=HILOW_SECTOR_SIZE;

    int sector=reg_a;

    printf("Reading data from sector %d length %04XH to address %04XH\n",sector,leer_datos,inicio_datos);


    if (sector>=HILOW_MAX_SECTORS) {
        //printf("Sector beyond maximum. Do nothing\n");

        debug_printf (VERBOSE_DEBUG,"Error. Trying to read beyond max HiLow Data Drive sectors. Size: %ld Asked sector: %d",
                HILOW_DEVICE_SIZE,sector);                
        retorno_error=1;
    }
    else {
        for (i=0;i<leer_datos;i++) {
            poke_byte_no_time(inicio_datos+i,hilow_read_byte_device(sector,i/*+offset_device*/));


        }    
    }



    //no error?
    //Z80_FLAGS=(Z80_FLAGS & (255-FLAG_C));
    //Z80_FLAGS |=FLAG_C;
    reg_a=retorno_error;


    //valor distinto de 0 retorna el error "Error en la cinta"

    reg_pc=pop_valor();


    //No seguro de esto
    reg_ix +=reg_de;

    //siguiente sector???
    //reg_bc++;


    printf("Returning from READ_SECTOR to address %04XH\n",reg_pc);

}

z80_byte cpu_core_loop_spectrum_hilow(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

        //Llamar a anterior
        debug_nested_core_call_previous(hilow_nested_id_core);


		hilow_automap_unmap_memory(reg_pc);

        if (hilow_mapped_rom.v==0) {
            //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
            return 0;
        }


        //debug de rutinas
        if (reg_pc==0x186D) {
            
            printf("\nEntering READ_WRITE_VERIFY_SECTOR. PC=%04XH return=%04XH A=%02XH Carry=%d Z=%d IX=%04XH DE=%04XH HL=%04XH BC=%04XH SP=%04XH\n",
                reg_pc,peek_word(reg_sp),reg_a,Z80_FLAGS & FLAG_C,Z80_FLAGS & FLAG_Z,reg_ix,reg_de,reg_hl,reg_bc,reg_sp);


            //printf("(3F31)=%04XH\n",peek_word(0x3f31));
            printf("Sector: %d\n",reg_a);

            
            if (!(Z80_FLAGS & FLAG_C)) {
                hilow_trap_write_verify();      
            }

            //No carry. Read
            else {         

                hilow_trap_read();
            }
        }

        //debug de rutinas
        if (reg_pc==0x16D0) {
            
            printf("\nEntering FORMAT_SECTOR. A=%02XH IX=%04XH DE=%04XH SP=%04XH\n",reg_a,reg_ix,reg_de,reg_sp);

            /*
            ; IX=inicio datos??  (quiza siempre direccion 8192)
            ; DE=longitud datos?? (quiza siempre escribe tamaño de HILOW_SECTOR_SIZE)
            ; A= sector
            ???
            */

           temp_debug_mem_registers();

            //mostrar algunos caracteres
            int i;
            for (i=0;i<2048;i++) {
                z80_byte c=peek_byte_no_time(8192+i);
                if (c>=32 && c<=126) printf("%c",c);
                else printf(" %02X ",c);
            }
            printf("\n");           

            
            int sector=reg_a;

            //sector 1=0??
            //Sin esto, al hacer un cat, no aparece el label de la cinta
            sector--;
            //if (sector==1) sector=0;

            if (sector==0) {
                temp_chapuza_espacio_disponible(8192);
            }
       

            hilow_write_mem_to_device(8192,sector,HILOW_SECTOR_SIZE,0);    


            hilow_create_sector_table();       

            //no error?
            reg_a=0;

            reg_pc=pop_valor();
            printf("Returning from WRITE_SECTOR to address %04XH\n",reg_pc);


        }        

        if (reg_pc==0x1A9E) {
            
            printf("\nEntering POST_FORMAT. A=%02XH IX=%04XH DE=%04XH\n",reg_a,reg_ix,reg_de);

            //saltar adelante en codigo. feo....
            //reg_pc=0x1ad8;
            reg_pc=0x1ac8;
            reg_pc=0x1acf;

            printf("Skipping to address %04XH\n",reg_pc);
        }              

        if (reg_pc==0x1AC0) {
            
            printf("\nEntering POST_FORMAT2. A=%02XH IX=%04XH DE=%04XH\n",reg_a,reg_ix,reg_de);

            //saltar adelante en codigo. feo....
            //reg_pc=0x1ad8;
            reg_pc=0x1ac8;
            reg_pc=0x1acf;

            printf("Skipping to address %04XH\n",reg_pc);
        }             

        if (reg_pc==0x1AF1) {
            
            printf("\nEntering POST_FORMAT3. A=%02XH IX=%04XH DE=%04XH\n",reg_a,reg_ix,reg_de);

            //engañar... para saltar una condicion que hace cancelar el bucle de sectores 1,2,3,...
            //Z80_FLAGS |=FLAG_Z;
        }


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


}

void hilow_alloc_device_memory(void)
{

    //z80_byte hilow_device_buffer[HILOW_DEVICE_SIZE];
    
    int size=HILOW_DEVICE_SIZE;  

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for hilow device emulation",size/1024);

    printf ("Allocating %d kb of memory for hilow device emulation\n",size/1024);

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

    ptr_hilow_romfile=fopen(HILOW_ROM_FILE_NAME,"rb");
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

char hilow_file_name[PATH_MAX]="";

int hilow_load_device_file(void)
{
    if (hilow_device_buffer==NULL) {
        debug_printf(VERBOSE_ERR,"HiLow is not enabled");
        return 1;
    }

    FILE *ptr_hilowfile;
    unsigned int leidos=0;

    debug_printf (VERBOSE_INFO,"Opening HiLow Data Drive File %s",hilow_file_name);
    ptr_hilowfile=fopen(hilow_file_name,"rb");


    unsigned int bytes_a_leer=HILOW_DEVICE_SIZE;


    if (ptr_hilowfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error opening HiLow Data Drive file %s",hilow_file_name);
        return 1;
    }

    leidos=fread(hilow_device_buffer,1,bytes_a_leer,ptr_hilowfile);
    fclose(ptr_hilowfile);



    //De momento no comprobamos tamaño leido, por si en el futuro cambiamos el formato
    /*
    if (leidos!=bytes_a_leer) {
        debug_printf (VERBOSE_ERR,"Error reading hilow. Asked: %ld Read: %d",bytes_a_leer,leidos);
        return 1;
    }
    */

    return 0;

}

void hilow_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable hilow on non Spectrum machine");
        return;
    }

	if (hilow_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"Already enabled");
		return;
	}


	hilow_alloc_rom_ram_memory();

    hilow_alloc_device_memory();

    if (hilow_load_device_file()) return;


	if (hilow_load_rom()) return;

	hilow_set_peek_poke_functions();

	hilow_enabled.v=1;

    //por alguna razon, el cat inicial necesita que este abierta la tapa
    //luego se autocierra en lectura o escritura
    //quiza indica mejor si esta a 0 -> cinta cambiada. y a 1- >cinta no cambiaa
    //quien indica que ya no ha sido abierta?
    hilow_tapa_action_was_opened();

	hilow_reset();




}

void hilow_disable(void)
{
	if (hilow_enabled.v==0) return;

	//Hacer flush si hay algun cambio
	hilow_flush_contents_to_disk();

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


void hilow_write_port_ff(z80_int port,z80_byte value)
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
	printf ("Writing hilow port %04XH value %02XH from PC=%04XH\n",port,value,reg_pc);
}


z80_byte hilow_read_port_ff(z80_int puerto)
{

    hilow_footer_operating();

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

L1C03:          IN      A,(HLWPORT)
                BIT     0,A
                RET     NZ
                DJNZ    L1C03
                RET


*/

	printf ("Reading hilow port %04XH value from PC=%04XH\n",puerto,reg_pc);


    z80_byte valor_retorno=0;

    if (hilow_cinta_insertada.v) valor_retorno |=4; //Hay cinta insertada

    if (hilow_tapa_has_been_opened.v==0) valor_retorno |=8; //No se ha abierto la tapa en algun momento

    valor_retorno |=64; //grabador encendido


    valor_retorno |=1; //listo

    return valor_retorno; 


}