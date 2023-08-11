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
   Snapshot to RAM functions
*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "cpu.h"
#include "debug.h"
#include "operaciones.h"

#include "snap_zsf.h"
#include "snap_ram.h"
#include "zxvision.h"
#include "autoselectoptions.h"




//Definir maximo 1000 slots para salvado automatico en RAM
//constante definida con : MAX_SNAPSHOTS_IN_RAM

//Esto se organiza en modo de lista, y cuando se llega al final, se sobreescribe el primero
struct s_snapshots_in_ram snapshots_in_ram[MAX_TOTAL_SNAPSHOTS_IN_RAM];

//primer elemento de la lista en snapshots. cuando se llena, se ira moviendo en adelante
int snapshots_in_ram_first_element=0;

//total de elementos en la lista
int snapshots_in_ram_total_elements=0;

//indice del elemento a escribir. Se podria deducir mediante el primero y el total, pero asi es mas facil usarlo
int snapshots_in_ram_index_to_write=0;

//total de snapshots que el usuario mantiene en ram
int snapshots_in_ram_maximum=MAX_TOTAL_SNAPSHOTS_IN_RAM;

int snapshot_in_ram_rewind_last_position;

int snapshot_in_ram_rewind_initialized=0;

//Contador desde pulsado rewind hasta que se libera la posicion
int snapshot_in_ram_enabled_timer=0;

int snapshot_in_ram_enabled_timer_timeout=10;

//contador para hacer el evento cada 50 frames
int snapshot_in_ram_frames_counter=0;

//cada cuantos segundos se escribe
int snapshot_in_ram_interval_seconds=1;

int snapshot_in_ram_interval_seconds_counter=0;

int snapshot_in_ram_rewind_initial_position=0;

//cuantos snapshots han pasado desde que se ha pulsado rewind. En cuanto llega mas alla del tamaño de la lista, no se puede seguir pulsando
//el boton de rewind
int snapshot_in_ram_rewind_cuantos_pasado=0;

//int snapshot_in_ram_pending_message_footer=0;

//char snapshot_in_ram_pending_message_footer_message[1000];

z80_bit snapshot_in_ram_enabled={0};


//Crea un snapshot en ram y retorna puntero asociado a dicha memoria asignada
//mete en *longitud la longitud de dicho snapshot
z80_byte *save_zsf_snapshot_to_ram(int *p_longitud)
{

    //Asignamos primero lo máximo pues no sabemos cuanto ocupara nuestro snapshot
    z80_byte *buffer_temp;
    buffer_temp=malloc(MAX_ZSF_SNAPSHOT_SIZE);
    if (buffer_temp==NULL) cpu_panic("Can not allocate memory for save_zsf_snapshot_to_ram");

    z80_byte *puntero=buffer_temp;
    int longitud;

    save_zsf_snapshot_file_mem(NULL,puntero,&longitud);

    debug_printf (VERBOSE_INFO,"Saving snapshot to ram. Length: %d",longitud);

    //Y luego asignamos ya la memoria definitiva y copiamos dicho snapshot

    z80_byte *buffer_final;
    buffer_final=malloc(longitud);
    if (buffer_final==NULL) cpu_panic("Can not allocate memory for save_zsf_snapshot_to_ram");

    memcpy(buffer_final,buffer_temp,longitud);

    *p_longitud=longitud;

    free(buffer_temp);



    return buffer_final;
}


//Inicializar todas variables
void snapshots_in_ram_reset(void)
{
    snapshots_in_ram_first_element=0;
    snapshots_in_ram_total_elements=0;
    snapshots_in_ram_index_to_write=0;
    snapshot_in_ram_rewind_initialized=0;
}

