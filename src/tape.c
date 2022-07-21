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


//#include <unistd.h>
//#include <time.h>
//#include <stdarg.h>
#include <dirent.h>


#include "tape.h"
#include "tape_tap.h"
#include "tape_tzx.h"
#include "tape_pzx.h"
#include "tape_smp.h"
#include "cpu.h"
#include "operaciones.h"
#include "debug.h"
#include "snap.h"
#include "snap_z81.h"
#include "compileoptions.h"
#include "zx8081.h"
#include "zxvision.h"
#include "menu_items.h"
#include "utils.h"
#include "audio.h"
#include "screen.h"
#include "zxuno.h"
#include "timex.h"
#include "timer.h"
#include "superupgrade.h"
#include "multiface.h"
#include "tbblue.h"
#include "settings.h"
#include "msx.h"
#include "samram.h"
#include "zvfs.h"

#include "autoselectoptions.h"

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif

char *realtape_name;

char *tapefile;
char *tape_out_file;
//FILE *ptr_mycinta;
void *buffer_tap_read=NULL;
z80_bit noautoload;
z80_bit tape_any_flag_loading;

//indica que el autoload es con load "". sino es con enter
//0: autoload con ENTER
//1: autoload con LOAD(J) ""
//2: autoload con L O A D "" (para spectrum 128k spanish)
//3: autoload con enter, cursor arriba dos veces y enter (para NextOS)
int autoload_spectrum_loadpp_mode;


//Si hay que detectar rutinas de cargadores
//z80_bit autodetect_loaders={1};

//Si hay que acelerar rutinas de cargadores
z80_bit accelerate_loaders={0};

//Si hay que autorebobinar cinta standard
z80_bit tape_auto_rewind={0};

int (*tape_block_open)(void);
int (*tape_block_readlength)(void);
int (*tape_block_read)(void *dir,int longitud);
int (*tape_block_seek)(int longitud,int direccion);
int (*tape_block_feof)(void);
void (*tape_block_rewindbegin)(void);

int (*tape_out_block_open)(void);
int (*tape_out_block_close)(void);
int (*tape_block_save)(void *dir,int longitud);
void (*tape_block_begin_save)(int longitud,z80_byte flag);

int tape_out_inserted_is_pzx=0;


//Indica si hay cintas insertadas
//z80_bit tape_load_inserted;
//z80_bit tape_save_inserted;

//Indica si hay cintas insertadas
int tape_loadsave_inserted;

//Dice que si hay cinta standard cargando y se detecta rutina de carga no standard, la pasa como real tape y vuelve a cargar
z80_bit standard_to_real_tape_fallback={1};

z80_int zxuno_punto_entrado_load;

//indicar que hay cinta insertada y hay que hacer load ""
z80_bit initial_tap_load;
int initial_tap_sequence;
//0=nada
//1 esperando a llegar a main-1 (48k) -> 0x12a9
//2 se ha llegado y se enviara J
//3 se dejara pulsado sym
//4 se enviara p
//5 se enviara p
//7 se libera sym
//8 se enviara ENTER

int tape_pause=0;


//si hay cinta cargando
//contador se decrementa a cada segundo
//sirve para indicar mediante overlay en pantalla que se esta cargando cinta
//despues de cargar, permanece durante x segundos en pantalla
int tape_loading_counter=0;

void draw_tape_text_top_speed(void)
{
        //menu_footer_activity("TSPEED");
        generic_footertext_print_operating("TSPEED");
}

void draw_tape_icon_activity(void)
{
        if (!zxdesktop_icon_tape_inverse) {
                zxdesktop_icon_tape_inverse=1;
                menu_draw_ext_desktop();
        }
}

void draw_realtape_icon_activity(void)
{
    //printf("Cinta\n");
    lowericon_realtape_frame++;
    if (lowericon_realtape_frame==4) lowericon_realtape_frame=0;

    if (realtape_playing.v) menu_draw_ext_desktop();      
}

void draw_tape_text(void)
{
//printf("draw_tape_text\n");
		//tape_loading_counter=2;

		//color inverso
		if (top_speed_timer.v) {
			draw_tape_text_top_speed();
		}
		else {
                        //menu_footer_activity("TAPE");
                        generic_footertext_print_operating("TAPE");

                        //Y poner icono de cinta en inverso
                        draw_tape_icon_activity();
		}

}

void draw_realtape_text(void)
{
//printf("draw_realtape_text\n");
		//tape_loading_counter=2;

		//color inverso
		if (top_speed_timer.v) {
			draw_tape_text_top_speed();
		}
		else {
            //menu_footer_activity("TAPE");
            generic_footertext_print_operating("TAPE");

            //Y poner icono de cinta en inverso
            draw_realtape_icon_activity();
		}

}

/*void delete_tape_text(void)
{
        menu_delete_footer_activity();
}*/


void insert_tape_load(void)
{
	tape_loadsave_inserted = tape_loadsave_inserted | TAPE_LOAD_INSERTED;
	//tape_load_inserted.v=1;
}

void insert_tape_save(void)
{
	tape_loadsave_inserted = tape_loadsave_inserted | TAPE_SAVE_INSERTED;
        //tape_save_inserted.v=1;
}

void eject_tape_load(void)
{
	tape_loadsave_inserted = tape_loadsave_inserted & (255 - TAPE_LOAD_INSERTED);
        //tape_load_inserted.v=0;

        //Actualizar zxdesktop. Para refrescar iconos de cinta y que aparezca como expulsado
        menu_draw_ext_desktop();
}

void eject_tape_save(void)
{
	tape_loadsave_inserted = tape_loadsave_inserted & (255 - TAPE_SAVE_INSERTED);
        //tape_save_inserted.v=0;
}

int tape_block_p_open(void)
{


return 0;

}

int tape_block_z81_open(void)
{


return 0;

}


int tape_out_block_p_open(void)
{


return 0;

}





void set_tape_file_options(char *filename)
{
        set_snaptape_fileoptions(filename);
}

void set_tape_file_machine(char *filename)
{
        set_snaptape_filemachine(filename);
}



void tape_init(void)
{
                if (tapefile!=0) {
                        debug_printf (VERBOSE_INFO,"Initializing Tape File");

                        //if (strstr(tapefile,".tap")!=NULL  || strstr(tapefile,".TAP")!=NULL) {
                        if (!util_compare_file_extension(tapefile,"tap") ) {
                                        debug_printf (VERBOSE_INFO,"TAP file detected");
                                        tape_block_open=tape_block_tap_open;
                                        tape_block_read=tape_block_tap_read;
                                        tape_block_readlength=tape_block_tap_readlength;
                                        tape_block_seek=tape_block_tap_seek;
                                        tape_block_feof=tape_block_tap_feof;
                                        tape_block_rewindbegin=tape_block_tap_rewindbegin;
                                        insert_tape_load();
                                }

                        else if (!util_compare_file_extension(tapefile,"tzx") ) {
                                        debug_printf (VERBOSE_INFO,"TZX file detected");
                                        tape_block_open=tape_block_tzx_open;
                                        tape_block_read=tape_block_tzx_read;
                                        tape_block_readlength=tape_block_tzx_readlength;
                                        tape_block_seek=tape_block_tzx_seek;
                                        tape_block_feof=tape_block_tzx_feof;
                                        tape_block_rewindbegin=tape_block_tzx_rewindbegin;
                                }

                        else if (!util_compare_file_extension(tapefile,"pzx") ) {
                                        debug_printf (VERBOSE_INFO,"PZX file detected");
                                        tape_block_open=tape_block_pzx_open; 
                                        tape_block_read=tape_block_pzx_read; 
                                        tape_block_readlength=tape_block_pzx_readlength; 
                                        tape_block_seek=tape_block_pzx_seek; 
                                        tape_block_feof=tape_block_pzx_feof; 
                                        tape_block_rewindbegin=tape_block_pzx_rewindbegin; 
                                }                                


                        else if (!util_compare_file_extension(tapefile,"o") || !util_compare_file_extension(tapefile,"80") ) {
                                        debug_printf (VERBOSE_INFO,"ZX80 Tape file detected");
                                        tape_block_open=tape_block_p_open;
                                }

                        else if (!util_compare_file_extension(tapefile,"p") || !util_compare_file_extension(tapefile,"81") ) {
                                        debug_printf (VERBOSE_INFO,"ZX81 Tape file detected");
                                        tape_block_open=tape_block_p_open;
                                }

                        else if (!util_compare_file_extension(tapefile,"z81") ) {
                                        debug_printf (VERBOSE_INFO,"ZX80/ZX81 (.Z81) Tape file detected");
                                        tape_block_open=tape_block_z81_open;
                        }

                        else if (!util_compare_file_extension(tapefile,"cas") ) {
                                        debug_printf (VERBOSE_INFO,"MSX (.CAS) Tape file detected");
                                        tape_block_open=tape_block_cas_open;
                                }



			//else if (!util_compare_file_extension(tapefile,"smp") ) {
                        //                debug_printf (VERBOSE_INFO,"SMP - raw audio -  Tape file detected");
			else if (!util_compare_file_extension(tapefile,"rwa") || !util_compare_file_extension(tapefile,"smp")
				|| !util_compare_file_extension(tapefile,"wav")

				) {
                                        debug_printf (VERBOSE_INFO,"RWA - raw audio -  Tape file detected");
                                        tape_block_open=tape_block_smp_open;
                                        tape_block_read=tape_block_smp_read;
                                        tape_block_readlength=tape_block_smp_readlength;
                                        tape_block_seek=tape_block_smp_seek;
                                        insert_tape_load();

                                }




                        else {
                                        debug_printf (VERBOSE_ERR,"Tape format not supported");
                                        tapefile=NULL;
                        }

                        //abrir cinta
                        if (tapefile!=0) tap_open();


			set_tape_file_machine(tapefile);
			set_tape_file_options(tapefile);
                }
}

