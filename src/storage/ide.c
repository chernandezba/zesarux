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
#include <dirent.h>


#include "ide.h"
#include "scorpion.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "zxvision.h"
#include "menu_items.h"
#include "screen.h"
#include "divide.h"


z80_bit ide_enabled={0};

z80_byte ide_drive=0;

z80_long_int ide_lba=0;

z80_byte ide_head=0;

#define IDE_STATUS_BUSY 128
#define IDE_STATUS_RDY  64
#define IDE_STATUS_DWF  32
#define IDE_STATUS_DSC  16
#define IDE_STATUS_DRQ  8
#define IDE_STATUS_CORR 4
#define IDE_STATUS_ERR  1

z80_byte ide_status_register=(IDE_STATUS_RDY|IDE_STATUS_DSC);

z80_byte ide_register_feature;
z80_byte ide_register_sector_count;
z80_byte ide_register_sector_number;
z80_byte ide_register_cylinder_low;
z80_byte ide_register_cylinder_high;
z80_byte ide_register_drive_head;
z80_byte ide_register_command;

#define IDE_SECTOR_SIZE 512


//Buffer de lectura de retorno de comando
#define IDE_MAX_RETURN_BUFFER IDE_SECTOR_SIZE
z80_byte ide_return_buffer[IDE_MAX_RETURN_BUFFER];

int ide_index_return_buffer=0;

/*
Temp de momento tarjeta de 32 MB
Ver apartado 2.6 Capacity Specifications

*/


//Probado IDE con:
// esxdos: no parece usar este comando. Obtiene la geometria según la información de la partición
// fatware: diferentes versiones. suele funcionar bien aunque a veces da problemas en el boot, no siempre
// demfir, gasware, mdos3, residos: ok
// atomlite (Sam coupe): boot ok. no he podido investigar mucho mas

z80_long_int ide_disk_sectors_card=62720;
int ide_disk_heads=4;
int ide_disk_sectors_track=32;
int ide_disk_cylinders=490;

z80_byte *ide_memory_pointer;

int ide_flash_must_flush_to_disk=0;


//0: primera tarjeta
//1: segunda tarjeta (no implementada)
int ide_card_selected=0;

//64 MB
long long int ide_size=64*1024*1024;

//Para que pueda leer tamaño inferior a lo esperado (los multiples de MB...)
long long int expected_read_ide_size=64*1024*1024;


int ide_write_sector_operation=0;
int ide_index_write_buffer=0;

char ide_file_name[PATH_MAX]="";


z80_bit ide_write_protection={0};


//Si cambios en escritura se hace flush a disco
z80_bit ide_persistent_writes={1};


int ide_set_image_parameters(void);
int ide_check_card_size(void);

/*Interfaz 8-bit simple
Se podria poner un archivo aparte para esto pero no vale la pena...
http://piters.tripod.com/simpif.htm
ROM +3e sm8



La controladora reacciona a cualquier acceso, ya sea en lectura o en escritura, de cualquier puerto con la línea A4=0.
El registro se selecciona con las líneas A2, A6 y A7, no importando el estado del resto de los bits.

I/O addresses for this solution:

Data register : #2B  	00 101 0 11

Parameter reg.: #2F	00 101 1 11

Sector count r.: #6B	01 101 0 11

Start sector r.: #6F	01 101 1 11

Cylinder low : #AB	10 101 0 11

Cylinder high : #AF	10 101 1 11

Head reg. : #EB		11 101 0 11

Command/status: #EF	11 101 1 11

*/

z80_bit eight_bit_simple_ide_enabled={0};


void ide_flush_flash_to_disk(void)
{

	if (ide_enabled.v==0) return;

    if (ide_flash_must_flush_to_disk==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush IDE to disk but no changes made");
        return;
    }

    if (ide_persistent_writes.v==0) {
        debug_printf (VERBOSE_DEBUG,"Trying to flush IDE to disk but persistent writes disabled");
        return;
    }


    debug_printf (VERBOSE_INFO,"Flushing IDE to disk");


    FILE *ptr_idefile;

	debug_printf (VERBOSE_INFO,"Opening IDE File %s",ide_file_name);
	ptr_idefile=fopen(ide_file_name,"wb");



    int escritos=0;
    long long int size;
    size=ide_size;


    if (ptr_idefile!=NULL) {
        z80_byte *puntero;
        puntero=ide_memory_pointer;

        //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
        //metera flush a 1
        ide_flash_must_flush_to_disk=0;

        escritos=fwrite(puntero,1,size,ptr_idefile);

        fclose(ptr_idefile);

    }

    //printf ("ptr_idefile: %d\n",ptr_idefile);
    //printf ("escritos: %d\n",escritos);

    if (escritos!=size || ptr_idefile==NULL) {
        debug_printf (VERBOSE_ERR,"Error writing to IDE file. Disabling write file operations");
        ide_persistent_writes.v=0;
    }

}

int ide_read_file_to_memory(void)
{
    if (ide_memory_pointer==NULL || ide_enabled.v==0)  {
        debug_printf(VERBOSE_ERR,"IDE is not enabled. You should not get this message");
        return 1;
    }

	FILE *ptr_idefile;
	unsigned int leidos=0;

    debug_printf (VERBOSE_INFO,"Opening IDE File %s",ide_file_name);
	ptr_idefile=fopen(ide_file_name,"rb");


	if (ptr_idefile!=NULL) {
        leidos=fread(ide_memory_pointer,1,expected_read_ide_size,ptr_idefile);
        fclose(ptr_idefile);
    }

    if (ptr_idefile==NULL) {
        debug_printf (VERBOSE_ERR,"Error opening ide file");
        return 1;
    }

    if (leidos!=expected_read_ide_size) {
        debug_printf (VERBOSE_ERR,"Error reading ide. Asked: %ld Read: %d",ide_size,leidos);
        return 1;
    }



return 0;
}

