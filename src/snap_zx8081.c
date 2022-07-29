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
#include <unistd.h>                
#include <string.h>
#include <dirent.h>

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "cpu.h"
#include "debug.h"
#include "operaciones.h"
#include "zx8081.h"
#include "audio.h"
#include "screen.h"
#include "tape.h"
#include "snap_zx8081.h"

#include "tape_smp.h"
#include "snap_z81.h"
#include "zxvision.h"
#include "snap.h"
#include "utils.h"

FILE *ptr_zx8081file;

//nombre de cinta insertada mediante LOAD "nombre" en zx81
char nombre_cinta_load_nombre_81[PATH_MAX];


//Si desactivamos tape traps del zx80/81
z80_bit zx8081_disable_tape_traps={0};

/* Inicio funciones principales */

//Funcion de deteccion de ROM para la carga de ZX80 y ZX81
int new_tap_load_detect_zx8081(void)
{
        if (MACHINE_IS_ZX80) return new_tap_load_detect_zx80();
        else return new_tap_load_detect_zx81();

}

//Funcion de deteccion de ROM para la grabacion de ZX80 y ZX81
int new_tap_save_detect_zx8081(void)
{
        if (MACHINE_IS_ZX80) return new_tap_save_detect_zx80();
        else return new_tap_save_detect_zx81();
}


//Funcion de carga de cinta entrando desde ROM para ZX80
//En ZX80 no existe variable RAMTOP del sistema
void new_tape_load_zx80(void)
{

    debug_printf (VERBOSE_INFO,"Loading tape %s",tapefile);

    //Si es SMP


    if (!util_compare_file_extension(tapefile,"rwa") || 
        !util_compare_file_extension(tapefile,"smp") || 
        !util_compare_file_extension(tapefile,"wav")

    ) {
        debug_printf (VERBOSE_INFO,"Tape is raw audio");
        new_snap_load_zx80_smp(tapefile);
    }


    else if (!util_compare_file_extension(tapefile,"z81")) {

        debug_printf (VERBOSE_INFO,"Assume z81 snapshot is ZX81. We will hotswap later to ZX80 if needed");
        current_machine_type=121;

        set_machine(NULL);
        reset_cpu();

        snap_load_zx80_zx81_load_z81_file(tapefile);
    }



    //Si es .O
    else {

        debug_printf (VERBOSE_INFO,"Assume format is .o/.80");

        //Cargamos archivo en memoria
        new_load_zx80_o_snapshot_in_mem(tapefile);

        //Y volvemos control al BASIC
        new_set_return_saveload_zx80();
    }


}


//Funcion de carga de cinta entrando desde ROM para ZX81
void new_tape_load_zx81(void)
{
    z80_int variable_ramtop;


    //ajusta variable ramtop del sistema si hay algun pack activo
    set_ramtop_with_rampacks();

    variable_ramtop=value_8_to_16(memoria_spectrum[0x4005],memoria_spectrum[0x4004]);


    debug_printf (VERBOSE_INFO,"Loading tape %s. RAMTOP=%d",tapefile,variable_ramtop);


    if (!util_compare_file_extension(tapefile,"rwa") || 
        !util_compare_file_extension(tapefile,"smp") || 
        !util_compare_file_extension(tapefile,"wav")
    ) {
        debug_printf (VERBOSE_INFO,"Tape is raw audio");
        new_snap_load_zx81_smp(tapefile);
    }

    else if (!util_compare_file_extension(tapefile,"z81")) {
        debug_printf (VERBOSE_INFO,"Assume z81 snapshot is ZX81. We will hotswap later to ZX80 if needed");
        snap_load_zx80_zx81_load_z81_file(tapefile);
        //los registros ya nos vienen indicados en el snapshot .z81
        //volver sin ejecutar el snap_load_save_zx80_zx81_setregisters_ramtop del final
        return;
    }

    else {
        debug_printf (VERBOSE_INFO,"Assume format is .p/.81");
        //Cargamos archivo en memoria
        new_load_zx81_p_snapshot_in_mem(tapefile);
    }

    //Hacemos EX DE,HL
    z80_int reg;
    reg=HL;
    HL=DE;
    DE=reg;

    new_set_return_saveload_zx81();



}




