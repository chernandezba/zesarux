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

DSK emulation

*/


#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#include "dsk.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "tape.h"
#include "menu_items.h"
#include "screen.h"
#include "pd765.h"


char dskplusthree_file_name[PATH_MAX]="";

z80_bit dskplusthree_write_protection={0};

int dskplusthree_must_flush_to_disk=0;


//Si cambios en escritura se hace flush a disco
z80_bit dskplusthree_persistent_writes={1};


z80_byte p3dsk_buffer_disco[DSK_MAX_BUFFER_DISCO];
int p3dsk_buffer_disco_size=DSK_MAX_BUFFER_DISCO; //Tamanyo del dsk leido. De momento establecemos en maximo

void dsk_insert_disk(char *nombre)
{
                strcpy(dskplusthree_file_name,nombre);
                
    if (noautoload.v==0) {
		reset_cpu();
	}
}


z80_bit dskplusthree_emulation={0};

//Zona de memoria dsk sector. Offset inicio, tamanyo y si esta activo
int dsk_memory_zone_dsk_sector_start=0;
int dsk_memory_zone_dsk_sector_size=0;
z80_bit dsk_memory_zone_dsk_sector_enabled={0};




void dskplusthree_flush_contents_to_disk(void)
{

        if (dskplusthree_emulation.v==0) return;

        if (dskplusthree_must_flush_to_disk==0) {
                debug_printf (VERBOSE_DEBUG,"Trying to flush DSK to disk but no changes made");
                return;
        }


        if (dskplusthree_persistent_writes.v==0) {
                debug_printf (VERBOSE_DEBUG,"Trying to flush DSK to disk but persistent writes disabled");
                return;
        }



        debug_printf (VERBOSE_INFO,"Flushing DSK to disk");


        FILE *ptr_dskplusthreefile;

        debug_printf (VERBOSE_INFO,"Opening DSK File %s",dskplusthree_file_name);
        ptr_dskplusthreefile=fopen(dskplusthree_file_name,"wb");



        int escritos=0;
        long long int size;
        size=p3dsk_buffer_disco_size;


        if (ptr_dskplusthreefile!=NULL) {
                z80_byte *puntero;
                puntero=p3dsk_buffer_disco; 

                //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
                //metera flush a 1
                dskplusthree_must_flush_to_disk=0;

                escritos=fwrite(puntero,1,size,ptr_dskplusthreefile);

                fclose(ptr_dskplusthreefile);


        }

        //debug_printf(VERBOSE_DEBUG,"ptr_dskplusthreefile: %d",ptr_dskplusthreefile);
        //debug_printf(VERBOSE_DEBUG,"escritos: %d",escritos);

        if (escritos!=size || ptr_dskplusthreefile==NULL) {
                debug_printf (VERBOSE_ERR,"Error writing to DSK file");
        }

}




void dskplusthree_disable(void)
{

	if (dskplusthree_emulation.v==0) return;

	debug_printf (VERBOSE_INFO,"Disabling DSK emulation");

	dskplusthree_emulation.v=0;

    dsk_memory_zone_dsk_sector_enabled.v=0;
}

                                
const char *dsk_signature_basic=   "MV - CPC";
//Usada en captain blood: "MV - CPC format Disk Image (DU54)\r\nDisk-Info"

const char *dsk_signature_extended="EXTENDED";

int dsk_file_type_extended=0;

void dsk_get_string_protected(int offset,char *buffer_signature,int total_bytes)
{
    int i;
    for (i=0;i<total_bytes;i++) {
        z80_byte c=p3dsk_buffer_disco[i+offset];
        if (c<32 || c>126) c='.';

        buffer_signature[i]=c;
    }

    buffer_signature[i]=0;    
}



void dsk_get_signature(char *buffer)
{
    //Mostrar firma ocultando caracteres no validos
    dsk_get_string_protected(0,buffer,DSK_SIGNATURE_LENGTH);
}

void dsk_get_creator(char *buffer)
{
    dsk_get_string_protected(0x22,buffer,DSK_CREATOR_LENGTH);
}


