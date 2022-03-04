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

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "superupgrade.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "contend.h"
#include "mem128.h"
#include "menu.h"
#include "screen.h"


z80_bit superupgrade_enabled={0};

char superupgrade_rom_file_name[PATH_MAX]="";

z80_byte *superupgrade_rom_memory_pointer;
z80_byte *superupgrade_ram_memory_pointer;

z80_byte *superupgrade_rom_memory_table[32];
z80_byte *superupgrade_ram_memory_table[32];

z80_byte *superupgrade_memory_paged[4];

z80_byte superupgrade_puerto_43b;

int superupgrade_nested_id_peek_byte;
int superupgrade_nested_id_peek_byte_no_time;
int superupgrade_nested_id_poke_byte;
int superupgrade_nested_id_poke_byte_no_time;


//Si esta a 1, la flash esta protegida a escritura y no se puede escribir

z80_bit superupgrade_flash_write_protected;


//Superupgrade buffer de escritura a flash
//Se escribe tanto los bytes de escritura de comando como los de escritura de bytes
z80_byte superupgrade_write_buffer[256];

//Indice a byte que se esta escribiendo en buffer de escritura
int superupgrade_write_buffer_index;



/*Estado superupgrade en escritura:
0=Acabado de arrancar, o listo para recibir comandos o escrituras
1=Recibiendo comando (de desproteccion de chip o proteccion)
2=Recibiendo escritura de 256 bytes
*/

int superupgrade_write_status;


int superupgrade_flash_must_flush_to_disk=0;

//Si se ha ejecutado un comando de proteger la flash y por tanto se debe escribir los 256 bytes y luego proteger la flash
z80_bit superupgrade_pending_protect_flash={0};


//Aviso de operacion de flash en footer
//int superupgrade_flash_operating_counter=0;

/* Codificacion de puertos de paginacion en superupgrade

7FFD

D0 Bits 0..2 de pagina de RAM
D1 ""
D2 ""
D3 No usado
D4 Bit bajo de la seleccion de ROM
D5 Deshabilitar paginado
D6 Bit 3 de pagina de RAM
D7 Bit 4 de pagina de RAM . TODO ver si se deshabilita de alguna manera estos bits 3 y 4


1FFD
D2 Bit alto de la seleccion de ROM
D3 Motor del disco
Resto de bits no usados

43B
D0-D4: Seleccion de pagina de rom 0..31
D5: deshabilitar funcionalidad puerto 7ffd
D6: deshabilitar funcionalidad puerto 1ffd
D7: desactivacion del divide

SI D5=1 y D6=0, significa "mostrar rom interna" y con esa combinación, puerto 7ffd está habilitado

Nota:
Para calcular en qué posición de mi array de memoria ROM tengo la dirección 0 del espacio de direcciones únicamente tengo que consultar los 5 bits más bajos dela variable del puerto 4ffd, sumarle la página de rom  determinada por los puertos 1ffd/7ffd y multiplicar su valor por 16384.

Nota2: Cuando se realiza una llamada al puerto 43B, se pone a 0 los bits correspondientes de las variables de los puertos 7ffd y 1ffd. El valor completo del puerto se copia a la variable del puerto 43b.


*/

int si_superupgrade_muestra_rom_interna(void)
{
  if ( (superupgrade_puerto_43b & (32+64))==32) return 1;
  else return 0;
}

void superupgrade_footer_print_flash_operating(void)
{

    generic_footertext_print_operating("FLASH");

}

void superupgrade_footer_flash_operating(void)
{
  superupgrade_footer_print_flash_operating();

}




//Utilizada en la escritura de byte en flash.
void superupgrade_common_poke_in_flash(z80_int dir,z80_byte valor)
{

  //printf ("Writing in flash memory. Dir: %04XH value: %02XH slot: %d\n",dir,valor,superupgrade_get_rom_bank() );

  int segmento;
	z80_byte *puntero;

  segmento=dir / 16384;
  dir = dir & 16383;
  puntero=superupgrade_memory_paged[segmento]+dir;

  *puntero=valor;


}


