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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>



#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "zvfs.h"
#include "ff.h"
#include "compileoptions.h"


/*
ZVFS = ZEsarUX Virtual File System

O sea, rutinas de acceso a file system, ya sea disco local u otros medios (por ejemplo imágenes FatFs montables)

En principio, estas rutinas solo se usan desde los menús

*/



//funcion fseek que soporta nativo del sistema o fatfs.
//Nota: para zvfs solo funciona con tipo SEEK_SET

void zvfs_fseek(int in_fatfs,FILE *ptr_file, long offset, int whence,FIL *fil)
{
    if (in_fatfs) {
        if (whence==SEEK_SET) {
            f_lseek(fil, offset);
        }
        else {
            debug_printf(VERBOSE_ERR,"zvfs_fseek only allows type SEEK_SET on FatFS volumes");
            return;
        }

    }
    else {
	    fseek(ptr_file, offset, whence);
    }
}

//funcion ftell que soporta nativo del sistema o fatfs.
long zvfs_ftell(int in_fatfs,FILE *ptr_file, FIL *fil)
{
    if (in_fatfs) {
        return f_tell(fil);

    }
    else {
	    return ftell(ptr_file);
    }
}

//funcion fclose que soporta nativo del sistema o fatfs
void zvfs_fclose(int in_fatfs,FILE *ptr_file_name,FIL *fil)
{
    if (in_fatfs) {
        f_close(fil);
    }
    else {
	    fclose(ptr_file_name);
    }
}

//funcion fclose que soporta nativo del sistema o fatfs
int zvfs_feof(int in_fatfs,FILE *ptr_file_name,FIL *fil)
{
    if (in_fatfs) {
        return f_eof(fil);
    }
    else {
	    return feof(ptr_file_name);
    }
}

//funcion fopen que soporta nativo del sistema o fatfs
//retorna <0 si error
int zvfs_fopen_read(char *file_name,int *in_fatfs,FILE **ptr_file_name,FIL *fil)
{
	//FILE *ptr_file_name;

    //Soporte para FatFS
    //FIL fil;        /* File object */
    FRESULT fr;     /* FatFs return code */

    *in_fatfs=util_path_is_mmc_fatfs(file_name);
    //printf("txt esta en fatfs: %d\n",*in_fatfs);

    if (*in_fatfs) {
        fr = f_open(fil, file_name, FA_READ);
        if (fr!=FR_OK)
        {
            //debug_printf (VERBOSE_ERR,"Unable to open %s file",file_name);
            return -1;
        }

        //Esto solo para que no se queje el compilador al llamar a zvfs_fread
        *ptr_file_name=NULL;
    }

    else {
	    *ptr_file_name=fopen(file_name,"rb");



        if (!(*ptr_file_name))
        {
            //debug_printf (VERBOSE_ERR,"Unable to open file for reading",file_name);
            return -1;
        }
    }

    return 0;


}

//funcion fopen que soporta nativo del sistema o fatfs
//retorna <0 si error
int zvfs_fopen_write(char *file_name,int *in_fatfs,FILE **ptr_file_name,FIL *fil)
{
	//FILE *ptr_file_name;

    //Soporte para FatFS
    //FIL fil;        /* File object */
    FRESULT fr;     /* FatFs return code */

    *in_fatfs=util_path_is_mmc_fatfs(file_name);
    //printf("txt esta en fatfs: %d\n",*in_fatfs);

    if (*in_fatfs) {
        fr = f_open(fil, file_name, FA_CREATE_ALWAYS | FA_WRITE );
        if (fr!=FR_OK)
        {
            //debug_printf (VERBOSE_ERR,"Unable to open file for writing",file_name);
            return -1;
        }

        //Esto solo para que no se queje el compilador al llamar a zvfs_fread
        *ptr_file_name=NULL;
    }

    else {
	    *ptr_file_name=fopen(file_name,"wb");



        if (!(*ptr_file_name))
        {
            //debug_printf (VERBOSE_ERR,"Unable to open %s file",file_name);
            return -1;
        }
    }

    return 0;


}


//funcion fread que soporta nativo del sistema o fatfs
int zvfs_fread(int in_fatfs,z80_byte *puntero_memoria,int bytes_to_load,FILE *ptr_file_hexdump_browser,FIL *fil)
{
    int leidos;

    if (in_fatfs) {
        UINT leidos_fatfs;
        f_read(fil,puntero_memoria,bytes_to_load,&leidos_fatfs);
        leidos=leidos_fatfs;
    }

    else {

        leidos=fread(puntero_memoria,1,bytes_to_load,ptr_file_hexdump_browser);
    }

    return leidos;
}