void dskplusthree_enable(void)
{

	if (dskplusthree_emulation.v) return;

	debug_printf (VERBOSE_INFO,"Enabling DSK emulation");
	debug_printf (VERBOSE_INFO,"Opening DSK File %s",dskplusthree_file_name);

	long long int tamanyo=get_file_size(dskplusthree_file_name);

	if (tamanyo>DSK_MAX_BUFFER_DISCO) {
		debug_printf(VERBOSE_ERR,"DSK size too big");
		return;
	}

        FILE *ptr_dskfile;
        ptr_dskfile=fopen(dskplusthree_file_name,"rb");

        if (!ptr_dskfile) {
                debug_printf(VERBOSE_ERR,"Unable to open disk %s",dskplusthree_file_name);
                return;
        }

        //int leidos=fread(p3dsk_buffer_disco,1,200000,ptr_configfile);
        fread(p3dsk_buffer_disco,1,DSK_MAX_BUFFER_DISCO,ptr_dskfile);


        fclose(ptr_dskfile);

    //Detect signature
    const int signature_check_length=8;

    if (!memcmp(dsk_signature_basic,p3dsk_buffer_disco,signature_check_length)) {
        printf("Detected Basic DSK\n");
        dsk_file_type_extended=0;
    }

    else if (!memcmp(dsk_signature_extended,p3dsk_buffer_disco,signature_check_length)) {
        printf("Detected Extended DSK\n");
        dsk_file_type_extended=1;
    }

    else {
        debug_printf(VERBOSE_ERR,"Unknown DSK file format");
        return;
    }

    //Mostrar firma ocultando caracteres no validos
    char buffer_signature[DSK_SIGNATURE_LENGTH+1];
    dsk_get_signature(buffer_signature);
    printf("DSK signature: %s\n",buffer_signature);

    char buffer_creator[DSK_CREATOR_LENGTH+1];
    dsk_get_creator(buffer_creator);
    printf("DSK creator: %s\n",buffer_creator);    

    printf("DSK total tracks: %d total sides: %d\n",dsk_get_total_tracks(),dsk_get_total_sides());

    char buffer_esquema_proteccion[DSK_MAX_PROTECTION_SCHEME+1];
    int protegido_no_soportado=dsk_get_protection_scheme(buffer_esquema_proteccion);
    if (protegido_no_soportado) {
        debug_printf(VERBOSE_ERR,"This disk is protected with an unsupported method: %s. It probably won't be readable",buffer_esquema_proteccion);
    }

    printf("Protection system: %s\n",buffer_esquema_proteccion);

	p3dsk_buffer_disco_size=tamanyo;

        dskplusthree_emulation.v=1;

}

int dsk_get_protection_scheme_aux(char *esquema)
{
    int longitud=strlen(esquema);
    int i;

    for (i=0;i<p3dsk_buffer_disco_size-longitud;i++) {
        if (!memcmp(&p3dsk_buffer_disco[i],esquema,longitud)) return 1;
    }

    return 0;
}

//Realmente es: SPEEDLOCK +3 DISC PROTECTION SYSTEM COPYRIGHT 1988 SPEEDLOCK ASSOCIATES FOR MORE DETAILS, PHONE (0734) 470303
//Ejemplo Batman The Caped Crusader.dsk
char *dsk_protection_scheme_speedlock_p3="SPEEDLOCK +3 DISC PROTECTION SYSTEM COPYRIGHT";

//Realmente es: SPEEDLOCK DISC PROTECTION SYSTEMS (C) 1989 SPEEDLOCK ASSOCIATES FOR MORE DETAILS, PHONE (0734) 470303
//Ejemplo Chuck Yeager's Advanced Flight Trainer.dsk
char *dsk_protection_scheme_speedlock_disc="SPEEDLOCK DISC PROTECTION SYSTEMS";

//Ejemplo: Cabal.dsk
char *dsk_protection_scheme_paul_owen="OCEAN SOFTWARE LIMITED\x80PAUL OWENS\x80PROTECTION SYSTEM";

