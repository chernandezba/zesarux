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
#include <unistd.h>
#include <string.h>


#include "pd765.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "dsk.h"

z80_bit pd765_enabled={0};

z80_bit pd765_motor={0};

z80_byte pd765_index_write_command=0;
z80_byte pd765_index_read_command=0;

//En operacion de read data, dice que byte estamos leyendo
int pd765_read_byte_index=0;

z80_byte pd765_last_command_write=0;

//#define TEMP_VALUE_BUSY 5
//z80_byte pd765_disk_busy=0;

z80_byte pd765_srt=0;
z80_byte pd765_hut=0;
z80_byte pd765_hlt=0;
z80_byte pd765_nd=0;
z80_byte pd765_hd=0;
z80_byte pd765_us0=0;
z80_byte pd765_us1=0;

z80_byte pd765_st0=0;
z80_byte pd765_st1=0;
z80_byte pd765_st2=0;
z80_byte pd765_st3=0;


z80_byte pd765_pcn=0;

z80_byte pd765_ncn=0;

z80_byte pd765_sector=0;
z80_byte pd765_bytes_sector=2;

z80_byte pd765_eot=0;
z80_byte pd765_gpl=0;
z80_byte pd765_dtl=0;



z80_byte pdc_buffer_retorno[20000];
int pdc_buffer_retorno_len=0;
int pdc_buffer_retorno_index=0;

z80_bit plus3dos_traps={0};





#define kBusy 0x10
#define kExec 0x20
#define kIO 0x40
#define kRM 0x80

#define kInvalid 0x1f

#define kRvmUPD765StateIdle    0
#define kRvmUPD765StateRead    1
#define kRvmUPD765StateWrite   2
#define kRvmUPD765StateFormat  3
#define kRvmUPD765StateSeek    4
//#define kRvmUPD765StateResult  5
#define kRvmUPD765StateCommand 7

#define kRvmUPD765Seeking      8

#define kRvmUPD765Result 0x80

#define kRvmUPD765Halt 0xff

#define kDriveReady 0x20


#define kReadingData 0x11
#define kReadingDataEnd 0x12
#define kWritingData 0x13
#define kWritingDataEnd 0x14

#define kReadingTrackData 0x15

#define kScanningData 0x16
#define kScanningDataEnd 0x17

int dwstate=0,drstate=0;

int traps_plus3dos_getoff_track_sector(int pista_buscar,int sector_buscar);
z80_byte plus3dsk_get_byte_disk(int offset);


void pd765_debug_getstacktrace(int items)
{
	int i;
                        for (i=0;i<items;i++) {
                                z80_int valor=peek_byte_z80_moto(reg_sp+i*2)+256*peek_byte_z80_moto(reg_sp+1+i*2);
                                debug_printf(VERBOSE_DEBUG,"%04XH ",valor);
                        }
}

int pd765_get_index_memory(int indice)
{
	//ncn pista
	//sector


	//4864 bytes cada salto
	//Inicio 256
	int offset;

	offset=pd765_ncn*4864+pd765_sector*512+indice;
	offset +=256; //del principio de disco
	offset +=256; //que nos situa despues de la cabecera de la pista

	return offset;
}


void pd765_next_sector(void)
{
        pd765_sector++;

        //TODO Esto es totalmente arbitrario, para pistas de 9 sectores
        if (pd765_sector==9) {
                pd765_sector=0;
                pd765_ncn++;
        }
}


void pd765_read_sector(int indice_destino)
{
        //ncn pista
        //sector

	debug_printf(VERBOSE_DEBUG,"Reading full sector at track %d sector %d",pd765_ncn,pd765_sector);


    /* 

    Habria que leer el sector asi , buscando el id , y no calcular offset de manera matematica
	int iniciosector=traps_plus3dos_getoff_track_sector(pd765_ncn,pd765_sector);


        int i;
	for (i=0;i<512;i++) {
		z80_byte byte_leido=plus3dsk_get_byte_disk(iniciosector+i);
        pdc_buffer_retorno[indice_destino++]=byte_leido;
	}


    return;    

    */

  

	int offset=pd765_get_index_memory(0);
	pd765_next_sector();

	//copiamos 512 bytes

//z80_byte p3dsk_buffer_disco[200000];

//z80_byte pdc_buffer_retorno[20000];

	int i;
	z80_byte byte_leido;
	for (i=0;i<512;i++) {
		byte_leido=p3dsk_buffer_disco[offset++];
		pdc_buffer_retorno[indice_destino++]=byte_leido;
	}


}



int contador_recallibrate_temp=0;

//Se deberia inicializar a 128 al hacer reset
z80_byte pd765_status_register=128;





