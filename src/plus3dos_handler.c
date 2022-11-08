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

PLUS3DOS handler

*/


#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#include "plus3dos_handler.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "mem128.h"
#include "dsk.h"


z80_bit plus3dos_traps={0};




void traps_plus3dos_return(void)
{
	reg_pc=pop_valor();
}

void traps_plus3dos_return_ok(void)
{

	Z80_FLAGS |=FLAG_C;
	traps_plus3dos_return();
}

void traps_plus3dos_return_error(void)
{

	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_C));
	traps_plus3dos_return();
}

/*
archivo de pruebas en cinta 
pruebaplustres.tap

0   (de program)
 1d 00 00 80 1d 00  (longitud, par1, par2)



datos:
00 01 0e 00 ea 68 6f 6c 
61 20 71 75 65 20 74 61  6c 0d 00 02 07 00 ea 61  
 64 69 6f 73 0d

*/

z80_byte p3dos_prueba_header[]={0,0x1d, 0x00, 00, 0x80, 0x1d ,00,0};
z80_byte p3dos_prueba_datos[]={00 ,0x01 ,0x0e ,0x00 ,0xea ,0x68 ,0x6f ,0x6c 
,0x61 ,0x20 ,0x71 ,0x75 ,0x65 ,0x20 ,0x74 ,0x61  ,0x6c ,0x0d ,0x00 ,0x02 ,0x07 ,0x00 ,0xea ,0x61  
 ,0x64 ,0x69 ,0x6f ,0x73 ,0x0d};


void traps_plus3dos_handle_ref_head(void)
{
	reg_ix=49152;
	int i;

			z80_byte *p;
		p=ram_mem_table[7];

	for (i=0;i<8;i++) {

		p[i]=p3dos_prueba_header[i];
	}
}

void traps_plus3dos_handle_dos_read(void)
{

	/*
Read bytes from a file into memory.

Advance the file pointer.

The destination buffer is in the following memory configuration:

	C000h...FFFFh (49152...65535)	- Page specified in C
	8000h...BFFFh (32768...49151)	- Page 2
	4000h...7FFFh (16384...32767)	- Page 5
	0000h...3FFFh (0...16383)	- DOS ROM

The routine does not consider soft-EOF.

Reading EOF will produce an error.

ENTRY CONDITIONS
	B = File number
	C = Page for C000h (49152)...FFFFh (65535)
	DE = Number of bytes to read (0 means 64K)
	HL = Address for bytes to be read

EXIT CONDITIONS
	If OK:
		Carry true
		A DE corrupt
	Otherwise:
		Carry false
		A = Error code
		DE = Number of bytes remaining unread
	Always:
		BC HL IX corrupt
		All other registers preserved
	*/


	reg_ix=49152;
	int i;

			z80_byte *p;
		p=ram_mem_table[reg_c];

		//temp
		p=ram_mem_table[5];

	for (i=0;i<reg_de;i++) {
		//poke_byte_no_time(reg_hl+i,p3dos_prueba_datos[i]);
		p[(reg_hl&16383)+i]=p3dos_prueba_datos[i];
	}
}

                        
void traps_plus3dos_dd_l_dpb(void)
{
/*
DD L DPB                Initialise a DPB from a disk specification
DD L DPB
018Ah (394)

Initialise a DPB for a given format.

This routine does not affect or consider the freeze flag.

ENTRY CONDITIONS
        IX = Address of destination DPB
        HL = Address of source disk specification

EXIT CONDITIONS
        If OK:
                Carry true
                A = Disk type recorded on disk
                DE = Size of allocation vector
                HL = Size of hash table
        If bad format:
                Carry false
                A = Error code
                DE HL corrupt
        Always:
                BC IX corrupt
                All other registers preserved
*/
		//De momento no tengo claro que retornar en DE, HL
		DE=16;
		HL=16;

		traps_plus3dos_return_ok();

}

void traps_plus3dos_dd_l_seek(void)
{
/*
DD L SEEK
018Dh (397)

Seek to required track.

Retry if fails.

ENTRY CONDITIONS
        C = Unit/head
                bits 0...1 = unit
                bit 2 = head
                bits 3...7 = 0
        D = Track
        IX = Address of XDPB

EXIT CONDITIONS
        If OK:
                Carry true
                A corrupt
        Otherwise:
                Carry false
                A = Error report
        Always:
                BC DE HL IX corrupt
                All other registers preserved
*/

	debug_printf(VERBOSE_DEBUG,"Seek to unit %d track %d",reg_c,reg_d);

	traps_plus3dos_return_ok();
}


