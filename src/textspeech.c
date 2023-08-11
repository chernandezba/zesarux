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

#include "textspeech.h"

#include "cpu.h"
#include "operaciones.h"
#include "debug.h"
#include "screen.h"
#include "chardetect.h"
#include "zxvision.h"
#include "utils.h"
#include "sam.h"
#include "chardevice.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>


//para fcntl
#include <fcntl.h>

#ifndef MINGW
	//Para waitpid
	#include <sys/wait.h>
#endif




//Para spawn en windows
#ifdef MINGW
	#include <process.h>


#endif


//programa para pasar texto a speech
//NOTA: Aunque las funciones se llamen speech, el programa de filtro puede ser cualquier cosa,
//aunque esta pensado para speech
char *textspeech_filter_program=NULL;


//programa encargado de detener el proceso de speech
char *textspeech_stop_filter_program=NULL;



//se pone a 0 cada vez que se envia speech
//se incrementa caad 1/50
int scrtextspeech_filter_counter=0;

//si hay que esperar a cada child y para el emulador hasta que finaliza
z80_bit textspeech_filter_program_wait={0};



//Linea actual en buffer
//+1 para guardar el 0 del final
char buffer_speech[MAX_BUFFER_SPEECH+1];
int index_buffer_speech=0;


//Total lineas en buffer
#define MAX_LINES_BUFFER 100
char buffer_speech_lineas[MAX_LINES_BUFFER][MAX_BUFFER_SPEECH+1];
int fifo_buffer_speech_read=0;
int fifo_buffer_speech_write=0;
int fifo_buffer_speech_size=0;


pid_t proceso_hijo_speech=0;

pid_t proceso_stop_filtro=0;

//Tiempo en segundos para enviar texto aunque no se reciba salto de linea. 0=nunca
int textspeech_timeout_no_enter=3;


int textspeech_operating_counter=0;

//Enviar tambien el texto que genera el menu a rutinas de Filter text program
z80_bit textspeech_also_send_menu={0};

//Si el texto de vuelta del script de speech, se envia tambien a ventana debug console
z80_bit textspeech_get_stdout={0};


//Rutinas que solo existen en Windows
#ifdef MINGW


char speech_windows_lock_file[PATH_MAX]="";
char *get_speech_windows_lock_file(void)
{
	//Si esta vacio, hacer el sprintf la primera vez solo
	if (speech_windows_lock_file[0]==0) {
		sprintf (speech_windows_lock_file,"%s\\zesarux_temp_speech_running.lock",get_tmpdir_base() );
		debug_printf (VERBOSE_DEBUG,"Getting first time speech_windows_lock_file: %s",speech_windows_lock_file);
	}
	return speech_windows_lock_file;
}


char speech_windows_text_file[PATH_MAX]="";
char *get_speech_windows_text_file(void)
{
	//Si esta vacio, hacer el sprintf la primera vez solo
        if (speech_windows_text_file[0]==0) {
		sprintf (speech_windows_text_file,"%s\\zesarux_temp_speechfile.out",get_tmpdir_base() );
		debug_printf (VERBOSE_DEBUG,"Getting first time speech_windows_text_file: %s",speech_windows_text_file);
	}
	return speech_windows_text_file;
}


char speech_windows_stdout_file[PATH_MAX]="";
char *get_speech_windows_stdout_file(void)
{
	//Si esta vacio, hacer el sprintf la primera vez solo
	if (speech_windows_stdout_file[0]==0) {
		sprintf (speech_windows_stdout_file,"%s\\zesarux_temp_speech_stdout.out",get_tmpdir_base() );
		debug_printf (VERBOSE_DEBUG,"Getting first time speech_windows_stdout_file: %s",speech_windows_stdout_file);
	}
	return speech_windows_stdout_file;
}


void textspeech_mete_comillas(char *origen,char *destino)
{
	sprintf (destino,"\"%s\"",origen);
}

void textspeech_cambia_barra(char *texto)
{

    int i;
    char c;
    for (i=0;texto[i];i++) {
            c=texto[i];
            if (c=='/') texto[i]='\\';
    }

}


#endif