//Funcion de grabacion de cinta entrando desde ROM para ZX80
void new_tape_save_zx80(void)
{
	//Grabamos snapshot
        debug_printf (VERBOSE_INFO,"Saving tape %s",tape_out_file);


	new_save_zx80_o_snapshot(tape_out_file);

	//Y volvemos a BASIC normalmente
	new_set_return_saveload_zx80();
}

//Funcion de grabacion de cinta entrando desde ROM para ZX81
void new_tape_save_zx81()
{
        //Grabamos snapshot
        debug_printf (VERBOSE_INFO,"Saving tape %s",tape_out_file);


        new_save_zx81_p_snapshot(tape_out_file);

        //Y volvemos a BASIC normalmente
        new_set_return_saveload_zx81();

}



//Funcion de carga de ZX80 O snapshot
void new_load_zx80_o_snapshot(char *archivo)
{

             debug_printf (VERBOSE_INFO,"Loading snapshot %s",archivo);

             //Cargamos archivo en memoria
             new_load_zx80_o_snapshot_in_mem(archivo);

             //Establecemos valores tipicos de registros, volvemos control al BASIC
  	     new_load_zx80_set_common_registers();

}

//Funcion de carga de ZX81 P snapshot
void new_load_zx81_p_snapshot(char *archivo)
{

        debug_printf (VERBOSE_INFO,"Loading snapshot %s",archivo);

        //ajusta variable ramtop del sistema si hay algun pack activo
        set_ramtop_with_rampacks();


	//Cargamos archivo en memoria
        new_load_zx81_p_snapshot_in_mem(archivo);

        //Establecemos valores tipicos de registros, volvemos control al BASIC
        new_load_zx81_set_common_registers(get_ramtop_with_rampacks());





}


//Funcion de grabacion de ZX80 O snapshot
void new_save_zx80_o_snapshot(char *filename)
{

       FILE *ptr_pfile;


        ptr_pfile=fopen(filename,"wb");
        if (!ptr_pfile) {
                debug_printf (VERBOSE_ERR,"Error writing snapshot file %s",filename);
                return;
        }

        z80_int puntero_inicio;

        z80_int bytes_grabar;
        z80_int e_line;
        


                e_line=value_8_to_16(peek_byte_no_time(16395),peek_byte_no_time(16394) );

		//En caso que e_line tenga valor invalido... por ejemplo que grabamos snapshot desde juego en ejecucion,
		//como con Breakout, que este valor se sale de rango

		if (e_line<16384 || e_line>32767) {
			debug_printf (VERBOSE_WARN,"Invalid value for end of basic program (e_line = %d ) . Setting maximum - 32767",e_line);
			e_line=32767;
		}

                puntero_inicio=0x4000;


        bytes_grabar=e_line-puntero_inicio+1;

        //Escritura de datos
        debug_printf (VERBOSE_INFO,"Saving %d bytes starting from %d address",bytes_grabar,puntero_inicio);
        fwrite(&memoria_spectrum[puntero_inicio],1,bytes_grabar,ptr_pfile);


        fclose(ptr_pfile);


}

//Funcion de grabacion de ZX81 P snapshot
void new_save_zx81_p_snapshot(char *filename)
{

       FILE *ptr_pfile;


        ptr_pfile=fopen(filename,"wb");
        if (!ptr_pfile) {
                debug_printf (VERBOSE_ERR,"Error writing snapshot file %s",filename);
                return;
        }

        z80_int puntero_inicio;

        z80_int bytes_grabar;
        z80_int e_line;
        


                puntero_inicio=0x4009;
                e_line=value_8_to_16(peek_byte_no_time(16405),peek_byte_no_time(16404) );

		//En caso que e_line tenga valor invalido... por ejemplo que grabamos snapshot desde juego en ejecucion, 
		//que puede que este valor se sale de rango

		//TODO: comprobar rampacks
                if (e_line<16384) {
                        debug_printf (VERBOSE_WARN,"Invalid value for end of basic program (e_line = %d ) . Setting maximum - 32767",e_line);
                        e_line=32767;
                }



        bytes_grabar=e_line-puntero_inicio+1;

        //Escritura de datos
        debug_printf (VERBOSE_INFO,"Saving %d bytes starting from %d address",bytes_grabar,puntero_inicio);
        fwrite(&memoria_spectrum[puntero_inicio],1,bytes_grabar,ptr_pfile);


        fclose(ptr_pfile);



}