//Retorna 0 si ok
int ide_read_file(void)
{

	//Si habia memoria asignada, desasignar
	if (ide_memory_pointer!=NULL) free (ide_memory_pointer);
	ide_memory_pointer=NULL;


    ide_memory_pointer=malloc(ide_size);
    if (ide_memory_pointer==NULL) {
        cpu_panic ("No enough memory for ide emulation");
    }

    if (ide_read_file_to_memory()) {
        return 1;
    }


    if (ide_set_image_parameters() ) {
					return 1;
    }

    return 0;

}


void ide_insert(void)
{
	//Meter capacidad, etc en CSD
	//Cargar archivo en memoria
	if (ide_enabled.v==0) {
		return;
	}

	//Si existe
	if (!si_existe_archivo(ide_file_name)) {
		debug_printf (VERBOSE_ERR,"File %s does not exist",ide_file_name);
		ide_disable();
        return;
	}



	ide_size=get_file_size(ide_file_name);
	debug_printf (VERBOSE_DEBUG,"ide file size: %ld",ide_size);
    expected_read_ide_size=ide_size;

	if (ide_check_card_size() ){
		ide_disable();
        return;
    }


	if (ide_read_file()) {
		ide_disable();
		return;
	}

}

void ide_reset(void)
{
    //Resetear estado
    ide_write_sector_operation=0;
    ide_index_return_buffer=0;
    ide_index_write_buffer=0;

	//TODO resetear otros contadores a 0
	ide_register_sector_number=1;


	ide_register_feature=0;
	ide_register_sector_count=0;
	ide_register_cylinder_low=0;
	ide_register_cylinder_high=0;
	ide_register_drive_head=0;
	ide_register_command=0;



    ide_status_register=(IDE_STATUS_RDY|IDE_STATUS_DSC);

}

void ide_enable(void)
{
    if (ide_enabled.v) return;

    debug_printf (VERBOSE_INFO,"Enabling ide");
    ide_enabled.v=1;

	ide_reset();

    ide_memory_pointer=NULL;

    ide_insert();


}

void ide_disable(void)
{
    if (ide_enabled.v==0) return;

    //Hacer flush si hay algun cambio
    ide_flush_flash_to_disk();


    //Desactivar Divide ports
    //divide_ide_ports_disable();


    ide_enabled.v=0;

    //Si habia memoria asignada, desasignar
    if (ide_memory_pointer!=NULL) free (ide_memory_pointer);
    ide_memory_pointer=NULL;

}





//Mirar si tarjeta ide tiene tamanyo admitido
int ide_check_card_size(void)
{

    int size_mb=ide_size/1024/1024;

    switch (ide_size) {
		case 8*1024*1024:
		case 16*1024*1024:
		case 32*1024*1024:
		case 64*1024*1024:
		case 128*1024*1024:
		case 256*1024*1024:
		case 512*1024*1024:
		case 1024*1024*1024:
		break;


        default:
            if (size_mb<8) size_mb=8;
            else if (size_mb<16) size_mb=16;
            else if (size_mb<32) size_mb=32;
            else if (size_mb<64) size_mb=64;
            else if (size_mb<128) size_mb=128;
            else if (size_mb<256) size_mb=256;
            else if (size_mb<512) size_mb=512;
            else if (size_mb<1024) size_mb=1024;
            else {
                debug_printf (VERBOSE_ERR,"Invalid card size. Maximum 1024 MB");
                return 1;
            }

            //No mostramos esto como error por pantalla.
            debug_printf (VERBOSE_INFO,"Warning. Unexpected ide size. Should be one of: 8, 16, 32, 64, 128, 256, 512, 1024 MB. "
                                       "Rounding up to %d and continuing anyway, use at your own risk!",size_mb);
            ide_size=size_mb*1024*1024;

            //Asignaremos memoria como si fuera un tamaño estandard. Además si el archivo se escribe porque se ha modificado,
            //ya se normalizará ese tamaño

        break;
    }

    return 0;

}

//asignar parametros de sectores, cilindros, etc de la tarjeta
int ide_set_image_parameters(void)
{
	switch (ide_size) {
                case 8*1024*1024:
			ide_disk_sectors_card=15680;
			ide_disk_heads=2;
			ide_disk_sectors_track=32;
			ide_disk_cylinders=245;
		break;

                case 16*1024*1024:
			ide_disk_sectors_card=31360;
			ide_disk_heads=2;
			ide_disk_sectors_track=32;
			ide_disk_cylinders=490;
		break;


                case 32*1024*1024:
			ide_disk_sectors_card=62720;
			ide_disk_heads=4;
			ide_disk_sectors_track=32;
			ide_disk_cylinders=490;
		break;

                case 64*1024*1024:
			ide_disk_sectors_card=125440;
			ide_disk_heads=8;
			ide_disk_sectors_track=32;
			ide_disk_cylinders=490;
		break;

                case 128*1024*1024:
			ide_disk_sectors_card=250880;
			ide_disk_heads=8;
			ide_disk_sectors_track=32;
			ide_disk_cylinders=980;
		break;

                case 256*1024*1024:
			ide_disk_sectors_card=501760;
			ide_disk_heads=16;
			ide_disk_sectors_track=32;
			ide_disk_cylinders=980;
		break;

                case 512*1024*1024:
			ide_disk_sectors_card=1000944;
			ide_disk_heads=16;
			ide_disk_sectors_track=63;
			ide_disk_cylinders=993;
		break;

                case 1024*1024*1024:
			ide_disk_sectors_card=2001888;
			ide_disk_heads=16;
			ide_disk_sectors_track=63;
			ide_disk_cylinders=1986;
		break;


                default:
                        debug_printf (VERBOSE_ERR,"Invalid card size. Must be one of: 8, 16, 32, 64, 128, 256, 512, 1024 MB");
                        return 1;
                break;
        }

	debug_printf (VERBOSE_INFO,"Card size: %ld MB Sectors in card: %d Heads: %d Sectors/track: %d Cylinders: %d",
		ide_size/1024/1024,
		ide_disk_sectors_card,ide_disk_heads,ide_disk_sectors_track,ide_disk_cylinders);


	return 0;
}



