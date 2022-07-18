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
   Menu fileselector functions
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>



#include "zxvision.h"
#include "menu_file_viewer_browser.h"
#include "screen.h"
#include "cpu.h"
#include "debug.h"
#include "timer.h"
#include "utils.h"
#include "compileoptions.h"
#include "ff.h"
#include "diskio.h"
#include "zvfs.h"
#include "snap.h"
#include "joystick.h"
#include "textspeech.h"
#include "network.h"

#if defined(__APPLE__)
	#include <sys/syslimits.h>

	#include <sys/resource.h>

#endif


#ifdef COMPILE_STDOUT
	#include "scrstdout.h"
//macro llama a funcion real
	#define scrstdout_menu_print_speech_macro scrstdout_menu_print_speech
//funcion llama
#else
//funcion no llama a nada
	#define scrstdout_menu_print_speech_macro(x)
#endif

//Elemento que identifica a un archivo en funcion de seleccion
struct s_filesel_item{
	char d_name[PATH_MAX];

    //siguiente item
    struct s_filesel_item *next;
};

typedef struct s_filesel_item filesel_item;

int filesel_total_items;
filesel_item *primer_filesel_item;

//linea seleccionada en selector de archivos (relativa al primer archivo 0...20 maximo seguramente)
int filesel_linea_seleccionada;

//numero de archivo seleccionado en selector (0...ultimo archivo en directorio)
int filesel_archivo_seleccionado;

//indica en que zona del selector estamos:
//0: nombre archivo
//1: selector de archivo
//2: zona filtros
int filesel_zona_pantalla;


//nombre completo (nombre+path)del archivo seleccionado
char filesel_nombre_archivo_seleccionado[PATH_MAX];

//Si mostrar en filesel utilidades de archivos
z80_bit menu_filesel_show_utils={0};

//Decir que en el menu drives aparecera (si es que esta montado) imagen mmc, aunque no estemos con file utils activo
z80_bit menu_filesel_drives_allow_fatfs={0};

//Si mostrar en filesel previews de archivos
z80_bit menu_filesel_show_previews={1};

//Si reducir previews a la mitad
z80_bit menu_filesel_show_previews_reduce={0};

//Si no caben todos los archivos en pantalla y por tanto se muestra "*" a la derecha
int filesel_no_cabe_todo;

//Total de archivos en el directorio mostrado
int filesel_total_archivos;

//En que porcentaje esta el indicador
int filesel_porcentaje_visible;


//filtros activos
char **filesel_filtros;

//filtros iniciales con los que se llama a la funcion
char **filesel_filtros_iniciales;

//filtro de todos archivos
char *filtros_todos_archivos[2];


//Ultimo directorio al salir con ESC desde fileselector
char menu_filesel_last_directory_seen[PATH_MAX];

//directorio inicial al entrar
char filesel_directorio_inicial[PATH_MAX];

//No mostrar subdirectorios en file selector
z80_bit menu_filesel_hide_dirs={0};

//No mostrar tamanyos en file selector
z80_bit menu_filesel_hide_size={0};

//Permitir borrar carpetas en file browser 
z80_bit menu_filesel_utils_allow_folder_delete={0};


int menu_recent_files_opcion_seleccionada=0;







filesel_item *menu_get_filesel_item(int index);
int menu_filesel_file_can_be_expanded(char *archivo);

//devuelve 1 si el directorio cumple el filtro
//realmente lo que hacemos aqui es ocultar/mostrar carpetas que empiezan con .
int menu_file_filter_dir(const char *name,char *filtros[])
{

        int i;
        //char extension[1024];

        //directorio ".." siempre se muestra
        if (!strcmp(name,"..")) return 1;

        char *f;


        //Bucle por cada filtro
        for (i=0;filtros[i];i++) {
                //si filtro es "", significa todo (*)
                //supuestamente si hay filtro "" no habrian mas filtros pasados en el array...

 	       f=filtros[i];
                if (f[0]==0) return 1;

                //si filtro no es *, ocultamos los que empiezan por "."
                if (name[0]=='.') return 0;

        }


	//y finalmente mostramos el directorio
        return 1;

}





//devuelve 1 si el archivo cumple el filtro
int menu_file_filter(const char *name,char *filtros[])
{

	int i;
	char extension[NAME_MAX];

	/*
	//obtener extension del nombre
	//buscar ultimo punto

	int j;
	j=strlen(name);
	if (j==0) extension[0]=0;
	else {
		for (;j>=0 && name[j]!='.';j--);

		if (j>=0) strcpy(extension,&name[j+1]);
		else extension[0]=0;
	}

	//printf ("Extension: %s\n",extension);

	*/

	//El archivo MENU_LAST_DIR_FILE_NAME zesarux_last_dir.txt usado para abrir archivos comprimidos, no lo mostrare nunca
	if (!strcmp(name,MENU_LAST_DIR_FILE_NAME)) return 0;

	//Archivo usado para indicar que archivo es la pantalla del juego. Usado en previews de tap, tzx etc
	if (!strcmp(name,MENU_SCR_INFO_FILE_NAME)) return 0;

	util_get_file_extension((char *) name,extension);

	char *f;

	//Si filtro[0]=="nofiles" no muestra ningun archivo
	if (!strcasecmp(filtros[0],"nofiles")) return 0;


	//Filtro para tipos autosnap "autosnap(o el que sea el prefijo)*.zsf"
	if (!strcasecmp(filtros[0],"autosnap")) {
		if (!strcasecmp(extension,"zsf")) {
			//prefijo snapshot_autosave_interval_quicksave_name
			char *existe;
			existe=strstr(name,snapshot_autosave_interval_quicksave_name);
			if (existe!=NULL) {
				//Y tiene que ser al inicio
				if (existe==name) return 1;
			}
		}

		//en caso contrario, no cumple el filtro
		return 0;
	}

	//Bucle por cada filtro 
	for (i=0;filtros[i];i++) {
		//si filtro es "", significa todo (*)
		//supuestamente si hay filtro "" no habrian mas filtros pasados en el array...

		f=filtros[i];
		//printf ("f: %d\n",f);
		if (f[0]==0) return 1;

		//si filtro no es *, ocultamos los que empiezan por "."
		//Aparentemente esto no tiene mucho sentido, con esto ocultariamos archivo de nombre tipo ".xxxx.tap" por ejemplo
		//Pero bueno, para volumenes que vienen de mac os x, los metadatos se guardan en archivos tipo:
		//._0186.tap
		if (name[0]=='.') return 0;


		//comparamos extension
		if (!strcasecmp(extension,f)) return 1;
	}

    //Otros archivos que siempre cumplen el filtro
	//Aqui agregamos todas las extensiones que en principio pueden generar muchos diferentes tipos de archivos,
	//ya sea porque son archivos comprimidos (p.ej. zip) o porque son archivos que se pueden expandir (p.j. tap)
	//Hay algunos que se pueden expandir y directamente los excluyo (como .P o .O) por ser su uso muy limitado 
	//(solo generan .baszx80 y .baszx81 en este caso)

	if (!strcasecmp(extension,"zip")) return 1;

	if (!strcasecmp(extension,"gz")) return 1;

	if (!strcasecmp(extension,"tar")) return 1;

	if (!strcasecmp(extension,"rar")) return 1;

	if (!strcasecmp(extension,"mdv")) return 1;

	if (!strcasecmp(extension,"hdf")) return 1;

	if (!strcasecmp(extension,"dsk")) return 1;

	if (!strcasecmp(extension,"tap")) return 1;	

	if (!strcasecmp(extension,"tzx")) return 1;	

	if (!strcasecmp(extension,"pzx")) return 1;

	if (!strcasecmp(extension,"trd")) return 1;		

	if (!strcasecmp(extension,"scl")) return 1;			

	if (!strcasecmp(extension,"epr")) return 1;
	if (!strcasecmp(extension,"eprom")) return 1;
	if (!strcasecmp(extension,"flash")) return 1;		


	return 0;

}

int menu_filesel_filter_func(const struct dirent *d)
{


	int tipo_archivo=get_file_type((char *)d->d_name);


	//si es directorio, ver si empieza con . y segun el filtro activo
	//Y si setting no mostrar directorios, no mostrar
	if (tipo_archivo == 2) {
		if (menu_filesel_hide_dirs.v) return 0;
		if (menu_file_filter_dir(d->d_name,filesel_filtros)==1) return 1;
		return 0;
	}

	//Si no es archivo ni link, no ok

	if (tipo_archivo  == 0) {


		debug_printf (VERBOSE_DEBUG,"Item is not a directory, file or link");

		return 0;
	}

	//es un archivo. ver el nombre

	if (menu_file_filter(d->d_name,filesel_filtros)==1) return 1;


	return 0;
}

int menu_filesel_alphasort(const struct dirent **d1, const struct dirent **d2)
{

	//printf ("menu_filesel_alphasort %s %s\n",(*d1)->d_name,(*d2)->d_name );

	//compara nombre
	return (strcasecmp((*d1)->d_name,(*d2)->d_name));
}

//Copia nombre y tipo de archivo de estructura de fatfs a dirent
void menu_filesel_fatfs_to_dirent(FILINFO* fno,struct dirent *dp)
{
    strcpy(dp->d_name,fno->fname);

    //d_type no se usa. de hecho, hay que EVITAR usar esa propiedad pues no está en todos los sistemas operativos,
    //haiku por ejemplo no la tiene

    /*
    The only fields in the dirent structure that are mandated by
       POSIX.1 are d_name and d_ino.  The other fields are
       unstandardized, and not present on all systems; see NOTES below
       for some further details.
    */

    /*
    if (fno->fattrib & AM_DIR) {
        dp->d_type=DT_DIR;
    }
    else {
        dp->d_type=DT_REG;
    } 
    */                   

}  

int menu_filesel_readdir_mmc_image(const char *directorio, struct dirent ***namelist,
              int (*filter)(const struct dirent *),
              int (*compar)(const struct dirent **, const struct dirent **))
{


	#define MAX_ARCHIVOS_SCANDIR_MINGW 20000
        int archivos=0;

        //Puntero a cada archivo leido
        struct dirent *memoria_archivos;

        //Array de punteros.
        struct dirent **memoria_punteros;

        //Asignamos memoria
        memoria_punteros=malloc(sizeof(struct dirent *)*MAX_ARCHIVOS_SCANDIR_MINGW);


        if (memoria_punteros==NULL) {
                cpu_panic("Error allocating memory when reading directory");
        }

        *namelist=memoria_punteros;

        //int indice_puntero;

        //printf("leyendo directorio %s desde menu_filesel_readdir_mmc_image\n",directorio);

       struct dirent dp;


        FRESULT res;
       FATFS_DIR dir;

       static FILINFO fno;

        res = f_opendir(&dir, directorio);                       /* Open the directory */
        if (res != FR_OK) {       
           //printf("Error abriendo directorio de mmc: %s\n",directorio);
           debug_printf(VERBOSE_ERR,"Can't open directory %s", directorio);
           return -1;
       }

        int salir=0;

        //FatFS parece que nunca muestra . o .., lo agregamos si no aparece
        int got_dotdot=0;

        while (!salir) {

                    //printf("antes readdir\n");
                   res = f_readdir(&dir, &fno);                   /* Read a directory item */
                   //printf("despues readdir\n");

                    if (res != FR_OK || fno.fname[0] == 0) {
                        //printf("temp: %s\n",fno.fname);
                        //printf("Fin leyendo directorio. res=%d\n",res);
                        //break;  /* Break on error or end of dir */
                        salir=1;
                    }

                else {
                    debug_printf(VERBOSE_DEBUG,"menu_filesel_readdir_mmc_image: file: %s",fno.fname);
                    //printf("menu_filesel_readdir_mmc_image: file: %s\n",fno.fname);

                    if (!strcmp(fno.fname,"..")) {
                        //printf("Hay ..\n");
                        got_dotdot=1;
                    }

                    //Pasar por el filtro, pero este solo entiende dirent
                    menu_filesel_fatfs_to_dirent(&fno,&dp);

                    if (filter(&dp)) {

                            //Asignar memoria para ese fichero
                            memoria_archivos=malloc(sizeof(struct dirent));

                        if (memoria_archivos==NULL) {
                                    cpu_panic("Error allocating memory when reading directory");
                            }

                            //Meter puntero
                            memoria_punteros[archivos]=memoria_archivos;

                            //Meter datos

                            memcpy(memoria_archivos,&dp,sizeof( struct dirent ));


                            archivos++;

                            if (archivos>=MAX_ARCHIVOS_SCANDIR_MINGW) {
                                    debug_printf(VERBOSE_ERR,"Error. Maximum files in directory reached: %d",MAX_ARCHIVOS_SCANDIR_MINGW);
                                    return archivos;
                            }

                    }
                }


       }
       f_closedir(&dir);

        //Agregar .. siempre que no estemos en la raiz
        //Nota mental: los strcmp son 0 cuando la comparacion es cierta
        //Aqui lo que queremos es que no sea cierta la comparacion, por tanto seria !!strcmp, o sea, strcmp
        //TODO: Si directorio es "."
        char directorio_actual[1024];
        zvfs_getcwd(directorio_actual,1023);
        //printf("ruta actual despues de leer directorio: %s\n",directorio_actual);

        if (!got_dotdot && strcmp(directorio_actual,"/") && strcmp(directorio_actual,"0:/") && strcmp(directorio_actual,"0://")) {

            //TODO: hacer que pase misma funcion de filter

            //printf("Adding .. entry\n");

            //Asignar memoria para ese fichero
            memoria_archivos=malloc(sizeof(struct dirent));

            if (memoria_archivos==NULL) {
                cpu_panic("Error allocating memory when reading directory");
            }

            //Meter puntero
            memoria_punteros[archivos]=memoria_archivos;

            //Meter datos
            strcpy(memoria_archivos->d_name,"..");

            //memoria_archivos->d_type=DT_DIR;

            archivos++;     

            if (archivos>=MAX_ARCHIVOS_SCANDIR_MINGW) {
                debug_printf(VERBOSE_ERR,"Error. Maximum files in directory reached: %d",MAX_ARCHIVOS_SCANDIR_MINGW);
                return archivos;
            }          


    }

	//lanzar qsort
	int (*funcion_compar)(const void *, const void *);

	funcion_compar=( int (*)(const void *, const void *)  )compar;

	qsort(memoria_punteros,archivos,sizeof( struct dirent *), funcion_compar);

    return archivos;


}

int menu_filesel_readdir(void)
{

/*
       lowing macro constants for the value returned in d_type:

       DT_BLK      This is a block device.

       DT_CHR      This is a character device.

       DT_DIR      This is a directory.

       DT_FIFO     This is a named pipe (FIFO).

       DT_LNK      This is a symbolic link.

       DT_REG      This is a regular file.

       DT_SOCK     This is a UNIX domain socket.

       DT_UNKNOWN  The file type is unknown.

*/

debug_printf(VERBOSE_DEBUG,"Reading directory");

filesel_total_items=0;
primer_filesel_item=NULL;


    struct dirent **namelist;

	struct dirent *nombreactual;

    int n;
//printf ("usando scandir\n");

	filesel_item *item;
	filesel_item *itemanterior;

    // Si unidad actual es la mmc montada
    //if (fatfs_disk_zero_memory!=NULL) 
    if (menu_current_drive_mmc_image.v)
    {
        n = menu_filesel_readdir_mmc_image(".", &namelist, menu_filesel_filter_func, menu_filesel_alphasort);
    }

    else {

#ifndef MINGW
	n = scandir(".", &namelist, menu_filesel_filter_func, menu_filesel_alphasort);
#else
	//alternativa scandir, creada por mi
	n = scandir_mingw(".", &namelist, menu_filesel_filter_func, menu_filesel_alphasort);
#endif

    }

    if (n < 0) {
		debug_printf (VERBOSE_ERR,"Error reading directory contents: %s",strerror(errno));
		return 1;
	}

    else {
        int i;

	//printf("total elementos directorio: %d\n",n);

        for (i=0;i<n;i++) {
		nombreactual=namelist[i];
            //printf("%s\n", nombreactual->d_name);
            //printf("%d\n", nombreactual->d_type);


		item=malloc(sizeof(filesel_item));
		if (item==NULL) cpu_panic("Error allocating file item");

		strcpy(item->d_name,nombreactual->d_name);


		item->next=NULL;

		//primer item
		if (primer_filesel_item==NULL) {
			primer_filesel_item=item;
		}

		//siguientes items
		else {
			itemanterior->next=item;
		}

		itemanterior=item;
		free(namelist[i]);


		filesel_total_items++;
        }

		free(namelist);

    }

	return 0;
	//free(namelist);

}


//Retorna 1 si ok
//Retorna 0 si no ok
int menu_avisa_si_extension_no_habitual(char *filtros[],char *archivo)
{

	int i;

	//Si es filtro "autosnap" es en teoria zsf
	if (!strcmp(filtros[0],"autosnap")) {
		if (!util_compare_file_extension(archivo,"zsf")) return 1;
	}


	for (i=0;filtros[i];i++) {
		if (!util_compare_file_extension(archivo,filtros[i])) return 1;

		//si filtro es "", significa todo (*)
		if (!strcmp(filtros[i],"")) return 1;

	}



	//no es extension habitual. Avisar
	return menu_confirm_yesno_texto("Unusual file extension","Do you want to use this file?");
}


int menu_filesel_copy_recursive(char *directorio_origen, char *directorio_destino,int simular)
{

/*
1) mkdir destino/carpeta. Entrar en destino/carpeta. Entrar en origen/carpeta. 
2) listado todo el directorio. Para cada archivo, copiar en destino
3) si es directorio, gosub 1)
4) si fin directorio, return

*/
    debug_printf(VERBOSE_DEBUG,"Copy_recursive: entering directory copy %s to %s",directorio_origen,directorio_destino);

    
    debug_printf(VERBOSE_DEBUG,"Copy_recursive: mkdir destination %s",directorio_destino);

    if (!simular) zvfs_mkdir(directorio_destino);

    int in_fatfs_origen=util_path_is_mmc_fatfs(directorio_origen);
    //int in_fatfs_destino=util_path_is_mmc_fatfs(directorio_destino);
    


    

    struct dirent *dp;
    DIR *dfd;

    FRESULT res;
    FATFS_DIR dir;

    static FILINFO fno;

    if (in_fatfs_origen) {

        res = f_opendir(&dir, directorio_origen);                       /* Open the directory */
        if (res != FR_OK) {       
        //printf("Error abriendo directorio de mmc: %s\n",directorio);
        debug_printf(VERBOSE_ERR,"Can't open directory %s", directorio_origen);
        return -1;

        }
    }

    else {

        if ((dfd = opendir(directorio_origen)) == NULL) {
            debug_printf(VERBOSE_ERR,"Can't open directory %s", directorio_origen);
            return -1;
        }
    }

    int salir=0;

    //FatFS parece que nunca muestra . o .., lo agregamos si no aparece
    //int got_dotdot=0;

    char archivo_origen_fullpath[PATH_MAX];
    char archivo_destino_fullpath[PATH_MAX];

    while (!salir) {

        char *nombre_origen;
        int origen_es_directorio=0;

        if (in_fatfs_origen) {
            //printf("antes readdir\n");
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            //printf("despues readdir\n");

            if (res != FR_OK || fno.fname[0] == 0) {
                //printf("temp: %s\n",fno.fname);
                //printf("Fin leyendo directorio. res=%d\n",res);
                //break;  /* Break on error or end of dir */
                salir=1;
            }
            else {


                    sprintf(archivo_origen_fullpath,"%s/%s",directorio_origen,fno.fname);
                    sprintf(archivo_destino_fullpath,"%s/%s",directorio_destino,fno.fname);


                nombre_origen=fno.fname;
                if (fno.fattrib & AM_DIR) {
                    origen_es_directorio=1;
                    //printf("%s es directorio\n",archivo_origen_fullpath);
                }
                else {
                    //printf("%s es archivo\n",archivo_origen_fullpath);   
                }
            }
        }
        else {
            dp = readdir(dfd);

            if (dp==NULL) salir=1;

            else {

                    sprintf(archivo_origen_fullpath,"%s/%s",directorio_origen,dp->d_name);
                    sprintf(archivo_destino_fullpath,"%s/%s",directorio_destino,dp->d_name);

                nombre_origen=dp->d_name;
                if (get_file_type(archivo_origen_fullpath)==2) {
                    //printf("%s es directorio\n",archivo_origen_fullpath);
                    origen_es_directorio=1;
                }
                else {
                    //printf("%s es archivo\n",archivo_origen_fullpath);
                }
            }
        }



            
        if (!salir) {
            //Si es directorio y no . ni .. , llamar recursivamente
            if (origen_es_directorio) {
                if (!strcasecmp(nombre_origen,".") ||
                    !strcasecmp(nombre_origen,"..")
                ) {
                    //Ignorar
                }
                else {
                    

                    menu_filesel_copy_recursive(archivo_origen_fullpath,archivo_destino_fullpath,simular);
                }
            }

            else {
                //Si es archivo, copiar a destino
                //char archivo_copiar_origen[PATH_MAX];
                //char archivo_copiar_destino[PATH_MAX];

                //sprintf(archivo_copiar_origen,"%s/%s",directorio_origen,nombre_origen);
                //sprintf(archivo_copiar_destino,"%s/%s",directorio_destino,nombre_origen);

                debug_printf(VERBOSE_DEBUG,"Copy_recursive: copy file %s to %s",archivo_origen_fullpath,archivo_destino_fullpath);

                if (!simular) {
                    util_copy_file(archivo_origen_fullpath,archivo_destino_fullpath);
                }
            }

            
                //Asignar memoria para ese fichero
    

        }
            


    }
    if (in_fatfs_origen) f_closedir(&dir);
    else closedir(dfd);

    //printf("Close dir %s , %s\n\n",directorio_origen,directorio_destino);

    return 0;



}