/* Fin funciones principales */

int new_tap_save_detect_zx81(void)
{
        if (reg_pc!=0x02fc) return 0;
        if (tape_out_file==0) return 0;
        //if (tape_save_inserted.v==0) return 0;
        if ( (tape_loadsave_inserted & TAPE_SAVE_INSERTED)==0) return 0;
        return 1;
}

int new_tap_save_detect_zx80(void)
{
        if (reg_pc!=0x01b6) return 0;
        if (tape_out_file==0) return 0;
        //if (tape_save_inserted.v==0) return 0;
        if ( (tape_loadsave_inserted & TAPE_SAVE_INSERTED)==0) return 0;
        return 1;
}


int new_tap_load_detect_zx81(void)
{
    
    if (zx8081_disable_tape_traps.v) return 0;
    
        if (reg_pc!=0x0347) return 0;
        //if (reg_pc!=0x0340) return 0;

        //pruebas averiguar nombre de LOAD "

        z80_int registrode;
        registrode=value_8_to_16(reg_d,reg_e);
        debug_printf (VERBOSE_DEBUG,"reg_de points to file name to load: %d",registrode);


	//temp. si es load"", decimos que apunta a "4.P". Para testeo de juego UnkaTris
	/*
	if (registrode>=32768) {
		//decimos que apunta a "4.P"
                tapefile="4.P";

                debug_printf (VERBOSE_INFO,"Inserting tape: %s",tapefile);

                tape_init();
	}
	*/


        if (registrode<32768) {
                //LOAD "nombre"

                z80_bit inv;

                char buffer_nombre[256];

                int i=0;
        
                z80_byte letra;
                //letra=da_codigo81(peek_byte_no_time(registrohl++),&inv);
                //buffer_nombre[i++]=letra;
                //if (letra!='"') {
                //      debug_printf (VERBOSE_INFO,"Initial character to load is not \" ");
                //      return 0;
                //}

                for (;i<254;i++) {
                                letra=da_codigo81(peek_byte_no_time(registrode++),&inv);
                                buffer_nombre[i]=letra;
                                if (letra=='"') {
                                        i++;
                                        break;
                                }
                }

                buffer_nombre[i]=0;

                debug_printf (VERBOSE_INFO,"Name in sentence LOAD: %s, name length: %d",buffer_nombre,(int) strlen(buffer_nombre) );


                //si nombre no es "", insertar cinta
        
                //si no es un LOAD"", insertar cinta "nombre"
//      if (registrode<32768) {
//      if (buffer_nombre[1]!='"') {


                //obtenemos directorio de la ultima cinta
                //si no hay directorio, nada
                //asumimos nombre vacio
                //nombre_cinta_load_nombre_81[0]=0;
                if (tapefile!=NULL) {
                        debug_printf (VERBOSE_DEBUG,"Get directory of last tape used: %s",tapefile);
                        //printf ("max directory: %d\n",PATH_MAX);
                        util_get_dir(tapefile,nombre_cinta_load_nombre_81);
                        //printf ("strlen directorio: %d directorio: %s\n",strlen(tapefile),tapefile);

                        //usamos ese directorio, siempre que no sea nulo
                        if (nombre_cinta_load_nombre_81[0]!=0) {
                                //le añadimos / al final si es que no lo tiene ya
                                int l;
                                l=strlen(nombre_cinta_load_nombre_81);
                                if (nombre_cinta_load_nombre_81[l-1]!='/')
                                        strcpy(&nombre_cinta_load_nombre_81[l],"/");
                        }
                }
                else {
                        nombre_cinta_load_nombre_81[0]=0;
                }

                int l;

                debug_printf (VERBOSE_DEBUG,"Directory of last tape used: %s",nombre_cinta_load_nombre_81);

                //quitar la comilla final y añadir ".P"
                l=strlen(buffer_nombre);
                strcpy(&buffer_nombre[l-1],".P");

                //Y lo añadimos a path final
                l=strlen(nombre_cinta_load_nombre_81);
                strcpy(&nombre_cinta_load_nombre_81[l],&buffer_nombre[0]);

                debug_printf (VERBOSE_DEBUG,"Final tape name: %s",nombre_cinta_load_nombre_81);
        
                tapefile=nombre_cinta_load_nombre_81;

                debug_printf (VERBOSE_INFO,"Inserting tape: %s",tapefile);


		//quitamos autoload, para que en juegos multicarga no se resetee


		//Guardamos valor actual
		z80_bit temp_autoload;
		temp_autoload.v=noautoload.v;
		//Desactivamos
		noautoload.v=1;


                tape_init();

		//Restauramos valor anterior
		noautoload.v=temp_autoload.v;

        }

        //Es un LOAD "" o bien seguimos del LOAD "nombre"


        if (tapefile==0) return 0;
        //if (tape_load_inserted.v==0) return 0;
        if ( (tape_loadsave_inserted & TAPE_LOAD_INSERTED)==0) return 0;
        return 1;
}


