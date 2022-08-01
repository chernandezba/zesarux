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
#include <string.h>
#include <stdlib.h>

#include "cpu.h"
#include "hilow_datadrive_audio.h"
#include "hilow_datadrive.h"

int hilow_read_audio_completamente_automatico=0;


z80_byte *read_hilow_audio_file(char *archivo)
{
    z80_byte *puntero;


   

    //Asignar memoria
    int tamanyo=hilow_read_audio_get_file_size(archivo);
    puntero=malloc(tamanyo);

    if (puntero==NULL) {
        printf("Can not allocate memory for hilow audio file");
        return NULL;
    }


    //cargarlo en memoria
    FILE *ptr_bmpfile;
    ptr_bmpfile=fopen(archivo,"rb");

    if (!ptr_bmpfile) {
            printf("Unable to open audio file %s\n",archivo);
            return NULL;
    }

    fread(puntero,1,tamanyo,ptr_bmpfile);
    fclose(ptr_bmpfile);

    //Si leemos cara 2, invertir todo el sonido (el principio al final)
    if (hilow_read_audio_leer_cara_dos) {
        hilow_read_audio_espejar_sonido(puntero,tamanyo);
    }

    return puntero;
}

void hilow_read_audio_read_hilow_ddh_file(char *archivo)
{
    //z80_byte *puntero;


    //Leer archivo ddh
    //Asignar memoria
    int tamanyo=HILOW_DEVICE_SIZE;
    hilow_read_audio_hilow_ddh=malloc(tamanyo);

    if (hilow_read_audio_hilow_ddh==NULL) {
        //TODO: que hacer si no se puede asignar memoria
        printf("Can not allocate memory for hilow ddh file");
        exit(1);
    }


    //cargarlo en memoria, si es que existe
    FILE *ptr_ddhfile;
    ptr_ddhfile=fopen(archivo,"rb");

    if (!ptr_ddhfile) {
        //Esto es normal, si archivo de output no existe
        printf("Unable to open ddh file %s\n",archivo);
        return;
    }

    fread(hilow_read_audio_hilow_ddh,1,tamanyo,ptr_ddhfile);
    fclose(ptr_ddhfile);    



}

void hilow_read_audio_write_hilow_ddh_file(char *archivo)
{
    z80_byte *puntero;

    int tamanyo=HILOW_DEVICE_SIZE;


    FILE *ptr_ddhfile;
    ptr_ddhfile=fopen(archivo,"wb");

    if (!ptr_ddhfile) {
            printf("Unable to open ddh file %s\n",archivo);
            return;
    }

    fwrite(hilow_read_audio_hilow_ddh,1,tamanyo,ptr_ddhfile);
    fclose(ptr_ddhfile);    

}

int hilow_read_audio_ask_save_sector(void)
{
    /*
    Posibles salidas: 
    -Fin. Finalizar proceso: Salida -1
    -No Grabar sector: Salida 0
    -Grabar sector: Salida 1
    -Repetir lectura sector: Salida 2
    */

    char buffer_pregunta[100];

    if (!hilow_read_audio_completamente_automatico) {
        buffer_pregunta[0]=0;

        do {

            printf("Grabar sector? (s/n) e: editar numero sector p: cambio parametros r: repetir f: fin  ");

            scanf("%s",buffer_pregunta);

            if (buffer_pregunta[0]=='r') {
                printf("Repeat\n");
                return 2;
            }

            if (buffer_pregunta[0]=='f') {
                printf("Ending\n");
                return -1;
            }

            if (buffer_pregunta[0]=='p') {

                int parm;

                do {
                    printf("Parametros: 1) autoadjust_bit_width %d  2) verbose %d  3) verbose_extra %d  4) invert %d  5) autocorrect %d  0) end \n",
                        hilow_read_audio_autoajustar_duracion_bits,hilow_read_audio_modo_verbose,hilow_read_audio_modo_verbose_extra,
                        hilow_read_audio_invertir_senyal, hilow_read_audio_autocorrect);

                    
                    char buffer_parm[100];
                    scanf("%s",buffer_parm);
                    parm=atoi(buffer_parm);
                    
                    if (parm==1) hilow_read_audio_autoajustar_duracion_bits ^=1;
                    if (parm==2) hilow_read_audio_modo_verbose ^=1;
                    if (parm==3) hilow_read_audio_modo_verbose_extra ^=1;
                    if (parm==4) hilow_read_audio_invertir_senyal ^=1;
                    if (parm==5) hilow_read_audio_autocorrect ^=1;


                } while(parm!=0);
            }              

            if (buffer_pregunta[0]=='e') {
                printf("Nuevo sector? : ");
                int sector;
                char buffer_sector[100];
                scanf("%s",buffer_sector);
                sector=atoi(buffer_sector);
                printf("Nuevo sector: %d\n",sector);
            }    

            if (buffer_pregunta[0]=='n') {
                printf("Not saving this sector\n");
                return 0;
            }

            if (buffer_pregunta[0]=='s') {
                printf("Saving this sector\n");
                return 1;
            }            

        } while (1);
    } 

    //Esta en modo automatico. Asumimos grabar sector
    return 1;  
}