//funcion getc que soporta nativo del sistema o fatfs
z80_byte zvfs_fgetc(int in_fatfs,FILE *ptr_file_hexdump_browser,FIL *fil)
{


    if (in_fatfs) {
        UINT leidos_fatfs;
        z80_byte byte_leido;
        f_read(fil,&byte_leido,1,&leidos_fatfs);
        return byte_leido;
    }

    else {
        return fgetc(ptr_file_hexdump_browser);
    }
}

//funcion fwrite que soporta nativo del sistema o fatfs
int zvfs_fwrite(int in_fatfs,z80_byte *puntero_memoria,int bytes_to_save,FILE *ptr_file_hexdump_browser,FIL *fil)
{
    int escritos;

    if (in_fatfs) {
        UINT escritos_fatfs;
        f_write(fil,puntero_memoria,bytes_to_save,&escritos_fatfs);
        escritos=escritos_fatfs;
    }

    else {
        escritos=fwrite(puntero_memoria,1,bytes_to_save,ptr_file_hexdump_browser);
    }

    return escritos;
}

void zvfs_chdir(char *dir)
{

/*
Cambios de ruta al estilo unix:

Si 0:/xxxx, cambiamos a unidad mmc
Si /xxxx o \xxxxx o X:\XXXXX o X:/XXXXX, no es unidad mmc

Cualquier otra cosa, chdir sin alterar unidad mmc activa o no
*/
    int usar_chdir_mmc=util_path_is_mmc_fatfs(dir);



    if (usar_chdir_mmc) {
        menu_current_drive_mmc_image.v=1;
        //printf("ruta de zvfs_chdir es de mmc\n");
        //printf("llamando f_chdir desde zvfs_chdir a %s\n",dir);
        //menu_mmc_chdir(dir);
        f_chdir(dir);

        char buffer[1024];
        f_getcwd(buffer,1023);

        //printf("ruta despues de f_chdir: %s\n",buffer);

        zvfs_getcwd(buffer,1023);
        //printf("ruta despues de zvfs_getcwd: %s\n",buffer);

    }

    else {
        menu_current_drive_mmc_image.v=0;
        chdir(dir);
    }
}

void zvfs_getcwd(char *dir,int len)
{
    //printf("path_max: %d\n",PATH_MAX);
    //Si unidad activa es la de mmc
    if (menu_current_drive_mmc_image.v) {
        //printf("unidad actual de zvfs_getcwd es de mmc\n");

        //Miramos si nos retorna un 0:/ y si no, lo agregamos
        //TODO: esperemos que nadie use rutas mas grandes que esto
        char buffer_cwd[2048];
        f_getcwd(buffer_cwd,len);

        if (util_path_is_prefix_mmc_fatfs(buffer_cwd)) {
            //La devolvemos tal cual
            strcpy(dir,buffer_cwd);
        }
        else {
            //Agregamos 0:/
            //TODO: aqui no hacemos caso del parametro len
            sprintf(dir,"0:/%s",buffer_cwd);
        }

    }

    else {
        getcwd(dir,len);
    }
}

void zvfs_rename(char *old,char *new)
{
    if (util_path_is_mmc_fatfs(old)) {
        f_rename(old,new);
    }
    else {
        rename(old,new);
    }
}

long long int zvfs_get_file_size(char *name)
{
    if (util_path_is_mmc_fatfs(name)) {
        FILINFO fno;
        f_stat(name,&fno);
        return fno.fsize;
    }
    else {
        return get_file_size(name);
    }
}

int zvfs_delete(char *filename)
{
    if (util_path_is_mmc_fatfs(filename)) {
        FRESULT resultado=f_unlink(filename);
        if (resultado!=FR_OK) return 1;
        else return 0;
    }
    else {
	    int resultado=unlink(filename);
        if (resultado!=0) return 1;
        else return 0;
    }
}

void zvfs_mkdir(char *directory)
{
    if (util_path_is_mmc_fatfs(directory)) {
        f_mkdir(directory);
    }

    else {

    #ifndef MINGW
        int tmpdirret=mkdir(directory,S_IRWXU);
    #else
        int tmpdirret=mkdir(directory);
    #endif

        if (tmpdirret!=0 && errno!=EEXIST) {
                    debug_printf (VERBOSE_ERR,"Error creating %s directory : %s",directory,strerror(errno) );
        }


    }

}

const char *zvfs_error_no_filesystem="No filesystem";
const char *zvfs_error_unknown="Unknown error";

const char *zvfs_get_strerror(FRESULT error)
{
    switch (error) {
        case FR_NO_FILESYSTEM:
            return zvfs_error_no_filesystem;
        break;

        default:
            return zvfs_error_unknown;
        break;
    }

}