int menu_filesel_delete_recursive(char *directorio_origen ,int simular)
{

/*
1) Entrar en origen/carpeta. 
2) listado todo el directorio. Para cada archivo, borrar
3) si es directorio, gosub 1). Y borrar carpeta
4) si fin directorio, remove folder. return

*/
    debug_printf(VERBOSE_DEBUG,"Delete_recursive: entering directory %s",directorio_origen);



    int in_fatfs_origen=util_path_is_mmc_fatfs(directorio_origen);


    struct dirent *dp;
    DIR *dfd;

    FRESULT res;
    FATFS_DIR dir;

    static FILINFO fno;

    if (in_fatfs_origen) {

        res = f_opendir(&dir, directorio_origen);                       /* Open the directory */
        if (res != FR_OK) {       
        //printf("Error abriendo directorio de mmc: %s\n",directorio);
        debug_printf(VERBOSE_ERR,"Can't open directory %s", directorio_origen);
        return -1;

        }
    }

    else {

        if ((dfd = opendir(directorio_origen)) == NULL) {
            debug_printf(VERBOSE_ERR,"Can't open directory %s", directorio_origen);
            return -1;
        }
    }

    int salir=0;


    char archivo_origen_fullpath[PATH_MAX];


    while (!salir) {

        char *nombre_origen;
        int origen_es_directorio=0;

        if (in_fatfs_origen) {
            //printf("antes readdir\n");
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            //printf("despues readdir\n");

            if (res != FR_OK || fno.fname[0] == 0) {

                salir=1;
            }
            else {

                sprintf(archivo_origen_fullpath,"%s/%s",directorio_origen,fno.fname);


                nombre_origen=fno.fname;
                if (fno.fattrib & AM_DIR) {
                    origen_es_directorio=1;
                    //printf("%s es directorio\n",archivo_origen_fullpath);
                }
                else {
                    //printf("%s es archivo\n",archivo_origen_fullpath);   
                }
            }
        }


        else {
            dp = readdir(dfd);

            if (dp==NULL) salir=1;

            else {

                sprintf(archivo_origen_fullpath,"%s/%s",directorio_origen,dp->d_name);

                nombre_origen=dp->d_name;

                if (get_file_type(archivo_origen_fullpath)==2) {
                    //printf("%s es directorio\n",archivo_origen_fullpath);
                    origen_es_directorio=1;
                }
                else {
                    //printf("%s es archivo\n",archivo_origen_fullpath);
                }
            }
        }



            
        if (!salir) {
            //Si es directorio y no . ni .. , llamar recursivamente
            if (origen_es_directorio) {
                if (!strcasecmp(nombre_origen,".") ||
                    !strcasecmp(nombre_origen,"..")
                ) {
                    //Ignorar
                }
                else {
                    //Volver a llamarse 
                    menu_filesel_delete_recursive(archivo_origen_fullpath,simular);

                
                }
            }

            else {
                //Si es archivo, borrar

                debug_printf(VERBOSE_DEBUG,"Delete_recursive: delete file %s",archivo_origen_fullpath);

                if (!simular) {
                    zvfs_delete(archivo_origen_fullpath);
                }
            }

              

        }
            


    }
    if (in_fatfs_origen) f_closedir(&dir);
    else closedir(dfd);

    //printf("Close dir %s \n\n",directorio_origen);


    //Y luego borrar carpeta
    debug_printf(VERBOSE_DEBUG,"Delete_recursive: delete folder %s",directorio_origen);

    if (!simular) {
        zvfs_delete(directorio_origen);
    }        

    return 0;



}






#define FILESEL_INICIAL_ANCHO 32
#define FILESEL_MAX_ANCHO OVERLAY_SCREEN_MAX_WIDTH

#define FILESEL_INICIAL_ALTO 24

#define FILESEL_INICIAL_X (menu_center_x()-FILESEL_INICIAL_ANCHO/2)
#define FILESEL_INICIAL_Y (menu_center_y()-FILESEL_INICIAL_ALTO/2)

#define FILESEL_INICIO_DIR 4

#define ZXVISION_POS_FILTER 6
#define ZXVISION_POS_LEYENDA 7




void zxvision_menu_filesel_print_filters(zxvision_window *ventana,char *filtros[])
{


        if (menu_filesel_show_utils.v) return; //Si hay utilidades activas, no mostrar filtros

        //texto para mostrar filtros. darle bastante margen aunque no quepa en pantalla
        char buffer_filtros[FILESEL_MAX_ANCHO+1]; //+1 para el 0 final


        char *f;

        int i,p;
        p=0;
        sprintf(buffer_filtros,"Filter: ");

        p=p+8;  //8 es lo que ocupa el texto "Filter: "


        for (i=0;filtros[i];i++) {
                //si filtro es "", significa todo (*)

                f=filtros[i];
                if (f[0]==0) f="*";

                //copiamos
                //sprintf(&buffer_filtros[p],"*.%s ",f);
                sprintf(&buffer_filtros[p],"%s ",f);
                p=p+strlen(f)+1;

        }

//Si texto filtros pasa del tope, rellenar con "..."
		int max_visible=(ventana->visible_width)-2;
	
        if (p>max_visible && max_visible>=3) {
                p=max_visible;
                buffer_filtros[p-1]='.';
                buffer_filtros[p-2]='.';
                buffer_filtros[p-3]='.';
        }


        buffer_filtros[p]=0;


        //borramos primero con espacios


	int posicion_filtros=ZXVISION_POS_FILTER;


	zxvision_print_string_defaults_fillspc(ventana,1,posicion_filtros,"");


        //y luego escribimos


        //si esta filesel_zona_pantalla=2, lo ponemos en otro color. TODO
        int inverso=0;
        if (filesel_zona_pantalla==2) inverso=1;



	int tinta=ESTILO_GUI_TINTA_NORMAL;
	int papel=ESTILO_GUI_PAPEL_NORMAL;

	if (inverso) {
		tinta=ESTILO_GUI_TINTA_SELECCIONADO;
		papel=ESTILO_GUI_PAPEL_SELECCIONADO;
	}


	zxvision_print_string(ventana,1,posicion_filtros,tinta,papel,0,buffer_filtros);
}

//Dice el archivo seleccionado por el cursor
filesel_item *menu_get_filesel_item_cursor(void)
{


    filesel_item *item_seleccionado;


    item_seleccionado=menu_get_filesel_item(filesel_archivo_seleccionado+filesel_linea_seleccionada);

    return item_seleccionado;


}

void filesel_return_free_mmc_mounted(int *total, int *free)
{
    //Retorna en MB espacio libre de la mmc
        FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;

    FRESULT res;

    /* Get volume information and free clusters of drive 1 */
    res = f_getfree("0:", &fre_clust, &fs);
    if (res) return;

    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

    /* Print the free space (assuming 512 bytes/sector) */
    //printf("%10d KiB total drive space.\n%10d KiB available.\n", tot_sect / 2, fre_sect / 2);

    int mb_total=tot_sect / 2/1024;
    int mb_free=fre_sect / 2 / 1024;

    *total=mb_total;
    *free=mb_free;

}

void zxvision_menu_filesel_print_legend(zxvision_window *ventana)
{

    //Forzar a mostrar atajos
    z80_bit antes_menu_writing_inverse_color;
    antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
    menu_writing_inverse_color.v=1;

	int posicion_leyenda=ZXVISION_POS_LEYENDA;
	int posicion_filtros=ZXVISION_POS_FILTER;

    //Obtener tipo de archivo al que apunta para saber si es archivo o directorio, para ocultar textos leyenda
    int es_directorio=0;

    filesel_item *item_seleccionado;

    item_seleccionado=menu_get_filesel_item_cursor();
    if (item_seleccionado!=NULL) {

        int tipo_archivo_seleccionado=get_file_type(item_seleccionado->d_name);

        //Si es directorio
        if (tipo_archivo_seleccionado==2) es_directorio=1;
    }


    if (menu_filesel_show_utils.v) {


        int ancho_visible=ventana->visible_width;

        char buffer_line_actions_short[OVERLAY_SCREEN_MAX_WIDTH+1];
        char buffer_line_actions_long[OVERLAY_SCREEN_MAX_WIDTH+1];
        char buffer_linea[OVERLAY_SCREEN_MAX_WIDTH+1];


        //                         01234  567890  12345  678901  2345678901
        sprintf(buffer_line_actions_short,"%sM~^Kdr ~^Inf",
                (es_directorio ? "" : "~^View ~^Trunc C~^Onv ~^Filemem ")
        );

        sprintf(buffer_line_actions_long,"%sMa~^Kedir ~^Info",
                (es_directorio ? "" : "~^View ~^Truncate C~^Onvert ~^Filemem ")
        );        

        menu_get_legend_short_long(buffer_linea,ancho_visible,buffer_line_actions_short,buffer_line_actions_long);
        zxvision_print_string_defaults_fillspc(ventana,1,posicion_filtros-1,buffer_linea);




        char buffer_sync[32];
        if (menu_mmc_image_montada) {
            strcpy(buffer_sync,"~^Umount ~^Sync ");
        }
        else {
            if (es_directorio) buffer_sync[0]=0;
            else strcpy(buffer_sync,"mo~^Unt ");
        }

        /*
        sprintf(buffer_linea,"%sD~^El Re~^N ~^Paste ~^Copy %s",
            //move, de momento  solo para archivos
            (es_directorio ? "" : "~^Move "),
            buffer_sync);
        */

        sprintf(buffer_line_actions_short,"%sD~^El Re~^N ~^Paste ~^Copy ~^Move",buffer_sync);
        sprintf(buffer_line_actions_long,"%sD~^Elete Re~^Name ~^Paste ~^Copy ~^Move",buffer_sync);

        menu_get_legend_short_long(buffer_linea,ancho_visible,buffer_line_actions_short,buffer_line_actions_long);
        zxvision_print_string_defaults_fillspc(ventana,1,posicion_filtros,buffer_linea);

    }

	char leyenda_inferior[64];


    //Si se puede expandir
    char buffer_expand[32];
    buffer_expand[0]=0;

    if (!es_directorio) {
        if (item_seleccionado!=NULL) {
            if (menu_filesel_file_can_be_expanded(item_seleccionado->d_name)) {
                strcpy(buffer_expand," ~^S~^P~^C: Expand");
            }
        }
    }    


	//Drive también mostrado en Linux y Mac
    //01234567890123456789012345678901
    // TAB: Section R: Recent D: Drive
	sprintf (leyenda_inferior,"~^T~^A~^B:Section ~^Recent ~^Drives%s",buffer_expand);

	zxvision_print_string_defaults_fillspc(ventana,1,posicion_leyenda,leyenda_inferior);

    //Restaurar comportamiento mostrar atajos
    menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;
}



filesel_item *menu_get_filesel_item(int index)
{
	filesel_item *p;

	p=primer_filesel_item;

	int i;

	for(i=0;i<index;i++) {
		p=p->next;
	}

	return p;

}

//Dice si archivo es de tipo comprimido/empaquetado. filename tiene que ser sin directorio
int menu_util_file_is_compressed(char *filename)
{
		//Si seleccion es archivo comprimido
							if (
							    //strstr(item_seleccionado->d_name,".zip")!=NULL ||
							    !util_compare_file_extension(filename,"zip") ||
                                                            !util_compare_file_extension(filename,"gz")  ||
                                                            !util_compare_file_extension(filename,"tar") ||
                                                            !util_compare_file_extension(filename,"rar") 


							) {
								return 1;
							}
	else return 0;
}

//obtiene linea a escribir con nombre de archivo + carpeta
void menu_filesel_print_file_get(char *buffer, char *s,unsigned int max_length_shown)
{
	unsigned int i;

        for (i=0;i<max_length_shown && (s[i])!=0;i++) {
                buffer[i]=s[i];
        }


        //si sobra espacio, rellenar con espacios
        for (;i<max_length_shown;i++) {
                buffer[i]=' ';
        }

        buffer[i]=0;


        //si no cabe, poner puntos suspensivos
        if (strlen(s)>max_length_shown && i>=3) {
                buffer[i-1]='.';
                buffer[i-2]='.';
                buffer[i-3]='.';
        }

    //y si es un directorio (sin nombre nulo ni espacio), escribir "<dir>
	//nota: se envia nombre " " (un espacio) cuando se lista el directorio y sobran lineas en blanco al final

	int test_dir=1;

	if (s[0]==0) test_dir=0;
	if (s[0]==' ' && s[1]==0) test_dir=0;

	if (test_dir) {
	        if (get_file_type(s) == 2 && i>=5) {
        	        buffer[i-1]='>';
                	buffer[i-2]='r';
	                buffer[i-3]='i';
        	        buffer[i-4]='d';
                	buffer[i-5]='<';
	        }

            else {
                //Mostrar tamanyo. Si no hay setting de desactivado
                if (menu_filesel_hide_size.v==0) {
                    long int tamanyo=get_file_size(s);
                    char buffer_tamanyo[100];
                    char buffer_sufijo[10];

                    tamanyo=get_size_human_friendly(tamanyo,buffer_sufijo);

                    //Con espacio por delante para separar, por si acaso ancho ventana muy pequeña
                    sprintf(buffer_tamanyo," %ld %s",tamanyo,buffer_sufijo);

                    unsigned int longitud_texto=strlen(buffer_tamanyo);

                    //printf("%s i: %d longitud_texto: %d\n",s,i,longitud_texto);

                    if (i>=longitud_texto) strcpy(&buffer[i-longitud_texto],buffer_tamanyo);
                }
            }

			//O si es empaquetado
			/*else if (menu_util_file_is_compressed(s) && i>=5) {
				    buffer[i-1]='>';
                	buffer[i-2]='p';
	                buffer[i-3]='x';
        	        buffer[i-4]='e';
                	buffer[i-5]='<';
			}*/
	}


}

//escribe el nombre de archivo o carpeta

//Margen de 8 lineas (4+4) de leyendas
#define ZXVISION_FILESEL_INITIAL_MARGIN 8

void zxvision_menu_filesel_print_file(zxvision_window *ventana,char *s,unsigned int max_length_shown,int y)
{

        char buffer[PATH_MAX];



        menu_filesel_print_file_get(buffer, s, max_length_shown);


	zxvision_print_string_defaults_fillspc(ventana,1,y+ZXVISION_FILESEL_INITIAL_MARGIN,buffer);	
}




void menu_filesel_switch_filters(void)
{

	//si filtro inicial, ponemos el *.*
	if (filesel_filtros==filesel_filtros_iniciales)
		filesel_filtros=filtros_todos_archivos;

	//si filtro *.* , ponemos el filtro inicial
	else filesel_filtros=filesel_filtros_iniciales;

}



/*char menu_minus_letra(char letra)
{
	if (letra>='A' && letra<='Z') letra=letra+('a'-'A');
	return letra;
}*/



void zxvision_menu_filesel_localiza_letra(zxvision_window *ventana,char letra)
{

        int i;
        filesel_item *p;
        p=primer_filesel_item;

        for (i=0;i<filesel_total_items;i++) {
                if (letra_minuscula(p->d_name[0])>=letra_minuscula(letra)) {
                        filesel_linea_seleccionada=0;
                        filesel_archivo_seleccionado=i;
			zxvision_set_cursor_line(ventana,i);
			zxvision_set_offset_y_or_maximum(ventana,i);
			//printf ("linea seleccionada en localizacion: %d\n",i);
                        return;
                }


                p=p->next;
        }

}



void zxvision_menu_filesel_localiza_archivo(zxvision_window *ventana,char *nombrebuscar)
{
        debug_printf (VERBOSE_DEBUG,"Searching last file %s",nombrebuscar);
        int i;
        filesel_item *p;
        p=primer_filesel_item;

        for (i=0;i<filesel_total_items;i++) {
                debug_printf (VERBOSE_DEBUG,"File number: %d Name: %s",i,p->d_name);
                //if (menu_minus_letra(p->d_name[0])>=menu_minus_letra(letra)) {
                if (strcasecmp(nombrebuscar,p->d_name)<=0) {
                        filesel_linea_seleccionada=0;
                        filesel_archivo_seleccionado=i;
						zxvision_set_cursor_line(ventana,i);
						zxvision_set_offset_y_or_maximum(ventana,i);
                        debug_printf (VERBOSE_DEBUG,"Found at position %d",i);
                        return;
                }


                p=p->next;
        }

}


int si_menu_filesel_no_mas_alla_ultimo_item(int linea)
{
	if (filesel_archivo_seleccionado+linea<filesel_total_items-1) return 1;
	return 0;
}

void file_utils_mount_mmc_image_prueba_escribir(void)
{
    FIL fil;        /* File object */
    //char line[100]; /* Line buffer */
    FRESULT fr;     /* FatFs return code */

    /* Open a text file */
    fr = f_open(&fil, "README.md", FA_CREATE_ALWAYS | FA_WRITE); 

    //fr = f_open(&fil, "README.md", FA_CREATE_NEW);
    
    //FA_WRITE no lo crea si no existe. Y si existe, con FA_WRITE empieza a escribir desde el principio, conservando
    //tamanyo y bytes no escritos con los antiguos

    /*

Hay que tener en cuenta la tabla de equivalencias:

POSIX	FatFs
"r"	FA_READ
"r+"	FA_READ | FA_WRITE

"w"	FA_CREATE_ALWAYS | FA_WRITE
"w+"	FA_CREATE_ALWAYS | FA_WRITE | FA_READ

"a"	FA_OPEN_APPEND | FA_WRITE
"a+"	FA_OPEN_APPEND | FA_WRITE | FA_READ

"wx"	FA_CREATE_NEW | FA_WRITE
"w+x"	FA_CREATE_NEW | FA_WRITE | FA_READ

    */

    if (fr!=FR_OK) {
        printf("Error abriendo archivo para escritura\n");
        return ; //(int)fr
    }

    char *buffer_texto="Hola que tal";

    UINT escritos;

    f_write(&fil,buffer_texto,strlen(buffer_texto),&escritos);

    if (escritos!=strlen(buffer_texto)) {
        printf("Error escribiendo archivo\n");
    }



    /* Close the file */
    f_close(&fil);    
}

void file_utils_mount_mmc_image_prueba_borrar(void)
{

    FRESULT fr;     /* FatFs return code */

    /* Open a text file */
    fr = f_unlink("README.md");

    if (fr!=FR_OK) {
        printf("error borrando\n");
    }
  
}

/*
void file_utils_mount_mmc_image_prueba_leer(void)
{
    FIL fil;        // File object 
    char line[100]; // Line buffer 
    FRESULT fr;     // FatFs return code 

    // Open a text file 
    fr = f_open(&fil, "README.md", FA_READ);
    if (fr) return ; //(int)fr;


    int salir=0;
    UINT leidos;
    int leer=99;
    while (!salir) {
        FRESULT resultado=f_read(&fil,line,99,&leidos);
        if (resultado==FR_OK) {
            line[leidos]=0;
            printf("%s\n",line);
            if (leidos!=leer) salir=1;
        }
        else {
            salir=1;
        }
    }

    // Close the file 
    f_close(&fil);    
}
*/