void textspeech_ver_si_stop_finalizado(void)
{
#ifndef MINGW
    //Recogemos valor de retorno de proceso stop speech
    if (textspeech_stop_filter_program!=NULL) {

		//printf ("esperar a que stop script acabe\n");


		//pausas de 100ms, 10 veces. Total espera: 1 segundo
		int contador_timeout=10;

		while (proceso_stop_filtro!=0 && contador_timeout) {
                        int status;
                        if (waitpid (proceso_stop_filtro, &status, WNOHANG)) {
                                //ha finalizado
                                //if (WIFEXITED(status)) {
                                //printf (" ha finalizado\n");
                                proceso_stop_filtro=0;
                        }
                        else {
                                //no ha finalizado
				//esperar 100ms
				debug_printf (VERBOSE_DEBUG,"Stop Script Filter not ended. Waiting");
				usleep(100*1000);
                        }
			contador_timeout--;
                }

		if (proceso_stop_filtro) {
			debug_printf (VERBOSE_DEBUG,"Stop Script Filter not ended after timeout. It can generate Zombie processes");
		}

    }
#endif
}


void textspeech_empty_speech_fifo(void)
{

	if (textspeech_filter_program==NULL) return;

        fifo_buffer_speech_size=0;
        fifo_buffer_speech_read=0;
        fifo_buffer_speech_write=0;


	//Y finalizamos proceso activo. Si hay stop script definido
	if (textspeech_stop_filter_program==NULL) return;

#ifndef MINGW

	textspeech_ver_si_stop_finalizado();

    proceso_stop_filtro = fork();


    switch (proceso_stop_filtro) {

            case -1:
                debug_printf (VERBOSE_ERR,"Can not run fork to stop program");
            break;

            case 0:
                execlp(textspeech_stop_filter_program,textspeech_stop_filter_program,NULL);

                //Si se llega aqui es que ha habido un error al executar programa filtro
                exit(0);
            break;

            default:
                    //Aqui esta el proceso padre. No hacer nada
            break;

    }

#else



	char parametro_programa[PATH_MAX];

	//Por defecto no esperar
	int modo=P_NOWAIT;


	//parametro programa tal cual sin comillas, porque sino, no inicia ni tan siquiera programa sin espacios
	sprintf (parametro_programa,"%s",textspeech_stop_filter_program);


	int resultado=spawnl(modo, parametro_programa, parametro_programa, NULL);
	debug_printf (VERBOSE_DEBUG,"Running program %s",parametro_programa);


#endif

}


//devuelve 0 si aun no ha finalizado
int textspeech_finalizado_hijo_speech(void)
{

#ifndef MINGW


    if (proceso_hijo_speech!=0) {
        int status;
        if (waitpid (proceso_hijo_speech, &status, WNOHANG)) {

            //if (WIFEXITED(status)) {
            //printf ("child ha finalizado\n");
            proceso_hijo_speech=0;
            return 1;
        }

        else {
            //printf ("child no ha finalizado\n");
            return 0;
        }
    }


#else
//Para Windows
    if (proceso_hijo_speech!=0) {
        if (!si_existe_archivo(get_speech_windows_lock_file()) ) {

            //printf ("child ha finalizado\n");
            proceso_hijo_speech=0;
            return 1;
        }

        else {
            //printf ("child no ha finalizado\n");
            return 0;
        }
    }



#endif

    //no hay proceso hijo. volver
    return 1;

}



//En sistemas no Windows no hace nada
void textspeech_borrar_archivo_windows_lock_file(void)
{

//Solo en Windows
#ifdef MINGW
	unlink(get_speech_windows_lock_file());
#endif

}

//En sistemas no Windows no hace nada
void textspeech_borrar_archivo_windows_speech_file(void)
{

//Solo en Windows
#ifdef MINGW
	unlink(get_speech_windows_text_file());
#endif

}






//Mira si la ruta al script tiene espacios y en ese caso da error y desactiva la ruta
//En sistemas no Windows no hace nada
void textspeech_filter_program_check_spaces(void)
{
#ifdef MINGW
	int i;
	int tiene_espacios=0;
	for (i=0;textspeech_filter_program[i];i++) {
		if (textspeech_filter_program[i]==' ') {
			tiene_espacios=1;
			break;
		}
	}

	if (tiene_espacios) {
		debug_printf (VERBOSE_ERR,"Full path to Text to Speech program %s has spaces. It won't work on Windows.",
			textspeech_filter_program);
		textspeech_filter_program=NULL;
	}
#endif
}