//Alkatraz
//THE ALKATRAZ PROTECTION SYSTEM   (C) 1987  Appleby Associates                    Antony Dunmore          James Wood            John Bayliffe
//Ejemplo: Echelon.dsk
char *dsk_protection_scheme_alkatraz="THE ALKATRAZ PROTECTION SYSTEM   (C) 1987";


//Esto no se si realmente es un sistema de proteccion
//Pero parece que da problemas al usar esos discos
//***Loader Copyright Three Inch Software 1988, All Rights Reserved. Three Inch Software, 73 Surbiton Road, Kingston upon Thames, KT1 2HG***
char *dsk_protection_scheme_three_inch="Loader Copyright Three Inch Software";

//Este viene en Norte y Sur, al parecer ejecuta comandos invalidos pero funciona en ZEsarUX
//NEW DISK PROTECTION SYSTEM. (C) 1990 BY NEW FRONTIER SOFT.                              
//SI TIENES CONCIMIENTOS DE CODIGO MAQUINA PUEDES TENER TRABAJO LLAMANDO AL NUMERO: (93) 485-11-57 O ESCRIBENDO A: 
//C/FRANCISCO DE ARANDA, 45-47 ENTLO. 1 08005 BARCELONA.
/*
Sectores de 4kb
Parece que despues de un seek a pista 0, lanza otro a pista 1 y
si ese seek no finaliza en menos de 97 t-estados, empieza a enviar valores random (leidos de la rom) hasta que finaliza
el seek (durante el seek los valores random se ignoran)
*/
char *dsk_protection_scheme_new_frontier="NEW DISK PROTECTION SYSTEM. (C) 1990 BY NEW FRONTIER SOFT";

//Retorna diciendo si esta protegido y ademas sin soporte en emulacion, o no
//TODO: cuando soporte alguno de estos esquemas, retornar 0 en algunos casos diciendo que ya se soporta
int dsk_get_protection_scheme(char *buffer)
{
    if (dsk_get_protection_scheme_aux(dsk_protection_scheme_speedlock_p3)) {
        strcpy(buffer,"SPEEDLOCK +3 DISC 1988");
        return 1;
    }

    if (dsk_get_protection_scheme_aux(dsk_protection_scheme_speedlock_disc)) {
        strcpy(buffer,"SPEEDLOCK DISC 1989");
        return 1;
    }    

    if (dsk_get_protection_scheme_aux(dsk_protection_scheme_paul_owen)) {
        strcpy(buffer,"Ocean Paul Owens");
        return 1;
    }

    if (dsk_get_protection_scheme_aux(dsk_protection_scheme_alkatraz)) {
        strcpy(buffer,"ALKATRAZ 1987");
        return 1;
    }

    if (dsk_get_protection_scheme_aux(dsk_protection_scheme_three_inch)) {
        strcpy(buffer,"Three Inch");
        return 1;
    }    
    
    //Este si que esta soportado
    if (dsk_get_protection_scheme_aux(dsk_protection_scheme_new_frontier)) {
        strcpy(buffer,"New Frontier");
        return 0;
    }

    strcpy(buffer,"None");
    return 0;
}

z80_byte plus3dsk_get_byte_disk(int offset)
{

        if (dskplusthree_emulation.v==0) return 0;

        if (offset>=p3dsk_buffer_disco_size) {
                debug_printf (VERBOSE_ERR,"Error. Trying to read beyond dsk. Size: %d Asked: %d. Disabling DSK",p3dsk_buffer_disco_size,offset);
                //TODO de momento no desactivamos disco, para poder hacer debug
                //dskplusthree_disable();
                return 0;
        }


        return p3dsk_buffer_disco[offset];
}