void ide_footer_ide_operating(void)
{

	generic_footertext_print_operating("IDE");
    watermark_tell_device_activity();

	//Y poner icono en inverso
	if (!zxdesktop_icon_ide_inverse) {
			zxdesktop_icon_ide_inverse=1;
			menu_draw_ext_desktop();
	}
}



z80_byte ide_read_byte_memory(unsigned int address)
{

        //no se ha asignado memoria
        if (ide_memory_pointer==NULL) return 0xff;

        if (address>=ide_size) {
                //debug_printf (VERBOSE_ERR,"Error. Trying to read beyond card size. Size: %ld Asked: %u. Disabling IDE",ide_size,address);
                //ide_disable();
                //No desmontamos ni avisamos. Algunos firmwares para divide suelen leer mas alla de lo permitido
                return 0;
        }
        else return ide_memory_pointer[address];
}

void ide_write_byte_memory(unsigned int address,z80_byte value)
{

        //no se ha asignado memoria
        if (ide_memory_pointer==NULL) return;

        if (address>=ide_size) {
                debug_printf (VERBOSE_ERR,"Error. Trying to write beyond card size. Size: %ld Asked: %u. Disabling IDE",ide_size,address);
                ide_disable();
                return;
        }


        if (ide_write_protection.v) return;

        ide_memory_pointer[address]=value;
        ide_flash_must_flush_to_disk=1;
}

int ide_return_lba_pointer(int lba)
{
                                z80_long_int l;

                                z80_long_int v1,v2,v3,v4;
                                v1=ide_register_drive_head&15;
                                v2=ide_register_cylinder_high;
                                v3=ide_register_cylinder_low;

                                v4=ide_register_sector_number;

				if (lba) {
                	                l=(v1<<24)|(v2<<16)|(v3<<8)|v4;
                                debug_printf (VERBOSE_DEBUG,"LBA Address: %d=0x%X (%X %X %X %X) sector count: %d",l,l,v1,v2,v3,v4,ide_register_sector_count);
				}
				else {
					int cilindro=v2*256+v3;
					l=(cilindro*ide_disk_heads+v1)*ide_disk_sectors_track+(ide_register_sector_number-1);
/*
LBA = (C × HPC + H) × SPT + (S - 1)
https://en.m.wikipedia.org/wiki/Logical_block_addressing
*/
/*
z80_long_int ide_disk_sectors_card=62720;
int ide_disk_heads=4;
int ide_disk_sectors_track=32;
int ide_disk_cylinders=490;
*/
                                debug_printf (VERBOSE_DEBUG,"NON LBA Address: %d=0x%X (%X %X %X %X) sector count: %d",l,l,v1,v2,v3,v4,ide_register_sector_count);
				}

				//if (!lba) sleep(5);

                                //LLenamos IDE_SECTOR_SIZE bytes de memoria del buffer de retorno.
                                int address=l*IDE_SECTOR_SIZE;

		return address;
}