//Mira si la ruta al script de paro de speech tiene espacios y en ese caso da error y desactiva la ruta
//En sistemas no Windows no hace nada
void textspeech_stop_filter_program_check_spaces(void)
{
#ifdef MINGW
    int i;
    int tiene_espacios=0;
    for (i=0;textspeech_stop_filter_program[i];i++) {
        if (textspeech_stop_filter_program[i]==' ') {
            tiene_espacios=1;
            break;
        }
    }

    if (tiene_espacios) {
        debug_printf (VERBOSE_ERR,"Full path to Text to Speech Stop program %s has spaces. It won't work on Windows.",
                textspeech_stop_filter_program);
        textspeech_stop_filter_program=NULL;
    }
#endif
}


void set_nonblock_flag(int desc)
{
#ifndef MINGW
    int oldflags = fcntl(desc, F_GETFL, 0);
    if (oldflags == -1) return;

    oldflags |= O_NONBLOCK;

    fcntl(desc, F_SETFL, oldflags);
#endif
}

//para capturar la salida, pipes que se crean una sola vez y luego se leen siempre
int textspeech_fds_output[2];

int textspeech_fds_output_initialized=0;


//Ver si hay texto en la pipe
//retorna 1 si habia salida
int textspeech_get_stdout_childs(void)
{
    //printf("start textspeech_get_stdout_childs\n");



    //printf("start2 textspeech_get_stdout_childs\n");

    if (textspeech_get_stdout.v) {

#ifdef MINGW
    //En Windows, leemos stdout de un archivo, siempre que tenga longitud >0
    if (si_existe_archivo(get_speech_windows_stdout_file())) {
        long long int count=get_file_size(get_speech_windows_stdout_file());
        if (count>0) {
            //leemos archivo
            char buffer[4096];

            if (count>4095) count=4095;
            lee_archivo(get_speech_windows_stdout_file(),buffer,count);

            //y borrar dicho archivo de stdout
            unlink(get_speech_windows_stdout_file());

            buffer[count]=0;
            //Si final caracter es 10 13, eliminarlo
            if (buffer[count-1]==10 || buffer[count-1]==13) buffer[count-1]=0;
            //windows puede acabar con los dos, por tanto mirar hacia atras tambien
            if (count>1) {
                if (buffer[count-2]==10 || buffer[count-2]==13) buffer[count-2]=0;
            }
            debug_printf(VERBOSE_ONLY_DEBUG_CONSOLE_WINDOW,"%s",buffer);

            return 1;
        }
    }

#else
        //volver si no se ha inicializado las pipes
        if (!textspeech_fds_output_initialized) return 0;

        int status=chardevice_status(textspeech_fds_output[0]);

        if (status & CHDEV_ST_RD_AVAIL_DATA) {

            //printf("Getting data from child stdout\n");
            char buffer[4096];

            ssize_t count = read(textspeech_fds_output[0], buffer, sizeof(buffer));

            //mostrar el texto hacia la consola de debug window
            //poner 0 al final
            if (count>0) {
                buffer[count]=0;
                //Si final caracter es 10 13, eliminarlo
                if (buffer[count-1]==10 || buffer[count-1]==13) buffer[count-1]=0;
                debug_printf(VERBOSE_ONLY_DEBUG_CONSOLE_WINDOW,"%s",buffer);
            }


            return 1;
        }
#endif

    }

    return 0;
}