void tape_out_init(void)
{
if (tape_out_file!=0) {
                        debug_printf (VERBOSE_INFO,"Initializing Out Tape File");

                        tape_out_inserted_is_pzx=0;

                        //if (strstr(tape_out_file,".tap")!=NULL  || strstr(tape_out_file,".TAP")!=NULL) {
                        if (!util_compare_file_extension(tape_out_file,"tap") ) {
                                        debug_printf (VERBOSE_INFO,"Out TAP file detected");
                                        tape_out_block_open=tape_out_block_tap_open;
                                        tape_out_block_close=tape_out_block_tap_close;
                                        tape_block_save=tape_block_tap_save;
                                        tape_block_begin_save=tape_block_tap_begin_save;
                                }

                        else if (!util_compare_file_extension(tape_out_file,"tzx") ) {
                                        debug_printf (VERBOSE_INFO,"Out TZX file detected");
                                        tape_out_block_open=tape_out_block_tzx_open;
                                        tape_out_block_close=tape_out_block_tzx_close;
                                        tape_block_save=tape_block_tzx_save;
                                        tape_block_begin_save=tape_block_tzx_begin_save;
                                }

                        else if (!util_compare_file_extension(tape_out_file,"pzx") ) {

                                       

                                        
                                        debug_printf (VERBOSE_INFO,"Out PZX file detected");
                                        tape_out_block_open=tape_out_block_pzx_open;
                                        tape_out_block_close=tape_out_block_pzx_close;
                                        tape_block_save=tape_block_pzx_save;
                                        tape_block_begin_save=tape_block_pzx_begin_save;
                                        tape_out_inserted_is_pzx=1;
                                        

                                       /*
                                       Problema: las funciones de save están pensadas para formatos tap y tzx binarios,
                                       en que al escribir el bloque, antes se le envian dos bytes con la longitud del bloque,
                                       luego el flag, luego los datos y luego el checksum, usando siempre funcion tape_block_save
                                       Esto para tzx y tap va perfecto. Para pzx no, dado que la longitud del bloque no viene
                                       exactamente antes de los datos en sí
                                       Ya he modificado la funcion tape_block_begin_save para permitir enviar longitud, antes del bloque 
                                       en si
                                       
                                       Todo esto se corrige usando variable tape_out_inserted_is_pzx

                                       
                                       */
                                }                                

                        else if (!util_compare_file_extension(tape_out_file,"o") ) {
                                        debug_printf (VERBOSE_INFO,"Out .O file detected");
					if (!(MACHINE_IS_ZX80)) {
						debug_printf (VERBOSE_ERR,"Out Tape format only supported on ZX80 models");
						tape_out_file=NULL;
					}
					else {
	                                        tape_out_block_open=tape_out_block_p_open;
					}
                                }

                        else if (!util_compare_file_extension(tape_out_file,"p") ) {
                                        debug_printf (VERBOSE_INFO,"Out .P file detected");
                                        if (!(MACHINE_IS_ZX81)) {
                                                debug_printf (VERBOSE_ERR,"Out Tape format only supported on ZX81 models");
                                                tape_out_file=NULL;
                                        }
                                        else {
                                                tape_out_block_open=tape_out_block_p_open;
                                        }
                                }



                        else {
                                        debug_printf (VERBOSE_ERR,"Out Tape format not supported");
                                        tape_out_file=NULL;
                        }



                        //NO abrir cinta. Esto ya se hará cada vez que se escribe
                        if (tape_out_file!=0) tap_out_open();
                }
}


int tap_open(void)
{

    initial_tap_load.v=0;

    if (tapefile!=0) {

	tape_block_open();


	//if (noautoload.v==0 && !MACHINE_IS_TBBLUE) { //TODO: desactivamos autoload en TBBLUE
        if (noautoload.v==0) { 
		debug_printf (VERBOSE_INFO,"Restarting autoload");
		initial_tap_load.v=1;
		initial_tap_sequence=0;

		debug_printf (VERBOSE_INFO,"Reset cpu due to autoload");
		reset_cpu();

		//Activamos top speed si conviene
		if (fast_autoload.v && !MACHINE_IS_MSX) {
                        debug_printf (VERBOSE_INFO,"Set top speed");
                        top_speed_timer.v=1;
                }

	}

	else {
		initial_tap_load.v=0;
	}

	insert_tape_load();
    }


    return 0;

}

int tap_close(void)
{
	eject_tape_load();
	return 0;
}



int tap_out_open(void)
{

    if (tape_out_file!=0) {

        insert_tape_save();

    }

    return 0;

}

int tap_out_close(void)
{
        eject_tape_save();
	return 0;
}


void tap_save_ace(void)
{

        z80_byte flag=reg_c;
        z80_int dir=reg_hl;
        z80_int longitud=value_8_to_16(reg_d,reg_e);

        reg_pc=pop_valor();

        debug_printf(VERBOSE_INFO,"Saving %d bytes at %d address with flag %d",longitud,dir,flag);


        if (tape_out_block_open()) return;

        //Avisamos que vamos a escribir un bloque... en tzx se usa para meter el id correspondiente
        tape_block_begin_save(longitud,0);

        //Escribimos longitud (contando checksum)
		//TAP en jupiter ace no incluye el flag, aunque la cinta real si
        longitud+=1;

        //Si es TZX, agregamos flag
        if (tape_block_save==tape_block_tzx_save) {
            debug_printf(VERBOSE_INFO,"Adding Flag as we are saving to TZX");
            longitud++;
        }        


        if (tape_block_save(&longitud, 2)!=2) {
                debug_printf(VERBOSE_ERR,"Error writing length");
                //tape_out_file=0;
                eject_tape_save();
                //tape_save_inserted.v=0;
                tape_out_block_close();
                return;
        }

        //Si es TZX, agregamos flag
        if (tape_block_save==tape_block_tzx_save) {
            //Escribimos flag
            if (tape_block_save(&flag, 1)!=1) {
                    debug_printf(VERBOSE_ERR,"Error writing flag");
                    //tape_out_file=0;
                    eject_tape_save();
                    //tape_save_inserted.v=0;
                    tape_out_block_close();
                    return;
            }
            longitud--;            
        }

		

        //Escribimos bytes
        longitud-=1;
        //z80_byte checksum=flag;
			z80_byte checksum=0;
        z80_byte leido;

        for (;longitud;longitud--,dir++) {
leido=peek_byte_no_time(dir);
                checksum=checksum ^ leido;
                if (tape_block_save(&leido, 1)!=1) {
                        debug_printf(VERBOSE_ERR,"Error writing bytes");
                        //tape_out_file=0;
                        eject_tape_save();
                        //tape_save_inserted.v=0;
                        tape_out_block_close();
                        return;
                }
        }


        //Escribimos checksum
        if (tape_block_save(&checksum, 1)!=1) {
                debug_printf(VERBOSE_ERR,"Error writing checksum");
                //tape_out_file=0;
                eject_tape_save();
                //tape_save_inserted.v=0;
                tape_out_block_close();
                return;
        }


        tape_out_block_close();
			DE=0;
			HL=dir;
        return;


}



void tap_load_ace(void)
{



	if (buffer_tap_read==NULL) {
        //asignamos buffer memoria temporal para lectura
        buffer_tap_read=malloc(65536);
        if (buffer_tap_read==NULL) {
                cpu_panic("Error allocating tap read memory buffer");
        }
    }

	z80_int cinta_pedido_inicio=reg_hl;
	z80_int cinta_pedido_longitud=value_8_to_16(reg_d,reg_e);
	z80_byte flag_asked=reg_c;


	//leemos longitud, flag de la cinta

    z80_int cinta_longitud;

    if (tape_block_readlength==NULL) {
        debug_printf (VERBOSE_ERR,"Tape functions uninitialized");
        //tapefile=NULL;
        eject_tape_load();
        //tape_load_inserted.v=0;
        Z80_FLAGS &=(255-FLAG_C);
        reg_pc=pop_valor();
        return;
    }


    cinta_longitud=tape_block_readlength();
    if (cinta_longitud==0) {
        debug_printf(VERBOSE_INFO,"Error read tape. Bytes=0");
        //tapefile=NULL;
        eject_tape_load();
        //tape_load_inserted.v=0;
        Z80_FLAGS &=(255-FLAG_C);
        reg_pc=pop_valor();
        return;
    }

    else    {
        //con esto restamos el byte de CRC??
        cinta_longitud-=1;
    }

    //TAP en jupiter ace no incluye flag, aunque la cinta real si
    //TZX en jupiter ace en cambio si que incluye flag
    /*
    Esto es un poco chapuza. Inicialmente solo teniamos el soporte de TAP en Jupiter Ace, el cual no incluye flag en las cintas
    Luego, se le dio soporte a los TZX, que si que incluye flag. Dado que las rutinas iniciales no tenian en cuenta el flag para nada,
    lo que hacemos es ignorar ese flag de los TZX
    Esto tiene el inconveniente de que cuando la rom quiere cargar un bloque de cabecera (flag 0), como lo estamos ignorando, cargara
    cualquier bloque (no solo de flag 0) como si quisiera ser de cabecera
    Esto provoca que si ponemos por ejemplo "load pepe" y hay otro bloque antes (normalmente una cabecera con flag 0 y luego datos con flag 255),
    al no tener en cuenta los flags, ese bloque de datos de flag 255 lo tratara tambien como si fuera de cabecera, mostrando en pantalla probablemente
    algo como Bytes: XQK2uy7xxu  (o cualquier otro texto con basura)
    En cambio, cuando se cargan bloques seguidos, uno despues del otro, siempre vendran en el orden que la rom espera:
    cabecera: flag 0, datos: flag 255, cabecera: flag 255, datos: flag 255, etc....
    */
    if (tape_block_read==tape_block_tzx_read) {
        //saltar flag
        //printf("skipping flag (%d) longitud %d\n",*puntero,leidos);
        //leidos--;
        debug_printf(VERBOSE_INFO,"Skipping flag as the input file is TZX");
        z80_byte flag_no_usado;
        tape_block_read(&flag_no_usado,1);
        cinta_longitud--;
    }    


    debug_printf(VERBOSE_INFO,"load start=%d length asked=%d length tape=%d (0x%04X) flag_asked=%d",
            cinta_pedido_inicio,cinta_pedido_longitud,cinta_longitud,cinta_longitud,flag_asked);


	int leidos=0;
	z80_byte checksum;


    if (cinta_longitud != cinta_pedido_longitud) {
        debug_printf(VERBOSE_INFO,"Tape length (%d) is not what asked (%d)",cinta_longitud,cinta_pedido_longitud);
        if (cinta_longitud>cinta_pedido_longitud) {
            debug_printf(VERBOSE_INFO,"Tape length is more than asked");
            leidos=tape_block_read(buffer_tap_read,cinta_pedido_longitud);
            //leemos checksum
            tape_block_read(&checksum,1);

            //y saltamos el resto que sobra
            debug_printf(VERBOSE_INFO,"Skipping %d bytes",cinta_longitud-cinta_pedido_longitud);
            //fseek(ptr_mycinta,cinta_longitud-cinta_pedido_longitud,SEEK_CUR);

            tape_block_seek(cinta_longitud-cinta_pedido_longitud,SEEK_CUR);

        }
        if (cinta_longitud<cinta_pedido_longitud) {
            debug_printf(VERBOSE_INFO,"Tape length is less than asked. Reading %d bytes",cinta_longitud);

            leidos=tape_block_read(buffer_tap_read,cinta_longitud);

            checksum=0;

            //devolver error si no es que estamos en modo any flag
            //descartar checksum
            z80_byte nada;
            tape_block_read(&nada,1);
            debug_printf(VERBOSE_INFO,"Returning load error");
            Z80_FLAGS &=(255-FLAG_C);
        }


    }

    else {

        leidos=tape_block_read(buffer_tap_read,cinta_longitud);
        //leemos checksum
        tape_block_read(&checksum,1);

    }

	//Copiamos estos datos en destino
	z80_byte *puntero;
	puntero=buffer_tap_read;

	//Primero metemos tipo de bloque, que no esta incluido en cinta tap
	/*
	if (flag_asked==0) {
		debug_printf(VERBOSE_INFO,"Tape block is header. Writing 0 value");
		poke_byte_no_time(cinta_pedido_inicio++,0);
	}
	else {
		debug_printf(VERBOSE_INFO,"Tape block is data. Writing 255 value");
		poke_byte_no_time(cinta_pedido_inicio++,255);
	}
	*/





	//for (;cinta_pedido_longitud>0;cinta_pedido_longitud--,puntero++) {
	for (;leidos>0;leidos--,puntero++) {
		//z80_byte c;
		//c=*puntero;
		//if (c>31 && c<128) printf ("%c",c);
		poke_byte_no_time(cinta_pedido_inicio++,*puntero);
	}

	HL=cinta_pedido_inicio;
	DE=0;


	debug_printf(VERBOSE_INFO,"Returning tape routine without error");

	Z80_FLAGS |=FLAG_C;

	//volver
	reg_pc=pop_valor();
}