/*Testear con
./zesarux --superupgrade-flash /Users/chernandezba/Downloads/varios_superupgrade/superupgrade_pruebas_2.rom --mmc-file /Users/chernandezba/Downloads/varios_superupgrade/pruebasplus3e.mmc --enable-mmc --enable-zxmmc media/spectrum/superupgrade/rman.tap --noautoload
Activar superupgrade
Arrancar con maquina +3e
Ir a basic y:
load "t:"
load ""
randomize usr 32768
*/

void superupgrade_common_write_flash(z80_int dir, z80_byte valor)
{

z80_byte slot=superupgrade_get_rom_bank();

  if (dir<16384) {
    //printf ("Pokeing in rom address %04XH value %02XH PC=%04XH index buffer: %d slot: %d\n",dir,valor,reg_pc,superupgrade_write_buffer_index,slot);

/* Bug en la rom del spectrum pokea en direccion 0. Evitar esto
Pokeing in rom address 0000H value 80H PC=33E1H
Pokeing in rom address 0001H value 00H PC=33EAH
Pokeing in rom address 0002H value 00H PC=33F4H
Pokeing in rom address 0003H value 00H PC=33F4H
Pokeing in rom address 0004H value 00H PC=33F4H
Pokeing in rom address 0000H value 81H PC=33E1H
Pokeing in rom address 0001H value 49H PC=33EAH
Pokeing in rom address 0002H value 0FH PC=33EAH
Pokeing in rom address 0003H value DAH PC=33EAH
Pokeing in rom address 0004H value A2H PC=33EAH
*/

  if (reg_pc==0x33E1 || reg_pc==0x33EA || reg_pc==0x33F4 || reg_pc==0x0E19) {
    //printf ("Write comes from rom bug. Do not do anything\n");
		//printf("Zona Bug, Addr=%d, Data=%d, index=%d \n", dir,valor,superupgrade_write_buffer_index);        
   return;
  }




    //Guardar byte en buffer
    superupgrade_write_buffer[superupgrade_write_buffer_index++]=valor;
	//printf("Escritura en ROM, Addr=%d Data=%d, index=%d, PC=%d \n", dir,valor,superupgrade_write_buffer_index,reg_pc);        

//	printf(str, "Escritura en ROM, Addr=%d Data=%d, index=%d, , stat=%d, PC=%d \n", dir,valor,superupgrade_write_buffer_index, superupgrade_write_status, reg_pc);        




    switch (superupgrade_write_status) {

      //Inicio de comando o escritura
      case 0:
        //Ready
        //Ver si la escritura es un posible comando
        if (slot==1 && dir==0x1555 && valor==0xAA) {
          //Es inicio de comando, cambiar estado
			//printf("primer byte, Addr=%d Data=%d, index=%d \n", dir,valor,superupgrade_write_buffer_index);        
         superupgrade_write_status=1;
        }
        else {
          //Tiene que ser escritura tal cual.
			//printf("Escritura normal en ROM, Addr=%d Data=%d, index=%d, PC=%d status=%d\n", dir,valor,superupgrade_write_buffer_index,reg_pc,superupgrade_write_status);        
//          superupgrade_write_status=2;
			if (superupgrade_flash_write_protected.v==0){
				superupgrade_write_status=2;
			} else {
				superupgrade_write_buffer_index =0;
				superupgrade_write_status=0;
			}
        }
      break;

      //Se esta enviando comando
      case 1:
        //Ver que posicion de comando
        //Segundo byte
        if (superupgrade_write_buffer_index==2) {
          if (slot==0 && dir==0x2AAA && valor!=0x55) {
            //Comando invalido. Decir que es escritura de sector
	    debug_printf (VERBOSE_DEBUG,"Invalid superupgrade command. Consider it as a sector write");
 //           superupgrade_write_status=2;
 			superupgrade_write_buffer_index =0;
			superupgrade_write_status=0;
         } else {
			//printf("segundo byte, Addr=%d Data=%d, index=%d \n", dir,valor,superupgrade_write_buffer_index);        
		  }
        }

        //Tercer byte
        if (superupgrade_write_buffer_index==3) {
          if (slot==1 && dir==0x1555) {
            if (valor==0xA0) {
            //Fin de comando de poner el chip en modo protegido
	    debug_printf (VERBOSE_DEBUG,"End superupgrade command protect flash (unprotect+write+protect)");
			//printf("End superupgrade command protect flash (unprotect+write+protect), Addr=%d Data=%d, index=%d\n", dir,valor,superupgrade_write_buffer_index);        
            superupgrade_write_status=0;
            superupgrade_write_buffer_index=0;
            superupgrade_flash_write_protected.v=0;
            superupgrade_pending_protect_flash.v=1;
            }

          //Sera comando de desproteger el chip?
            else if (valor!=0x80) {
              //Comando invalido. Decir que es escritura de sector
				debug_printf (VERBOSE_DEBUG,"Invalid superupgrade command. Consider it as a sector write");
				//printf ("Invalid superupgrade command. Consider it as a sector write\n");
//				superupgrade_write_status=2;
				superupgrade_write_buffer_index =0;
				superupgrade_write_status=0;
            }
          }
        }

        //Cuarto byte
        if (superupgrade_write_buffer_index==4) {
          if (slot==1 && dir==0x1555 && valor!=0xAA) {
            //Comando invalido. Decir que es escritura de sector
            debug_printf (VERBOSE_DEBUG,"Invalid superupgrade command. Consider it as a sector write");
            superupgrade_write_status=2;
          }
        }

        //Quinto byte
        if (superupgrade_write_buffer_index==5) {
          if (slot==0 && dir==0x2AAA && valor!=0x55) {
            //Comando invalido. Decir que es escritura de sector
            debug_printf (VERBOSE_DEBUG,"Invalid superupgrade command. Consider it as a sector write");
            superupgrade_write_status=2;
          }
        }

        //Sexto byte
        if (superupgrade_write_buffer_index==6) {
          if (slot==1 && dir==0x1555) {
            if (valor==0x20) {
            //Fin de comando de poner el chip en modo desprotegido
            debug_printf (VERBOSE_DEBUG,"End superupgrade command unprotect flash");
            superupgrade_write_status=0;
            superupgrade_write_buffer_index=0;
            superupgrade_flash_write_protected.v=0;
            }


            else {
              //Comando invalido. Decir que es escritura de sector
              debug_printf (VERBOSE_DEBUG,"Invalid superupgrade command. Consider it as a sector write");
              superupgrade_write_status=2;
            }
          }
        }
      break;

      case 2:
      //Se esta escriendo sector de 256 bytes
			//printf("escribiendo, Addr=%d, Data=%d, index=%d, status=%d \n", dir,valor,superupgrade_write_buffer_index,superupgrade_write_status);        
      break;

      default:
        cpu_panic("Invalid superupgrade write status");
      break;
    }


    //Si buffer llega al final, resetear a 0 y controlar si hay que escribir
    if (superupgrade_write_buffer_index==256) {


      //Si esta desprotegido, escribir
      if (superupgrade_flash_write_protected.v==0) {
        debug_printf (VERBOSE_DEBUG,"Write 256 byte sector in superupgrade flash");
        //Escribir los 256 bytes
        int i;
        for (i=0;i<256;i++) {
          superupgrade_common_poke_in_flash(dir-255+i,superupgrade_write_buffer[i]);
        }

        superupgrade_flash_must_flush_to_disk=1;
        superupgrade_write_status=0;

        superupgrade_footer_flash_operating();

        //Se habia ejecutado un comando de proteger la flash. Protegerla
        if (superupgrade_pending_protect_flash.v) {
          superupgrade_flash_write_protected.v=1;
          superupgrade_pending_protect_flash.v=0;
          debug_printf (VERBOSE_DEBUG,"Protecting superupgrade flash");
          //printf ("Protecting superupgrade flash");
        }

      }
      else {
        debug_printf (VERBOSE_DEBUG,"Trying to write 256 byte sector in superupgrade flash but it is protected");
        //printf ("Trying to write 256 byte sector in superupgrade flash but it is protected");
      }

      superupgrade_write_buffer_index=0;
    }

/*
Comandos modo protegido/desprotegido
-Luego, para poner el chip en modo desprotegido, escribo las direcciones desde el punto de vista del Z80 y el slot activo:

Escribir AA en la dirección $1555 , con slot 1 activo
Escribir 55 en la dirección $2AAA, con slot 0 activo
Escribir 80 en la dirección $1555 , con slot 1 activo
Escribir AA en la dirección $1555, con slot 1 activo
Escribir 55 en la dirección $2AAA, con slot 0 activo
Escribir 20 en la dirección $1555, con slot 1 activo


-Luego, para poner el chip en modo protegido, la secuencia de direccion del z80+slot seria:

Escribir AA en la dirección $1555 , con slot 1 activo
Escribir 55 en la dirección $2AAA, con slot 0 activo
Escribir A0 en la dirección $1555 , con slot 1 activo
Y escribir 256 bytes de datos.
Entendemos pues, tal y como dice la especificación del chip, que para protegerlo hay que enviar esa secuencia de 3 bytes y extrañamente escribir 256 bytes de datos a la flash para que finalmente se proteja.


*/


  }
}

