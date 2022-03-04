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
OLD and unused source file for ZXPand emulation used before Charlie Robson explained me the technical details of the interface
Just kept it here for historical reasons: before those details, I made long debuggin sessions trying to deduce the protocol,
and I discovered aproximately 50% of it
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>



#include "zxpand.h"
#include "mmc.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "menu.h"
#include "screen.h"
#include "zx8081.h"

z80_bit zxpand_enabled={0};


z80_byte *zxpand_memory_pointer;

int zxpand_operating_counter=0;

//Si rom zxpand esta activa (1) o no (0)
z80_bit zxpand_overlay_rom={0};

enum zxpand_comandos {
	ZXPAND_FOPEN_READ,
	ZXPAND_FOPEN_WRITE,
	ZXPAND_OPEN_DIR,
	ZXPAND_CONFIG,
	ZXPAND_FREAD,
	ZXPAND_FCLOSE,
	ZXPAND_NEXT_ENTRY,
	ZXPAND_READ_JOYSTICK
};

//Ultimo comando enviado. 
enum zxpand_comandos zxpand_last_command;

//Indice al mensaje comando. -1 si no se ha recibido ninguno
int zxpand_index_message=-1;
char zxpand_buffer_comando[256];

//Indice al comando. -1 si no se ha recibido ninguno
int zxpand_index_command=-1;

//Todo el mensaje mas el 40, 1 del principio mas el FF del final no pueden ser mayor que 32. O sea el mensaje debe ser maximo 32-3=29
                           //1234567890123456789012345678901
			   //                         3.1-SN
char *zxpand_config_message="ZXpand ZEsarUX " EMULATOR_VERSION;

//Al leer directorio se usa zxpand_root_dir y zxpand_cwd
char zxpand_root_dir[PATH_MAX];
char zxpand_cwd[PATH_MAX]="";

/*
La rom del zxpand debe cargar en otra pagina de memoria aparte , y no en la zona de memoria_spectrum,
porque parece que esta rom no tiene tabla de caracteres, y dicha tabla de caracteres la ula la carga siempre de la rom principal
Por eso es que tenemos zxpand_memory_pointer=malloc(8192); y la rutina de peek byte de zx80 mira cuando zxpand esta enabled o no
*/

#define ZXPAND_ROM_NAME "zxpand.rom"


int zxpand_load_parameter_x=0;
int zxpand_load_parameter_address=-1;

//nombre del archivo abierto
char zxpand_archivo_abierto_nombre[PATH_MAX];

//tamanyo del archivo abierto
int zxpand_archivo_abierto_longitud;

void zxpand_alloc_mem(void)
{
       zxpand_memory_pointer=malloc(8192);
       if (zxpand_memory_pointer==NULL) cpu_panic ("Can not allocate memory for zxpand ROM");
}

void zxpand_footer_print_zxpand_operating(void)
{
        if (zxpand_operating_counter) {
                //color inverso
                menu_putstring_footer(WINDOW_FOOTER_ELEMENT_X_ZXPAND,1," ZXPAND ",WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);
        }
}

void zxpand_footer_zxpand_operating(void)
{

        //Si ya esta activo, no volver a escribirlo. Porque ademas el menu_putstring_footer consumiria mucha cpu
        if (!zxpand_operating_counter) {

                zxpand_operating_counter=2;
                zxpand_footer_print_zxpand_operating();

        }
}




