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
   Some routines for Text Adventures
*/

#include "textadventure.h"

#include "cpu.h"
#include "operaciones.h"
#include "debug.h"
#include "screen.h"
#include "compileoptions.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifndef MINGW
	//Para waitpid
	#include <sys/wait.h>
#endif




//Para spawn en windows
#ifdef MINGW
	#include <process.h>


#endif

char textimage_filter_program[PATH_MAX]="";

void textadv_location_desc_run_convert(void);

//para controlar el tiempo desde el borrado de pantalla hasta fin de descripcion localidad
int textadv_location_desc_counter=0;
//Maximo valor para ese contador a partir del cual se considera fin de localidad
int max_textadv_location_desc_counter=1000;

//Estado de la deteccion de localidad:
//0: esperando borrado
//1: se ha producido borrado, esperando a que se pida leer una tecla
#define TEXTADV_LOC_STATE_IDLE 0
#define TEXTADV_LOC_STATE_CLS  1
int textadv_location_desc_state=TEXTADV_LOC_STATE_IDLE;

//Para la detección de descripción de localidades
int textadv_location_desc_nested_id_poke_byte;
int textadv_location_desc_nested_id_poke_byte_no_time;
int textadv_location_desc_nested_id_peek_byte;
int textadv_location_desc_nested_id_peek_byte_no_time;
z80_bit textadv_location_desc_enabled={0};

//Total de conversiones realizadas, para llevar un conteo de los creditos consumidos de la api externa
int textadv_location_total_conversions=0;

//Contador de tiempo desde ultimo caracter recibido
int textadv_location_desc_no_char_counter=0;
//Maximo valor para ese contador a partir del cual se considera fin de localidad
//En juegos de paws por ejemplo (Juanito y su baloncito, o super lopez), es importante este parametro,
//porque meten texto en el dibujo y lo considera erroneamente como parte de localidad y podria acabar
//antes de leer todo el texto de localidad
int max_textadv_location_desc_no_char_counter=1000;

//agregar caracter que le llega desde chardetect
#define TEXTADV_LOCATION_MAX_DESCRIPTION 500
char textadv_location_text[TEXTADV_LOCATION_MAX_DESCRIPTION+1];
int textadv_location_text_index=0;
void textadv_location_add_char(z80_byte c)
{
    if (textadv_location_desc_enabled.v==0) return;

    //Solo si estamos en estado de recepcion texto de localidad
    if (textadv_location_desc_state==TEXTADV_LOC_STATE_IDLE) return;

    textadv_location_desc_no_char_counter=0;

    //si llega al limite
    if (textadv_location_text_index==TEXTADV_LOCATION_MAX_DESCRIPTION) {
        printf("Reached maximum description length\n");
        return;
    }   

    //Fitros de otros caracteres especiales
    switch (c) {
        case '\'':
        case '"':
        case '#':
        case '!':
        case '%':
        case '/':
            c=32;
        break;
    }
    
    //filtros de caracteres, se supone que aqui solo llegan caracteres imprimibles, pero por si acaso
    if (c>31 && c<127) {

        textadv_location_text[textadv_location_text_index++]=c;

    }

}



void textadv_location_desc_ended_description(void)
{

    //solo aceptarlo si ha pasado 1 segundo al menos
    if (textadv_location_desc_counter<max_textadv_location_desc_counter) {
        printf("\nDo not accept finish location description until some time passes\n");
        return;
    }

    //Aceptar un minimo de caracteres
    if (textadv_location_text_index<10) {
        printf("\nDo not accept finish location description until minimum text\n");
        return;
    }

    printf("no char counter: %d\n",textadv_location_desc_no_char_counter);

    //Aceptarlo solo cuando haya pasado un tiempo desde el ultimo caracter recibido
    //En milisegundos
    if (textadv_location_desc_no_char_counter<max_textadv_location_desc_no_char_counter) {
        printf("\nDo not accept finish location before certain time with no chars received\n");
        return;
    }

    //Si habia empezado una localidad, decir que se ha finalizado la localidad pues se pide tecla
    printf("\nFinish reading location description\n");

    textadv_location_text[textadv_location_text_index]=0;
    printf("\nLocation description: [%s]\n",textadv_location_text);

    //Avisar a la ventana de text adventure image que estamos recreando imagen
    menu_textadv_loc_image_tell_show_creating_image(); 

    textadv_location_desc_run_convert();

    textadv_location_desc_state=TEXTADV_LOC_STATE_IDLE;
}