int traps_plus3dos_pistas=40;
int traps_plus3dos_sect_pista=9;
int traps_plus3dos_bytes_sector=512;

//9 sectores por pista, 512 bytes por sector

int traps_plus3dos_getoff_start_trackinfo(int pista_lineal)
{
	int traps_plus3dos_dsk_trackstep=(traps_plus3dos_bytes_sector*traps_plus3dos_sect_pista)+256;

	return 0x100+pista_lineal*traps_plus3dos_dsk_trackstep;
}

int traps_plus3dos_getoff_start_track(int pista_lineal)
{
	return traps_plus3dos_getoff_start_trackinfo(pista_lineal)+0x100;
}



//Retorna el offset al dsk segun la pista y sector dados 
//Pista empieza en 0
//Sectores empiezan en el 1....
int traps_plus3dos_getoff_track_sector(int pista_buscar,int sector_buscar)
{

/*
sectores van alternados:
00000100  54 72 61 63 6b 2d 49 6e  66 6f 0d 0a 00 00 00 00  |Track-Info......|
00000110  00 00 00 00 02 09 4e e5  00 00 c1 02 00 00 00 02  |......N.........|
00000120  00 00 c6 02 00 00 00 02  00 00 c2 02 00 00 00 02  |................|
00000130  00 00 c7 02 00 00 00 02  00 00 c3 02 00 00 00 02  |................|
00000140  00 00 c8 02 00 00 00 02  00 00 c4 02 00 00 00 02  |................|
00000150  00 00 c9 02 00 00 00 02  00 00 c5 02 00 00 00 02  |................|
00000160  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|

1,6,2,7,3,8


0 1 2 3 4 5 6 7 8  
0,5,1,6,2,7,3,8,4

*/

	int pista;
	int sector;

	int iniciopista_orig=256;

    printf("buscando traps_plus3dos_getoff_track_sector pista_buscar %d sector_buscar %d\n",pista_buscar,sector_buscar);

	//Buscamos en todo el archivo dsk
	for (pista=0;pista<traps_plus3dos_pistas;pista++) {

		int sectores_en_pista=plus3dsk_get_byte_disk(iniciopista_orig+0x15);
		debug_printf(VERBOSE_DEBUG,"Iniciopista: %XH (%d). Sectores en pista %d: %d. IDS pista:  ",iniciopista_orig,iniciopista_orig,pista,sectores_en_pista);

		//int iniciopista_orig=traps_plus3dos_getoff_start_trackinfo(pista);
		int iniciopista=iniciopista_orig;
		//saltar 0x18
		iniciopista +=0x18;

		for (sector=0;sector<sectores_en_pista;sector++) {
			int offset_tabla_sector=sector*8; 
			z80_byte pista_id=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector); //Leemos pista id
			z80_byte sector_id=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+2); //Leemos c1, c2, etc

			debug_printf(VERBOSE_DEBUG,"%02X ",sector_id);
            
            printf("Sector id leido: %02XH\n",sector_id);

			sector_id &=0xF;

			//sector_id--;  //empiezan en 1...

			if (pista_id==pista_buscar && sector_id==sector_buscar) {
				debug_printf(VERBOSE_DEBUG,"Found sector %d/%d at %d/%d",pista_buscar,sector_buscar,pista,sector);
                printf("Found sector %d/%d at %d/%d\n",pista_buscar,sector_buscar,pista,sector);
                        //sleep(3);
		                //int offset=traps_plus3dos_getoff_start_track(pista);
		                int offset=iniciopista_orig+256;

                		//int iniciopista=traps_plus3dos_getoff_start_track(pista);
                        int offset_retorno=offset+traps_plus3dos_bytes_sector*sector;
                        printf("Offset sector: %XH\n",offset_retorno);

		                return offset_retorno;
			}

		}

		debug_printf(VERBOSE_DEBUG,"");

		iniciopista_orig +=256;
		iniciopista_orig +=traps_plus3dos_bytes_sector*sectores_en_pista;
	}

	debug_printf(VERBOSE_DEBUG,"Not found sector %d/%d",pista_buscar,sector_buscar);	
	//TODO
	//de momento retornamos offset fuera de rango
	return DSK_MAX_BUFFER_DISCO;


	//Old

	//Sector 0 esta en posicion 0
	//Sector 5 esta en posicion 1

	//Sector 1 esta en posicion 2
	//Sector 6 esta en posicion 3

	//Sector 2 esta en posicion 4
	//Sector 7 esta en posicion 5

	//Sector 3 esta en posicion 6
	//Sector 8 esta en posicion 7

	//Sector 4 esta en posicion 8