void plus3dsk_put_byte_disk(int offset,z80_byte value)
{
        if (dskplusthree_emulation.v==0) return;

        if (offset>=p3dsk_buffer_disco_size) {
                debug_printf (VERBOSE_ERR,"Error. Trying to read beyond dsk. Size: %d Asked: %d. Disabling DSK",p3dsk_buffer_disco_size,offset);
                dskplusthree_disable();
                return;
        }


	if (dskplusthree_write_protection.v) return;

        p3dsk_buffer_disco[offset]=value;

	dskplusthree_must_flush_to_disk=1;
}

int dsk_get_total_tracks(void)
{
    return p3dsk_buffer_disco[0x30];
}

int dsk_get_total_sides(void)
{
    return p3dsk_buffer_disco[0x31];
}

void dsk_show_activity(void)
{
	generic_footertext_print_operating("DISK");

	//Y poner icono en inverso
	if (!zxdesktop_icon_plus3_inverse) {
			zxdesktop_icon_plus3_inverse=1;
			menu_draw_ext_desktop();
	}	
}

const int dsk_sector_sizes_numbers[]={
    0,    //0: TODO: no tengo claro que 0 sea tal cual sector size 0
    256,  //1
    512,  //2
    1024, //3
    2048, //4
    4096, //5
    8192, //6
};


//entrada: offset a track information block
int dsk_get_gap_length_track_from_offset(int offset)
{
    z80_byte gap=plus3dsk_get_byte_disk(offset+0x16);

    return gap;
}



int dsk_get_gap_length_track(int pista,int cara)
{
    int offset=dsk_get_start_track(pista,cara);
    return dsk_get_gap_length_track_from_offset(offset);

}


//entrada: offset a track information block
int dsk_get_datarate_track_from_offset(int offset)
{
    z80_byte datarate=plus3dsk_get_byte_disk(offset+0x12);

    return datarate;
}



int dsk_get_datarate_track(int pista,int cara)
{
    int offset=dsk_get_start_track(pista,cara);

    return dsk_get_datarate_track_from_offset(offset);

}


//entrada: offset a track information block
int dsk_get_recordingmode_track_from_offset(int offset)
{
    z80_byte recordingmode=plus3dsk_get_byte_disk(offset+0x16);

    return recordingmode;
}



int dsk_get_recordingmode_track(int pista,int cara)
{
    int offset=dsk_get_start_track(pista,cara);

    return dsk_get_recordingmode_track_from_offset(offset);

}

//entrada: offset a track information block
int dsk_get_filler_byte_track_from_offset(int offset)
{
    z80_byte filler=plus3dsk_get_byte_disk(offset+0x17);

    return filler;
}

int dsk_get_filler_byte_track(int pista,int cara)
{
    int offset=dsk_get_start_track(pista,cara);
    return dsk_get_filler_byte_track_from_offset(offset);

}


//entrada: offset a track information block
int dsk_get_total_sectors_track_from_offset(int offset)
{
    int total_sectors=plus3dsk_get_byte_disk(offset+0x15);

    return total_sectors;
}


int dsk_get_total_sectors_track(int pista,int cara)
{
    int offset=dsk_get_start_track(pista,cara);
    return dsk_get_total_sectors_track_from_offset(offset);

}


//entrada: offset a track information block
int dsk_get_sector_size_track_from_offset(int offset)
{
    int sector_size=plus3dsk_get_byte_disk(offset+0x14);
    if (sector_size>6) {
        printf("Sector size %d unsupported\n",sector_size);
        return -1;
    }

    sector_size=dsk_sector_sizes_numbers[sector_size];

    return sector_size;
}

int dsk_get_sector_size_track(int pista,int cara)
{
    int offset=dsk_get_start_track(pista,cara);
    return dsk_get_sector_size_track_from_offset(offset);
}



const char *dsk_track_info_signature="Track-Info\r\n";

int dsk_check_track_signature(int offset)
{
    int i;
    for (i=0;i<12;i++) {
        int leido_firma=dsk_track_info_signature[i];
        int leido_pista=plus3dsk_get_byte_disk(offset+i);
        if (leido_firma!=leido_pista) return -1;
    }
    return 0;    
}