void ide_write_command_register(z80_byte value)
{

	int i;
	z80_byte drive;

	switch (value) {
		case 236:


			//Identify drive


                        drive=(ide_register_drive_head&16)>>4;

            debug_printf (VERBOSE_PARANOID,"Ata command identify drive %d",drive);

			//No hay disco numero 1. Solo el disco numero 0
                        if (drive==1) {
				for (i=0;i<IDE_MAX_RETURN_BUFFER;i++) ide_return_buffer[i]=0;
			}

			else { //Disco 0
			ide_return_buffer[0]=0x84;
			ide_return_buffer[1]=0x8A;
			ide_return_buffer[2]=value_16_to_8h(ide_disk_cylinders);
			ide_return_buffer[3]=value_16_to_8l(ide_disk_cylinders);
			ide_return_buffer[4]=0;
			ide_return_buffer[5]=0;
			ide_return_buffer[6]=value_16_to_8h(ide_disk_heads);
			ide_return_buffer[7]=value_16_to_8l(ide_disk_heads);
			ide_return_buffer[8]=0;
			ide_return_buffer[9]=0;
			ide_return_buffer[10]=0x02;
			ide_return_buffer[11]=0x40;
			ide_return_buffer[12]=value_16_to_8h(ide_disk_sectors_track);
			ide_return_buffer[13]=value_16_to_8l(ide_disk_sectors_track);


			ide_return_buffer[14]=(ide_disk_sectors_card>>8)&255;
			ide_return_buffer[15]=(ide_disk_sectors_card)&255;


			ide_return_buffer[16]=(ide_disk_sectors_card>>24)&255;
			ide_return_buffer[17]=(ide_disk_sectors_card>>16)&255;


			ide_return_buffer[18]=0;
			ide_return_buffer[19]=0;
			ide_return_buffer[20]=' ';
			ide_return_buffer[21]=' ';
			ide_return_buffer[22]=' ';
			ide_return_buffer[23]=' ';
			ide_return_buffer[24]=' ';
			ide_return_buffer[25]=' ';
			ide_return_buffer[26]=' ';
			ide_return_buffer[27]=' ';
			ide_return_buffer[28]=' ';
			ide_return_buffer[29]=' ';
			ide_return_buffer[30]=' ';
			ide_return_buffer[31]=' ';
			ide_return_buffer[32]='1';
			ide_return_buffer[33]='1';
			ide_return_buffer[34]='0';
			ide_return_buffer[35]='5';
			ide_return_buffer[36]='2';
			ide_return_buffer[37]='0';
			ide_return_buffer[38]='1';
			ide_return_buffer[39]='6';

			ide_return_buffer[40]=0x00;
			ide_return_buffer[41]=0x02;
			ide_return_buffer[42]=0x00;
			ide_return_buffer[43]=0x02;
			ide_return_buffer[44]=0x00;
			ide_return_buffer[45]=0x04;
			ide_return_buffer[46]='0';
			ide_return_buffer[47]='0';
			ide_return_buffer[48]='0';
			ide_return_buffer[49]='0';
			ide_return_buffer[50]='0';
			ide_return_buffer[51]='0';
			ide_return_buffer[52]='0';
			ide_return_buffer[53]='1';

			for (i=54;i<=93;i++) ide_return_buffer[i]=' ';

			ide_return_buffer[54]='Z';
			ide_return_buffer[55]='E';

			ide_return_buffer[56]='s';
			ide_return_buffer[57]='a';

			ide_return_buffer[58]='r';
			ide_return_buffer[59]='U';

			ide_return_buffer[60]='X';
			ide_return_buffer[61]=' ';

			ide_return_buffer[62]='I';
			ide_return_buffer[63]='D';

			ide_return_buffer[64]='E';
			ide_return_buffer[65]=' ';

			ide_return_buffer[94]=0x00;
			ide_return_buffer[95]=0x01;
			ide_return_buffer[96]=0x00;
			ide_return_buffer[97]=0x00;
			ide_return_buffer[98]=0x02;
			ide_return_buffer[99]=0x00;
			ide_return_buffer[100]=0x00;
			ide_return_buffer[101]=0x00;

			ide_return_buffer[102]=0x02;
			ide_return_buffer[103]=0x00;

			ide_return_buffer[104]=0x00;
			ide_return_buffer[105]=0x00;

			ide_return_buffer[106]=0x00;
			ide_return_buffer[107]=0x03;

			ide_return_buffer[108]=value_16_to_8h(ide_disk_cylinders);
            ide_return_buffer[109]=value_16_to_8l(ide_disk_cylinders);

			ide_return_buffer[110]=value_16_to_8h(ide_disk_heads);
            ide_return_buffer[111]=value_16_to_8l(ide_disk_heads);

            ide_return_buffer[112]=value_16_to_8h(ide_disk_sectors_track);
            ide_return_buffer[113]=value_16_to_8l(ide_disk_sectors_track);


            ide_return_buffer[114]=(ide_disk_sectors_card>>8)&255;
            ide_return_buffer[115]=(ide_disk_sectors_card)&255;

            ide_return_buffer[116]=(ide_disk_sectors_card>>24)&255;
            ide_return_buffer[117]=(ide_disk_sectors_card>>16)&255;



			ide_return_buffer[118]=0x01;
			ide_return_buffer[119]=0x00; //Multiple sector no lo hacemos valido

			ide_return_buffer[120]=(ide_disk_sectors_card>>8)&255;
			ide_return_buffer[121]=(ide_disk_sectors_card)&255;

			ide_return_buffer[122]=(ide_disk_sectors_card>>24)&255;
			ide_return_buffer[123]=(ide_disk_sectors_card>>16)&255;

			ide_return_buffer[124]=0x00;
			ide_return_buffer[125]=0x00;
			ide_return_buffer[126]=0x00;
			ide_return_buffer[127]=0x00;

			//64 en word es 128
			ide_return_buffer[128]=0x00;
			ide_return_buffer[129]=0x03;

			ide_return_buffer[130]=0x00;
			ide_return_buffer[131]=0x00;
			ide_return_buffer[132]=0x00;
			ide_return_buffer[133]=0x00;

			//67 en  word
			ide_return_buffer[134]=0x00;
			ide_return_buffer[135]=0x78;

			ide_return_buffer[136]=0x00;
			ide_return_buffer[137]=0x78;

			//69 en word

			for (i=138;i<=511;i++) ide_return_buffer[i]=0;

			}

            //Invertir todo a LSB
            //Nota: al contrario de lo que parece en las especificaciones, cada par de bytes están en formato LSB
            //Podría simplemente haber escrito todo lo anterior considerando LSB, pero para hacerlo mas fácil la correción
            //TODO: es probable que alguno de los valores anteriores siga siendo incorrecto

            z80_byte v1,v2;
            for (i=0;i<512;i+=2) {
                v1=ide_return_buffer[i];
                v2=ide_return_buffer[i+1];

                ide_return_buffer[i+1]=v1;
                ide_return_buffer[i]=v2;

            }

		break;


		case 0x20:
		case 0x21:
			debug_printf (VERBOSE_PARANOID,"Read Sector command");

			z80_byte lba_mode=(ide_register_drive_head&64)>>6;
                        drive=(ide_register_drive_head&16)>>4;
                        debug_printf (VERBOSE_PARANOID,"LBA mode %d drive %d",lba_mode,drive);



				int address=ide_return_lba_pointer(lba_mode);

				//LLenamos IDE_SECTOR_SIZE bytes de memoria del buffer de retorno.
				//TODO Gestionar multiple escritura de sectores
				int i;
				//printf ("Memory pointer read buffer: %d\n",address);



				for (i=0;i<IDE_SECTOR_SIZE;i++) {
					ide_return_buffer[i]=ide_read_byte_memory(address++);
				}


		break;

		case 0x30:
                        debug_printf (VERBOSE_PARANOID,"Write Sector command");
			ide_write_sector_operation=1;
			ide_index_write_buffer=0;
		break;

		case 0x91:
			debug_printf (VERBOSE_PARANOID,"Initialize Drive Parameters");
			/*
This command enables the host to set the number of sectors per track and the number of heads per cylinder.
Only the Sector Count and the Card/Drive/Head registers are used by this command.
NOTE: SanDisk recommends NOT using this command in any system because DOS determines the offset to the
Boot Record based on the number of heads and sectors per track.
If a CompactFlash Memory Card is “Formatted” with one head and sector per track value,
the same CompactFlash Card will not operate correctly with DOS configured with another
heads and sectors per track value.
			*/

			//Asi pues, no hacemos nada
            //decir status normal
            ide_status_register=(IDE_STATUS_RDY|IDE_STATUS_DSC);
		break;

		//temp para 8-bit simple con +3e roms
		case 0xA0:
			if (eight_bit_simple_ide_enabled.v) {
                                        //decir status normal
                                        //ide_status_register=(IDE_STATUS_RDY|IDE_STATUS_DSC|IDE_STATUS_BUSY);
                ide_status_register=(IDE_STATUS_RDY|IDE_STATUS_DSC);
			}
		break;

		//temp para 8-bit simple con +3e roms
		case 0xB0:
			if (eight_bit_simple_ide_enabled.v) {
				debug_printf (VERBOSE_PARANOID,"SMART DISABLE OPERATIONS - B0h. NOT implemented");
                ide_status_register=(IDE_STATUS_RDY|IDE_STATUS_DSC);
			}
		break;

		case 0xEF:
			debug_printf (VERBOSE_PARANOID,"Set features command. NOT implemented");
			debug_printf (VERBOSE_PARANOID,"Register 6: %02XH Register 2: %02XH Register 1: %02XH",
				ide_register_drive_head,ide_register_sector_count,ide_register_feature);
		break;

		default:
			debug_printf (VERBOSE_DEBUG,"Unknown ATA command 0x%02X",value);
		break;
	}
}