/*

			    //0 1 2 3 4 5 6 7 8
	int saltossectores[]={0,2,4,6,8,1,3,5,7};


	int sectorfinal=saltossectores[sector];

	int sectorpista=iniciopista+512*sectorfinal;

	return sectorpista;
*/
}

void traps_poke_addr_page(z80_byte page,z80_int dir,z80_byte value)
{

        z80_byte *p;
	int segmento=dir/16384;
	if (dir<16384) return;

	if (dir>49151) {
		p=ram_mem_table[page];
	}

	else {
		p=memory_paged[segmento];
	}
	

        p[dir&16383]=value;

}


z80_byte traps_peek_addr_page(z80_byte page,z80_int dir)
{

        z80_byte *p;
        int segmento=dir/16384;

        if (dir>49151) {
                p=ram_mem_table[page];
        }

        else {
                p=memory_paged[segmento];
        }


        return p[dir&16383];

}


void traps_plus3dos_write_sector(void)
{
/*
DD WRITE SECTOR
0166h (358)

Write a sector.

ENTRY CONDITIONS
        B = Page for C000h (49152)...FFFFh (65535)
        C = Unit (0/1)
        D = Logical track, 0 base
        E = Logical sector, 0 base
        HL = Address of buffer
        IX = Address of XDPB

EXIT CONDITIONS
        If OK:
                Carry true
                A corrupt
        Otherwise:
                Carry false
                A = Error code
        Always:
                BC DE HL IX corrupt
                All other registers preserved
*/

        int iniciosector=traps_plus3dos_getoff_track_sector(reg_d,reg_e+1);


        int i;
        for (i=0;i<512;i++) {
		z80_byte byte_a_grabar=traps_peek_addr_page(reg_b,reg_hl+i);
                plus3dsk_put_byte_disk(iniciosector+i,byte_a_grabar);
        }


	traps_plus3dos_return_ok();

}

                                       
void traps_plus3dos_read_sector(void)
{

/*


DD READ SECTOR
0163h (355)

Read a sector.

ENTRY CONDITIONS
        B = Page for C000h (49152)...FFFFh (65535)
        C = Unit (0/1)
        D = Logical track, 0 base
        E = Logical sector, 0 base
        HL = Address of buffer
        IX = Address of XDPB

EXIT CONDITIONS
        If OK:
                Carry true
                A corrupt
        Otherwise:
                Carry false
                A = Error code
        Always:
                BC DE HL IX corrupt
                All other registers preserved


        */

/*
Formato DSK
Primera pista:
00000100  54 72 61 63 6b 2d 49 6e  66 6f 0d 0a 00 00 00 00  |Track-Info......|
00000110  00 00 00 00 02 09 4e e5  00 00 c1 02 00 00 00 02  |......N.........|
00000120  00 00 c6 02 00 00 00 02  00 00 c2 02 00 00 00 02  |................|
00000130  00 00 c7 02 00 00 00 02  00 00 c3 02 00 00 00 02  |................|
00000140  00 00 c8 02 00 00 00 02  00 00 c4 02 00 00 00 02  |................|
00000150  00 00 c9 02 00 00 00 02  00 00 c5 02 00 00 00 02  |................|
00000160  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000200  00 43 4f 4d 50 49 4c 45  52 c2 49 4e 00 00 00 80  |.COMPILER.IN....|


Segunda pista:
00001400  54 72 61 63 6b 2d 49 6e  66 6f 0d 0a 00 00 00 00  |Track-Info......|
00001410  01 00 00 00 02 09 4e e5  01 00 c1 02 00 00 00 02  |......N.........|
00001420  01 00 c6 02 00 00 00 02  01 00 c2 02 00 00 00 02  |................|
00001430  01 00 c7 02 00 00 00 02  01 00 c3 02 00 00 00 02  |................|
00001440  01 00 c8 02 00 00 00 02  01 00 c4 02 00 00 00 02  |................|
00001450  01 00 c9 02 00 00 00 02  01 00 c5 02 00 00 00 02  |................|
00001460  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00001700  60 ed 5b f7 a5 b7 ed 52  22 0a 60 2a 06 60 11 00  |`.[....R".`*.`..|
00001710  60 19 ed 5b f5 a5 b7 ed  52 28 0f 38 0d 2a 06 60  |`..[....R(.8.*.`|


Diferencia de bytes entre pistas: 1400h-100h=1300=4864


Principio de pista, offset 0x18:

offset 	description 	bytes
00 	track (equivalent to C parameter in NEC765 commands) 	1
01 	side (equivalent to H parameter in NEC765 commands) 	1
02 	sector ID (equivalent to R parameter in NEC765 commands) 	1
03 	sector size (equivalent to N parameter in NEC765 commands) 	1
04 	FDC status register 1 (equivalent to NEC765 ST1 status register) 	1
05 	FDC status register 2 (equivalent to NEC765 ST2 status register) 	1
06 - 07 	notused (0) 	2

Estos 8 bytes son los de read id


ENTRY CONDITIONS
        B = Page for C000h (49152)...FFFFh (65535)
        C = Unit (0/1)
        D = Logical track, 0 base
        E = Logical sector, 0 base
        HL = Address of buffer
        IX = Address of XDPB

*/


	int iniciosector=traps_plus3dos_getoff_track_sector(reg_d,reg_e+1);


        int i;
	for (i=0;i<512;i++) {
		z80_byte byte_leido=plus3dsk_get_byte_disk(iniciosector+i);
        if (i<10) printf("Byte %d Valor %02XH\n",i,byte_leido);
		traps_poke_addr_page(reg_b,reg_hl+i,byte_leido);
	}

	traps_plus3dos_return_ok();

}