void pd765_enable(void)
{

	if (pd765_enabled.v) return;

	debug_printf (VERBOSE_INFO,"Enabling PD765");
	pd765_enabled.v=1;

	//Leer disco de prueba


}

void pd765_disable(void)
{

	if (pd765_enabled.v==0) return;

        debug_printf (VERBOSE_INFO,"Disabling PD765");
        pd765_enabled.v=0;
}




void pd765_motor_on(void)
{
	if (pd765_motor.v==0) {
		dsk_show_activity();
		pd765_motor.v=1;
	}
}

void pd765_motor_off(void)
{
	if (pd765_motor.v) {
		pd765_motor.v=0;
	}
}

int temp_operacion_pendiente=0;

void pd765_write_command(z80_byte value)
{
	if (pd765_index_write_command==0) debug_printf(VERBOSE_DEBUG,"------------------------Sending PD765 command: 0x%02X PC=0x%04X------------------------",value,reg_pc);
	else debug_printf(VERBOSE_DEBUG,"Sending PD765 command data 0x%02X index write: %d for previous command (0x%02X) PC=0x%04X",value,pd765_index_write_command-1,pd765_last_command_write,reg_pc);


	//Envio comando inicial
	if (pd765_index_write_command==0) {
		if (value==0x03) {
			debug_printf(VERBOSE_DEBUG,"PD765 command: specify");
			pd765_last_command_write=3;
			pd765_index_write_command=1;
			pd765_status_register= (pd765_status_register & 0xF) | kRM ; 
		}

		else if (value==0x04) {
                        debug_printf(VERBOSE_DEBUG,"PD765 command: sense drive status");
			pd765_last_command_write=4;
                        pd765_index_write_command=1;
			pd765_status_register= (pd765_status_register & 0xF) | kRM ; 
			
                }

		else if (value==7) {
			debug_printf(VERBOSE_DEBUG,"PD765 command: recalibrate");
			pd765_last_command_write=7;
                        pd765_index_write_command=1;
			pd765_status_register= (pd765_status_register & 0xF) | kRM ; 
			temp_operacion_pendiente=1;
                }

		else if (value==8) {
                        debug_printf(VERBOSE_DEBUG,"PD765 command: sense interrupt status");
                        pd765_last_command_write=8;
                        pd765_index_write_command=0; //no necesita parametros
			pd765_index_read_command=1;

			pd765_st0=(pd765_hd&1)<<2 | (pd765_us1&1) << 1 | (pd765_us0&1) | 32;
			debug_printf(VERBOSE_DEBUG,"us0: %d us1: %d",pd765_us0,pd765_us1);
			

			pd765_status_register= (pd765_status_register & 0xF) | kRM | kIO; 

			if (temp_operacion_pendiente==0) {
				//Decir comando incorrecto, ya que no habia ninguna operacion pendiente
				pd765_st0 |=128;
				debug_printf(VERBOSE_DEBUG,"No habia operacion pendiente. Indicar comando incorrecto en st0");
				
			}

			if (temp_operacion_pendiente==1) {
				temp_operacion_pendiente=0;
			}



			                  pdc_buffer_retorno_len=2;
                                        pdc_buffer_retorno_index=0;
                                        pdc_buffer_retorno[0]=pd765_st0;
                                        pdc_buffer_retorno[1]=pd765_pcn;
                                        pd765_status_register=(pd765_status_register & 0xf) |  kIO  | kRM; //???
                                        drstate=kRvmUPD765Result;
                                        dwstate=kRvmUPD765StateIdle;


                }

		else if ((value&15)==10) {
			debug_printf(VERBOSE_DEBUG,"PD765 command: read id");
			
			if (value&64) debug_printf(VERBOSE_DEBUG,"TODO multitrack");

			pd765_st0=(pd765_hd&1)<<2 | (pd765_us1&1) << 1 | (pd765_us0&1) | 32    ; //Indicar 32 de seek end
			//Cuando finaliza read id, bits de st0: 7 a 0, 6 a 1
			pd765_st0 |=64;

			pd765_st1=0;
			pd765_st2=0;
			pd765_st3=32; //de momento solo ready

			pd765_last_command_write=10;
			pd765_index_write_command=1;
			pd765_status_register=(pd765_status_register & 0xf) | kRM;
			temp_operacion_pendiente=1;
                }

		else if ((value&15)==6) {
			debug_printf(VERBOSE_DEBUG,"PD765 command: read data");
			
			if (value&128) debug_printf(VERBOSE_DEBUG,"TODO MT");
			if (value&64) debug_printf(VERBOSE_DEBUG,"TODO MF");
			if (value&32) debug_printf(VERBOSE_DEBUG,"TODO SK");

			pd765_last_command_write=6;
			pd765_index_write_command=1;
			pd765_status_register=(pd765_status_register & 0xf) | kRM;
			temp_operacion_pendiente=1;
		}

		else if (value==15) {
			debug_printf(VERBOSE_DEBUG,"PD765 command: seek");
                        pd765_last_command_write=15;
			pd765_index_write_command=1;
			temp_operacion_pendiente=1;
                }



		else {
			debug_printf(VERBOSE_DEBUG,"Unknown command");
			pd765_index_write_command=0; //Reseteamos a comando inicial
			
		}
	}

	//Envio datos asociados a comando
	else {
		switch (pd765_last_command_write) {
			case 3:
				//Specify
				if (pd765_index_write_command==1) {
					//SRT/HUD
					pd765_srt=(value>>4)&15;
					pd765_hut=value&15;
					debug_printf(VERBOSE_DEBUG,"Setting SRT: %d HUT: %d",pd765_srt,pd765_hut);
				}

				if (pd765_index_write_command==2) {
					//HLT/ND
					pd765_hlt=(value>>4)&15;
                                        pd765_nd=value&15;
					debug_printf(VERBOSE_DEBUG,"Setting HLT: %d ND: %d",pd765_hlt,pd765_nd);
				}
				
				pd765_index_write_command++;
				if (pd765_index_write_command==3) {
					pd765_index_write_command=0;
			                pd765_status_register=(pd765_status_register & 0xf) | kRM;
			                dwstate=kRvmUPD765StateIdle;
				}
			break;


			case 4:
				//Sense drive status
				if (pd765_index_write_command==1) {
					//HD US1 US0
					pd765_hd=(value>>2)&1;
					pd765_us1=(value>>1)&1;
					pd765_us0=value&1;
					debug_printf(VERBOSE_DEBUG,"Setting HD: %d US1: %d US0: %d",pd765_hd,pd765_us1,pd765_us0);
				}

				pd765_index_write_command++;

                                if (pd765_index_write_command==2) {
					pd765_index_write_command=0;
					//Y meter valor de retorno en lectura
					pd765_st3=32; //de momento solo ready
					if (pd765_us0 || pd765_us1) {
						//Decir no ready b:
						pd765_st3 &=(255-32);
					}
					pdc_buffer_retorno_len=1;
					pdc_buffer_retorno_index=0;
					pdc_buffer_retorno[0]=pd765_st3;
					pd765_status_register=(pd765_status_register & 0xf) | kRM | kIO | kBusy;
			                drstate=kRvmUPD765Result;
				        dwstate=kRvmUPD765StateIdle;
				}
					
			break;

			case 6:
				//Read data
				if (pd765_index_write_command==1) {
                                        //HD US1 US0
                                        pd765_hd=(value>>2)&1;
                                        pd765_us1=(value>>1)&1;
                                        pd765_us0=value&1;
                                        debug_printf(VERBOSE_DEBUG,"Setting HD: %d US1: %d US0: %d",pd765_hd,pd765_us1,pd765_us0);
                                }

				if (pd765_index_write_command==2) {
					pd765_ncn=value;

				}
				
				if (pd765_index_write_command==3) {
					pd765_hd=value;
				}
				
				if (pd765_index_write_command==4) {
					pd765_sector=value;
					//Parece que cuando dice "sector 1" quiere decir el 0
					//pd765_sector--;
				}
				
				if (pd765_index_write_command==5) {
					pd765_bytes_sector=value;
				}
				

				if (pd765_index_write_command==6) {
                                        pd765_eot=value;
				}

				if (pd765_index_write_command==7) {
                                        pd765_gpl=value;
				}

				if (pd765_index_write_command==8) {
                                        pd765_dtl=value;
				}


	
                                pd765_index_write_command++;
				if (pd765_index_write_command==9) {
					pd765_index_read_command=1;
					pd765_index_write_command=0;

                    //printf("Leer cilindro %d cabeza %d sector %d\n",pd765_ncn,pd765_hd,pd765_sector);
                    //sleep(3);

		                        pd765_st0=(pd765_hd&1)<<2 | (pd765_us1&1) << 1 | (pd765_us0&1) | 32    ; //Indicar 32 de seek end
                		        pd765_st1=0;
		                        pd765_st2=0;

	                                pdc_buffer_retorno_len=7;

					//Longitud de los datos de retorno es la cabecera (7) + la longitud del sector,
					//de momento forzado a 512. TODO
					pdc_buffer_retorno_len +=512;

					pdc_buffer_retorno_index=0;
                                        pdc_buffer_retorno[0]=pd765_st0;
                                        pdc_buffer_retorno[1]=pd765_st1;
                                        pdc_buffer_retorno[2]=pd765_st2;
                                        pdc_buffer_retorno[3]=pd765_ncn;
                                        pdc_buffer_retorno[4]=pd765_hd;
                                        pdc_buffer_retorno[5]=pd765_sector;
                                        pdc_buffer_retorno[6]=pd765_bytes_sector;

					pd765_read_sector(7);

                                        pd765_status_register=(pd765_status_register & 0xf) | kRM | kIO | kBusy;
                                        drstate=kRvmUPD765Result;
                                        dwstate=kRvmUPD765StateIdle;

					//prueba
					//pd765_disk_busy=TEMP_VALUE_BUSY;

                                }

			break;

			case 7:
				//Recalibrate
				if (pd765_index_write_command==1) {
					//US1 US0
					//debug_printf(VERBOSE_DEBUG,"Temporal no cambiamos us0 o us1");
					pd765_us1=(value>>1)&1;
                                        pd765_us0=value&1;
                                        debug_printf(VERBOSE_DEBUG,"Setting US1: %d US0: %d",pd765_us1,pd765_us0);
					pd765_ncn=0;
					pd765_sector=0;
					
                                }

				pd765_index_write_command++;
                                if (pd765_index_write_command==2) {
                                        pd765_index_write_command=0;
					//E irse al track 0
					pd765_pcn=0;
					pd765_st0=(pd765_hd&1)<<2 | (pd765_us1&1) << 1 | (pd765_us0&1) | 32    ; //Indicar 32 de seek end   
					contador_recallibrate_temp=0;
				}

			break;


			case 8:
				//No necesita parametros
				/*
                                      pdc_buffer_retorno_len=2;
					pdc_buffer_retorno_index=0;
                                        pdc_buffer_retorno[0]=pd765_st0;
                                        pdc_buffer_retorno[1]=pd765_pcn;
                                        pd765_status_register=(pd765_status_register & 0xf) |  kIO | kBusy; //???
                                        drstate=kRvmUPD765Result;
                                        dwstate=kRvmUPD765StateIdle;
				*/

			break;

			case 10:

				 //Read id
				if (pd765_index_write_command==1) {
                                        //HD US1 US0
                                        pd765_hd=(value>>2)&1;
                                        pd765_us1=(value>>1)&1;
                                        pd765_us0=value&1;
                                        debug_printf(VERBOSE_DEBUG,"Setting HD: %d US1: %d US0: %d",pd765_hd,pd765_us1,pd765_us0);
                                }

                                pd765_index_write_command++;
                                if (pd765_index_write_command==2) {
                                        pd765_index_write_command=0;
                                        pd765_st0=(pd765_hd&1)<<2 | (pd765_us1&1) << 1 | (pd765_us0&1)    ; //Indicar 32 de seek end
                                        pd765_st1=0;
                                        pd765_st2=0;

                                        pdc_buffer_retorno_len=7;
					pdc_buffer_retorno_index=0;
                                        pdc_buffer_retorno[0]=pd765_st0;
                                        pdc_buffer_retorno[1]=pd765_st1;
                                        pdc_buffer_retorno[2]=pd765_st2;
                                        pdc_buffer_retorno[3]=pd765_ncn;
                                        pdc_buffer_retorno[4]=pd765_hd;
                                        pdc_buffer_retorno[5]=pd765_sector;
                                        pdc_buffer_retorno[6]=pd765_bytes_sector;

                                        pd765_status_register=(pd765_status_register & 0xf) | kRM | kIO | kBusy;
                                        drstate=kRvmUPD765Result;
                                        dwstate=kRvmUPD765StateIdle;
				}

			break;

			case 15:
				//Seek
				if (pd765_index_write_command==1) {
                                        //HD US1 US0
                                        pd765_hd=(value>>2)&1;
                                        pd765_us1=(value>>1)&1;
                                        pd765_us0=value&1;
                                        debug_printf(VERBOSE_DEBUG,"Setting HD: %d US1: %d US0: %d",pd765_hd,pd765_us1,pd765_us0);
                                }

                                if (pd765_index_write_command==2) {
                                        //NCN
                                        pd765_ncn=value;
					pd765_sector=0;
                                        debug_printf(VERBOSE_DEBUG,"Setting NCN: %d",pd765_ncn);

					//Indicar que el disco esta en seek mode
					//temp pd765_status_register=1;

					//Y luego indicar que el present cylinder es ese mismo (nos posicionamos "instantaneamente");
					pd765_pcn=pd765_ncn;
                                }

                                pd765_index_write_command++;
                                if (pd765_index_write_command==3) {
					pd765_index_write_command=0;
					contador_recallibrate_temp=0;
					pd765_st0=(pd765_hd&1)<<2 | (pd765_us1&1) << 1 | (pd765_us0&1) | 32    ; //Indicar 32 de seek end

                                        //prueba
                                        //pd765_disk_busy=TEMP_VALUE_BUSY;


				}
			break;


			default:
				debug_printf(VERBOSE_DEBUG,"Sending data from unknown command: 0x%02X",pd765_last_command_write);
				
			break;
		}
	}

}