void delete_zxpand_text(void)
{
    
                                                            // " ZXPAND "
        menu_putstring_footer(WINDOW_FOOTER_ELEMENT_X_ZXPAND,1,"        ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
}


//0 si ok
int zxpand_load_rom_overlay(void)
{
        FILE *ptr_zxpand_romfile;
        int leidos=0;

        //debug_printf (VERBOSE_INFO,"Loading zxpand rom %s",ZXPAND_ROM_NAME);
        printf ("Loading zxpand rom %s\n",ZXPAND_ROM_NAME);
        
        //ptr_zxpand_romfile=fopen(ZXPAND_ROM_NAME,"rb");
        open_sharedfile(ZXPAND_ROM_NAME,&ptr_zxpand_romfile);
        
        
        if (ptr_zxpand_romfile!=NULL) {
                
                leidos=fread(zxpand_memory_pointer,1,8192,ptr_zxpand_romfile);
                //leidos=fread(memoria_spectrum,1,8192,ptr_zxpand_romfile);
                fclose(ptr_zxpand_romfile);
                
        }
        
        
        
        if (leidos!=8192 || ptr_zxpand_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading ZXPAND rom. Disabling ZXpand");
		zxpand_disable();
                return 1;
        }

        return 0;
}


//void zxpand_load_normal_rom(void)
//{
//	rom_load(NULL);
//}

void zxpand_enable(void)
{

	//asignar memoria
	zxpand_alloc_mem();

	//Cuando hay reset (o hard reset) hay que volver a meter activa esta rom
	zxpand_overlay_rom.v=1;

	//Volver si error
	if (zxpand_load_rom_overlay()) return;

	//ZXpand habilita 32kb. En las direcciones 8-40k o bien 16-48k
	//de momento yo habilito desde 8-48k
	enable_ram_in_32768();
	ram_in_8192.v=1;


	zxpand_enabled.v=1;

	//root dir apunta a directorio actual
        getcwd(zxpand_root_dir,PATH_MAX);

	//directorio zxpand vacio
	zxpand_cwd[0]=0;


}

void zxpand_disable(void)
{
	zxpand_overlay_rom.v=0;
	//zxpand_load_normal_rom();
	zxpand_enabled.v=0;
}


FILE *ptr_zxpand_read_file_command=NULL;
int zxpand_read_file_leidos;

z80_byte temp_conta=0;
int temp_numero_archivos=0;

z80_byte temp_valor_retorno_desconocido=0;

z80_byte old_zxpand_read(void)
{

	//Activar footer de ZXpand, pero no si solo leemos el joystick
	if (zxpand_last_command!=ZXPAND_READ_JOYSTICK) zxpand_footer_zxpand_operating();

			//      	  12345678901234567890123456789012
//			char mensaje[33]="holaquetalcomoestas             ";
			char mensaje[33]="))))))))))))))))))))))))))))))))";


	z80_int longit;

	//segun ultimo comando
	//0 load   1 save    2 cat     3 config

	printf ("leer puerto: ultimo comando: %d indice: %d\n",zxpand_last_command,zxpand_index_command);

	switch (zxpand_last_command) {

		//load
		case ZXPAND_FOPEN_READ:

			

			zxpand_index_command++;


			if (zxpand_index_command==1) {
				//archivo a abrir esta en zxpand_buffer_comando
				sprintf (zxpand_archivo_abierto_nombre,"%s",zxpand_buffer_comando);

				printf ("abriendo archivo (%s)\n",zxpand_archivo_abierto_nombre);
				if (!si_existe_archivo(zxpand_archivo_abierto_nombre)) {
					printf ("no existe archivo\n");
					return 0x44;
				}

				ptr_zxpand_read_file_command=fopen(zxpand_archivo_abierto_nombre,"rb");
				zxpand_read_file_leidos=0;

				//Guardar longitud del archivo
				zxpand_archivo_abierto_longitud=get_file_size(zxpand_archivo_abierto_nombre);

				return 0x40;
			}

			//siguientes parametros es longitud?
//chernandez@supertux:~/docs cesar/spectrum/zesarux$ ls -l  PRUEBA.P 
//-rw-r--r-- 1 chernandez chernandez 12898 jul 27 12:33 PRUEBA.P
			longit=get_file_size(zxpand_buffer_comando);
			mensaje[0]=value_16_to_8l(longit);
			mensaje[1]=value_16_to_8h(longit);

			//parece que luego mete la direccion de inicio
//int zxpand_load_parameter_x=0;
//int zxpand_load_parameter_address=-1;

			z80_int inicio;

			if (zxpand_load_parameter_address==-1) {
				if (MACHINE_IS_ZX80) inicio=16384;
				if (MACHINE_IS_ZX81) inicio=16384+9;
			}

			else inicio=zxpand_load_parameter_address;
			
			printf ("direccion inicio: %d\n",inicio);
			//if (zxpand_index_command==2) sleep(1);

			mensaje[2]=value_16_to_8l(inicio);
			mensaje[3]=value_16_to_8h(inicio);

			//flags. 0
			mensaje[4]=0;
		

			//Esto lo va metiendo la rom en la direccion 16444 (PR_BUF)

                        if (zxpand_index_command>=2 && zxpand_index_command<=2+32-1) {
                                printf ("Devolviendo mensaje de load indice: %d hl: 0x%x de: 0%x\n",zxpand_index_command-2,reg_hl,reg_de);
                                return mensaje[zxpand_index_command-2];
                        }

						

		break;

                case ZXPAND_FOPEN_WRITE:
                        //save
			printf ("lectura de comando save no implementada\n");
                        sleep(5);
                        return 0;
                break;


		case ZXPAND_OPEN_DIR:
			//open dir
			printf ("Devolviendo opendir\n");
			zxpand_index_command++;

			if (zxpand_index_command==1) return 0x40;
		break;

		//config
		case ZXPAND_CONFIG:
			//Primer valor. letra de error con inverso al reves (normal con bit 7 alzado)
			//Devolviendo valor 4 se resetea
/*
despues de leer valor hace:
cp 40
ret z 
add a,3f
ld hl,fffb
adc hl,sp
ld (hl),cf

inc hl
ld (hl),a
dec hl
jp (hl)
*/
			//printf ("Valor: ");
			//scanf("%d",&valor);

			//return valor;
			zxpand_index_command++;
			//mensaje[31]=118;

			//Si metemos un 0x14, parece que hace un new con la direccion de ramtop que se le indica 
			//Esto corresponde a CONFIG "R=nnnn"
			//mensaje[0]=0x14;
			//[1],[2]=ramtop- va  a la 4004 4005 h
			//z80_int ramtop=16384+16384-100;
			//mensaje[1]=value_16_to_8l(ramtop);
			//mensaje[2]=value_16_to_8h(ramtop);


			//Si es un 1, hay mensaje. Acabado en FF
			//mensaje[0]=0;

			mensaje[0]=1;
			mensaje[31]=255;



			//Devolver 0x40 es un ok
			//Valores a partir de 0x41 son mensajes de error. aparecen con letra invertida
			if (zxpand_index_command==1) {
				return 0x40;		
			}


			if (zxpand_index_command==2) return 1;

			int long_texto_config=strlen(zxpand_config_message);

			if (zxpand_index_command>=3 && zxpand_index_command<=3+long_texto_config-1) {
				printf ("Devolviendo letra config indice: %d letra: %d\n",
						zxpand_index_command-3,zxpand_config_message[zxpand_index_command-3]);
				return ascii_to_zx81(zxpand_config_message[zxpand_index_command-3]);
			}

			//Final de texto
			if (zxpand_index_command>3+long_texto_config-1) return 0xff;


			//if (zxpand_index_command==1) return 0x49;		
			//if (zxpand_index_command>=2 && zxpand_index_command<=2+32-1) {
			//	printf ("Devolviendo mensaje de config indice: %d\n",zxpand_index_command-2);
			//	return mensaje[zxpand_index_command-2];		
			//}
		break;


		//desconocido de despues de load. Read bytes?
		case ZXPAND_FREAD:
                        zxpand_index_command++;
                        if (zxpand_index_command==1) return 0x40;

//chernandez@supertux:~/docs cesar/spectrum/zesarux$ ls -l  PRUEBA.P 
//-rw-r--r-- 1 chernandez chernandez 12898 jul 27 12:33 PRUEBA.P


			//Lee al principio en la memoria adicional ram 2000h-3fffh. Habilitarla!!
			//Primer byte va a parar a 2929h

			z80_byte byte_leido;
			if (ptr_zxpand_read_file_command!=NULL) {
				printf ("Devolviendo byte %d de archivo en comando read file pc: 0x%x hl: 0x%x de: 0x%x\n",
				zxpand_read_file_leidos++,reg_pc,reg_hl,reg_de);
				fread(&byte_leido,1,1,ptr_zxpand_read_file_command);
			}

			else {
				//Si no se ha abierto archivo, devolver 255
				byte_leido=255;
			}

			return byte_leido;




		break;

		case ZXPAND_FCLOSE: 
			//desconocido de despues de load bytes. Despues de cada bloque de 256. fclose?
			zxpand_index_command++;

			printf ("Devolver valor para comando fclose?\n");

			//Si devuelvo 0x40 quiere decir que desactiva overlay rom (como la X en LOAD "...;X")
			//Es conveniente cargar MAZOGS con X dado que en in de teclado entra en conflicto con puertos ZXPAND
                        //if (zxpand_index_command==1) return 0x40;

			//Ver si X
			if (zxpand_load_parameter_x) {
				printf ("Devolver 0x40 dado que se ha hecho LOAD con X\n");
				return 0x40;
			}


			//Devuelvo ok si no ha llegado al final
			//longit=get_file_size("PRUEBA.P");
			//if (zxpand_index_command==1 && zxpand_read_file_leidos<longit) return 0x0;
			//si es final, devolver 0xff
			//if (zxpand_index_command==1 && zxpand_read_file_leidos>=longit) return 0xff;


			//Valores negativos hace que se reintente
			if (zxpand_index_command==1) {
				//que devuelvo???
				//le suma 3f. 
				//return 0;
				//return 127;
				//return 0x0f;
				//return 0x40;
				//TODO. no se cual es el valor correcto a enviar
				//Devolver 0x40 hara desactivar la overlay rom
				//Valores negativos hace que se reintente el IN
				//A otros valores se le suma 3F y se llama a la rutina de gestion de errores
				//printf ("valor?\n");
				//int v;
				//scanf("%d",&v);
				//return v;

				//z80_byte v=temp_valor_retorno_desconocido;
				//temp_valor_retorno_desconocido++;
				//printf ("devolvemos valor desconocido: %d\n",v);
				//return v;

				//Cualquier valor retornado aqui excepto 0x40 genera un error de RST8
				//Valores negativos provocan reintentos del puerto
				//0x40 parece un ok, aunque esto provoca el envio de un out a puerto para desactivar overlay rom
				//ld bc, %1110000000000111
				//ld a,$b0
				//out (c),a
				//Cosa que me hace pensar que hay algo que determina que al leer esto se desactive realmente la rom o no
				//Desactivando el comando de desactivar overlay, me encuentro con que el CMD.P ya puede cargar archivos BMP
/*
carga bmp:

Devolviendo byte 1665 de archivo en comando read file pc: 0x1e31 hl: 0x9e81 de: 0xff01
Devolviendo valor 0x0
--fin load--
Out Port ZXpand 0x8007 value : 0x80 ( ), PC after=0x1e48
puerto_h: 0x80
ZXpand canal (3 bits superiores): 0x4 valor: 0x80
comando de despues de load, fclose????
In Port ZXpand 0x8007 asked, PC after=0x87e
leer puerto: ultimo comando: 5 indice: 0
Devolver valor para comando fclose?
Devolviendo valor 0x40
Out Port ZXpand 0xE007 value : 0xb0 (K), PC after=0x207
puerto_h: 0xe0
ZXpand canal (3 bits superiores): 0x7 valor: 0xb0
Comando config
No desactivamos overlay rom aunque hemos recibido peticion de ello




carga juego desde commander (que llama a X): (lo mismo que carga bmp despues de leer el archivo)


leer puerto: ultimo comando: 4 indice: 55
Devolviendo byte 10550 de archivo en comando read file pc: 0x1e31 hl: 0x693f de: 0xff02
Devolviendo valor 0x76
In Port ZXpand 0x2007 asked, PC after=0x1e31
leer puerto: ultimo comando: 4 indice: 56
Devolviendo byte 10551 de archivo en comando read file pc: 0x1e31 hl: 0x6940 de: 0xff01
Devolviendo valor 0x80
--fin load--
Out Port ZXpand 0x8007 value : 0x80 ( ), PC after=0x1e48
puerto_h: 0x80
ZXpand canal (3 bits superiores): 0x4 valor: 0x80
comando de despues de load, fclose????
In Port ZXpand 0x8007 asked, PC after=0x87e
leer puerto: ultimo comando: 5 indice: 0
Devolver valor para comando fclose?
Devolviendo valor 0x40
Out Port ZXpand 0xE007 value : 0xb0 (K), PC after=0x207
puerto_h: 0xe0
ZXpand canal (3 bits superiores): 0x7 valor: 0xb0
Comando config
No desactivamos overlay rom aunque hemos recibido peticion de ello


Carga desde basic con X al final
(lo mismo que carga bmp despues de leer el archivo)

Devolviendo byte 7402 de archivo en comando read file pc: 0x1e31 hl: 0x5cf3 de: 0xff01
Devolviendo valor 0x0
--fin load--
Out Port ZXpand 0x8007 value : 0x80 ( ), PC after=0x1e48
puerto_h: 0x80
ZXpand canal (3 bits superiores): 0x4 valor: 0x80
comando de despues de load, fclose????
In Port ZXpand 0x8007 asked, PC after=0x87e
leer puerto: ultimo comando: 5 indice: 0
Devolver valor para comando fclose?
Devolver 0x40 dado que se ha hecho LOAD con X
Devolviendo valor 0x40
Out Port ZXpand 0xE007 value : 0xb0 (K), PC after=0x207
puerto_h: 0xe0
ZXpand canal (3 bits superiores): 0x7 valor: 0xb0
Comando config
No desactivamos overlay rom aunque hemos recibido peticion de ello
Out Port ZXpand 0xE007 value : 0xa1 (5), PC after=0x88c
puerto_h: 0xe0
ZXpand canal (3 bits superiores): 0x7 valor: 0xa1
Comando config
In Port ZXpand 0xE007 asked, PC after=0x891
leer puerto: ultimo comando: 3 indice: 0
Devolviendo valor 0x40

*/



				return 0x40;


				//Devolver 0 genera siempre error rst 8, aunque no provoca que luego se llame a disable overlay rom.
				return 0;
			}


			//Devuelvo ok si no ha llegado al final
			if (zxpand_index_command==1 && zxpand_read_file_leidos<zxpand_archivo_abierto_longitud) return 0x0;
			//si es final, devolver ..? 0xff
                        if (zxpand_index_command==1 && zxpand_read_file_leidos>=zxpand_archivo_abierto_longitud) return 0xff;

			if (zxpand_index_command>=2) {
				//que retorno aqui??
				//return 0x40;
	
				//le suma 3f. por tanto, 3f+c1=00
				return 0xC1;
			}

			//Devuelvo codigo error
			//if (zxpand_index_command==1) return 0x49;


		break;


		//next entry
		case ZXPAND_NEXT_ENTRY:
			printf ("Devolviendo next entry. zxpand_index_command: %d\n",zxpand_index_command);
			zxpand_index_command++;

			//if (zxpand_index_command>1) sleep(2);

			//retornar 0x3f en el final
			//retornar 0x4x para mensajes de error
			//if (zxpand_index_command==1) return 33;

			if (zxpand_index_command==1) {
				if (temp_numero_archivos++>9) {
					printf ("--devolvemos no hay mas archivos. pc=0x%x\n",reg_pc);
					//sleep(5);
					return 0x3f;
				}
			}


			if (zxpand_index_command==1) return 0x40;

				char archivo_ficticio[PATH_MAX];
				sprintf (archivo_ficticio,"PRUEBA%d.P",temp_numero_archivos);
				if (temp_numero_archivos==1) {
					//eso es una carpeta. metemos final de texto
					sprintf (archivo_ficticio,"CARPETA");
				}

				//Prueba archivo numero 5 sera mas largo. Supuestamente 12 maximo.
				//Si es 10, el byte 11 es un espacio (el 0) y el byte 12 que es?
				//Si es 11, el byte 12 es un espacio
				//Si es 12, el byte 13 tiene espacio de final?
				if (temp_numero_archivos==3) {
					sprintf (archivo_ficticio,"1234567890123456");
				}


				//Prueba archivo bmp
				if (temp_numero_archivos==5) {
					sprintf (archivo_ficticio,"IMAGE.BMP");
				}
				
				//Prueba archivo texto
				if (temp_numero_archivos==6) {
					sprintf (archivo_ficticio,"PRUEBA.TXT");
				}
				

				int longit_arch=strlen(archivo_ficticio);

				//si pasa de 12, truncar a 12
				if (longit_arch>12) longit_arch=12;

			if (zxpand_index_command>=2 && zxpand_index_command<=2+longit_arch-1) {
				printf ("Devolviendo letra nombre\n");
						   //    12345678901
				//sleep (2);
				//return ascii_to_zx81('A');
				return ascii_to_zx81(archivo_ficticio[zxpand_index_command-2]);
			}

			if (zxpand_index_command==2+longit_arch) {
				//0 de final de archivo
				return 0;
			}



			//if (zxpand_index_command>=2+longit_arch && zxpand_index_command<=2+11) {
			//	//0 de final de archivo
			//	return 0;
			//}

			//Parece que los bits de tamanyo y estado van justo despues del nombre, no en posicion fija

			if (zxpand_index_command==2+longit_arch+1) {
				/*
   bit   4,(iy+FFLAGS)        ; bit 4 is set for a folder
   bit   0,(iy+FFLAGS)        ; bit 0 is set for a read-only file
				*/
				//flags
				printf ("Devolviendo flags de archivo\n");
				//Si es el 1, es carpeta
				if (temp_numero_archivos==1) return 16;
				return 0;
			}

			//Longitud son 3 bytes
			if (zxpand_index_command>=2+longit_arch+2 && zxpand_index_command<2+longit_arch+2+3) {
				//length
				//tamanyo simulador
				z80_int tamanyo=(temp_numero_archivos+1)*1000;

				//archivo 5 es image.bmp de 1666 bytes
				//if (temp_numero_archivos==5) tamanyo=1666;

				//tamanyo real
				if (si_existe_archivo(archivo_ficticio)) {
					printf ("retornando tamanyo real de archivo %s\n",archivo_ficticio);
					tamanyo=get_file_size(archivo_ficticio);
				}


				if (zxpand_index_command==2+longit_arch+2) return value_16_to_8l(tamanyo);
				if (zxpand_index_command==2+longit_arch+2+1) return value_16_to_8h(tamanyo);
				if (zxpand_index_command==2+longit_arch+2+2) return 0;
				
				return 12;
			}

			//cualquier otra cosa, retornamos 0
			return 0;
			//12b'name + 1b'flags + 3b'length

		break;
		

		case ZXPAND_READ_JOYSTICK:
			//leer joystick
			printf ("devolvemos comando leer joystick\n");
			//no quiero que esto genere texto footer de ZXPAND
			//sleep(3);
			return 0xFF;
		break;


		default:
			printf ("-----------retornando valor para comando desconocido: 0x%0x. Esto no deberia pasar pues se consideran "
				"todos los casos 0-7\n",zxpand_last_command);
			sleep(5);
			return 0;
	 	break;
	}

	return 0;

}

/*

Al iniciar zxpand.rom, devolviendo 255:

Loading zxpand rom zxpand-v66.bin
Out Port ZXpand 0xE007 value : 0xac, PC after=2fd
In Port ZXpand 0xE007 asked, PC after=301A

Si devolvemos 0, intenta cargar el menu:
Loading zxpand rom zxpand-v66.bin
Out Port ZXpand 0xE007 value : 0xac (G), PC after=2fd
In Port ZXpand 0xE007 asked, PC after=301
Devolviendo valor 0x0

Out Port ZXpand 0x7 value : 0xff (?), PC after=1f85
Out Port ZXpand 0x4007 value : 0x32 (M), PC after=1fde
Out Port ZXpand 0x4007 value : 0x2a (E), PC after=1fde
Out Port ZXpand 0x4007 value : 0x33 (N), PC after=1fde
Out Port ZXpand 0x4007 value : 0xba (U), PC after=1fde
Out Port ZXpand 0x4007 value : 0x0 ( ), PC after=1fe6
Out Port ZXpand 0x8007 value : 0x0 ( ), PC after=8a8
In Port ZXpand 0x8007 asked, PC after=87e
Devolviendo valor 0x1
Out Port ZXpand 0x8007 value : 0x80 ( ), PC after=1e48
In Port ZXpand 0x8007 asked, PC after=87e
Devolviendo valor 0x2





//Comando CONFIG "V"
Out Port ZXpand 0x7 value : 0xff, PC after=1f85
Out Port ZXpand 0x4007 value : 0xbb, PC after=1fde  (0xbb - 128 = 59 = V)
Out Port ZXpand 0x4007 value : 0x0, PC after=1fe6
Out Port ZXpand 0xE007 value : 0x0, PC after=1f3b
In Port ZXpand 0xE007 asked, PC after=87e


Al ejecutar comando extendido CAT
Out Port ZXpand 0x7 value : 0xff, PC after=1f85
Out Port ZXpand 0x4007 value : 0x0, PC after=1fe6
Out Port ZXpand 0x6007 value : 0x0, PC after=1ee2
In Port ZXpand 0x6007 asked, PC after=87e


Al cargar LOAD "ABCD"
Out Port ZXpand 0x7 value : 0xff, PC after=1f85
Out Port ZXpand 0x4007 value : 0x26, PC after=1fde
Out Port ZXpand 0x4007 value : 0x27, PC after=1fde
Out Port ZXpand 0x4007 value : 0x28, PC after=1fde
Out Port ZXpand 0x4007 value : 0xa9, PC after=1fde   (a9=0x29+128)
Out Port ZXpand 0x4007 value : 0x0, PC after=1fe6
Out Port ZXpand 0x8007 value : 0x0, PC after=8a8
In Port ZXpand 0x8007 asked, PC after=87e


Despues de Load , enviar nombre a cargar y recibir 32 bytes de datos hace:
Out Port ZXpand 0xA007 value : 0x37 (R), PC after=1e29
puerto_h: 0xa0
ZXpand canal (3 bits superiores): 0x5


Comando SAVE "ABCD"
Out Port ZXpand 0x7 value : 0xff, PC after=1f85
Out Port ZXpand 0x4007 value : 0x26, PC after=1fde
Out Port ZXpand 0x4007 value : 0x27, PC after=1fde
Out Port ZXpand 0x4007 value : 0x28, PC after=1fde
Out Port ZXpand 0x4007 value : 0xa9, PC after=1fde
Out Port ZXpand 0x4007 value : 0x0, PC after=1fe6
Out Port ZXpand 0x8007 value : 0x1, PC after=8a8
In Port ZXpand 0x8007 asked, PC after=87e


Comando CONFIG "ABCD", de manera similar:
Out Port ZXpand 0x7 value : 0xff, PC after=1f85
Out Port ZXpand 0x4007 value : 0x26, PC after=1fde
Out Port ZXpand 0x4007 value : 0x27, PC after=1fde
Out Port ZXpand 0x4007 value : 0x28, PC after=1fde
Out Port ZXpand 0x4007 value : 0xa9, PC after=1fde
Out Port ZXpand 0x4007 value : 0x0, PC after=1fe6
Out Port ZXpand 0xE007 value : 0x0, PC after=1f3b
In Port ZXpand 0xE007 asked, PC after=87e


De Zxpand commander:
http://www.sinclairzxworld.com/viewtopic.php?f=6&t=956

; wait for long command to finish, a has response code on return
;
api_responder  .equ $1ff6

; de = fname pointer, with optional start,length;  a = operation.: 0 = load, 1 = delete, 2 = rename, 80-ff = save
;
api_fileop     .equ $1ff8

; de = high bit terminated string, terminating zero will be sent
;
api_sendstring .equ $1ffa

; de = memory to xfer, l = len, a = mode: 0 = read, 1 = write
;
api_xfer       .equ $1ffc

; C has joy bits on return 7:5 := UDLRF---
;
api_rdjoy      .equ $1ffe




*/


void old_zxpand_write(z80_byte puerto_h,z80_byte value)
{

	
	//Supuestamente el canal esta en los 3 bits superiores
	printf ("puerto_h: 0x%x\n",puerto_h);
	z80_byte canal=(puerto_h>>5)&7;
	printf ("ZXpand canal (3 bits superiores): 0x%x valor: 0x%x\n",canal,value);

        //Activar footer de ZXpand, pero no si solo leemos el joystick
	if (! (canal==7 && value==0xA0)) {
		zxpand_footer_zxpand_operating();
	}


	switch (canal)
	{

		case 0:

//Inicializar envio mensaje
//Out Port ZXpand 0x7 value : 0xff (?), PC after=1f85
		if (value==0xFF) {
			printf ("Recibido inicio mensaje\n");
			zxpand_index_message=0;
		}			


		break;


                case 1:
                        printf ("---------canal no implementado 1\n");
                        sleep(5);
                break;



		case 2:
		//Meter texto en buffer
		if (zxpand_index_message>=0) {
			printf ("metiendo letra en buffer zxpand_buffer_comando\n");
			zxpand_buffer_comando[zxpand_index_message]=da_codigo_zx81_no_artistic(value&127);
			zxpand_index_message++;
			//Si es final, bit 7 alzado
			if ( (value&128) ) {
				zxpand_buffer_comando[zxpand_index_message++]=0;
				printf ("final de texto comando. texto recibido: %s\n",zxpand_buffer_comando);
				sleep(1);
			}
		}

		break;





		case 3:
			if (value==0) {
			printf ("open dir\n");
			zxpand_last_command=ZXPAND_OPEN_DIR; 
			zxpand_index_message=-1;
			zxpand_index_command=0;
			temp_numero_archivos=0;
			}

			else if (value==0xff) {
			printf ("next entry\n");
			zxpand_last_command=ZXPAND_NEXT_ENTRY;
                        zxpand_index_message=-1;
                        zxpand_index_command=0;
			}

			else {
				printf ("----------canal3 pero comando ni open dir ni next entry\n");
				sleep(5);
			}
		break;

		//Load   Out Port ZXpand 0x8007 value : 0x0, PC after=8a8
		//Save   Out Port ZXpand 0x8007 value : 0x1, PC after=8a8
		//Config Out Port ZXpand 0xE007 value : 0x0, PC after=1f3b
		//CAT    Out Port ZXpand 0x6007 value : 0x0, PC after=1ee2
		//Puertos de finalizacion de mensaje
		case 4:

			if (value==0) {
				printf ("comando load\n");
				zxpand_last_command=ZXPAND_FOPEN_READ; 
				zxpand_index_message=-1;
				zxpand_index_command=0;

				//Evaluamos parametros:
				//LOAD "filename"
				//LOAD "filename;X"
				//LOAD "filename;address"
				//zxpand
				//buscar hasta la ;
				//Por defecto no estan
				zxpand_load_parameter_x=0;
				zxpand_load_parameter_address=-1;

				char *encontrado;
				encontrado=strstr(zxpand_buffer_comando,";");
			
				if (encontrado!=NULL) {
					//Ver si es X o address
					//Ponemos 0 de final de texto
					*encontrado=0;
					encontrado++;
					if (*encontrado=='X') {
						printf ("encontrado X\n");
						zxpand_load_parameter_x=1;
					}
					else {
						printf ("encontrado address\n");
						zxpand_load_parameter_address=atoi(encontrado);
					}
				}

			}

			else if (value==1) {
                        printf ("comando save\n");
                        zxpand_last_command=ZXPAND_FOPEN_WRITE;
                        zxpand_index_message=-1;
                        zxpand_index_command=0;

			}

			else if (value==0x80) {
			printf ("comando de despues de load, fclose????\n");
			zxpand_last_command=ZXPAND_FCLOSE;
                        zxpand_index_message=-1;
                        zxpand_index_command=0;


                        }

			
		
			else {
				printf ("---------canal 4 pero comando no load ni save ni fin de load?\n");
				sleep(5);
			}

		break;


		case 5:
			printf ("comando leer bytes\n");
			zxpand_last_command=ZXPAND_FREAD;
			zxpand_index_message=-1;
                        zxpand_index_command=0;

		break;

		case 6:
			printf ("---------canal no implementado 6\n");
			sleep(5);
		break;


		case 7:

			printf ("Comando config\n");
			zxpand_last_command=ZXPAND_CONFIG; 
			zxpand_index_message=-1;
			zxpand_index_command=0;

			//B0 = 1011 0000
			if (value==0xB0) {
				printf ("Desactivar overlay rom hasta nuevo reset\n");
				zxpand_overlay_rom.v=0;
				//printf ("No desactivamos overlay rom aunque hemos recibido peticion de ello\n");
				//Nota: ver supuesto mensaje fclose 
				sleep(5);
			}

			if (value==0xA0) {
				printf ("leer joystick\n");
				zxpand_last_command=ZXPAND_READ_JOYSTICK;
				//sleep(3);
			}

		break;


		default:
			printf ("------------canal %d comando desconocido. Esto no deberia pasar pues se consideran "
                                "todos los casos 0-7\n",canal);			
			sleep(10);
		break;

	}

	//temp
	//if (value==0xAC) {
                //        printf ("Desactivar overlay rom hasta nuevo reset\n");
                //        zxpand_overlay_rom.v=0;
       //}

}

/*
>> -variable LATD holds the value returned to a IN opcode
>> -variable PORTD holds the value sent to a OUT opcode
>> -variable PORTA is the 8 bit high port address (>>5) send to a IN or OUT function 
*/
z80_byte zxpand_latd;
z80_byte zxpand_portd;
z80_byte zxpand_porta;

z80_byte zxpand_globalindex;
z80_byte zxpand_globaldata[256];

//usado en lectura de archivos
z80_int zxpand_globalAmount;

//donde guardar nombre fichero
z80_byte *fp_fn = &zxpand_globaldata[128];

//Variables al abrir fichero
z80_int zxpand_flags;
z80_int zxpand_start;
z80_int zxpand_length;





char *zxpand_texto_comandos[8]={
	"data channel",
	"get data from globalbuffer",
	"put data into globalbuffer",
	"directory control port",
	"file control port",
	"get data from file",
	"put data to file",
	"interface control"
};

char zxpand_defaultExtension;
z80_int zxpand_defaultLoadAddr;

char zx80Token2ascii[] = ";,()?-+*/???=<>";


char *SEMICOL = ";";
char* SEPARATOR = "=";

//usados al leer directorio
z80_byte zxpand_filinfo_fattrib;
struct dirent *zxpand_dp;
DIR *zxpand_dfd=NULL;


void zxpand_changedir(char *d);

void deZeddify(unsigned char* buffer)
{
   unsigned char q;
   while (*buffer)
   {
      q = *buffer;
      if (q & 0x40)
      {
         if (q > 0xd6 && q < 0xe6)
         {
            *buffer = zx80Token2ascii[q - 0xd7];
         }
         else
         {
            *buffer = '?';
         }
      }
      else
      {
         *buffer = da_codigo_zx81_no_artistic(q & 0x3f);
      }

      ++buffer;
   }
}

void zeddify(unsigned char *buffer)
{
   while (*buffer)
   {
      *buffer = ascii_to_zx81(*buffer);
      ++buffer;
   }
   // force 'zeddy' type termination
   *buffer = 255;
}




// check that the supplied ascii filename only consists of alphanums slash and dot.
char isValidFN(unsigned char* buffer)
{

	char c;

   while (*buffer)
   {
      //char c = chk_chr(validfns, *buffer);

	c=*buffer;
	
	if (
	     !
		(
			(c>='A' && c<='Z') ||
			(c>='0' && c<'9') ||
			(c=='/') ||
			(c==';') ||
			(c=='.')

		)
	   ) 
	{
		c=0;
	}
	
      if (c == 0 || c == ';') return c;
      ++buffer;
   }

   return 1;
}




//Funciones de gestion de comandos

//modo lectura o escritura
//rellena fullpath con ruta completa
z80_byte zxpand_fileopen(char *modo,char *fullpath)
{

   char* token;
   char autogenext = 1;
   char* p = (char*)zxpand_globaldata;

   deZeddify((unsigned char *)p);

   if (*p == '+' || *p == '>')
   {
      ++p;
   }

   if (*p == '/')
   {
      autogenext = 0;
   }

   if (!isValidFN((unsigned char*)p))
   {
      return 0x40 + ZXPAND_FR_INVALID_NAME;
   }

   //token = strtokpgmram(p, (RFC)SEMICOL);
   token = strtok(p, SEMICOL);
   if (token==NULL)
   {
      // no filename specified
      return 0x40 + ZXPAND_FR_INVALID_NAME;
   }

   // change $ for 0x7e, to support short LFN forms
   {
           if (p[6] == '$')
           {
                   p[6] = 0x7e;
           }
   }


   zxpand_start = zxpand_defaultLoadAddr;
   zxpand_length = 0;
   zxpand_flags = 0;

   // parse optional parameters
   //
   //while ((token = strtokpgmram((char*)NULL, (RFC)SEMICOL)) != NULL)
   while ((token = strtok((char*)NULL, SEMICOL)) != NULL)
   {
      // if it starts with an alpha then it's a flag - add it to the bitset
      if (isalpha(*token))
      {
         if (*token == 'X')
         {
            zxpand_flags |= 1;
         }
      }
      else
      {
         // see if it's of the form start,end: if not it's just start
         //
         char* comma = strchr(token,',');
         zxpand_start = atoi(token);
         if (comma)
         {
            zxpand_length = atoi(comma+1);
         }
      }
   }

   // now all the params are parsed, we can create the filename
   //
   {
      char* newFN = fp_fn;
      char found = 0;
      for(token = p; *token; ++token, ++newFN)
      {
         *newFN = *token;
         if (*token == '.')
         {
            found = 1;
         }
      }
      *newFN = 0;
      if (!found && autogenext)
      {
         *newFN = '.';
         ++newFN;
         *newFN = zxpand_defaultExtension;
         ++newFN;
         *newFN = 0;
      }
   }



	//TODO: habria que proteger que en el nombre indicado no se use ../.. para ir a ruta raiz inferior a zxpand_root_dir
	sprintf (fullpath,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,(char *)fp_fn);

	printf ("llamando fopen para nombre %s (fullpath: %s) con modo %s\n",fp_fn,fullpath,modo);
	
	ptr_zxpand_read_file_command=fopen(fullpath,modo);
	if (ptr_zxpand_read_file_command==NULL) {
		//file not found.
		return 0x40+ZXPAND_FR_NOT_FOUND;
	}

	return 0x40;


}

void zxpand_COM_FileOpenRead(void)
{

	printf ("zxpand_COM_FileOpenRead\n");
	char fullpath[PATH_MAX];
	z80_byte res=zxpand_fileopen("rb",fullpath);

  if (res==0x40)
   {
	long int longitud_total=get_file_size(fullpath);
	

      if (zxpand_length == 0)
      {
         zxpand_length = longitud_total & 65535;
      }

      // hack to make programs auto-disable ROM if read-only attribute is set
      //if (zxpand_filinfo_fattrib & AM_RDO)
      //{
      //   flags |= 1;
      //}

      zxpand_globaldata[0] = zxpand_length & 255;
      zxpand_globaldata[1] = zxpand_length / 256;
      zxpand_globaldata[2] = zxpand_start & 255;
      zxpand_globaldata[3] = zxpand_start / 256;
      zxpand_globaldata[4] = zxpand_flags & 255;
      zxpand_globaldata[5] = zxpand_flags / 256;

      zxpand_globaldata[6] = longitud_total & 0xff;
      zxpand_globaldata[7] = (longitud_total >>  8) & 0xff;
      zxpand_globaldata[8] = (longitud_total >> 16) & 0xff;
      zxpand_globaldata[9] = (longitud_total >> 24) & 0xff;

      memset(&zxpand_globaldata[10], 0, 32-10);

	printf ("zxpand_start: 0x%X zxpand_length: 0x%X zxpand_flags: 0x%X\n",zxpand_start,zxpand_length,zxpand_flags);

   }

   zxpand_latd = res;

}

void zxpand_COM_FileOpenWrite(void)
{
        char fullpath[PATH_MAX];
	z80_byte res=zxpand_fileopen("wb",fullpath);

//TODO si existe
/*
   if (res == 0x48)
   {
      // file exists
      if (zxpand_globaldata[0] == '+' || (fsConfig & 0x03) == 1)
      {
         char* p = fp_fnBak;
         memcpy((void*)fp_fnBak, (void*)fp_fn, 32);
         while(*p != '.'){++p;}
         strcpypgm2ram(p, (RFC)".BAK");
         res = 0x40 | f_rename(fp_fn, fp_fnBak);
      }

      if (zxpand_globaldata[0] == '>' || (fsConfig & 0x03) == 2)
      {
         // overwrite (ala dos pipe)
         res = 0x40 | f_unlink(fp_fn);
      }

      if (0x40 == res)
      {
         // now try again
         res = 0x40 | f_open(&fil, fp_fn, FA_CREATE_NEW|FA_WRITE);
      }
   }
*/

   if (res==0x40)
   {

	long int longitud_total;

	//Si no existe, tiene sentido esto??
      zxpand_globaldata[0] = zxpand_length & 255;
      zxpand_globaldata[1] = zxpand_length / 256;
      zxpand_globaldata[2] = zxpand_start & 255;
      zxpand_globaldata[3] = zxpand_start / 256;
      zxpand_globaldata[4] = zxpand_flags & 255;
      zxpand_globaldata[5] = zxpand_flags / 256;

      zxpand_globaldata[6] = longitud_total & 0xff;
      zxpand_globaldata[7] = (longitud_total >>  8) & 0xff;
      zxpand_globaldata[8] = (longitud_total >> 16) & 0xff;
      zxpand_globaldata[9] = (longitud_total >> 24) & 0xff;

	memset(&zxpand_globaldata[10], 0, 32-10);

   }

   zxpand_latd = res;

}

void zxpand_COM_FileClose(void)
{

	//TODO: cerrar archivo realmente??

	printf ("zxpand_COM_FileClose\n");


	if (ptr_zxpand_read_file_command!=NULL) {
		fclose(ptr_zxpand_read_file_command);
		ptr_zxpand_read_file_command=NULL;
	}

	zxpand_latd=0x40;

//   LATD=0x40 | f_close(&fil);

}

void zxpand_COM_FileRead(void)
{

  //esto no deberia pasar:
        if (ptr_zxpand_read_file_command==NULL) {
                printf ("trying to read from a non opened file. error\n");
		sleep(1);
                return;
        }


   z80_int read;

   if (zxpand_globalAmount == 0)
   {
      zxpand_globalAmount = 256;
   }

	printf ("zxpand_COM_FileRead. leyendo %d bytes\n",zxpand_globalAmount);

read=fread(&zxpand_globaldata,1,zxpand_globalAmount,ptr_zxpand_read_file_command);

	//de momento siempre devolver ok
	zxpand_latd=0x40;


   //zxpand_LATD = 0x40 | f_read(&fil, zxpand_globaldata, globalAmount, &read);

}

void zxpand_COM_FileWrite(void)
{

  //esto no deberia pasar:
	if (ptr_zxpand_read_file_command==NULL) {
		printf ("trying to write to a non opened file. error\n");
		sleep(1);
		return;
	}

   z80_int written;

   if (zxpand_globalAmount == 0)
   {
      zxpand_globalAmount = 256;
   }

	printf ("zxpand_COM_FileWrite. escribiendo %d bytes\n",zxpand_globalAmount);

written=fwrite(&zxpand_globaldata,1,zxpand_globalAmount,ptr_zxpand_read_file_command);

	printf ("escritos %d bytes\n",written);


        //de momento siempre devolver ok
        zxpand_latd=0x40;


//   LATD = 0x40 | f_write(&fil, (void*)zxpand_globaldata, globalAmount, &written);
}

void zxpand_COM_FileSeek(void)
{
        printf ("zxpand_COM_FileSeek not implemented\n");
	sleep(5);
}

void zxpand_COM_FileRename(void)
{
        printf ("zxpand_COM_FileRename not implemented\n");
	sleep(5);
}

void zxpand_COM_FileDelete(void)
{
   z80_byte ret;

	char fullpath[PATH_MAX];

   deZeddify(zxpand_globaldata);
   if (isValidFN(zxpand_globaldata))
   {

	//TODO: habria que proteger que en el nombre indicado no se use ../.. para ir a ruta raiz inferior a zxpand_root_dir
        sprintf (fullpath,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,zxpand_globaldata);

        printf ("llamando file delete nombre %s (fullpath: %s)\n",zxpand_globaldata,fullpath);

	ret=0x40;

	if (unlink(fullpath)) ret |=4;

   }

   zxpand_latd = ret;

}


//Comando Config
void zxpand_COM_ParseBuffer(void)
{
   char* token;
   z80_byte retcode = 0x40;

   // keep any raw keycodes that might be lost in a conversion
   //
   memcpy((void*)&zxpand_globaldata[128], (void*)&zxpand_globaldata[0], 128);

   deZeddify(zxpand_globaldata);

   if(!isalpha(zxpand_globaldata[0]))
   {
      //TODO retcode |= FR_INVALID_OBJECT;
	retcode |=0xE;
      goto retok;
   }

   // 'A' = 3.
   zxpand_globaldata[0] = zxpand_globaldata[0]-'A'+3;
   //token = strtokpgmram((char*)&zxpand_globaldata[1], (RFC)SEPARATOR);
   token = strtok((char*)&zxpand_globaldata[1], SEPARATOR);

   if (zxpand_globaldata[0] == 'V'-'A'+3)
   {
      // version string

      //strcpypgm2ram((char*)&zxpand_globaldata[1], (RFC)VERSION);

	//zxpand_config_message

	sprintf((char *) &zxpand_globaldata[1],"%s",zxpand_config_message);

      zeddify(&zxpand_globaldata[1]);
      zxpand_globaldata[0] = 1;
      goto retok;
   }

   else
   if (zxpand_globaldata[0] == 'D'-'A'+3)
   {
      // set current working directory

      if (!token)
      {
         zxpand_globaldata[32]='\\';
         zxpand_globaldata[33]=0;
         token = (char*)&zxpand_globaldata[32];
      }

      //retcode |= f_chdir(token);
	retcode=0x40;
	zxpand_changedir(token);
      goto retok;
   }

/*
   else if (zxpand_globaldata[0] == 'M'-'A'+3)
   {
      // memory map control

      if (token)
      {
         if (*token == 'H')
         {
            // HI ram
            GO_HIGH;
         }
         else if (*token == 'L')
         {
            // LO ram
            GO_LOW;
         }
         else
         {
            retcode |= FR_INVALID_OBJECT;
            goto retok;
         }
      }
      else
      {
         if (L_LOW)
         {
            strcpypgm2ram((char*)&zxpand_globaldata[1], (RFC)EIGHT40);
         }
         else
         {
            strcpypgm2ram((char*)&zxpand_globaldata[1], (RFC)SIXTEEN48);
         }

         zeddify(&zxpand_globaldata[1]);
         zxpand_globaldata[0] = 1;
      }

      goto retok;
   }
   else if (zxpand_globaldata[0] == 'C'-'A'+3)
   {
      // config control

      if (token)
      {
         unsigned char n = *token - '0';
         if (n > 9) n -= 7;
         if (n > 15)
         {
            retcode |= FR_INVALID_OBJECT;
            goto retok;
         }
         ++token;
         configByte = n * 16;
         n = *token - '0';
         if (n > 9) n -= 7;
         if (n > 15)
         {
            retcode |= FR_INVALID_OBJECT;
            goto retok;
         }
         configByte += n;
         WriteEEPROM(0x04, configByte);
      }
      else
      {
         unsigned char* p = &zxpand_globaldata[0];
         *p = 1;
         ++p;
         *p = ((configByte & 0xf0) >> 4) + 0x1c;
         ++p;
         *p = (configByte & 15) + 0x1c;
         ++p;
         *p = 0xff;
         ++p;
         *p = configByte;
      }

      goto retok;
   }
   else if (zxpand_globaldata[0] == 'O'-'A'+3)
   {
      // overwrite control
      if (token)
      {
         unsigned char n = *token - '0';
         if (n > 2)
         {
            retcode |= FR_INVALID_OBJECT;
            goto retok;
         }
         fsConfig = n;
         WriteEEPROM(0x05, fsConfig);
      }
      else
      {
         unsigned char* p = &zxpand_globaldata[0];
         *p = 1;
         ++p;
         if ((fsConfig & 3) == 1)
         {
            strcpypgm2ram((char*)p, (RFC)"BAK");
         }
         else if ((fsConfig & 3) == 2)
         {
            strcpypgm2ram((char*)p, (RFC)"OVR");
         }
         else
         {
            strcpypgm2ram((char*)p, (RFC)"ERR");
         }
         zeddify(p);
         p+= 3;
         *p = 0xff;
      }

      goto retok;
   }
   else if (zxpand_globaldata[0] == 'J'-'A'+3)
   {
      // joystick mapping control

      if (token)
      {
         token += 128;
         mapJS(0, *token);
         ++token;
         mapJS(1, *token);
         ++token;
         mapJS(2, *token);
         ++token;
         mapJS(3, *token);
         ++token;
         mapJS(4, *token & 0x3f); // might have top bit set being last char in string
         // might not. there might not be 4 chars there.
      }
      else
      {
         retcode |= FR_INVALID_OBJECT;
      }

      goto retok;
   }

*/



   // generic command follows? e.g.:
   // R(amtop),nnnn

   {
      z80_byte n = 0;

      //token = strtokpgmram((char*)&zxpand_globaldata[0], (RFC)SEPARATOR);
      token = strtok((char*)&zxpand_globaldata[0], SEPARATOR);
      if (token)
      {
         // parse optional parameters
         //
         while ((token = strtok((char*)NULL, SEPARATOR)) != NULL)
         {
            zxpand_start = atoi(token);

		printf ("posible comando config ramtop. valor: %d\n",zxpand_start);

            zxpand_globaldata[128+n] = zxpand_start & 255;
            zxpand_globaldata[129+n] = zxpand_start / 256;
            n += 2;
         }

         memcpy((void*)&zxpand_globaldata[1], (void*)&zxpand_globaldata[128], n);

         // values have been decoded to the buffer. parameterised commands may now execute



   /* comentado en origen
         // to ensure that a paramterised command has its parameters, put it inside this clause
         //
         if (zxpand_globaldata[0] == 'B'-'A'+3)
         {
            // set bank

            if (zxpand_globaldata[1])
            {
               GO_BANK1;
            }
            else
            {
               GO_BANK0;
            }
         }
         if (zxpand_globaldata[0] == 'E'-'A'+3)
         {
            retcode |= zxpand_globaldata[1];
         }
 fin comentado en origen  */


      }

      if (n == 0 && zxpand_globaldata[0] == 'R'-'A'+3)
      {
         //retcode |= FR_INVALID_OBJECT;
	retcode |=0xE;
      }
   }



retok:
   zxpand_latd = retcode;
}

//tener en cuenta raiz y directorio actual
//si localdir no es NULL, devolver directorio local (quitando zxpand_root_dir)
void zxpand_get_final_directory(char *dir, char *finaldir, char *localdir)
{


	printf ("zxpand_get_final_directory. dir: %s zxpand_root_dir: %s\n",dir,zxpand_root_dir);

	//Guardamos directorio actual del emulador
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

	//cambiar a directorio indicado, juntando raiz, dir actual de zxpand, y dir
	char dir_pedido[PATH_MAX];

	//Si directorio pedido es absoluto, cambiar cwd
	if (dir[0]=='/') {
		sprintf (zxpand_cwd,"%s",dir);
		sprintf (dir_pedido,"%s/%s",zxpand_root_dir,zxpand_cwd);
	}
		

	else {
		sprintf (dir_pedido,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,dir);
	}

	zvfs_chdir(dir_pedido);

	//Ver en que directorio estamos
	//char dir_final[PATH_MAX];
	getcwd(finaldir,PATH_MAX);
	printf ("directorio final total: %s\n",finaldir);

	//Ahora hay que quitar la parte del directorio raiz
	char *s=strstr(finaldir,zxpand_root_dir);

	if (s==NULL) {
		printf ("cambio de directorio no permitido\n");
		//directorio final es el mismo que habia
		sprintf (finaldir,"%s",zxpand_cwd);
		sleep(3);
		return;
	}

	//Si esta bien, meter parte local
	if (localdir!=NULL) {
		int l=strlen(zxpand_root_dir);
		sprintf (localdir,"%s",&finaldir[l]);
		printf ("directorio local: %s\n",localdir);
		sleep(1);
	}

	//printf ("directorio final local de zxpand: %s\n",finaldir);
		sleep(1);


	//Restauramos directorio actual del emulador
        zvfs_chdir(directorio_actual);
}


void zxpand_changedir(char *d)
{
        char directorio_final[PATH_MAX];
        //obtener directorio final

	//if (d[0]=='/')
	//zxpand_cwd

        zxpand_get_final_directory(d,directorio_final,zxpand_cwd);

}


//Abrir directorio.
void zxpand_COM_DirectoryOpen(void)
{
   char ret = 0x40 + ZXPAND_FR_INVALID_NAME;

   deZeddify(zxpand_globaldata);

   zxpand_filinfo_fattrib = 0; // normal dir mode

   if (*zxpand_globaldata == '>')
   {
      // we will change to the directory
      //TODO ret = 0x40 | f_chdir((char*)(zxpand_globaldata + 1));

	zxpand_changedir((char *)zxpand_globaldata + 1);
	ret=0x40;


      if (ret == 0x40)
      {
         // the CD succeeded, so instruct the 'read directory entry' routine to exit with 'all done'
         zxpand_filinfo_fattrib = 0x42;
      }
   }
   else if (*zxpand_globaldata == '+')
   {
      // we will try to create a directory
      //TODO ret = 0x40 | f_mkdir((char*)(zxpand_globaldata + 1));

        char directorio_actual[PATH_MAX];
        //obtener directorio final
        zxpand_get_final_directory("",directorio_actual,NULL);


	char directorio_final[PATH_MAX];
	sprintf (directorio_final,"%s/%s",directorio_actual,(char *) (&zxpand_globaldata[1]) );

	printf ("creando directorio %s (ruta total: %s)\n",(char *) (&zxpand_globaldata[1]),directorio_final);

	sleep(1);


#ifndef MINGW
     int tmpdirret=mkdir(directorio_final,S_IRWXU);
#else
        int tmpdirret=mkdir(directorio_final);
#endif

	//TODO 
	ret = 0x40;


      if (ret == 0x40)
      {
         // the CD succeeded, so instruct the 'read directory entry' routine to exit with 'all done'
         zxpand_filinfo_fattrib = 0x42;
      }
   }
   else
   {
      // Separate wildcard and path
      //TODO GetWildcard();

      if (isValidFN(zxpand_globaldata))
      {
         //ret = 0x40 + f_opendir(&dir, (const char*)&zxpand_globaldata[0]);

	//Si directorio es "", cambiar por "."
	/*if (zxpand_globaldata[0]==0) {
		zxpand_globaldata[0]='.';
		zxpand_globaldata[1]=0;
	}
	*/

	//Temp. Si directorio es "/", cambiar por "."
        /*if (!strcmp((char *)zxpand_globaldata,"/")) {
                zxpand_globaldata[0]='.';
                zxpand_globaldata[1]=0;
        }
	*/



	printf ("abriendo directorio %s\n",zxpand_globaldata);
	char directorio_final[PATH_MAX];
	//obtener directorio final
	zxpand_get_final_directory((char *) zxpand_globaldata,directorio_final,NULL);


	zxpand_dfd = opendir(directorio_final);

       if (zxpand_dfd == NULL) {
           debug_printf(VERBOSE_ERR,"Can't open directory %s", zxpand_globaldata);
           ret=0x40 + 5;
       }

	else ret=0x40;

      }
   }

   zxpand_latd = ret;
}


//comprobaciones de nombre de archivos en directorio
int zxpand_readdir_no_valido(char *s)
{

	//Si longitud mayor que 12 (8 nombre, punto, 3 extension)
	//if (strlen(s)>12) return 0;

	printf ("comprobando si valido nombre %s\n",s);


	char extension[NAME_MAX];
	char nombre[NAME_MAX];

	util_get_file_extension(s,extension);
	util_get_file_without_extension(s,nombre);

	//si nombre mayor que 8
	if (strlen(nombre)>8) return 0;

	//si extension mayor que 3
	if (strlen(extension)>3) return 0;

	int i;

	//si hay letras minusculas
	for (i=0;s[i];i++) {
		if (s[i]>='a' && s[i]<'z') return 0;
	}



	return 1;

}

void zxpand_COM_DirectoryRead(void)
{

	if (zxpand_dfd==NULL) {
		zxpand_latd=0x3f;
                return;
	}

	do {

		zxpand_dp = readdir(zxpand_dfd);
	
		if (zxpand_dp == NULL) {
			closedir(zxpand_dfd);
			zxpand_dfd=NULL;
			//no hay mas archivos
			zxpand_latd=0x3f;
			return;
		}


	} while(!zxpand_readdir_no_valido(zxpand_dp->d_name));


//if (isValidFN(zxpand_globaldata)

	int longitud_nombre=strlen(zxpand_dp->d_name);

        sprintf((char *) &zxpand_globaldata[0],"%s",zxpand_dp->d_name);

      zeddify(&zxpand_globaldata[0]);

	//nombre acabado con 0
	zxpand_globaldata[longitud_nombre]=0;

	//meter flags
	zxpand_filinfo_fattrib=0;
	//si directorio
	if (zxpand_dp->d_type==DT_DIR) zxpand_filinfo_fattrib |=16;


	int indice=longitud_nombre+1;

	zxpand_globaldata[indice++]=zxpand_filinfo_fattrib;

	//meter longitud
	long int longitud_total=get_file_size(zxpand_dp->d_name);

	//copia para ir dividiendo entre 256
	long int l=longitud_total;

	zxpand_globaldata[indice++]=l&0xFF;

	l=l>>8;
	zxpand_globaldata[indice++]=l&0xFF;

	l=l>>8;
	zxpand_globaldata[indice++]=l&0xFF;

	l=l>>8;
	zxpand_globaldata[indice++]=l&0xFF;

	zxpand_latd=0x40;

	
}


//Funcion de lectura/escritura de zxpand
void zxpand_process(int waswrite)
{

	void (*worker)(void) = NULL;


        //Activar footer de ZXpand, pero no si solo leemos el joystick
        if (! (zxpand_porta==7 && ( zxpand_portd==0xA0 || zxpand_portd==0xA1) )) {
                zxpand_footer_zxpand_operating();
        }


	switch (zxpand_porta) {
		// data channel
		case 0x00:
			// poke 0 here to prepare for reading,
			// poke nonzero here to prepare for writing to globalbuffer.
			if (waswrite) {
				if (zxpand_portd==0) {
					// initialise the output latch with the 1st globalbuffer byte,
					// set the read index to the next byte to be read
               				zxpand_latd = zxpand_globaldata[0];
					zxpand_globalindex = 1;
					//temp
					zxpand_globalindex = 0;

				}

				else if (zxpand_portd == 42) {
					//Dragons Lair hack
					zxpand_globalindex=0;
					//TODO
					printf ("dragons lair hack. not implemented\n");
				}

				else {
					// reset buffer index ready for writing
					zxpand_globalindex = 0;
				 }

			}
		break;


		// get data from globalbuffer
		case 0x01:
			// must have poked zero at port 0 before starting to peek data from here.
			zxpand_latd=(zxpand_globaldata[zxpand_globalindex]);
			printf ("Devolviendo valor 0x%02X de buffer index %d\n",zxpand_latd,zxpand_globalindex);
			zxpand_globalindex++;
		break;

		// put data into globalbuffer
		case 0x02:
			// must have poked nonzero at port 0 before starting to poke data to here.
			if (waswrite) {
				zxpand_globaldata[zxpand_globalindex] = zxpand_portd;
				zxpand_globalindex++;
			}
		break;


   // directory control port
   case 0x03:
      {
         if (waswrite)
         {
            zxpand_latd=0x80;

            if (zxpand_portd == 0u)
            {
               // reset the directory reader
               //
               worker = zxpand_COM_DirectoryOpen;
            }
            else
            {
               // get next directory entry
               //
               worker = zxpand_COM_DirectoryRead;
            }
         }
      }
      break;


   // file control port
   case 0x04:
      
         if (waswrite)
         {

	

            z80_byte pd = zxpand_portd;
            zxpand_latd=0x80;

            if (pd & 2) // ZX80 defaults to 'O' file extension and load address of 0x4000
            {
               zxpand_defaultExtension = 'O';
               zxpand_defaultLoadAddr = 0x4000;
            }
            else
            {
               zxpand_defaultExtension = 'P';
               zxpand_defaultLoadAddr = 0x4009;
            }

            // remove bit 1 - the zx80/zx81 flag / bit 1 set = zx80 file
            pd &= 0xfd;

            if (pd == 0)
            {
               // open the file with name in global data buffer for read access
               // indicate there are no bytes in the sector buffer
               // leaves 32 byte file info in global data: size (W), load address (W), flags (W) + 26 others
               //
               worker = zxpand_COM_FileOpenRead;
            }
            else if (pd == 1)
            {
               // open the file with name in global data buffer for write access
               // creating if necessary. global buffer not damaged in case a delete() is following
               //
               worker = zxpand_COM_FileOpenWrite;
            }
            else if (pd == 0x80)
            {
               // close the open file
               //
               worker = zxpand_COM_FileClose;
            }
            else if (pd == 0xd0)
            {
                                // seek information is already in the global buffer
                                //
               worker = zxpand_COM_FileSeek;
            }
            else if (pd == 0xe0)
            {
               // delete the file with name in global data buffer
               //
               worker = zxpand_COM_FileRename;
            }
            else if (pd == 0xf0)
            {
               // delete the file with name in global data buffer
               //
               worker = zxpand_COM_FileDelete;
            }
         }
      
      break;

   // get data from file
   case 0x05:
      {
         if (waswrite)
         {
            zxpand_latd=0x80;

            // read the next N bytes into the data buffer
            // PORTD = 0 means read 256 bytes.
            //
            zxpand_globalAmount = zxpand_portd;
            worker = zxpand_COM_FileRead;
         }
      }
      break;

   // put data to file
   case 0x06:
      {
         // write the next N bytes from the global data buffer
         // PORTD = 0 means write 256 bytes.
         //
         if (waswrite)
         {
            zxpand_latd=0x80;

            zxpand_globalAmount = zxpand_portd;
            worker = zxpand_COM_FileWrite;
         }
      }
      break;


   // interface control
   case 0x7:
         //
         if (waswrite)
         {
            unsigned char pd = zxpand_portd;

            zxpand_latd=0x80;

            if (pd == 0x00)
            {
               // parse a command buffer
               worker = zxpand_COM_ParseBuffer;
            }
            else
            /*  0xAn functions return something */
            if (pd == 0xa0)
            {
               // assemble joystick value/card detect and pop it into latd
               // 0b11111001
               //
		//TODO
		zxpand_latd=0xff;
	
               //char temp = GETJS;
               //LATD = temp & 0xf9;
            }

	/*
            else
            if (pd == 0xa1)
            {
               // fully decoded j/s - this command may take a few nanos.
               // used by INKEY$
               //
               decodeJS();
            }
                        else
            if (pd == 0xa1)
            {
               // fully decoded j/s - this command may take a few nanos.
               // used by INKEY$
               //
               decodeJS();
            }
                        else
            if (pd == 0xa2)
            {
                BYTE temp = ~GETJS;
                                BYTE fb = (temp & 16) << 1;
                                BYTE db = temp >> 4;
                                LATD = fb | db;
            }
            //else
            //if (pd == 0xaa) : see heartbeat, below
            else
            if (pd == 0xab)
            {
               if (mousex < 0) mousex = 0;
               if (mousex > 255) mousex = 255;
               if (mousey < 0) mousey = 0;
               if (mousey > 191) mousey = 191;
               zxpand_globaldata[0] = mouseSerialNum;
               zxpand_globaldata[1] = mousex;
               zxpand_globaldata[2] = mousey;
               zxpand_globaldata[3] = (PORTB & 8) | (PORTC & 128);
               zxpand_globaldata[4] = ~mouseSerialNum;
               ++mouseSerialNum;
               LATD = 0x40;
            }
            else
            if (pd == 0xac)
            {
               LATD = configByte;
            }
            else
            if (pd == 0xae)
            {
               // get EEPROM data; global data contains start,count
               BYTE i = 0;
               BYTE j = 2;
               BYTE k = zxpand_globaldata[0] + 5;
               while (i < zxpand_globaldata[1])
               {
                  zxpand_globaldata[j] = ReadEEPROM(k);
                  ++j;
                  ++k;
                  ++i;
               }
               LATD = 0x40;
            }
            else
            if (pd == 0xaf)
            {
               // return whether a card is present
               //
               LATD = CARDPRESENT();
            }

	*/

            else
            //  0xBn functions set something and may take time 
            if (pd == 0xb0)
            {
               // temporarily disable overlay until next zeddy reset
               //DISABLE_OVERLAY;
		zxpand_overlay_rom.v=0;
		printf ("Desactivar overlay rom hasta nuevo reset\n");
            }
            else
            if (pd == 0xb1)
            {
               // temporarily disable overlay until next zeddy reset
		//??disable or enable??
               //ENABLE_OVERLAY;
		zxpand_overlay_rom.v=1;
		printf ("Activamos overlay rom\n");
            }
/*
            else
            if (pd == 0xb2)
            {
               // M=L
               GO_LOW;
            }
            else
            if (pd == 0xb3)
            {
               // M=H
               GO_HIGH;
            }
            else
            if (pd == 0xb4)
            {
               GO_BANK0;
            }
            else
            if (pd == 0xb5)
            {
               GO_BANK1;
            }
            else
            if (pd == 0xb6)
            {
               GREENLEDON();
            }
            else
            if (pd == 0xb7)
            {
               GREENLEDOFF();
            }
            else
            if (pd == 0xb8)
            {
               REDLEDON();
            }
            else
            if (pd == 0xb9)
            {
               REDLEDOFF();
            }
            else
            if (pd == 0xba)
            {
               // enable mouse mode

               mousex = 0;
               mousey = 0;
               GOMOUSE;
            }
            else
            if (pd == 0xbb)
            {
               mousex = zxpand_globaldata[0];
               mousey = zxpand_globaldata[1];
            }
            else
            if (pd == 0xbc)
            {
               // set config byte from global data buffer
               //
               configByte = zxpand_globaldata[0];
               WriteEEPROM(0x04, configByte);
               LATD = 0x40;
            }
            else
            if (pd == 0xbe)
            {
               BYTE i = 0;
               BYTE j = 2;
               BYTE k = zxpand_globaldata[0] + 5;
               while(i < zxpand_globaldata[1])
               {
                  WriteEEPROM(k, zxpand_globaldata[j]);
                  ++j;
                  ++k;
                  ++i;
               }
               LATD = 0x40;
            }

*/

            else
            if (pd == 0xf0)
            {
               // delay a couple of milliseconds, then disable
               //TODO delay DelayMillis(2);
               //DISABLE_OVERLAY;
                zxpand_overlay_rom.v=0;
                printf ("Desactivar overlay rom hasta nuevo reset, con delay\n");
            }


/*

            else
            if (pd == 0xaa)
            {
               // heartbeat/debug 0
               LATD = 0xf0;
            }
            else
            if (pd == 0x55)
            {
               // heartbeat/debug 1
               LATD = 0x0f;
            }

            if ((pd & 0xf0) == 0xb0)
            {
               LATD = 0x40;
            }

	*/

         }
      break;






	}

	if (worker) {
		printf ("calling worker function %p\n",worker);
		worker();
	}

}


z80_byte zxpand_read(z80_byte puerto_h)
{
	zxpand_porta=(puerto_h>>5)&7;
	printf ("zxpand read operation. command: %d (%s)\n",zxpand_porta,zxpand_texto_comandos[zxpand_porta]);
	zxpand_process(0);
	printf ("zxpand read operation result: 0x%02X\n",zxpand_latd);
	return zxpand_latd;
}
void zxpand_write(z80_byte puerto_h,z80_byte value)
{
	zxpand_porta=(puerto_h>>5)&7;
	zxpand_portd=value;
	printf ("zxpand write operation. command: %d (%s) value: 0x%02X\n",zxpand_porta,zxpand_texto_comandos[zxpand_porta],zxpand_portd);
	zxpand_process(1);
}

