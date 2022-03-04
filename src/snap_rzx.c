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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>


#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif

#include "cpu.h"
#include "snap_rzx.h"
#include "snap.h"
#include "debug.h"
#include "operaciones.h"
#include "utils.h"
#include "menu.h"
#include "screen.h"




z80_byte *rzx_file_mem=NULL;

int tamanyo_rzx=0;
int rzx_posicion_puntero=0;

int rzx_reproduciendo=0;

int rzx_elapsed_time=0;

int rzx_estimado_segundos;

//Princio del todo (o final de input recording) y hay que leer otro
int rzx_pendiente_leer_input_recording;

//Total frames del bloque de input recording
long int rzx_frames_input_recording;

//Total frames leidos del bloque de input recording
long int rzx_frames_input_recording_counter;

//Total lecturas i/o en el frame
z80_int rzx_in_reads_in_frame;

//Total lecturas realizadas en este frame
z80_int rzx_in_reads_in_frame_counter=0;

z80_byte rzx_get_byte(int posicion)
{
  if (posicion>=tamanyo_rzx) {
    debug_printf(VERBOSE_ERR,"Trying to read beyond rzx file. Asked: %d Size rzx: %d",posicion,tamanyo_rzx);
    eject_rzx_file();
    return 0;
  }

  return rzx_file_mem[posicion];
}

void eject_rzx_file(void)
{
  tamanyo_rzx=0;
  rzx_posicion_puntero=0;
  rzx_reproduciendo=0;
  rzx_delete_footer();
}

void rzx_create_temporary_file(char *extension, char *destino, int puntero, int longitud)
{
  sprintf (destino,"%s/tmp_rzxfile.%s",get_tmpdir_base(),extension);
  debug_printf (VERBOSE_INFO,"Creating temporary file %s",destino);

  FILE *ptr_destino;
  ptr_destino=fopen(destino,"wb");
  if (ptr_destino==NULL) {
          debug_printf (VERBOSE_ERR,"Error creating target file");
          return;
  }

  z80_byte byte_leido;
  for (;longitud;longitud--,puntero++) {
    byte_leido=rzx_get_byte(puntero);
    fwrite(&byte_leido,1,1,ptr_destino);
  }

  fclose(ptr_destino);

}


void rzx_create_temporary_gz_file(char *extension, char *nombre_base,char *destino, int puntero, int longitud)
{
  sprintf (destino,"%s/%s.%s.gz",get_tmpdir_base(),nombre_base,extension);
  debug_printf (VERBOSE_INFO,"Creating temporary file %s",destino);

  FILE *ptr_destino;
  ptr_destino=fopen(destino,"wb");
  if (ptr_destino==NULL) {
          debug_printf (VERBOSE_ERR,"Error creating target file");
          return;
  }

  //Escribir cabecera gz
  char gz_header[]="\x1f\x8b\x08\x00\x00\x00\x00\x00";
  fwrite(gz_header,1,8,ptr_destino);

  z80_byte byte_leido;
  for (;longitud;longitud--,puntero++) {
    byte_leido=rzx_get_byte(puntero);
    fwrite(&byte_leido,1,1,ptr_destino);
  }

  fclose(ptr_destino);

}

void rzx_load_z80_included_snapshot(char *s)
{
  //Llamamos a load_z80_snapshot en vez de snapshot_load para no alterar ultimo snapshot cargado
  load_z80_snapshot(s);
}

/*void load_rzx_snapshot(void)
{
  load_rzx_snapshot_file(snapfile);
}*/