z80_byte temp_random_pd;

z80_byte temp_leer_dato;

z80_byte pd765_read_command(void)
{
	debug_printf(VERBOSE_DEBUG,"Read data after command index:  %d PC=0x%04X",pdc_buffer_retorno_index,reg_pc);
        z80_byte value;

			/*
                        if (pd765_index_read_command>=8) {
					//Retornar datos hasta que haya final. 512 bytes (sector size=2)
					int final_datos=512;
					if (pd765_read_byte_index>=final_datos) {
						pd765_status_register &=(255-64); //Final
						debug_printf(VERBOSE_DEBUG,"Fin leer datos");
						//Avanzamos numero de sector + cilindro
						pd765_next_sector();
					}
					else {
						debug_printf(VERBOSE_DEBUG,"Devolviendo dato indice: %d",pd765_read_byte_index);
						//Devolver datos de disco
						//value=temp_leer_dato++;
						value=pd765_get_disk_value(pd765_read_byte_index);
						//return pd765_p3dsk_buffer_disco[pd765_read_byte_index];
						pd765_read_byte_index++;
						
					}
			}

			if (pd765_index_read_command<=7) pd765_index_read_command++;

			//Ya no hay mas valores de retorno
			if (pd765_index_read_command==8 && pd765_last_command_write!=6) {
				//pd765_status_register &=(255-64); 
				//Quitar busy
				//pd765_status_register &=(255-16);
			}
			*/


    switch(drstate)
    {
      case kRvmUPD765Result:
	pdc_buffer_retorno_len--;
	if (!pdc_buffer_retorno_len) {
          pd765_status_register=(pd765_status_register & 0xf) | kRM;
          drstate=kRvmUPD765StateIdle;
	  debug_printf(VERBOSE_DEBUG,"fin datos retorno");
        }
        value=pdc_buffer_retorno[pdc_buffer_retorno_index++];
	break;
  
      case kReadingDataEnd:
      case kReadingData:
      case kReadingTrackData:
	pd765_status_register &=0x7f;
	//Retornando datos
	value=0xee; //TODO
	break;
        
      default:
        value=0xff;
      break;
    }

        debug_printf(VERBOSE_DEBUG,"Reading PD765 command result: return value: 0x%02X char: %c",value,
		(value>=32 && value<=127 ? value : '.') );

	return value;
}