void tap_load(void)
{

//printf ("tap load\n");


			if (buffer_tap_read==NULL) {
				//asignamos buffer memoria temporal para lectura
				buffer_tap_read=malloc(65536);
				if (buffer_tap_read==NULL) {
					cpu_panic("Error allocating tap read memory buffer");
				}
			}

			z80_int cinta_pedido_inicio=reg_ix;
			z80_byte cinta_pedido_flag=reg_a_shadow;
			z80_int cinta_pedido_longitud=value_8_to_16(reg_d,reg_e);

			//z80_byte h,l;
			z80_byte checksum,checksum_calculado;



			//leemos longitud, flag de la cinta

			z80_int cinta_longitud;

			if (tape_block_readlength==NULL) {
				debug_printf (VERBOSE_ERR,"Tape functions uninitialized");
                                //tapefile=NULL;
				eject_tape_load();
				//tape_load_inserted.v=0;
				Z80_FLAGS &=(255-FLAG_C);
				reg_pc=pop_valor();

				return;
			}

			cinta_longitud=tape_block_readlength();
			if (cinta_longitud==0) {
                int retornar_error=1;
                if (tape_auto_rewind.v) {
                    if (tape_block_feof()) {
                        debug_printf(VERBOSE_INFO,"End of tape and autorewind enabled. Rewind tape");
                        tape_block_rewindbegin();
                        
                        cinta_longitud=tape_block_readlength();

                        if (cinta_longitud!=0) retornar_error=0;
                    }
                }

                if (retornar_error) {
                    debug_printf(VERBOSE_INFO,"Error read tape. Bytes=0");
                    //tapefile=NULL;
                    eject_tape_load();
                    //tape_load_inserted.v=0;
                    Z80_FLAGS &=(255-FLAG_C);
                    reg_pc=pop_valor();
                    return;
                }
            }



			z80_byte cinta_flag=0;


			//if (tape_any_flag_loading.v==0) flag_Z_shadow.v=0;
			if (tape_any_flag_loading.v==0) Z80_FLAGS_SHADOW &=(255-FLAG_Z);


			//if (flag_Z_shadow.v==1) {
                        if (Z80_FLAGS_SHADOW & FLAG_Z) {
				debug_printf(VERBOSE_INFO,"Mode any flag");

				char buffer_reg[1000];
				print_registers(buffer_reg);

				debug_printf(VERBOSE_INFO,"%s",buffer_reg);

				cinta_flag=0;



				//TODO. En teoria el modo de cualquier flag deberia cargar el flag como byte... pero esto no funciona bien
				//con rocman o en snapshot de chase hq por ejemplo. deberia ser cinta_longitud-=1 y no hacer el fread,
				//para que se leyese el flag como byte
			}

			else	{
				cinta_longitud-=2;
				tape_block_read(&cinta_flag,1);
			}


			debug_printf(VERBOSE_INFO,"load start=%d flag asked=%d length asked=%d flag tape=%d length tape=%d",
				cinta_pedido_inicio,cinta_pedido_flag,cinta_pedido_longitud,cinta_flag,cinta_longitud);


			if (cinta_pedido_flag!=cinta_flag && (Z80_FLAGS_SHADOW & FLAG_Z)==0 ) {
				debug_printf(VERBOSE_INFO,"Tape flag is not what asked");
				//fseek(ptr_mycinta,cinta_longitud,SEEK_CUR);
				tape_block_seek(cinta_longitud,SEEK_CUR);
                                //saltamos checksum
                                tape_block_read(&checksum,1);
				//volver a cargar
				if (MACHINE_IS_ZXUNO) reg_pc=zxuno_punto_entrado_load;
				else if (MACHINE_IS_TIMEX_T2068) reg_pc=255;
                                else reg_pc=1378;

			}

			//leemos bytes
			else {
				Z80_FLAGS |=FLAG_C;

				//no hace falta inicializarlo a 0, solo es para evitar un warning de compilacion
				//warning: ‘leidos’ may be used uninitialized in this function
				//Realmente no pasara nunca, siempre entrara en alguno de los if o else y se inicializará
				int leidos=0;

				if (cinta_longitud != cinta_pedido_longitud) {
					debug_printf(VERBOSE_INFO,"Tape length (%d) is not what asked (%d)",cinta_longitud,cinta_pedido_longitud);
					if (cinta_longitud>cinta_pedido_longitud) {
						debug_printf(VERBOSE_INFO,"Tape length is more than asked");
						leidos=tape_block_read(buffer_tap_read,cinta_pedido_longitud);
						//leemos checksum si no en modo any flag
						if ((Z80_FLAGS_SHADOW & FLAG_Z)==0)	tape_block_read(&checksum,1);

						//y saltamos el resto que sobra
						debug_printf(VERBOSE_INFO,"Skipping %d bytes",cinta_longitud-cinta_pedido_longitud);
		                                //fseek(ptr_mycinta,cinta_longitud-cinta_pedido_longitud,SEEK_CUR);

						tape_block_seek(cinta_longitud-cinta_pedido_longitud,SEEK_CUR);

					}
					if (cinta_longitud<cinta_pedido_longitud) {
						debug_printf(VERBOSE_INFO,"Tape length is less than asked. Reading %d bytes",cinta_longitud);

						leidos=tape_block_read(buffer_tap_read,cinta_longitud);



						checksum=0;

						//devolver error si no es que estamos en modo any flag
						if ((Z80_FLAGS_SHADOW & FLAG_Z)==0) {
						//TODO: completar esto
						//descartar checksum
						z80_byte nada;
						tape_block_read(&nada,1);

							debug_printf(VERBOSE_INFO,"Returning load error");
							Z80_FLAGS &=(255-FLAG_C);
						}


					}
				}

				else {
					//leidos=fread(buffer_tap_read,1,cinta_longitud,ptr_mycinta);
					leidos=tape_block_read(buffer_tap_read,cinta_longitud);
        	                        //leemos checksum
                	                tape_block_read(&checksum,1);
				}

//				printf ("leidos: %d\n",leidos);

				//simulamos sonido de carga, pokeando en destino tambien
				load_spectrum_simulate_loading(buffer_tap_read,cinta_pedido_inicio,leidos,cinta_flag);


				//calculamos checksum
				z80_byte *origen,tempbyte;
				origen=buffer_tap_read;
				checksum_calculado=cinta_flag;

				//printf ("calculamos checksum. leidos=%d checksum_calculado_inicial: %d\n",leidos,checksum_calculado);

				for (;leidos;leidos--) {
					tempbyte=*origen;
					poke_byte_no_time(cinta_pedido_inicio++,tempbyte);
//					printf ("byte\n");
					origen++;
					checksum_calculado=checksum_calculado ^ tempbyte;
					//printf ("0x%x ", checksum_calculado);
				}

				checksum_calculado=checksum_calculado ^ checksum;





				if (checksum_calculado!=0) {
					debug_printf(VERBOSE_INFO,"Tape checksum is not 0");
					Z80_FLAGS &=(255-FLAG_C);
                                }

				reg_pc=pop_valor();

				//reg_a=checksum_calculado;
				//H=checksum
				reg_h=checksum_calculado;
				reg_ix=cinta_pedido_inicio++;


                                //En principio la salida de carga siempre retorna flag Z a 0
                                //esto corrige un problema en la carga de Rocman:
                                //carga bloque de atributos, con "any flag loading" (flag Z' a 1), en 22527, con longitud 769
                                //al volver de ese bloque, carga el siguiente (tal cual como esté flag Z, no lo toca),
                                //y si no lo ponemos a 0, entonces el siguiente bloque lo cargaria como any flag loading, de nuevo,
                                //cosa que es error, pues el siguiente bloque lo carga en 16384
                                //de ahi que reseteemos siempre flag Z al volver
                                //Probablemente este comportamiento sea un bug en el juego, en como carga,
                                //pues antes de cargar el primer bloque, hace un XOR A, esto activa flag Z, cosa que no tiene sentido,
                                //probablemente el programador vio que cargaba los atributos en 22529, sin saber por qué (metiendo en 22528 el flag),
                                //y simplemente cambió la carga a 225287 y aumentó la longitud en 1. O no... quien sabe
                                Z80_FLAGS &=(255-FLAG_Z);


				debug_printf(VERBOSE_INFO,"Returning H=0x%x IX=%d",reg_h,reg_ix);

			}

		//}



}



//TODO: quiza el salvado de datos (flag,datos,checksum) se podria hacer aqui de manera comun, e ir llamando a funciones de grabado de bytes
void tap_save(void)
{

	z80_byte flag=reg_a;
	z80_int dir=reg_ix;
	z80_int longitud=value_8_to_16(reg_d,reg_e);

	reg_pc=pop_valor();

	debug_printf(VERBOSE_INFO,"Saving %d bytes at %d address with flag %d",longitud,dir,flag);


        if (tape_out_block_open()) return;

        //Escribimos longitud (contando flag+checksum)
        longitud+=2;

	//Avisamos que vamos a escribir un bloque... en tzx se usa para meter el id correspondiente
	tape_block_begin_save(longitud,flag);        


        //Solo hacer esto si no es un archivo tipo PZX
        if (!tape_out_inserted_is_pzx) {

        if (tape_block_save(&longitud, 2)!=2) {
                debug_printf(VERBOSE_ERR,"Error writing length");
                //tape_out_file=0;
		eject_tape_save();
		//tape_save_inserted.v=0;
		tape_out_block_close();
                return;
        }

        }


        //Escribimos flag
        if (tape_block_save(&flag, 1)!=1) {
                debug_printf(VERBOSE_ERR,"Error writing flag");
                //tape_out_file=0;
		eject_tape_save();
		//tape_save_inserted.v=0;
		tape_out_block_close();
                return;
        }

        //Escribimos bytes
        longitud-=2;
        z80_byte checksum=flag;
        z80_byte leido;

        for (;longitud;longitud--,dir++) {
                leido=peek_byte_no_time(dir);
                checksum=checksum ^ leido;
                if (tape_block_save(&leido, 1)!=1) {
                        debug_printf(VERBOSE_ERR,"Error writing bytes");
                        //tape_out_file=0;
			eject_tape_save();
			//tape_save_inserted.v=0;
			tape_out_block_close();
                        return;
                }
        }


        //Escribimos checksum
        if (tape_block_save(&checksum, 1)!=1) {
                debug_printf(VERBOSE_ERR,"Error writing checksum");
                //tape_out_file=0;
		eject_tape_save();
		//tape_save_inserted.v=0;
		tape_out_block_close();
                return;
        }


	tape_out_block_close();
        return;


}