void load_rzx_snapshot_file(char *archivo)
{

  eject_rzx_file();

  rzx_elapsed_time=0;
  rzx_estimado_segundos=0;

  tamanyo_rzx=get_file_size(archivo);

  if (tamanyo_rzx==0) {
    debug_printf(VERBOSE_ERR,"RZX file is empty");
    return;
  }

  //Asignar memoria. Desasignando antes si es necesario
  if (rzx_file_mem!=NULL) {
    debug_printf(VERBOSE_DEBUG,"Freeing old memory used to read rzx file");
    free(rzx_file_mem);
  }

  //Asignar
  rzx_file_mem=malloc(tamanyo_rzx);

  if (rzx_file_mem==NULL) cpu_panic("Error allocating memory to read RZX file");

  //Leer archivo

  FILE *ptr_rzxfile;
  ptr_rzxfile=fopen(archivo,"rb");

  if (!ptr_rzxfile) {
        debug_printf(VERBOSE_ERR,"Unable to open rzx file");
        eject_rzx_file();
        return;
  }

  int leidos=fread(rzx_file_mem,1,tamanyo_rzx,ptr_rzxfile);

  if (leidos!=tamanyo_rzx) {
    debug_printf(VERBOSE_ERR,"Error reading RZX file");
    eject_rzx_file();
  }

  debug_printf (VERBOSE_DEBUG,"Read %d bytes of RZX file",leidos);

  fclose(ptr_rzxfile);

  //Leer primero firma de archivo
  if (rzx_get_byte(0)!='R' ||
      rzx_get_byte(1)!='Z' ||
      rzx_get_byte(2)!='X' ||
      rzx_get_byte(3)!='!') {
        debug_printf(VERBOSE_ERR,"RZX header invalid");
        eject_rzx_file();
        return;
  }

  debug_printf (VERBOSE_INFO,"RZX file version %d.%d",rzx_get_byte(4),rzx_get_byte(5));
  //z80_byte rzx_flags=rzx_get_byte(6);

  rzx_posicion_puntero=10;

  //Gestionar tipos de ids
  z80_byte block_type;

  int final=0;
  z80_long_int aux_block_length;
  int i;

  while (!final) {
    block_type=rzx_get_byte(rzx_posicion_puntero);

    debug_printf (VERBOSE_DEBUG,"Block type %d",block_type);

    switch (block_type) {

      //Creator information block
      case 0x10:
        aux_block_length=rzx_get_byte(rzx_posicion_puntero+1) + 256*rzx_get_byte(rzx_posicion_puntero+2) +
                      65536*rzx_get_byte(rzx_posicion_puntero+3) + 16777216*rzx_get_byte(rzx_posicion_puntero+4);

        debug_printf (VERBOSE_DEBUG,"Creator information block. Length: %d",aux_block_length);
        char creator_name[20];

        for (i=0;i<20;i++) creator_name[i]=rzx_get_byte(rzx_posicion_puntero+5+i);

        debug_printf (VERBOSE_INFO,"Creator : %s. Version: %d.%d",creator_name,rzx_get_byte(rzx_posicion_puntero+25),rzx_get_byte(rzx_posicion_puntero+26) );

        rzx_posicion_puntero+=aux_block_length;
      break;


      //Snapshot block
      case 0x30:
        aux_block_length=rzx_get_byte(rzx_posicion_puntero+1) + 256*rzx_get_byte(rzx_posicion_puntero+2) +
                      65536*rzx_get_byte(rzx_posicion_puntero+3) + 16777216*rzx_get_byte(rzx_posicion_puntero+4);

        debug_printf (VERBOSE_DEBUG,"Snapshot block. Length: %d",aux_block_length);

        z80_byte snapshot_flags=rzx_get_byte(rzx_posicion_puntero+5);

        debug_printf (VERBOSE_DEBUG,"First snapshot flags byte: %d",snapshot_flags);

        char snapshot_extension[4];

        for (i=0;i<4;i++) snapshot_extension[i]=rzx_get_byte(rzx_posicion_puntero+9+i);

        debug_printf (VERBOSE_INFO,"Snapshot extension : %s",snapshot_extension );





        z80_long_int uncompressed_snapshot_length=rzx_get_byte(rzx_posicion_puntero+13) + 256*rzx_get_byte(rzx_posicion_puntero+14) +
                      65536*rzx_get_byte(rzx_posicion_puntero+15) + 16777216*rzx_get_byte(rzx_posicion_puntero+16);

        debug_printf (VERBOSE_DEBUG,"Uncompressed snapshot length: %d",uncompressed_snapshot_length);

        //Cargar snapshot a partir de rzx_posicion_puntero+0x11
        //De momento comprobar tipo snapshot
        if (!strcasecmp(snapshot_extension,"z80")) {
          debug_printf (VERBOSE_INFO,"Loading z80 snapshot");
          //Dado que actualmente la lectura de snap z80 lo hace mediante ruta a archivo, creamos archivo temporal
          //con contenido del snapshot contenido en el archivo rzx y se lo enviamos
          //Si esta comprimido

          char nombre_final[PATH_MAX];

          if (snapshot_flags&2) {

            //Suponemos descompresion con GZ
            rzx_create_temporary_gz_file("z80", "tmp_rzxfile", nombre_final, rzx_posicion_puntero+0x11, aux_block_length-17);

            //Y descomprimimos ese archivo
            char descomprimido[PATH_MAX];
            char tmpdir[PATH_MAX];
            sprintf (tmpdir,"%s",get_tmpdir_base());
            sprintf (descomprimido,"%s/rzx_snapshot.z80",tmpdir);

            if (uncompress_gz(nombre_final,descomprimido)) {
              debug_printf (VERBOSE_ERR,"RZX: Error uncompressing snapshot");
              eject_rzx_file();
              final=1;
            }



            //sprintf (tmpdir,"%s/%s",get_tmpdir_base(),archivo);
            rzx_load_z80_included_snapshot(descomprimido);
          }

          else {
            rzx_create_temporary_file("z80", nombre_final, rzx_posicion_puntero+0x11, aux_block_length-17);
            rzx_load_z80_included_snapshot(nombre_final);
          }
          }




        else {
          debug_printf (VERBOSE_ERR,"Unknown snapshot type %s",snapshot_extension);
          eject_rzx_file();
          final=1;
        }

        rzx_posicion_puntero+=aux_block_length;

      break;


      case 0x80:
        //Asumimos que a partir de aqui todo son frames 0x80. Reproducir juego
        final=1;
        rzx_pendiente_leer_input_recording=1;
        rzx_reproduciendo=1;
        rzx_in_reads_in_frame_counter=0;
      break;


      default:
        debug_printf (VERBOSE_ERR,"Unknown RZX block type %d",block_type);
        eject_rzx_file();
        final=1;
      break;
    }
  }



}