int dsk_basic_get_start_track(int pista_encontrar,int cara_encontrar)
{
    int pista;
    int offset=0x100;

    for (pista=0;pista<dsk_get_total_tracks();pista++) {
        //Validar que estemos en informacion de pista realmente mirando la firma
        //TODO: quiza esta validacion se pueda quitar y/o hacerla al abrir el dsk 
        if (dsk_check_track_signature(offset)) {
            debug_printf(VERBOSE_ERR,"DSK: Track signature not found on track %XH offset %XH",pista,offset);
        }    

        z80_byte track_number=plus3dsk_get_byte_disk(offset+0x10);
        z80_byte side_number=plus3dsk_get_byte_disk(offset+0x11);

        if (track_number==pista_encontrar && side_number==cara_encontrar) {
            return offset;
        }

        int sector_size=dsk_get_sector_size_track_from_offset(offset);
        if (sector_size<0) {
            debug_printf(VERBOSE_ERR,"DSK Basic: Sector size not supported on track %d",pista);
            return -1;
        }

        int total_sectors=dsk_get_total_sectors_track_from_offset(offset);

        int saltar=total_sectors*sector_size+256; //256 ocupa el sector block

        offset +=saltar;
    }

    return -1;

}

//Retorna numero de pista. 
//Entrada: offset: offset a track-info
int dsk_get_track_number_from_offset(int offset)
{
    z80_byte track_number=plus3dsk_get_byte_disk(offset+0x10);
    return track_number;
}


int dsk_get_track_number(int pista,int cara)
{
    int offset=dsk_get_start_track(pista,cara);
    return dsk_get_track_number_from_offset(offset);
}


//Retorna numero de cata. 
//Entrada: offset: offset a track-info
int dsk_get_track_side_from_offset(int offset)
{
    z80_byte side_number=plus3dsk_get_byte_disk(offset+0x11);
    return side_number;
}

int dsk_get_track_side(int pista,int cara)
{
    int offset=dsk_get_start_track(pista,cara);
    return dsk_get_track_side_from_offset(offset);
}


int dsk_extended_get_start_track(int pista_encontrar,int cara_encontrar)
{
    int pista,cara;
    int offset=0x100;
    int offset_track_table=0x34;

    for (pista=0;pista<dsk_get_total_tracks();pista++) {
        for (cara=0;cara<dsk_get_total_sides();cara++) {
        //Validar que estemos en informacion de pista realmente mirando la firma
        //TODO: quiza esta validacion se pueda quitar y/o hacerla al abrir el dsk 
        if (dsk_check_track_signature(offset)) {
            debug_printf(VERBOSE_ERR,"DSK: Track signature not found on track %XH size %d offset %XH",pista,cara,offset);
        } 

        z80_byte track_number=dsk_get_track_number_from_offset(offset);
        z80_byte side_number=dsk_get_track_side_from_offset(offset);

        //printf("dsk_extended_get_start_track: pista: %d current_track: %d offset: %XH buscar pista: %d\n",
        //    pista,track_number,offset,pista_encontrar);        

        if (track_number==pista_encontrar && side_number==cara_encontrar) {
            //printf("dsk_extended_get_start_track: return %X\n",offset);
            return offset;
        }

        int sector_size=dsk_get_sector_size_track_from_offset(offset);
        if (sector_size<0) {
            debug_printf(VERBOSE_ERR,"DSK Extended: Sector size not supported on track %d side %d",pista,cara);
            return -1;
        }

        
        int saltar=plus3dsk_get_byte_disk(offset_track_table)*256;
        offset +=saltar;

        
        offset_track_table++;


    }
    }

    return -1;

}


//Retorna -1 si pista no encontrada
//Retorna offset al Track information block
int dsk_get_start_track(int pista,int cara)
{
    //Hacerlo diferente si dsk basico o extendido
    if (dsk_file_type_extended) return dsk_extended_get_start_track(pista,cara);
    else return dsk_basic_get_start_track(pista,cara);
}

