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
#include <string.h>
#include <sys/stat.h>

#include "enhanced_zx81_read.h"




//Retorna tamanyo archivo
long long int enh_get_file_size(char *nombre)
{


#if __MINGW32__
        //Para que funcione la llamada en windows 32 bit. Si no, trata tamanyos de archivos con enteros de 32 bits
        //y mostrara mal cualquier tamanyo mayor de 2 GB
        struct __stat64 buf_stat;
        if (_stat64(nombre, &buf_stat) != 0) {
#else

        struct stat buf_stat;
        if (stat(nombre, &buf_stat)!=0) {
#endif
            printf("Unable to get status of file %s\n",nombre);
            return 0;
        }

        else {
			//printf ("file size: %ld\n",buf_stat.st_size);
			return buf_stat.st_size;
        }

}


//Por cada amplitud probada, que longitud se genera
int longitudes_autodetectar[256];
//Por cada amplitud probada, cuantos errores de pulsos se genera
int errores_pulsos_autodetectar[256];

void print_mensajes(char *texto)
{
    printf("%s\n",texto);
}

void print_nombre(int longitud_nombre,z80_byte *memoria_p81)
{
    int i;

    for (i=0;i<longitud_nombre;i++) {
        printf("%c",return_zx81_char(memoria_p81[i]));
    }
    printf("\n");
}

int main(int argc,char *argv[])
{
    printf(
        "ENHANCED ZX81 READ V1.0\n"
        "(c) Cesar Hernandez Bano (19/08/2025)\n"
        "\n"
    );

    if (argc<5) {
        printf("Syntax: %s input_file amplitude invert debug \n",argv[0]);

        printf(
            "amplitude should be a value between 1 and 255 or word autodetect\n"
            "invert must be 0 or 1 - to invert signal or not\n"
            "debug must be 0 or 1\n"
            "input_file must be raw, 8 bit, unsigned, 1 channel. No matter recorded frequency\n"
            "Output files will be output.p81 and output.p; I recommend you to use output.p81 becase it's a full dump with the name\n"
        );
        exit(1);
    }

    char *rwafile=argv[1];

    //Valor para control de stocks
    //Nota: es un int porque en autodeteccion hacemos un bucle hasta 255 y no quiero que de la vuelta
    int amplitud_media=18;


    int autodetectar_amplitud=0;

    if (!strcasecmp(argv[2],"autodetect")) {
        autodetectar_amplitud=1;
    }
    else amplitud_media=atoi(argv[2]);

    int invert_signal=atoi(argv[3]);

    int debug_print=atoi(argv[4]);

    int tamanyo_archivo=enh_get_file_size(rwafile);

    FILE *ptr_archivo;
    ptr_archivo=fopen(rwafile,"rb");


    if (!ptr_archivo) {
        printf("Unable to open rwa file: %s\n",rwafile);
        return 1;
    }


    //Cargarlo todo en memoria
    z80_byte *enhanced_memoria=malloc(tamanyo_archivo);
    if (enhanced_memoria==NULL) {
        printf("Can not allocate memory for load rwa file\n");
        return 1;
    }

    fread(enhanced_memoria,1,tamanyo_archivo,ptr_archivo);
    fclose(ptr_archivo);

    if (invert_signal) {
        int i;
        for (i=0;i<tamanyo_archivo;i++) {
            int valor=enhanced_memoria[i];
            valor=255-valor;
            enhanced_memoria[i]=valor;
        }
    }



    z80_byte *memoria_p81=malloc(65536); //mas de 64 kb para un .P81 seria absurdo

    if (memoria_p81==NULL) {
        printf("Can not allocate memory for load rwa file\n");
        return 1;
    }

    int longitud_nombre;

    int longitud_p81;


    if (autodetectar_amplitud) {

        int inicio_autodetectar=5;
        int final_autodetectar=255;

        for (amplitud_media=inicio_autodetectar;amplitud_media<=final_autodetectar;amplitud_media++) {

            int pulsos_sospechosos_para_esta_amplitud;

            //no queremos hacer print de mensajes de deteccion, a no ser que el usuario active el debug
            if (debug_print) longitud_p81=enh_zx81_lee_datos(enhanced_memoria,tamanyo_archivo,memoria_p81,amplitud_media,
                                debug_print,&longitud_nombre,print_mensajes,NULL,NULL,&pulsos_sospechosos_para_esta_amplitud,NULL);

            else longitud_p81=enh_zx81_lee_datos(enhanced_memoria,tamanyo_archivo,memoria_p81,amplitud_media,
                    debug_print,&longitud_nombre,NULL,NULL,NULL,&pulsos_sospechosos_para_esta_amplitud,NULL);

            longitudes_autodetectar[amplitud_media]=longitud_p81;
            errores_pulsos_autodetectar[amplitud_media]=pulsos_sospechosos_para_esta_amplitud;


            printf("Amplitude=%d Pulse errors=%d Name length: %d Length p81: %d Name: ",
                amplitud_media,pulsos_sospechosos_para_esta_amplitud,longitud_nombre,longitud_p81);

            print_nombre(longitud_nombre,memoria_p81);
        }

        //buscar longitud maxima
        int longitud_maxima=0;
        int i;
        for (i=inicio_autodetectar;i<=final_autodetectar;i++) {
            if (longitudes_autodetectar[i]>longitud_maxima) {
                longitud_maxima=longitudes_autodetectar[i];
                //De momento amplitud usamos esta inicial
                amplitud_media=i;
            }
        }

        printf("Autodetected Maximum length %d. Let's see which one has less errors\n",longitud_maxima);

        //buscar cual de las amplitudes que genera esa longitud maxima es la que tiene menos errores
        int errores_detectados=999999;
        for (i=inicio_autodetectar;i<=final_autodetectar;i++) {
            if (longitudes_autodetectar[i]==longitud_maxima) {
                if (errores_pulsos_autodetectar[i]<errores_detectados) {
                    errores_detectados=errores_pulsos_autodetectar[i];
                    amplitud_media=i;
                    printf("Less errors using amplitude %d (%d errors)\n",i,errores_detectados);
                }
            }
        }

        //printf("Final obtencion menos errores con amplitud=%d, errores=%d\n",amplitud_media,errores_detectados);

        //"Nombre" para poder hacer grep por consola y que me salgan los nombres anteriores de cada prueba y esta linea tambien
        printf("Autodetected best amplitude %d errors=%d\n",amplitud_media,errores_detectados);


    }



    longitud_p81=enh_zx81_lee_datos(enhanced_memoria,tamanyo_archivo,memoria_p81,amplitud_media,
        debug_print,&longitud_nombre,print_mensajes,NULL,NULL,NULL,NULL);


    printf("Amplitude=%d Name length: %d Length p81: %d Name: ",amplitud_media,longitud_nombre,longitud_p81);

    print_nombre(longitud_nombre,memoria_p81);

    FILE *ptr_dskplusthreefile;
    ptr_dskplusthreefile=fopen("output.p81","wb");


    if (ptr_dskplusthreefile!=NULL) {

        fwrite(memoria_p81,1,longitud_p81,ptr_dskplusthreefile);


        fclose(ptr_dskplusthreefile);
    }

    //Generamos tambien el .p

    ptr_dskplusthreefile=fopen("output.p","wb");


    if (ptr_dskplusthreefile!=NULL) {

        fwrite(&memoria_p81[longitud_nombre],1,longitud_p81-longitud_nombre,ptr_dskplusthreefile);


        fclose(ptr_dskplusthreefile);
    }


    free(enhanced_memoria);
    free(memoria_p81);



    return 0;


}