/*
Logica de operacion:

rzx_in_reads_in_frame_counter se pone a 0 al principio de cada frame, llamando a rzx_siguiente_frame_recording
Operacion de lectura de puerto, llama a rzx_lee_puerto
Si retorna 0, quiere decir que no devolvemos lectura puerto y se lee puerto de manera habitual
Si no retorna 0, retorna en valor_puerto valor de lectura puerto del frame.
Cuando el rzx_in_reads_in_frame_counter llega a rzx_in_reads_in_frame, devolvemos 0

Cuando hay cambio de frame, se salta a siguiente frame de recording.
Si siguiente frame de recording excede rzx_frames_input_recording, hay que leer nuevo bloque recording

Cada bloque recording puede estar comprimido. Al leerlo se guarda en un buffer de memoria aparte, que hay que asignar memoria


*/

z80_byte *rzx_io_block_mem=NULL;

//Memoria usada por el bloque de io
z80_long_int memory_to_assign_io_block;

int rzx_puntero_io_block=0;

void free_rzx_io_block_mem(void)
{
  if (rzx_io_block_mem!=NULL) {
    debug_printf(VERBOSE_DEBUG,"Freeing previous io block memory");
    free(rzx_io_block_mem);
  }
}

z80_byte rzx_get_io_block_byte(z80_long_int posicion)
{
  if (posicion>=memory_to_assign_io_block) {
    debug_printf(VERBOSE_ERR,"Trying to read beyond rzx file. Asked: %d Total memory: %d",posicion,memory_to_assign_io_block);
    eject_rzx_file();
    return 0;
  }

  return rzx_io_block_mem[posicion];
}



//Ultimo bloque con longitud diferente de 65535
int rzx_puntero_io_block_last=0;