void scrtextspeech_filter_run_pending(void)
{

    if (textspeech_filter_program==NULL) return;

    if (fifo_buffer_speech_size==0) return;

    int longit;
	longit=strlen(buffer_speech_lineas[fifo_buffer_speech_read]);

	debug_printf (VERBOSE_DEBUG,"Run pending text filter with text: %s",buffer_speech_lineas[fifo_buffer_speech_read]);

	if (longit>0) {
		textspeech_operating_counter=3;
		textspeech_print_operating();
	}

    int esperarhijo=0;
    if (fifo_buffer_speech_size==MAX_LINES_BUFFER) esperarhijo=1;
    if (textspeech_filter_program_wait.v) esperarhijo=1;

#ifndef MINGW

    int fds[2];

    if (pipe(fds)<0) {
            debug_printf (VERBOSE_ERR,"Can not make pipe to speech for sending text");
            return;
    }

    //para capturar la salida
    //int fds_output[2];
    if (textspeech_get_stdout.v) {
        if (!textspeech_fds_output_initialized) {

            if (pipe(textspeech_fds_output)<0) {
                    debug_printf (VERBOSE_ERR,"Can not make pipe to speech for receiving text");
                    return;
            }
            set_nonblock_flag(textspeech_fds_output[0]);
            set_nonblock_flag(textspeech_fds_output[1]);

            textspeech_fds_output_initialized=1;
        }
    }


    //printf("Launching child process\n");
    proceso_hijo_speech = fork();


    switch (proceso_hijo_speech) {

        case -1:
            debug_printf (VERBOSE_ERR,"Can not run fork to speech");
        break;

        case 0:
            close (0);
            dup (fds[0]);
            close(fds[1]);

            //para capturar el stdout
            if (textspeech_get_stdout.v) {
                dup2(textspeech_fds_output[1], STDOUT_FILENO);
                close(textspeech_fds_output[1]);
            }

            execlp(textspeech_filter_program,textspeech_filter_program,NULL);

            //Si se llega aqui es que ha habido un error al executar programa filtro
            exit(0);
        break;

        default:
            close(fds[0]);
            //longit=strlen(buffer_speech_lineas[fifo_buffer_speech_read]);
            write(fds[1],buffer_speech_lineas[fifo_buffer_speech_read],longit);
            close(fds[1]);

            fifo_buffer_speech_read++;
            if (fifo_buffer_speech_read==MAX_LINES_BUFFER) fifo_buffer_speech_read=0;

            if (fifo_buffer_speech_size>=0) fifo_buffer_speech_size--;

            //mantengo las pipes abiertas siempre
            //if (textspeech_get_stdout.v) close(fds_output[1]);



            //printf("antes de waitpid\n");

            if (esperarhijo) {
                    debug_printf (VERBOSE_DEBUG,"Wait for text filter child");
                    waitpid (proceso_hijo_speech, NULL, 0);
            }

            //printf("despues de waitpid\n");


        break;

    }

#else

	all_interlace_scr_refresca_pantalla();

	//En windows enviarle texto como primer parametro
	char buffer_texto[MAX_BUFFER_SPEECH+1];

	int i;
	if (longit>0) {
		for (i=0;i<longit;i++) {
			buffer_texto[i]=buffer_speech_lineas[fifo_buffer_speech_read][i];
		}
		buffer_texto[i]=0;

		//char buffer_comando[MAX_BUFFER_SPEECH+1+PATH_MAX];
		//sprintf (buffer_comando,"%s \"%s\"",textspeech_filter_program,buffer_texto);

		//debug_printf (VERBOSE_DEBUG,"Running command: %s",buffer_comando);

		//system(buffer_comando);
/*
mode:
P_OVERLAY 	Overlays parent process with child, which destroys the parent. This has the same effect as the exec* functions.
P_WAIT 	Suspends parent process until the child process has finished executing (synchronous spawn).
P_NOWAIT, P_NOWAITO 	Continues to execute calling process concurrently with new process (asynchronous spawn).
P_DETACH 	the child is run in background without access to the console or keyboard. Calls to _cwait upon the new process will fail (asynchronous spawn)
*/


		//En caso de Windows esto simplemente dice que hay hijo pero no el pid concreto
		proceso_hijo_speech=1;

		//Creamos archivo de lock
        FILE *ptr_lockfile;
        ptr_lockfile=fopen(get_speech_windows_lock_file(),"wb");

        if (ptr_lockfile==NULL) 	{
                debug_printf (VERBOSE_ERR,"Error writing lockfile %s",get_speech_windows_lock_file());
        }

        else {
			fclose(ptr_lockfile);
		}


		//Crear archivo tempspeechfile.out con texto para reproducir
        FILE *ptr_speechfile;
        ptr_speechfile=fopen(get_speech_windows_text_file(),"wb");

        if (ptr_speechfile==NULL)         {
            debug_printf (VERBOSE_ERR,"Error writing speechfile %s",get_speech_windows_text_file());
        }

        else {
            fwrite(buffer_texto, 1, longit, ptr_speechfile);
            fclose(ptr_speechfile);
        }



		//Por defecto no esperar
		int modo=P_NOWAIT;

		if (esperarhijo) modo=P_WAIT;



		//Parametro 1 es la ruta al archivo de speech
		//Parametro 2 es la ruta al archivo de lock
        //Parametro 3 es la ruta al archivo de stdout. El script debe escribir ahi el texto que se quiera de vuelta para el emulador,
        //por ejemplo texto traducido que se puede mostrar en debug console window

		//importante las comillas cuando hay rutas con espacios
		//Al script de windows le llegan las comillas tal cual,
		//por tanto los parametros en un .bat de windows se deben usar tal cual %1 y no "%1", sino le meteria doble comillas ""%1""
		//+2 para meter las comillas
		char parametro_programa[PATH_MAX];
		char parametro_uno[PATH_MAX+2];
		char parametro_dos[PATH_MAX+2];
        char parametro_tres[PATH_MAX+2];

		//parametro programa sin comillas, porque sino, no inicia ni tan siquiera programa sin espacios
		sprintf (parametro_programa,"%s",textspeech_filter_program);

		//cambiamos barra / por contrabarra aunque parece que no es necesario
		//textspeech_cambia_barra(parametro_programa);

		//cambiamos barra / por contrabarra aunque parece que no es necesario
		//textspeech_cambia_barra(textspeech_filter_program);


		//Esto si que es necesario para poder enviar la ruta a archivos de speech y temporales cuando tienen espacios
		textspeech_mete_comillas(get_speech_windows_text_file(),parametro_uno);
		textspeech_mete_comillas(get_speech_windows_lock_file(),parametro_dos);
        textspeech_mete_comillas(get_speech_windows_stdout_file(),parametro_tres);


		//con spawnl
		int resultado=spawnl(modo, parametro_programa, parametro_programa, parametro_uno, parametro_dos, parametro_tres, NULL);
		debug_printf (VERBOSE_DEBUG,"Running program %s with parameters %s and %s",parametro_programa,parametro_uno,parametro_dos, parametro_tres);

		//printf ("Resultado spawn: %d\n",resultado);
		if (resultado<0) {
			debug_printf (VERBOSE_DEBUG,"Error running speech program");
			//Borrar archivo de lock
			textspeech_borrar_archivo_windows_lock_file();

			proceso_hijo_speech=0;
		}


	}


    fifo_buffer_speech_read++;
    if (fifo_buffer_speech_read==MAX_LINES_BUFFER) fifo_buffer_speech_read=0;

    if (fifo_buffer_speech_size>=0) fifo_buffer_speech_size--;


#endif

}