void superupgrade_common_poke(z80_int dir,z80_byte valor)
{

  superupgrade_common_write_flash(dir,valor);

	int segmento;
	z80_byte *puntero;
        if (dir>16383) {
                segmento=dir / 16384;
                dir = dir & 16383;
                puntero=superupgrade_memory_paged[segmento]+dir;

                *puntero=valor;
        }
}


z80_byte superupgrade_poke_byte_no_time(z80_int dir,z80_byte valor)
{

  superupgrade_common_poke(dir,valor);
	debug_nested_poke_byte_no_time_call_previous(superupgrade_nested_id_poke_byte_no_time,dir,valor);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}




z80_byte superupgrade_poke_byte(z80_int dir,z80_byte valor)
{

  superupgrade_common_poke(dir,valor);
	debug_nested_poke_byte_call_previous(superupgrade_nested_id_poke_byte,dir,valor);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;

}


z80_byte superupgrade_peek_byte_no_time(z80_int dir_orig,z80_byte value GCC_UNUSED)
{

      //Si se muestra ROM interna en vez de pagina de superupgrade
      if (dir_orig<16384 && si_superupgrade_muestra_rom_interna() ) {
        return debug_nested_peek_byte_no_time_call_previous(superupgrade_nested_id_peek_byte_no_time,dir_orig);
      }

        int segmento;
        z80_byte *puntero;
	z80_int dir=dir_orig;
        segmento=dir / 16384;

        dir = dir & 16383;
        puntero=superupgrade_memory_paged[segmento]+dir;

	//Aunque no usamos el valor de peek, llamamos para realizar contienda, llamar a otras funciones anidadas, etc
	//superupgrade_original_peek_byte_no_time(dir_orig);
	debug_nested_peek_byte_no_time_call_previous(superupgrade_nested_id_peek_byte_no_time,dir);

        return *puntero;

}