//Longitud del frame del ultimo diferente de 65535
z80_int rzx_in_reads_in_frame_last;

//De donde lee la operacion de i/o
int rzx_puntero_io_block_lectura=0;


//Puntero al siguiente frame
int rzx_next_puntero_io_block=0;


//Estos dos realmente no los uso, aunque los leo y los incremento cuando toca
z80_int rzx_in_fetch_counter_til_next_int;
z80_int rzx_in_fetch_counter_til_next_int_counter;

void rzx_next_frame_recording(void)
{

  debug_printf(VERBOSE_DEBUG,"RZX: Begin rzx frame. Total frames in this input recording block: %d. Current frame: %d",rzx_frames_input_recording,rzx_frames_input_recording_counter);

  if (rzx_frames_input_recording_counter>=rzx_frames_input_recording) {
    debug_printf(VERBOSE_DEBUG,"End of frames in this input recording block. TODO: Be able to read next frame");
    eject_rzx_file();
    return;
  }

  rzx_puntero_io_block=rzx_next_puntero_io_block;


  rzx_in_fetch_counter_til_next_int=rzx_get_io_block_byte(rzx_puntero_io_block+0)+256*rzx_get_io_block_byte(rzx_puntero_io_block+1);

  debug_printf(VERBOSE_DEBUG,"RZX: Fetch counter til next interrupt: %d",rzx_in_fetch_counter_til_next_int);

  rzx_in_reads_in_frame=rzx_get_io_block_byte(rzx_puntero_io_block+2)+256*rzx_get_io_block_byte(rzx_puntero_io_block+3);

  rzx_puntero_io_block+=4;

  debug_printf(VERBOSE_DEBUG,"RZX: In reads in this frame: %d, pointer: %d",rzx_in_reads_in_frame,rzx_puntero_io_block);


  if (rzx_in_reads_in_frame==65535) {
    rzx_puntero_io_block_lectura=rzx_puntero_io_block_last;
    rzx_in_reads_in_frame=rzx_in_reads_in_frame_last;

    rzx_next_puntero_io_block=rzx_puntero_io_block;

    debug_printf(VERBOSE_DEBUG,"RZX: Repeated frame: In reads in this frame: %d, pointer: %d, read pointer: %d",rzx_in_reads_in_frame,rzx_puntero_io_block,rzx_puntero_io_block_lectura);
  }

  else {

      rzx_puntero_io_block_lectura=rzx_puntero_io_block;

      rzx_puntero_io_block_last=rzx_puntero_io_block;
      rzx_in_reads_in_frame_last=rzx_in_reads_in_frame;

      rzx_next_puntero_io_block=rzx_puntero_io_block+rzx_in_reads_in_frame;
  }

  rzx_in_reads_in_frame_counter=0;

  rzx_frames_input_recording_counter++;

  rzx_in_fetch_counter_til_next_int_counter=0;

}

z80_byte rzx_ultimo_puerto_retornado;