FRESULT file_utils_prueba_dir(char *path) 
{
    FRESULT res;
    FATFS_DIR dir;
    UINT i;
    static FILINFO fno;

    const int recursiva=0;

    printf("Abriendo dir: %s\n",path);
    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */


            //Llamar recursivamente
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                if (recursiva) {
                    i = strlen(path);
                    sprintf(&path[i], "/%s", fno.fname);

                    //manera recursiva!
                    res = file_utils_prueba_dir(path);                    /* Enter the directory */
                    if (res != FR_OK) break;
                    path[i] = 0;
                }
                else {
                    printf("%s/%s     <dir>\n", path, fno.fname);
                }
            } 


            else {                                       /* It is a file. */
                if (recursiva) {
                    printf("%s/%s %d\n", path, fno.fname,fno.fsize);
                }
                else {
                    printf("%s %d\n", fno.fname,fno.fsize);
                }
            }
        }
        f_closedir(&dir);
    }

    return res;
}

//Esto va fuera porque todas las operaciones lo usan
FATFS FatFs_menu_mmc_mount;   /* Work area (filesystem object) for logical drive */

//Si tenemos una mmc montada
int menu_mmc_image_montada=0;


//Si unidad actual es la mmc
z80_bit menu_current_drive_mmc_image={0};


//TODO: no estoy seguro del maximo de esto
//Directorio actual en la imagen mmc
//char menu_mmc_cwd[1024]="0:/";

/*void menu_mmc_chdir(char *ruta)
{
    //TODO: gestionar rutas relativas
    strcpy(menu_mmc_cwd)

}*/

void file_utils_umount_mmc_image(void)
{
    //printf("Unmounting image\n");

    //Decir que no montado y cambiar drive a local
    menu_mmc_image_montada=0;
    menu_current_drive_mmc_image.v=0;

    FRESULT resultado=f_mount(0, "", 0);

    if (resultado!=FR_OK) {
        debug_printf(VERBOSE_ERR,"Error desmontando imagen : %d\n",resultado);
        return;
    }    

    menu_first_aid("mount_mmc_fileutils");
}

//Directorio anterior en el local filesystem antes de ir a ruta de imagen montada
char previous_path_before_going_mounted_drive[PATH_MAX]="";

void menu_filesel_guardar_cwd_antes_mounted(char *siguiente_directorio)
{
    //Si pasamos de disco local a 0:/ , guardar ruta anterior (para luego usarla al ir a local drive)
    if (menu_current_drive_mmc_image.v==0 && !strcmp(siguiente_directorio,"0:/")) {
        //printf("Guardando path anterior a montaje\n");
        char current_dir[PATH_MAX];
        zvfs_getcwd(current_dir,PATH_MAX);
        strcpy(previous_path_before_going_mounted_drive,current_dir);
        //printf("y es: %s\n",current_dir);
    }
}


//Retorna 0 si ok
int file_utils_mount_mmc_image(char *fullpath)
{
    debug_printf(VERBOSE_INFO,"Mounting %s",fullpath);

    strcpy(fatfs_disk_zero_path,fullpath);



    //prueba abrir archivo de la mmc

    //disk_initialize(0);



    /* Gives a work area to the default drive */
    FRESULT resultado=f_mount(&FatFs_menu_mmc_mount, "", 1);

    if (resultado!=FR_OK) {
        debug_printf(VERBOSE_ERR,"Error %d mounting image %s: %s",resultado,fullpath,zvfs_get_strerror(resultado));
        return 1;
    }

    menu_mmc_image_montada=1;






    //Y cambiar a dicho directorio
    menu_filesel_guardar_cwd_antes_mounted("0:/");
    zvfs_chdir("0:/");

    return 0;

}    