void textspeech_add_speech_fifo_filter_unknown(void)
{
	int i;
	unsigned char c;

	for (i=0;buffer_speech[i];i++) {
		c=buffer_speech[i];
		if (c<32 || c>126 || c=='^' || c=='~') buffer_speech[i]=' ';
	}
}

void textspeech_add_speech_fifo(void)
{

    if (textspeech_filter_program==NULL) return;

    //Filtrar de la cadena de speech caracteres <32 o >126 u otros
    textspeech_add_speech_fifo_filter_unknown();


    //printf ("buffer fifo size: %d\n",fifo_buffer_speech_size);

    //printf ("enviando %s\n",buffer_speech);

    scrtextspeech_filter_counter=0;


    //Meter texto en fifo. Si hay espacio
    if (fifo_buffer_speech_size<MAX_LINES_BUFFER) {
        int longitud=index_buffer_speech;
        buffer_speech[longitud]=0;
        index_buffer_speech=0;
        sprintf (buffer_speech_lineas[fifo_buffer_speech_write],"%s",buffer_speech);


        //Avanzar puntero escritura
        fifo_buffer_speech_write++;
        if (fifo_buffer_speech_write==MAX_LINES_BUFFER) fifo_buffer_speech_write=0;

        fifo_buffer_speech_size++;

        return;

    }

    else {
                //Si no hay espacio en la fifo, esperar
		//no mostrar mensaje mediante debug_printf dado que esto, en stdout y si esta verbose 3 al menos,
		//generaria un mensaje a enviar a speech, y volveriamos a entrar aqui, de manera recursiva,
		//hasta que peta con segmentation fault (porque se ha llenado el stack de llamadas)
        printf ("Text to Speech filter fifo full\n");
    }



}