int rzx_lee_puerto(z80_byte *valor_puerto)
{
  /*
  //Princio del todo (o final de input recording) y hay que leer otro
  int rzx_pendiente_leer_input_recording;

  //Total frames del bloque de input recording
  long int rzx_frames_input_recording;

  //Total frames leidos del bloque de input recording
  long int rzx_frames_input_recording_counter;

  //Total lecturas i/o en el frame
  z80_int rzx_in_reads_in_frame;

  //Total lecturas realizadas en este frame
  z80_int rzx_in_reads_in_frame_counter=0;
  */


  if (rzx_pendiente_leer_input_recording) {
    //Leer input recording
    z80_byte block_type=rzx_get_byte(rzx_posicion_puntero);
    if (block_type!=0x80) {
      debug_printf(VERBOSE_ERR,"RZX: Can't support reading non 0x80 blocks when the first has appeared");
      eject_rzx_file();
      return 0;
    }


    z80_long_int compressed_iorecording_length=rzx_get_byte(rzx_posicion_puntero+1) + 256*rzx_get_byte(rzx_posicion_puntero+2) +
                  65536*rzx_get_byte(rzx_posicion_puntero+3) + 16777216*rzx_get_byte(rzx_posicion_puntero+4);

    rzx_frames_input_recording=rzx_get_byte(rzx_posicion_puntero+5) + 256*rzx_get_byte(rzx_posicion_puntero+6) +
                  65536*rzx_get_byte(rzx_posicion_puntero+7) + 16777216*rzx_get_byte(rzx_posicion_puntero+8);

  debug_printf(VERBOSE_DEBUG,"RZX: Total frames in this input recording block: %d",rzx_frames_input_recording);
  //sleep(3);


/*
  Este valor suele ser 0 o menor de 20. No tiene sentido usarlo
    z80_long_int rzx_t_estados_al_inicio=rzx_get_byte(rzx_posicion_puntero+0x0A) + 256*rzx_get_byte(rzx_posicion_puntero+0x0B) +
                  65536*rzx_get_byte(rzx_posicion_puntero+0x0C) + 16777216*rzx_get_byte(rzx_posicion_puntero+0x0D);

    printf ("t_estados al inicio: %d\n",rzx_t_estados_al_inicio);
    */

   z80_byte ioblock_flags=rzx_get_byte(rzx_posicion_puntero+0x0E);

   memory_to_assign_io_block=compressed_iorecording_length;

   //Si comprimido
   /*if (ioblock_flags&2) {
     //Cuanta memoria asignamos???
     memory_to_assign_io_block *=10;
   }*/

   //Si encriptado
   if (ioblock_flags&1) {
     debug_printf (VERBOSE_ERR,"RZX: Can not read encrypted block");
     eject_rzx_file();
     return 0;
   }

   //liberar bloque anterior
   free_rzx_io_block_mem();


   if (ioblock_flags&2) {
     //Leerlo comprimido
     char nombre_final[PATH_MAX];
     rzx_create_temporary_gz_file("raw", "tmp_rzxfile", nombre_final, rzx_posicion_puntero+0x12, compressed_iorecording_length-18);

     //Descomprimir

     char descomprimido[PATH_MAX];
     char tmpdir[PATH_MAX];
     sprintf (tmpdir,"%s",get_tmpdir_base());
     sprintf (descomprimido,"%s/uncompressed_rzx.raw",tmpdir);


     if (uncompress_gz(nombre_final,descomprimido)) {
       debug_printf (VERBOSE_ERR,"RZX: Error uncompressing data");
       eject_rzx_file();
       return 0;
     }

     //Leer archivo en memoria de rzx_io_block_mem
     memory_to_assign_io_block=get_file_size(descomprimido);

     rzx_io_block_mem=malloc(memory_to_assign_io_block);

     FILE *ptr_rzxfile;
     ptr_rzxfile=fopen(descomprimido,"rb");

     if (!ptr_rzxfile) {
           debug_printf(VERBOSE_ERR,"Unable to open rzx file");
           eject_rzx_file();
           return 0;
     }

     int leidos=fread(rzx_io_block_mem,1,memory_to_assign_io_block,ptr_rzxfile);


     debug_printf (VERBOSE_DEBUG,"RZX: Read %d bytes of RZX file",leidos);

     fclose(ptr_rzxfile);

  }

    else {
      debug_printf(VERBOSE_ERR,"RZX: Can not read uncompressed block");
      eject_rzx_file();
      return 0;
    }

    rzx_frames_input_recording_counter=0;
    rzx_puntero_io_block=0;
    rzx_next_puntero_io_block=0;

    rzx_next_frame_recording();


    rzx_pendiente_leer_input_recording=0;

  }


  debug_printf (VERBOSE_PARANOID,"RZX: Reading port. rzx_in_fetch_counter_til_next_int_counter: %d rzx_in_fetch_counter_til_next_int: %d",rzx_in_fetch_counter_til_next_int_counter,rzx_in_fetch_counter_til_next_int);

  if (rzx_in_reads_in_frame_counter>=rzx_in_reads_in_frame) {
    debug_printf (VERBOSE_PARANOID,"RZX: Reading port beyond frame. rzx_in_reads_in_frame_counter: %d rzx_in_reads_in_frame: %d",rzx_in_reads_in_frame_counter,rzx_in_reads_in_frame);

    //No hay suficiente info en este frame. Ir al siguiente frame
    while (rzx_in_reads_in_frame_counter>=rzx_in_reads_in_frame && rzx_reproduciendo) {
    //usleep(100000); //0.1 segundo
    rzx_next_frame_recording();
    }

    if (!rzx_reproduciendo) {
      return 0;
    }

    //*valor_puerto=0xFF;
    //return 1;


    //return rzx_ultimo_puerto_retornado;
  }

  z80_byte rzx_ultimo_puerto_retornado=rzx_get_io_block_byte(rzx_puntero_io_block_lectura++);

  debug_printf (VERBOSE_PARANOID,"RZX: Returning %02XH to port read",rzx_ultimo_puerto_retornado);

  //if (rzx_in_reads_in_frame!=65535) rzx_puntero_io_block++;



  *valor_puerto=rzx_ultimo_puerto_retornado;

  rzx_in_reads_in_frame_counter++;


  //Meter byte de i/o
  return 1;

}