int check_cls_display(void)
{
    

    //Ver la pantalla esta borrada
    //TODO: quiza solo comprobar primer tercio?
    int linea,x;

    //Ver si filas 1-7 (exceptuando fila 0 donde se situan el texto de localidad de daad) estan a 0

    for (linea=8;linea<64;linea++) {
        for (x=0;x<32;x++) {
            z80_int dir=16384+ (screen_addr_table[linea*32+x] & 8191);
            if (peek_byte_no_time(dir)!=0) return 0;
        }
    }

    return 1;
}

void handle_textadv_location_states(void)
{
    if (textadv_location_desc_state==TEXTADV_LOC_STATE_IDLE) {
        int borrado=check_cls_display();
        if (borrado) {
            printf("\nDisplay has been cleared\n");
            textadv_location_desc_state=1;
            textadv_location_desc_counter=0;
            textadv_location_text_index=0;
        }
    }


}

void textadv_location_timer_event(void)
{
    //Se llama aqui cada 20ms
    textadv_location_desc_counter+=20;

    //Contador de tiempo desde ultimo caracter recibido
    textadv_location_desc_no_char_counter +=20;
}



//Si se lee direccion de sistema donde se guarda la tecla
void handle_textadv_read_keyboard_memory(z80_int dir)
{
    //printf("read key mem\n");
    if (textadv_location_desc_state==TEXTADV_LOC_STATE_IDLE) return;

    //TODO: juegos de paws finalizan antes: Ended description reading from 23560 PC=8DE9H
    //quiza hacer que se haya leido un minimo de texto (al menos 10 caracteres?)

    if (reg_pc<16384) return;
    
    if (dir==0x5c00) {
        printf("\nEnded description reading from 0x5c00 PC=%XH\n",reg_pc);
        textadv_location_desc_ended_description();
    }

    //5c3b. bit 6 Set when a new key has been pressed
    if (dir==0x5c3b) {
        printf("\nEnded description reading from 0x5c3b PC=%XH\n",reg_pc);
        textadv_location_desc_ended_description();
    }    

    /*if (dir==0x5c08) {
        printf("\nEnded description reading from 0x5c08 PC=%XH\n",reg_pc);
        textadv_location_desc_ended_description();
    } */
}


//Se llama aqui cuando se lee puerto de teclado
void textadv_location_desc_read_keyboard_port(void)
{
    if (textadv_location_desc_state==TEXTADV_LOC_STATE_IDLE) return;

    //Si estan interrupciones habilitadas e im0/1, entendemos que se lee desde la rom y por tanto esto no indica que estemos
    //leyendo el prompt
    //if (iff1.v && (im_mode==0 || im_mode==1)) return;

    if (reg_pc<16384) return;

    printf("\nEnded description reading keyboard port PC=%XH\n",reg_pc);

    textadv_location_desc_ended_description();
}

z80_byte textadv_location_desc_poke_byte_no_time(z80_int dir,z80_byte valor)
{
	debug_nested_poke_byte_no_time_call_previous(textadv_location_desc_nested_id_poke_byte_no_time,dir,valor);

    handle_textadv_location_states();


	//Para que no se queje el compilador
	return 0;
}

z80_byte textadv_location_desc_poke_byte(z80_int dir,z80_byte valor)
{
	debug_nested_poke_byte_call_previous(textadv_location_desc_nested_id_poke_byte,dir,valor);

    handle_textadv_location_states();

	//Para que no se queje el compilador
	return 0;
}

z80_byte textadv_location_desc_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{
	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(textadv_location_desc_nested_id_peek_byte_no_time,dir);

    handle_textadv_read_keyboard_memory(dir);
    
    return valor_leido;

	
}

z80_byte textadv_location_desc_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{
	z80_byte valor_leido=debug_nested_peek_byte_call_previous(textadv_location_desc_nested_id_peek_byte,dir);

    handle_textadv_read_keyboard_memory(dir);

  return valor_leido;
  

}

//Establecer rutinas propias
void textadv_location_desc_set_peek_poke_functions(void)
{

    textadv_location_desc_nested_id_poke_byte=debug_nested_poke_byte_add(textadv_location_desc_poke_byte,"textadv_location_desc poke_byte");
    textadv_location_desc_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(textadv_location_desc_poke_byte_no_time,"textadv_location_desc poke_byte_no_time");
            textadv_location_desc_nested_id_peek_byte=debug_nested_peek_byte_add(textadv_location_desc_peek_byte,"textadv_location_desc peek_byte");
            textadv_location_desc_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(textadv_location_desc_peek_byte_no_time,"textadv_location_desc peek_byte_no_time");    

}