//Meter un snapshot en elemento de la lista
void snapshot_to_ram_element(int indice)
{
    z80_byte *puntero;

    int longitud;

    puntero=save_zsf_snapshot_to_ram(&longitud);

    snapshots_in_ram[indice].memoria=puntero;
    snapshots_in_ram[indice].longitud=longitud;

    //fecha grabacion
    //agregar hora, minutos, segundos
    time_t tiempo = time(NULL);
    struct tm tm = *localtime(&tiempo);

    snapshots_in_ram[indice].hora=tm.tm_hour;
    snapshots_in_ram[indice].minuto=tm.tm_min;
    snapshots_in_ram[indice].segundo=tm.tm_sec;

    //printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

//Incrementar el indice. Si llega al final, dar la vuelva
int snapshot_increment_index(int indice)
{
    indice++;
    if (indice==snapshots_in_ram_maximum) {
        //el primero (el mas antiguo) vuelve a ser el 0
        indice=0;
    }

    return indice;
}



//Agregar snapshot en siguiente elemento de la lista
void snapshot_add_in_ram(void)
{
    if (snapshot_in_ram_enabled.v==0) return;

    //Hacerlo 1 vez cada 50 frames
    snapshot_in_ram_frames_counter++;

    if ((snapshot_in_ram_frames_counter %50)!=0) return;

    snapshot_in_ram_interval_seconds_counter++;

    if (snapshot_in_ram_interval_seconds_counter<snapshot_in_ram_interval_seconds) return;

    snapshot_in_ram_interval_seconds_counter=0;

    //Indice de donde escribir el snapshot
    int indice_a_escribir;

    //Si esta llena la lista, habrá que liberar el mas antiguo
    if (snapshots_in_ram_total_elements==snapshots_in_ram_maximum) {
        z80_byte *memoria_liberar;
        memoria_liberar=snapshots_in_ram[snapshots_in_ram_first_element].memoria;
        debug_printf (VERBOSE_DEBUG,"Freeing oldest snapshot because the list is full");
        free(memoria_liberar);

        //Y el primero avanza
        snapshots_in_ram_first_element=snapshot_increment_index(snapshots_in_ram_first_element);

    }

    else {
        //Aun no esta lleno, hay 1 elemento mas
        snapshots_in_ram_total_elements++;
    }

    indice_a_escribir=snapshots_in_ram_index_to_write;

    //Y escribimos snapshot
    snapshot_to_ram_element(indice_a_escribir);

    //Y decimos el ultimo cual es
    snapshots_in_ram_index_to_write=snapshot_increment_index(snapshots_in_ram_index_to_write);

    //printf("snapshot_add_in_ram. after add. snapshots_in_ram_index_to_write %d snapshots_in_ram_total_elements %d snapshots_in_ram_first_element %d\n",
    //    snapshots_in_ram_index_to_write,snapshots_in_ram_total_elements,snapshots_in_ram_first_element);


    snapshot_in_ram_rewind_cuantos_pasado++;

}

//Retorna un indice a elemento snapshot dentro de la lista
//0=el mas antiguo
//retorna <0 si se va mas alla del tamaño escrito
int snapshot_in_ram_get_element(int indice)
{

    if (indice>=snapshots_in_ram_total_elements) {
        return -1;
    }

    //Si no esta lleno, es facil
    /*if (snapshots_in_ram_total_elements!=MAX_SNAPSHOTS_IN_RAM) {
        return indice;
    }*/

    //Y si esta lleno, dara la vuelta
    //snapshots_in_ram_first_element

    //Si le pedimos mas alla del array y da la vuelta
    if (snapshots_in_ram_first_element+indice>=snapshots_in_ram_total_elements) {
        int sumado=snapshots_in_ram_first_element+indice;

        int resto=sumado-snapshots_in_ram_maximum;
        return resto;
        //Ejemplo: primer elemento en 998. Pedimos el elemento numero 7
        //sumado=998+7=1005
        //resto=1005-1000=5
        //retorna 5
    }
    else {
        //no da la vuelta
        return snapshots_in_ram_first_element+indice;
    }
}

//Recupera un snapshot de ram. posicion=0 es el mas antiguo
//retorna <0 si error
int snapshot_in_ram_load(int posicion)
{
    int indice=snapshot_in_ram_get_element(posicion);

    if (indice<0) return -1;

    z80_byte *puntero_memoria;
    int longitud;

    puntero_memoria=snapshots_in_ram[indice].memoria;
    longitud=snapshots_in_ram[indice].longitud;

    debug_printf(VERBOSE_INFO,"Loading snapshot from RAM length: %d",longitud);

    load_zsf_snapshot_file_mem(NULL,puntero_memoria,longitud,0);

                    //z80_byte *temp_puntero;
                    //int temp_temp_longitud;
                //    load_zsf_snapshot_file_mem(NULL,temp_puntero,temp_temp_longitud,0);
                //}

    return 0;

}





//Accion de "rebobinar"
void snapshot_in_ram_rewind(void)
{

    if (snapshot_in_ram_enabled.v==0) {
        debug_printf(VERBOSE_ERR,"Snapshots in RAM are not enabled. Go to menu Snapshot->Snapshots to RAM and enable it");
        return;
    }

    if (!snapshot_in_ram_rewind_initialized) {
        //printf("We don't have a initial rewind position. Generating it\n");

        //Si no se ha generado ni el primero
        if (snapshots_in_ram_total_elements==0) {
            debug_printf (VERBOSE_INFO,"We haven't generated any snapshot yet");
            return;
        }

        //posicion es la actual-1
        //si total elementos=1, posicion actual=0
        int posicion_actual=snapshots_in_ram_total_elements-1;

        //Vamos al ultimo snapshot
        snapshot_in_ram_rewind_last_position=posicion_actual; //-1;

        //solo para mostrar este contador, pero no tiene uso real
        snapshot_in_ram_rewind_initial_position=snapshot_in_ram_rewind_last_position;

        snapshot_in_ram_rewind_initialized=1;


        snapshot_in_ram_rewind_cuantos_pasado=0;
    }

    //Esto sucedera cuando va pulsando rewind hasta ir mas alla del principio
    if (snapshot_in_ram_rewind_last_position<0) {
        debug_printf(VERBOSE_INFO,"Can't rewind beyond beginning");
        //snapshot_in_ram_rewind_initialized=0;
        return;
    }

    //Esto sucedera cuando haya escrito X snapshots despues de pulsar rewind, donde X es el total de la lista
    if (snapshot_in_ram_rewind_cuantos_pasado>=snapshots_in_ram_maximum) {
        debug_printf (VERBOSE_INFO,"Rewind snapshot reference has been overwritten, can not rewind");
        //snapshot_in_ram_rewind_initialized=0;
        return;
    }

    //Restaurar ese snapshot
    debug_printf(VERBOSE_INFO,"Restoring to snapshot number %d (0=oldest)",snapshot_in_ram_rewind_last_position);
    snapshot_in_ram_load(snapshot_in_ram_rewind_last_position);

    char buffer_mensaje[100];
    int indice=snapshot_in_ram_get_element(snapshot_in_ram_rewind_last_position);

    //Contar cuantos segundos atras sera eso
    int diferencia=snapshot_in_ram_rewind_initial_position-snapshot_in_ram_rewind_last_position;

    int segundos=(diferencia+1)*snapshot_in_ram_interval_seconds;

    sprintf(buffer_mensaje,"<<<< %d seconds (%02d:%02d:%02d)",segundos,snapshots_in_ram[indice].hora,snapshots_in_ram[indice].minuto,snapshots_in_ram[indice].segundo);


    //TODO: este splash solo se ve cuando se acciona por accion de joystick,
    //y no por F-key, porque al pulsar F-Key se abre y cierra el menu
    screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);


    //printf("%s\n",buffer_mensaje);

    put_footer_first_message(buffer_mensaje);

    //snapshot_in_ram_pending_message_footer=1;


    //Activar un timer, y cuando pasen 10 segundos, se reinicializara
    snapshot_in_ram_enabled_timer=snapshot_in_ram_enabled_timer_timeout;

    //Y el contador de rewind se decrementa, pues va desplazandose atras en el tiempo
    if (snapshot_in_ram_rewind_last_position>=0) snapshot_in_ram_rewind_last_position--;

    //Y esta mas lejos el snapshot de referencia, dado que hemos ido uno mas atras
    snapshot_in_ram_rewind_cuantos_pasado++;
}