z80_byte superupgrade_peek_byte(z80_int dir_orig,z80_byte value GCC_UNUSED)
{

        //Si se muestra ROM interna en vez de pagina de superupgrade
        if (dir_orig<16384 && si_superupgrade_muestra_rom_interna() ) {
          return debug_nested_peek_byte_call_previous(superupgrade_nested_id_peek_byte,dir_orig);
        }

        int segmento;
        z80_byte *puntero;
	z80_int dir=dir_orig;
        segmento=dir / 16384;

        dir = dir & 16383;
        puntero=superupgrade_memory_paged[segmento]+dir;

	//Aunque no usamos el valor de peek, llamamos para realizar contienda, llamar a otras funciones anidadas, etc
	//superupgrade_original_peek_byte(dir_orig);
	debug_nested_peek_byte_call_previous(superupgrade_nested_id_peek_byte,dir);

        return *puntero;

}




//Establecer rutinas propias
void superupgrade_set_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Setting superupgrade poke / peek functions");
                //Guardar anteriores
                //superupgrade_original_poke_byte=poke_byte;
                //superupgrade_original_poke_byte_no_time=poke_byte_no_time;
                //superupgrade_original_peek_byte=peek_byte;
                //superupgrade_original_peek_byte_no_time=peek_byte_no_time;

                //Modificar y poner las de superupgrade
                //poke_byte=superupgrade_poke_byte;
                //poke_byte_no_time=superupgrade_poke_byte_no_time;
                //peek_byte=superupgrade_peek_byte;
                //peek_byte_no_time=superupgrade_peek_byte_no_time;

        superupgrade_nested_id_poke_byte=debug_nested_poke_byte_add(superupgrade_poke_byte,"Superupgrade poke_byte");
        superupgrade_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(superupgrade_poke_byte_no_time,"Superupgrade poke_byte_no_time");
        superupgrade_nested_id_peek_byte=debug_nested_peek_byte_add(superupgrade_peek_byte,"Superupgrade peek_byte");
        superupgrade_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(superupgrade_peek_byte_no_time,"Superupgrade peek_byte_no_time");

}