void rzx_delete_footer(void)
{
                           //01234567890123456789012345678901
  menu_putstring_footer(0,2,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
  menu_footer_bottom_line();
}

void rzx_retorna_minutos_segundos(int segundos, int *m, int *s)
{
  *s=segundos%60;

  int min=segundos/60;

  if (min>99) min=99;

  *m=min;
}



void rzx_print_footer(void)
{
	if (rzx_reproduciendo) {

    //debug_printf(VERBOSE_DEBUG,"RZX: Begin rzx frame. Total frames in this input recording block: %d. Current frame: %d",rzx_frames_input_recording,rzx_frames_input_recording_counter);

    int total;

    if (rzx_frames_input_recording==0) return;

    //if (rzx_frames_input_recording==0) total=100;
    //else

    total=(rzx_frames_input_recording_counter*100)/rzx_frames_input_recording;

    if (total>100) total=100;

    int elapsed_min,elapsed_sec;
    rzx_retorna_minutos_segundos(rzx_elapsed_time,&elapsed_min,&elapsed_sec);

    //Calculo aproximado del total de segundos
    //Sabemos total frames, frame actual, segundos actuales
    //Teniendo un total de porcentaje de 1,
    //Si ha transcurrido un 0.1 en 5 segundos,tiempo total=5*10=5*(1/0.1)=elapsed*(1/porcentaje transcurrido)
    //porcentaje transcurrido=(frame_actual/total frames)
    //tiempo total=elapsed*(1/(frame_actual/total frames))=(elapsed*total frames)/frame actual
    //controlar division por 0 para frame actual=0


    //Recalcular tiempo estimado solo cada 10 segundos
    if ((rzx_elapsed_time%10)==0) {
      if (rzx_frames_input_recording_counter==0) rzx_estimado_segundos=60*99; //el maximo
      else {
        rzx_estimado_segundos=(rzx_elapsed_time*rzx_frames_input_recording)/rzx_frames_input_recording_counter;
      }
    }

    int est_sec,est_min;

    rzx_retorna_minutos_segundos(rzx_estimado_segundos,&est_min,&est_sec);

    char buffer_tiempo_estimado[6];
    //Escribir mm:ss pero el : parpadea

    //Hasta que no hayan pasado unos 10 segundos, no mostrar tiempo estimado
    if (rzx_elapsed_time>=10) sprintf (buffer_tiempo_estimado,"%02d:%02d",est_min,est_sec);
    else sprintf (buffer_tiempo_estimado,"%s","UNK");

    char buffer_texto[33];

    //Escribir mm:ss pero el : parpadea
    sprintf (buffer_texto,"RZX Playing: %02d%c%02d/%s (%d%%)",elapsed_min,(elapsed_sec&1 ? ':' : ' '),elapsed_sec,
            buffer_tiempo_estimado,total);
		//color inverso
		menu_putstring_footer(0,2,buffer_texto,WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);

    rzx_elapsed_time++;
	}
}