int new_tap_load_detect_zx80(void)
{
    if (zx8081_disable_tape_traps.v) return 0;
    
        if (reg_pc!=0x0206) return 0;
        if (tapefile==0) return 0;
        //if (tape_load_inserted.v==0) return 0;
        if ( (tape_loadsave_inserted & TAPE_LOAD_INSERTED)==0) return 0;
        return 1;
}








//Simular un loop a nada en simulacion de carga de zx8081
void new_snap_load_zx8081_simulate_cpuloop(void)
{
                //apuntar a un nop
                if (MACHINE_IS_ZX81_TYPE) reg_pc=8191;
                else reg_pc=4095;

                //que indique en gui el indicador de carga
                tape_loading_counter=2;


                cpu_core_loop();

}


//simular bit de carga
void new_snap_load_zx8081_simulate_bit(z80_bit valor)
{

        int i,max_bucle;

	//Franjas de carga y sonido
        if (valor.v==0) lee_puerto_zx80(0,0xfe);
        else out_port_zx80(0xff,0);

        if (tape_loading_simulate_fast.v==0) max_bucle=150;
        else max_bucle=15;

        for (i=0;i<max_bucle;i++) {
                new_snap_load_zx8081_simulate_cpuloop();
        }
}


//simular byte de carga
void new_snap_load_zx8081_simulate_byte(z80_byte valor)
{
        int i=0;
        z80_bit bit_enviar;
        z80_byte veces_onda;

        for (i=0;i<8;i++) {
                if ( (valor&128) ) veces_onda=9;
                else veces_onda=4;

                for (;veces_onda>0;veces_onda--) {
                        bit_enviar.v=1;
                        new_snap_load_zx8081_simulate_bit(bit_enviar);
                        bit_enviar.v=0;
                        new_snap_load_zx8081_simulate_bit(bit_enviar);
                }

                //pausa de cada bit
                bit_enviar.v=0;
                veces_onda=7;
                for (;veces_onda>0;veces_onda--) new_snap_load_zx8081_simulate_bit(bit_enviar);

                valor=valor&127;
                valor=valor<<2;
        }
}