//Usado en funciones de print, para que hagan speech
void textspeech_print_speech(char *texto)
{
	if (textspeech_filter_program==NULL) return;
    //fflush(stdout);
    index_buffer_speech=strlen(texto);


	//printf ("longitud speech: %d\n",index_buffer_speech);

    if (index_buffer_speech>MAX_BUFFER_SPEECH) {
            sprintf (buffer_speech,"%s","Sorry, text is too large to for the text filter");
            index_buffer_speech=strlen(buffer_speech);
    }

    else {

        //Si es todo espacios sin ningun caracter, no enviar
        int vacio=1;
        int indice;
        for (indice=0;texto[indice]!=0;indice++) {
        //printf ("%d",texto[indice]);
                if (texto[indice]!=' ' && texto[indice]!=10) {
                        vacio=0;
                        break;
                }
        }

        if (vacio) {
            debug_printf (VERBOSE_DEBUG,"Contents sent to textspeech_print_speech is blank. Do not send");
            return;
        }

        sprintf (buffer_speech,"%s",texto);
    }


    textspeech_add_speech_fifo();
}



void textspeech_add_character(z80_byte c)
{

	if (textspeech_filter_program==NULL) return;

	//Si codigo 8 de retroceso
	if (c==8) {

		if (index_buffer_speech) {
			index_buffer_speech--;
		}

		else debug_printf(VERBOSE_DEBUG,"Received char 8 but we are at the beginning of the speech buffer");

		return;

	}

    buffer_speech[index_buffer_speech++]=c;
    //si llega al limite
    //if (index_buffer_speech==MAX_BUFFER_SPEECH || c==10) {
    if (index_buffer_speech==MAX_BUFFER_SPEECH) {
        textspeech_add_speech_fifo();
    }

}

void textspeech_send_new_line(void)
{
    //printf ("salto de linea");
    //printf("reset chardetect_x_position on textspeech_send_new_line\n");
    chardetect_x_position=0;

    textspeech_add_speech_fifo();
}



void textspeech_print_operating(void)
{
    generic_footertext_print_operating("SPEECH");

}




int index_buffer_pantalla_speech;

//En spectrum: 768 bytes
//En Z88, maximo de 106*6
//En ZX81 hi res, 36*32 = 1152
//Por si acaso, 2000
#define MAX_TEXTSPEECH_ENVIAR_PANTALLA 2000
//Buffer para enviar toda la pantalla a speech. sumar +1 del 0 al final
char buffer_pantalla_speech[MAX_TEXTSPEECH_ENVIAR_PANTALLA+1];

//Retorna ancho de pantalla en caracteres segun maquina activa
int textspeech_enviar_speech_da_ancho(void)
{
	if (MACHINE_IS_SPECTRUM) return 32;
	if (MACHINE_IS_Z88) return 106;
	if (MACHINE_IS_ZX8081) {
		if (rainbow_enabled.v) return 36;
		return 32;
	}

	//cualquier otra cosa??
	return 32;
}

void textspeech_enviar_speech_pantalla_printf (z80_byte c)
{
    //printf ("%c",c);
	scr_detectedchar_print(c);
	buffer_pantalla_speech[index_buffer_pantalla_speech]=c;

	if (index_buffer_pantalla_speech<MAX_TEXTSPEECH_ENVIAR_PANTALLA) index_buffer_pantalla_speech++;

	//Aunque caberia toda la pantalla en el buffer, trocearla por lineas
	if (index_buffer_pantalla_speech>=textspeech_enviar_speech_da_ancho() || c==10 || c==13) {
		buffer_pantalla_speech[index_buffer_pantalla_speech]=0;
		textspeech_print_speech(buffer_pantalla_speech);
		index_buffer_pantalla_speech=0;
	}

}

void textspeech_enviar_speech_pantalla_spectrum(void)
{

    screen_text_repinta_pantalla_spectrum_comun(0,textspeech_enviar_speech_pantalla_printf,1);
}

void textspeech_refresca_pantalla_sam_modo_013_fun_color(z80_byte color GCC_UNUSED, int *brillo GCC_UNUSED, int *parpadeo GCC_UNUSED)
{
	//no hacer nada
}


//Para saber cuando hay salto de linea
int textspeech_refresca_pantalla_sam_modo_013_last_y=-1;
void textspeech_refresca_pantalla_sam_modo_013_fun_caracter(int x GCC_UNUSED,int y,int brillo GCC_UNUSED, unsigned char inv GCC_UNUSED,z80_byte caracter )
{

    if (y!=textspeech_refresca_pantalla_sam_modo_013_last_y) textspeech_enviar_speech_pantalla_printf ('\n');

    textspeech_enviar_speech_pantalla_printf(caracter);

	textspeech_refresca_pantalla_sam_modo_013_last_y=y;

}