int dsk_extended_get_track_size(int pista,int cara)
{
    int offset_track_table=0x34;
    
    //este incremento sobre la tabla es el doble cuando el disco tiene dos caras
    int incremento=pista*dsk_get_total_sides();
    
    offset_track_table +=incremento;

    if (cara==1) offset_track_table++;

    return plus3dsk_get_byte_disk(offset_track_table)*256;

}


//Retorna el offset al dsk segun la pista y sector id dados 
//Retorna tambien el sector fisico: 0,1,2,3....
//Parametro minimo_sector permite escoger un sector mayor que dicho parametro
int dsk_get_sector(int pista,int parametro_r,z80_byte *sector_fisico,int minimo_sector,int search_deleted,int skip_not_match)
{

    int iniciopista=dsk_get_start_track(pista,0); //TODO: de momento solo cara 0

    printf("Inicio pista %d: %XH\n",pista,iniciopista);

    int total_sectors=dsk_get_total_sectors_track_from_offset(iniciopista);   

    printf("Total sectors: %d\n",total_sectors); 


    int sector_information_list=iniciopista+0x18;

    int sector;

    for (sector=0;sector<total_sectors;sector++) {

        printf("Buscando sector ID %02XH on track %d estamos en pos sector %d\n",parametro_r,pista,sector);


        z80_byte sector_id=plus3dsk_get_byte_disk(sector_information_list+2); 


        
        //Para obtener el tipo de sector (borrado si/no)
        z80_byte leido_id_st1=plus3dsk_get_byte_disk(sector_information_list+4); 
        z80_byte leido_id_st2=plus3dsk_get_byte_disk(sector_information_list+5); 

        //sector borrado
        int deleted_sector=0;
        if (leido_id_st2 & PD765_STATUS_REGISTER_TWO_CM_MASK) deleted_sector=1;

        int match_condition=0;

        if (search_deleted==deleted_sector) {
            //Encontramos el tipo de sector que buscamos (borrado si/no)
            match_condition=1;
        }
        else {
            //No lo encontramos. Si skip=0, no saltarlo
            if (!skip_not_match) match_condition=1;
        }


        //Sector no tiene marca ni de borrado ni de no borrado, por tanto no hay match
        //Aun no me he encontrado ningun disco con esto, pero por si acaso
        if ((leido_id_st2 & PD765_STATUS_REGISTER_TWO_MD_MASK) || (leido_id_st1 & PD765_STATUS_REGISTER_ONE_MA_MASK)) {
            match_condition=0;
            printf("DSK: sector does not have deleted nor not deleted mask\n");
            //sleep(3);
        }

        if (sector_id==parametro_r && sector>minimo_sector && match_condition) {
            //debug_printf(VERBOSE_DEBUG,"Found sector  ID track %d/sector %d at  pos track %d/sector %d",pista_buscar,sector_buscar,pista,sector);
            printf("Found sector ID %02XH on track %d at pos sector %d\n",parametro_r,pista,sector);


            int offset=iniciopista+0x100;

            int sector_size;

            //Ejemplos en los que es necesario leer el tamanyo de esta manera: Riptoff Master Disk.dsk
            if (dsk_file_type_extended) {
                //TODO: cara 0 de momento solamente
                sector_size=dsk_get_real_sector_size_extended(pista,0,sector);
            }

            else sector_size=dsk_get_sector_size_track_from_offset(iniciopista);
            if (sector_size<0) {
                debug_printf(VERBOSE_ERR,"dsk_get_sector: Sector size not supported on track %d sector %d",pista,sector);
                return -1;
            }                        

            //int iniciopista=traps_plus3dos_getoff_start_track(pista);
            int offset_retorno=offset+sector_size*sector;
            //printf("Offset sector: %XH\n",offset_retorno);

            *sector_fisico=sector;
            printf("Found sector ID %02XH on track %d at offset in DSK: %XH\n",parametro_r,pista,offset_retorno);
            return offset_retorno;
        }

        sector_information_list +=8;

    }



    printf("NOT Found sector ID %02XH on track %d (max sectors: %d)\n",parametro_r,pista,total_sectors);
	return -1;

}