void new_snap_load_zx80_zx81_simulate_loading(z80_byte *puntero_inicio,z80_byte *buffer_lectura,int leidos)
{
	//para simular franjas de carga
	//tambien llamado en una carga normal

                                //guardamos valores anteriores
                                z80_bit antes_zx8081_vsync_sound,antes_rainbow_enabled,antes_interrupts,antes_nmi_generator_active;
                                //z80_bit antes_video_zx8081_shows_vsync_on_display;
                                antes_zx8081_vsync_sound.v=zx8081_vsync_sound.v;
                                antes_rainbow_enabled.v=rainbow_enabled.v;
                                antes_interrupts.v=iff1.v;
                                antes_nmi_generator_active.v=nmi_generator_active.v;
                                //antes_video_zx8081_shows_vsync_on_display.v=video_zx8081_shows_vsync_on_display.v;

				//Alterar algunos valores si esta la simulacion de carga activa
				if (tape_loading_simulate.v==1) {
	                                nmi_generator_active.v=0;
        	                        enable_rainbow();
                        	        zx8081_vsync_sound.v=1;
                                	amplitud_speaker_actual_zx8081=AMPLITUD_BEEPER_GRABACION_ZX8081;
	                                iff1.v=0;
        	                        //video_zx8081_shows_vsync_on_display.v=1;
				}

                                //para controlar si se pulsa tecla
                                snap_simulate_load_espera_no_tecla();
                                z80_byte tecla_pulsada;

                                int se_ha_pulsado_tecla=0;

                                int i;
                                z80_byte byte_leido;
                                for (i=0;i<leidos;i++) {
                                        byte_leido=buffer_lectura[i];

                                        puntero_inicio[i]=byte_leido;

                                        //simular franjas de carga

					if (tape_loading_simulate.v==1) {
                                                if ( (i%128)==0 && i!=0) debug_printf (VERBOSE_DEBUG,"Read %d bytes...",i);
                                                if (!se_ha_pulsado_tecla) new_snap_load_zx8081_simulate_byte(byte_leido);
                                        }

                                        tecla_pulsada=menu_da_todas_teclas()&31;

                                        //si se pulsa algo
                                        if (tecla_pulsada!=31) {
						//realmente al comparar esto no vemos que la tecla es diferente,

                                                        se_ha_pulsado_tecla=1;
                                        }


                                }

                                //restauramos valores anteriores
                                zx8081_vsync_sound.v=antes_zx8081_vsync_sound.v;
                                rainbow_enabled.v=antes_rainbow_enabled.v;
                                iff1.v=antes_interrupts.v;
                                nmi_generator_active.v=antes_nmi_generator_active.v;
                                //video_zx8081_shows_vsync_on_display.v=antes_video_zx8081_shows_vsync_on_display.v;

                                amplitud_speaker_actual_zx8081=AMPLITUD_BEEPER;

}




//Rutina para cargar archivo snap zx80 en memoria
void new_load_zx80_o_snapshot_in_mem(char *archivo)
{

        int leidos;
        z80_byte *puntero_inicio;
        int max_leer;

        z80_byte *buffer_lectura;

               //Load File
                  ptr_zx8081file=fopen(archivo,"rb");
                  if (ptr_zx8081file) {

                        
                                puntero_inicio=memoria_spectrum+0x4000;
                                max_leer=65536-0x4000;
                        
                        

                        buffer_lectura=malloc(65536);
                        if (buffer_lectura==NULL) cpu_panic("Error allocating read buffer");


                        leidos=fread(buffer_lectura,1,max_leer,ptr_zx8081file);

                        if (leidos<=0) {
                                debug_printf (VERBOSE_ERR,"Load error");
                        }

                        else {

                                //copiar bytes a destino
                                        new_snap_load_zx80_zx81_simulate_loading(puntero_inicio,buffer_lectura,leidos);

                                //memcpy(puntero_inicio,buffer_lectura,leidos);
      }


                        /*
                        //temp
                        z80_bit inverse;

                        int i;
                        for (i=0;i<leidos;i++) 
                        printf ("%c",da_codigo81(puntero_inicio[i],&inverse) );
                        */
                

                        fclose(ptr_zx8081file);

                        debug_printf (VERBOSE_INFO,"Loaded bytes: %d",leidos);


                        free(buffer_lectura);

                }

                else {
                        debug_printf (VERBOSE_ERR,"File %s not found",archivo);
                }

}