void traps_plus3dos_read_id(void)
{

/*
DD READ ID
016Fh (367)

Read a sector identifier.

ENTRY CONDITIONS
        C = Unit (0/1)
        D = Logical track, 0 base
        IX = Address of XDPB

EXIT CONDITIONS
        If OK:
                Carry true
                A = Sector number from identifier
        Otherwise:
                Carry false
                A = Error code
        Always:
                HL = Address of result buffer in page 7
                BC DE IX corrupt
                All other registers preserved

Parece que es el sector info de los dsk:

Track0
00 00 c1 02 00 00 00 02  sector 0
00 00 c2 02 00 00 00 02  sector 1
...

Track1
01 00 c1 02 00 00 00 02  sector 0
01 00 c2 02 00 00 00 02  sector 1
...

*/
	//???? Que retornar en A?
	int sector=0;
	debug_printf(VERBOSE_DEBUG,"READ ID: Unit: %d Track: %d",reg_c,reg_d);

	z80_byte sector_id=0xc0 | (sector+1);

	reg_a=sector_id;
	reg_hl=49152; 


        int i=0;
                        z80_byte *p;
                p=ram_mem_table[7];


                p[i]=reg_d;
		i++;

                p[i]=0;
		i++;

                p[i]=sector_id;
		i++;

                p[i]=2;
		i++;

                p[i]=0;
		i++;

                p[i]=0;
		i++;

                p[i]=0;
		i++;

                p[i]=2;
		i++;

	//Incrementar sector??? ni idea
	traps_plus3dos_return_ok();
}