void ide_write_command_block_register(z80_byte ide_register,z80_byte value)
{

	if (ide_enabled.v==0) return;

	ide_footer_ide_operating();

	int indice;

	switch (ide_register) {

		case 0:
			//Escribir en buffer y luego a disco
			//TODO solo admite un sector cada vez

			if (ide_write_sector_operation) {

        	                //printf ("Writing in buffer index: %d\n",ide_index_write_buffer);
                	        indice=ide_index_write_buffer&(IDE_MAX_RETURN_BUFFER-1);
                        	ide_return_buffer[indice]=value;
				ide_index_write_buffer++;



				//Si estaba una operacion de escritura, escribir a memoria y activar flush
                	        //Si ha escrito ya IDE_SECTOR_SIZE

				if (ide_write_sector_operation && ide_index_write_buffer==IDE_SECTOR_SIZE) {
                	        	//decir status normal
                        		ide_status_register=(IDE_STATUS_RDY|IDE_STATUS_DSC);

	        	                z80_byte lba_mode=(ide_register_drive_head&64)>>6;
        	        	        z80_byte drive=(ide_register_drive_head&16)>>4;
                	        	debug_printf (VERBOSE_PARANOID,"LBA mode %d drive %d",lba_mode,drive);



                	                	int address=ide_return_lba_pointer(lba_mode);
						//printf ("Writing %d byte buffer to ide. lba address: %d\n",IDE_SECTOR_SIZE,address);

						int i;
						for (i=0;i<IDE_SECTOR_SIZE;i++) {
							ide_write_byte_memory(address+i,ide_return_buffer[i]);
						}



					ide_write_sector_operation=0;
        		                ide_index_write_buffer=0;
				}

			}

		break;

		case 1:
			ide_register_feature=value;
		break;

		case 2:
			ide_register_sector_count=value;
		break;

		case 3:
			ide_register_sector_number=value;
		break;

		case 4:
			ide_register_cylinder_low=value;
		break;

		case 5:
			ide_register_cylinder_high=value;
		break;


		case 6:
			ide_register_drive_head=value;

			//[ DRIVE/HEAD REGISTER (R/W) (too LBA bits 24..28) ]
			//xxxx xxxx  1011 1011, 0bbh, 187
			//1 LBA 1 DRV HS3 HS2 HS1 HS0
		break;

		case 7:
			ide_register_command=value;
			ide_status_register=(IDE_STATUS_RDY|IDE_STATUS_DSC|IDE_STATUS_DRQ);
			ide_index_return_buffer=0;
			ide_write_command_register(value);
		break;


		default:
			debug_printf (VERBOSE_DEBUG,"Unknown IDE register on write %d, value: %d",ide_register,value);
		break;
	}
}

z80_byte ide_get_data_register(void)
{
    int indice;

    indice=ide_index_return_buffer&(IDE_MAX_RETURN_BUFFER-1);
    return ide_return_buffer[indice];
}

z80_byte ide_get_error_register(void)
{
    //[ ERROR REGISTER (R) / FEATURES REGISTER (W) ]
    //xxxx xxxx  1010 0111, 0a7h, 167

    //De momento decimos que la interfaz nunca da error
    return 0;
}

z80_byte ide_read_command_block_register(z80_byte ide_register)
{


	z80_byte return_value;

	if (ide_enabled.v==0) return 255;


	ide_footer_ide_operating();

	//int indice;

        switch (ide_register) {

		case 0:
            //Data Register
			//printf ("Reading return buffer index: %d\n",ide_index_return_buffer);
            return_value=ide_get_data_register();
			//indice=ide_index_return_buffer&(IDE_MAX_RETURN_BUFFER-1);
			//return_value=ide_return_buffer[indice];
			ide_index_return_buffer++;

			//Este registro realmente es de 16 bits pero en divide y 8-bit simple ide, lee 8 bits solamente

			//Si ha leido ya IDE_SECTOR_SIZE, decir status normal
			if (ide_index_return_buffer==IDE_SECTOR_SIZE) ide_status_register=(IDE_STATUS_RDY|IDE_STATUS_DSC);

		break;

		case 1:
			return_value=ide_get_error_register();
		break;

        case 2:
                return_value=ide_register_sector_count;
        break;

        case 3:
                return_value=ide_register_sector_number;
        break;

        case 4:
                return_value=ide_register_cylinder_low;
        break;

        case 5:
                return_value=ide_register_cylinder_high;
        break;


        case 6:
                return_value=ide_register_drive_head;
		break;

		case 7:
//[ STATUS REGISTER (R) / COMMAND REGISTER (W) ]
//xxxx xxxx  1011 1111, 0bfh, 191

			//BUSY RDY DWF DSC DRQ CORR 0 ERR

                        //Prueba para interface 8-bit simple
/*
#define IDE_STATUS_BUSY 128
#define IDE_STATUS_RDY  64
#define IDE_STATUS_DWF  32
#define IDE_STATUS_DSC  16
#define IDE_STATUS_DRQ  8
#define IDE_STATUS_CORR 4
#define IDE_STATUS_ERR  1
*/


			return_value=ide_status_register;
			debug_printf (VERBOSE_PARANOID,"Returning status register: %d",return_value);
			//debug_printf (VERBOSE_PARANOID,"Returning status register: %d PC=%d contador=%d",return_value,reg_pc,temp_contador_tonto);

		break;

        }

	//printf ("Returning %d\n",return_value);
	return return_value;

}