/*
En Spectrum detectar direccion 1378
En ZX-Uno, detectar:

lbytes  di                      ; disable interrupts
        ld      a, $0f          ; make the border white and mic off.
        out     ($fe), a        ; output to port.
        push    ix
        pop     hl              ; pongo la direccion de comienzo en hl
        ld      c, 2
        exx

Y siempre carga flag 255

lbytes vale: 3F02H

*/

int is_tape_inserted(void)
{
	if ((tape_loadsave_inserted & TAPE_LOAD_INSERTED)!=0 && tapefile!=NULL) return 1;
	else return 0;
}

int tap_load_detect(void)
{

		//En ZX-UNO detectar carga tipica de la rom o la de la bios
		if (MACHINE_IS_ZXUNO) {
			if (reg_pc!=1378 && reg_pc!=0x3F02) return 0;
		}

		else if (MACHINE_IS_PRISM) {
			if (reg_pc!=1378) return 0;
		}

		else if (MACHINE_IS_TSCONF) {
			if (reg_pc!=1378) return 0;
		}                

		else if (MACHINE_IS_TBBLUE) {
                        if (reg_pc!=1378) return 0;
                }

                else if (MACHINE_IS_CHROME) {
                        if (reg_pc!=1378) return 0;
                }


		//Para Timex
		else if (MACHINE_IS_TIMEX_T2068) {
			if (reg_pc!=255) return 0;
		}

                else if (reg_pc!=1378) return 0;


                if (tapefile==0) return 0;
		//if (tape_load_inserted.v==0) return 0;
		if ( (tape_loadsave_inserted & TAPE_LOAD_INSERTED)==0) return 0;



    //Si esta multiface y esta mapeada su rom, no detectar carga
    if (multiface_enabled.v && multiface_switched_on.v) return 0;

		//Caso superupgrade
		//no mirar rom porque no sabemos que rom estamos leyendo (si la rom 3 de carga en un +2a, si la rom 1 en un 128k, etc)
                //Detectar por PC y por instrucciones
                if (superupgrade_enabled.v) {
                                //Ver que instruccion sea IN A,FEH, en caso de rutina normal de la ROM
                                if (peek_byte_no_time(reg_pc)!=0xDB) return 0;
                                if (peek_byte_no_time(reg_pc+1)!=0xFE) return 0;

				return 1;
                }



                if (MACHINE_IS_SPECTRUM_16_48) {

                                //maquina 16k, inves ,48k o tk
                                return 1;
                }

                if (MACHINE_IS_SPECTRUM_128_P2)  {

                                //maquina 128k. rom 1 mapeada
                                if ((puerto_32765 & 16) ==16)
                                return 1;
                }



                //Para Timex
		if (MACHINE_IS_TIMEX_T2068) {
                        //Si rom EX mapeada
                        if ( (timex_port_f4 &1) == 0) return 0; //Home mapeada , volver
                        if ( (timex_port_ff&128) == 0 ) return 0; //Dock mapeada, volver
			return 1;
		}

                if (MACHINE_IS_SPECTRUM_P2A_P3) {
                                //maquina +2A
                                if ((puerto_32765 & 16) ==16   && ((puerto_8189&4) ==4  ))
                                return 1;
                }

		//Para PRISM, detectar como zxuno. Por PC y por instrucciones
		//no mirar rom porque no sabemos que rom estamos leyendo (si la rom 3 de carga en un +2a, si la rom 1 en un 128k, etc)
                //Detectar por PC y por instrucciones
		if (MACHINE_IS_PRISM) {
				//Ver que instruccion sea IN A,FEH, en caso de rutina normal de la ROM
                                if (peek_byte_no_time(reg_pc)!=0xDB) return 0;
                                if (peek_byte_no_time(reg_pc+1)!=0xFE) return 0;

				return 1;

		}

		//Para TSCONF, detectar como zxuno. Por PC y por instrucciones
		//no mirar rom porque no sabemos que rom estamos leyendo (si la rom 3 de carga en un +2a, si la rom 1 en un 128k, etc)
                //Detectar por PC y por instrucciones
		if (MACHINE_IS_TSCONF) {
				//Ver que instruccion sea IN A,FEH, en caso de rutina normal de la ROM
                                if (peek_byte_no_time(reg_pc)!=0xDB) return 0;
                                if (peek_byte_no_time(reg_pc+1)!=0xFE) return 0;

				return 1;

		}                

		//Para TBBlue, detectar como zxuno. Por PC y por instrucciones
                //no mirar rom porque no sabemos que rom estamos leyendo (si la rom 3 de carga en un +2a, si la rom 1 en un 128k, etc)
                //Detectar por PC y por instrucciones

		if (MACHINE_IS_TBBLUE) {
				//Ver que instruccion sea IN A,FEH, en caso de rutina normal de la ROM
                                if (peek_byte_no_time(reg_pc)!=0xDB) return 0;
                                if (peek_byte_no_time(reg_pc+1)!=0xFE) return 0;

				return 1;

                }

  if (MACHINE_IS_CHROME) {
            				//Ver que instruccion sea IN A,FEH, en caso de rutina normal de la ROM
          if (peek_byte_no_time(reg_pc)!=0xDB) return 0;
          if (peek_byte_no_time(reg_pc+1)!=0xFE) return 0;

            				return 1;

      }


                if (MACHINE_IS_ZXUNO_BOOTM_DISABLED) {
		/*
		Nota. Rutinas rom carga 1378 y grabacion 1222 en otras roms diferentes de la 3:
		-Spectrum 128k, +2. Rom0. 1378. Corresponde a mensaje L0561:  DEFB $7F                           ; '(c)'.
								        DEFM " 1986 Sinclair Research Lt"  ;
		  registro PC no llegara ahi nunca
		-Spectrum 128k, +2. Rom0. 1222. Corresponde a mensaje L04C1:  DEFM "File already exist"          ; Report 'e'.
							        DEFB 's'+$80

		-Spanish Spectrum 128k. Rom0. 1222. Corresponde a mensaje L04C0:  DEFM "NOTA FUERA DE RANG"          ; Report 'm'.
        DEFB 'O'+$80

		-Spanish Spectrum 128k. Rom0. 1378. Corresponde a         DEFB $06, $00     ; Stream $02 leads to channel 'S'.


		-Spectrum +2A. rom0. english. 1222. entra en medio de ultima instruccion:.l04bf  ld      ($fc9a),hl
		        call    $1420
		        ld      (E_PPC),hl
		-Spectrum +2A. rom0. english. 1378. entra en ultimo cp!!!!!! : .l0559  push    bc
        ld      bc,$0023
        lddr
        pop     bc
        ld      a,b
        dec     c
        cp      c
        jr      c,l0559             ; (-12)

.l0565  ex      de,hl
		En este caso se pensaria que esa rutina es la de carga... :(


		*/

			if (reg_pc==0x3F02) {
				//Trap para rutina de la bios del zx-uno
				if (peek_byte_no_time(reg_pc)==0xF3) {
					//metemos flag
					reg_a_shadow=255;
					zxuno_punto_entrado_load=0x3F02;

					return 1;
				}
				return 0;
			}

				zxuno_punto_entrado_load=1378;

				//Ver que instruccion sea IN A,FEH, en caso de rutina normal de la ROM
				if (peek_byte_no_time(reg_pc)!=0xDB) return 0;
				if (peek_byte_no_time(reg_pc+1)!=0xFE) return 0;

				return 1;

                }


        return 0;
}


int tap_save_detect(void)
{

		//Para Timex
                if (MACHINE_IS_TIMEX_T2068) {
                        if (reg_pc!=108) return 0;
                }

                else if (reg_pc!=1222) return 0;
                if (tape_out_file==0) return 0;
		//if (tape_save_inserted.v==0) return 0;
		if ( (tape_loadsave_inserted & TAPE_SAVE_INSERTED)==0) return 0;

    //Si esta multiface y esta mapeada su rom, no detectar grabacion
    if (multiface_enabled.v && multiface_switched_on.v) return 0;


    if (samram_enabled.v) {
        return samram_tap_save_detect();
    }


                if (superupgrade_enabled.v) {
                        //Como maquina +2A
                        //Ver que instruccion sea ld hl,1f80
                                if (peek_byte_no_time(reg_pc)!=0x21) return 0;
                                if (peek_byte_no_time(reg_pc+1)!=0x80) return 0;
                                if (peek_byte_no_time(reg_pc+2)!=0x1f) return 0;

                                //Sea cual sea la rom, si reg_pc coincide e instruccion es la indicada antes
                                return 1;
                }



                if (MACHINE_IS_SPECTRUM_16_48) {

                                //maquina 16k, inves o 48k
                                return 1;
                }

                if (MACHINE_IS_SPECTRUM_128_P2) {

                                //maquina 128k. rom 1 mapeada
                                if ((puerto_32765 & 16) ==16)
                                return 1;
                }



		//Para Timex
                if (MACHINE_IS_TIMEX_T2068) {
                        //Si rom EX mapeada
                        if ( (timex_port_f4 &1) == 0) return 0; //Home mapeada , volver
                        if ( (timex_port_ff&128) == 0 ) return 0; //Dock mapeada, volver
                        return 1;
                }



                if (MACHINE_IS_SPECTRUM_P2A_P3) {
                                //maquina +2A
                                if ((puerto_32765 & 16) ==16   && ((puerto_8189&4) ==4  ))
                                return 1;
                }

		if (MACHINE_IS_PRISM) {
			//Como maquina +2A
			//Ver que instruccion sea ld hl,1f80
                                if (peek_byte_no_time(reg_pc)!=0x21) return 0;
                                if (peek_byte_no_time(reg_pc+1)!=0x80) return 0;
                                if (peek_byte_no_time(reg_pc+2)!=0x1f) return 0;


                                //Sea cual sea la rom, si reg_pc coincide e instruccion es la indicada antes
                                return 1;
		}
	
                if (MACHINE_IS_TSCONF) {
			//Como maquina +2A
			//Ver que instruccion sea ld hl,1f80
                                if (peek_byte_no_time(reg_pc)!=0x21) return 0;
                                if (peek_byte_no_time(reg_pc+1)!=0x80) return 0;
                                if (peek_byte_no_time(reg_pc+2)!=0x1f) return 0;


                                //Sea cual sea la rom, si reg_pc coincide e instruccion es la indicada antes
                                return 1;
		}


    if (MACHINE_IS_CHROME) {
			//Como maquina +2A
			//Ver que instruccion sea ld hl,1f80
                                if (peek_byte_no_time(reg_pc)!=0x21) return 0;
                                if (peek_byte_no_time(reg_pc+1)!=0x80) return 0;
                                if (peek_byte_no_time(reg_pc+2)!=0x1f) return 0;


                                //Sea cual sea la rom, si reg_pc coincide e instruccion es la indicada antes
                                return 1;
		}

		if (MACHINE_IS_TBBLUE) {
			//Como maquina +2A
                        //Ver que instruccion sea ld hl,1f80
                                if (peek_byte_no_time(reg_pc)!=0x21) return 0;
                                if (peek_byte_no_time(reg_pc+1)!=0x80) return 0;
                                if (peek_byte_no_time(reg_pc+2)!=0x1f) return 0;


                                //Sea cual sea la rom, si reg_pc coincide e instruccion es la indicada antes
                                return 1;
                }


                if (MACHINE_IS_ZXUNO_BOOTM_DISABLED) {
                                //ZX-Uno. Como maquina +2A

	                       //Ver que instruccion sea ld hl,1f80
                        	if (peek_byte_no_time(reg_pc)!=0x21) return 0;
                        	if (peek_byte_no_time(reg_pc+1)!=0x80) return 0;
	                        if (peek_byte_no_time(reg_pc+2)!=0x1f) return 0;


				//Sea cual sea la rom, si reg_pc coincide e instruccion es la indicada antes
                                return 1;
                }


        return 0;
}