void textspeech_refresca_pantalla_sam_modo_2(void)
{
    scr_refresca_pantalla_sam_modo_2(textspeech_refresca_pantalla_sam_modo_013_fun_color,textspeech_refresca_pantalla_sam_modo_013_fun_caracter);
}

void textspeech_refresca_pantalla_sam_modo_013(int modo)
{
    scr_refresca_pantalla_sam_modo_013(modo,textspeech_refresca_pantalla_sam_modo_013_fun_color,textspeech_refresca_pantalla_sam_modo_013_fun_caracter);
}


void textspeech_enviar_speech_pantalla_sam(void)
{
	z80_byte modo_video=(sam_vmpr>>5)&3;

	switch (modo_video) {
        case 0:
                textspeech_refresca_pantalla_sam_modo_013(0);
        break;

        case 1:
                textspeech_refresca_pantalla_sam_modo_013(1);
        break;

        case 2:
                textspeech_refresca_pantalla_sam_modo_2();
        break;

        case 3:
                textspeech_refresca_pantalla_sam_modo_013(3);
        break;
    }

}


void textspeech_refresca_pantalla_cpc_fun_color(z80_byte color GCC_UNUSED, int *brillo GCC_UNUSED, int *parpadeo GCC_UNUSED)
{

/* No hacer nada

*/

}


void textspeech_refresca_pantalla_cpc_fun_saltolinea(void)
{

	printf ("\n");

}

void textspeech_refresca_pantalla_cpc_fun_caracter(int x GCC_UNUSED,int y GCC_UNUSED,int brillo GCC_UNUSED, unsigned char inv GCC_UNUSED,z80_byte caracter )
{
	textspeech_enviar_speech_pantalla_printf(caracter);
}


void textspeech_enviar_speech_pantalla_cpc(void)
{
	scr_refresca_pantalla_cpc_text(textspeech_refresca_pantalla_cpc_fun_color,textspeech_refresca_pantalla_cpc_fun_caracter,textspeech_refresca_pantalla_cpc_fun_saltolinea);
}


void textspeech_enviar_speech_pantalla_zx8081(void)
{
	screen_text_repinta_pantalla_zx81_comun(0,textspeech_enviar_speech_pantalla_printf,1);
}


void textspeech_enviar_speech_pantalla_z88_printf_newline(struct s_z88_return_character_atributes *z88_caracter GCC_UNUSED)
{
    textspeech_enviar_speech_pantalla_printf('\n');
}

void textspeech_enviar_speech_pantalla_z88_printf(struct s_z88_return_character_atributes *z88_caracter)
{

    //Si caracter no es nulo
    if (z88_caracter->null_caracter==0) {
        textspeech_enviar_speech_pantalla_printf(z88_caracter->ascii_caracter);
    }
}


void textspeech_enviar_speech_pantalla_z88(void)
{
    struct s_z88_return_character_atributes z88_caracter;

    z88_caracter.f_new_line=textspeech_enviar_speech_pantalla_z88_printf_newline;
    z88_caracter.f_print_char=textspeech_enviar_speech_pantalla_z88_printf;

    screen_repinta_pantalla_z88(&z88_caracter);

}




void textspeech_enviar_speech_pantalla(void)
{

	//if (textspeech_filter_program==NULL) return;

	index_buffer_pantalla_speech=0;
	if (MACHINE_IS_SPECTRUM) textspeech_enviar_speech_pantalla_spectrum();
	else if (MACHINE_IS_ZX8081) textspeech_enviar_speech_pantalla_zx8081();
	else if (MACHINE_IS_Z88) textspeech_enviar_speech_pantalla_z88();
	else if (MACHINE_IS_SAM) textspeech_enviar_speech_pantalla_sam();
	else if (MACHINE_IS_CPC) textspeech_enviar_speech_pantalla_cpc();


	//0 al final
	buffer_pantalla_speech[index_buffer_pantalla_speech]=0;
	textspeech_print_speech(buffer_pantalla_speech);
}