z80_byte pd765_read_status_register(void)
{
	z80_byte value;

	value=pd765_status_register;

	//Si esta ocupado haciendo lecturas, devolver busy
	//if (pd765_disk_busy) {	
	//	value=31;
	//	pd765_disk_busy--;
	//	debug_printf(VERBOSE_DEBUG,"Reading PD765 status register disk busy");
	//}


	debug_printf(VERBOSE_DEBUG,"Reading PD765 status register: return value 0x%02X PC=0x%04X",value,reg_pc);
	debug_printf(VERBOSE_DEBUG,"Stack trace: ");
	pd765_debug_getstacktrace(20);
	debug_printf(VERBOSE_DEBUG,"");
	


	return value;
}

/*

Sentencia que ha hecho saltar el bucle y llegar a un comando 4A (read id)
-----------------
PD765 command: sense interrupt status

Reading PD765 status register: return value 0xC0 PC=0x209E

Read data after command PC=0x20A8
Read command after Sense interrupt status
Returning ST0. Reading PD765 command: 0xBF

Reading PD765 status register: return value 0xC0 PC=0x209E

Read data after command PC=0x20A8
Read command after Sense interrupt status
Returning PCN. Reading PD765 command: 0x00

Reading PD765 status register: return value 0x80 PC=0x209E

Reading PD765 status register: return value 0x80 PC=0x211C
------------------------
Sending PD765 command: 0x4A PC=0x2126
------------------------


Unknown command

Reading PD765 status register: return value 0x80 PC=0x211C


Descomentar linea
value=temp_random_pd--;
Para poder llegar aqui

Que sentido tiene que ST0 valga BF???
O es por culpa del PCN que vale 0???
*/