void traps_plus3dos(void)
{

	if (!MACHINE_IS_SPECTRUM_P2A_P3) return;

	
		z80_byte rom_entra=((puerto_32765>>4)&1) + ((puerto_8189>>1)&2);

		if (rom_entra!=2) return;

		if (1) {

			//Mostrar llamadas a PLUS3DOS
			if (reg_pc>=256 && reg_pc<=412) debug_printf(VERBOSE_DEBUG,"PLUS3DOS routine jump table entry start. reg_pc=%d %04xH",reg_pc,reg_pc);

			int estrap=1;


			switch (reg_pc) {

				case 256:
					debug_printf(VERBOSE_DEBUG,"-----DOS INITIALISE");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					//traps_plus3dos_return_ok();
				break;

				case 0x062d:
				case 262:
					debug_printf(VERBOSE_DEBUG,"-----DOS OPEN");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					/*
						If file newly created:
		Carry true
		Zero true
		A corrupt
	If existing file opened:
		Carry true
		Zero false
		A corrupt
					*/

					//Z80_FLAGS=(Z80_FLAGS & (255-FLAG_Z));
					//traps_plus3dos_return_ok();

				break;

				case 0x0740:
				case 265:
					debug_printf(VERBOSE_DEBUG,"-----DOS CLOSE");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					//traps_plus3dos_return_ok();
				break;

				case 0x0761:
				case 268:
					debug_printf(VERBOSE_DEBUG,"-----DOS ABANDON");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					//traps_plus3dos_return_ok();
				break;

				case 271:
					debug_printf(VERBOSE_DEBUG,"-----DOS REF HEAD");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
/*
EXIT CONDITIONS
	If OK, but file doesn't have a header:
		Carry true
		Zero true
		A corrupt
		IX = Address of header data in page 7
	If OK, file has a header:
		Carry true
		Zero false
		A corrupt
		IX = Address of header data in page 7
		*/

					//Problema: Como asigno IX dentro de pagina 7? A saber
					//traps_plus3dos_handle_ref_head();
					


					//Z80_FLAGS=(Z80_FLAGS & (255-FLAG_Z));
					//traps_plus3dos_return_ok();
					
				break;

				case 274:
					debug_printf(VERBOSE_DEBUG,"-----DOS READ. Address: %d Length: %d",reg_hl,reg_de);
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					
					//traps_plus3dos_handle_dos_read();
					//traps_plus3dos_return_ok();
				break;

				case 286:
					debug_printf(VERBOSE_DEBUG,"-----DOS CATALOG");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
				break;

				case 0x08f2:
				case 289:
				//.l0121  jp      l08f2           ; DOS_FREE_SPACE
					debug_printf(VERBOSE_DEBUG,"-----DOS FREE SPACE");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
				break;

				case 334:
					debug_printf(VERBOSE_DEBUG,"-----DOS SET MESSAGE");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
				break;

				case 340:
					debug_printf(VERBOSE_DEBUG,"-----DOS MAP B");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
				break;

				case 355:
				case 0x197c:
				case 0x1bff:
					debug_printf(VERBOSE_DEBUG,"-----DD READ SECTOR track %d sector %d buffer %d xdpb: %d",
					reg_d,reg_e,reg_hl,reg_ix);	
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
/*
ENTRY CONDITIONS
	B = Page for C000h (49152)...FFFFh (65535)
	C = Unit (0/1)
	D = Logical track, 0 base
	E = Logical sector, 0 base
	HL = Address of buffer
	IX = Address of XDPB
	*/		
					dsk_show_activity(); //Aunque ya lo hace al encender motor, pero por si acaso
					traps_plus3dos_read_sector();
								
				break;
			

				case 349:
					debug_printf(VERBOSE_DEBUG,"-----DD SETUP");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
				break;

				case 346:
					debug_printf(VERBOSE_DEBUG,"-----DD INIT");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					//traps_plus3dos_return_ok();
				break;
			

				case 0x1f27:
				case 343:
					debug_printf(VERBOSE_DEBUG,"-----DD INTERFACE");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					//traps_plus3dos_return_ok();
				break;
			
				case 379:
					debug_printf(VERBOSE_DEBUG,"-----DD ASK 1");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					//traps_plus3dos_return_error();
				break;

				case 394:
				case 0x1d30:
					debug_printf(VERBOSE_DEBUG,"-----DD_L_DPB");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					//traps_plus3dos_dd_l_dpb();
				break;


				case 397:
				case 0x1f76:
					debug_printf(VERBOSE_DEBUG,"-----DD_L_SEEK");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					traps_plus3dos_dd_l_seek();
				break;


				case 406:
		                 case 0x212b:
                		        debug_printf(VERBOSE_DEBUG,"-----DD_L_ON_MOTOR");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
                		        dsk_show_activity();
					traps_plus3dos_return_ok();
				break;
                 		
				case 367:
				case 0x1c36:
					debug_printf(VERBOSE_DEBUG,"-----DD_READ_ID");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					traps_plus3dos_read_id();
				break;


		                case 0x2114:
                		        debug_printf(VERBOSE_DEBUG,"-----Undocumented Wait FD & Output");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);

					//realizando traps_plus3dos_return las lecturas funcionan pero write_sector no se llama nunca
					//traps_plus3dos_return();
		                break;

		                case 0x206f:
		                        debug_printf(VERBOSE_DEBUG,"-----Undocumented Wait FDC ready for new command");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);

					//realizando traps_plus3dos_return las lecturas funcionan pero write_sector no se llama nunca
					//traps_plus3dos_return();
				break;


		                case 0x1be9:
                		        debug_printf(VERBOSE_DEBUG,"-----Undocumented Subroutine to read A bytes from a sector");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
		                break;


//.l0166  jp      l1c0d           ; DD_WRITE_SECTOR
				case 358:
				case 0x1982:
				case 0x1c0d:
					debug_printf(VERBOSE_DEBUG,"-----DD_WRITE_SECTOR");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					
					dsk_show_activity(); //Aunque ya lo hace al encender motor, pero por si acaso
					traps_plus3dos_write_sector();
				break;


//.l0172  jp      l1e65           ; DD_TEST_UNSUITABLE
				case 370:
				case 0x1e65:
					debug_printf(VERBOSE_DEBUG,"-----DD_TEST_UNSUITABLE");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					traps_plus3dos_return_ok();
				break;


//l016c  jp      l1c24           ; DD_FORMAT
				case 364:
				case 0x1c24:
					debug_printf(VERBOSE_DEBUG,"-----DD_FORMAT");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
					dsk_show_activity(); //Aunque ya lo hace al encender motor, pero por si acaso
					traps_plus3dos_return_ok(); 
				break;



                 case 0x019f:
			debug_printf(VERBOSE_DEBUG,"-----DOS_INITIALISE");
					debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
		break;

                 case 0x01cd:
		break;




                 case 0x08b1:
		break;

                 case 0x10ea:
		break;

                 case 0x11fe:
		break;

                 case 0x11a8:
		break;

                 case 0x1298:
		break;

                 case 0x0a19:
		break;

                 case 0x0924:
		break;

                 case 0x096f:
		break;

                 case 0x1ace:
		break;

                 case 0x090f:
		break;

                 case 0x08fc:
		break;

                 case 0x1070:
		break;

                 case 0x108c:
		break;

                 case 0x1079:
		break;

                 case 0x01d8:
		break;

                 case 0x01de:
		break;

                 case 0x05c2:
		break;

                 case 0x08c3:
		break;

                 case 0x0959:
		break;

                 case 0x0706:
		break;

                 case 0x02e8:
			debug_printf(VERBOSE_DEBUG,"-----DOS_SET_MESSAGE");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
		break;
                 case 0x1847:
		break;

                 case 0x1943:
			debug_printf(VERBOSE_DEBUG,"-----DOS_MAP_B");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
			//.l0154  jp      l1943           ; DOS_MAP_B
		break;


                 case 0x1f32:
			debug_printf(VERBOSE_DEBUG,"-----DD_INIT");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
			//.l015a  jp      l1f32           ; DD_INIT
		break;

                 case 0x1f47:
			debug_printf(VERBOSE_DEBUG,"-----DD_SETUP");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
			//.l015d  jp      l1f47           ; DD_SETUP
		break;

                 case 0x1e7c:
		break;


                 case 0x1c16:
		break;


                 case 0x1c80:
			debug_printf(VERBOSE_DEBUG,"-----DD_LOGIN");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
			//.l0175  jp      l1c80           ; DD_LOGIN
		break;

                 case 0x1cdb:
			debug_printf(VERBOSE_DEBUG,"-----DD_SEL_FORMAT");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
			//.l0178  jp      l1cdb           ; DD_SEL_FORMAT
		break;

                 case 0x1edd:
			debug_printf(VERBOSE_DEBUG,"-----DD_ASK_1");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
			//.l017b  jp      l1edd           ; DD_ASK_1
		break;

                 case 0x1ee9:
			debug_printf(VERBOSE_DEBUG,"-----DD_DRIVE_STATUS");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
			//.l017e  jp      l1ee9           ; DD_DRIVE_STATUS
		break;

                 case 0x1e75:
		break;

                 case 0x1bda:
		break;

                 case 0x1cee:
			debug_printf(VERBOSE_DEBUG,"-----DD_L_XDPB");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
			//.l0187  jp      l1cee           ; DD_L_XDPB
		break;


                 case 0x20c3:
		break;

                 case 0x20cc:
		break;

                 case 0x2150:
			debug_printf(VERBOSE_DEBUG,"-----DD_L_T_OFF_MOTOR");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
			//.l0199  jp      l2150           ; DD_L_T_OFF_MOTOR
		break;

                 case 0x2164:
			debug_printf(VERBOSE_DEBUG,"-----DD_L_OFF_MOTOR");
			debug_printf(VERBOSE_DEBUG,"reg_pc=%d %04xH",reg_pc,reg_pc);
			//.l019c  jp      l2164           ; DD_L_OFF_MOTOR
		 break;


		default:
			estrap=0;
		break;






		}

			if (estrap) debug_printf(VERBOSE_DEBUG,"PLUS3DOS call. After trap table: reg_pc=%d %04xH",reg_pc,reg_pc);
			
		}

	
}