//Rutina para cargar archivo snap zx81 en memoria
void new_load_zx81_p_snapshot_in_mem(char *archivo) 
{

        int leidos;
        z80_byte *puntero_inicio;
        int max_leer;

        z80_byte *buffer_lectura;

               //Load File
                  ptr_zx8081file=fopen(archivo,"rb");
                  if (ptr_zx8081file) {

                        
                        
                                puntero_inicio=memoria_spectrum+0x4009;
                                max_leer=65536-0x4009;
                       

                        buffer_lectura=malloc(65536);
                        if (buffer_lectura==NULL) cpu_panic("Error allocating read buffer");


                        leidos=fread(buffer_lectura,1,max_leer,ptr_zx8081file);

                        if (leidos<=0) {
                                debug_printf (VERBOSE_ERR,"Load error");
                        }

                        else {



                                //copiar bytes a destino
                                        new_snap_load_zx80_zx81_simulate_loading(puntero_inicio,buffer_lectura,leidos);

                                //memcpy(puntero_inicio,buffer_lectura,leidos);
                      }


                        /*
                        //temp
                        z80_bit inverse;

                        int i;
                        for (i=0;i<leidos;i++) 
                        printf ("%c",da_codigo81(puntero_inicio[i],&inverse) );
                        */
                


                        fclose(ptr_zx8081file);

                        debug_printf (VERBOSE_INFO,"Loaded bytes: %d",leidos);


                        free(buffer_lectura);

                }

                else {
                        debug_printf (VERBOSE_ERR,"File %s not found",archivo);
                }

}





void new_set_return_saveload_zx80(void)
{
        reg_pc=0x0283;
}

void new_set_return_saveload_zx81(void)
{
	reg_pc=0x0207;

}



//Establecemos valores de registros de cpu para ZX80 para volver sin problemas al basic
void new_load_zx80_set_common_registers(void)
{

	z80_int ramtopvalue=ramtop_zx8081;

        //32767 para 16kb ram -> 7fff

        z80_int constante_ramtop_regsp=ramtopvalue;

        debug_printf (VERBOSE_DEBUG,"ramtop zx8081: %d (0x%x)",constante_ramtop_regsp+1,constante_ramtop_regsp+1);

        constante_ramtop_regsp -=3;
        debug_printf (VERBOSE_DEBUG,"Setting reg_sp to: %d (0x%x).",constante_ramtop_regsp,constante_ramtop_regsp);


        reg_a=0x0B;

        z80_byte flags=0x85;
	store_flags(flags);


        reg_b=0x00; reg_c=0xFF;
        reg_d=0x43; reg_e=0x99; reg_h=0xC3; reg_l=0x99;
        reg_a_shadow=0xE2;


        z80_byte flags_shadow=0xA1;
	store_flags_shadow(flags_shadow);

        reg_b_shadow=0x81; reg_c_shadow=0x02;
        reg_d_shadow=0x00; reg_e_shadow=0x2B; reg_h_shadow=0x00; reg_l_shadow=0x00;

        reg_i=0x0E;


        iff1.v=iff2.v=0;

        reg_r=0xDD;

        reg_ix=0x281; reg_iy=0x4000;

        //Mismo 7F que antes....
        //reg_sp=0x7FFC;
        reg_sp=constante_ramtop_regsp;



                //zx80
                /* XXX is this still valid given proper video? */
                /* don't ask me why, but the ZX80 ROM load routine does this if it
                 * works...
                 */
        
                reg_pc=0x0283;



}