//Restaurar rutinas de superupgrade
void superupgrade_restore_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before superupgrade");
                //poke_byte=superupgrade_original_poke_byte;
                //poke_byte_no_time=superupgrade_original_poke_byte_no_time;
                //peek_byte=superupgrade_original_peek_byte;
                //peek_byte_no_time=superupgrade_original_peek_byte_no_time;
        debug_nested_poke_byte_del(superupgrade_nested_id_poke_byte);
        debug_nested_poke_byte_no_time_del(superupgrade_nested_id_poke_byte_no_time);
        debug_nested_peek_byte_del(superupgrade_nested_id_peek_byte);
        debug_nested_peek_byte_no_time_del(superupgrade_nested_id_peek_byte_no_time);

}



void superupgrade_alloc_memory(void)
{
        int size=(SUPERUPGRADE_ROM_SIZE+SUPERUPGRADE_RAM_SIZE);

        debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for superupgrade emulation",size/1024);

        superupgrade_rom_memory_pointer=malloc(size);
        if (superupgrade_rom_memory_pointer==NULL) {
                cpu_panic ("No enough memory for superupgrade emulation");
        }

	//La RAM empieza despues de la ROM
	superupgrade_ram_memory_pointer=&superupgrade_rom_memory_pointer[SUPERUPGRADE_ROM_SIZE];


}

int superupgrade_load_rom(void)
{

        FILE *ptr_superupgrade_romfile;
        int leidos=0;

        debug_printf (VERBOSE_INFO,"Loading superupgrade rom %s",superupgrade_rom_file_name);

  			ptr_superupgrade_romfile=fopen(superupgrade_rom_file_name,"rb");
                if (!ptr_superupgrade_romfile) {
                        debug_printf (VERBOSE_ERR,"Unable to open ROM file");
                }

        if (ptr_superupgrade_romfile!=NULL) {

                leidos=fread(superupgrade_rom_memory_pointer,1,SUPERUPGRADE_ROM_SIZE,ptr_superupgrade_romfile);
                fclose(ptr_superupgrade_romfile);

        }



        if (leidos!=SUPERUPGRADE_ROM_SIZE || ptr_superupgrade_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading superupgrade rom");
                return 1;
        }

        return 0;
}



void superupgrade_init_memory_tables(void)
{

	int pagina;

	for (pagina=0;pagina<32;pagina++) {
		superupgrade_rom_memory_table[pagina]=&superupgrade_rom_memory_pointer[16384*pagina];
		superupgrade_ram_memory_table[pagina]=&superupgrade_ram_memory_pointer[16384*pagina];
	}
}

z80_byte superupgrade_get_rom_bank(void)
{
	z80_byte banco;

	banco=(superupgrade_puerto_43b&31)+((puerto_32765>>4)&1)+((puerto_8189>>1)&2);

	return banco;
}

z80_byte superupgrade_get_ram_bank(void)
{
	z80_byte banco;

	//Maquinas de 128k solo soporta 128kb de RAM
	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) banco=(puerto_32765&7);

	else banco=(puerto_32765&7)+((puerto_32765>>3)&24);

	return banco;
}

void superupgrade_set_memory_pages(void)
{
	z80_byte rom_page=superupgrade_get_rom_bank();
	z80_byte ram_page=superupgrade_get_ram_bank();

	superupgrade_memory_paged[0]=superupgrade_rom_memory_table[rom_page];

	superupgrade_memory_paged[1]=superupgrade_ram_memory_table[5];
	superupgrade_memory_paged[2]=superupgrade_ram_memory_table[2];
	superupgrade_memory_paged[3]=superupgrade_ram_memory_table[ram_page];



                debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+rom_page;
                debug_paginas_memoria_mapeadas[1]=5;
                debug_paginas_memoria_mapeadas[2]=2;
                debug_paginas_memoria_mapeadas[3]=ram_page;

}

void superupgrade_hard_reset(void)
{
	superupgrade_puerto_43b=0;
  superupgrade_flash_write_protected.v=1;
  superupgrade_write_status=0;
  superupgrade_write_buffer_index=0;
  superupgrade_pending_protect_flash.v=0;

	superupgrade_set_memory_pages();
}