void eight_bit_simple_ide_write(z80_byte port,z80_byte value)
{
	//El registro se selecciona con las líneas A2, A6 y A7, no importando el estado del resto de los bits.
	//Port: D7 D6 D5 D4 D3 D2 D1 D0
	//Reg:  D2 D1 X  X  X  D0 X  X
	z80_byte ide_register=((port&4)>>2) | ((port&192)>>5);
	//printf ("8bit ide write: register: %d value: %d\n",ide_register,value);
	ide_write_command_block_register(ide_register,value);

    //TODO ver si esto es solo para atom y 8-bit simple o es un fallo de mi emulacion IDE
    //ide_status_register=0x50;

}


z80_byte eight_bit_simple_ide_read(z80_byte port)
{
	z80_byte ide_register=((port&4)>>2) | ((port&192)>>5);
	//printf ("8bit ide read: register: %d\n",ide_register);
	z80_byte value=ide_read_command_block_register(ide_register);

    //TODO ver si esto es solo para atom y 8-bit simple o es un fallo de mi emulacion IDE
    //ide_status_register ^=0x08;

	return value;
}

void eight_bit_simple_ide_enable(void)
{
	eight_bit_simple_ide_enabled.v=1;
}

void eight_bit_simple_ide_disable(void)
{
	eight_bit_simple_ide_enabled.v=0;
}

/* SMUC (Scorpion & MOA Universal Controller) es la tarjeta de expansión
 * que proporciona la interfaz ATA/IDE para discos y el acceso a otros
 * periféricos. Las variables siguientes emulan sus registros y señales.
 *
 * La EEPROM 24LC16 es una memoria no volátil de 2 KiB del SMUC. El firmware
 * puede guardar y recuperar datos en ella mediante el bus serie SDA/SCL;
 * no forma parte de la RAM ni de la ROM paginada de la máquina.
 *
 * La CMOS es el conjunto de registros del reloj de tiempo real: incluye
 * hora, fecha, estado y una pequeña zona de configuración. En la tarjeta
 * física el reloj y sus registros pueden mantenerse con una pila cuando
 * se apaga el ordenador. Aquí se representan con 128 bytes y el estado
 * necesario para actualizar el reloj.
 */
/* Controla la presencia del SMUC, independientemente del disco IDE. */
z80_bit ide_smuc_enabled={0};
// Indica qué interfaz recibió el último comando ATA para la DMA de TSConf.
int ide_smuc_dma_selected=0;



/* FFBAh: D0 reinicia ATA, D4 es SDA, D6 es SCL y D7 selecciona
 * los registros alternativos ATA y la escritura en RTC. */
z80_byte ide_smuc_control=1;

/* 7FBAh: registro de configuración del SMUC; los bits 0-2 y 4-5
 * se leen siempre como unos en esta revisión del controlador. */
z80_byte ide_smuc_config;

/* D8BEh conserva el byte alto del último dato ATA de 16 bits. */
z80_byte ide_smuc_data_high;

/* DFBAh: dirección seleccionada y contenido de los registros CMOS. */
static z80_byte ide_smuc_cmos_address;

static z80_byte ide_smuc_cmos[128];

/* Estado de la interrupción de actualización del reloj, registro C. */
static time_t ide_smuc_cmos_last_second;

static int ide_smuc_cmos_update_flag;

/* Sustituye únicamente la palabra 0 de IDENTIFY por un disco ATA fijo. */
int ide_smuc_identify_first_word;

/* EEPROM 24LC16 y estado de la transferencia serie por SDA/SCL. */
static z80_byte ide_smuc_eeprom[2048];

/* Indica si el contenido de la EEPROM ya se inicializó al encender la máquina. */
int ide_smuc_eeprom_initialized;

/* Fase actual de la transferencia serie: comando, dirección, datos o ACK. */
int ide_smuc_eeprom_state;

/* Dirección de 11 bits del siguiente byte que se leerá o escribirá. */
int ide_smuc_eeprom_address;

/* Acumula los bits recibidos por SDA hasta formar un byte. */
int ide_smuc_eeprom_input;

/* Cuenta los bits recibidos del byte en curso. */
int ide_smuc_eeprom_input_bits;

/* Byte de la EEPROM que se desplaza para enviarlo por SDA. */
int ide_smuc_eeprom_output;

/* Cuenta los bits enviados del byte en curso. */
int ide_smuc_eeprom_output_bits;

/* Indica si la EEPROM ha soltado SDA para que lo controle el maestro. */
int ide_smuc_eeprom_release;

/* Valor visible al leer el bus serie: D6 representa la línea SDA. */
z80_byte ide_smuc_eeprom_bus=0xff;