//Establecemos valores de registros de cpu para ZX81 para volver sin problemas al basic
void new_load_zx81_set_common_registers(z80_int ramtopvalue)
{

    //32767 para 16kb ram -> 7fff

    z80_int constante_ramtop_regsp=ramtopvalue;

    debug_printf (VERBOSE_DEBUG,"ramtop zx8081: %d (0x%x)",constante_ramtop_regsp+1,constante_ramtop_regsp+1);

    constante_ramtop_regsp -=3;
    debug_printf (VERBOSE_DEBUG,"Setting reg_sp to: %d (0x%x).",constante_ramtop_regsp,constante_ramtop_regsp);


    z80_byte constante_ramtop_regsp_l=value_16_to_8l(constante_ramtop_regsp);
    z80_byte constante_ramtop_regsp_h=value_16_to_8h(constante_ramtop_regsp);


    z80_int constante_ramtop_masuno=ramtopvalue+1;
    debug_printf (VERBOSE_DEBUG,"Setting stack relative to RAMTOP: %d (0x%x).",constante_ramtop_masuno,constante_ramtop_masuno);

    z80_byte constante_ramtop_masuno_l=value_16_to_8l(constante_ramtop_masuno);
    z80_byte constante_ramtop_masuno_h=value_16_to_8h(constante_ramtop_masuno);
        




    //Este 7FFC parece indicar 32767 o cerca...
    //  static unsigned char bit1[9]={0xFF,0x80,0xFC,0x7F,0x00,0x80,0x00,0xFE,0xFF};
    unsigned char bit1[9]={0xFF,0x80,constante_ramtop_regsp_l,constante_ramtop_regsp_h,constante_ramtop_masuno_l,constante_ramtop_masuno_h,0x00,0xFE,0xFF};

    static unsigned char bit2[4]={0x76,0x06,0x00,0x3e};


    memcpy(memoria_spectrum+0x4000,bit1,9);

    //Aqui tambien hay referencias a 7FFC
    //memcpy(memoria_spectrum+0x7ffc,bit2,4);
    memcpy(memoria_spectrum+constante_ramtop_regsp,bit2,4);


    reg_a=0x0B;

    z80_byte flags=0x85;
	store_flags(flags);


    reg_b=0x00; reg_c=0xFF;
    reg_d=0x43; reg_e=0x99; reg_h=0xC3; reg_l=0x99;
    reg_a_shadow=0xE2;


    z80_byte flags_shadow=0xA1;
	store_flags_shadow(flags_shadow);

    reg_b_shadow=0x81; reg_c_shadow=0x02;
    reg_d_shadow=0x00; reg_e_shadow=0x2B; reg_h_shadow=0x00; reg_l_shadow=0x00;

    reg_i=0x1E;


    iff1.v=iff2.v=0;

    reg_r=0xDD;

    reg_ix=0x281; reg_iy=0x4000;

    //Mismo 7F que antes....
    //reg_sp=0x7FFC;
    reg_sp=constante_ramtop_regsp;




    //zx81
    reg_pc=0x207;

    //forzar siempre modo slow. Util para 3D monster maze, que parece que arranca en slow
    //S1    16443   403B    CDFLAG  Various flags. Bit 7 is on (1) during compute & display mode.
    //Bit 6 - the true fast/slow flag
    //Bit 7 - copy of the fast/slow flag. RESET when FAST needed

    //z80_byte cdflag=peek_byte_no_time(0x403B);
    //cdflag=cdflag | (64+128);
    //poke_byte_no_time(0x403B,cdflag);


    //suponemos que esta esto activo . TODO: detectar modo fast/slow por las variables de sistema
    nmi_generator_active.v=1;
    hsync_generator_active.v=1;



}



//Rutina para cargar cinta de tipo SMP para zx80
void new_snap_load_zx80_smp(char *archivo)
{       
        //Cargamos archivo en memoria
        snap_load_zx80_zx81_load_smp();

        //Y volvemos control al BASIC
        new_set_return_saveload_zx80();

        //para que no se queje el compilador de variable no usada
        archivo=0;
        archivo++;
}



//Rutina para cargar cinta de tipo SMP para zx81
void new_snap_load_zx81_smp(char *archivo)
{
        //Cargamos archivo en memoria
        snap_load_zx80_zx81_load_smp();

        //Y volvemos control al BASIC
        new_set_return_saveload_zx81();

        //para que no se queje el compilador de variable no usada
        archivo=0;
        archivo++;
}