void file_utils_file_convert(char *fullpath)
{

	//Obtener directorio y archivo
	char archivo[PATH_MAX];
	char directorio[PATH_MAX];

	util_get_file_no_directory(fullpath,archivo);
	util_get_dir(fullpath,directorio);

	//Archivo de destino
	char archivo_destino[PATH_MAX];



	//printf ("convert\n");

	if (!util_compare_file_extension(archivo,"tap")) {
		char *opciones[]={
			"TAP to TZX",
            "TAP to TZX Turbo (4000 bauds)",
            "TAP to PZX",
			"TAP to RWA",
			"TAP to WAV",
			NULL};

		int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}

		switch (opcion) {
			case 0:
				sprintf(archivo_destino,"%s/%s.tzx",directorio,archivo);
				util_extract_tap(fullpath,NULL,archivo_destino,0);
			break;	

			case 1:
				sprintf(archivo_destino,"%s/%s.tzx",directorio,archivo);
				util_extract_tap(fullpath,NULL,archivo_destino,1);
			break;	            

			case 2:
				sprintf(archivo_destino,"%s/%s.pzx",directorio,archivo);
				util_extract_tap(fullpath,NULL,archivo_destino,0);
			break;	            

			case 3:
				sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
				convert_tap_to_rwa(fullpath,archivo_destino);
			break;

			case 4:
				sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
				convert_any_to_wav(fullpath,archivo_destino);
			break;

		}
	}

        else if (!util_compare_file_extension(archivo,"tzx")) {
                char *opciones[]={
						"TZX to TAP",
                        "TZX to RWA",
			"TZX to WAV",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.tap",directorio,archivo);
								util_extract_tzx(fullpath,NULL,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_tzx_to_rwa(fullpath,archivo_destino);
                        break;

                        case 2:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_any_to_wav(fullpath,archivo_destino);
                        break;

                }
        }
/*
extern int convert_smp_to_rwa_tmpdir(char *origen, char *destino);
extern int convert_wav_to_rwa_tmpdir(char *origen, char *destino);
extern int convert_o_to_rwa_tmpdir(char *origen, char *destino);
extern int convert_p_to_rwa_tmpdir(char *origen, char *destino);
*/

        else if (!util_compare_file_extension(archivo,"smp")) {
                char *opciones[]={
                        "SMP to RWA",
			            "SMP to WAV",
                        "SMP to TAP",
                        "SMP to P",
                        "SMP to O",                        
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_smp_to_rwa(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_any_to_wav(fullpath,archivo_destino);
                        break;

                        case 2:
                                sprintf(archivo_destino,"%s/%s.tap",directorio,archivo);
                                util_realtape_browser(fullpath, NULL,0,archivo_destino,NULL,0,NULL);
                        break;    

                        case 3:
                                sprintf(archivo_destino,"%s/%s.p",directorio,archivo);
                                convert_realtape_to_po(fullpath, archivo_destino,NULL,0);
                        break;

                        case 4:
                                sprintf(archivo_destino,"%s/%s.o",directorio,archivo);
                                convert_realtape_to_po(fullpath, archivo_destino,NULL,0);
                        break;                                              

                }
        }

        else if (!util_compare_file_extension(archivo,"wav")) {
                char *opciones[]={
                        "WAV to RWA",
                        "WAV to TAP",
                        "WAV to P",
                        "WAV to O",                        
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_wav_to_rwa(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.tap",directorio,archivo);
                                util_realtape_browser(fullpath, NULL,0,archivo_destino,NULL,0,NULL);
                        break;    

                        case 2:
                                sprintf(archivo_destino,"%s/%s.p",directorio,archivo);
                                convert_realtape_to_po(fullpath, archivo_destino,NULL,0);
                        break;

                        case 3:
                                sprintf(archivo_destino,"%s/%s.o",directorio,archivo);
                                convert_realtape_to_po(fullpath, archivo_destino,NULL,0);
                        break;                                                  

                }
        }

        else if (!util_compare_file_extension(archivo,"rwa")) {
                char *opciones[]={
                        "RWA to WAV",
                        "RWA to TAP",
                        "RWA to P",
                        "RWA to O",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_rwa_to_wav(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.tap",directorio,archivo);
                                util_realtape_browser(fullpath, NULL,0,archivo_destino,NULL,0,NULL);
                        break;       

                        case 2:
                                sprintf(archivo_destino,"%s/%s.p",directorio,archivo);
                                convert_realtape_to_po(fullpath, archivo_destino,NULL,0);
                        break;

                        case 3:
                                sprintf(archivo_destino,"%s/%s.o",directorio,archivo);
                                convert_realtape_to_po(fullpath, archivo_destino,NULL,0);
                        break;                        

                }
        }        

        else if (!util_compare_file_extension(archivo,"o")) {
                char *opciones[]={
                        "O to RWA",
			"O to WAV",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_o_to_rwa(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_any_to_wav(fullpath,archivo_destino);
                        break;

                }
        }

        else if (!util_compare_file_extension(archivo,"p")) {
                char *opciones[]={
                        "P to RWA",
			"P to WAV",
			"P to SCR",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_p_to_rwa(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_any_to_wav(fullpath,archivo_destino);
                        break;

                        case 2:
                                sprintf(archivo_destino,"%s/%s.scr",directorio,archivo);
								util_convert_p_to_scr(fullpath,archivo_destino);
                        break;


                }
        }

        else if (!util_compare_file_extension(archivo,"pzx")) {
                char *opciones[]={
					"PZX to TAP",
                        "PZX to RWA",
						"PZX to WAV",
                        NULL};

        int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.tap",directorio,archivo);
								util_extract_pzx(fullpath,NULL,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_pzx_to_rwa(fullpath,archivo_destino);
                        break;

                        case 2:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_any_to_wav(fullpath,archivo_destino);
                        break;
 
                }
        }		


		else if (!util_compare_file_extension(archivo,"scr")) {
                char *opciones[]={
					"SCR to TAP",
                    "SCR to TXT",
                        NULL};

        int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.tap",directorio,archivo);
								convert_scr_to_tap(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.txt",directorio,archivo);
								convert_scr_to_txt(fullpath,archivo_destino);
                        break;                        

 
                } 
        }		

		else if (!util_compare_file_extension(archivo,"sna")) {
                char *opciones[]={
					"SNA to SCR",
                        NULL};

        int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.scr",directorio,archivo);
								util_convert_sna_to_scr(fullpath,archivo_destino);
                        break;

 
                } 
        }			


		else if (!util_compare_file_extension(archivo,"sp")) {
                char *opciones[]={
					"SP to SCR",
                        NULL};

        int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.scr",directorio,archivo);
								util_convert_sp_to_scr(fullpath,archivo_destino);
                        break;

 
                } 
        }	

		else if (!util_compare_file_extension(archivo,"z80")) {
                char *opciones[]={
					"Z80 to SCR",
                        NULL};

        int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.scr",directorio,archivo);
								util_convert_z80_to_scr(fullpath,archivo_destino);
                        break;

 
                } 
        }		


		else if (!util_compare_file_extension(archivo,"zsf")) {
                char *opciones[]={
					"ZSF to SCR",
                        NULL};

        int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.scr",directorio,archivo);
								util_convert_zsf_to_scr(fullpath,archivo_destino);
                        break;

 
                } 
        }	

        else if (!util_compare_file_extension(archivo,"hdf")) {
                char *opciones[]={
                        "HDF to IDE",
			"HDF to MMC",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC 
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.ide",directorio,archivo);
                                convert_hdf_to_raw(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.mmc",directorio,archivo);
                                convert_hdf_to_raw(fullpath,archivo_destino);
                        break;

                }
        }		

	else {
		menu_error_message("No conversion valid for this file type");
		return;
	}

	//Si no hay error
	if (!if_pending_error_message) {
		//char buffer_mensaje_ok[PATH_MAX+1024];
		//sprintf (buffer_mensaje_ok,"File converted to %s",archivo_destino);

		//menu_generic_message_splash("File converter",buffer_mensaje_ok);
		//menu_warn_message(buffer_mensaje_ok);

		menu_generic_message_format("File converter","OK. File converted to %s",archivo_destino);
	}

}

//Cargar archivo en memory zone
void file_utils_file_mem_load(char *archivo)
{
    int tamanyo=get_file_size(archivo);
    //Asignar memoria si no estaba asignada

    int error_limite=0;

    //Max 16 mb  (0x1000000), para no usar mas de 6 digitos al mostrar la direccion
    if (tamanyo>0x1000000) {
        tamanyo=0x1000000;
        error_limite=1;
    }

    //liberar si habia algo
    if (memory_zone_by_file_size>0) {
        debug_printf(VERBOSE_DEBUG,"Freeing previous file memory zone");
        free(memory_zone_by_file_pointer);
    }

    debug_printf(VERBOSE_DEBUG,"Allocating %d bytes for file memory zone",tamanyo);
    memory_zone_by_file_pointer=malloc(tamanyo);
    if (memory_zone_by_file_pointer==NULL) {
        cpu_panic("Can not allocate memory for file read");
    }

    memory_zone_by_file_size=tamanyo;

    FILE *ptr_load;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(archivo,&in_fatfs,&ptr_load,&fil)<0) {
        debug_printf (VERBOSE_ERR,"Unable to open file %s",archivo);
        return;
    }

    /*
                ptr_load=fopen(archivo,"rb");

                if (!ptr_load) {
                        debug_printf (VERBOSE_ERR,"Unable to open file %s",archivo);
                        return;
                }
    */

/*
extern char memory_zone_by_file_name[];
extern z80_byte *memory_zone_by_file_pointer;
extern int memory_zone_by_file_size;
*/

    //Copiamos el nombre del archivo aunque de momento no lo uso
    strcpy(memory_zone_by_file_name,archivo);


    int leidos;

    leidos=zvfs_fread(in_fatfs,memory_zone_by_file_pointer,tamanyo,ptr_load,&fil);
    
    //leidos=fread(memory_zone_by_file_pointer,1,tamanyo,ptr_load);
    if (leidos!=tamanyo) {
            debug_printf (VERBOSE_ERR,"Error reading file. Bytes read: %d bytes",leidos);
    }

    zvfs_fclose(in_fatfs,ptr_load,&fil);
    //fclose(ptr_load);

    if (error_limite) menu_warn_message("File too big. Reading first 16 Mb");
    else menu_generic_message_splash("File memory zone","File loaded to File memory zone");
}

//Indica con un flag que esta copiando recursivamente
int menu_filesel_copying_recursive_flag=0;

void menu_filesel_copy_recursive_withflag(char *archivo,char *nombre_final,int simular)
{
    menu_filesel_copying_recursive_flag=1;
    menu_filesel_copy_recursive(archivo,nombre_final,simular);
    menu_filesel_copying_recursive_flag=0;
}


//Si hay soporte de threads, mostrar ventana de progreso
#ifdef USE_PTHREADS

int contador_menu_copying_recurse_progress_print=0;
pthread_t menu_copying_recurse_progress_thread;

char menu_copying_recurse_progress_source[PATH_MAX];
char menu_copying_recurse_progress_destination[PATH_MAX];
int menu_copying_recurse_progress_simular;

void *menu_copying_recurse_progress_thread_function(void *nada GCC_UNUSED)
{
    debug_printf(VERBOSE_DEBUG,"Starting sync thread");

    menu_filesel_copy_recursive_withflag(menu_copying_recurse_progress_source,menu_copying_recurse_progress_destination,menu_copying_recurse_progress_simular);

    debug_printf(VERBOSE_DEBUG,"Finishing sync thread");


    return 0;

}

int menu_copying_recurse_progress_cond(zxvision_window *w GCC_UNUSED)
{
        return !menu_filesel_copying_recursive_flag;
}


void menu_copying_recurse_progress_print(zxvision_window *w)
{
        char *mensaje="|/-\\";

        int max=strlen(mensaje);
        char mensaje_dest[32];

        int pos=contador_menu_copying_recurse_progress_print % max;

        sprintf(mensaje_dest,"Copying %c",mensaje[pos]);

        zxvision_print_string_defaults_fillspc(w,1,0,mensaje_dest);
        zxvision_draw_window_contents(w);

        contador_menu_copying_recurse_progress_print++;

}
void menu_filesel_copy_recursive_start(char *archivo,char *nombre_final,int simular)
{


    //TODO: enviar estos parametros como parametros del thread
    strcpy(menu_copying_recurse_progress_source,archivo);

    strcpy(menu_copying_recurse_progress_destination,nombre_final);

    menu_copying_recurse_progress_simular=simular;


                //Inicializar thread
        debug_printf (VERBOSE_DEBUG,"Initializing thread menu_copying_recurse_progress_thread");


        //Lanzar el thread de sync


        //Antes de lanzarlo, decir que se ejecuta, por si el usuario le da enter rapido a la ventana de progreso y el thread aun no se ha lanzado
        menu_filesel_copying_recursive_flag=1;

        if (pthread_create( &menu_copying_recurse_progress_thread, NULL, &menu_copying_recurse_progress_thread_function, NULL )) {
                debug_printf(VERBOSE_ERR,"Can not create menu_copying_recurse_progress_thread thread");
                return;
        }    


        contador_menu_copying_recurse_progress_print=0;
        zxvision_simple_progress_window("Copying", menu_copying_recurse_progress_cond,menu_copying_recurse_progress_print );

        if (menu_filesel_copying_recursive_flag) {
                        //Al parecer despues de ventana de zxvision_simple_progress_window no se espera a liberar tecla
                        menu_espera_no_tecla();
                        menu_warn_message("Copying has not ended yet");
        }

        else {
            menu_generic_message("Copy folder","OK. Folder copied");
        }


}

#else

void menu_filesel_copy_recursive_start(char *archivo,char *nombre_final,int simular)
{

    //Sync tal cual sin progreso
    //No se si hay alguien que compile sin soporte de threads, pero al menos, avisarle y mostrarle un ok cuando finalice
    menu_warn_message("Copying folder may take a while. Press Enter and wait please");
    menu_filesel_copy_recursive_withflag(archivo,nombre_final,simular);
    menu_generic_message("Copy folder","OK. Folder copied");

}

#endif



//parametro rename: 
//si 0, move
//si 1, es rename
//si 2, copy
void file_utils_move_rename_copy_file(char *archivo,int rename_move)
{
	char nombre_sin_dir[PATH_MAX];
	char directorio[PATH_MAX];
	char nombre_final[PATH_MAX];

	util_get_dir(archivo,directorio);
	util_get_file_no_directory(archivo,nombre_sin_dir);



	int ejecutar_accion=1;

	//Rename
	if (rename_move==1) {
		menu_ventana_scanf("New name",nombre_sin_dir,PATH_MAX);
		sprintf(nombre_final,"%s/%s",directorio,nombre_sin_dir);
	}

	//Copy or move
	else if (rename_move==2 || rename_move==0) {
		//Move or copy
		char *filtros[2];

       	 	filtros[0]="nofiles";
        	filtros[1]=0;


        	//guardamos directorio actual
        	char directorio_actual[PATH_MAX];
        	zvfs_getcwd(directorio_actual,PATH_MAX);

        	int ret;


        	char nada[PATH_MAX];


        	//Ocultar utilidades
        	menu_filesel_show_utils.v=0;
            //Decir que el menu de drives debe incluir 0:/, aunque file utils no este activo
            menu_filesel_drives_allow_fatfs.v=1;

        	ret=menu_filesel("Set target dir & press ESC",filtros,nada);
        	//Volver a mostrar utilidades
        	menu_filesel_show_utils.v=1;

            menu_filesel_drives_allow_fatfs.v=0;


        	//Si sale con ESC
        	if (ret==0) {

        		//Move
                if (rename_move==0) debug_printf (VERBOSE_DEBUG,"Move file %s to directory %s",archivo,menu_filesel_last_directory_seen);

                //Copy
                if (rename_move==2) debug_printf (VERBOSE_DEBUG,"Copy file %s to directory %s",archivo,menu_filesel_last_directory_seen);
                sprintf(nombre_final,"%s/%s",menu_filesel_last_directory_seen,nombre_sin_dir);

        	}
        	else {
        		//TODO: hacer de manera facil que menu_filesel no deje seleccionar archivos con enter y solo deje salir con ESC
        		menu_warn_message("You must select the directory exiting with ESC key. Aborting!");
        		ejecutar_accion=0;
        	}


        	//volvemos a directorio inicial
        	zvfs_chdir(directorio_actual);
	}

	if (ejecutar_accion) {
		debug_printf (VERBOSE_INFO,"Original name: %s dir: %s new name %s final name %s"
				,archivo,directorio,nombre_sin_dir,nombre_final);

        //releer con speech
        //parece que a veces no lee el titulo de la ventana y es importante
        //probablemente porque para elegir el directorio destino se pulsa ESC y por tanto
        //eso evita que se envie el siguiente texto a speech
        menu_speech_tecla_pulsada=0;   

		if (menu_confirm_yesno_texto("Confirm operation","Sure?")==0) return;

        //Ver si ruta origen y destino es la misma, en el caso de copy y move
        if (rename_move==0 || rename_move==2) {
            char dir_origen[PATH_MAX];
            char dir_destino[PATH_MAX];

            util_get_dir(archivo,dir_origen);
            util_get_dir(nombre_final,dir_destino);

            if (!strcmp(dir_origen,dir_destino)) {
                debug_printf(VERBOSE_ERR,"Source and target directory are the same (%s). Aborting!",dir_origen);
                return;
            }
        }

		//Si existe archivo destino
		if (si_existe_archivo(nombre_final)) {
			if (menu_confirm_yesno_texto("Item exists","Overwrite?")==0) return;
		}

        //Copy
		if (rename_move==2) {
            int tipo_archivo=get_file_type(archivo);
            if (tipo_archivo==2) {
                if (menu_confirm_yesno_texto("Source is folder","Copy recursive?")==0) return;

                //Copiar tal cual
                //menu_filesel_copy_recursive(archivo,nombre_final,0);

                //Copiar con ventana de progreso
                menu_filesel_copy_recursive_start(archivo,nombre_final,0);

                //menu_generic_message("Copy folder","OK. Folder copied");
            }
            else {
                util_copy_file(archivo,nombre_final);
                menu_generic_message("Copy file","OK. File copied");

            }
            
        }
		//Rename
		else if (rename_move==1) {
            zvfs_rename(archivo,nombre_final);
            menu_generic_message("Rename file","OK. File renamed");
        }
        //Move
		else {
            //No permitir move si el origen es carpeta y origen y destino son diferentes dispositivos
            int tipo_archivo=get_file_type(archivo);
            if (tipo_archivo==2) {
                //Ver dispositivo origen y destino
                int in_fatfs_origen=util_path_is_mmc_fatfs(archivo);
                int in_fatfs_destino=util_path_is_mmc_fatfs(nombre_final);

                if (in_fatfs_origen!=in_fatfs_destino) {
                    debug_printf(VERBOSE_ERR,"Moving folder between diferent source and target type is not allowed. Aborting!");
                    return;
                }

                zvfs_rename(archivo,nombre_final);
                menu_generic_message("Move folder","OK. Folder moved");

            }

            else {          
                //En el caso de archivos, para que podamos hacer move entre local y mmc, hacemos como un copy pero luego borramos origen
                util_copy_file(archivo,nombre_final);
                zvfs_delete(archivo);
                menu_generic_message("Move file","OK. File moved");
            }
            

            
        }


		
	}
}


void file_utils_delete(char *nombre)
{


    int tipo_archivo=get_file_type(nombre);
    if (tipo_archivo==2) {
        //Es directorio

        //No permitir borrar . o ..
        char file_no_directory[PATH_MAX];
        util_get_file_no_directory(nombre,file_no_directory);
        if (!strcasecmp(file_no_directory,".") ||
            !strcasecmp(file_no_directory,"..")
         ) {
                debug_printf(VERBOSE_ERR,"Deleting directory . or .. is not allowed");
                return;                
            }

        if (menu_filesel_utils_allow_folder_delete.v==0) {
            debug_printf(VERBOSE_ERR,"Allow delete folders setting is not enabled. Enable it AT YOUR OWN RISK on Settings-> File Browser");
            return;
        }

        if (menu_confirm_yesno_texto("WARNING! Source is folder","Remove folder entirely?")==0) return;


        menu_filesel_delete_recursive(nombre,0);

        menu_generic_message("Delete","OK. Folder has been deleted");

    }
    else {


        int resultado=zvfs_delete(nombre);
        if (resultado) {
            menu_error_message("Error deleting item");
        }
        else {
            menu_generic_message("Delete","OK. File deleted");
        }


    }
}


void file_utils_paste_clipboard(void)
{

	if (menu_clipboard_pointer==NULL) {
		debug_printf(VERBOSE_ERR,"Clipboard is empty, you can fill it from a text window and press key c");
		return;
	}

	char directorio_actual[PATH_MAX];
    zvfs_getcwd(directorio_actual,PATH_MAX);

	char nombre_sin_dir[PATH_MAX];
	char nombre_final[PATH_MAX];


	nombre_sin_dir[0]=0;
	menu_ventana_scanf("Filename?",nombre_sin_dir,PATH_MAX);
	sprintf(nombre_final,"%s/%s",directorio_actual,nombre_sin_dir);


	//Ver si archivo existe y preguntar
	if (si_existe_archivo(nombre_final)) {

		if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

    }


	menu_paste_clipboard_to_file(nombre_final);

	menu_generic_message_splash("Clipboard","File saved with ZEsarUX clipboard contents");


}



void zxvision_menu_filesel_cursor_arriba(zxvision_window *ventana)
{
	//ver que no sea primer archivo
    if (filesel_archivo_seleccionado+filesel_linea_seleccionada!=0) {
	 zxvision_dec_cursor_line(ventana);
                                                //ver si es principio de pantalla
                                                if (filesel_linea_seleccionada==0) {
							zxvision_send_scroll_up(ventana);
                                                        filesel_archivo_seleccionado--;
                                                }
                                                else {
                                                        filesel_linea_seleccionada--;
                                                }
                                        }

	//Por si el cursor no esta visible en pantalla (al haberse hecho scroll con raton)	
	if (zxvision_adjust_cursor_top(ventana)) {
		zxvision_send_scroll_up(ventana);
		filesel_linea_seleccionada=0;
		filesel_archivo_seleccionado=ventana->cursor_line;
	}
}

int zxvision_get_filesel_alto_dir(zxvision_window *ventana)
{
	return ventana->visible_height - ventana->upper_margin - ventana->lower_margin - 2;
}

int zxvision_get_filesel_pos_filters(zxvision_window *ventana)
{
	return ventana->visible_height - 3;
}


void zxvision_menu_filesel_cursor_abajo(zxvision_window *ventana)
{
	//ver que no sea ultimo archivo
	if (si_menu_filesel_no_mas_alla_ultimo_item(filesel_linea_seleccionada)) {
		zxvision_inc_cursor_line(ventana);
                                                //ver si es final de pantalla
                                                if (filesel_linea_seleccionada==zxvision_get_filesel_alto_dir(ventana)-1) {
                                                        filesel_archivo_seleccionado++;
							zxvision_send_scroll_down(ventana);
                                                }
                                                else {
                                                        filesel_linea_seleccionada++;
                                                }
                                        }
	//Por si el cursor no esta visible en pantalla (al haberse hecho scroll con raton)									
	if (zxvision_adjust_cursor_bottom(ventana)) {
		zxvision_send_scroll_down(ventana);
		filesel_linea_seleccionada=zxvision_get_filesel_alto_dir(ventana)-1;
		filesel_archivo_seleccionado=ventana->cursor_line-filesel_linea_seleccionada;
	}

}


//liberar memoria usada por la lista de archivos
void menu_filesel_free_mem(void)
{

	filesel_item *aux;
        filesel_item *nextfree;


        aux=primer_filesel_item;

        //do {

	//puede que no haya ningun archivo, por tanto esto es NULL
	//sucede con las carpetas /home en macos por ejemplo
	while (aux!=NULL) {

                //printf ("Liberando %p : %s\n",aux,aux->d_name);
                nextfree=aux->next;
                free(aux);

                aux=nextfree;
        };
        //} while (aux!=NULL);

	//printf ("fin liberar filesel\n");


}


int menu_filesel_mkdir(char *directory)
{
#ifndef MINGW
     int tmpdirret=mkdir(directory,S_IRWXU);
#else
	int tmpdirret=mkdir(directory);
#endif

     if (tmpdirret!=0 && errno!=EEXIST) {
                  debug_printf (VERBOSE_ERR,"Error creating %s directory : %s",directory,strerror(errno) );
     }

	return tmpdirret;

}

void menu_filesel_exist_ESC(void)
{
                                                
                                                menu_espera_no_tecla();
                                                zvfs_chdir(filesel_directorio_inicial);
                                                menu_filesel_free_mem();
}

//Elimina la extension de un nombre
void menu_filesel_file_no_ext(char *origen, char *destino)
{



	int j;

        j=strlen(origen);

	//buscamos desde el punto final

        for (;j>=0 && origen[j]!='.';j--);

	if (j==-1) {
		//no hay punto
		j=strlen(origen);
	}

	//printf ("posicion final: %d\n",j);


	//y copiamos
	for (;j>0;j--) {
		*destino=*origen;
		origen++;
		destino++;
	}

	//Y final de cadena
	*destino = 0;

	//printf ("archivo sin extension: %s\n",copiadestino);

}


#define COMPRESSED_ZIP 1
#define COMPRESSED_GZ  2
#define COMPRESSED_TAR 3
#define COMPRESSED_RAR 4

int menu_filesel_is_compressed(char *archivo)
{
  int compressed_type=0;

	//if ( strstr(archivo,".zip")!=NULL || strstr(archivo,".ZIP")!=NULL) {
	if ( !util_compare_file_extension(archivo,"zip") ) {
		debug_printf (VERBOSE_DEBUG,"Is a zip file");
		compressed_type=COMPRESSED_ZIP;
	}

        else if ( !util_compare_file_extension(archivo,"gz") ) {
                debug_printf (VERBOSE_DEBUG,"Is a gz file");
                compressed_type=COMPRESSED_GZ;
        }

        else if ( !util_compare_file_extension(archivo,"tar") ) {
                debug_printf (VERBOSE_DEBUG,"Is a tar file");
                compressed_type=COMPRESSED_TAR;
        }

        else if ( !util_compare_file_extension(archivo,"rar") ) {
                debug_printf (VERBOSE_DEBUG,"Is a rar file");
                compressed_type=COMPRESSED_RAR;
        }

	return compressed_type;	
}

//Devuelve 0 si ok

int menu_filesel_uncompress (char *archivo,char *tmpdir)
{


//descomprimir creando carpeta TMPDIR_BASE/zipname
 sprintf (tmpdir,"%s/%s",get_tmpdir_base(),archivo);
 menu_filesel_mkdir(tmpdir);


  int compressed_type=menu_filesel_is_compressed(archivo);


//descomprimir
 char uncompress_command[PATH_MAX];

 char uncompress_program[PATH_MAX];

		char archivo_no_ext[PATH_MAX];

switch (compressed_type) {

	case COMPRESSED_ZIP:
		//sprintf (uncompress_program,"/usr/bin/unzip");
 		//-n no sobreescribir
		//sprintf (uncompress_command,"unzip -n \"%s\" -d %s",archivo,tmpdir);

		/*sprintf (uncompress_program,"%s",external_tool_unzip);
 		//-n no sobreescribir
		sprintf (uncompress_command,"%s -n \"%s\" -d %s",external_tool_unzip,archivo,tmpdir);*/


		//printf ("Using internal zip decompressor\n");
		util_extract_zip(archivo,tmpdir);
		return 0;
	break;

        case COMPRESSED_GZ:
		menu_filesel_file_no_ext(archivo,archivo_no_ext);
                //sprintf (uncompress_program,"/bin/gunzip");
                //sprintf (uncompress_command,"gunzip -c \"%s\" > \"%s/%s\" ",archivo,tmpdir,archivo_no_ext);
                sprintf (uncompress_program,"%s",external_tool_gunzip);
                sprintf (uncompress_command,"%s -c \"%s\" > \"%s/%s\" ",external_tool_gunzip,archivo,tmpdir,archivo_no_ext);
        break;

        case COMPRESSED_TAR:
                //sprintf (uncompress_program,"/bin/tar");
                //sprintf (uncompress_command,"tar -xvf \"%s\" -C %s",archivo,tmpdir);
                sprintf (uncompress_program,"%s",external_tool_tar);
                sprintf (uncompress_command,"%s -xvf \"%s\" -C %s",external_tool_tar,archivo,tmpdir);
        break;

        case COMPRESSED_RAR:
                //sprintf (uncompress_program,"/usr/bin/unrar");
                //sprintf (uncompress_command,"unrar x -o+ \"%s\" %s",archivo,tmpdir);
                sprintf (uncompress_program,"%s",external_tool_unrar);
                sprintf (uncompress_command,"%s x -o+ \"%s\" %s",external_tool_unrar,archivo,tmpdir);
        break;




	default:
		debug_printf(VERBOSE_ERR,"Unknown compressed file");
		return 1;
	break;

}

 //unzip
 struct stat buf_stat;


 	if (stat(uncompress_program, &buf_stat)!=0) {
		debug_printf(VERBOSE_ERR,"Unable to find uncompress program: %s",uncompress_program);
		return 1;

	}

	debug_printf (VERBOSE_DEBUG,"Running %s",uncompress_command);

	if (system (uncompress_command)==-1) {
		debug_printf (VERBOSE_DEBUG,"Error running command %s",uncompress_command);
		return 1;
 	}


	return 0;

}

int menu_filesel_file_can_be_expanded(char *archivo)
{
    char *extensiones_validas[]={
        "hdf","tap","tzx","cdt","pzx",
        "trd","dsk","epr","eprom",
        "flash","p","o","mdv","scl","ddh",
        NULL
    };

    int i;

    for (i=0;extensiones_validas[i]!=NULL;i++) {
        if (!util_compare_file_extension(archivo,extensiones_validas[i]) ) {
            return 1;
        }
    }

    //Ver si comprimido
    if (menu_filesel_is_compressed(archivo)) return 1;

    return 0;
}

//Expandir archivos (no descomprimir, sino expandir por ejemplo un tap o un hdf)
//Devuelve 0 si ok
int menu_filesel_expand(char *archivo,char *tmpdir)
{

    if (!menu_filesel_file_can_be_expanded(archivo)) {
        //printf("File can not be expanded\n");
        return 1;
    }

	sprintf (tmpdir,"%s/%s",get_tmpdir_base(),archivo);
	menu_filesel_mkdir(tmpdir);

        //TODO: hdf no se puede expandir si el archivo esta en una imagen mmc
        if (!util_compare_file_extension(archivo,"hdf") ) {
                debug_printf (VERBOSE_DEBUG,"Is a hdf file");
                return util_extract_hdf(archivo,tmpdir);
        }

        else if (!util_compare_file_extension(archivo,"tap") ) {
                debug_printf (VERBOSE_DEBUG,"Is a tap file");
        	return util_extract_tap(archivo,tmpdir,NULL,0);
        }

        else if (!util_compare_file_extension(archivo,"ddh") ) {
                debug_printf (VERBOSE_DEBUG,"Is a ddh file");
        	return util_extract_ddh(archivo,tmpdir);
        }        

        else if (
            !util_compare_file_extension(archivo,"tzx") || 
            !util_compare_file_extension(archivo,"cdt")
            ) {
                debug_printf (VERBOSE_DEBUG,"Is a tzx file");
                return util_extract_tzx(archivo,tmpdir,NULL);
        }


        else if (!util_compare_file_extension(archivo,"pzx") ) {
                debug_printf (VERBOSE_DEBUG,"Is a pzx file");
                return util_extract_pzx(archivo,tmpdir,NULL);
        }		

        else if (!util_compare_file_extension(archivo,"trd") ) {
                debug_printf (VERBOSE_DEBUG,"Is a trd file");
                return util_extract_trd(archivo,tmpdir);
        }		

        else if (!util_compare_file_extension(archivo,"dsk") ) {
                debug_printf (VERBOSE_DEBUG,"Is a dsk file");
                return util_extract_dsk(archivo,tmpdir);
        }		

        //TODO: epr, eprom, flash no se puede expandir si el archivo esta en una imagen mmc
        else if (
			!util_compare_file_extension(archivo,"epr")  ||
			!util_compare_file_extension(archivo,"eprom")  ||
			!util_compare_file_extension(archivo,"flash")  
		) {
                debug_printf (VERBOSE_DEBUG,"Is a Z88 card file");
                return util_extract_z88_card(archivo,tmpdir);
        }				

        else if (!util_compare_file_extension(archivo,"p") ) {
                debug_printf (VERBOSE_DEBUG,"Is a P file");
        	return util_extract_p(archivo,tmpdir);
        }	

        else if (!util_compare_file_extension(archivo,"o") ) {
                debug_printf (VERBOSE_DEBUG,"Is a O file");
        	return util_extract_o(archivo,tmpdir);
        }				

        //TODO: mdv no se puede expandir si el archivo esta en una imagen mmc
        else if ( !util_compare_file_extension(archivo,"mdv") ) {
                debug_printf (VERBOSE_DEBUG,"Is a mdv file");
                return util_extract_mdv(archivo,tmpdir);
        }

        else if ( !util_compare_file_extension(archivo,"scl") ) {
                debug_printf (VERBOSE_DEBUG,"Is a scl file");
                return util_extract_scl(archivo,tmpdir);
        }		

		else if (menu_filesel_is_compressed(archivo)) {
			debug_printf (VERBOSE_DEBUG,"Expanding Compressed file");
			return menu_filesel_uncompress(archivo,tmpdir);
		}

		//else debug_printf(VERBOSE_DEBUG,"Don't know how to expand that file");


        return 1;


}


//escribir archivo que indique directorio anterior
void menu_filesel_write_file_last_dir(char *directorio_anterior)
{
	debug_printf (VERBOSE_DEBUG,"Writing temp file " MENU_LAST_DIR_FILE_NAME " to tell last directory before uncompress (%s)",directorio_anterior);


    FILE *ptr_lastdir;
    ptr_lastdir=fopen(MENU_LAST_DIR_FILE_NAME,"wb");

	if (ptr_lastdir!=NULL) {
	        fwrite(directorio_anterior,1,strlen(directorio_anterior),ptr_lastdir);
        	fclose(ptr_lastdir);
	}
}

//leer contenido de archivo que indique directorio anterior
//Devuelve 0 si OK. 1 si error
int menu_filesel_read_file_last_dir(char *directorio_anterior)
{

        FILE *ptr_lastdir;
        ptr_lastdir=fopen(MENU_LAST_DIR_FILE_NAME,"rb");

	if (ptr_lastdir==NULL) {
		debug_printf (VERBOSE_DEBUG,"Error opening " MENU_LAST_DIR_FILE_NAME);
                return 1;
        }


        int leidos=fread(directorio_anterior,1,PATH_MAX,ptr_lastdir);
        fclose(ptr_lastdir);

	if (leidos) {
		directorio_anterior[leidos]=0;
	}
	else {
		if (leidos==0) debug_printf (VERBOSE_DEBUG,"Error. Read 0 bytes from " MENU_LAST_DIR_FILE_NAME);
		if (leidos<0) debug_printf (VERBOSE_DEBUG,"Error reading from " MENU_LAST_DIR_FILE_NAME);
		return 1;
	}

	return 0;
}


void menu_textspeech_say_current_directory(void)
{

	//printf ("\nmenu_textspeech_say_current_directory\n");

        char current_dir[PATH_MAX];
        char buffer_texto[PATH_MAX+200];
        getcwd(current_dir,PATH_MAX);

        sprintf (buffer_texto,"Current directory: %s",current_dir);

	//Quiero que siempre se escuche
	menu_speech_tecla_pulsada=0;
	menu_textspeech_send_text(buffer_texto);
}




int zxvision_si_mouse_zona_archivos(zxvision_window *ventana)
{
    int inicio_y_dir=1+FILESEL_INICIO_DIR;

    if (menu_mouse_y>=inicio_y_dir && menu_mouse_y<inicio_y_dir+zxvision_get_filesel_alto_dir(ventana) && menu_mouse_x<ventana->visible_width-1) {
		//printf ("Mouse en zona de archivos\n");
		return 1;
	}
    
	return 0;
}


void zxvision_menu_filesel_print_text_contents(zxvision_window *ventana)
{
    char buffer_linea[100];

        if (menu_mmc_image_montada && menu_current_drive_mmc_image.v) {
            int total,free;
            filesel_return_free_mmc_mounted(&total,&free);
            sprintf(buffer_linea,"Directory Contents: (Image total %d MB Free %d MB)",total,free);
        }
        else {
            strcpy(buffer_linea,"Directory Contents:");
        }



	zxvision_print_string_defaults_fillspc(ventana,1,2,buffer_linea);
}

void file_utils_info_file(char *archivo)
{

    char buffer_tamanyo[100]="";

    //Si es carpeta no mostrar tamaño
    int tipo_archivo=get_file_type(archivo);
    if (tipo_archivo!=2) {    

        long int tamanyo=get_file_size(archivo);
        sprintf(buffer_tamanyo,"Size: %ld bytes\n",tamanyo);
    
    }
        //fecha
       int hora;
        int minutos;
        int segundos;

        int anyo;
        int mes;
        int dia;


        get_file_date_from_name(archivo,&hora,&minutos,&segundos,&dia,&mes,&anyo);




	menu_generic_message_format("Info file","Full path: %s\n\n%sModified: %02d:%02d:%02d %02d/%02d/%02d",
		archivo,buffer_tamanyo,hora,minutos,segundos,dia,mes,anyo);

}


//Si hay que iniciar el filesel pero mover el cursor a un archivo concreto
z80_bit menu_filesel_posicionar_archivo={0};
char menu_filesel_posicionar_archivo_nombre[PATH_MAX]="";

void menu_filesel_change_to_tmp(char *tmpdir)
{
                                                                        //cambiar a tmp dir.

                                                                        //Dejar antes un archivo temporal en ese directorio que indique directorio anterior
                                                                        char directorio_actual[PATH_MAX];
                                                                        zvfs_getcwd(directorio_actual,PATH_MAX);

                                                                        zvfs_chdir(tmpdir);

                                                                        //escribir archivo que indique directorio anterior
                                                                        menu_filesel_write_file_last_dir(directorio_actual);

                                                                        menu_filesel_free_mem();
}


void zxvision_menu_print_dir(int inicial,zxvision_window *ventana)
{

	//TODO: no tiene sentido usar variable "inicial"
	inicial=0;

	//printf ("\nmenu_print_dir\n");
    //Si directorio vacio por ejemplo, no se escribiria nunca este texto de File:. Al menos escribirlo vacio al principio
    zxvision_print_string_defaults_fillspc(ventana,1,1,"File: ");

	//escribir en ventana directorio de archivos

	//Para speech
	char texto_opcion_activa[PATH_MAX+100]; //Dado que hay que meter aqui el nombre del archivo y un poquito mas de texto
	//Asumimos por si acaso que no hay ninguna activa
	texto_opcion_activa[0]=0;



	filesel_item *p;
	int i;

	int mostrados_en_pantalla=(ventana->visible_height)-10;
	//trucar el maximo en pantalla. dado que somos zxvision, se pueden mostar ya todos en resolucion virtual de ventana
	mostrados_en_pantalla=999999;

	p=menu_get_filesel_item(inicial);

	//Para calcular total de archivos de ese directorio, siguiendo el filtro. Util para mostrar indicador de porcentaje '*'
	//int total_archivos=inicial;

	filesel_total_archivos=inicial;

	for (i=0;p!=NULL;i++,filesel_total_archivos++) {
		//printf ("file: %s\n",p->d_name);

		//Solo hacer esto si es visible en pantalla
		if (i<mostrados_en_pantalla) {
		
            zxvision_menu_filesel_print_file(ventana,p->d_name,(ventana->total_width)-2,i);
            

            //if (filesel_linea_seleccionada==i) {
            if (ventana->cursor_line==i) {
                char buffer[OVERLAY_SCREEN_MAX_WIDTH+1],buffer2[OVERLAY_SCREEN_MAX_WIDTH+1+32];
                

                strcpy(filesel_nombre_archivo_seleccionado,p->d_name);

                //menu_tape_settings_trunc_name(filesel_nombre_archivo_seleccionado,buffer,22);
                //printf ("antes de trunc\n");

                int tamanyo_mostrar=ventana->visible_width-6-1; //6 ocupa el texto "File: "

                menu_tape_settings_trunc_name(filesel_nombre_archivo_seleccionado,buffer,tamanyo_mostrar); 

                sprintf (buffer2,"File: %s",buffer);
                
                zxvision_print_string_defaults_fillspc(ventana,1,1,buffer2);


                debug_printf (VERBOSE_DEBUG,"Selected: %s. filesel_zona_pantalla: %d",p->d_name,filesel_zona_pantalla);
                //Para speech
                //Si estamos en zona central del selector de archivos, decirlo
                if (filesel_zona_pantalla==1) {

                    if (menu_active_item_primera_vez) {
        

                        sprintf (texto_opcion_activa,"Selected item: %s %s",p->d_name,(get_file_type(p->d_name) == 2 ? "directory" : ""));
                        menu_active_item_primera_vez=0;
                    }

                    else {
                        sprintf (texto_opcion_activa,"%s %s",p->d_name,(get_file_type(p->d_name) == 2 ? "directory" : ""));
                    }

                }


            }
		}

		p=p->next;

    }



	//int texto_no_cabe=0;
	filesel_no_cabe_todo=0;

	debug_printf (VERBOSE_DEBUG,"Total files read (applying filters): %d",filesel_total_archivos);
	if (filesel_total_archivos>mostrados_en_pantalla) {
		filesel_no_cabe_todo=1;
	}




	//Imprimir directorio actual
	//primero borrar con espacios

    //menu_escribe_texto_ventana(14,0,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"               ");


	char current_dir[PATH_MAX];
	char buffer_dir[OVERLAY_SCREEN_MAX_WIDTH+1];
	char buffer3[OVERLAY_SCREEN_MAX_WIDTH+1+32];

	//getcwd(current_dir,PATH_MAX);
    zvfs_getcwd(current_dir,PATH_MAX);

	menu_tape_settings_trunc_name(current_dir,buffer_dir,ventana->visible_width-14); //14 es lo que ocupa el texto "Current dir: "
	sprintf (buffer3,"Current dir: %s",buffer_dir);
	//menu_escribe_texto_ventana(1,0,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer3);
	zxvision_print_string_defaults_fillspc(ventana,1,0,buffer3);


    if (texto_opcion_activa[0]!=0) {

        debug_printf (VERBOSE_DEBUG,"Send active line to speech: %s",texto_opcion_activa);
        //Selected item siempre quiero que se escuche

        //Guardamos estado actual
        int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
        menu_speech_tecla_pulsada=0;

        menu_textspeech_send_text(texto_opcion_activa);

        //Restauro estado
        //Pero si se ha pulsado tecla, no restaurar estado
        //Esto sino provocaria que , por ejemplo, en la ventana de confirmar yes/no,
        //se entra con menu_speech_tecla_pulsada=0, se pulsa tecla mientras se esta leyendo el item activo,
        //y luego al salir de aqui, se pierde el valor que se habia metido (1) y se vuelve a poner el 0 del principio
        //provocando que cada vez que se mueve el cursor, se relea la ventana entera
        if (menu_speech_tecla_pulsada==0) menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;
    }


}


              //Si en linea de "File"
int menu_filesel_change_zone_if_clicked(zxvision_window *ventana,int *filesel_zona_pantalla,int *tecla)
{
     if (!si_menu_mouse_en_ventana() ) return 0;
	if (!mouse_left) return 0;

	int futura_zona=-1;
    if (menu_mouse_y==2 && menu_mouse_x<ventana->visible_width-1) {
                //printf ("Pulsado zona File\n");
						futura_zona=0;
    }

                //Si en linea de filtros
    if (menu_mouse_y==zxvision_get_filesel_pos_filters(ventana)  && menu_mouse_x<ventana->visible_width-1) {
								//printf ("Pulsado zona Filtros\n");
                                                                futura_zona=2;
    }


		//En zona seleccion archivos
    if (zxvision_si_mouse_zona_archivos(ventana)) {	
							//printf ("En zona seleccion archivos\n");
							futura_zona=1;
		}


	if (futura_zona!=-1) {
		if (futura_zona!=*filesel_zona_pantalla) {
			//Cambio de zona
			menu_reset_counters_tecla_repeticion();
			*filesel_zona_pantalla=futura_zona;
			*tecla=0;
			return 1;
		}
	}




	return 0;
}



void menu_filesel_cambiar_unidad_common(char *destino)
{

    //de momento
    destino[0]=0;



	char buffer_unidades[100]; //Aunque son 26 maximo, pero por si acaso
#ifdef MINGW    
	int unidades=util_get_available_drives(buffer_unidades);
	if (unidades==0) {
		menu_error_message("No available drives");
		return;
	}    
#else
    //En sistemas no windows, no hay unidades.
    //Hago el menor código dependiente de MINGW, por ejemplo el bucle que hay abajo con las unidades
    //para poder detectar mas fácilmente errores
    int unidades=0;
#endif    



	//printf ("total unidades: %d string Unidades: %s 0 %d 1 %d 2 %d 3 %d\n",unidades,buffer_unidades,buffer_unidades[0],buffer_unidades[1],buffer_unidades[2],buffer_unidades[3]);


        menu_item *array_menu_filesel_unidad;
        menu_item item_seleccionado;
        int retorno_menu;

	int menu_filesel_unidad_opcion_seleccionada=0;


	menu_add_item_menu_inicial(&array_menu_filesel_unidad,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

	int i;

	for (i=0;i<unidades;i++) {
		char letra=buffer_unidades[i];
        /*
        Nota: pongo X:/, donde X es la unidad, con la / final
        La / es importante para que cuando estemos en unidad mmc, si cambiamos a c:/, la rutina util_path_is_mmc_fatfs (llamada desde zvfs_chdir)
        detecta que la ruta es absoluta. Si no, detectaria ruta relativa y no cambiaria de unidad
        */
		menu_add_item_menu_format(array_menu_filesel_unidad,MENU_OPCION_NORMAL,NULL,NULL,"%c:/",letra);
		//menu_add_item_menu_shortcut(array_menu_filesel_unidad,letra_minuscula(letra));
		//menu_add_item_menu_valor_opcion(array_menu_filesel_unidad,letra);
	}

#ifdef MINGW
//nada
#else

//Agregar ruta /media (en linux) o a /Volumes en Mac

	#if defined(__APPLE__)

//En Mac
        menu_add_item_menu_format(array_menu_filesel_unidad,MENU_OPCION_NORMAL,NULL,NULL,"/Volumes");

	#else

//En Linux

		menu_add_item_menu_format(array_menu_filesel_unidad,MENU_OPCION_NORMAL,NULL,NULL,"/media");

	#endif

#endif 

    //Si hay imagen montada y (esta file utils o bien permitimos mostrar en drives), permitir seleccionarla
    //No queremos que en ventanas que no sean file utils, se pueda seleccionar 0:/
    if (menu_mmc_image_montada && 
    (menu_filesel_show_utils.v || menu_filesel_drives_allow_fatfs.v)
    ) 
    {
        menu_add_item_menu_format(array_menu_filesel_unidad,MENU_OPCION_NORMAL,NULL,NULL,"0:/");
        menu_add_item_menu_tooltip(array_menu_filesel_unidad,"This is the first mmc mounted image");
        menu_add_item_menu_ayuda(array_menu_filesel_unidad,"This is the first mmc mounted image");

        //Y si esa imagen es el disco actual, permitir elegir el disco local
        if (menu_current_drive_mmc_image.v) {
            menu_add_item_menu_format(array_menu_filesel_unidad,MENU_OPCION_NORMAL,NULL,NULL,"Local Drive");
        }
    }


                menu_add_item_menu(array_menu_filesel_unidad,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_filesel_unidad);
                retorno_menu=menu_dibuja_menu(&menu_filesel_unidad_opcion_seleccionada,&item_seleccionado,array_menu_filesel_unidad,"Select Drive" );

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    strcpy(destino,item_seleccionado.texto_opcion);

    				return;
                }



	return;
}

//Retorna 1 si ha realizado cambio cursor. 0 si no
int menu_filesel_set_cursor_at_mouse(zxvision_window *ventana)
{
								int inicio_y_dir=1+FILESEL_INICIO_DIR;
                            //if (menu_mouse_y>=inicio_y_dir && menu_mouse_y<inicio_y_dir+(FILESEL_ALTO-10)) {
                            //printf ("Dentro lista archivos\n");

                            //Ver si linea dentro de rango
                            int linea_final=menu_mouse_y-inicio_y_dir;

                            //Si esta en la zona derecha de selector de porcentaje no hacer nada
                            
                            
							if (menu_mouse_x==(ventana->visible_width)-1) return 0;
							

                            //filesel_linea_seleccionada=menu_mouse_y-inicio_y_dir;

                            if (si_menu_filesel_no_mas_alla_ultimo_item(linea_final-1)) {

								//Ajustar cursor ventana

								
								filesel_archivo_seleccionado=ventana->offset_y;
								zxvision_set_cursor_line(ventana,filesel_archivo_seleccionado);
								
					
	
								//printf ("Seleccionamos item %d\n",linea_final);
                                filesel_linea_seleccionada=linea_final;

                                int linea_cursor_final=filesel_archivo_seleccionado;
                                linea_cursor_final += filesel_linea_seleccionada;
                                zxvision_set_cursor_line(ventana,linea_cursor_final);

                                menu_speech_tecla_pulsada=1;
								return 1;
                            }
                            else {
                                //printf ("Cursor mas alla del ultimo item\n");
                            }

	return 0;

}

void menu_filesel_recent_files_clear(MENU_ITEM_PARAMETERS)
{
	last_filesused_clear();
	menu_generic_message_splash("Clear List","OK. List cleared");
}

char *menu_filesel_recent_files(void)
{
	menu_item *array_menu_recent_files;
    menu_item item_seleccionado;
    int retorno_menu;


	menu_add_item_menu_inicial(&array_menu_recent_files,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

    #define MAX_RECENT_FILE_CHAR_LENGHT MAX_TEXTO_OPCION
    //char string_last_file_shown[MAX_RECENT_FILE_CHAR_LENGHT];


    int i;
	int hay_alguno=0;
    for (i=0;i<MAX_LAST_FILESUSED;i++) {
		if (last_files_used_array[i][0]!=0) {

			//Mostrar solo nombre de archivo sin path
			char archivo_sin_dir[PATH_MAX];
			util_get_file_no_directory(last_files_used_array[i],archivo_sin_dir);

            //menu_tape_settings_trunc_name(archivo_sin_dir,string_last_file_shown,MAX_RECENT_FILE_CHAR_LENGHT);
            //menu_add_item_menu_format(array_menu_recent_files,MENU_OPCION_NORMAL,NULL,NULL,string_last_file_shown);

            //En vez de cortar como habitualmente por la izquierda con menu_tape_settings_trunc_name, cortamos por la derecha
            util_trunc_name_right(archivo_sin_dir,MAX_RECENT_FILE_CHAR_LENGHT-1,PATH_MAX);

            //no agregar con funcion menu_add_item_menu_format o habra problemas si el nombre contiene % (que son especiales para printf)
            menu_add_item_menu(array_menu_recent_files,archivo_sin_dir,MENU_OPCION_NORMAL,NULL,NULL);

			//Agregar tooltip con ruta entera
			menu_add_item_menu_tooltip(array_menu_recent_files,last_files_used_array[i]);

			hay_alguno=1;
		}
	}

	if (!hay_alguno) menu_add_item_menu_format(array_menu_recent_files,MENU_OPCION_NORMAL,NULL,NULL,"<Empty List>");

	menu_add_item_menu(array_menu_recent_files,"",MENU_OPCION_SEPARADOR,NULL,NULL);
	menu_add_item_menu_format(array_menu_recent_files,MENU_OPCION_NORMAL,menu_filesel_recent_files_clear,NULL,"Clear List");

    menu_add_item_menu(array_menu_recent_files,"",MENU_OPCION_SEPARADOR,NULL,NULL);
    menu_add_ESC_item(array_menu_recent_files);

    retorno_menu=menu_dibuja_menu(&menu_recent_files_opcion_seleccionada,&item_seleccionado,array_menu_recent_files,"Recent files" );

    if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

		//Seleccion de borrar lista
		if (item_seleccionado.menu_funcion!=NULL) {
            item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
        }

		//Seleccion de un archivo
		else if (hay_alguno) {
        	int indice=menu_recent_files_opcion_seleccionada;

//lastfilesuser_scrolldown
//Quitar el que hay ahi, desplazando hacia abajo y ponerlo arriba del todo
//Copiarlo temporamente a otro sitio
		char buffer_recent[PATH_MAX];
		strcpy(buffer_recent,last_files_used_array[indice]);

		//Movemos el trozo desde ahi hasta arriba
		lastfilesuser_scrolldown(0,indice);

		//Y lo metemos arriba del todo
		strcpy(last_files_used_array[0],buffer_recent);

		//Por tanto el indice final sera 0
		indice=0;

		//Y cursor ponerloa arriba entonces tambien
		menu_recent_files_opcion_seleccionada=0;

			debug_printf (VERBOSE_INFO,"Returning recent file %s",last_files_used_array[indice]);
			return last_files_used_array[indice];
		}
	}

	return NULL;

}



struct s_first_aid_list first_aid_list[MAX_MENU_FIRST_AID];


//Si hay soporte de threads, mostrar ventana de progreso
#ifdef USE_PTHREADS

int contador_menu_syncing_mmc_progress_print=0;
pthread_t menu_syncing_mmc_progress_thread;

void *menu_syncing_mmc_progress_thread_function(void *nada GCC_UNUSED)
{
    debug_printf(VERBOSE_DEBUG,"Starting sync thread");

    diskio_sync();

    debug_printf(VERBOSE_DEBUG,"Finishing sync thread");


    return 0;

}

int menu_syncing_mmc_progress_cond(zxvision_window *w GCC_UNUSED)
{
        return !diskio_syncing_flag;
}


void menu_syncing_mmc_progress_print(zxvision_window *w)
{
        char *mensaje="|/-\\";

        int max=strlen(mensaje);
        char mensaje_dest[32];

        int pos=contador_menu_syncing_mmc_progress_print % max;

        sprintf(mensaje_dest,"Syncing %c",mensaje[pos]);

        zxvision_print_string_defaults_fillspc(w,1,0,mensaje_dest);
        zxvision_draw_window_contents(w);

        contador_menu_syncing_mmc_progress_print++;

}
void menu_filesel_mmc_sync(void)
{

                //Thread aparte para descomprimir. Necesario en caso de imagen de 2 gb que tarda mucho
                //Inicializar thread
        debug_printf (VERBOSE_DEBUG,"Initializing thread menu_syncing_mmc_progress_thread");


                //Lanzar el thread de sync
        //struct menu_syncing_mmc_progress_struct parametros;


        //Antes de lanzarlo, decir que se ejecuta, por si el usuario le da enter rapido a la ventana de progreso y el thread aun no se ha lanzado
        //menu_syncing_mmc_progress_thread_running=1;

        if (pthread_create( &menu_syncing_mmc_progress_thread, NULL, &menu_syncing_mmc_progress_thread_function, NULL )) {
                debug_printf(VERBOSE_ERR,"Can not create menu_syncing_mmc_progress_thread thread");
                return;
        }    


            contador_menu_syncing_mmc_progress_print=0;
        zxvision_simple_progress_window("Syncing", menu_syncing_mmc_progress_cond,menu_syncing_mmc_progress_print );

        if (diskio_syncing_flag) {

                        //Al parecer despues de ventana de zxvision_simple_progress_window no se espera a liberar tecla
                        menu_espera_no_tecla();
                        menu_warn_message("Syncing has not ended yet");
        }

        else {
            menu_generic_message_splash("Sync","OK. All changes have been written to the disk image");
        }


}

#else

void menu_filesel_mmc_sync(void)
{

    //Sync tal cual sin progreso
    //No se si hay alguien que compile sin soporte de threads, pero al menos, avisarle y mostrarle un ok cuando finalice
    menu_warn_message("Syncing disk image may take a while. Press Enter and wait please");
    diskio_sync();
    menu_generic_message_splash("Sync","OK. All changes have been written to the disk image");

}

#endif

/*
int old_menu_filesel_cambiar_unidad_o_volumen(void)
{

	int releer_directorio=0;

#ifdef MINGW
	//Mostrar selector de unidades
						char letra=menu_filesel_cambiar_unidad_windows();
					//printf ("letra: %d\n",letra);
					if (letra!=0) {
						char directorio[3];
						sprintf (directorio,"%c:",letra);

						//printf ("Changing to unit %s\n",directorio);

						zvfs_chdir(directorio);
						releer_directorio=1;
						
					}
#else

//Cambiar a ruta /media (en linux) o a /Volumes en Mac

	#if defined(__APPLE__)

//En Mac
		zvfs_chdir("/Volumes");
		releer_directorio=1;

	#else

//En Linux

		zvfs_chdir("/media");
		releer_directorio=1;	

	#endif


#endif

	return releer_directorio;
}
*/

int menu_filesel_cambiar_unidad_o_volumen(void)
{

	int releer_directorio=0;

    char directorio[PATH_MAX];

    menu_filesel_cambiar_unidad_common(directorio);

    if (directorio[0]) {
        //printf("Cambiando a directorio %s\n",directorio);

        //Si es "local drive", es que estabamos en la imagen mmc y hay que ir a imagen local
        if (!strcasecmp(directorio,"local drive")) {
            menu_current_drive_mmc_image.v=0;

            //E ir a directorio anterior
            zvfs_chdir(previous_path_before_going_mounted_drive);
            return 1;
        }

        else {
            //Si pasamos de disco local a 0:/ , guardar ruta anterior (para luego usarla al ir a local drive)
            menu_filesel_guardar_cwd_antes_mounted(directorio);

            zvfs_chdir(directorio);
            return 1;
        }
    }

    else return 0;


/*
#ifdef MINGW
	//Mostrar selector de unidades
						char letra=menu_filesel_cambiar_unidad_windows();
					//printf ("letra: %d\n",letra);
					if (letra!=0) {
						char directorio[3];
						sprintf (directorio,"%c:",letra);

						//printf ("Changing to unit %s\n",directorio);

						zvfs_chdir(directorio);
						releer_directorio=1;
						
					}
#else

//Cambiar a ruta /media (en linux) o a /Volumes en Mac

	#if defined(__APPLE__)

//En Mac
		zvfs_chdir("/Volumes");
		releer_directorio=1;

	#else

//En Linux

		zvfs_chdir("/media");
		releer_directorio=1;	

	#endif


#endif
*/

	return releer_directorio;
}

//Ultimas coordenadas de filesel
int last_filesel_ventana_x=0;
int last_filesel_ventana_y=0;
int last_filesel_ventana_visible_ancho=FILESEL_INICIAL_ANCHO;
int last_filesel_ventana_visible_alto=FILESEL_INICIAL_ALTO;
int filesel_primera_vez=1;

void menu_filesel_save_params_window(zxvision_window *ventana)
{
				//Guardar anteriores tamaños ventana
			/*last_filesel_ventana_x=ventana->x;
			last_filesel_ventana_y=ventana->y;

			last_filesel_ventana_visible_ancho=ventana->visible_width;
			last_filesel_ventana_visible_alto=ventana->visible_height;*/

	util_add_window_geometry_compact(ventana);
}


zxvision_window *menu_filesel_overlay_window;

int menu_filesel_overlay_valor_contador_segundo_anterior;


//Estructura de memoria para mostrar previews. coordenadas, color

struct s_filesel_preview_mem {
	int color;
};

//Datos del ultimo preview mostrado

int menu_filesel_overlay_last_preview_width=0;
int menu_filesel_overlay_last_preview_height=0;

//Puntero a la previsualizacion
struct s_filesel_preview_mem *menu_filesel_overlay_last_preview_memory=NULL;


void menu_filesel_overlay_assign_memory_preview(int width,int height)
{

	//Liberar si conviene
	if (menu_filesel_overlay_last_preview_memory==NULL) free(menu_filesel_overlay_last_preview_memory);


	int elementos=width*height;

	int total_mem=elementos*sizeof(struct s_filesel_preview_mem);

	menu_filesel_overlay_last_preview_memory=malloc(total_mem);

	if (menu_filesel_overlay_last_preview_memory==NULL) cpu_panic("Cannot allocate memory for image preview");

	menu_filesel_overlay_last_preview_width=width;
	menu_filesel_overlay_last_preview_height=height;
}

void menu_filesel_overlay_draw_preview_scr(int xorigen,int yorigen,int ancho,int alto,int reducir)
{   int x,y;
    int contador=0;

    int incremento=1;

    if (reducir) incremento=2;

    for (y=0;y<alto;y+=incremento) {
        for (x=0;x<ancho;x+=incremento) {

            int color_final;
            int xdestino,ydestino;

            if (reducir) {
                int colores_cuadricula[4];

                //Sacar los 4 colores de la cuadricula de 2x2
                int offset_orig;
                offset_orig=y*ancho+x;
                colores_cuadricula[0]=menu_filesel_overlay_last_preview_memory[offset_orig].color;

                offset_orig=y*ancho+x+1;
                colores_cuadricula[1]=menu_filesel_overlay_last_preview_memory[offset_orig].color;

                offset_orig=(y*ancho+1)+x;
                colores_cuadricula[2]=menu_filesel_overlay_last_preview_memory[offset_orig].color;

                offset_orig=(y*ancho+1)+x+1;
                colores_cuadricula[3]=menu_filesel_overlay_last_preview_memory[offset_orig].color;



                //Dado que partimos de una pantalla de spectrum, en una cuadricula de 2x2 habran como mucho 2 colores diferentes
                //Ver cual de los dos se repite mas

                //Asumimos el primer color, para simplificar la comparacion mas abajo
                int color_final1=colores_cuadricula[0];
                //Segundo color inicialmente a nada valido
                int color_final2=-1;

                int veces_color_final1=0;
                int veces_color_final2=0;		

                int i;

                for (i=0;i<3;i++) {	

                    if (colores_cuadricula[i]==color_final1) {
                        veces_color_final1++;
                    }
                    else {
                        color_final2=colores_cuadricula[i];
                        veces_color_final2++;
                    }

                }
                                        


                if (veces_color_final1>veces_color_final2) color_final=color_final1;
                else color_final=color_final2;

                xdestino=x/2;
                ydestino=y/2;

            }

            else {
                color_final=menu_filesel_overlay_last_preview_memory[contador].color;
                contador++;
                xdestino=x;
                ydestino=y;
            }

            //Por si acaso comprobar rangos
            if (color_final<0 || color_final>=EMULATOR_TOTAL_PALETTE_COLOURS) color_final=0;
            zxvision_putpixel(menu_filesel_overlay_window,xorigen+xdestino,yorigen+ydestino,color_final);
        }
    }
}

void menu_filesel_overlay_draw_preview_sombra_recuadro(int xorigen,int yorigen,int ancho_miniatura,int alto_miniatura)
{

    int x,y;

    //Le pongo recuadro en el mismo tamaño del preview
    int color_recuadro=ESTILO_GUI_PAPEL_TITULO;

    //Horizontal
    for (x=0;x<ancho_miniatura;x++) {
        zxvision_putpixel(menu_filesel_overlay_window,xorigen+x,yorigen,color_recuadro);
        zxvision_putpixel(menu_filesel_overlay_window,xorigen+x,yorigen+alto_miniatura-1,color_recuadro);
    }

    //Vertical
    for (y=0;y<alto_miniatura;y++) {
        zxvision_putpixel(menu_filesel_overlay_window,xorigen,yorigen+y,color_recuadro);
        zxvision_putpixel(menu_filesel_overlay_window,xorigen+ancho_miniatura-1,yorigen+y,color_recuadro);
    }

    //ponerle sombreado

    int offset_sombra=4;

    int grosor_sombra=4;

    int color_sombra=ESTILO_GUI_TINTA_NORMAL;
    int color_sombra_no=ESTILO_GUI_PAPEL_NORMAL;

    //Vertical
    for (y=offset_sombra;y<alto_miniatura+grosor_sombra;y++) {
        int i;
        for (i=0;i<grosor_sombra;i++) 
        {
            int xfinal=xorigen+ancho_miniatura+i;
            int yfinal=yorigen+y;
            int sombra_si=(xfinal+yfinal) % 2;
            int color=(sombra_si ? color_sombra : color_sombra_no);
            zxvision_putpixel(menu_filesel_overlay_window,xfinal,yfinal,color);
        }
    }

    //Horizontal
    for (x=offset_sombra;x<ancho_miniatura+grosor_sombra;x++) {
        int i;
        for (i=0;i<grosor_sombra;i++) 
        {
            int xfinal=xorigen+x;
            int yfinal=yorigen+alto_miniatura+i;
            int sombra_si=(xfinal+yfinal) % 2;
            int color=(sombra_si ? color_sombra : color_sombra_no);            
            zxvision_putpixel(menu_filesel_overlay_window,xfinal,yfinal,color);
        }
    }	    
}

void menu_filesel_overlay_draw_preview(void)
{
	//No hay imagen asignada?
	if (menu_filesel_overlay_last_preview_memory==NULL) return;	

	//Pero tiene tamanyo?
	if (menu_filesel_overlay_last_preview_width<=0 || menu_filesel_overlay_last_preview_height<=0) return;


    //printf("putpixel preview\n");

    int ancho_ventana=menu_filesel_overlay_window->visible_width-1;

    int alto_ventana=menu_filesel_overlay_window->visible_height-2;		

    //Restar barra desplazamiento, texto <dir> y mas margen
    int margen_x_coord=9;

    //asumimos imagen miniatura
    int reducir=1;

    //Minimo de caracteres a la izquierda en la ventana que queremos que se vean
    int minimo_caracteres_a_mostrar=15;

    //Lo que ocupa en caracteres en ancho un preview entero
    int caracter_ancho_miniatura=menu_filesel_overlay_last_preview_width/menu_char_width;

    //Minimo de ancho ventana para que se empiece a mostrar la miniatura (en este caso seria para mitad de scr)
    int minimo_ancho=minimo_caracteres_a_mostrar+caracter_ancho_miniatura/2;

    if (ancho_ventana<minimo_ancho) {
        //debug_printf(VERBOSE_DEBUG,"Fileselector width size too small: %d",ancho_ventana);
        return;
    }     

    //Lo que ocupa en caracteres en ancho un preview entero
    int caracter_alto_miniatura=menu_filesel_overlay_last_preview_height/menu_char_height;

    //Minimo de alto ventana para que se empiece a mostrar la miniatura (en este caso seria para mitad de scr), mas un margen
    int minimo_alto=5+caracter_alto_miniatura/2;

    //printf("minimo alto %d alto_ventana %d\n",minimo_alto,alto_ventana);

    if (alto_ventana<minimo_alto) {
        //debug_printf(VERBOSE_DEBUG,"Fileselector height size too small: %d",alto_ventana);
        return;
    }         

    //Segun el ancho de ventana, metemos una miniatura de tamaño real o dividida a la mitad, y pegada a la derecha o con margen por la derecha

    int ancho_miniatura;
    int alto_miniatura;

    //printf("%d\n",ancho_ventana);

    //En caso de tener un ancho no muy grande, desplazamos el preview a la derecha quitando el margen
    if (ancho_ventana<minimo_ancho+margen_x_coord) {
        //Tamaño reducido pegado a la derecha
        margen_x_coord=1; //1 de la barra de progreso
    }

    else if (ancho_ventana<minimo_caracteres_a_mostrar+caracter_ancho_miniatura) {
        //Tamaño reducido no pegado a la derecha
    }    

    //Y si se permite full size previews
    //Ver que el alto sea un minimo que permite un preview con tamaño razonable
    //Esto depende tambien del if posterior : if (xorigen<0 || yorigen<0) {
    else if (menu_filesel_show_previews_reduce.v==0 && alto_ventana>caracter_alto_miniatura+3) {

        reducir=0;

        if (ancho_ventana<minimo_caracteres_a_mostrar+caracter_ancho_miniatura+margen_x_coord) {  
            //Tamaño entero pero pegado a la derecha
            margen_x_coord=1; //1 de la barra de progreso
        }

        //Cualquier otro tamaño, es tamaño entero y con margen por la derecha

    }



    ancho_miniatura=menu_filesel_overlay_last_preview_width;
    alto_miniatura=menu_filesel_overlay_last_preview_height;    

    if (reducir) {
        ancho_miniatura /=2;
        alto_miniatura /=2;
    }


    int xorigen=ancho_ventana-margen_x_coord-ancho_miniatura/menu_char_width;


    int yorigen;


    //Con esto se clavaria justo en el cursor
    //yorigen=menu_filesel_overlay_window->upper_margin+filesel_linea_seleccionada;

    int alto_zona_dir=zxvision_get_filesel_alto_dir(menu_filesel_overlay_window)-1;

    //Si cursor esta por arriba
    if (filesel_linea_seleccionada<=alto_zona_dir/2+1) {
        //El preview esta abajo
        yorigen=alto_zona_dir-alto_miniatura/menu_char_height+1;
    }

    else {
        //Si no, arriba
        yorigen=0;
    }


    //Sumar zona de cabeceras
    yorigen +=menu_filesel_overlay_window->upper_margin;

    //Y ver que no se salga por la izquierda por ejemplo
    if (xorigen<0 || yorigen<0) {
        //printf("Fuera de rango: x %d y %d\n",xorigen,yorigen);
        return;
    }



    //Sumar scroll ventana
    xorigen +=menu_filesel_overlay_window->offset_x;
    yorigen +=menu_filesel_overlay_window->offset_y;


    xorigen *=menu_char_width;
    yorigen *=menu_char_height;

    
    menu_filesel_overlay_draw_preview_scr(xorigen,yorigen,menu_filesel_overlay_last_preview_width,menu_filesel_overlay_last_preview_height,reducir);

    menu_filesel_overlay_draw_preview_sombra_recuadro(xorigen,yorigen,ancho_miniatura,alto_miniatura);


}


//Reduce una imagen de un buffer , monocroma, a la mitad con destino en preview
//Entrada: colores son 0  o 1
//Salida: colores son 7 o 0
//No usado
void menu_filesel_preview_reduce_monochome(int *buffer_intermedio,int ancho, int alto)
{


	int x,y;

	int offset_final=0;

	for (y=0;y<alto;y+=2) {
		for (x=0;x<ancho;x+=2) {

			int offset_orig;
			offset_orig=y*ancho+x;
			int color1=buffer_intermedio[offset_orig];

			offset_orig=y*ancho+x+1;
			int color2=buffer_intermedio[offset_orig];

			offset_orig=(y*ancho+1)+x;
			int color3=buffer_intermedio[offset_orig];

			offset_orig=(y*ancho+1)+x+1;
			int color4=buffer_intermedio[offset_orig];

			int suma=color1+color2+color3+color4;

			//maximo sera 4

			int color_final=(suma>2 ? 0 : 7);

			//int offset_final=(y/2)*ancho_final+x/2;

			//buffer_intermedio[offset_final++]=color_final;

			menu_filesel_overlay_last_preview_memory[offset_final++].color=color_final;

		}
	}


}


//Reduce una imagen de un buffer , color, a la mitad con destino en preview. No usado
void menu_filesel_preview_reduce_scr_color(int *buffer_intermedio,int ancho, int alto)
{


	int x,y;

	int offset_final=0;



	for (y=0;y<alto;y+=2) {
		for (x=0;x<ancho;x+=2) {

			int colores_cuadricula[4];

			//Sacar los 4 colores de la cuadricula de 2x2
			int offset_orig;
			offset_orig=y*ancho+x;
			colores_cuadricula[0]=buffer_intermedio[offset_orig];

			offset_orig=y*ancho+x+1;
			colores_cuadricula[1]=buffer_intermedio[offset_orig];

			offset_orig=(y*ancho+1)+x;
			colores_cuadricula[2]=buffer_intermedio[offset_orig];

			offset_orig=(y*ancho+1)+x+1;
			colores_cuadricula[3]=buffer_intermedio[offset_orig];



			//Dado que partimos de una pantalla de spectrum, en una cuadricula de 2x2 habran como mucho 2 colores diferentes
			//Ver cual de los dos se repite mas

			//Asumimos el primer color, para simplificar la comparacion mas abajo
			int color_final1=colores_cuadricula[0];
			//Segundo color inicialmente a nada valido
			int color_final2=-1;

			int veces_color_final1=0;
			int veces_color_final2=0;		

			int i;

			for (i=0;i<3;i++) {	

				if (colores_cuadricula[i]==color_final1) {
					veces_color_final1++;
				}
				else {
					color_final2=colores_cuadricula[i];
					veces_color_final2++;
				}

			}
									

			int color_final;

			if (veces_color_final1>veces_color_final2) color_final=color_final1;
			else color_final=color_final2;

			menu_filesel_overlay_last_preview_memory[offset_final++].color=color_final;

		}
	}


}


//Copia imagen preview sin reducir
void menu_filesel_preview_no_reduce_scr(int *buffer_intermedio,int ancho, int alto)
{


	int x,y;

	int offset_final=0;
    int offset_orig=0;


	for (y=0;y<alto;y++) {
		for (x=0;x<ancho;x++) {

			int color_final=buffer_intermedio[offset_orig++];


			menu_filesel_overlay_last_preview_memory[offset_final++].color=color_final;            
        }

	}


}

void menu_filesel_preview_render_scr(char *archivo_scr)
{
			//printf("es pantalla\n");

	//Si no existe archivo, liberar preview
	if (!si_existe_archivo(archivo_scr)) {
		debug_printf(VERBOSE_DEBUG,"File SCR %s does not exist",archivo_scr);
		menu_filesel_overlay_last_preview_width=0;
		menu_filesel_overlay_last_preview_height=0;
		return;	
	}

	

		//Leemos el archivo en memoria


		debug_printf(VERBOSE_DEBUG,"Rendering SCR");  

		//buffer lectura archivo
		z80_byte *buf_pantalla;

		buf_pantalla=malloc(6912);

		if (buf_pantalla==NULL) cpu_panic("Can not allocate buffer for screen read");

		int leidos=lee_archivo(archivo_scr,(char *)buf_pantalla,6912);

		if (leidos<=0) return;

		//fread(buf_pantalla,1,6912,ptr_scrfile);

		//fclose(ptr_scrfile);



		//Asignamos primero buffer intermedio
		int *buffer_intermedio;

		int ancho=256;
		int alto=192;


		int elementos=ancho*alto;

		buffer_intermedio=malloc(sizeof(int)*elementos);

		if (buffer_intermedio==NULL)  cpu_panic("Cannot allocate memory for reduce buffer");					  
		

		int x,y,bit_counter;

		z80_int offset_lectura=0;
		for (y=0;y<192;y++) {
			for (x=0;x<32;x++) {
				z80_byte leido;
				int offset_orig=screen_addr_table[y*32+x];
				//fread(&leido,1,1,ptr_scrfile);
				leido=buf_pantalla[offset_orig];

				//int xdestino,ydestino;

				//esta funcion no es muy rapida pero....
				//util_spectrumscreen_get_xy(offset_lectura,&xdestino,&ydestino);

				offset_lectura++;

				int offset_destino=y*256+x*8;

				int tinta;
				int papel;

				z80_byte atributo;

				int pos_attr;

				//pos_attr=(ydestino/8)*32+(xdestino/8);

				pos_attr=6144+((y/8)*32)+x;
				//printf("%d\n",pos_attr);

				atributo=buf_pantalla[pos_attr];

				//atributo=56;

				tinta=(atributo)&7;
				papel=(atributo>>3)&7;

				if (atributo & 64) {
					tinta +=8;
					papel +=8;
				}

				

				for (bit_counter=0;bit_counter<8;bit_counter++) {
					
					//de momento solo 0 o 1
					int color=(leido & 128 ? tinta : papel);

					
					//menu_filesel_overlay_last_preview_memory[offset].color=color;

					buffer_intermedio[offset_destino+bit_counter]=color;
					leido=leido << 1;
				}
			}
		}
		


		free(buf_pantalla);

        //TODO: detectar esto en base al tamaño ventana (ancho y alto)
        /*int reducir_mitad=1;


        if (reducir_mitad) {
		    //Reducir a 128x96
		    menu_filesel_overlay_assign_memory_preview(128,96);

		    //menu_filesel_preview_reduce_monochome(buffer_intermedio,256,192);

		    menu_filesel_preview_reduce_scr_color(buffer_intermedio,256,192);
        }

        else {
		    //No reducir, tal cual a 256x192
		    menu_filesel_overlay_assign_memory_preview(256,192);

		    menu_filesel_preview_no_reduce_scr(buffer_intermedio,256,192);            
        }*/


        //Ahora siempre se lee el preview a tamaño completo,
        //y si hay que reducirlo se hace sobre la marcha en la funcion de overlay
        menu_filesel_overlay_assign_memory_preview(256,192);

        menu_filesel_preview_no_reduce_scr(buffer_intermedio,256,192);           

		free(buffer_intermedio);



}


char menu_filesel_last_preview_file[PATH_MAX]="";

//Renderizar preview en memoria del archivo seleccionado
void menu_filesel_overlay_render_preview_in_memory(void)
{



	//de momento nada mas
	debug_printf(VERBOSE_DEBUG,"Preview File: %s",filesel_nombre_archivo_seleccionado);

	if (!strcmp(menu_filesel_last_preview_file,filesel_nombre_archivo_seleccionado)) {
		debug_printf(VERBOSE_DEBUG,"File is the same as before. Do not do anything");
		return;
	}    

    strcpy(menu_filesel_last_preview_file,filesel_nombre_archivo_seleccionado);

    //Si no existe
    //Esto sucede cuando se escribe el nombre del archivo a mano desde el campo File del fileselector
    if (!si_existe_archivo(filesel_nombre_archivo_seleccionado)) {
        debug_printf(VERBOSE_DEBUG,"%s does not exist when rendering preview",filesel_nombre_archivo_seleccionado);
        return;
    }


    if (file_is_directory(filesel_nombre_archivo_seleccionado)) {
        debug_printf(VERBOSE_DEBUG,"File is a directory, do not do anything");
        //Pero quitar la posible preview anterior
        menu_filesel_overlay_last_preview_width=0;
        menu_filesel_overlay_last_preview_height=0;	        
        return;
    }


	debug_printf(VERBOSE_DEBUG,"Rendering file preview");

	long int file_size=get_file_size(filesel_nombre_archivo_seleccionado);

	//Creamos carpeta temporal por si no existe
	char tmpdir[PATH_MAX];

	//Carpeta temporal debe ser distinta del nombre del archivo
	//por si a alguien le da por hacer preview de un archivo de esa misma carpeta temporal
	//que eso sucede por ejemplo al descargar juegos del online browser en zx81
	sprintf (tmpdir,"%s/%s_previewdir",get_tmpdir_base(),filesel_nombre_archivo_seleccionado);
	//sprintf (tmpdir,"%s/%s",get_tmpdir_base(),filesel_nombre_archivo_seleccionado);

	//Crear carpeta solo cuando va a haber un preview
	//menu_filesel_mkdir(tmpdir);	

	//Definimos tmpfile_scr para los que convierten snapshot directo a scr
	char tmpfile_scr[PATH_MAX];	
	sprintf (tmpfile_scr,"%s/%s.scr",tmpdir,filesel_nombre_archivo_seleccionado);	


	//Si es tap o tzx o pzx o trd
	// 
	if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"tap") ||
		!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"tzx") ||
		!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"pzx") ||
		!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"trd") ||
		!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"dsk") ||
        !util_compare_file_extension(filesel_nombre_archivo_seleccionado,"ddh") 
	
	) {
	
		menu_filesel_mkdir(tmpdir);

		//Ver si hay archivo que indica pantalla
		char archivo_info_pantalla[PATH_MAX];
		sprintf(archivo_info_pantalla,"%s/%s",tmpdir,MENU_SCR_INFO_FILE_NAME);

		if (!si_existe_archivo(archivo_info_pantalla)) {
			//Archivo scr no existe. Extraer
			debug_printf(VERBOSE_DEBUG,"File SCR does not exist. Extracting");


			int retorno=1;

			if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"tap") ) {
					debug_printf (VERBOSE_DEBUG,"Is a tap file");
					retorno=util_extract_tap(filesel_nombre_archivo_seleccionado,tmpdir,NULL,0);
			}

			else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"tzx") ) {
					debug_printf (VERBOSE_DEBUG,"Is a tzx file");
					retorno=util_extract_tzx(filesel_nombre_archivo_seleccionado,tmpdir,NULL);
			}

			else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"pzx") ) {
					debug_printf (VERBOSE_DEBUG,"Is a pzx file");
					retorno=util_extract_pzx(filesel_nombre_archivo_seleccionado,tmpdir,NULL);
			}		

			else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"trd") ) {
					debug_printf (VERBOSE_DEBUG,"Is a trd file");
					retorno=util_extract_trd(filesel_nombre_archivo_seleccionado,tmpdir);
			}		

			else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"ddh") ) {
					debug_printf (VERBOSE_DEBUG,"Is a ddh file");
					retorno=util_extract_ddh(filesel_nombre_archivo_seleccionado,tmpdir);
			}	            

			else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"dsk") ) {
					debug_printf (VERBOSE_DEBUG,"Is a dsk file");
                    //Ejemplos de DSK que muestran pantalla: CASTLE MASTER.DSK , Drazen Petrovic Basket.dsk
                    //printf("Before extract dsk\n");
					retorno=util_extract_dsk(filesel_nombre_archivo_seleccionado,tmpdir);
                    //printf("After extract dsk\n");
			}				

            //printf("if_pending_error_message: %d\n",if_pending_error_message);
            //Quitar posibles errores al preparar esta preview
            //no queremos alertar al usuario por archivos incorrectos
            //De todas maneras siempre se vería el error en la consola
            if_pending_error_message=0;

			if (retorno!=0) {
				//ERROR
				return;
			}

		}

		else {
			debug_printf(VERBOSE_DEBUG,"SCR file already exists");
		}

		//Ver si hay archivo que indica pantalla
        //printf("archivo_info_pantalla %s\n",archivo_info_pantalla);

		if (si_existe_archivo(archivo_info_pantalla)) {
			//printf("HAY PANTALLA--------------- \n");

			char buf_archivo_scr[PATH_MAX];

			lee_archivo(archivo_info_pantalla,buf_archivo_scr,PATH_MAX-1);

			//printf ("PANTALLA:     %s\n",buf_archivo_scr);

			menu_filesel_preview_render_scr(buf_archivo_scr);
		}

		else {
			//printf("NO HAY PANTALLA****************\n");
            debug_printf(VERBOSE_DEBUG,"There is no SCR file");

			//liberar preview
			menu_filesel_overlay_last_preview_width=0;
			menu_filesel_overlay_last_preview_height=0;			
		}
	}


	//Si es scr o tamaño 6912
    //TODO: en el caso improbable que otro archivo que no sea una pantalla y ocupe 6912 bytes, se mostrara como pantalla
    //esto lo hago porque por ejemplo si expandimos un dsk u otro archivo expandible que tiene dentro una pantalla,
    //seguramente no tendra extension scr y queremos mostrar una pantalla que este dentro al expandir el archivo
	else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"scr")
        || file_size==6912
        ) {
		debug_printf(VERBOSE_DEBUG,"File is a scr screen");

		menu_filesel_preview_render_scr(filesel_nombre_archivo_seleccionado);

	}

	//Si es sna
	else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"sna")) {
		debug_printf(VERBOSE_DEBUG,"File is a sna snapshot");

		menu_filesel_mkdir(tmpdir);

		//Si no existe preview
		if (!si_existe_archivo(tmpfile_scr)) {
			util_convert_sna_to_scr(filesel_nombre_archivo_seleccionado,tmpfile_scr);
		}

		menu_filesel_preview_render_scr(tmpfile_scr);

	}	

	//Si es sp
	else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"sp")) {
		debug_printf(VERBOSE_DEBUG,"File is a sp snapshot");

		menu_filesel_mkdir(tmpdir);

		//Si no existe preview
		if (!si_existe_archivo(tmpfile_scr)) {
			util_convert_sp_to_scr(filesel_nombre_archivo_seleccionado,tmpfile_scr);
		}

		menu_filesel_preview_render_scr(tmpfile_scr);

	}	

	//Si es z80
	else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"z80")) {
		debug_printf(VERBOSE_DEBUG,"File is a z80 snapshot");

		menu_filesel_mkdir(tmpdir);

		//Si no existe preview
		if (!si_existe_archivo(tmpfile_scr)) {
			util_convert_z80_to_scr(filesel_nombre_archivo_seleccionado,tmpfile_scr);
		}

		menu_filesel_preview_render_scr(tmpfile_scr);

	}		

	//Si es P
	else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"p")) {
		debug_printf(VERBOSE_DEBUG,"File is a p snapshot");

		menu_filesel_mkdir(tmpdir);

		//Si no existe preview
		if (!si_existe_archivo(tmpfile_scr)) {
			util_convert_p_to_scr(filesel_nombre_archivo_seleccionado,tmpfile_scr);
		}

		menu_filesel_preview_render_scr(tmpfile_scr);

	}		

	//Si es ZSF
	else if (!util_compare_file_extension(filesel_nombre_archivo_seleccionado,"zsf")) {
		debug_printf(VERBOSE_DEBUG,"File is a zsf snapshot");

		menu_filesel_mkdir(tmpdir);

		//Si no existe preview
		if (!si_existe_archivo(tmpfile_scr)) {
			util_convert_zsf_to_scr(filesel_nombre_archivo_seleccionado,tmpfile_scr);
		}

		menu_filesel_preview_render_scr(tmpfile_scr);

	}			


	else {
		//Cualquier otra cosa, liberar preview
		menu_filesel_overlay_last_preview_width=0;
		menu_filesel_overlay_last_preview_height=0;
	}

    //printf("if_pending_error_message: %d\n",if_pending_error_message);
    //Quitar posibles errores al preparar esta preview
    //no queremos alertar al usuario por archivos incorrectos
    //De todas maneras siempre se vería el error en la consola
    if_pending_error_message=0;


}