int tap_load_detect_ace(void)
{

	if (tapefile==0) return 0;
        if ( (tape_loadsave_inserted & TAPE_LOAD_INSERTED)==0) return 0;


	if (reg_pc==0x18a7) return 1;
	return 0;
}

int tap_save_detect_ace(void)
{

        if (tape_out_file==0) return 0;
        if ( (tape_loadsave_inserted & TAPE_SAVE_INSERTED)==0) return 0;


        if (reg_pc==0x1820) return 1;
        return 0;
}

void gestionar_autoload_spectrum_start_cursorenter(void)
{
        debug_printf (VERBOSE_INFO,"Autoload tape with Space, Cursor up (twice) and ENTER");
        initial_tap_sequence=1;
        autoload_spectrum_loadpp_mode=3;
}


void gestionar_autoload_spectrum_start_loadpp(void)
{
        debug_printf (VERBOSE_INFO,"Autoload tape with LOAD \"\" ");
        initial_tap_sequence=1;
        autoload_spectrum_loadpp_mode=2;
}


void gestionar_autoload_spectrum_start_jloadpp(void)
{
	debug_printf (VERBOSE_INFO,"Autoload tape with LOAD(J) \"\" ");
	initial_tap_sequence=1;
	autoload_spectrum_loadpp_mode=1;
}

void gestionar_autoload_spectrum_start_enter(void)
{
        debug_printf (VERBOSE_INFO,"Autoload tape with ENTER");
        initial_tap_sequence=1;
        autoload_spectrum_loadpp_mode=0;
}


void gestionar_autoload_spectrum_48kmode(void)
{
	if (reg_pc==0x12a9) {
		gestionar_autoload_spectrum_start_jloadpp();
	}
}


void gestionar_autoload_spectrum(void)
{

	if (initial_tap_load.v==1 && initial_tap_sequence==0 &&
		( (tape_loadsave_inserted & TAPE_LOAD_INSERTED)!=0  || (realtape_inserted.v==1) )

		) {


		if (superupgrade_enabled.v) {
                                //Para superupgrade. Pasa como zxuno
                                //dado que no sabemos exactamente que maquina ha ejecutado superupgrade y por ejemplo,
                                //un spectrum 48k se carga en rom0 (y lo ideal seria que se cargase en rom3)
                                        //Para 128k, +2, +2a enviar enter
                                        if (
                                          reg_pc==0x3683 ||
                                          reg_pc==0x36a9 ||
                                          reg_pc==0x36be ||
                                          reg_pc==0x36bb ||
                                          reg_pc==0x1875 ||
                                          reg_pc==0x187a ||
                                          reg_pc==0x1891
                                        ) gestionar_autoload_spectrum_start_enter();

                                        //para spanish 128k
                                        else if (reg_pc==0x25a0) gestionar_autoload_spectrum_start_loadpp();

                                        //Para 48k
                                        else gestionar_autoload_spectrum_48kmode();

                 }



		int actual_rom;

		switch (current_machine_type) {

			case MACHINE_ID_SPECTRUM_16:
			case MACHINE_ID_SPECTRUM_48:
			case MACHINE_ID_INVES:
			case MACHINE_ID_MICRODIGITAL_TK90X:
			case MACHINE_ID_MICRODIGITAL_TK90X_SPA:
			case MACHINE_ID_MICRODIGITAL_TK95:
            case MACHINE_ID_SPECTRUM_48_PLUS_SPA:
            case MACHINE_ID_SPECTRUM_48_PLUS_ENG:
            case MACHINE_ID_TIMEX_TC2048:
				//Ver para maquinas 48k
				gestionar_autoload_spectrum_48kmode();
				break;

			case 6:
			case 21:
            case 22:
				//Para maquina 128k
				//si en rom0
				actual_rom=get_actual_rom_128k();
				if (actual_rom==0) {
					if (reg_pc==0x3683) gestionar_autoload_spectrum_start_enter();
				}

				//rom 1, la de 48k
				else gestionar_autoload_spectrum_48kmode();

				break;


			case 7:
				//Para maquina 128k spanish
				//si en rom0
                                actual_rom=get_actual_rom_128k();
                                if (actual_rom==0) {
                                        if (reg_pc==0x25a0) gestionar_autoload_spectrum_start_loadpp();
                                }

                                //rom 1, la de 48k
                                else gestionar_autoload_spectrum_48kmode();
				break;

			case 8:

				//Para maquina +2
				actual_rom=get_actual_rom_128k();
				if (actual_rom==0) {
					if (reg_pc==0x36a9) gestionar_autoload_spectrum_start_enter();
				}

				//rom 1, la de 48k
				else gestionar_autoload_spectrum_48kmode();


				break;

			case 9:

                                //Para maquina +2 french
                                actual_rom=get_actual_rom_128k();
                                if (actual_rom==0) {
                                        if (reg_pc==0x36be) gestionar_autoload_spectrum_start_enter();
                                }

                                //rom 1, la de 48k
                                else gestionar_autoload_spectrum_48kmode();


                                break;

                        case 10:

                                //Para maquina +2 spanish
                                actual_rom=get_actual_rom_128k();
                                if (actual_rom==0) {
                                        if (reg_pc==0x36bb) gestionar_autoload_spectrum_start_enter();
                                }

                                //rom 1, la de 48k
                                else gestionar_autoload_spectrum_48kmode();


                                break;


			case 11:

				//Para maquina +2A English rom 4.0
				actual_rom=get_actual_rom_p2a();
				if (actual_rom==0) {
					if (reg_pc==0x1875) gestionar_autoload_spectrum_start_enter();
				}

				else if (actual_rom==3) gestionar_autoload_spectrum_48kmode();

				break;

			case 12:

                                //Para maquina +2A English rom 4.1
                                actual_rom=get_actual_rom_p2a();
                                if (actual_rom==0) {
                                        if (reg_pc==0x187a) gestionar_autoload_spectrum_start_enter();
                                }

                                else if (actual_rom==3) gestionar_autoload_spectrum_48kmode();

                                break;


			case 13:

				//Para maquina +2A Spanish
				actual_rom=get_actual_rom_p2a();
				if (actual_rom==0) {
					if (reg_pc==0x1891) gestionar_autoload_spectrum_start_enter();
				}

				else if (actual_rom==3) gestionar_autoload_spectrum_48kmode();

				break;

			case 14:
				//Para ZX-Uno, bootm=0. como +2a
				//en zx-uno dado que no sabemos exactamente que maquina ha ejecutado zxuno y por ejemplo,
				//un spectrum 48k se carga en rom0 (y lo ideal seria que se cargase en rom3)
				if (ZXUNO_BOOTM_DISABLED) {
					//Para 128k, +2, +2a enviar enter
					if (
					  reg_pc==0x3683 ||
					  reg_pc==0x36a9 ||
					  reg_pc==0x36be ||
					  reg_pc==0x36bb ||
					  reg_pc==0x1875 ||
					  reg_pc==0x187a ||
					  reg_pc==0x1891
					) gestionar_autoload_spectrum_start_enter();

					//para spanish 128k
					else if (reg_pc==0x25a0) gestionar_autoload_spectrum_start_loadpp();

					//Para 48k
					else gestionar_autoload_spectrum_48kmode();

				}
				break;

			


			case 17:
				//Para Timex

				//Si rom mapeada en segmento bajo
                                if ( (timex_port_f4 &1)==0 ) {
					if (reg_pc==0x11f8) gestionar_autoload_spectrum_start_jloadpp();
				}
				break;


			case 18:
                                //Para Prism. Pasa como zxuno
                                //dado que no sabemos exactamente que maquina ha ejecutado prism y por ejemplo,
                                //un spectrum 48k se carga en rom0 (y lo ideal seria que se cargase en rom3)
                                        //Para 128k, +2, +2a enviar enter
                                        if (
                                          reg_pc==0x3683 ||
                                          reg_pc==0x36a9 ||
                                          reg_pc==0x36be ||
                                          reg_pc==0x36bb ||
                                          reg_pc==0x1875 ||
                                          reg_pc==0x187a ||
                                          reg_pc==0x1891
                                        ) gestionar_autoload_spectrum_start_enter();

                                        //para spanish 128k
                                        else if (reg_pc==0x25a0) gestionar_autoload_spectrum_start_loadpp();

                                        //Para 48k
                                        else gestionar_autoload_spectrum_48kmode();

                        break;

			case 19:
				//Para TBBlue. Pasa como Prism
				        //Para 128k, +2, +2a enviar enter
                        //Siempre que no este en la rom de arranque, 
                        //pues acaba creyendose 
                                //que esta en la rom del basic pues entra en reg_pc==0x12a9
                                if (tbblue_fast_boot_mode.v==0) {
                                        if (!tbblue_bootrom.v) {
                                                if (reg_pc==0x23f2) {
                                                        //Solo envio de cursor arriba dos veces, enter , en menu NextOS
                                                        //printf ("Sending autoload cursor up (2) + enter\n");
                                                        gestionar_autoload_spectrum_start_cursorenter();
                                                }
                                        }
                                }

                                else {
                                        //modo tbblue fast
                                                //printf ("gestionar como spectrum 48k\n");
                                                gestionar_autoload_spectrum_48kmode();
                                }

                                

          break;

			case 23:
                                //Para TSConf. Pasa como zxuno
                                //dado que no sabemos exactamente que maquina ha ejecutado prism y por ejemplo,
                                //un spectrum 48k se carga en rom0 (y lo ideal seria que se cargase en rom3)
                                        //Para 128k, +2, +2a enviar enter
                                        if (
                                          reg_pc==0x3683 ||
                                          reg_pc==0x36a9 ||
                                          reg_pc==0x36be ||
                                          reg_pc==0x36bb ||
                                          reg_pc==0x1875 ||
                                          reg_pc==0x187a ||
                                          reg_pc==0x1891
                                        ) gestionar_autoload_spectrum_start_enter();

                                        //para spanish 128k
                                        else if (reg_pc==0x25a0) gestionar_autoload_spectrum_start_loadpp();

                                        //Para 48k
                                        else gestionar_autoload_spectrum_48kmode();

                        break;

		}


	}


}

