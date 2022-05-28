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

#include "hilow_audio.h"


int main(int argc,char *argv[])
{

    int mostrar_ayuda=0;

    if (argc>1 && !strcasecmp(argv[1],"--help")) mostrar_ayuda=1;

    if(argc<3 || mostrar_ayuda) {
        printf("%s source_wav destination.ddh [--autoadjust_bit_width] [--onlysector] "
                "[--verbose] [--verboseextra] [--pause] [--automatic] [--bside]\n",argv[0]);
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

        //con opcion autoadjust_bit_width suele cargar peor

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


    hilow_read_audio_tamanyo_archivo=hilow_read_audio_get_file_size(archivo);


    hilow_read_audio_read_hilow_memoria=read_hilow_audio_file(archivo);

    hilow_read_audio_read_hilow_ddh_file(archivo_ddh);
    //printf("puntero: %p\n",hilow_read_audio_hilow_ddh);
    //hilow_read_audio_pausa(2);

    int posicion=0;
    int total_bytes_leidos;

    if (hilow_read_audio_directo_a_pista) {

        //temp. En hilow_read_audio_directo_a_pista esto no se debe hacer
        //posicion=hilow_read_audio_buscar_dos_sync_bits(posicion);
        //posicion=hilow_read_audio_buscar_inicio_sector(posicion);
        
        hilow_read_audio_lee_sector(posicion,&total_bytes_leidos);
        hilow_read_audio_write_hilow_ddh_file(archivo_ddh);
    }

    else {

        while (posicion!=-1) {

            printf("\n");
        
            posicion=hilow_read_audio_buscar_inicio_sector(posicion);

            if (hilow_read_audio_modo_verbose) printf("Posicion inicio bits de datos de sector: %d\n",posicion);

            //hilow_read_audio_pausa(5);
            

            posicion=hilow_read_audio_lee_sector(posicion,&total_bytes_leidos);

            hilow_read_audio_write_hilow_ddh_file(archivo_ddh);

        }

    }


    free(hilow_read_audio_read_hilow_memoria);

    free(hilow_read_audio_hilow_ddh);

    printf("Finalizado proceso\n");


    return 0;
}