//Overlay para mostrar los previews
void menu_filesel_overlay(void)
{
	if (!zxvision_drawing_in_background) normal_overlay_texto_menu();


	//Y el procesado de nueva preview no tan seguido
	//esto hara ejecutar esto 5 veces por segundo
	if ( ((contador_segundo%200) == 0 && menu_filesel_overlay_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
		menu_filesel_overlay_valor_contador_segundo_anterior=contador_segundo;

		//renderizar preview en memoria si conviene

		menu_filesel_overlay_render_preview_in_memory();

	}



	//El overlay de la pantalla siempre
    menu_filesel_overlay_draw_preview();

    //Realmente no tiene contenido de texto este overlay, pero dado que estamos
    //dibujando un preview, necesitamos que cuando no haya preview, este zxvision_draw_window_contents limpie
    //el contenido de la ventana gracias al parametro de must_clear_cache_on_draw que hemos activado al crear la ventana
    //Si no se hiciera esto, la cache de putchar veria que no hay modificaciones y no limpia el preview anterior
    zxvision_draw_window_contents(menu_filesel_overlay_window);
}

void menu_filesel_preexit(zxvision_window *ventana)
{
    //restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);

    zxvision_destroy_window(ventana);

}

//Retorna 1 si seleccionado archivo. Retorna 0 si sale con ESC
//Si seleccionado archivo, lo guarda en variable *archivo
//Si sale con ESC, devuelve en menu_filesel_last_directory_seen ultimo directorio
int menu_filesel(char *titulo,char *filtros[],char *archivo)
{

	//En el caso de stdout es mucho mas simple
    if (!strcmp(scr_new_driver_name,"stdout")) {
		printf ("%s :\n",titulo);
		scrstdout_menu_print_speech_macro(titulo);
		scanf("%s",archivo);
		return 1;
    }


	menu_reset_counters_tecla_repeticion();

	int tecla;


	filesel_zona_pantalla=1;

	getcwd(filesel_directorio_inicial,PATH_MAX);


    //printf ("confirm\n");

	//printf ("antes menu_espera_no_tecla en menu filesel\n");

	menu_espera_no_tecla();

	//printf ("despues menu_espera_no_tecla en menu filesel\n");
    	
	zxvision_window ventana_filesel;
	zxvision_window *ventana;

	//Inicialmente a NULL
	ventana=NULL;


	//guardamos filtros originales
	filesel_filtros_iniciales=filtros;



    filtros_todos_archivos[0]="";
    filtros_todos_archivos[1]=0;

	filesel_filtros=filtros;

	filesel_item *item_seleccionado;

	int aux_pgdnup;
	//menu_active_item_primera_vez=1;

	//Decir directorio activo
	menu_textspeech_say_current_directory();

	//Inicializar mouse wheel a 0, por si acaso
	mouse_wheel_vertical=mouse_wheel_horizontal=0;


//Esto lo hago para poder debugar facilmente la opcion de cambio de unidad
/*#ifdef MINGW
	int we_are_windows=1;
#else
	int we_are_windows=0;
	
#endif*/


	do {
		menu_speech_tecla_pulsada=0;
		menu_active_item_primera_vez=1;
		filesel_linea_seleccionada=0;
		filesel_archivo_seleccionado=0;
		//leer todos archivos
		int ret=menu_filesel_readdir();
		if (ret) {
			//Error leyendo directorio
			//restauramos modo normal de texto de menu
     		set_menu_overlay_function(normal_overlay_texto_menu);
			
			menu_espera_no_tecla();
			zvfs_chdir(filesel_directorio_inicial);
			menu_filesel_free_mem();
			zxvision_destroy_window(ventana);
			return 0;
                                		
		}


		//printf ("Total archivos en directorio: %d\n",filesel_total_items);
		//printf ("despues leer directorio\n");
		//Crear ventana. Si ya existia, borrarla
		if (ventana!=NULL) {
			//printf ("Destroy previous filesel window\n");
			

			//Guardar anteriores tamaños ventana
			menu_filesel_save_params_window(ventana);

            menu_filesel_preexit(ventana);
		}
		ventana=&ventana_filesel;

		int alto_total=filesel_total_items+ZXVISION_FILESEL_INITIAL_MARGIN; //Sumarle las leyendas, etc
		

		//Usar ultimas coordenadas y tamaño, sin comprobar rango de maximo ancho y alto 32x24
		//Si no hay ultimas, poner las de por defecto

        int is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;  //aunque no usamos estos parametros

		if (!util_find_window_geometry("filesel",&last_filesel_ventana_x,&last_filesel_ventana_y,
            &last_filesel_ventana_visible_ancho,&last_filesel_ventana_visible_alto,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {
			last_filesel_ventana_x=FILESEL_INICIAL_X;
			last_filesel_ventana_y=FILESEL_INICIAL_Y;
			last_filesel_ventana_visible_ancho=FILESEL_INICIAL_ANCHO;
			last_filesel_ventana_visible_alto=FILESEL_INICIAL_ALTO;	
		}


		//zxvision_new_window_check_range(&last_filesel_ventana_x,&last_filesel_ventana_y,&last_filesel_ventana_visible_ancho,&last_filesel_ventana_visible_alto);
		//zxvision_new_window_no_check_range(ventana,last_filesel_ventana_x,last_filesel_ventana_y,last_filesel_ventana_visible_ancho,last_filesel_ventana_visible_alto,last_filesel_ventana_visible_ancho-1,alto_total,titulo);
		zxvision_new_window_nocheck_staticsize(ventana,last_filesel_ventana_x,last_filesel_ventana_y,last_filesel_ventana_visible_ancho,last_filesel_ventana_visible_alto,last_filesel_ventana_visible_ancho-1,alto_total,titulo);

	    ventana->upper_margin=4;
	    ventana->lower_margin=4;
		zxvision_set_visible_cursor(ventana);
		strcpy(ventana->geometry_name,"filesel");

		if (menu_filesel_show_utils.v) {
			//Activar los hotkeys desde raton en el caso de file utilities
			ventana->can_mouse_send_hotkeys=1;
		}

        //Para que elimine restos de anteriores previews
        ventana->must_clear_cache_on_draw=1;

		zxvision_draw_window(ventana);


		//Overlay para los previews. Siempre que tengamos video driver completo
        if (si_complete_video_driver() ) {
                
			if (menu_filesel_show_previews.v) {
				menu_filesel_overlay_window=ventana;
				set_menu_overlay_function(menu_filesel_overlay);
			}

		}

		zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
		zxvision_menu_filesel_print_text_contents(ventana);
		zxvision_menu_filesel_print_legend(ventana);
		int releer_directorio=0;



		//El menu_print_dir aqui no hace falta porque ya entrara en el switch (filesel_zona_pantalla) inicialmente cuando filesel_zona_pantalla vale 1
		//menu_print_dir(filesel_archivo_seleccionado);

		zxvision_draw_window_contents(ventana);

		menu_refresca_pantalla();


		if (menu_filesel_posicionar_archivo.v) {
			zxvision_menu_filesel_localiza_archivo(ventana,menu_filesel_posicionar_archivo_nombre);

			menu_filesel_posicionar_archivo.v=0;
		}


		do {
			//printf ("\nReleer directorio\n");
			//printf ("cursor_line: %d filesel_linea_seleccionada: %d filesel_archivo_seleccionado %d\n",
			//	ventana->cursor_line,filesel_linea_seleccionada,filesel_archivo_seleccionado);

			//printf("filesel_linea_seleccionada: %d\n",filesel_linea_seleccionada);


			//printf ("(FILESEL_ALTO-10): %d zxvision_get_filesel_alto_dir: %d\n",(FILESEL_ALTO-10),zxvision_get_filesel_alto_dir(ventana) );

			switch (filesel_zona_pantalla) {
				case 0:
				//zona superior de nombre de archivo
				zxvision_reset_visible_cursor(ventana);
		                zxvision_menu_print_dir(filesel_archivo_seleccionado,ventana);
				zxvision_draw_window_contents(ventana);
                		//para que haga lectura del edit box
		                menu_speech_tecla_pulsada=0;

				int ancho_mostrado=ventana->visible_width-6-2;
				if (ancho_mostrado<2) {
					//La ventana es muy pequeña como para editar
					menu_reset_counters_tecla_repeticion();
					filesel_zona_pantalla=1;
					//no releer todos archivos
					menu_speech_tecla_pulsada=1;					

				}

				else {


				tecla=zxvision_scanf(ventana,filesel_nombre_archivo_seleccionado,PATH_MAX,ancho_mostrado,7,1,0);
				//); //6 ocupa el texto "File: "

				if (tecla==15) {
					//printf ("TAB. siguiente seccion\n");
					menu_reset_counters_tecla_repeticion();
					filesel_zona_pantalla=1;
					//no releer todos archivos
					menu_speech_tecla_pulsada=1;
				}

				//ESC
                if (tecla==2) {
                	menu_filesel_exist_ESC();

                    menu_filesel_preexit(ventana);
                    return 0;                    
				}

				if (tecla==13) {

					//Si es Windows y se escribe unidad: (ejemplo: "D:") hacer chdir
					int unidadwindows=0;
#ifdef MINGW

                    if (strlen(filesel_nombre_archivo_seleccionado)==3 && 
                    util_path_is_windows_with_drive(filesel_nombre_archivo_seleccionado)
                    ) {

					//if (filesel_nombre_archivo_seleccionado[0] &&
					//	filesel_nombre_archivo_seleccionado[1]==':' &&
					//	filesel_nombre_archivo_seleccionado[2]==0 )
					//	{
						debug_printf (VERBOSE_INFO,"%s is a Windows drive",filesel_nombre_archivo_seleccionado);
						unidadwindows=1;
					}
#endif


					//si es directorio, cambiamos
                    /*
					struct stat buf_stat;
					int stat_valor;
					stat_valor=stat(filesel_nombre_archivo_seleccionado, &buf_stat);
					if (
						(stat_valor==0 && S_ISDIR(buf_stat.st_mode) ) ||
						(unidadwindows)
						) {
                    */
                    //printf("tipo archivo: %s: %d\n",filesel_nombre_archivo_seleccionado,get_file_type(filesel_nombre_archivo_seleccionado));
                    //TODO: esto no funciona para raiz 0:/ pero si para 0:/DOCS por ejemplo
                    if (get_file_type(filesel_nombre_archivo_seleccionado)==2 || unidadwindows) {


						debug_printf (VERBOSE_DEBUG,"%s Is a directory or windows drive. Change",filesel_nombre_archivo_seleccionado);
                                                zvfs_chdir(filesel_nombre_archivo_seleccionado);
						menu_filesel_free_mem();
                                                releer_directorio=1;
						filesel_zona_pantalla=1;

					        //Decir directorio activo
						//Esperar a liberar tecla si no la tecla invalida el speech
						menu_espera_no_tecla();
					        menu_textspeech_say_current_directory();


					}


					//sino, devolvemos nombre con path, siempre que extension sea conocida
					else {
                    	
                        menu_espera_no_tecla();

						if (menu_avisa_si_extension_no_habitual(filtros,filesel_nombre_archivo_seleccionado)) {

                        //unimos directorio y nombre archivo. siempre que inicio != '/'
						if (filesel_nombre_archivo_seleccionado[0]!='/') {
                        	getcwd(archivo,PATH_MAX);
                            sprintf(&archivo[strlen(archivo)],"/%s",filesel_nombre_archivo_seleccionado);
						}

						else sprintf(archivo,"%s",filesel_nombre_archivo_seleccionado);


                        zvfs_chdir(filesel_directorio_inicial);
						menu_filesel_free_mem();

						menu_filesel_preexit(ventana);
						last_filesused_insert(archivo);
						return 1;

						}

						else {
							//Extension no conocida. No modificar variable archivo
							//printf ("Unknown extension. Do not modify archivo. Contents: %s\n",archivo);
							//restauramos modo normal de texto de menu
     						menu_filesel_preexit(ventana);
							return 0;
						}
						



						//Volver con OK
                        //return 1;

					}
				}

				}

				break;
			
			case 1:
				//zona selector de archivos

				debug_printf (VERBOSE_DEBUG,"Read directory. menu_speech_tecla_pulsada=%d",menu_speech_tecla_pulsada);
				zxvision_set_visible_cursor(ventana);
				zxvision_menu_print_dir(filesel_archivo_seleccionado,ventana);

                //Queremos que actualice la leyenda sobretodo en el caso de fileutils, para mostrar/ocultar
                //opciones segun si seleccionado archivo o directorio
                zxvision_menu_filesel_print_legend(ventana);
				zxvision_draw_window_contents(ventana);

				//Para no releer todas las entradas
				menu_speech_tecla_pulsada=1;


				tecla=zxvision_common_getkey_refresh();
				//printf ("Despues lee tecla\n");


				//Si se ha pulsado boton de raton
                                if (mouse_left) {
					//printf ("Pulsado boton raton izquierdo\n");

					 //Si en linea de "File"
					menu_filesel_change_zone_if_clicked(ventana,&filesel_zona_pantalla,&tecla);
                                        /*if (menu_mouse_y==2 && menu_mouse_x<ventana->visible_width-1) {
						printf ("Pulsado zona File\n");
                                                                menu_reset_counters_tecla_repeticion();
                                                                filesel_zona_pantalla=0;
                                                                tecla=0;
                                        }*/

					if (si_menu_mouse_en_ventana() && zxvision_si_mouse_zona_archivos(ventana) ) {
						//Ubicamos cursor donde indica raton
						if (menu_filesel_set_cursor_at_mouse(ventana)) {
							//Como pulsar enter
							tecla=13;
						}
					}
				}


				//Si se ha movido raton. Asumimos que ha vuelto de leer tecla, tecla=0 y no se ha pulsado mouse
				if (!tecla && !mouse_left) {
				 //if (mouse_movido) {
                    //printf ("mouse x: %d y: %d menu mouse x: %d y: %d\n",mouse_x,mouse_y,menu_mouse_x,menu_mouse_y);
                    //printf ("ventana x %d y %d ancho %d alto %d\n",ventana_x,ventana_y,ventana_ancho,ventana_alto);
                    if (si_menu_mouse_en_ventana() ) {
                        //printf ("dentro ventana\n");
                        //Ver en que zona esta
                        
                        if (zxvision_si_mouse_zona_archivos(ventana)) {
							menu_filesel_set_cursor_at_mouse(ventana);						

                        }

                        else if (menu_mouse_y==(ventana->visible_height-4)+1) {
                            //En la linea de filtros
                            //nada en especial
                            //printf ("En linea de filtros\n");
                        }
                    }
                else {
                    //printf ("fuera ventana\n");
                }

        }



				switch (tecla) {
					//abajo
					case 10:
						zxvision_menu_filesel_cursor_abajo(ventana);
						//Para no releer todas las entradas
						menu_speech_tecla_pulsada=1;
					break;

					//arriba
					case 11:
						zxvision_menu_filesel_cursor_arriba(ventana);
						//Para no releer todas las entradas
						menu_speech_tecla_pulsada=1;
					break;

					//PgDn
					case 25:
						for (aux_pgdnup=0;aux_pgdnup<zxvision_get_filesel_alto_dir(ventana);aux_pgdnup++)
							zxvision_menu_filesel_cursor_abajo(ventana);
						//releer todas entradas
						menu_speech_tecla_pulsada=0;
						//y decir selected item
						menu_active_item_primera_vez=1;
                    break;

					//PgUp
					case 24:
						for (aux_pgdnup=0;aux_pgdnup<zxvision_get_filesel_alto_dir(ventana);aux_pgdnup++)
							zxvision_menu_filesel_cursor_arriba(ventana);
						//releer todas entradas
						menu_speech_tecla_pulsada=0;
						//y decir selected item
						menu_active_item_primera_vez=1;
                    break;


					case 15:
					//tabulador
						menu_reset_counters_tecla_repeticion();
						if (menu_filesel_show_utils.v==0) filesel_zona_pantalla=2;
						else filesel_zona_pantalla=0; //Si hay utils, cursor se va arriba
					break;

					//ESC
					case 2:
						//meter en menu_filesel_last_directory_seen nombre directorio
						//getcwd(archivo,PATH_MAX);
						zvfs_getcwd(menu_filesel_last_directory_seen,PATH_MAX);
						//printf ("salimos con ESC. nombre directorio: %s\n",archivo);
                        menu_filesel_exist_ESC();

                        //Guardamos geometria al pulsar Escape
                        menu_filesel_save_params_window(ventana);

						menu_filesel_preexit(ventana);
                        return 0;

					break;

					//Expandir archivos
					case 32:
						menu_first_aid("filesel_enter_key");

                                                item_seleccionado=menu_get_filesel_item(filesel_archivo_seleccionado+filesel_linea_seleccionada);
                                                menu_reset_counters_tecla_repeticion();

                                                //printf ("despues de get filesel item. item_seleccionado=%p\n",item_seleccionado);

                                                if (item_seleccionado==NULL) {
                                                        //Esto pasa en las carpetas vacias, como /home en Mac OS
                                                                        menu_filesel_exist_ESC();
																		menu_filesel_preexit(ventana);
                                                                        return 0;


                                                }

						if (get_file_type(item_seleccionado->d_name)==2) {
							debug_printf(VERBOSE_INFO,"Can't expand directories");
						}

						else {
								debug_printf(VERBOSE_DEBUG,"Expanding file %s",item_seleccionado->d_name);
                                                                char tmpdir[PATH_MAX];

                                                                if (menu_filesel_expand(item_seleccionado->d_name,tmpdir) ) {
									//TODO: Si lanzo este warning se descuadra el dibujado de ventana
									//menu_warn_message("Don't know how to expand that file");
									debug_printf(VERBOSE_INFO,"Don't know how to expand that file");
                                                                }

                                                                else {
                                                                        menu_filesel_change_to_tmp(tmpdir);
																		releer_directorio=1;
                                                                }
						}


					break;

					case 13:

						menu_first_aid("filesel_enter_key");

						//si seleccion es directorio
						item_seleccionado=menu_get_filesel_item(filesel_archivo_seleccionado+filesel_linea_seleccionada);
						menu_reset_counters_tecla_repeticion();

						//printf ("despues de get filesel item. item_seleccionado=%p\n",item_seleccionado);

						if (item_seleccionado==NULL) {
							//Esto pasa en las carpetas vacias, como /home en Mac OS
                                                                        menu_filesel_exist_ESC();
																		menu_filesel_preexit(ventana);
                                                                        return 0;


						}

						if (get_file_type(item_seleccionado->d_name)==2) {
							debug_printf (VERBOSE_DEBUG,"Is a directory. Change");
							char *directorio_a_cambiar;

							//suponemos esto:
							directorio_a_cambiar=item_seleccionado->d_name;
							char last_directory[PATH_MAX];

							//si es "..", ver si directorio actual contiene archivo que indica ultimo directorio
							//en caso de descompresiones
							if (!strcmp(item_seleccionado->d_name,"..")) {
								debug_printf (VERBOSE_DEBUG,"Is directory ..");
								if (si_existe_archivo(MENU_LAST_DIR_FILE_NAME)) {
									debug_printf (VERBOSE_DEBUG,"Directory has file " MENU_LAST_DIR_FILE_NAME " Changing "
											"to previous directory");

									if (menu_filesel_read_file_last_dir(last_directory)==0) {
										debug_printf (VERBOSE_DEBUG,"Previous directory was: %s",last_directory);

										directorio_a_cambiar=last_directory;
									}

								}
							}

							debug_printf (VERBOSE_DEBUG,"Changing to directory %s",directorio_a_cambiar);

                            //printf("cambiando a directorio %s desde filesel\n",directorio_a_cambiar);

							zvfs_chdir(directorio_a_cambiar);


							menu_filesel_free_mem();
							releer_directorio=1;

						        //Decir directorio activo
							//Esperar a liberar tecla si no la tecla invalida el speech
							menu_espera_no_tecla();
						        menu_textspeech_say_current_directory();

						}

						else {

							//Si seleccion es archivo comprimido
							if (menu_util_file_is_compressed(item_seleccionado->d_name) ) {
								debug_printf (VERBOSE_DEBUG,"Is a compressed file");

								char tmpdir[PATH_MAX];

								if (menu_filesel_uncompress(item_seleccionado->d_name,tmpdir) ) {
									menu_filesel_exist_ESC();
									menu_filesel_preexit(ventana);
									return 0;
								}

								else {
									menu_filesel_change_to_tmp(tmpdir);
                        	                                        releer_directorio=1;
								}

							}

							else {
								//Enter. No es directorio ni archivo comprimido
								//Si estan las file utils, enter no hace nada
								if (menu_filesel_show_utils.v==0) { 

					                
        	                        menu_espera_no_tecla();

									if (menu_avisa_si_extension_no_habitual(filtros,filesel_nombre_archivo_seleccionado)) {

									//unimos directorio y nombre archivo
									getcwd(archivo,PATH_MAX);
									sprintf(&archivo[strlen(archivo)],"/%s",item_seleccionado->d_name);

									zvfs_chdir(filesel_directorio_inicial);
									menu_filesel_free_mem();

									//return menu_avisa_si_extension_no_habitual(filtros,archivo);
									//Guardar anteriores tamaños ventana
									menu_filesel_save_params_window(ventana);

									menu_filesel_preexit(ventana);
									last_filesused_insert(archivo);
									return 1;

									}

                                    else {
                                                        //Extension no conocida. No modificar variable archivo
                                                        //printf ("Unknown extension. Do not modify archivo. Contents: %s\n",archivo);

														menu_filesel_preexit(ventana);
                                                        return 0;
                                    }


									//Volver con OK
									//return 1;
								}
							}

						}
					break;
				}

				//entre a y z y numeros
				if ( (tecla>='a' && tecla<='z') || (tecla>='0' && tecla<='9') ) {
					menu_first_aid("filesel_uppercase_keys");
					zxvision_menu_filesel_localiza_letra(ventana,tecla);
				}


				if (tecla=='D') {
					releer_directorio=menu_filesel_cambiar_unidad_o_volumen();
					
				}

				if (tecla=='R') {	

					//Archivos recientes
					char *archivo_reciente=menu_filesel_recent_files();
					if (archivo_reciente!=NULL) {
						//printf ("Loading file %s\n",archivo_reciente);
						strcpy(archivo,archivo_reciente);

                        zvfs_chdir(filesel_directorio_inicial);
                        menu_filesel_free_mem();

                        //return menu_avisa_si_extension_no_habitual(filtros,archivo);

                        menu_filesel_preexit(ventana);
                        return 1;

					}
				}

				//Si esta filesel, opciones en mayusculas
				if (menu_filesel_show_utils.v) {
					
					if ( (tecla>='A' && tecla<='Z') ) {
						menu_espera_no_tecla();
						//TODO: Si no se pone espera_no_tecla,
						//al aparecer menu de, por ejemplo truncate, el texto se fusiona con el fondo de manera casi transparente,
						//como si no borrase el putpixel cache
						//esto también sucede en otras partes del código del menú pero no se por que es

						menu_reset_counters_tecla_repeticion();
						
						//Comun para acciones que usan archivo seleccionado
						if (tecla=='V' || tecla=='T' || tecla=='E' || tecla=='M' || tecla=='N' || tecla=='C' 
                            || tecla=='P' || tecla=='F' || tecla=='O' || tecla=='I' || tecla=='U' || tecla=='S') {


						    //releer con speech
						    menu_speech_tecla_pulsada=0;                                
							
							//Obtener nombre del archivo al que se apunta
							char file_utils_file_selected[PATH_MAX]="";
							item_seleccionado=menu_get_filesel_item_cursor();
							if (item_seleccionado!=NULL) {

                                int tipo_archivo_seleccionado=get_file_type(item_seleccionado->d_name);

								//Esto pasa en las carpetas vacias, como /home en Mac OS
									//unimos directorio y nombre archivo
									//getcwd(file_utils_file_selected,PATH_MAX);
                                    zvfs_getcwd(file_utils_file_selected,PATH_MAX);


									sprintf(&file_utils_file_selected[strlen(file_utils_file_selected)],"/%s",item_seleccionado->d_name);								
								//Info para cualquier tipo de archivo
								if (tecla=='I') file_utils_info_file(file_utils_file_selected);


                                //Rename para cualquier tipo de archivo
                                if (tecla=='N') {
                                    file_utils_move_rename_copy_file(file_utils_file_selected,1);
                                    releer_directorio=1;
                                }

                                //Copy para cualquier tipo de origen. Si es directorio, hara copy recursive
                                if (tecla=='C') {
                                    file_utils_move_rename_copy_file(file_utils_file_selected,2);
                                    //Restaurar variables globales que se alteran al llamar al otro filesel
                                    //TODO: hacer que estas variables no sean globales sino locales de esta funcion menu_filesel
                                    filesel_filtros_iniciales=filtros;
                                    filesel_filtros=filtros;
                                    
                                    releer_directorio=1;
                                }     

                                //Move para cualquier tipo de origen. Aunque no permitimos mover carpetas entre diferentes filesystems
                                if (tecla=='M') {
                                    file_utils_move_rename_copy_file(file_utils_file_selected,0);
                                    //Restaurar variables globales que se alteran al llamar al otro filesel
                                    //TODO: hacer que estas variables no sean globales sino locales de esta funcion menu_filesel
                                    filesel_filtros_iniciales=filtros;
                                    filesel_filtros=filtros;
    
                                    releer_directorio=1;

                                }                                                                                                                               


                                //Delete para cualquier tipo de archivo
                                if (tecla=='E') {
                                    if (menu_confirm_yesno_texto("Delete","Sure?")) {
                                        file_utils_delete(file_utils_file_selected);
                                      
                                        releer_directorio=1;
                                    }

                                }                                

                                //Umount da igual el tipo de archivo seleccionado
                                if (tecla=='U') {
                                    if (menu_mmc_image_montada) {
                                        //printf("Umount\n");
                                        file_utils_umount_mmc_image();

                                        menu_generic_message_splash("Umount Image","Ok image has been unmounted");
                                    }


                                    //Aunque mount solo cuando se haya seleccionado archivo
                                    else  {
                                        //Si no es directorio
                                        if (tipo_archivo_seleccionado!=2) {
                                            //printf("Mount\n");
                                            if (!file_utils_mount_mmc_image(file_utils_file_selected)) {
                                                menu_generic_message_splash("Mount Image","Ok image has been mounted on 0:/");
                                            }
                                        }
                                    }
										
										                                    

                                    releer_directorio=1;
                                }	

                                //Sync mmc image
                                if (tecla=='S' && menu_mmc_image_montada) {
                                    if (menu_confirm_yesno_texto("Sync changes","Sure?")) menu_filesel_mmc_sync();                              
                                }	

								//Si no es directorio
								if (tipo_archivo_seleccionado!=2) {
									//unimos directorio y nombre archivo
									//getcwd(file_utils_file_selected,PATH_MAX);
									//sprintf(&file_utils_file_selected[strlen(file_utils_file_selected)],"/%s",item_seleccionado->d_name);
									
									//Visor de archivos
									if (tecla=='V') {
                                        menu_file_viewer_read_file("Text file view",file_utils_file_selected);
                                        menu_muestra_pending_error_message(); //Si se genera un error derivado del view
                                    }

									//Truncate
									if (tecla=='T') {
										if (menu_confirm_yesno_texto("Truncate","Sure?")) util_truncate_file(file_utils_file_selected);
									}


									//Filemem
									if (tecla=='F') {
										file_utils_file_mem_load(file_utils_file_selected);
									}

									//Convert
									if (tecla=='O') {
										file_utils_file_convert(file_utils_file_selected);
										releer_directorio=1;
									}

								

								}
							}

							
						}

						//Mkdir
						if (tecla=='K') {
							char string_carpeta[200];
							string_carpeta[0]=0;
							menu_ventana_scanf("Folder name",string_carpeta,200);
							if (string_carpeta[0]) {
                                zvfs_mkdir(string_carpeta);
								//menu_filesel_mkdir(string_carpeta);
								releer_directorio=1;
							}
						}


						//Paste text
						if (tecla=='P') {
							file_utils_paste_clipboard();
										
										
							releer_directorio=1;
						}
			
						

						//Redibujar ventana
						//releer_directorio=1;
						
						zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
						zxvision_menu_filesel_print_legend(ventana);

						zxvision_menu_filesel_print_text_contents(ventana);
					}
					
				}

				//menu_espera_no_tecla();
				menu_espera_no_tecla_con_repeticion();




			break;

			case 2:
				//zona filtros
				zxvision_reset_visible_cursor(ventana);
                                zxvision_menu_print_dir(filesel_archivo_seleccionado,ventana);

                                //para que haga lectura de los filtros
                                menu_speech_tecla_pulsada=0;

				zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
				zxvision_draw_window_contents(ventana);
	

				tecla=zxvision_common_getkey_refresh();


				if (menu_filesel_change_zone_if_clicked(ventana,&filesel_zona_pantalla,&tecla)) {
					zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
                                         releer_directorio=1;

                                                menu_espera_no_tecla();
				}

                //ESC
                else if (tecla==2) {
                                                
                    menu_espera_no_tecla();
                    zvfs_chdir(filesel_directorio_inicial);
                    menu_filesel_free_mem();

                    menu_filesel_preexit(ventana);
                    return 0;
                }

				//cambiar de zona con tab
				else if (tecla==15) {
					menu_reset_counters_tecla_repeticion();
					filesel_zona_pantalla=0;
					zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
					//no releer todos archivos
					menu_speech_tecla_pulsada=1;
				}


				else {

					//printf ("conmutar filtros\n");
					if (tecla || (tecla==0 && mouse_left)) { 

						//conmutar filtros
						menu_filesel_switch_filters();

					        zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
						releer_directorio=1;

						menu_espera_no_tecla();
					}
				}

			break;
			}

		} while (releer_directorio==0);
	} while (1);


	//Aqui no se va a llegar nunca


}



//Inicializar vacio
void last_filesused_clear(void)
{

	int i;
	for (i=0;i<MAX_LAST_FILESUSED;i++) {
		last_files_used_array[i][0]=0;
	}
}

//Desplazar hacia abajo desde posicion superior indicada. La posicion indicada sera un duplicado de la siguiente posicion por tanto
void lastfilesuser_scrolldown(int posicion_up,int posicion_down)
{
	int i;
	for (i=posicion_down;i>=posicion_up+1;i--) {
		strcpy(last_files_used_array[i],last_files_used_array[i-1]);
	}	
}

//Insertar entrada en last smartload
void last_filesused_insert(char *s)
{
	//Desplazar todos hacia abajo e insertar en posicion 0
	//Desde abajo a arriba

	//int i;
	/*for (i=MAX_LAST_FILESUSED-1;i>=1;i--) {
		strcpy(last_files_used_array[i],last_files_used_array[i-1]);
	}*/

	lastfilesuser_scrolldown(0,MAX_LAST_FILESUSED-1);


	//Meter en posicion 0
	strcpy(last_files_used_array[0],s);

	debug_printf (VERBOSE_INFO,"Inserting recent file %s at position 0",s);

	//printf ("Dump smartload:\n");

	//for (i=0;i<MAX_LAST_FILESUSED;i++) {
	//	printf ("Entry %d: [%s]\n",i,last_files_used_array[i]);
	//}
}



//
//----------------------------------------------
//Finish filesel
//