enum {
    IDE_SMUC_EEPROM_IDLE,
    IDE_SMUC_EEPROM_COMMAND,
    IDE_SMUC_EEPROM_ADDRESS,
    IDE_SMUC_EEPROM_DATA,
    IDE_SMUC_EEPROM_SEND,
    IDE_SMUC_EEPROM_ACK
};
/* Procesa el bus serie de la EEPROM 24LC16 del SMUC.
 * Entrada: valor escrito en FFBAh. Salida: ninguna; actualiza SDA leído.
 */
static void ide_smuc_eeprom_write(z80_byte value)
{
    int old_clock=ide_smuc_control & 0x40;
    int new_clock=value & 0x40;
    int old_data=ide_smuc_control & 0x10;
    int new_data=value & 0x10;

    if (old_clock!=new_clock) {
        if (new_clock) {
            if (ide_smuc_eeprom_state==IDE_SMUC_EEPROM_ACK) {
                if (new_data) ide_smuc_eeprom_state=IDE_SMUC_EEPROM_IDLE;
                else {
                    ide_smuc_eeprom_state=IDE_SMUC_EEPROM_SEND;
                    ide_smuc_eeprom_output=
                        ide_smuc_eeprom[ide_smuc_eeprom_address];
                    ide_smuc_eeprom_address=(ide_smuc_eeprom_address+1)&0x7ff;
                    ide_smuc_eeprom_output_bits=0;
                }
            }
            else if (ide_smuc_eeprom_release &&
                     ide_smuc_eeprom_state>=IDE_SMUC_EEPROM_COMMAND &&
                     ide_smuc_eeprom_state<=IDE_SMUC_EEPROM_DATA) {
                ide_smuc_eeprom_input=(ide_smuc_eeprom_input<<1)|!!new_data;
                ide_smuc_eeprom_input_bits++;
            }
        }
        else {
            if (ide_smuc_eeprom_input_bits==8) {
                int data=ide_smuc_eeprom_input & 0xff;
                ide_smuc_eeprom_input=0;
                ide_smuc_eeprom_input_bits=0;
                if (ide_smuc_eeprom_state==IDE_SMUC_EEPROM_COMMAND) {
                    if ((data & 0xf0)!=0xa0) {
                        ide_smuc_eeprom_state=IDE_SMUC_EEPROM_IDLE;
                    }
                    else {
                        ide_smuc_eeprom_address=
                            (ide_smuc_eeprom_address & 0xff)|((data & 0x0e)<<7);
                        if (data & 1) {
                            ide_smuc_eeprom_state=IDE_SMUC_EEPROM_SEND;
                            ide_smuc_eeprom_output=
                                ide_smuc_eeprom[ide_smuc_eeprom_address];
                            ide_smuc_eeprom_address=(ide_smuc_eeprom_address+1)&0x7ff;
                            ide_smuc_eeprom_output_bits=0;
                        }
                        else ide_smuc_eeprom_state=IDE_SMUC_EEPROM_ADDRESS;
                    }
                }
                else if (ide_smuc_eeprom_state==IDE_SMUC_EEPROM_ADDRESS) {
                    ide_smuc_eeprom_address=
                        (ide_smuc_eeprom_address & 0x700)|data;
                    ide_smuc_eeprom_state=IDE_SMUC_EEPROM_DATA;
                }
                else if (ide_smuc_eeprom_state==IDE_SMUC_EEPROM_DATA) {
                    if (!(value & 0x20))
                        ide_smuc_eeprom[ide_smuc_eeprom_address]=data;
                    ide_smuc_eeprom_address=
                        (ide_smuc_eeprom_address & 0x7f0)|
                        ((ide_smuc_eeprom_address+1)&0x0f);
                }
                ide_smuc_eeprom_release=0;
                ide_smuc_eeprom_bus=0xbf;
                return;
            }
            if (ide_smuc_eeprom_state==IDE_SMUC_EEPROM_SEND) {
                if (ide_smuc_eeprom_output_bits==8) {
                    ide_smuc_eeprom_state=IDE_SMUC_EEPROM_ACK;
                    ide_smuc_eeprom_release=1;
                }
                else {
                    ide_smuc_eeprom_bus=
                        (ide_smuc_eeprom_output & 0x80) ? 0xff : 0xbf;
                    ide_smuc_eeprom_output<<=1;
                    ide_smuc_eeprom_output_bits++;
                    ide_smuc_eeprom_release=0;
                    return;
                }
            }
            else ide_smuc_eeprom_release=1;
        }
    }
    else if (new_clock && old_data!=new_data) {
        ide_smuc_eeprom_state=new_data ?
            IDE_SMUC_EEPROM_IDLE : IDE_SMUC_EEPROM_COMMAND;
        ide_smuc_eeprom_input=0;
        ide_smuc_eeprom_input_bits=0;
        ide_smuc_eeprom_release=1;
    }
    if (ide_smuc_eeprom_release)
        ide_smuc_eeprom_bus=new_data ? 0xff : 0xbf;
}

/* Convierte un valor del reloj a BCD si el registro B lo solicita.
 * Entrada: valor binario y registro B del RTC. Salida: valor para el Z80.
 */
static z80_byte ide_smuc_cmos_number(int number,z80_byte rtc_reg_b)
{
    if (rtc_reg_b & 4) return number;
    return (number / 10) * 16 + number % 10;
}

/* Lee el reloj del SMUC y sus registros de estado.
 * Entrada: ninguna. Salida: valor del registro RTC seleccionado.
 */