int superupgrade_supported_machine(void)
{
	if (
		(    (MACHINE_IS_SPECTRUM_16_48 || MACHINE_IS_SPECTRUM_128_P2 || MACHINE_IS_SPECTRUM_128_P2_P2A_P3) && !(MACHINE_IS_INVES)     )
	) return 1;

	return 0;

}

void superupgrade_enable(int hard_reset)
{
  if (!MACHINE_IS_SPECTRUM) {
    debug_printf(VERBOSE_INFO,"Can not enable superupgrade on non Spectrum machine");
    return;
  }

	if (superupgrade_enabled.v) return;

	//En todas maquinas 16, 48, 128 , +2 y +2A pero en inves no
	if (!superupgrade_supported_machine()) {
                debug_printf (VERBOSE_ERR,"Superupgrade is not allowed on this machine");
		return;
	}

	if (superupgrade_rom_file_name[0]==0) {
		debug_printf (VERBOSE_ERR,"Trying to enable Superupgrade but no ROM file selected");
		return;
	}

	superupgrade_alloc_memory();
	superupgrade_init_memory_tables();
	superupgrade_set_memory_pages();
	if (superupgrade_load_rom()) return;

	superupgrade_set_peek_poke_functions();

	superupgrade_enabled.v=1;

	if (hard_reset) hard_reset_cpu();

}

void superupgrade_disable(void)
{
	if (superupgrade_enabled.v==0) return;

	superupgrade_restore_peek_poke_functions();

	free(superupgrade_rom_memory_pointer);

	superupgrade_enabled.v=0;



}



void superupgrade_write_7ffd(z80_byte value)
{
	//si desactivado, volver
	if (puerto_32765&32) return;

  //Bit indica desactivado pero el otro bit (el de 1ffd) no esta a 0, cosa que indicaria que se muestra la rom interna de superupgrade
	if (superupgrade_puerto_43b&32 && !si_superupgrade_muestra_rom_interna() ) return;
	puerto_32765=value;
	superupgrade_set_memory_pages();
}

void superupgrade_write_1ffd(z80_byte value)
{
	//si desactivado, volver
	if (superupgrade_puerto_43b&64) return;

        puerto_8189=value;
        superupgrade_set_memory_pages();
}

void superupgrade_write_43b(z80_byte value)
{
        superupgrade_puerto_43b=value;

	//Poner a 0 bits de paginacion de 8189  y 32765
	/*Cuando se realiza una llamada al puerto 43B, se pone a 0 los bits correspondientes de las variables de los puertos 7ffd y 1ffd. El valor completo del puerto se copia a la variable del puerto 43b.*/

	puerto_32765 &=(255-16);
	puerto_8189 &=(255-4);

        superupgrade_set_memory_pages();
}


void superupgrade_flush_flash_to_disk(void)
{

        if (superupgrade_enabled.v==0) return;

        if (superupgrade_flash_must_flush_to_disk==0) {
                debug_printf (VERBOSE_DEBUG,"Trying to flush superupgrade to disk but no changes made");
                return;
        }


        debug_printf (VERBOSE_INFO,"Flushing superupgrade flash to disk");
        //printf ("Flushing superupgrade to disk\n");


        FILE *ptr_superupgradefile;

        debug_printf (VERBOSE_INFO,"Opening superupgrade File %s",superupgrade_rom_file_name);
        ptr_superupgradefile=fopen(superupgrade_rom_file_name,"wb");



        int escritos=0;
        long int size;
        size=SUPERUPGRADE_ROM_SIZE;


        if (ptr_superupgradefile!=NULL) {
                z80_byte *puntero;
                puntero=superupgrade_rom_memory_pointer;

                //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
                //metera flush a 1
                superupgrade_flash_must_flush_to_disk=0;

                escritos=fwrite(puntero,1,size,ptr_superupgradefile);

                fclose(ptr_superupgradefile);

        }



       //printf ("ptr_superupgradefile: %d\n",ptr_superupgradefile);
       //printf ("escritos: %d\n",escritos);

       if (escritos!=size || ptr_superupgradefile==NULL) {
               debug_printf (VERBOSE_ERR,"Error writing to superupgrade file");
       }

}
