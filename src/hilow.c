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

//de momento asi abierta para probar
z80_bit hilow_tapa_abierta={1};

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
	dir &= 2047; 


	//La RAM esta despues de los 8kb de rom
	return hilow_memory_pointer[8192+dir];
}

void hilow_poke_ram(z80_int dir,z80_byte value)
{

	if (hilow_check_if_ram_area(dir) ) {
		//printf ("Poke ram byte to %04XH with value %02XH\n",dir,value);
		dir &= 2047; 
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

void hilow_automap_unmap_memory(z80_int dir)
{
	//Si hay que mapear/desmapear memorias
	//printf ("test dir %04XH\n",dir); 

	//Puntos de mapeo rom
	//Si no estaba mapeada
	if (hilow_mapped_rom.v==0) {
		if (dir==0x04C2 || dir==0x0556 || dir==0x0976) {
			//printf ("Mapeando rom en %04XH\n",dir);
			hilow_mapped_rom.v=1;
		}
	}

	//Puntos de desmapeo rom
	//Si estaba mapeada
	if (hilow_mapped_rom.v==1) {
		if (dir==0x0052) {
			hilow_mapped_rom.v=0;
			//printf ("Desmapeando rom en %04XH\n",dir);
		}
	}	


	//Mapeo de ram de momento identico que rom
	//TODO: si realmente es identico, meter este codigo arriba
	//Puntos de mapeo ram
	//Si no estaba mapeada
	if (hilow_mapped_ram.v==0) {
		if (dir==0x04C2 || dir==0x0556 || dir==0x0976) {
			hilow_mapped_ram.v=1;
			//printf ("Mapeando ram en %04XH\n",dir);
		}
	}

	//Puntos de desmapeo ram
	//Si estaba mapeada
	if (hilow_mapped_ram.v==1) {
		if (dir==0x0052) {
			hilow_mapped_ram.v=0;
			//printf ("Desmapeando ram en %04XH\n",dir);
		}
	}	

}

//temporal para guardar la imagen del datadrive
//esto probablemente deberia ser 512 kb: 256 sectores X 2048 bytes/sector = 512 KB
z80_byte temp_hilow_buffer[1024*1024];

void temp_hilow_write(int sector,int offset,z80_byte valor)
{
    offset +=(sector*2048);

    temp_hilow_buffer[offset]=valor;
}

z80_byte temp_hilow_read(int sector,int offset)
{
    offset +=(sector*2048);

    return temp_hilow_buffer[offset];
}


z80_byte cpu_core_loop_spectrum_hilow(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

        //Llamar a anterior
        debug_nested_core_call_previous(hilow_nested_id_core);


		hilow_automap_unmap_memory(reg_pc);


        //debug de rutinas
        if (reg_pc==0x186D && hilow_mapped_rom.v) {
            //probablemente esta direccion NO es lectura de sector
            printf("Entering READ_SECTOR. from %04XH A=%02XH IX=%04XH DE=%04XH HL=%04XH BC=%04XH SP=%04XH\n",
                peek_word(reg_sp),reg_a,reg_ix,reg_de,reg_hl,reg_bc,reg_sp);

            char buffer[2048];
            print_registers(buffer);

            printf ("%s\n",buffer);

            //mostrar algunos caracteres
            int i;
            for (i=0;i<100;i++) {
                z80_byte c=hilow_read_ram_byte(i);
                printf("%c",(c>=32 && c<=126 ? c : '.'));
            }
            printf("\n");            

            //pruebas de handler. Le escribo datos y retorno de dicha funcion

            
            z80_int inicio_datos=8192;

                for (i=0;i<2048;i++) {
                    //poke_byte_no_time(reg_ix+i,'!');
                    //reg_de?
                    //poke_byte_no_time(inicio_datos+i,'!');

                    //todo a 255
                    //poke_byte_no_time(inicio_datos+i,255);

                    //lectura de la imagen de memoria
                    poke_byte_no_time(inicio_datos+i,temp_hilow_read(reg_a,i));
                }     

                //trampeamos los dos primeros bytes que da espacio ocupado??
                if (reg_a==0) {
                 //   poke_byte_no_time(inicio_datos+0,2);   
                   // poke_byte_no_time(inicio_datos+1,10);   


                    //en algun punto de esta zona debe estar a 255 para que retorne 510 KB libres
                    //for (i=1000;i<2048;i++) {
                    for (i=1000;i<1048;i++) {
                        //poke_byte_no_time(reg_ix+i,'!');


                        poke_byte_no_time(inicio_datos+i,255);
                    }                   
                }

            
            
            /*
            if (reg_a==0) { //Sector 0 directorio

                for (i=0;i<2048;i++) {
                    //poke_byte_no_time(reg_ix+i,'!');
                    //reg_de?
                    poke_byte_no_time(inicio_datos+i,'!');


                    poke_byte_no_time(inicio_datos+i,255);
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

                //tamaño archivo. 
                poke_byte_no_time(inicio_datos+11+11,1); 
                poke_byte_no_time(inicio_datos+11+12,3); 

                //primer parametro de cabecera de cinta: direccion, line, etc
                poke_byte_no_time(inicio_datos+11+13,4);
                poke_byte_no_time(inicio_datos+11+14,2);
                
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
            */

            //no error?
            //Z80_FLAGS=(Z80_FLAGS & (255-FLAG_C));
            //Z80_FLAGS |=FLAG_C;
            reg_a=0;

            //valor distinto de 0 retorna el error "Error en la cinta"
            //reg_a=1;

            reg_pc=pop_valor();

            //reg_pc=0x1927;

            //reg_pc=0x18a3;

            //no carry
	        //Z80_FLAGS=(Z80_FLAGS & (255-FLAG_C));

            //carry
        	//Z80_FLAGS |=FLAG_C;


            printf("Returning from READ_SECTOR to address %04XH\n",reg_pc);
        }

        //debug de rutinas
        if (reg_pc==0x16D0 && hilow_mapped_rom.v) {
            
            printf("Entering WRITE_SECTOR. A=%02XH IX=%04XH DE=%04XH\n",reg_a,reg_ix,reg_de);

            //mostrar algunos caracteres
            int i;
            for (i=0;i<2048;i++) {
                z80_byte c=hilow_read_ram_byte(i);
                if (c>=32 && c<=126) printf("%c",c);
                else printf(" %02XH ",c);
            }
            printf("\n");

            //sector 1=0??
            int sector=reg_a;

            if (sector==1) sector=0;

            for (i=0;i<2048;i++) {
                z80_byte c=hilow_read_ram_byte(i);
                temp_hilow_write(sector,i,c);
            }            

            //no error?
            reg_a=0;

            reg_pc=pop_valor();
            printf("Returning from WRITE_SECTOR to address %04XH\n",reg_pc);


        }        

        if (reg_pc==0x1A9E && hilow_mapped_rom.v) {
            
            printf("Entering POST_FORMAT. A=%02XH IX=%04XH DE=%04XH\n",reg_a,reg_ix,reg_de);

            //saltar adelante en codigo. feo....
            //reg_pc=0x1ad8;
            reg_pc=0x1ac8;
            reg_pc=0x1acf;

            printf("Skipping to address %04XH\n",reg_pc);
        }              

        if (reg_pc==0x1AC0 && hilow_mapped_rom.v) {
            
            printf("Entering POST_FORMAT2. A=%02XH IX=%04XH DE=%04XH\n",reg_a,reg_ix,reg_de);

            //saltar adelante en codigo. feo....
            //reg_pc=0x1ad8;
            reg_pc=0x1ac8;
            reg_pc=0x1acf;

            printf("Skipping to address %04XH\n",reg_pc);
        }             

        if (reg_pc==0x1AF1 && hilow_mapped_rom.v) {
            
            printf("Entering POST_FORMAT3. A=%02XH IX=%04XH DE=%04XH\n",reg_a,reg_ix,reg_de);

            //engañar... para saltar una condicion que hace cancelar el bucle de sectores 1,2,3,...
            //Z80_FLAGS |=FLAG_Z;
        }


        if (reg_pc==0x08FB && hilow_mapped_rom.v) {
            printf("Entering L08FB. A=%02XH IX=%04XH DE=%04XH\n",reg_a,reg_ix,reg_de);

            //saltar opcode JP      Z,BREAKCONT
            reg_pc +=3;

            printf("Skipping to address %04XH\n",reg_pc);

            //Esto al hacer un SAVE al final parece ir a la direccion 0 y se resetea...
        }
        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;

}



//Establecer rutinas propias
void hilow_set_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Setting hilow poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	hilow_nested_id_poke_byte=debug_nested_poke_byte_add(hilow_poke_byte,"Hilow poke_byte");
	hilow_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(hilow_poke_byte_no_time,"Hilow poke_byte_no_time");
	hilow_nested_id_peek_byte=debug_nested_peek_byte_add(hilow_peek_byte,"Hilow peek_byte");
	hilow_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(hilow_peek_byte_no_time,"Hilow peek_byte_no_time");


	hilow_nested_id_core=debug_nested_core_add(cpu_core_loop_spectrum_hilow,"Hilow Spectrum core");


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



void hilow_alloc_memory(void)
{
        int size=HILOW_MEM_SIZE;  

        debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for hilow emulation",size/1024);

        hilow_memory_pointer=malloc(size);
        if (hilow_memory_pointer==NULL) {
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


	hilow_alloc_memory();
	if (hilow_load_rom()) return;

	hilow_set_peek_poke_functions();

	hilow_enabled.v=1;

	hilow_reset();




}

void hilow_disable(void)
{
	if (hilow_enabled.v==0) return;

	hilow_restore_peek_poke_functions();

	free(hilow_memory_pointer);

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


/*
Lectura:

Bit 7: ??
Bit 6: A 1 si grabador encendido
Bit 5: ??
Bit 4: ??
Bit 3: A 1 si tapa cerrada?? fin de cinta??
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

    if (hilow_tapa_abierta.v==0) valor_retorno |=8; //tapa cerrada??

    valor_retorno |=64; //grabador encendido


    valor_retorno |=1; //listo

    return valor_retorno; 




}