void gestionar_autoload_sam(void)
{

        if (initial_tap_load.v==1 && initial_tap_sequence==0 &&
                ( (tape_loadsave_inserted & TAPE_LOAD_INSERTED)!=0  || (realtape_inserted.v==1) )

                ) {


                        if (reg_pc==0xd5d1) {
                                debug_printf (VERBOSE_INFO,"Autoload tape with LOAD");
                                initial_tap_sequence=1;
                        }
        }

}

void gestionar_autoload_cpc(void)
{

        if (initial_tap_load.v==1 && initial_tap_sequence==0 &&
                ( (tape_loadsave_inserted & TAPE_LOAD_INSERTED)!=0  || (realtape_inserted.v==1) )

                ) {


			if (reg_pc==0x1a4f) {
        			debug_printf (VERBOSE_INFO,"Autoload tape with CTRL+Enter");
			        initial_tap_sequence=1;
			}
	}

}





FILE *ptr_realtape;
char realtape_last_value;

//2 para ZX81 mejor
//0 para spectrum mejor
char realtape_volumen=0;

//#define FREQ_SMP 11111

//int contador=FRECUENCIA_SONIDO;

z80_bit realtape_inserted={0};
z80_bit realtape_playing={0};
z80_bit realtape_loading_sound={1};

//Archivo temporal
//FILE *ptr_realtape_rwa;
char realtape_name_rwa[PATH_MAX];


char realtape_wave_offset=0;

//0= RWA - raw tal cual
//1= SMP
//2= WAV
//3= TZX
//4= P
//5= O
//6= TAP
//7= PZX
int realtape_tipo=0;


//ajusta un valor de sample de unsigned char a char, teniendo en cuenta offset de onda
char realtape_adjust_offset_sign(unsigned char value)
{
	//pasamos a int con mayor precision para controlar topes
	int value16=value;
	value16 = value16-128+realtape_wave_offset;

	if (value16>127) value16=127;
	if (value16<-128) value16=-128;

	return value16;
}

//Para mostrar indicador de progreso cargado
long int realtape_file_size=0;
long int realtape_file_size_counter=0;

void realtape_get_byte_rwa(void)
{


	if (feof(ptr_realtape)) {
		realtape_eject();
        return;
    }


    silence_detection_counter=0;
    //beeper_silence_detection_counter=0;
	unsigned char valor_leido_audio;

	fread(&valor_leido_audio, 1,1 , ptr_realtape);
	realtape_last_value=realtape_adjust_offset_sign(valor_leido_audio);
    realtape_file_size_counter++;
}

void realtape_get_byte_cont(void)
{

    //RWA, SMP, WAV, TZX, P, O, TAP, PZX
	if (realtape_tipo>=0 || realtape_tipo<=7) {
		realtape_get_byte_rwa();
		return;
	}

}

//Para animar el caracter que se mueve
int realtape_print_footer_last_char=0;

int realtape_get_elapsed_percentage(void)
{
    long int total=realtape_file_size;
    long int transcurrido=realtape_file_size_counter;

    int progreso;

    if (total==0) progreso=100;
    else progreso=(transcurrido*100)/total;

    if (progreso>100) progreso=100;

    return progreso;
}

//Convierte una cantidad de bytes leidos en cuanto tiempo representa
int realtape_get_seconds_numbytes(long int numero)
{
    //15600 hz, 8 bit
    return numero/15600;
}

int realtape_get_elapsed_seconds(void)
{
    return realtape_get_seconds_numbytes(realtape_file_size_counter);

}

int realtape_get_total_seconds(void)
{
    return realtape_get_seconds_numbytes(realtape_file_size);

}

void realtape_print_footer(void)
{
    if (realtape_inserted.v==0 || realtape_playing.v==0) return;

    int progreso=realtape_get_elapsed_percentage();

    debug_printf (VERBOSE_DEBUG,"RealTape loading progress: %d %%",progreso);

    char buffer_texto_playing[33];
    char buffer_texto_progreso[33];
    char buffer_texto[33];

                                    //01234567890123456789012345678901
    sprintf (buffer_texto_playing,"RealTape Playing %3d%%",progreso);
    //Con indicador de progreso. 10 posiciones
    int posicion_progreso=progreso%10;

    char loading_character;

    if (realtape_print_footer_last_char==0) loading_character='o';
    else loading_character='O';

    realtape_print_footer_last_char ^=1;

    int i;
    for (i=0;i<10;i++) {
            buffer_texto_progreso[i]='.';
            if (i==posicion_progreso) buffer_texto_progreso[i]=loading_character;
    }
    buffer_texto_progreso[i]=0;

    sprintf (buffer_texto,"%s %s",buffer_texto_playing,buffer_texto_progreso);

	//color inverso
	menu_putstring_footer(0,2,buffer_texto,WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);

    //Y poner icono de cinta en inverso
    draw_realtape_icon_activity();        
}

//comun para ambos. si accion=1, es avanzar
void realtape_rewind_ffwd_common(int accion,int porcentaje)
{
    if (realtape_inserted.v==0) {
        debug_printf(VERBOSE_ERR,"No real tape inserted");
        return;
    }
    
    long int total=realtape_file_size;
    long int transcurrido=realtape_file_size_counter;

    //tenemos precisamente lo transcurrido asi que no hay que obtener la posicion con fget
    //cuanto es 5% del total
    long int offset=(total*porcentaje)/100;

    if (accion) {
        //avanzar
        transcurrido +=offset;
    }
    else {
        //rebobinar
        transcurrido -=offset;        
    }

    if (transcurrido>=total) transcurrido=total-1;
    if (transcurrido<0) transcurrido=0;

    realtape_file_size_counter=transcurrido;

    fseek(ptr_realtape, transcurrido, SEEK_SET);    
}

//Rebobinar al principio la cinta
void realtape_rewind_begin(void)
{
    if (realtape_inserted.v==0) {
        debug_printf(VERBOSE_ERR,"No real tape inserted");
        return;
    }


    long int transcurrido=0;

    realtape_file_size_counter=transcurrido;

    fseek(ptr_realtape, transcurrido, SEEK_SET);    
}

//rebobina 5%
void realtape_rewind_five(void)
{
    realtape_rewind_ffwd_common(0,5);
}

//avanza 5%
void realtape_ffwd_five(void)
{
    realtape_rewind_ffwd_common(1,5);
}

//rebobina 1%
void realtape_rewind_one(void)
{
    realtape_rewind_ffwd_common(0,1);
}

//avanza 1%
void realtape_ffwd_one(void)
{
    realtape_rewind_ffwd_common(1,1);
}