static z80_byte ide_smuc_cmos_read(void)
{
    time_t now=time(NULL);
    struct tm *clock=localtime(&now);
    z80_byte reg=ide_smuc_cmos_address & 0x7f;
    z80_byte rtc_reg_b=ide_smuc_cmos[11];

    if (ide_smuc_cmos_last_second &&
        now!=ide_smuc_cmos_last_second)
        ide_smuc_cmos_update_flag=1;
    ide_smuc_cmos_last_second=now;

    if (clock) {
        switch (reg) {
            case 0: return ide_smuc_cmos_number(clock->tm_sec,rtc_reg_b);
            case 2: return ide_smuc_cmos_number(clock->tm_min,rtc_reg_b);
            case 4: return ide_smuc_cmos_number(clock->tm_hour,rtc_reg_b);
            case 6: return clock->tm_wday+1;
            case 7: return ide_smuc_cmos_number(clock->tm_mday,rtc_reg_b);
            case 8: return ide_smuc_cmos_number(clock->tm_mon+1,rtc_reg_b);
            case 9: return ide_smuc_cmos_number(clock->tm_year%100,rtc_reg_b);
        }
    }
    if (reg==10) return 0x20 | (ide_smuc_cmos[10] & 0x0f);
    if (reg==11) return (rtc_reg_b & 4) | 2;
    if (reg==12) {
        z80_byte result=ide_smuc_cmos_update_flag ? 0x10 : 0;
        ide_smuc_cmos_update_flag=0;
        return result;
    }
    if (reg==13) return 0x80;
    return ide_smuc_cmos[reg];
}

/* Lee un puerto SMUC conectado al IDE de ZEsarUX.
 * Entrada: puerto Z80 y puntero al resultado. Salida: 1 si el puerto
 * pertenece al SMUC activo; 0 si debe seguir la lectura normal.
 */
int ide_smuc_read(z80_int port,z80_byte *value)
{
    if (!ide_smuc_enabled.v) return 0;
    if (MACHINE_IS_SCORPION_GMX) {
        if (scorpion_gmx_is_pentagon_scheme()) return 0;
    }
    else if (!MACHINE_IS_TSCONF) return 0;

    if ((port & 0x18a3)!=(0xfffe & 0x18a3)) return 0;
    if ((port & 0xa044)==(0x5fba & 0xa044)) {
        *value=(port & 4) ? 0x57 : 0x3f;
        return 1;
    }
    if ((port & 0xa044)==(0x7fba & 0xa044)) {
        *value=(port & 4) ? 0x57 : (ide_smuc_config | 0x37);
        return 1;
    }
    if ((port & 0xa044)==(0xdfba & 0xa044)) {
        *value=ide_smuc_cmos_read();
        return 1;
    }
    if ((port & 0xa044)==(0xffba & 0xa044)) {
        *value=ide_smuc_eeprom_bus;
        return 1;
    }
    if ((port & 0x8044)==(0xffbe & 0x8044)) {
        if (ide_smuc_control & 0x80) {
            *value=(port & 0x100) ? 0xff : ide_read_command_block_register(7);
            return 1;
        }
        if (!(port & 0x2000)) {
            *value=ide_smuc_data_high;
            return 1;
        }
        int reg=(port>>8)&7;
        if (reg==0) {
            *value=ide_read_command_block_register(0);
            ide_smuc_data_high=ide_read_command_block_register(0);
            if (ide_smuc_identify_first_word && ide_enabled.v) {
                // El firmware SMUC espera un disco ATA fijo; el IDE genérico anuncia CF.
                *value=0x40;
                ide_smuc_data_high=0;
                ide_smuc_identify_first_word=0;
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
int ide_smuc_write(z80_int port,z80_byte value)
{
    if (!ide_smuc_enabled.v) return 0;
    if (MACHINE_IS_SCORPION_GMX) {
        if (scorpion_gmx_is_pentagon_scheme()) return 0;
    }
    else if (!MACHINE_IS_TSCONF) return 0;

    if ((port & 0x18a3)!=(0xfffe & 0x18a3)) return 0;
    if ((port & 0xa044)==(0xffba & 0xa044)) {
        if (value & 1) ide_reset();
        ide_smuc_eeprom_write(value);
        ide_smuc_control=value;
        return 1;
    }
    if ((port & 0xa044)==(0xdfba & 0xa044)) {
        if (ide_smuc_control & 0x80)
            ide_smuc_cmos[ide_smuc_cmos_address & 0x7f]=value;
        else ide_smuc_cmos_address=value;
        return 1;
    }
    if ((port & 0xa044)==(0x7fba & 0xa044)) {
        ide_smuc_config=value;
        return 1;
    }
    if ((port & 0x8044)==(0xffbe & 0x8044)) {
        if (ide_smuc_control & 0x80) return 1;
        if (!(port & 0x2000)) {
            ide_smuc_data_high=value;
            return 1;
        }
        int reg=(port>>8)&7;
        if (reg==0) {
            ide_write_command_block_register(0,value);
            ide_write_command_block_register(0,ide_smuc_data_high);
        }
        else {
            if (reg==7) {
                ide_smuc_identify_first_word=(value==0xec && ide_enabled.v);
                if (MACHINE_IS_TSCONF) ide_smuc_dma_selected=1;
            }
            ide_write_command_block_register(reg,value);
        }
        return 1;
    }
    return 0;
}




// Restablece los registros volátiles del SMUC y conserva su EEPROM.
void ide_smuc_reset(void)
{
    ide_smuc_dma_selected=0;
    ide_smuc_control=1;
    ide_smuc_config=0;
    ide_smuc_data_high=0;
    ide_smuc_cmos_address=0;
    ide_smuc_cmos_last_second=0;
    ide_smuc_cmos_update_flag=0;
    ide_smuc_identify_first_word=0;
    ide_smuc_eeprom_state=IDE_SMUC_EEPROM_IDLE;
    ide_smuc_eeprom_address=0;
    ide_smuc_eeprom_input=0;
    ide_smuc_eeprom_input_bits=0;
    ide_smuc_eeprom_output=0;
    ide_smuc_eeprom_output_bits=0;
    ide_smuc_eeprom_release=1;
    ide_smuc_eeprom_bus=0xff;
    if (!ide_smuc_eeprom_initialized) {
        int i;
        for (i=0;i<2048;i++) ide_smuc_eeprom[i]=0xff;
        ide_smuc_eeprom_initialized=1;
    }
}