//Retorna el offset al dsk segun la pista y sector fisico
int dsk_get_sector_fisico(int pista,int cara,int sector_fisico)
{

    int iniciopista=dsk_get_start_track(pista,cara); 

    //printf("Inicio pista %d: %XH\n",pista,iniciopista);


    int offset=iniciopista+0x100;

    int sector_size;

    //Ejemplos en los que es necesario leer el tamanyo de esta manera: Riptoff Master Disk.dsk
    if (dsk_file_type_extended) {
        sector_size=dsk_get_real_sector_size_extended(pista,cara,sector_fisico);
    }

    else sector_size=dsk_get_sector_size_track_from_offset(iniciopista);


    if (sector_size<0) {
        debug_printf(VERBOSE_ERR,"dsk_get_sector: Sector size not supported on track %d sector %d",pista,sector_fisico);
        return -1;
    }                        

    //int iniciopista=traps_plus3dos_getoff_start_track(pista);
    int offset_retorno=offset+sector_size*sector_fisico;
    //printf("Offset sector: %XH\n",offset_retorno);

    
    //printf("Found sector ID %02XH on track %d at offset in DSK: %XH\n",parametro_r,pista,offset_retorno);
    return offset_retorno;





}


//Devolver CHRN de una pista y sector concreto
void dsk_get_chrn(int pista,int cara,int sector_fisico,z80_byte *parametro_c,z80_byte *parametro_h,z80_byte *parametro_r,z80_byte *parametro_n)
{


    int iniciopista=dsk_get_start_track(pista,cara); 

    //printf("En dsk_get_chrn Inicio pista %d: %XH\n",pista,iniciopista);

    //saltar 0x18
    iniciopista +=0x18;


    int offset_tabla_sector=sector_fisico*8; 
    //z80_byte pista_id=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector); //Leemos pista id
    //z80_byte sector_id=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+2); //Leemos c1, c2, etc

    //debug_printf(VERBOSE_DEBUG,"%02X ",sector_id);

    *parametro_c=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector); 
    *parametro_h=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+1); 
    *parametro_r=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+2); 
    *parametro_n=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+3);             
            



}




//Devolver st1,2 de una pista y sector concreto

void dsk_get_st12(int pista,int cara,int sector_fisico,z80_byte *parametro_st1,z80_byte *parametro_st2)
{

    int iniciopista=dsk_get_start_track(pista,cara); 

    //printf("En dsk_get_st12 Inicio pista %d: %XH\n",pista,iniciopista);

    //saltar 0x18
    iniciopista +=0x18;


    int offset_tabla_sector=sector_fisico*8; 
    //z80_byte pista_id=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector); //Leemos pista id
    //z80_byte sector_id=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+2); //Leemos c1, c2, etc

    //debug_printf(VERBOSE_DEBUG,"%02X ",sector_id);


    *parametro_st1=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+4); 
    *parametro_st2=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+5);             
            


}

//Devolver tamaño real de una pista y sector concreto, para tipo extendido

int dsk_get_real_sector_size_extended(int pista,int cara,int sector_fisico)
{

    int iniciopista=dsk_get_start_track(pista,cara); 

    //printf("En dsk_get_st12 Inicio pista %d: %XH\n",pista,iniciopista);

    //saltar 0x18
    iniciopista +=0x18;


    int offset_tabla_sector=sector_fisico*8; 
    //z80_byte pista_id=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector); //Leemos pista id
    //z80_byte sector_id=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+2); //Leemos c1, c2, etc

    //debug_printf(VERBOSE_DEBUG,"%02X ",sector_id);


    int tamanyo=plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+6)+256*plus3dsk_get_byte_disk(iniciopista+offset_tabla_sector+7);  

    return tamanyo;            


}


int dsk_is_track_formatted(int pista,int cara)
{
    int sinformatear=0;

    if (dsk_file_type_extended) {
        int track_size=dsk_extended_get_track_size(pista,cara);
        if (!track_size) sinformatear=1;
    }

    if (sinformatear) return 0;

    return 1;
}