void realtape_delete_footer(void)
{
                           //01234567890123456789012345678901
  menu_putstring_footer(0,2,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
  menu_footer_bottom_line();


        //Quitar footer y icono en inverso
        delete_generic_footertext();  
}

void realtape_get_byte(void)
{

	realtape_get_byte_cont();

}


//almacenar los datos que se veran en la ventana de visual real tape. Valores maximo y minimo ([0] minimo, [1] maximo)
//Un minimo de REALTAPE_VISUAL_MAX_SIZE y maximo REALTAPE_VISUAL_MAX_SIZE*2
z80_byte realtape_visual_data[REALTAPE_VISUAL_MAX_SIZE*2][2];

int realtape_visual_total_used=REALTAPE_VISUAL_MAX_SIZE;

char visual_realtape_textbrowse[MAX_TEXTO_BROWSER];

#define VISUAL_REALTAPE_MAX_POSITIONS 256
long visual_realtape_array_positions[VISUAL_REALTAPE_MAX_POSITIONS];

//0=unknown
//1= spectrum
//2= zx80
//3= zx81
int realtape_visual_detected_tape_type=0;

void init_visual_real_tape(void)
{
    //Ponerlo todo a onda plana
    int i;
    for (i=0;i<REALTAPE_VISUAL_MAX_SIZE*2;i++) {
        realtape_visual_data[i][0]=realtape_visual_data[i][1]=128;
    }    

    //Inicializar array de posiciones
    visual_realtape_array_positions[0]=-1;
}

void realtape_load_visuals(char *filename)
{

    //vaciar de nuevo por si al cargar es un archivo menor que REALTAPE_VISUAL_MAX_SIZE (cosa rara) y quedan "restos" de lo anterior
    init_visual_real_tape();

    long int total_archivo=get_file_size(filename);

    //Hacer trocitos de total_archivo/REALTAPE_VISUAL_MAX_SIZE de maximo
    //4096: trozos de 1 samples
    //8192: trozos de 2 samples

    int tamanyo_trozo=total_archivo/REALTAPE_VISUAL_MAX_SIZE;

    //si muy pequeño
    if (tamanyo_trozo==0) tamanyo_trozo=1;

    //Definimos el total usado real remultiplicando el resultado
    int realtape_visual_total_used=tamanyo_trozo*REALTAPE_VISUAL_MAX_SIZE;

    FILE *ptr_visual;
    ptr_visual=fopen(filename,"rb");


    if (!ptr_visual) {
        debug_printf(VERBOSE_ERR,"Unable to open realtape file %s",filename);
        return;
    }

    //-r 15600 -b 8 -e unsigned -c 1

    int minimo,maximo;
    int leidos=0;
    int posicion_visual=0;

    minimo=maximo=128;

    while (total_archivo>0) {
        z80_byte byte_leido;
        fread(&byte_leido,1,1,ptr_visual);

        //acumulado=acumulado+byte_leido;
        if (byte_leido<minimo) minimo=byte_leido;
        if (byte_leido>maximo) maximo=byte_leido;

        total_archivo--;
        leidos++;

        if ((leidos%tamanyo_trozo)==0) {
            //siguiente trozo
            //acumulado /=tamanyo_trozo;

            //por si acaso controlar maximo
            if (posicion_visual<realtape_visual_total_used) {
                //printf("Writing position %4d value min %3d max %3d\n",posicion_visual,minimo,maximo);
                realtape_visual_data[posicion_visual][0]=minimo;
                realtape_visual_data[posicion_visual][1]=maximo;

                posicion_visual++;
            }
            else {
                //printf("Trying to write beyond realtape visual: %d\n",posicion_visual);
                return;
            }

            minimo=maximo=128;

        }
    }

    

    fclose(ptr_visual);
    
}

void realtape_insert(void)
{

	debug_printf (VERBOSE_INFO,"Inserting real tape: %s",realtape_name);
        realtape_file_size_counter=0;

	if (!util_compare_file_extension(realtape_name,"rwa")) {
		debug_printf (VERBOSE_INFO,"Detected raw file RWA");
		realtape_tipo=0;
		debug_printf (VERBOSE_INFO,"Opening File %s",realtape_name);
        	ptr_realtape=fopen(realtape_name,"rb");
                realtape_file_size=get_file_size(realtape_name);
	}
	else if (!util_compare_file_extension(realtape_name,"smp")) {
		debug_printf (VERBOSE_INFO,"Detected raw file SMP");
		realtape_tipo=1;
		if (convert_smp_to_rwa_tmpdir(realtape_name,realtape_name_rwa)) {
			//debug_printf(VERBOSE_ERR,"Error converting input file");
			return;
		}

		if (!si_existe_archivo(realtape_name_rwa)) {
			debug_printf(VERBOSE_ERR,"Error converting input file. Target file not found");
			return;
		}

		debug_printf (VERBOSE_INFO,"Opening File %s",realtape_name_rwa);
        	ptr_realtape=fopen(realtape_name_rwa,"rb");
                realtape_file_size=get_file_size(realtape_name_rwa);
	}

        else if (!util_compare_file_extension(realtape_name,"wav")) {
                debug_printf (VERBOSE_INFO,"Detected WAV file");
                realtape_tipo=2;
		if (convert_wav_to_rwa_tmpdir(realtape_name,realtape_name_rwa)) {
			//debug_printf(VERBOSE_ERR,"Error converting input file");
                        return;
                }

		if (!si_existe_archivo(realtape_name_rwa)) {
			debug_printf(VERBOSE_ERR,"Error converting input file. Target file not found");
			return;
		}
		debug_printf (VERBOSE_INFO,"Opening File %s",realtape_name_rwa);
                ptr_realtape=fopen(realtape_name_rwa,"rb");
                realtape_file_size=get_file_size(realtape_name_rwa);
        }

        else if (!util_compare_file_extension(realtape_name,"tzx") ||
		 !util_compare_file_extension(realtape_name,"cdt")

		) {
                debug_printf (VERBOSE_INFO,"Detected TZX file");
                realtape_tipo=3;
                if (convert_tzx_to_rwa_tmpdir(realtape_name,realtape_name_rwa)) {
			//debug_printf(VERBOSE_ERR,"Error converting input file");
                        return;
                }

		if (!si_existe_archivo(realtape_name_rwa)) {
			debug_printf(VERBOSE_ERR,"Error converting input file. Target file not found");
			return;
		}

		debug_printf (VERBOSE_INFO,"Opening File %s",realtape_name_rwa);
                ptr_realtape=fopen(realtape_name_rwa,"rb");
                realtape_file_size=get_file_size(realtape_name_rwa);
        }

        else if (!util_compare_file_extension(realtape_name,"p")) {
                debug_printf (VERBOSE_INFO,"Detected P file");
                realtape_tipo=4;
                if (convert_p_to_rwa_tmpdir(realtape_name,realtape_name_rwa)) {
                        //debug_printf(VERBOSE_ERR,"Error converting input file");
                        return;
                }

                if (!si_existe_archivo(realtape_name_rwa)) {
                        debug_printf(VERBOSE_ERR,"Error converting input file. Target file not found");
                        return;
                }

		debug_printf (VERBOSE_INFO,"Opening File %s",realtape_name_rwa);
                ptr_realtape=fopen(realtape_name_rwa,"rb");
                realtape_file_size=get_file_size(realtape_name_rwa);
        }

        else if (!util_compare_file_extension(realtape_name,"o")) {
                debug_printf (VERBOSE_INFO,"Detected O file");
                realtape_tipo=5;
                if (convert_o_to_rwa_tmpdir(realtape_name,realtape_name_rwa)) {
                        //debug_printf(VERBOSE_ERR,"Error converting input file");
                        return;
                }

                if (!si_existe_archivo(realtape_name_rwa)) {
                        debug_printf(VERBOSE_ERR,"Error converting input file. Target file not found");
                        return;
                }

                ptr_realtape=fopen(realtape_name_rwa,"rb");
                realtape_file_size=get_file_size(realtape_name_rwa);
        }

    

        else if (!util_compare_file_extension(realtape_name,"tap")) {
                debug_printf (VERBOSE_INFO,"Detected TAP file");
                realtape_tipo=6;
                if (convert_tap_to_rwa_tmpdir(realtape_name,realtape_name_rwa)) {
                        //debug_printf(VERBOSE_ERR,"Error converting input file");
                        return;
                }

                if (!si_existe_archivo(realtape_name_rwa)) {
                        debug_printf(VERBOSE_ERR,"Error converting input file. Target file not found");
                        return;
                }

		debug_printf (VERBOSE_INFO,"Opening File %s",realtape_name_rwa);
                ptr_realtape=fopen(realtape_name_rwa,"rb");
                realtape_file_size=get_file_size(realtape_name_rwa);
        }


        else if (!util_compare_file_extension(realtape_name,"pzx")) {
                debug_printf (VERBOSE_INFO,"Detected PZX file");
                realtape_tipo=7;
                if (convert_pzx_to_rwa_tmpdir(realtape_name,realtape_name_rwa)) {
                        //debug_printf(VERBOSE_ERR,"Error converting input file");
                        return;
                }

                if (!si_existe_archivo(realtape_name_rwa)) {
                        debug_printf(VERBOSE_ERR,"Error converting input file. Target file not found");
                        return;
                }

                ptr_realtape=fopen(realtape_name_rwa,"rb");
                realtape_file_size=get_file_size(realtape_name_rwa);
        }    




	else {
        debug_printf (VERBOSE_ERR,"Unknown input tape type");
        realtape_eject();
        return;
    }



	realtape_stop_playing();
	realtape_inserted.v=1;

    char *name_to_use;

    if (realtape_tipo==0) {
        name_to_use=realtape_name;
    }
    else {
        name_to_use=realtape_name_rwa;
    }

    realtape_load_visuals(name_to_use);



    //precargar posiciones de cada bloque en cinta para luego mostrar en Visual Real Tape
    realtape_visual_detected_tape_type=0; //de momento tipo desconocido
    int codigo_retorno;
    util_realtape_browser(name_to_use, visual_realtape_textbrowse,MAX_TEXTO_BROWSER,NULL, 
        visual_realtape_array_positions, VISUAL_REALTAPE_MAX_POSITIONS,&codigo_retorno);

    if (codigo_retorno) {
        debug_printf(VERBOSE_INFO,"Error trying to convert audio to Spectrum Tape Blocks. Probably invalid carry in some blocks");
        //de momento no tratamos el error
    }

    if (visual_realtape_textbrowse[0]==0) {
        //printf("Intentamos conversion ZX80/81\n");
        //Intentamos con conversión ZX80/81

        //TODO: no autodetectara cintas de ZX80, hay que seleccionar maquina ZX80 como actual para que el conversor asuma ZX80 
        convert_realtape_to_po(name_to_use, NULL, visual_realtape_textbrowse,0);
        //printf("texto: %s\n",visual_realtape_textbrowse);
        //array de posiciones con un solo bloque
        visual_realtape_array_positions[0]=0;
        visual_realtape_array_positions[1]=-1;


        //si el texto empieza con "ZX80 Tape", es ZX80
        if (visual_realtape_textbrowse[0]=='Z' &&
            visual_realtape_textbrowse[1]=='X' &&
            visual_realtape_textbrowse[2]=='8' &&
            visual_realtape_textbrowse[3]=='0') {
             
            realtape_visual_detected_tape_type=2;
        }

        //si el texto empieza con "ZX81 Tape", es ZX81
        if (visual_realtape_textbrowse[0]=='Z' &&
            visual_realtape_textbrowse[1]=='X' &&
            visual_realtape_textbrowse[2]=='8' &&
            visual_realtape_textbrowse[3]=='1') {
             
            realtape_visual_detected_tape_type=3;
        }        

    }

    else {
        //cinta spectrum
        realtape_visual_detected_tape_type=1;
    }


    


	//Activamos realvideo para que:
	//En Spectrum, se vean las franjas de carga en el borde
	//En ZX80,81, se vean las franjas de carga en toda la pantalla

	//Aun asi esto solo es un motivo estetico, se puede desactivar realvideo y la carga funcionara igualmente
	//Solo si autodetect real video esta activo
    //y maquina no es cpc (carga normal no cambia border)
	if (autodetect_rainbow.v && !MACHINE_IS_CPC) {
        debug_printf (VERBOSE_INFO,"Enabling realvideo due to real tape insert");
        enable_rainbow();
    }


    //if (noautoload.v==0 && !MACHINE_IS_TBBLUE) { //TODO: desactivamos autoload en TBBLUE
    if (noautoload.v==0) { 
        debug_printf (VERBOSE_INFO,"Restarting autoload");
        initial_tap_load.v=1;
        initial_tap_sequence=0;

		//Inicia play en cualquier maquina menos en CPC, dado que CPC controla el motor ella sola
		if (!MACHINE_IS_CPC) realtape_start_playing();

        //si esta autoload, tambien hacer reset para que luego se haga load automaticamente
        debug_printf (VERBOSE_INFO,"Reset cpu due to autoload");
        reset_cpu();

		//Activamos top speed si conviene
		if (fast_autoload.v) {
            debug_printf (VERBOSE_INFO,"Set top speed");
            top_speed_timer.v=1;                
        }

    }


}

void realtape_eject(void)
{
	if (realtape_inserted.v) {
        realtape_stop_playing();
        realtape_inserted.v=0;

		if (ptr_realtape!=NULL) {
			fclose (ptr_realtape);
			ptr_realtape=NULL;
		}
        
        realtape_delete_footer();
	}
}


void realtape_start_playing(void)
{
	if (realtape_playing.v==0) {
		realtape_playing.v=1;
		draw_realtape_text();
		//no quitar texto de TAPE
		tape_loading_counter=9999999;
	}
}

void realtape_stop_playing(void)
{
	if (realtape_playing.v==1) {
		realtape_playing.v=0;
		//quitar texto de tape en 1 segundo
		tape_loading_counter=1;
	}
}


void realtape_pause_unpause(void)
{
        if (realtape_playing.v) realtape_stop_playing();
        else realtape_start_playing();
}

//Rutinas de autodeteccion de rutinas de carga


/* loader.c: loader detection
   Copyright (c) 2006 Philip Kendall

   $Id: loader.c 3941 2009-01-09 22:38:21Z pak21 $

   Author contact information:

   E-mail: philip-fuse@shadowmagic.org.uk

*/

/*static int successive_reads = 0;
static int last_tstates_read = -100000;
static z80_byte last_b_read = 0x00;
static int length_known1 = 0, length_known2 = 0;
static int length_long1 = 0, length_long2 = 0;
*/


acceleration_mode_t acceleration_mode;
size_t acceleration_pc;

/*void
loader_frame( int frame_length )
{
  if( last_tstates_read > -100000 ) {
    last_tstates_read -= frame_length;
  }
}

void
loader_tape_play( void )
{
  successive_reads = 0;
  acceleration_mode = ACCELERATION_MODE_NONE;
}

void
loader_tape_stop( void )
{
  successive_reads = 0;
  acceleration_mode = ACCELERATION_MODE_NONE;
}
*/

/*
static void
do_acceleration( void )
{

//TODO. Que intenta hacer aqui con estas modificaciones de registros?

  if( length_known1 ) {
    int set_b_high = length_long1;
    set_b_high ^= ( acceleration_mode == ACCELERATION_MODE_DECREASING );
    if( set_b_high ) {
      reg_b = 0xfe;
    } else {
      reg_b = 0x00;
    }
    reg_a |= 0x01;
    //z80.pc.b.l = peek_byte_no_time( z80.sp.w ); reg_sp++;
    //z80.pc.b.h = peek_byte_no_time( z80.sp.w ); reg_sp++;
	reg_pc=peek_word_no_time(reg_sp);
	reg_sp+=2;

    //event_remove_type( tape_edge_event );
    //tape_next_edge( tstates, 0, NULL );

    successive_reads = 0;
  }

  length_known1 = length_known2;
  length_long1 = length_long2;
}
*/

acceleration_mode_t
acceleration_detector( z80_int pc )
{
  int state = 0, count = 0;
  while( 1 ) {
    z80_byte b = peek_byte_no_time( pc ); pc++; count++;
    switch( state ) {
    case 0:
      switch( b ) {
      case 0x04: state = 1; break;	/* INC B - Many loaders */
      default: state = 13; break;	/* Possible Digital Integration */
      }
      break;
    case 1:
      switch( b ) {
      case 0xc8: state = 2; break;	/* RET Z */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 2:
      switch( b ) {
      case 0x3e: state = 3; break;	/* LD A,nn */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 3:
      switch( b ) {
      case 0x00:			/* Search Loader */
      case 0x7f:			/* ROM loader and variants */
	state = 4; break;		/* Data byte */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 4:
      switch( b ) {
      case 0xdb: state = 5; break;	/* IN A,(nn) */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 5:
      switch( b ) {
      case 0xfe: state = 6; break;	/* Data byte */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 6:
      switch( b ) {
      case 0x1f: state = 7; break;	/* RRA */
      case 0xa9: state = 24; break;	/* XOR C - Search Loader */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 7:
      switch( b ) {
      case 0x00:			/* NOP - Bleepload */
      case 0xa7:			/* AND A - Microsphere */
      case 0xc8:			/* RET Z - Paul Owens */
      case 0xd0:			/* RET NC - ROM loader */
	state = 8; break;
      case 0xa9: state = 9; break;	/* XOR C - Speedlock */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 8:
      switch( b ) {
      case 0xa9: state = 9; break;	/* XOR C */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 9:
      switch( b ) {
      case 0xe6: state = 10; break;	/* AND nn */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 10:
      switch( b ) {
      case 0x20: state = 11; break;	/* Data byte */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 11:
      switch( b ) {
      case 0x28: state = 12; break;	/* JR nn */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 12:
      if( b == 0x100 - count ) {
	return ACCELERATION_MODE_INCREASING;
      } else {
	return ACCELERATION_MODE_NONE;
      }
      break;

      /* Digital Integration loader */

    case 13:
      state = 14; break;		/* Possible Digital Integration */
    case 14:
      switch( b ) {
      case 0x05: state = 15; break;	/* DEC B - Digital Integration */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 15:
      switch( b ) {
      case 0xc8: state = 16; break;	/* RET Z */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 16:
      switch( b ) {
      case 0xdb: state = 17; break;	/* IN A,(nn) */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 17:
      switch( b ) {
      case 0xfe: state = 18; break;	/* Data byte */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 18:
      switch( b ) {
      case 0xa9: state = 19; break;	/* XOR C */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 19:
      switch( b ) {
      case 0xe6: state = 20; break;	/* AND nn */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 20:
      switch( b ) {
      case 0x40: state = 21; break;	/* Data byte */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 21:
      switch( b ) {
      case 0xca: state = 22; break;	/* JP Z,nnnn */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 22:				/* LSB of jump target */
      if( b == ( reg_pc - 4 ) % 0x100 ) {
	state = 23;
      } else {
	return ACCELERATION_MODE_NONE;
      }
      break;
    case 23:				/* MSB of jump target */
      if( b == ( reg_pc - 4 ) / 0x100 ) {
	return ACCELERATION_MODE_DECREASING;
      } else {
	return ACCELERATION_MODE_NONE;
      }

      /* Search loader */

    case 24:
      switch( b ) {
      case 0xe6: state = 25; break;	/* AND nn */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 25:
      switch( b ) {
      case 0x40: state = 26; break;	/* Data byte */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 26:
      switch( b ) {
      case 0xd8: state = 27; break;	/* RET C */
      default: return ACCELERATION_MODE_NONE;
      }
      break;
    case 27:
      switch( b ) {
      case 0x00: state = 11; break;	/* NOP */
      default: return ACCELERATION_MODE_NONE;
      }
      break;

    default:
      /* Can't happen */
      break;
    }
  }

}


void tape_check_known_loaders(void)
{
                          /* If the IN occured at a different location to the one we're
                             accelerating, stop acceleration */
                          if( acceleration_mode && reg_pc != acceleration_pc )
                            acceleration_mode = ACCELERATION_MODE_NONE;

                          /* If we're not accelerating, check if this is a loader */
                          if( !acceleration_mode ) {
                            acceleration_mode = acceleration_detector( reg_pc - 6 );
                            acceleration_pc = reg_pc;
                          }
}



void detectar_conocidos(void)
{

	tape_check_known_loaders();

  if( acceleration_mode ) {
        //printf ("modo aceleracion\n");
        //do_acceleration();

        //Si hemos llegado aqui, es que hay cinta standard. Meter cinta real
                //printf ("tipo aceleracion: %d\n",acceleration_mode);
                //printf ("registro PC: %d\n",reg_pc);


	//Death Wish 3 (Erbe - Serie Leyenda).tzx -  (DeathWish3(IBSA).tzx.zip)
	//tiene rutina de carga propia pero llama a las rutinas de la rom, por tanto,
	//esta deteccion salta cuando pc=1523
	//y por tanto no restringir solo a detecciones mas alla de la 16384
        //if (reg_pc>=16384) {
		char buffer_mensaje[100];
		sprintf (buffer_mensaje,"Detected custom loader routine at address %d. Reinserting tape as Real Tape",reg_pc);
		debug_printf (VERBOSE_INFO,buffer_mensaje);
                screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);


                //Meter como cinta real. Nos guardamos nombre
		sprintf (menu_realtape_name,"%s",tapefile);
		realtape_name=menu_realtape_name;

        	//Expulsar cinta insertada
	        eject_tape_load();
        	tapefile=NULL;

                //Y metemos cinta real
                realtape_insert();
        //}

  }

  else {
        //printf ("no modo aceleracion\n");
  }
}

void tape_detectar_realtape(void)
{
    detectar_conocidos();
}

/*
static void
check_for_acceleration( void )
{
  // If the IN occured at a different location to the one we're
  //   accelerating, stop acceleration
  if( acceleration_mode && reg_pc != acceleration_pc )
    acceleration_mode = ACCELERATION_MODE_NONE;

  // If we're not accelerating, check if this is a loader
  if( !acceleration_mode ) {
    acceleration_mode = acceleration_detector( reg_pc - 6 );
    acceleration_pc = reg_pc;
  }

  if( acceleration_mode ) {
        //printf ("modo aceleracion\n");
        do_acceleration();
        if (porcentaje_velocidad_emulador==100) {
                //Esto fuse lo hace diferente. De alguna manera acelera "al maximo" la cpu
                //y no de la misma manera que lo hago yo (mediante porcentaje)
                porcentaje_velocidad_emulador=1000;
                set_emulator_speed();
                screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Speeding up Z80 Core on loading");
        }
  }

  else {
        //printf ("no modo aceleracion\n");
        if (porcentaje_velocidad_emulador!=100) {
                porcentaje_velocidad_emulador=100;
                set_emulator_speed();
        }

  }


}

*/

/*
void
loader_detect_loader( void )
{
  int tstates_diff = t_estados - last_tstates_read;
  z80_byte b_diff = reg_b - last_b_read;

  last_tstates_read = t_estados;
  last_b_read = reg_b;

  if( autodetect_loaders.v ) {

    if( realtape_playing.v ) {
      if( tstates_diff > 1000 || ( b_diff != 1 && b_diff != 0 &&
				   b_diff != 0xff ) ) {
	successive_reads++;
	if( successive_reads >= 2 ) {
	  //temp no parar cinta
	  realtape_stop_playing();
	}
      } else {
	successive_reads = 0;
      }
    } else {
      if( tstates_diff <= 500 && ( b_diff == 1 || b_diff == 0xff ) ) {
	successive_reads++;
	if( successive_reads >= 10 ) {
	  realtape_start_playing();
	}
      } else {
	successive_reads = 0;
      }
    }

  }
   else {

    successive_reads = 0;

  }

  if( accelerate_loaders.v && realtape_playing.v )
    check_for_acceleration();

}
*/

/*
void
loader_set_acceleration_flags( int flags )
{
  if( flags & LIBSPECTRUM_TAPE_FLAGS_LENGTH_SHORT ) {
    length_known2 = 1;
    length_long2 = 0;
  } else if( flags & LIBSPECTRUM_TAPE_FLAGS_LENGTH_LONG ) {
    length_known2 = 1;
    length_long2 = 1;
  } else {
    length_known2 = 0;
  }
}
*/




char realtape_previous_value=0;
char realtape_previous_return_value=0;
int realtape_algorithm_new_noise_reduction=0;

//Retorna el siguiente bit de cinta realtape, si es 1 o 0
int realtape_get_current_bit_playing(void)
{
    
    if (realtape_algorithm_new.v) {

        //acevaders no carga con este metodo a no ser que se tenga noise reduction a 20 o algo mas
        //orquesta no carga con este metodo a no ser que se tenga noise reduction a 2 o algo mas

        //realmente no hace falta inicializarlo a 0 pues siempre retornara valor,
        //solo es para el compilador para que no se queje
        char return_value=0;

        //sacar diferencia valor anterior con actual
        int diferencia=realtape_last_value-realtape_previous_value;

        if (diferencia<0) diferencia=-diferencia;

        //Si la onda esta mas o menos igual, damos valor anterior
        if (diferencia<=realtape_algorithm_new_noise_reduction) {
            //printf("igual\n");
            return_value=realtape_previous_return_value;            
        }

        else {

            //Si la onda "sube", es +1
            if (realtape_last_value>realtape_previous_value) {
                //printf ("superior\n");
                return_value=1;
            }
            //Si la onda "baja", es -1
            else if (realtape_last_value<realtape_previous_value) {
                return_value=0;
                //printf("inferior\n");
            }

            //Si la onda esta igual, damos valor anterior
            //else {
            //    printf("igual\n");
            //    return_value=realtape_previous_return_value;
            //}


        }

        realtape_previous_value=realtape_last_value;
        realtape_previous_return_value=return_value;

        //printf("retornar %d\n",return_value);

        return return_value;

    }

    else {
                 
        //Por ejemplo, al grabar con spectrum con audio to file, y no activar el "rom save filter", no carga con este algorimo,
        //pues la onda no está centrada en 0


        if (realtape_last_value>=realtape_volumen) {
            //printf ("1 \n");
            return 1;
            
        }
        else {
            //printf ("0 \n");
            return 0;
        }
    }
                
}