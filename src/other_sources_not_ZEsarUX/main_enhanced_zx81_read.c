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


typedef long long int z80_64bit;
typedef unsigned char z80_byte;

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

char *rwafile="prueba.rwa";

int main(void)
{


    z80_64bit tamanyo_archivo=enh_get_file_size(rwafile);

    FILE *ptr_archivo;
    ptr_archivo=fopen(rwafile,"rb");


    if (!ptr_archivo) {
        printf("Unable to open rwa file: %s\n",rwafile);
        return 1;
    }


    //Cargarlo todo en memoria
    z80_byte *enhanced_memoria=malloc(tamanyo_archivo);
    if (enhanced_memoria==NULL) {
        printf("Can not allocate memory for load rwa file");
        return 0;
    }
    fread(enhanced_memoria,1,tamanyo_archivo,ptr_archivo);
    fclose(ptr_archivo);

    main_enhanced_zx81_read(enhanced_memoria,tamanyo_archivo);

    return 0;


}