//Restaurar rutinas de textadv_location_desc
void textadv_location_desc_restore_peek_poke_functions(void)
{
		debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before textadv_location_desc");
                //poke_byte=textadv_location_desc_original_poke_byte;
                //poke_byte_no_time=textadv_location_desc_original_poke_byte_no_time;
                //peek_byte=textadv_location_desc_original_peek_byte;
                //peek_byte_no_time=textadv_location_desc_original_peek_byte_no_time;

    debug_nested_poke_byte_del(textadv_location_desc_nested_id_poke_byte);
    debug_nested_poke_byte_no_time_del(textadv_location_desc_nested_id_poke_byte_no_time);
        debug_nested_peek_byte_del(textadv_location_desc_nested_id_peek_byte);
        debug_nested_peek_byte_no_time_del(textadv_location_desc_nested_id_peek_byte_no_time);    


}

void textadv_location_desc_enable(void)
{
    if (textadv_location_desc_enabled.v) return;

    textadv_location_desc_set_peek_poke_functions();
    textadv_location_desc_enabled.v=1;
}

void textadv_location_desc_disable(void)
{
    if (textadv_location_desc_enabled.v==0) return;

    textadv_location_desc_restore_peek_poke_functions();
    textadv_location_desc_enabled.v=0;
}


void textadv_location_desc_mete_comillas(char *origen,char *destino)
{
	sprintf (destino,"\"%s\"",origen);
}

int proceso_hijo_text_convert;

void textadv_location_desc_run_convert(void)
{

  
    if (textimage_filter_program[0]==0) return;

    //Incrementar contador de conversiones realizadas
    textadv_location_total_conversions++;

#ifndef MINGW



   
    //printf("Launching child process\n");
    proceso_hijo_text_convert = fork();


    switch (proceso_hijo_text_convert) {

        case -1:
            debug_printf (VERBOSE_ERR,"Can not run fork to text to image");
        break;

        case 0:
      
            execlp(textimage_filter_program,textimage_filter_program,textadv_location_text,NULL);

            //Si se llega aqui es que ha habido un error al executar programa filtro
            exit(0);
        break;

        default:

           //TODO: de momento no esperar hijo, para no detener juego

           /*
                    printf("Wait for text filter child\n");
                    waitpid (proceso_hijo_text_convert, NULL, 0);
         

            printf("despues de waitpid\n");
            */


                
        break;

    }

#else


	all_interlace_scr_refresca_pantalla();

	

    //En caso de Windows esto simplemente dice que hay hijo pero no el pid concreto
    proceso_hijo_text_convert=1;




    //Por defecto no esperar
    int modo=P_NOWAIT;

    //if (esperarhijo) modo=P_WAIT;



    //Parametro 1 es la descripcion de la localidad


    //importante las comillas cuando hay rutas con espacios
    //Al script de windows le llegan las comillas tal cual,
    //por tanto los parametros en un .bat de windows se deben usar tal cual %1 y no "%1", sino le meteria doble comillas ""%1""
    //+2 para meter las comillas
    char parametro_programa[PATH_MAX];
    char parametro_uno[TEXTADV_LOCATION_MAX_DESCRIPTION+2];


    //parametro programa sin comillas, porque sino, no inicia ni tan siquiera programa sin espacios
    sprintf (parametro_programa,"%s",textimage_filter_program);



    //Esto si que es necesario para poder enviar la descripcion de la localidad
    textadv_location_desc_mete_comillas(textadv_location_text,parametro_uno);



    //con spawnl
    int resultado=spawnl(modo, parametro_programa, parametro_programa, parametro_uno, NULL);
    debug_printf (VERBOSE_DEBUG,"Running program %s with parameters %s and %s",parametro_programa,parametro_uno);
    printf ("Running program %s with parameters %s and %s\n",parametro_programa,parametro_uno);

    //printf ("Resultado spawn: %d\n",resultado);
    if (resultado<0) {
        debug_printf (VERBOSE_DEBUG,"Error running text to image program");

    }





	
#endif

}


//Mira si la ruta al script tiene espacios y en ese caso da error y desactiva la ruta
//En sistemas no Windows no hace nada
int textimage_filter_program_check_spaces(void)
{
#ifdef MINGW
	int i;
	int tiene_espacios=0;
	for (i=0;textimage_filter_program[i];i++) {
		if (textimage_filter_program[i]==' ') {
			tiene_espacios=1;
			break;
		}
	}

	if (tiene_espacios) {
		debug_printf (VERBOSE_ERR,"Full path to Text to Image program %s has spaces. It won't work on Windows.",
			textimage_filter_program);
		return 1;
	}
#endif

return 0;
}