//Para funciones OCR que guardan en string final. Derivado de textspeech pero usando otro buffer y sin sacar nada por pantalla
char *ocr_text_buffer;
int ocr_index_position;

void ocr_enviar_printf (z80_byte c)
{
    ocr_text_buffer[ocr_index_position++]=c;
}




void ocr_z88_printf(struct s_z88_return_character_atributes *z88_caracter)
{

    //Si caracter no es nulo
    if (z88_caracter->null_caracter==0) {
        ocr_enviar_printf(z88_caracter->ascii_caracter);
    }
}

void ocr_z88_printf_newline(struct s_z88_return_character_atributes *z88_caracter GCC_UNUSED)
{
    ocr_enviar_printf('\n');
}

void ocr_pantalla_sam_modo_013_fun_color(z80_byte color GCC_UNUSED, int *brillo GCC_UNUSED, int *parpadeo GCC_UNUSED)
{
	//no hacer nada
}


//Para saber cuando hay salto de linea
int ocr_pantalla_sam_modo_013_last_y=-1;
void ocr_pantalla_sam_modo_013_fun_caracter(int x GCC_UNUSED,int y,int brillo GCC_UNUSED, unsigned char inv GCC_UNUSED,z80_byte caracter )
{

    if (y!=ocr_pantalla_sam_modo_013_last_y) ocr_enviar_printf ('\n');

    ocr_enviar_printf(caracter);

    ocr_pantalla_sam_modo_013_last_y=y;

}

void ocr_pantalla_sam_modo_2(void)
{
    scr_refresca_pantalla_sam_modo_2(ocr_pantalla_sam_modo_013_fun_color,ocr_pantalla_sam_modo_013_fun_caracter);
}

void ocr_pantalla_sam_modo_013(int modo)
{
    scr_refresca_pantalla_sam_modo_013(modo,ocr_pantalla_sam_modo_013_fun_color,ocr_pantalla_sam_modo_013_fun_caracter);
}


void ocr_enviar_speech_pantalla_sam(void)
{
	z80_byte modo_video=(sam_vmpr>>5)&3;

	switch (modo_video) {
        case 0:
                ocr_pantalla_sam_modo_013(0);
        break;

        case 1:
                ocr_pantalla_sam_modo_013(1);
        break;

        case 2:
                ocr_pantalla_sam_modo_2();
        break;

        case 3:
                ocr_pantalla_sam_modo_013(3);
        break;
    }

}


void ocr_pantalla_cpc_fun_color(z80_byte color GCC_UNUSED, int *brillo GCC_UNUSED, int *parpadeo GCC_UNUSED)
{

/* No hacer nada

*/

}


void ocr_pantalla_cpc_fun_saltolinea(void)
{

	ocr_enviar_printf ('\n');

}

void ocr_pantalla_cpc_fun_caracter(int x GCC_UNUSED,int y GCC_UNUSED,int brillo GCC_UNUSED, unsigned char inv GCC_UNUSED,z80_byte caracter )
{
	ocr_enviar_printf(caracter);
}


void ocr_enviar_speech_pantalla_cpc(void)
{
	scr_refresca_pantalla_cpc_text(ocr_pantalla_cpc_fun_color,ocr_pantalla_cpc_fun_caracter,ocr_pantalla_cpc_fun_saltolinea);
}



void ocr_get_text(char *s)
{
    ocr_text_buffer=s;
    ocr_index_position=0;
    if (MACHINE_IS_SPECTRUM) screen_text_repinta_pantalla_spectrum_comun(0,ocr_enviar_printf,1);
    else if (MACHINE_IS_ZX8081) screen_text_repinta_pantalla_zx81_comun(0,ocr_enviar_printf,1);
    else if (MACHINE_IS_Z88) {
        struct s_z88_return_character_atributes z88_caracter;

        z88_caracter.f_new_line=ocr_z88_printf_newline;
        z88_caracter.f_print_char=ocr_z88_printf;

        screen_repinta_pantalla_z88(&z88_caracter);

		}
    else if (MACHINE_IS_SAM) {
        ocr_enviar_speech_pantalla_sam();
    }

    else if (MACHINE_IS_CPC) ocr_enviar_speech_pantalla_cpc();

    else if (MACHINE_IS_TSCONF) {
		if (rainbow_enabled.v) {
		}
	}

    ocr_text_buffer[ocr_index_position++]=0;
}