//Accion de "avanzar" dentro de un previo rewind
void snapshot_in_ram_ffw(void)
{

    //TODO: al pulsar la primera vez FFW, restaura el mismo snapshot ultimo que se habia recuperado con rewind,
    //esto es una pijada, se podria corregir, pero tampoco afecta mucho

    if (snapshot_in_ram_enabled.v==0) {
        debug_printf(VERBOSE_ERR,"Snapshots in RAM are not enabled. Go to menu Snapshot->Snapshots to RAM and enable it");
        return;
    }

    if (!snapshot_in_ram_rewind_initialized) {
        debug_printf(VERBOSE_INFO,"We don't have a initial rewind position. Returning");

        return;
    }

    //Y el contador de rewind se incrementa, pues va desplazandose atras en el tiempo
    if (snapshot_in_ram_rewind_last_position<snapshots_in_ram_maximum-1) {
        snapshot_in_ram_rewind_last_position++;
    }

    //printf("snapshot_in_ram_rewind_last_position %d\n",snapshot_in_ram_rewind_last_position);

    //Esto sucedera cuando va pulsando ffw hasta ir mas alla del final
    if (snapshot_in_ram_rewind_last_position>snapshot_in_ram_rewind_initial_position) {
        debug_printf(VERBOSE_INFO,"Can't ffw beyond end");
        //snapshot_in_ram_rewind_initialized=0;
        return;
    }

    //Esto sucedera cuando haya escrito X snapshots despues de pulsar rewind, donde X es el total de la lista
    if (snapshot_in_ram_rewind_cuantos_pasado>=snapshots_in_ram_maximum) {
        debug_printf(VERBOSE_INFO,"Rewind snapshot reference has been overwritten, can not rewind");
        //snapshot_in_ram_rewind_initialized=0;
        return;
    }

    //Restaurar ese snapshot
    debug_printf(VERBOSE_INFO,"Restoring to snapshot number %d (0=oldest)",snapshot_in_ram_rewind_last_position);
    snapshot_in_ram_load(snapshot_in_ram_rewind_last_position);

    char buffer_mensaje[100];
    int indice=snapshot_in_ram_get_element(snapshot_in_ram_rewind_last_position);

    //Contar cuantos segundos atras sera eso
    int diferencia=snapshot_in_ram_rewind_initial_position-snapshot_in_ram_rewind_last_position;

    int segundos=(diferencia+1)*snapshot_in_ram_interval_seconds;

    sprintf(buffer_mensaje,"<<<< %d seconds (%02d:%02d:%02d)",segundos,snapshots_in_ram[indice].hora,snapshots_in_ram[indice].minuto,snapshots_in_ram[indice].segundo);


    //TODO: este splash solo se ve cuando se acciona por accion de joystick,
    //y no por F-key, porque al pulsar F-Key se abre y cierra el menu
    screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);

    //printf("%s\n",buffer_mensaje);

    put_footer_first_message(buffer_mensaje);

    //snapshot_in_ram_pending_message_footer=1;


    //Activar un timer, y cuando pasen 10 segundos, se reinicializara
    snapshot_in_ram_enabled_timer=snapshot_in_ram_enabled_timer_timeout;



    //Y esta mas cerca el snapshot de referencia, dado que hemos ido uno mas adelante
    snapshot_in_ram_rewind_cuantos_pasado--;
}



void snapshot_in_ram_rewind_timer(void)
{
    //temporizador despues de pulsar rewind. Al cabo de 10 segundos (por defecto) se resetea la posicion
    if (snapshot_in_ram_enabled_timer>=0) {
        snapshot_in_ram_enabled_timer--;
        if (snapshot_in_ram_enabled_timer==0) {
            snapshot_in_ram_rewind_initialized=0;
            debug_printf(VERBOSE_INFO,"Reseting rewind position after %d seconds",snapshot_in_ram_enabled_timer_timeout);
        }
    }
}