void hilow_read_audio_dump_sector_contents(void)
{
     int total=HILOW_SECTOR_SIZE+1;

    //dump total ascii+hexa
    int colwidth=50;     

    int i;

    for (i=1;i<total /*&& posicion!=-1*/;i+=colwidth) {
        int col;

        printf("%08X ",i-1);

        for (col=0;col<colwidth && i+col<HILOW_SECTOR_SIZE+1;col++) {
            z80_byte byte_leido=hilow_read_audio_buffer_result[i+col];

            printf("%02X",byte_leido);
        }        

        printf(" ");

        for (col=0;col<colwidth && i+col<HILOW_SECTOR_SIZE+1;col++) {

            z80_byte byte_leido=hilow_read_audio_buffer_result[i+col];

            printf("%c",(byte_leido>=32 && byte_leido<=126 ? byte_leido : '.'));
        }

        printf("\n");

    } 

    printf("\n");      
}

int hilow_read_audio_lee_sector_preguntando(int posicion,int *total_bytes_leidos)
{
    //int repetir;

    int posicion_inicial=posicion;

    int sector;

    //repetir=0;

    int respuesta;

    do {

        posicion=posicion_inicial;
        posicion=hilow_read_audio_lee_sector(posicion,total_bytes_leidos,&sector);


        hilow_read_audio_pausa(1);
        hilow_read_audio_dump_sector_contents();   
        
        if (!hilow_read_audio_directo_a_pista) {
            if (hilow_read_audio_warn_if_sector_mismatch(sector)) {
                printf("Probably sector mismatch!\n");
                hilow_read_audio_print_mostrar_ids_sector();
                hilow_read_audio_pausa(2);            
            }
        }

        printf("Total bytes leidos: %d\n",*total_bytes_leidos);
        printf("Sector %d\n",sector);        

        respuesta=hilow_read_audio_ask_save_sector();

        /*
        Posibles salidas: 
        -Fin. Finalizar proceso: Salida -1
        -No Grabar sector: Salida 0
        -Grabar sector: Salida 1
        -Repetir lectura sector: Salida 2
        */


    } while(respuesta==2);

    //respuesta puede valer -1, 0 o 1. el caso 2 no puede entrar pues es repeticion y se queda en el bucle while
    if (respuesta==-1) return -1;
    else {
        if (respuesta) hilow_read_audio_write_sector_to_memory(sector);
    }

    return posicion;
}
int main(int argc,char *argv[])
{

    printf( "lee_hilow_audio v.1.0\n"
            "Copyright (C) 2022 Cesar Hernandez Bano\n"
            "Utility to convert audio file from a HiLow Data Drive to a binary dump (.ddh file) useful for ZEsarUX emulator\n"
            "Import audio must be: raw format (no headers), mono, 8 bits, unsigned, 44100 Hz\n\n");

    int mostrar_ayuda=0;

    if (argc>1 && !strcasecmp(argv[1],"--help")) mostrar_ayuda=1;

    if(argc<3 || mostrar_ayuda) {
        printf( "Usage: %s source.raw destination.ddh\n",argv[0]);
        printf("Available switches:\n"
                "\n"   
                "--autocorrect: Try to fix read errors depending on S_START_BYTE signal\n"   
                "--autoadjust_bit_width: Try to adjust bit length width depending on the S_START_BYTE signal\n"
                "--automatic: Do not ask anything\n"
                "--bside: Needed to convert audio from B side\n"
                "--invert: Invert audio input signal ('mirror' vertically), needed for some tapes\n"
                "--onlysector: Convert dump audio which starts just at the sector data, and convert only one sector\n"
                "--pause: Add delays after some actions\n"
                "--verbose: Print debugging info\n"
                "--verboseextra: Print even more debugging info\n"
        );
        exit(1);
    }

    

    char *archivo;

    //44100hz, unsigned 8 bits

    archivo=argv[1];


    char *archivo_ddh;
    archivo_ddh=argv[2];

    

    int indice_argumento=3;

    //Leidos ya el programa y source y destino
    int argumentos_leer=argc-3;

    while (argumentos_leer>0) {

        if (!strcasecmp(argv[indice_argumento],"--autoadjust_bit_width")) hilow_read_audio_autoajustar_duracion_bits=1;
        else if (!strcasecmp(argv[indice_argumento],"--autocorrect")) hilow_read_audio_autocorrect=1;
        else if (!strcasecmp(argv[indice_argumento],"--invert")) hilow_read_audio_invertir_senyal=1;
        else if (!strcasecmp(argv[indice_argumento],"--onlysector")) hilow_read_audio_directo_a_pista=1;
        else if (!strcasecmp(argv[indice_argumento],"--verbose")) hilow_read_audio_modo_verbose=1;
        else if (!strcasecmp(argv[indice_argumento],"--verboseextra")) hilow_read_audio_modo_verbose_extra=1;
        else if (!strcasecmp(argv[indice_argumento],"--pause")) hilow_read_audio_ejecutar_sleep=1;
        else if (!strcasecmp(argv[indice_argumento],"--automatic")) hilow_read_audio_completamente_automatico=1;
        else if (!strcasecmp(argv[indice_argumento],"--bside")) hilow_read_audio_leer_cara_dos=1;

        else {
            printf("Invalid parameter %s\n",argv[indice_argumento]);
            exit(1);
        }

        indice_argumento++;
        argumentos_leer--;
    }

    printf("Parametros: origen %s destino %s autoadjust_bit_width %d solopista %d verbose %d\n",
        archivo,archivo_ddh,hilow_read_audio_autoajustar_duracion_bits,hilow_read_audio_directo_a_pista,hilow_read_audio_modo_verbose);
    hilow_read_audio_pausa(2);


    hilow_read_audio_tamanyo_archivo_audio=hilow_read_audio_get_file_size(archivo);


    hilow_read_audio_read_hilow_memoria_audio=read_hilow_audio_file(archivo);

    hilow_read_audio_read_hilow_ddh_file(archivo_ddh);
    //printf("puntero: %p\n",hilow_read_audio_hilow_ddh);
    //hilow_read_audio_pausa(2);

    int posicion=0;
    int total_bytes_leidos;

    if (hilow_read_audio_directo_a_pista) {  
        hilow_read_audio_lee_sector_preguntando(posicion,&total_bytes_leidos);
    }

    else {

        while (posicion!=-1) {

            printf("\n");
            posicion=hilow_read_audio_buscar_inicio_sector(posicion);
            if (hilow_read_audio_modo_verbose) printf("Posicion inicio bits de datos de sector: %d\n",posicion);
            
            posicion=hilow_read_audio_lee_sector_preguntando(posicion,&total_bytes_leidos);

        }

    }

    hilow_read_audio_write_hilow_ddh_file(archivo_ddh);

    free(hilow_read_audio_read_hilow_memoria_audio);
    free(hilow_read_audio_hilow_ddh);

    printf("Finalizado proceso\n");


    return 0;
}