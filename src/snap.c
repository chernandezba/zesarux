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
#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cpu.h"
#include "debug.h"
#include "operaciones.h"
#include "zx8081.h"
#include "mem128.h"
#include "ay38912.h"
#include "compileoptions.h"
#include "tape_smp.h"
#include "audio.h"
#include "screen.h"
#include "menu.h"
#include "tape.h"
#include "snap.h"
#include "snap_z81.h"
#include "snap_zx8081.h"
#include "utils.h"
#include "ula.h"
#include "joystick.h"
#include "realjoystick.h"
#include "z88.h"
#include "chardetect.h"
#include "jupiterace.h"
#include "cpc.h"
#include "timex.h"
#include "zxuno.h"
#include "ulaplus.h"
#include "chloe.h"
#include "prism.h"
#include "diviface.h"
#include "snap_rzx.h"
#include "snap_zsf.h"
#include "snap_spg.h"
#include "settings.h"
#include "tbblue.h"
#include "esxdos_handler.h"


#include "autoselectoptions.h"

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif

char *snapfile;

//Version 6 al grabar predeterminado
z80_byte snap_zx_version_save=CURRENT_ZX_VERSION;

//Permitir cargar snapshots zx de versiones desconocidas
//Util para cargar snapshots de versiones superiores en emuladores antiguos
z80_bit snap_zx_permitir_versiones_desconocidas={0};


//Autoguardar un snapshot a intervalos fijos
z80_bit snapshot_contautosave_interval_enabled={0};


//Prefijos y directorios de autosave y de quicksave
char snapshot_autosave_interval_quicksave_name[PATH_MAX]="autosnap";
char snapshot_autosave_interval_quicksave_directory[PATH_MAX]="";

//Cada cuanto se guarda un snapshot
int snapshot_autosave_interval_seconds=1;

//Contador actual de autograbacion de snapshots a intervalo
int snapshot_autosave_interval_current_counter=0;

#define ZX_HEADER_SIZE 294
#define Z80_HEADER_SIZE_SAVE 87

z80_bit sna_setting_no_change_machine={0};

void set_snap_file_options(char *filename)
{
	set_snaptape_fileoptions(filename);
}





char *zxfile_machines_id[]={
"Sinclair 16k",              //0
"Sinclair 48k", 
"Inves Spectrum+", 
"Sinclair 128k", 
"Amstrad +2", 
"Amstrad +2 - French",       //5
"Amstrad +2 - Spanish", 
"Amstrad +2A (ROM v4.0)",     
"Amstrad +2A (ROM v4.1)", 
"Amstrad +2A - Spanish", 
"Spectrum 128k (Spanish)",     //10
"TK90X", 
"TK90XS", 
"TK95", 
"ZX80", 
"ZX81",       //15
"Z88", 
"Jupiter Ace", 
"Amstrad CPC 464", 
"Timex TS 2068", 
"ZX-Uno",    //20
"Chloe 140SE", 
"Chloe 280SE", 
"Prism", 
"Spectrum 48k (Spanish)"  //24

};

//Empezando desde 7 esta estandarizado, antes depende de si version 2 o 3
char *z80file_machines_id[]={
    "48k",
    "48k + If.1",
    "SamRam",
    "48k + M.G.T.",
    "128k",
    "128k + If.1", //5
    "128k + M.G.T.",
    "Spectrum +3",
    "Spectrum +3",
    "Pentagon (128K)",
    "Scorpion (256K)", //10
    "Didaktik-Kompakt",
    "Spectrum +2",
    "Spectrum +2A",
    "TC2048",
    "TC2068"   //15
};




void autosave_snapshot(void)
{

	char *nombre_autosave=AUTOSAVE_NAME;

	char buffer_nombre[PATH_MAX];

	//si hay ruta de guardado
	if (autosave_snapshot_path_buffer[0]!=0) {
		sprintf (buffer_nombre,"%s/%s",autosave_snapshot_path_buffer,nombre_autosave);
	}


	else {
		sprintf (buffer_nombre,"%s",nombre_autosave);
	}

	debug_printf (VERBOSE_INFO,"Autosaving snapshot %s",buffer_nombre);

	snapshot_save(buffer_nombre);

}

void autoload_snapshot(void)
{

	char *nombre_autosave=AUTOSAVE_NAME;

	char buffer_nombre[PATH_MAX];

        //si hay ruta de guardado
        if (autosave_snapshot_path_buffer[0]!=0) {
                sprintf (buffer_nombre,"%s/%s",autosave_snapshot_path_buffer,nombre_autosave);
        }


        else {
                sprintf (buffer_nombre,"%s",nombre_autosave);
        }

	//Ver si existe archivo. Sino, no cargar y no dar error
	if (si_existe_archivo(buffer_nombre)) {

	        debug_printf (VERBOSE_INFO,"Autoloading snapshot %s",buffer_nombre);

	        snapshot_load_name(buffer_nombre);

	}

	else {

                debug_printf (VERBOSE_INFO,"Not Autoloading snapshot because %s does not exist",buffer_nombre);

	}

}



void snap_simulate_load_espera_no_tecla(void)
{

        //Esperar a liberar teclas
        z80_byte acumulado;

        do {

		//fijar pc a algun sitio que no haga nada
		if (MACHINE_IS_ZX81) reg_pc=8191;
		else if (MACHINE_IS_ZX80) reg_pc=4095;
		else reg_pc=0;
                cpu_core_loop();

                acumulado=menu_da_todas_teclas();

        } while ( (acumulado & 31) != 31);

}




void snap_load_spectrum_simulate_cpuloop(void)
{
                //apuntar a un nop o algo sin peligro.
		reg_pc=0;



		//que indique en gui el indicador de carga
		tape_loading_counter=2;


                cpu_core_loop();


}

void snap_load_spectrum_simulate_cpuloop_tape_pause(void)
{
		z80_byte tecla_pulsada;

		if (tape_pause) debug_printf (VERBOSE_DEBUG,"Making Pause on simulate load tape_pause: %d",tape_pause);

                while (tape_pause) {
                        reg_pc=0;
                        cpu_core_loop();
		        tecla_pulsada=menu_da_todas_teclas()&31;
			//printf ("tecla: %d\n",tecla_pulsada);

                         //si se pulsa algo
                         if (tecla_pulsada!=31) tape_pause=0;

                }

}




//simular bit de carga
void snap_load_spectrum_simulate_bit(z80_bit valor)
{

        int i,max_bucle;

        if (valor.v==0) out_port(254,1);
        else out_port(254,16+6);

        if (tape_loading_simulate_fast.v==0) max_bucle=57;
        else max_bucle=57/4;

        for (i=0;i<max_bucle;i++) {
                snap_load_spectrum_simulate_cpuloop();
        }
}

#define TONO_GUIA_FREQ 89

//simular bit de carga tono guia
void snap_load_spectrum_simulate_bit_guia(z80_bit valor)
{

        int i,max_bucle;

        if (valor.v==0) out_port(254,2);
        else out_port(254,16+5);

        if (tape_loading_simulate_fast.v==0) max_bucle=TONO_GUIA_FREQ;
        else max_bucle=TONO_GUIA_FREQ/4;

        for (i=0;i<max_bucle;i++) {
                snap_load_spectrum_simulate_cpuloop();
        }
}





//simular byte de carga
void snap_load_spectrum_simulate_byte(z80_byte valor)
{
        int i=0;
	int j;
        z80_bit bit_enviar;
        z80_byte veces_onda;

	//printf ("valor byte: %d\n",valor);

        for (i=0;i<8;i++) {
		if ( (valor&128) ) veces_onda=8;
                else veces_onda=4;

                for (j=0;j<veces_onda;j++) {
                        bit_enviar.v=1;
                        snap_load_spectrum_simulate_bit(bit_enviar);
                }

                for (j=0;j<veces_onda;j++) {
                        bit_enviar.v=0;
                        snap_load_spectrum_simulate_bit(bit_enviar);
                }


                valor=valor&127;
                valor=valor<<2;
        }
}


void snap_load_spectrum_simulate_sync_false_aux(z80_bit valor)
{

        int i,max_bucle;

        if (valor.v==0) out_port(254,1);
        else out_port(254,16+6);

        if (tape_loading_simulate_fast.v==0) max_bucle=57;
        else max_bucle=57/4;

        for (i=0;i<max_bucle;i++) {
                snap_load_spectrum_simulate_cpuloop();
        }
}


//simular onda falsa de sincronismo
void snap_load_spectrum_simulate_sync_false(void)
{
        //int i=0;
        int j;
        z80_bit bit_enviar;
        z80_byte veces_onda;

        //printf ("valor byte: %d\n",valor);

                veces_onda=3;

                for (j=0;j<veces_onda;j++) {
                        bit_enviar.v=1;
                        snap_load_spectrum_simulate_sync_false_aux(bit_enviar);
                }

                for (j=0;j<veces_onda;j++) {
                        bit_enviar.v=0;
                        snap_load_spectrum_simulate_sync_false_aux(bit_enviar);
                }
}


void snap_load_spectrum_simulate_silence(void)
{
	int i;

	out_port(254,2);

	//aprox 1 segundo. nop son 4 ciclos. pantalla completa son unos 70000. Por 50 ciclos en un segundo
	for (i=0;i<70000/4*50;i++)
	snap_load_spectrum_simulate_cpuloop();

}

void load_spectrum_simulate_loading(z80_byte *buffer_lectura,z80_int destino,int leidos,z80_byte flag)
{
	if (tape_loading_simulate.v==0) return;

	//guardamos tape pause para restaurarlo luego
	int tape_pause_orig=tape_pause;

	//printf ("simular carga. buffer_lectura %p destino: %d leidos: %d flag: %d\n",buffer_lectura,destino,leidos,flag);

  //guardamos valores anteriores
                                z80_bit antes_rainbow_enabled,antes_interrupts,antes_diviface_enabled;

                                antes_rainbow_enabled.v=rainbow_enabled.v;
                                antes_interrupts.v=iff1.v;
				antes_diviface_enabled.v=diviface_enabled.v;

                                //rainbow_enabled.v=1;
				enable_rainbow();
                                iff1.v=0;
				diviface_enabled.v=0; //Para que no salte el trap de paginacion cuando cambiamos reg_pc en bucles de espera

				//printf ("antes menu da todas teclas\n");

				snap_simulate_load_espera_no_tecla();

                                //para controlar si se pulsa tecla
                                z80_byte tecla_pulsada;

                                int se_ha_pulsado_tecla=0;

				//printf ("despues menu da todas teclas\n");

				//primero franjas de flag
				//flag 0: un poco menos de 5 segundos
				//flag 255: aprox 2 segundos


				//400 es flag 0
				int contador_flag=400-flag;

				contador_flag *=10;
				int j;
				int tono_guia=6;
				z80_bit tono_enviar;

				for (;contador_flag>0 && !se_ha_pulsado_tecla;contador_flag--) {
					//printf ("tono guia. contador_flag:%d\n",contador_flag);
			                for (j=0;j<tono_guia;j++) {
			                        tono_enviar.v=1;
        	        		        snap_load_spectrum_simulate_bit_guia(tono_enviar);
			                }

			                for (j=0;j<tono_guia;j++) {
			                        tono_enviar.v=0;
			                        snap_load_spectrum_simulate_bit_guia(tono_enviar);
		        	        }

                                        tecla_pulsada=menu_da_todas_teclas()&31;


					//printf ("despues menu da todas teclas\n");

                                        //si se pulsa algo
                                        if (tecla_pulsada!=31) {
                                                        se_ha_pulsado_tecla=1;
                                        }


				}


				//el semipulso que van despues de tono guia
				snap_load_spectrum_simulate_sync_false();


				//y los bytes

                                int i;
                                z80_byte byte_leido;
                                for (i=0;i<leidos;i++) {
                                        byte_leido=buffer_lectura[i];
					//printf ("poke %d valor %d\n",destino,byte_leido);
					poke_byte_no_time(destino++,byte_leido);


                                        if (tape_loading_simulate.v==1) {
                                                if ( (i%1024)==0 && i!=0) debug_printf (VERBOSE_DEBUG,"Read %d bytes...",i);
                                                if (!se_ha_pulsado_tecla) snap_load_spectrum_simulate_byte(byte_leido);
                                        }

                                        tecla_pulsada=menu_da_todas_teclas()&31;

                                        //si se pulsa algo
                                        if (tecla_pulsada!=31) {

                                                        se_ha_pulsado_tecla=1;
                                        }


                                }

				//y silencio
				snap_load_spectrum_simulate_silence();

				//restauramos tape pause y hacemos pausa si conviene
				tape_pause=tape_pause_orig;
				snap_load_spectrum_simulate_cpuloop_tape_pause();

                                //restauramos valores anteriores

                                rainbow_enabled.v=antes_rainbow_enabled.v;
                                iff1.v=antes_interrupts.v;
				diviface_enabled.v=antes_diviface_enabled.v;


}


//Usado para funcion peek en grabacion snapshots z80 y zx
z80_byte save_z80zx_snapshot_bytes_128k_peek(z80_int dir,z80_byte ram_inicial)
{
int segmento;
z80_byte *puntero;
                segmento=dir / 16384;
                dir = dir & 16383;
                puntero=ram_mem_table[ram_inicial+segmento]+dir;

                return *puntero;
}


//Rutina comun para poke 48k y 128k, en carga de ZX
void load_zx_snapshot_bytes_128k_48k_poke(z80_int dir,z80_byte valor,z80_byte ram_inicial,int si_128k)
{
	if (si_128k) {
int segmento;
z80_byte *puntero;
                segmento=dir / 16384;
                dir = dir & 16383;
                puntero=ram_mem_table[ram_inicial+segmento]+dir;

                *puntero=valor;

	}

	else {
        //para que funcione tambien para zx80/81, pokear asi:

        memoria_spectrum[dir]=valor;

	}

}


//Rutina comun para cargar zx, tanto 48k como 128k
//TODO- gestionar en zx cuando se llega a final de memoria y hay algun DD activo
z80_byte *load_zx_snapshot_bytes_128k_48k(z80_byte *buffer_lectura,int leidos,z80_int direccion_destino,z80_byte ram_inicial,int si_128k)
{
                        int si_dd=0;
                        z80_byte byte_leido;

                        for (;leidos>0;leidos--) {
                                byte_leido=*buffer_lectura++;
                                //printf ("inicio bucle leidos: %d byte leido: %d direccion: %d\n",leidos,byte_leido,direccion_destino);
                                if (si_dd) {
                                        //si siguiente ed, hay repeticion
                                        if (byte_leido==0xDD) {
                                                //hay repeticion
                                                //leemos veces y caracter
                                                z80_byte byte_repetir=*buffer_lectura++;
                                                z80_byte byte_veces=*buffer_lectura++;
                                                int veces=byte_veces;
                                                if (byte_veces==0) veces=256;
                                                //printf ("bloque repeticion. dir: %d byte: %d veces: %d\n",direccion_destino,byte_repetir,veces);
                                                leidos -=2;

                                                if (leidos>0) for (;veces;veces--) load_zx_snapshot_bytes_128k_48k_poke(direccion_destino++,byte_repetir,ram_inicial,si_128k);
                                                //else debug_printf(VERBOSE_INFO,"It seems end of block. Don't do byte repetition");
                                        }

                                        //siguiente no era DD. pokeamos los dos
                                        else {
                                                load_zx_snapshot_bytes_128k_48k_poke(direccion_destino++,0xDD,ram_inicial,si_128k);
                                                load_zx_snapshot_bytes_128k_48k_poke(direccion_destino++,byte_leido,ram_inicial,si_128k);
                                        }
                                        si_dd=0;
                                }
                                else {
                                        if (byte_leido==0xDD) {
                                                //primer DD
                                                si_dd=1;
                                        }
                                        else {
                                                load_zx_snapshot_bytes_128k_48k_poke(direccion_destino++,byte_leido,ram_inicial,si_128k);
                                        }
                                }

                                if (direccion_destino==0 && !si_dd && si_128k) {
                                        //salir
                                        leidos=0;
                                }
                }
        return buffer_lectura;
}



//Rutina comun para poke Z88, en carga de ZX
void load_zx_snapshot_bytes_z88_poke(z80_int dir,z80_byte valor,z80_byte bank)
{

		z80_long_int offset=(bank*16384) + dir;
		z88_puntero_memoria[offset]=valor;


}

//Rutina comun para cargar zx, En Z88
//TODO- gestionar en zx cuando se llega a final de memoria y hay algun DD activo
z80_byte *load_zx_snapshot_bytes_z88(z80_byte *buffer_lectura,int leidos,z80_byte bank)
{
                        int si_dd=0;
                        z80_byte byte_leido;
			z80_int direccion_destino=0;


                        for (;leidos>0;leidos--) {
                                byte_leido=*buffer_lectura++;
                                //printf ("inicio bucle leidos: %d byte leido: %d direccion: %d\n",leidos,byte_leido,direccion_destino);
                                if (si_dd) {
                                        //si siguiente ed, hay repeticion
                                        if (byte_leido==0xDD) {
                                                //hay repeticion
                                                //leemos veces y caracter
                                                z80_byte byte_repetir=*buffer_lectura++;
                                                z80_byte byte_veces=*buffer_lectura++;
                                                int veces=byte_veces;
                                                if (byte_veces==0) veces=256;
                                                //printf ("bloque repeticion. dir: %d byte: %d veces: %d\n",direccion_destino,byte_repetir,veces);
                                                leidos -=2;

                                                if (leidos>0) for (;veces;veces--) load_zx_snapshot_bytes_z88_poke(direccion_destino++,byte_repetir,bank);
                                                //else debug_printf(VERBOSE_INFO,"It seems end of block. Don't do byte repetition");
                                        }

                                        //siguiente no era DD. pokeamos los dos
                                        else {
                                                load_zx_snapshot_bytes_z88_poke(direccion_destino++,0xDD,bank);
                                                load_zx_snapshot_bytes_z88_poke(direccion_destino++,byte_leido,bank);
                                        }
                                        si_dd=0;
                                }
                                else {
                                        if (byte_leido==0xDD) {
                                                //primer DD
                                                si_dd=1;
                                        }
                                        else {
                                                load_zx_snapshot_bytes_z88_poke(direccion_destino++,byte_leido,bank);
                                        }

         }

                                if (direccion_destino==16384 && !si_dd) {
                                        //salir
                                        leidos=0;
                                }
                }
        return buffer_lectura;
}


//Rutina comun para poke generic, en carga de ZX
void load_zx_snapshot_bytes_generic_poke(z80_byte *puntero,z80_byte valor)
{

                //z80_long_int offset=(bank*16384) + dir;
                //z88_puntero_memoria[offset]=valor;
		*puntero=valor;


}

//Rutina comun para cargar zx, Genérico bloques 16 kb
//TODO- gestionar en zx cuando se llega a final de memoria y hay algun DD activo
z80_byte *load_zx_snapshot_bytes_generic_16kb(z80_byte *buffer_lectura,int leidos,z80_byte *puntero)
{
                        int si_dd=0;
                        z80_byte byte_leido;
                        z80_int direccion_destino=0;


                        for (;leidos>0;leidos--) {
                                byte_leido=*buffer_lectura++;
                                //printf ("inicio bucle leidos: %d byte leido: %d direccion: %d\n",leidos,byte_leido,direccion_destino);
                                if (si_dd) {
                                        //si siguiente ed, hay repeticion
                                        if (byte_leido==0xDD) {
                                                //hay repeticion
                                                //leemos veces y caracter
                                                z80_byte byte_repetir=*buffer_lectura++;
                                                z80_byte byte_veces=*buffer_lectura++;
                                                int veces=byte_veces;
                                                if (byte_veces==0) veces=256;
                                                //printf ("bloque repeticion. dir: %d byte: %d veces: %d\n",direccion_destino,byte_repetir,veces);
                                                leidos -=2;

                                                if (leidos>0) for (;veces;veces--)
								load_zx_snapshot_bytes_generic_poke(&puntero[direccion_destino++],byte_repetir);

                                        }

                                        //siguiente no era DD. pokeamos los dos
                                        else {
                                                load_zx_snapshot_bytes_generic_poke(&puntero[direccion_destino++],0xDD);
                                                load_zx_snapshot_bytes_generic_poke(&puntero[direccion_destino++],byte_leido);
                                        }
                                        si_dd=0;
                                }
                                else {
					if (byte_leido==0xDD) {
                                                //primer DD
                                                si_dd=1;
                                        }
                                        else {
                                                load_zx_snapshot_bytes_generic_poke(&puntero[direccion_destino++],byte_leido);
                                        }

         }

                                if (direccion_destino==16384 && !si_dd) {
                                        //salir
                                        leidos=0;
                                }
                }
        return buffer_lectura;
}

z80_int load_zx_snapshot_bytes_generic_read_16bit_number(FILE *ptr_zxfile)
{
                        //leemos 16 bits
                        z80_byte bloque_l,bloque_h;
                        fread(&bloque_l,1,1,ptr_zxfile);
                        fread(&bloque_h,1,1,ptr_zxfile);
                        z80_int valor=value_8_to_16(bloque_h,bloque_l);


			return valor;
}



//Cargar registros comunes a formatos ZX y SP
void load_zxsp_snapshot_common_registers(z80_byte *header)
{
	reg_c=header[6];
	reg_b=header[7];
	reg_e=header[8];
	reg_d=header[9];
	reg_l=header[10];
	reg_h=header[11];

        store_flags(header[12]);
        reg_a=header[13];

        reg_ix=value_8_to_16(header[15],header[14]);
        reg_iy=value_8_to_16(header[17],header[16]);

        reg_c_shadow=header[18];
        reg_b_shadow=header[19];
        reg_e_shadow=header[20];
        reg_d_shadow=header[21];
        reg_l_shadow=header[22];
        reg_h_shadow=header[23];

        store_flags_shadow(header[24]);
        reg_a_shadow=header[25];

        reg_r=header[26];
        reg_r_bit7=reg_r&128;

	reg_i=header[27];

        reg_sp=value_8_to_16(header[29],header[28]);

        reg_pc=value_8_to_16(header[31],header[30]);

        out_254=header[34] & 7;
        modificado_border.v=1;

        im_mode=header[36] & 2;
	if (im_mode==1) im_mode=2;

        iff1.v=iff2.v=header[36] &1;


	//printf ("Interrupciones: %d\n",interrupts.v);





}


//Cargar ZX snapshot
void load_zx_snapshot(char *archivo)
{


        //Cabecera
        z80_byte zx_header[ZX_HEADER_SIZE];

        FILE *ptr_zxfile;
	z80_byte *buffer_lectura;

        int leidos;
	z80_byte zx_version;

        //leer datos
        buffer_lectura=malloc(128*1024);
        //buffer_lectura=malloc(4096*1024);
        if (buffer_lectura==NULL) cpu_panic("Cannot allocate memory when loading .zx file");


	//Load File
        ptr_zxfile=fopen(archivo,"rb");
	if (ptr_zxfile==NULL) {
		debug_printf(VERBOSE_ERR,"Error opening %s",archivo);
		return;
	}

        //if (ptr_zxfile) {
                leidos=fread(zx_header,1,ZX_HEADER_SIZE,ptr_zxfile);
                if (leidos!=ZX_HEADER_SIZE) {
                        debug_printf(VERBOSE_ERR,"Error reading %d bytes of header",ZX_HEADER_SIZE);
                        return;
                }
	//}

	//Ver si signatura correcta
	if (zx_header[0]!='Z' || zx_header[1]!='X') {
			debug_printf(VERBOSE_ERR,"Unknown ZX signature: 0x%x 0x%x",zx_header[0],zx_header[1]);
			return;
	}

	//Obtener version
	zx_version=zx_header[38];
	debug_printf(VERBOSE_INFO,"ZX Version %d file detected",zx_version);

	//Limite de numero de version zx
	if (zx_version>CURRENT_ZX_VERSION) {
		if (snap_zx_permitir_versiones_desconocidas.v) {
			debug_printf(VERBOSE_ERR,"Unknown ZX version: %d. Trying to load anyway",zx_version);
		}
		else {
			debug_printf(VERBOSE_ERR,"Unknown ZX version: %d",zx_version);
			return;
		}
        }

	if (zx_version>=3) {
                //tipo ordenador emulado
                z80_byte tipo_ordenador=zx_header[71];
                debug_printf(VERBOSE_INFO,"Machine type %d",tipo_ordenador);

	/* en cabecera ZX
	0=Sinclair 16k
	1=Sinclair 48k
	2=Inves Spectrum+
	3=Sinclair 128k
	4=Amstrad +2
	5=Amstrad +2 - Frances
	6=Amstrad +2 - Español
	7=Amstrad +2A (ROM v4.0)
	8=Amstrad +2A (ROM v4.1)
	9=Amstrad +2A - Español
10=Sinclair 128k Español
11=tk90x
12=tk90xs
13=tk95
14=zx80
15=zx81
16=z88 (a partir version 5)
17=jupiter ace (a partir version 6)
18=amstrad cpc (a partir version 6)
19=timex ts 2068 (a partir version 6)
	*/

		switch (tipo_ordenador) {
			//Sinclair 16k
			case 0:
				current_machine_type=0;
			break;

			//Sinclair 48k
			case 1:
				current_machine_type=1;
			break;

			//Inves Spectrum+
			case 2:
				current_machine_type=2;
			break;

			//Sinclair 128k
			case 3:
				current_machine_type=6;
			break;

			//Amstrad +2
			case 4:
				current_machine_type=8;
			break;

			//Amstrad +2 - Frances
			case 5:
				current_machine_type=9;
			break;

			//Amstrad +2 - Español
			case 6:
				current_machine_type=10;
			break;

			//Amstrad +2A (ROM v4.0)
			case 7:
				current_machine_type=11;
			break;

			//Amstrad +2A (ROM v4.1)
			case 8:
				current_machine_type=12;
			break;

			//Amstrad +2A - Español
			case 9:
				current_machine_type=13;
			break;

			//Sinclair 128k Español
			case 10:
				current_machine_type=7;
			break;

			//tk90x
			case 11:
				current_machine_type=3;
			break;

			//tk90xs
			case 12:
				current_machine_type=4;
			break;

			//tk95
			case 13:
				current_machine_type=5;
			break;

			//zx80
			case 14:
				current_machine_type=120;
			break;

			//zx81
			case 15:
				current_machine_type=121;
			break;

			//Z88
			case 16:
				current_machine_type=130;
			break;

			//Jupiter Ace
			case 17:
                                current_machine_type=122;
                        break;


			//CPC 464
			case 18:
				current_machine_type=140;
			break;

			//Timex TS 2068
			case 19:
				current_machine_type=17;
			break;

			//ZX-Uno
			case 20:
				current_machine_type=14;
			break;

			//Chloe 140SE
			case 21:
				current_machine_type=15;
			break;

			//Chloe 280SE
			case 22:
				current_machine_type=16;
			break;

			//Prism
			case 23:
				current_machine_type=18;
			break;

			//Spectrum 48k spanish
			case 24:
				current_machine_type=20;
			break;



			default:
				debug_printf(VERBOSE_ERR,"Unknown machine type %d",tipo_ordenador);
				return;
			break;
		}

	}

	z80_byte bits_estado0=zx_header[47];

	if (zx_version==2) {

		//ver valor de bits_estado0,bit 4
		if ( (bits_estado0 &16)!=0) {
			debug_printf(VERBOSE_INFO,"Version 2 and 128k. Assume Spectrum +2A Spanish");
			current_machine_type=13;
		}

		else {
			debug_printf(VERBOSE_INFO,"Version 2 and 48k. Assume Spectrum 48k");
                	current_machine_type=1;
		}

	}

	if (zx_version==1) {
		debug_printf(VERBOSE_INFO,"In version 1 all snapshots are 48k");
		 current_machine_type=1;
        }


        set_machine(NULL);
        reset_cpu();

        load_zxsp_snapshot_common_registers(zx_header);


	//Parametro que viene desde la version 1, pero no usado en version 3
	if (zx_version!=3) {
		joystick_autofire_frequency=zx_header[45];
		debug_printf(VERBOSE_DEBUG,"Setting autofire frequency to: 50/%d on zx snapshot",joystick_autofire_frequency);


		if ((bits_estado0 &64)==0) {
			joystick_autofire_frequency=0;
			debug_printf(VERBOSE_DEBUG,"Setting autofire to disabled on zx snapshot");
		}

		else {
			debug_printf(VERBOSE_DEBUG,"Setting autofire to enabled on zx snapshot");
		}

	}


	//Parametros posteriores a reset cpu y carga de registros
	if (zx_version>=3) {
	        z80_byte bits_estado2=zx_header[72];

        	nmi_generator_active.v=bits_estado2 &1;
	        hsync_generator_active.v=(bits_estado2>>1)&1;


		//Algun parametro por defecto que no esta en version anterior a la 4
		ram_in_8192.v=0;
		ram_in_32768.v=0;
		ram_in_49152.v=0;


		//Version 4
		if (zx_version>=4) {

			//Fecha. Solo para informacion. No se usa para nada mas
			char buffer_fecha[64];
			sprintf(buffer_fecha," Snapshot saved on: %d/%02d/%02d %02d:%02d ",value_8_to_16(zx_header[78],zx_header[77]),zx_header[76],zx_header[75],zx_header[79],zx_header[80]);
			debug_printf(VERBOSE_INFO,buffer_fecha);

			//TODO. pruebas. mostrar fecha del snapshot en second overlay
			//util_print_second_overlay(buffer_fecha,0,1);

			//Realvideo
			if ( (bits_estado2 &4) ) {
				debug_printf(VERBOSE_DEBUG,"Realvideo enabled on zx snapshot");
				enable_rainbow();
			}

			else {
				debug_printf(VERBOSE_DEBUG,"Realvideo disabled on zx snapshot");
				disable_rainbow();
			}

			//WRX
                        if ( (bits_estado2 &8) ) {
                                debug_printf(VERBOSE_DEBUG,"WRX enabled on zx snapshot");
                                enable_wrx();
                        }

                        else {
                                debug_printf(VERBOSE_DEBUG,"WRX disabled on zx snapshot");
                                disable_wrx();
                        }


			//RAM in 2000H
                        if ( (bits_estado2 &16) ) {
                                debug_printf(VERBOSE_DEBUG,"8KB RAM block on 2000H enabled on zx snapshot");
                                ram_in_8192.v=1;
                        }

                        else {
                                debug_printf(VERBOSE_DEBUG,"8KB RAM block on 2000H disabled on zx snapshot");
                                ram_in_8192.v=0;
                        }


                        //RAM in 8000H
                        if ( (bits_estado2 &32) ) {
                                debug_printf(VERBOSE_DEBUG,"16KB RAM block on 8000H enabled on zx snapshot");
                                ram_in_32768.v=1;
                        }

                        else {
                                debug_printf(VERBOSE_DEBUG,"16KB RAM block on 8000H disabled on zx snapshot");
                                ram_in_32768.v=0;
                        }


                        //RAM in C000H
                        if ( (bits_estado2 &64) ) {
                                debug_printf(VERBOSE_DEBUG,"16KB RAM block on C000H enabled on zx snapshot");
                                ram_in_49152.v=1;
                        }

                        else {
                                debug_printf(VERBOSE_DEBUG,"16KB RAM block on C000H disabled on zx snapshot");
                                ram_in_49152.v=0;
                        }

			//AY CHIP
                        if ( (bits_estado2 &128) ) {
                                debug_printf(VERBOSE_DEBUG,"AY Chip enabled on zx snapshot");
                                ay_chip_present.v=1;
                        }

                        else {
                                debug_printf(VERBOSE_DEBUG,"AY Chip disabled on zx snapshot");
                                ay_chip_present.v=0;
                        }

			z80_byte bits_estado3=zx_header[73];


                        //Horizontal stabilization
                        if ( (bits_estado3 &1) ) {
				//Al activar realvideo se activa siempre esto tambien. Por eso, por si acaso, lo hacemos despues
                                debug_printf(VERBOSE_DEBUG,"Horizontal Stabilization enabled on zx snapshot");
				video_zx8081_estabilizador_imagen.v=1;
                        }

                        else {
                                debug_printf(VERBOSE_DEBUG,"Horizontal Stabilization disabled on zx snapshot");
				video_zx8081_estabilizador_imagen.v=0;
                        }


			//LNCTR video adjust
                        //if ( (bits_estado3 &2) ) {
                        //        debug_printf(VERBOSE_DEBUG,"LNCTR video adjust enabled on zx snapshot");
                        //        video_zx8081_lnctr_adjust.v=1;
                        //}

                        //else {
                        //        debug_printf(VERBOSE_DEBUG,"LNCTR video adjust disabled on zx snapshot");
                        //        video_zx8081_lnctr_adjust.v=0;
                        //}



                        //VSYNC sound
                        if ( (bits_estado3 &4) ) {
                                debug_printf(VERBOSE_DEBUG,"VSYNC sound enabled on zx snapshot");
                                zx8081_vsync_sound.v=1;
                        }

                        else {
                                debug_printf(VERBOSE_DEBUG,"VSYNC sound disabled on zx snapshot");
                                zx8081_vsync_sound.v=0;
                        }

			//TRAP RST16
			if ( (bits_estado3 &8) ) {
                                debug_printf(VERBOSE_DEBUG,"Stdout Trap RST 16 enabled on zx snapshot");
                                chardetect_printchar_enabled.v=1;
                        }

			else {
                                debug_printf(VERBOSE_DEBUG,"Stdout Trap RST 16 disabled on zx snapshot");
                                chardetect_printchar_enabled.v=0;
                        }

                        //stdout automatic redraw
                        if ( (bits_estado3 &16) ) {
                                debug_printf(VERBOSE_DEBUG,"Stdout Automatic redraw enabled on zx snapshot");
				stdout_simpletext_automatic_redraw.v=1;
                        }

                        else {
                                debug_printf(VERBOSE_DEBUG,"Stdout Automatic redraw disabled on zx snapshot");
				stdout_simpletext_automatic_redraw.v=0;
                        }

                        //stdout second trap sum32
                        if ( (bits_estado3 &32) ) {
                                debug_printf(VERBOSE_DEBUG,"Stdout second trap sum 32 enabled on zx snapshot");
				chardetect_second_trap_sum32.v=1;
                        }

                        else {
                                debug_printf(VERBOSE_DEBUG,"Stdout second trap sum 32 disabled on zx snapshot");
				chardetect_second_trap_sum32.v=0;
                        }



                        //kempston mouse emulation
                        if ( (bits_estado3 &64) ) {
                                debug_printf(VERBOSE_DEBUG,"Kempston mouse emulation enabled on zx snapshot");
                                kempston_mouse_emulation.v=1;
                        }

                        else {
                                debug_printf(VERBOSE_DEBUG,"Kempston mouse emulation disabled on zx snapshot");
                                kempston_mouse_emulation.v=0;
                        }


			//Z88 Keymap
			if ( (bits_estado3 &128) ) {
				debug_printf(VERBOSE_DEBUG,"Setting Z88 Keymap to Spanish");
				z88_cpc_keymap_type=1;
			}

			else {
                                debug_printf(VERBOSE_DEBUG,"Setting Z88 Keymap to Default");
                                z88_cpc_keymap_type=0;
                        }
			scr_z88_cpc_load_keymap();




			//TODO. No usado
			//char t=zx_header[74];
			//offset_zx8081_t_estados=t;
			//debug_printf(VERBOSE_DEBUG,"Setting t_states offset to: %d",offset_zx8081_t_estados);


			z80_int zx8081ram=zx_header[81];

			//Alertar de valores invalidos
			if (zx8081ram<1 || zx8081ram>16) {
				debug_printf(VERBOSE_ERR,"Can not set ZX80/81 standard ram to %d kb",zx8081ram);
			}

			else {
				debug_printf(VERBOSE_DEBUG,"Setting ZX80/81 standard ram to %d kb",zx8081ram);
				set_zx8081_ramtop(zx8081ram);
			}


			last_inves_low_ram_poke_menu=zx_header[82];
			if (MACHINE_IS_INVES) {
				debug_printf(VERBOSE_DEBUG,"Poking all low Inves RAM with value: %d",last_inves_low_ram_poke_menu);
				poke_inves_rom(last_inves_low_ram_poke_menu);
			}


			/*z80_byte invula=zx_header[83];
			//Ignorar valores fuera de rango (normalmente entre 1...4)
			if (invula>=1 && invula<=4) {
				inves_ula_delay_factor=invula;
				debug_printf(VERBOSE_DEBUG,"Setting Inves Ula Delay to %d",inves_ula_delay_factor);
			}
			*/

			//Second trap char dir
			chardetect_second_trap_char_dir=value_8_to_16(zx_header[85],zx_header[84]);
			debug_printf(VERBOSE_DEBUG,"Setting Stdout Second Trap char address to: %d",chardetect_second_trap_char_dir);

			//Third trap char dir
			chardetect_third_trap_char_dir=value_8_to_16(zx_header[87],zx_header[86]);
			debug_printf(VERBOSE_DEBUG,"Setting Stdout Third Trap char address to: %d",chardetect_third_trap_char_dir);

			//Stdout line width
			chardetect_line_width=zx_header[88];
			debug_printf(VERBOSE_DEBUG,"Setting Stdout Line width to: %d",chardetect_line_width);

			//Stdout char filter
			z80_byte f=zx_header[89];
			if (f>=CHAR_FILTER_TOTAL) {
				debug_printf (VERBOSE_ERR,"Stdout char filter out of range: %d",f);
			}

			else {
				chardetect_char_filter=f;
				debug_printf(VERBOSE_DEBUG,"Setting Stdout Char filter to: %s",chardetect_char_filter_names[chardetect_char_filter]);
			}

			//Joystick type
                        z80_byte j=zx_header[90];
                        if (j>JOYSTICK_TOTAL) {
                                debug_printf (VERBOSE_ERR,"Joystick type out of range: %d",j);
                        }

                        else {
                                joystick_emulation=j;
                                debug_printf(VERBOSE_DEBUG,"Setting Joystick type to: %s",joystick_texto[joystick_emulation]);
                        }


			//Gunstick type
			z80_byte g=zx_header[91];
			if (g>GUNSTICK_TOTAL) {
				debug_printf (VERBOSE_ERR,"Lightgun type out of range: %d",g);
			}

			else {
				gunstick_emulation=g;
				debug_printf(VERBOSE_DEBUG,"Setting Lightgun type to: %s",gunstick_texto[gunstick_emulation]);
			}


		}

		if (zx_version>=5) {
			z88_internal_rom_size=(zx_header[92]*16384)-1;
			debug_printf(VERBOSE_DEBUG,"Setting Z88 Internal ROM Size to %d",z88_internal_rom_size+1);
			z88_internal_ram_size=(zx_header[93]*16384)-1;
			debug_printf(VERBOSE_DEBUG,"Setting Z88 Internal RAM Size to %d",z88_internal_ram_size+1);




			int slot_leido=1;
			z88_memory_slots[slot_leido].type=zx_header[94] & 3;
			//si hay alguna tarjeta de memoria de tipo 1 es hibrida ram+eprom
			if (z88_memory_slots[slot_leido].type==1) z88_memory_slots[slot_leido].type=4;

			debug_printf(VERBOSE_DEBUG,"Setting Z88 Slot 1 Memory type to: %s",z88_memory_types[z88_memory_slots[1].type]);
			slot_leido++;

			z88_memory_slots[slot_leido].type=(zx_header[94]>>2) & 3;
			//si hay alguna tarjeta de memoria de tipo 1 es hibrida ram+eprom
			if (z88_memory_slots[slot_leido].type==1) z88_memory_slots[slot_leido].type=4;

			debug_printf(VERBOSE_DEBUG,"Setting Z88 Slot 2 Memory type to: %s",z88_memory_types[z88_memory_slots[2].type]);
			slot_leido++;

			z88_memory_slots[slot_leido].type=(zx_header[94]>>4) & 3;
			//si hay alguna tarjeta de memoria de tipo 1 es hibrida ram+eprom
			if (z88_memory_slots[slot_leido].type==1) z88_memory_slots[slot_leido].type=4;

			debug_printf(VERBOSE_DEBUG,"Setting Z88 Slot 3 Memory type to: %s",z88_memory_types[z88_memory_slots[3].type]);


			if (zx_header[95]) z88_memory_slots[1].size=(zx_header[95]*16384)-1;
			else z88_memory_slots[1].size=0;

			if (zx_header[96]) z88_memory_slots[2].size=(zx_header[96]*16384)-1;
			else z88_memory_slots[2].size=0;

			if (zx_header[97]) z88_memory_slots[3].size=(zx_header[97]*16384)-1;
			else z88_memory_slots[3].size=0;

			//Si hay EPROM o Flash en slot 3, cambiarlo a RAM y size 0

			int avisarerror=0;
			int i=3;
				if (z88_memory_slots[i].type==2 || z88_memory_slots[i].type==3 || z88_memory_slots[i].type==4) {
					if (z88_memory_slots[i].size!=0) {

						z88_memory_slots[i].size=0;
						avisarerror=1;
					}

					z88_memory_slots[i].type=0;
				}

			if (avisarerror) {
				debug_printf (VERBOSE_ERR,"Snapshot had an EPROM or Flash card on Slot 3. It is NOT loaded. You must insert it manually");
			}


			//Mostrar en debug tamanyo slots
			z80_long_int size;
			int sl;
			for (sl=1;sl<=3;sl++) {
				size=z88_memory_slots[sl].size;
				debug_printf(VERBOSE_DEBUG,"Setting Z88 Slot %d Size to: %d",sl,(size ? size +1 : 0));

			}


			//Leer registros del blink


			blink_pixel_base[0]=value_8_to_16(zx_header[99],zx_header[98]);
			blink_pixel_base[1]=value_8_to_16(zx_header[101],zx_header[100]);
			blink_pixel_base[2]=value_8_to_16(zx_header[103],zx_header[102]);
			blink_pixel_base[3]=value_8_to_16(zx_header[105],zx_header[104]);

			blink_sbr=value_8_to_16(zx_header[107],zx_header[106]);


	                blink_com=zx_header[108];
        	        blink_int=zx_header[109];

                	blink_sta=zx_header[110];
	                blink_epr=zx_header[111];

        	        blink_tmk=zx_header[112];
                	blink_tsta=zx_header[113];

	                blink_mapped_memory_banks[0]=zx_header[114];
        	        blink_mapped_memory_banks[1]=zx_header[115];
                	blink_mapped_memory_banks[2]=zx_header[116];
	                blink_mapped_memory_banks[3]=zx_header[117];

	                blink_tim[0]=zx_header[118];
        	        blink_tim[1]=zx_header[119];
                	blink_tim[2]=zx_header[120];
	                blink_tim[3]=zx_header[121];
        	        blink_tim[4]=zx_header[122];

                	blink_rxd=zx_header[123];
	                blink_rxe=zx_header[124];

        	        blink_rxc=zx_header[125];
                	blink_txd=zx_header[126];

	                blink_txc=zx_header[127];
        	        blink_umk=zx_header[128];

                	blink_uit=zx_header[129];

		}

                //Version 6
                if (zx_version>=6) {
			//Ramtop de jupiter Ace
                        //Dado que el soporte de jupiter ace esta a partir de version 5 pero a partir de ZEsarUX 3.2,
                        //nos podiamos encontrar por ejemplo con un snapshot de ZEsarUX 3.1, que es version 5, pero
                        //que no usa este campo, y entonces meteriamos un valor de ace ram invalido (255 al estar no usado)
                        //Por tanto, solo establecemos la ram de jupiter ace cuando la maquina a cargar es Jupiter Ace
                        if (MACHINE_IS_ACE) {
                                z80_int aceram=zx_header[130];
                                debug_printf(VERBOSE_DEBUG,"Setting Jupiter Ace ram to %d kb",aceram);
                                set_ace_ramtop(aceram);
                        }


			timex_port_f4=zx_header[131];
			timex_port_ff=zx_header[132];


			z80_byte bits_estado4=zx_header[133];

			if (bits_estado4 & 1) {
				debug_printf (VERBOSE_DEBUG,"Enabling ULAplus");
				ulaplus_presente.v=1;
			}
		        else {
				debug_printf (VERBOSE_DEBUG,"Disabling ULAplus");
				ulaplus_presente.v=0;
			}

			if (bits_estado4 & 2) ulaplus_enabled.v=1;
			else ulaplus_enabled.v=0;


			if (bits_estado4 & 4) {
				debug_printf (VERBOSE_DEBUG,"Enabling Timex Video Support");
				timex_video_emulation.v=1;
			}

			else {
				debug_printf (VERBOSE_DEBUG,"Disabling Timex Video Support");
				timex_video_emulation.v=0;
			}


			ulaplus_last_send_BF3B=zx_header[134];
			ulaplus_last_send_FF3B=zx_header[135];

			ulaplus_mode=zx_header[136];
			//Evitar valores 255 de versiones de testing
			if (ulaplus_mode==255) {
				ulaplus_mode=0;
				ulaplus_enabled.v=0;
			}

			debug_printf (VERBOSE_DEBUG,"Setting ULAplus mode %d",ulaplus_mode);

			//Leer 64 bytes de paleta ulaplus
			int i;
			for (i=0;i<64;i++) ulaplus_palette_table[i]=zx_header[137+i];


		}
	}


	//cargar datos

        //carga de 48k
        if (MACHINE_IS_SPECTRUM_16_48) {
                leidos=fread(buffer_lectura,1,49152,ptr_zxfile);
                debug_printf(VERBOSE_INFO,"Reading bytes of %d compressed data bytes at 16384 address",leidos);
                load_zx_snapshot_bytes_128k_48k(buffer_lectura,leidos,16384,0,0);
        }

	else if (MACHINE_IS_ZX8081) {
		int direccion=16384;
		if (ram_in_8192.v) direccion=8192;

                leidos=fread(buffer_lectura,1,49152+8192,ptr_zxfile);
                debug_printf(VERBOSE_INFO,"Reading bytes of %d compressed data bytes at %d address",leidos,direccion);
                load_zx_snapshot_bytes_128k_48k(buffer_lectura,leidos,direccion,0,0);
        }

        else if (MACHINE_IS_ACE) {
                int direccion=8192;

                leidos=fread(buffer_lectura,1,49152+8192,ptr_zxfile);
                debug_printf(VERBOSE_INFO,"Reading bytes of %d compressed data bytes at %d address",leidos,direccion);
                load_zx_snapshot_bytes_128k_48k(buffer_lectura,leidos,direccion,0,0);
        }

	else if (MACHINE_IS_Z88) {

		while (!feof(ptr_zxfile)) {
			//leemos numero banco
			z80_byte bank;
			fread(&bank,1,1,ptr_zxfile);
			if (!feof(ptr_zxfile)) {
				//leemos longitud bloque
				z80_byte longitud_l,longitud_h;
				fread(&longitud_l,1,1,ptr_zxfile);
				fread(&longitud_h,1,1,ptr_zxfile);
				z80_int longitud=value_8_to_16(longitud_h,longitud_l);
				debug_printf(VERBOSE_INFO,"Reading %d bytes of compressed data bytes at Bank %02XH",longitud,bank);
				fread(buffer_lectura,1,longitud,ptr_zxfile);
				load_zx_snapshot_bytes_z88(buffer_lectura,longitud,bank);
			}
		}


		//Para actualizar footer de bancos del Z88
		menu_init_footer();

	}

	else if (MACHINE_IS_CPC_464) {

		//Leer datos puertos, etc cpc
		fread(cpc_gate_registers,1,4,ptr_zxfile);
                fread(cpc_palette_table, 1, 16, ptr_zxfile);
                fread(cpc_ppi_ports, 1, 4, ptr_zxfile);
                fread(cpc_crtc_registers, 1, 32, ptr_zxfile);
                fread(&cpc_border_color,1,1,ptr_zxfile);
                fread(&cpc_crtc_last_selected_register,1,1,ptr_zxfile);

		//Paginamos segun registros del gate
		cpc_set_memory_pages();


                while (!feof(ptr_zxfile)) {
                        //leemos numero bloque
                        z80_int bloque=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);

                        if (!feof(ptr_zxfile)) {
                                //leemos longitud bloque
				z80_int longitud=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);
                                debug_printf(VERBOSE_INFO,"Reading %d bytes of compressed data bytes at Block %d",longitud,bloque);
                                fread(buffer_lectura,1,longitud,ptr_zxfile);
                                load_zx_snapshot_bytes_generic_16kb(buffer_lectura,longitud,cpc_ram_mem_table[bloque]);
                        }
                }


        }

	else if (MACHINE_IS_TIMEX_TS2068) {
		//Paginamos
		timex_set_memory_pages();

		while (!feof(ptr_zxfile)) {
                        //leemos numero bloque
                        z80_int bloque=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);

                        if (!feof(ptr_zxfile)) {
                                //leemos longitud bloque
				z80_int longitud=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);
                                debug_printf(VERBOSE_INFO,"Reading %d bytes of compressed data bytes at Block %d",longitud,bloque);
                                fread(buffer_lectura,1,longitud,ptr_zxfile);
                                load_zx_snapshot_bytes_generic_16kb(buffer_lectura,longitud,timex_home_ram_mem_table[bloque]);
                        }
                }
	}

	else if (MACHINE_IS_CHLOE_140SE) {
		//Paginamos, leyendo antes valores puerto 32765 y 8189
		puerto_8189=zx_header[49];
		puerto_32765=zx_header[48];
		chloe_set_memory_pages();

		while (!feof(ptr_zxfile)) {
                        //leemos numero bloque
                        z80_int bloque=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);

                        if (!feof(ptr_zxfile)) {
                                //leemos longitud bloque
                                z80_int longitud=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);
                                debug_printf(VERBOSE_INFO,"Reading %d bytes of compressed data bytes at Block %d",longitud,bloque);
                                fread(buffer_lectura,1,longitud,ptr_zxfile);
                                load_zx_snapshot_bytes_generic_16kb(buffer_lectura,longitud,chloe_home_ram_mem_table[bloque]);
                        }
                }
        }

        else if (MACHINE_IS_CHLOE_280SE) {
		//Paginamos, leyendo antes valores puerto 32765 y 8189
		puerto_8189=zx_header[49];
		puerto_32765=zx_header[48];
                chloe_set_memory_pages();

                while (!feof(ptr_zxfile)) {
                        //leemos numero bloque
                        z80_int bloque=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);

                        if (!feof(ptr_zxfile)) {
                                //leemos longitud bloque
                                z80_int longitud=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);
                                debug_printf(VERBOSE_INFO,"Reading %d bytes of compressed data bytes at Block %d",longitud,bloque);
                                fread(buffer_lectura,1,longitud,ptr_zxfile);

				//Numero bloque. Si 0-7, Ex Ram. Si 8-15, Dock RAM. Si 16-23, Home RAM. Tener en cuenta que bloques de dock y home
				//se graban 16 kb pero internamente el array es de 8 kb. Por tanto se cargan bloques 0,2,4,6. Bloque 0 por ejemplo
				//se carga 16 kb pero al ser memoria consecutiva, la pagina 0 tendra los primeros 8 kb y la pagina 1 los siguientes 8 kb

				if (bloque>=0 && bloque<=7) load_zx_snapshot_bytes_generic_16kb(buffer_lectura,longitud,chloe_ex_ram_mem_table[bloque]);
				else if (bloque>=8 && bloque<=15) load_zx_snapshot_bytes_generic_16kb(buffer_lectura,longitud,chloe_dock_ram_mem_table[bloque-8]);
				else if (bloque>=16) load_zx_snapshot_bytes_generic_16kb(buffer_lectura,longitud,chloe_home_ram_mem_table[bloque-16]);
                        }
                }
        }

	else if (MACHINE_IS_PRISM) {
		//Leer bytes estado
		fread(&prism_rom_page,1,1,ptr_zxfile);
		fread(&prism_ae3b_registers[0],1,1,ptr_zxfile);
		fread(&prism_ula2_palette_control_colour,1,1,ptr_zxfile);
		fread(&prism_ula2_palette_control_index,1,1,ptr_zxfile);
		fread(prism_ula2_palette_control_rgb,1,3,ptr_zxfile);
		fread(prism_ula2_registers,1,16,ptr_zxfile);

		//Leer paleta 2. 256 valores de 16 bits. Leer en bucle para asegurarse que es little endian
                int i;
                z80_int color;
                z80_byte hi,lo;
                for (i=0;i<256;i++) {
			fread(&lo,1,1,ptr_zxfile);
			fread(&hi,1,1,ptr_zxfile);
			color=value_8_to_16(hi,lo);
			prism_palette_two[i]=color;
                }

		//Paginamos, leyendo antes valores puerto 32765 y 8189
                puerto_8189=zx_header[49];
                puerto_32765=zx_header[48];
		prism_set_memory_pages();


                while (!feof(ptr_zxfile)) {
                        //leemos numero bloque
                        z80_int bloque=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);

                        if (!feof(ptr_zxfile)) {
                                //leemos longitud bloque
                                z80_int longitud=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);
                                debug_printf(VERBOSE_INFO,"Reading %d bytes of compressed data bytes at Block %d",longitud,bloque);
                                fread(buffer_lectura,1,longitud,ptr_zxfile);

                                //Numero bloque. Si 240-, vram. Resto RAM
                                //se graban 16 kb pero internamente el array es de 8 kb. Por tanto se cargan bloques 0,2,4,6. Bloque 0 por ejemplo
                                //se carga 16 kb pero al ser memoria consecutiva, la pagina 0 tendra los primeros 8 kb y la pagina 1 los siguientes 8 kb

                                if (bloque>=240) load_zx_snapshot_bytes_generic_16kb(buffer_lectura,longitud,prism_vram_mem_table[bloque-240]);
                                else load_zx_snapshot_bytes_generic_16kb(buffer_lectura,longitud,prism_ram_mem_table[bloque]);
                        }
                }

	}



	else if (MACHINE_IS_ZXUNO) {
		//Leer bytes estado
                fread(&last_port_FC3B,1,1,ptr_zxfile);
                fread(zxuno_ports,1,256,ptr_zxfile);
                fread(zxuno_spi_bus,1,8,ptr_zxfile);
                fread(&zxuno_spi_bus_index,1,1,ptr_zxfile);
                fread(&next_spi_read_byte,1,1,ptr_zxfile);
                fread(&zxuno_spi_status_register,1,1,ptr_zxfile);

		//Paginamos, leyendo antes valores puerto 32765 y 8189
		puerto_8189=zx_header[49];
		puerto_32765=zx_header[48];

		//Paginar RAM y ROM
                //zxuno_mem_page_ram_p2a();
                //zxuno_mem_page_rom_p2a();

                zxuno_set_memory_pages();

                //Leer 2 valores de 24 bits
                z80_byte buffer_spi_address[6];
		fread(buffer_spi_address,1,6,ptr_zxfile);

                //last_spi_write_address
		last_spi_write_address=(buffer_spi_address[0]) + (256 * buffer_spi_address[1]) + (65536 * buffer_spi_address[2]);
		last_spi_read_address=(buffer_spi_address[3]) + (256 * buffer_spi_address[4]) + (65536 * buffer_spi_address[5]);


		while (!feof(ptr_zxfile)) {
                        //leemos numero bloque
                        z80_int bloque=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);

                        if (!feof(ptr_zxfile)) {
                                //leemos longitud bloque
				z80_int longitud=load_zx_snapshot_bytes_generic_read_16bit_number(ptr_zxfile);
                                debug_printf(VERBOSE_INFO,"Reading %d bytes of compressed data bytes at Block %d",longitud,bloque);
                                fread(buffer_lectura,1,longitud,ptr_zxfile);
                                load_zx_snapshot_bytes_generic_16kb(buffer_lectura,longitud,zxuno_sram_mem_table_new[bloque]);
                        }
                }
	}




        else {
		//128kb
                leidos=fread(buffer_lectura,1,131072,ptr_zxfile);
                debug_printf(VERBOSE_INFO,"Reading bytes of compressed data bytes at RAMS 0-3");
		//printf ("buffer lectura: %p\n",buffer_lectura);
                z80_byte *finbloque1=load_zx_snapshot_bytes_128k_48k(buffer_lectura,leidos,0,0,1);


                debug_printf(VERBOSE_INFO,"Reading bytes at offset %d of compressed data bytes at RAMS 4-7",finbloque1-buffer_lectura);
		//printf ("buffer lectura: %p\n",finbloque1);
                load_zx_snapshot_bytes_128k_48k(finbloque1,leidos,0,4,1);

        }



	if (zx_version>=2) {
	                //registros del chip AY
                        int reg_ay;
                        for (reg_ay=0;reg_ay<16;reg_ay++) {
				//En zx80, zx81 tambien se entra aqui... Aunque esos puertos no son iguales para el chip AY,
				//la funcion llama directamente a out_port_spectrum y ahi no distingue entre spectrum y zx80/81

				//No enviar asi. Sino, hace autoactivado del chip ay
                                //out_port_spectrum(0xfffd,reg_ay);
                                //out_port_spectrum(0xbffd,zx_header[55+reg_ay]);

				//Meter valores en registros AY tal cual
				ay_3_8912_registros[0][reg_ay]=zx_header[55+reg_ay];
                        }



                        //y ultimo valor enviado
                        //out_port_spectrum(0xfffd,zx_header[54]);
			ay_3_8912_registro_sel[0]=zx_header[54];


			//si modo 128k, dejar bien la paginacion
			//Primero enviar puerto 8189. Si enviasemos antes puerto 32765, puede pasar
			//que la paginacion este deshabilitada (bit 5 de 32765 a 1) y entonces el envio de 8189 no tendria ningun efecto

                        //+2A
			if  (MACHINE_IS_SPECTRUM_P2A_P3) {
				z80_byte valor_8189=zx_header[49];
				debug_printf (VERBOSE_DEBUG,"Port 8189 value: %d",valor_8189);
				out_port_spectrum_no_time(8189,valor_8189);
			}


			//128k, +2a
                        if  (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
				z80_byte valor_32765=zx_header[48];
				debug_printf (VERBOSE_DEBUG,"Port 32765 value: %d",valor_32765);
                                out_port_spectrum_no_time(32765,valor_32765);
                        }
	}

	fclose(ptr_zxfile);

}




//Funcion auxiliar de Cargar .SP snapshot
void load_sp_snapshot_bytes_48k(z80_byte *buffer_lectura,int leidos,z80_int direccion_destino)
{

	debug_printf (VERBOSE_INFO,"Loading %d bytes at address %d",leidos,direccion_destino);

                        z80_byte byte_leido;

                        for (;leidos>0;leidos--) {
				byte_leido=*buffer_lectura++;
				poke_byte_no_time(direccion_destino++,byte_leido);
			}
}


//Cargar snapshot SP
//formato zx es una extension de sp
void load_sp_snapshot(char *archivo)
{

	
        //Cabecera
        z80_byte sp_header[SP_HEADER_SIZE];

        FILE *ptr_spfile;
        z80_byte *buffer_lectura;

        int leidos;

        //leer datos
        buffer_lectura=malloc(49152);
        if (buffer_lectura==NULL) cpu_panic("Cannot allocate memory when loading .sp file");


        //Load File
        ptr_spfile=fopen(archivo,"rb");
        if (ptr_spfile==NULL) {
                debug_printf(VERBOSE_ERR,"Error opening %s",archivo);
                return;
        }

        //if (ptr_spfile) {
                leidos=fread(sp_header,1,SP_HEADER_SIZE,ptr_spfile);
                if (leidos!=SP_HEADER_SIZE) {
                        debug_printf(VERBOSE_ERR,"Error reading %d bytes of header",SP_HEADER_SIZE);
                        return;
                }
        //}

        //Ver si signatura correcta
        if (sp_header[0]!='S' || sp_header[1]!='P') {
                        debug_printf(VERBOSE_ERR,"Unknown SP signature: 0x%x 0x%x",sp_header[0],sp_header[1]);
                        return;
        }


	//Solo es maquina Spectrum 48k
        current_machine_type=1;

        set_machine(NULL);
        reset_cpu();

        load_zxsp_snapshot_common_registers(sp_header);

        //cargar datos

        leidos=fread(buffer_lectura,1,49152,ptr_spfile);
        load_sp_snapshot_bytes_48k(buffer_lectura,leidos,16384);

	fclose(ptr_spfile);

}

void load_sna_snapshot_common_registers(z80_byte *header)
{

/*
   Offset   Size   Description
   ------------------------------------------------------------------------
   0        1      byte   I
   1        8      word   HL',DE',BC',AF'
   9        10     word   HL,DE,BC,IY,IX
   19       1      byte   Interrupt (bit 2 contains IFF2, 1=EI/0=DI)
   20       1      byte   R
   21       4      words  AF,SP
   25       1      byte   IntMode (0=IM0/1=IM1/2=IM2)
   26       1      byte   BorderColor (0..7, not used by Spectrum 1.7)
   27       49152  bytes  RAM dump 16384..65535
   ------------------------------------------------------------------------
   Total: 49179 bytes
*/

        reg_c=header[13];
        reg_b=header[14];
        reg_e=header[11];
        reg_d=header[12];
        reg_l=header[9];
        reg_h=header[10];

        store_flags(header[21]);
        reg_a=header[22];

        reg_ix=value_8_to_16(header[18],header[17]);
        reg_iy=value_8_to_16(header[16],header[15]);

        reg_c_shadow=header[5];
        reg_b_shadow=header[6];
        reg_e_shadow=header[3];
        reg_d_shadow=header[4];
        reg_l_shadow=header[1];
        reg_h_shadow=header[2];

        store_flags_shadow(header[7]);
        reg_a_shadow=header[8];

        reg_r=header[20];
        reg_r_bit7=reg_r&128;

        reg_i=header[0];

        reg_sp=value_8_to_16(header[24],header[23]);

        out_254=header[26] & 7;
        modificado_border.v=1;

        im_mode=header[25] & 3;
	//valor fuera de rango
	if (im_mode==3) im_mode=2;

	//$13  IFF2    [Only bit 2 is defined: 1 for EI, 0 for DI]
	//printf ("header 19: %d\n",header[19]);
	if (header[19] & 4) iff1.v=iff2.v=1;
	else iff1.v=iff2.v=0;
	

	//Lo siguiente es incorrecto. Segun https://faqwiki.zxnet.co.uk/wiki/SNA_format
	//At least one source[http://www.zx-modules.de/fileformats/snaformat.html] 
	//incorrectly states that bit 0 of byte $13 holds the state of IFF1.

	/*
	if (header[19] & 4) iff2.v=1;
	else iff2.v=0;

	if (header[19] & 1) iff1.v=1;
	else iff1.v=0;	
	*/



}

void load_sna_snapshot_bytes_128k(z80_byte *buffer_lectura,z80_byte pagina_entra)
{

	debug_printf (VERBOSE_INFO,"Reading 16Kb block at RAM page %d",pagina_entra);

	z80_byte valor_puerto_32765=(puerto_32765&(255-7));
	out_port_spectrum_no_time(32765,valor_puerto_32765 | pagina_entra);

	z80_int direccion_destino=49152;
	int l;
	z80_byte byte_leido;
	for (l=0;l<16384;l++) {
		byte_leido=*buffer_lectura;
		buffer_lectura++;
		poke_byte_no_time(direccion_destino++,byte_leido);
	}
}

int load_sna_snapshot_must_change_machine(void)
{
                            //Si setting de no cambiar maquina al cargar sna
                        //Se cambia siempre por defecto. Pero si se activa el setting, no cambiarlo a no ser que maquina no sea spectrum
                        int cambiar_maquina=1;

                        if (sna_setting_no_change_machine.v) cambiar_maquina=0;

                        if (!MACHINE_IS_SPECTRUM) cambiar_maquina=1;
    return cambiar_maquina;
}

//Cargar snapshot sna
void load_sna_snapshot(char *archivo)
{

        
        //Cabeceras
        z80_byte sna_48k_header[SNA_48K_HEADER_SIZE];

	
	//cabecera adicional para 128k
        z80_byte sna_128k_header[SNA_128K_HEADER_SIZE];

        FILE *ptr_snafile;
        z80_byte *buffer_lectura;

        int leidos;


	//ver tamaño archivo
	//si 49179, snapshot de 48k
        struct stat buf_stat;

              //Escribir cabecera tzx. Pero si el archivo lo reutilizamos, tendra longitud>0, y no debemos reescribir la cabecera

                if (stat(archivo, &buf_stat)!=0) {
                        debug_printf(VERBOSE_ERR,"Unable to get status of file %s",archivo);
                }

                else {

			off_t     size;
			size=buf_stat.st_size;

			switch (size) {
				case 49179:
					//Archivo de 48k
					debug_printf (VERBOSE_INFO,".SNA 48k file");

				        //leer datos
				        buffer_lectura=malloc(49152);
				        if (buffer_lectura==NULL) cpu_panic("Cannot allocate memory when loading .sna file");


				        //Load File
				        ptr_snafile=fopen(archivo,"rb");
				        if (ptr_snafile==NULL) {
				                debug_printf(VERBOSE_ERR,"Error opening %s",archivo);
				                return;
				        }

			                leidos=fread(sna_48k_header,1,SNA_48K_HEADER_SIZE,ptr_snafile);
			                if (leidos!=SNA_48K_HEADER_SIZE) {
			                        debug_printf(VERBOSE_ERR,"Error reading %d bytes of header",SNA_48K_HEADER_SIZE);
		                        	return;
                			}


                        //Si setting de no cambiar maquina al cargar sna
                        

                        if (load_sna_snapshot_must_change_machine() ) {

				            //maquina Spectrum 48k
				            current_machine_type=1;

				            set_machine(NULL);
                        }
				        reset_cpu();

                        

        				load_sna_snapshot_common_registers(sna_48k_header);

				        //cargar datos

				        leidos=fread(buffer_lectura,1,49152,ptr_snafile);
				        load_sp_snapshot_bytes_48k(buffer_lectura,leidos,16384);

				        fclose(ptr_snafile);

				        //reg_pc viene de pop
				        reg_pc=pop_valor();
					free(buffer_lectura);
					return;


				break;

/*
The 128K version of the .sna format is the same as above, with extensions to include the extra memory banks of the 128K/+2 machines, and fixes the problem with the PC being pushed onto the stack - now it is located in an extra variable in the file (and is not pushed onto the stack at all). The first 49179 bytes of the snapshot are otherwise exactly as described above, so the full description is:

   Offset   Size   Description
   ------------------------------------------------------------------------
   0        27     bytes  SNA header (see above)
   27       16Kb   bytes  RAM bank 5 \
   16411    16Kb   bytes  RAM bank 2  } - as standard 48Kb SNA file
   32795    16Kb   bytes  RAM bank n / (currently paged bank)
   49179    2      word   PC
   49181    1      byte   port 0x7ffd setting
   49182    1      byte   TR-DOS rom paged (1) or not (0)
   49183    16Kb   bytes  remaining RAM banks in ascending order
   ...
   ------------------------------------------------------------------------
   Total: 131103 or 147487 bytes

The third RAM bank saved is always the one currently paged, even if this is page 5 or 2 - in this case, the bank is actually included twice. 
The remaining RAM banks are saved in ascending order - e.g. if RAM bank 4 is paged in, 
the snapshot is made up of banks 5, 2 and 4 to start with, and banks 0, 1, 3, 6 and 7 afterwards. 
If RAM bank 5 is paged in, the snapshot is made up of banks 5, 2 and 5 again, followed by banks 0, 1, 3, 4, 6 and 7.
*/

				case 131103:
				case 147487:
					//Archivo de 128k
					debug_printf (VERBOSE_INFO,".SNA 128k file");

					//TODO: Fuse carga esto como maquina Pentagon. Normal?

					//leer datos
					buffer_lectura=malloc(16384);
					if (buffer_lectura==NULL) cpu_panic("Cannot allocate memory when loading .sna file");


					//Load File
					ptr_snafile=fopen(archivo,"rb");
					if (ptr_snafile==NULL) {
							debug_printf(VERBOSE_ERR,"Error opening %s",archivo);
							return;
					}

					leidos=fread(sna_48k_header,1,SNA_48K_HEADER_SIZE,ptr_snafile);
					if (leidos!=SNA_48K_HEADER_SIZE) {
							debug_printf(VERBOSE_ERR,"Error reading %d bytes of header",SNA_48K_HEADER_SIZE);
							return;
					}

					if (load_sna_snapshot_must_change_machine() ) {
						//maquina Spectrum 128k
						current_machine_type=6;

						set_machine(NULL);
					}
					reset_cpu();

					load_sna_snapshot_common_registers(sna_48k_header);

					//Suponemos primero pagina 0, para habilitar paginacion, por si estuviera deshabilitada
					puerto_32765=0;

					z80_byte valor_puerto_32765;

					//cargar datos
					//leemos ram 5
					leidos=fread(buffer_lectura,1,16384,ptr_snafile);
					load_sna_snapshot_bytes_128k(buffer_lectura,5);

					//cargar datos
					//leemos ram 2
					leidos=fread(buffer_lectura,1,16384,ptr_snafile);
					load_sna_snapshot_bytes_128k(buffer_lectura,2);

					//leer ram N. luego veremos a donde corresponde
					leidos=fread(buffer_lectura,1,16384,ptr_snafile);
					/*
					   49179    2      word   PC
					   49181    1      byte   port 0x7ffd setting
					   49182    1      byte   TR-DOS rom paged (1) or not (0)
					   49183    16Kb   bytes  remaining RAM banks in ascending order
					*/

					leidos=fread(sna_128k_header,1,SNA_128K_HEADER_SIZE,ptr_snafile);
					reg_pc=value_8_to_16(sna_128k_header[1],sna_128k_header[0]);

					valor_puerto_32765=sna_128k_header[2];
					z80_byte ram_paged=valor_puerto_32765&7;

					//TODO: usar el byte de TR-DOS rom paged (1) or not (0)

					//cargar esa pagina
					load_sna_snapshot_bytes_128k(buffer_lectura,ram_paged);

					//Cargar RAMS 0,1,3,4,6,7. Si ram_paged es alguna de esas, no cargarla
					z80_byte paginas[6]={0,1,3,4,6,7};
					int i;
					for (i=0;i<6;i++) {
						z80_byte pagina_entra=paginas[i];
						if (pagina_entra!=ram_paged) {
							leidos=fread(buffer_lectura,1,16384,ptr_snafile);
							load_sna_snapshot_bytes_128k(buffer_lectura,pagina_entra);
						}
					}

					//Y dejar pagina RAM normal
					//valor_puerto_32765=(puerto_32765&(255-7));
					out_port_spectrum_no_time(32765,valor_puerto_32765);

					fclose(ptr_snafile);

					free(buffer_lectura);
					return;


				break;

				default:
					debug_printf(VERBOSE_ERR,".SNA file corrupt");
				break;
			}
		}


}


void load_snx_snapshot(char *archivo)
{
    load_sna_snapshot(archivo);


/*
".snx is a Spectrum snapshot file, more suitable as emulator compatibility than a real format. 
It is identical to a 128K .sna file, but when loaded using the browser or the SPECTRUM command, 
NextZXOS leaves file handle 0 open for further use by the program. 
The program is expected close the handle before exiting. .snx files may also have private data appended to them. 
They are not supported by esxDOS."
*/  

    //Si no esta esxdos handler habilitado, avisar y no hacer nada mas
    if (esxdos_handler_enabled.v) {
        load_nex_snapshot_open_esxdos(archivo,1);
        //printf("handler: %d\n",esx_file_handler);
    }


          
}

//Escribe bytes de repeticion, si conviene, o bytes aislados
void save_z80_snapshot_escribe_repeticion(z80_byte **puntero,z80_byte byte_repeticion,z80_byte veces_repeticion,z80_byte byte_antes_repeticion)
{


			z80_byte *archivo;
			archivo=*puntero;

			//si el byte de antes era justo 237 entonces...
			//237 de antes de la repetición,byte a repetir,237,237,byte a repetir,numero de veces-1
			if (byte_antes_repeticion==237) {
				*archivo++=byte_repeticion;
				veces_repeticion--;
			}


                        if (veces_repeticion>4 || (byte_repeticion==237 && veces_repeticion>=2) ) {
				z80_byte buffer[4];
				buffer[0]=237;
				buffer[1]=237;
				buffer[2]=veces_repeticion;
				buffer[3]=byte_repeticion;
                               	memcpy(archivo,buffer, 4);
										archivo +=4;
                        }

                        else {
                        	for (;veces_repeticion>0;veces_repeticion--) *archivo++=byte_repeticion;
                                	//fwrite(&byte_repeticion, 1, 1, archivo);
                        }
	*puntero=archivo;
}


//Bucle central de grabacion de datos
void save_z80_snapshot_bytes_48k_128k(z80_byte **puntero,int direccion,int si_128k,z80_byte ram_inicial)
{

	z80_byte *archivo;
	archivo=*puntero;

	//int si_dd=0;
	//z80_int direccion=16384;
	z80_byte byte_leido;

	z80_byte byte_repeticion;
	z80_byte veces_repeticion=0;

	z80_byte byte_antes_repeticion=0;

	int direccion_inicial=direccion;

	//valores iniciales
	//valor inicial diferente
	if (si_128k) byte_repeticion=save_z80zx_snapshot_bytes_128k_peek(direccion,ram_inicial)^255;
	else byte_repeticion=peek_byte_no_time(direccion)^255;

	do {
		if (si_128k) byte_leido=save_z80zx_snapshot_bytes_128k_peek(direccion++,ram_inicial);
		else byte_leido=peek_byte_no_time(direccion++);

		if (byte_leido!=byte_repeticion) {
			//escribir bloque si conviene
			save_z80_snapshot_escribe_repeticion(&archivo,byte_repeticion,veces_repeticion,byte_antes_repeticion);

			veces_repeticion=1;
			byte_antes_repeticion=byte_repeticion;
			byte_repeticion=byte_leido;
		}
		else {
			veces_repeticion++;
			//256 veces
			if (veces_repeticion==0) {
				save_z80_snapshot_escribe_repeticion(&archivo,byte_repeticion,255,byte_antes_repeticion);
				veces_repeticion=1;
			}
		}

		//printf ("Direccion: %d direccion_inicial: %d punteroarchivo: %p\n",direccion,direccion_inicial,archivo);

		if (direccion-direccion_inicial>=16384) {
		//if (direccion==0) {
			save_z80_snapshot_escribe_repeticion(&archivo,byte_repeticion,veces_repeticion,byte_antes_repeticion);
			*puntero=archivo;
			return;
		}

	} while(1);
}


//Guardar registros en la cabecera
void save_z80_snapshot_registers(z80_byte *header)
{
        header[0]=reg_a;
	header[1]=get_flags();

        header[3]=reg_b;
        header[2]=reg_c;

        header[5]=reg_h;
        header[4]=reg_l;

	header[6]=0;
	header[7]=0;

	header[9]=value_16_to_8h(reg_sp);
	header[8]=value_16_to_8l(reg_sp);
        header[10]=reg_i;

        header[11]=reg_r;
	z80_byte aux;
	aux=reg_r_bit7 | ((out_254&7)<<1) | 32;
	header[12]=aux;

        header[14]=reg_d;
        header[15]=reg_e;

        header[16]=reg_b_shadow;
        header[15]=reg_c_shadow;

        header[18]=reg_d_shadow;
        header[17]=reg_e_shadow;

        header[20]=reg_h_shadow;
        header[19]=reg_l_shadow;

        header[21]=reg_a_shadow;

	header[22]=get_flags_shadow();

	header[24]=value_16_to_8h(reg_iy);
	header[23]=value_16_to_8l(reg_iy);
	header[26]=value_16_to_8h(reg_ix);
	header[25]=value_16_to_8l(reg_ix);

	header[27]=(iff1.v == 0 ? 0 : 1);

        header[29]=im_mode;

	//length of additional header
	header[30]=55;
	header[31]=0;

	header[33]=value_16_to_8h(reg_pc);
	header[32]=value_16_to_8l(reg_pc);
}


//guardar cabecera de cada bloque de 16kb
void save_z80_put_data_header(z80_byte *buffer,z80_int length,z80_byte page)
{
	buffer[0]=value_16_to_8l(length);
	buffer[1]=value_16_to_8h(length);
	buffer[2]=page;
}

//guardar cada bloque de 16kb
void save_z80_block_16kb (FILE *ptr_z80file,z80_byte *buff,z80_int dir,int si_128k,z80_byte ram_inicial,z80_byte page)
{

	int bytes;


                z80_byte *buff2;
                buff2=buff;

		//como la cabecera va antes que el bloque, nos apuntamos la posicion para recuperarla despues

                buff2 +=3;

                save_z80_snapshot_bytes_48k_128k(&buff2,dir,si_128k,ram_inicial);


                bytes=buff2-buff-3;
		//printf ("puntero antes: %p puntero despues: %p bytes total: %d\n",buff,buff2,bytes);


                save_z80_put_data_header(buff,bytes,page);

		if (si_128k) debug_printf (VERBOSE_INFO,"Saving 16k compressed block. Initial dir: %d RAM: %d Header page: %d Length=%d",dir,ram_inicial,page,bytes);
		else debug_printf (VERBOSE_INFO,"Saving 16k compressed block. Initial dir: %d Header page: %d Length=%d",dir,page,bytes);

                fwrite(buff,1,bytes+3,ptr_z80file);


}

void copy_ay_registers_to_mem(z80_byte *dest)
{
	int i;
	for (i=0;i<16;i++,dest++) {
		*dest=ay_3_8912_registros[0][i];
	}
}



//Guardar snapshot Z80
void save_z80_snapshot(char *filename)
{



	z80_byte header[Z80_HEADER_SIZE_SAVE];


       FILE *ptr_z80file;


	//Create header


	z80_byte maquina_header;

	z80_byte byte37=1 | (ay_chip_present.v<<2);

	z80_bit modify_hardware;
	modify_hardware.v=0;

	switch (current_machine_type)
	{

		//16kb
		case 0:
			maquina_header=0;
			modify_hardware.v=1;
		break;

		//48kb
		case 1:
			maquina_header=0;
		break;

		//Sinclair 128k
		case 6:
			maquina_header=4;
		break;

		//Amstrad +2
		case 8:
			maquina_header=12;
		break;

		//Amstrad +2A (ROM v4.0)
		case MACHINE_ID_SPECTRUM_P2A_40:
			maquina_header=13;
		break;

        //Amstrad +2A (ROM v4.1)
        case MACHINE_ID_SPECTRUM_P2A_41:
            maquina_header=13;
			debug_printf (VERBOSE_ERR,"Saved Amstrad +2A (ROM v4.1) as Z80 snapshot. It will be loaded as Amstrad +2A (ROM v4.0), so it may fail");
    	break;

		//Amstrad +2A (Spanish ROM)
		case MACHINE_ID_SPECTRUM_P2A_SPA:
				maquina_header=13;
				debug_printf (VERBOSE_ERR,"Saved Amstrad +2A (Spanish ROM) as Z80 snapshot. It will be loaded as Amstrad +2A (ROM v4.0), so it may fail");
		break;				



		//Amstrad +3 (ROM v4.0)
		case MACHINE_ID_SPECTRUM_P3_40:
			maquina_header=7;
		break;

        //Amstrad +3 (ROM v4.1)
        case MACHINE_ID_SPECTRUM_P3_41:
            maquina_header=7;
			debug_printf (VERBOSE_ERR,"Saved Amstrad +3 (ROM v4.1) as Z80 snapshot. It will be loaded as Amstrad +3 (ROM v4.0), so it may fail");
    	break;

		//Amstrad +3 (Spanish ROM)
		case MACHINE_ID_SPECTRUM_P3_SPA:
				maquina_header=7;
				debug_printf (VERBOSE_ERR,"Saved Amstrad +3 (Spanish ROM) as Z80 snapshot. It will be loaded as Amstrad +3 (ROM v4.0), so it may fail");
		break;			


		//Pentagon 128
		case 21:
			maquina_header=9;
		break;


		default:
	                debug_printf (VERBOSE_ERR,".Z80 Snapshot not supported on machine %s",get_machine_name(current_machine_type));
        	        return;
		break;
	}




	header[34]=maquina_header;

	byte37=byte37 | (modify_hardware.v*128);
	header[37]=byte37;


	save_z80_snapshot_registers(header);


	header[35]=puerto_32765;
	header[86]=puerto_8189;


	header[54]=ay_3_8912_registro_sel[0];

	//chip sonido
	//memcpy(&header[39],ay_3_8912_registros,16);
	copy_ay_registers_to_mem(&header[39]);

	//otros valores a poner a 0
	header[28]=0;
	//TODO: Low T state counter
	header[55]=0;
	header[56]=0;
	//TODO: Hi T state counter
	header[57]=0;
	header[58]=0;
	header[59]=0;
	header[60]=0;

	//61      1       0xff if 0-8191 is ROM, 0 if RAM
        //62      1       0xff if 8192-16383 is ROM, 0 if RAM
	header[61]=255;
	header[62]=255;

	//otros mas siguientes a cero...
	int i;
	for (i=63;i<=85;i++) header[i]=0;

        //Save header File
        ptr_z80file=fopen(filename,"wb");
        if (!ptr_z80file) {
		debug_printf (VERBOSE_ERR,"Error writing snapshot file %s",filename);
		return;
	}

	fwrite(header, 1, Z80_HEADER_SIZE_SAVE, ptr_z80file);


	//Escritura de datos

	z80_byte *buff;

	//esto es para bloques normalmente de 16kb
	buff=malloc(20000);
        if (buff==NULL) cpu_panic("Cannot allocate memory when saving .z80 file");

	if (MACHINE_IS_SPECTRUM_16_48) {
		//Escritura de 48k
		//tres bloques de 16kb cada uno
		//debug_printf (VERBOSE_INFO,"Saving 16kb block for address 16384-32767");
		save_z80_block_16kb (ptr_z80file,buff,16384,0,0,8);

		//debug_printf (VERBOSE_INFO,"Saving 16kb block for address 32768-49151");
		save_z80_block_16kb (ptr_z80file,buff,32768,0,0,4);

		//debug_printf (VERBOSE_INFO,"Saving 16kb block for address 49152-65535");
		save_z80_block_16kb (ptr_z80file,buff,49152,0,0,5);



		}
	else if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
		//Escritura de 128k. 8 bloques de 16kb cada uno
		int i;
		for (i=0;i<8;i++) {
			save_z80_block_16kb (ptr_z80file,buff,0,1,i,3+i);
		}
	}


	//liberar buff
	free(buff);

	fclose(ptr_z80file);


}


//Cargar registros de cabecera Z80
void load_z80_snapshot_common_registers(z80_byte *header)
{
	reg_a=header[0];
	store_flags(header[1]);

	reg_c=header[2];
	reg_b=header[3];

	reg_l=header[4];
	reg_h=header[5];


	reg_sp=value_8_to_16(header[9],header[8]);
	reg_i=header[10];

	reg_r=header[11];
	reg_r_bit7=(header[12]&1) <<7;

	out_254=((header[12])>>1) & 7;
	modificado_border.v=1;

	reg_e=header[13];
	reg_d=header[14];

        reg_c_shadow=header[15];
        reg_b_shadow=header[16];

        reg_e_shadow=header[17];
        reg_d_shadow=header[18];

        reg_l_shadow=header[19];
        reg_h_shadow=header[20];

	reg_a_shadow=header[21];

	store_flags_shadow(header[22]);

	reg_iy=value_8_to_16(header[24],header[23]);
	reg_ix=value_8_to_16(header[26],header[25]);

	im_mode=header[29] & 3;
	iff1.v=iff2.v=(header[27] == 0 ? 0 : 1);

}

void load_z80_snapshot_bytes_poke(z80_int direccion_destino,z80_byte valor,z80_byte *puntero_memoria)
{
	if (puntero_memoria==NULL) {
		poke_byte_no_time(direccion_destino,valor);
	}
	else {
		puntero_memoria[direccion_destino]=valor;
	}
}

//Bucle de lectura de datos de Z80
//Vale tanto para cargar en maquina emulada como para un puntero de memoria (dependiendo de si puntero_memoria es NULL o no)
void load_z80_snapshot_bytes(z80_byte *buffer_lectura,int leidos,z80_int direccion_destino,int comprimido,z80_byte *puntero_memoria)
{
			int si_ed=0;
			z80_byte byte_leido;

                        for (;leidos>0;leidos--) {
                                byte_leido=*buffer_lectura++;
                                //printf ("inicio bucle leidos: %d byte leido: %d direccion: %d\n",leidos,byte_leido,direccion_destino);
                                if (si_ed && comprimido) {
                                        //si siguiente ed, hay repeticion
                                        if (byte_leido==0xED) {
                                                //hay repeticion
                                                //leemos veces y caracter
                                                z80_byte byte_veces=*buffer_lectura++;
                                                z80_byte byte_repetir=*buffer_lectura++;
                                                //printf ("bloque repeticion. byte: %d veces: %d\n",byte_repetir,byte_veces);
                                                leidos -=2;

                                                if (leidos>0) for (;byte_veces;byte_veces--) {
													//poke_byte_no_time(direccion_destino++,byte_repetir);
													load_z80_snapshot_bytes_poke(direccion_destino++,byte_repetir,puntero_memoria);
												}
                                                else debug_printf(VERBOSE_INFO,"It seems end of block. Don't do byte repetition");
                                                //TODO: The block is terminated by an end marker, 00 ED ED 00.
                                        }

                                        //siguiente no era ED. pokeamos los dos
                                        else {
                                                //poke_byte_no_time(direccion_destino++,0xED);
												load_z80_snapshot_bytes_poke(direccion_destino++,0xED,puntero_memoria);
                                                //poke_byte_no_time(direccion_destino++,byte_leido);
												load_z80_snapshot_bytes_poke(direccion_destino++,byte_leido,puntero_memoria);
                                        }
                                        si_ed=0;
                                }
                                else {
                                        if (byte_leido==0xED && comprimido) {
                                                //primer ED
                                                si_ed=1;
                                        }
                                        else {
                                                //poke_byte_no_time(direccion_destino++,byte_leido);
												load_z80_snapshot_bytes_poke(direccion_destino++,byte_leido,puntero_memoria);
                                        }
                                }
                                //printf ("fin bucle leidos: %d byte leido: %d direccion: %d\n",leidos,byte_leido,direccion_destino);

                        }
}

void load_ace_snapshot(char *archivo)
{

	current_machine_type=122;
	set_machine(NULL);
	reset_cpu();

	//Cargar archivo empezando en direccion 8192 teniendo en cuenta repeticiones de bytes:
	//1) ED 00       end of file
	//2) ED xx yy    repeat byte yy, xx times.

	z80_int puntero_destino=8192;


        FILE *ptr_acefile;

        //Load File
        ptr_acefile=fopen(archivo,"rb");
        if (ptr_acefile==NULL) {
                debug_printf(VERBOSE_ERR,"Error opening %s",archivo);
                return;
        }

	int final=0;
	z80_byte byte_leido,byte_leido2;

	while (!final) {
		fread(&byte_leido,1,1,ptr_acefile);
		if (feof(ptr_acefile)) final=1;

		else {
			//Si 0xED, casos especiales
			if (byte_leido==0xED) {
				fread(&byte_leido,1,1,ptr_acefile);
				switch (byte_leido) {
					case 0:
						final=1;
					break;

					default:
						//Repeticion de bytes
						fread(&byte_leido2,1,1,ptr_acefile);

						//ED xx yy    repeat byte yy, xx times.
						debug_printf (VERBOSE_PARANOID,"Read ED repeticion byte=0x%02X times=%d",byte_leido2,byte_leido);


						for (;byte_leido;byte_leido--) memoria_spectrum[puntero_destino++]=byte_leido2;
					break;
				}

			}

			else {
				memoria_spectrum[puntero_destino++]=byte_leido;
			}
		}
	}


	fclose(ptr_acefile);


        //Ramtop
        //4000 si ramtop=3FFH (3KB)
        //8000 si ramtop=7FFH (19 KB)
        //C000 si ramtop=BFFH (35 KB)

        z80_int ramtop;
	ramtop=memoria_spectrum[0x2081];
	debug_printf (VERBOSE_DEBUG,"Ramtop byte value: 0x%02X",ramtop);

	//valkyr ocupa 22 kb total el snapshot y en este byte hay 00. Suponemos 19 kb
	//Cualquier valor no normal suponemos 19 KB (80h)
	if (ramtop!=0x40 && ramtop!=0x80 && ramtop!=0xc0) {
		debug_printf (VERBOSE_INFO,"Ramtop byte value unknown: 0x%02X. Assume 80H (35 KB Ram)");
		ramtop=0x80;
	}

	ramtop=ramtop*256;

	ramtop--;
	ramtop_ace=ramtop;

	debug_printf (VERBOSE_INFO,"Setting Ramtop from snapshot: emulating Jupiter Ace with %d KB (ramtop=%d)",(ramtop_ace-16383)/1024+3,ramtop_ace);




	//Asignar registros
	z80_int registros=0x2100;
/*
Addr:	last state             Registers

2100	50, 04, 00, 00		AF
	00, 00, 00, 00		BC
	E2, 26, 00, 00		DE
	28, 3C, 00, 00		HL
	00, 3C, 00, 00		IX
	C8, 04, 00, 00		IY
	FE, 7F, 00, 00		SP
	9D, 05, 00, 00		PC
	40, 20, 00, 00		AF'
	00, 01, 00, 00		BC'
	60, 00, 00, 00		DE'
	80, 26, 00, 00		HL'
	01, 00, 00, 00		IM
	01, 00, 00, 00		IFF1
	01, 00, 00, 00		IFF2
	00, 00, 00, 00		I
	11, 00, 00, 00		R
	80, 00, 00, 00		?
*/

        store_flags(memoria_spectrum[registros++]);
        reg_a=memoria_spectrum[registros++];
        registros +=2;

        reg_c=memoria_spectrum[registros++];
        reg_b=memoria_spectrum[registros++];
        registros +=2;

        reg_e=memoria_spectrum[registros++];
        reg_d=memoria_spectrum[registros++];
        registros +=2;

        reg_l=memoria_spectrum[registros++];
        reg_h=memoria_spectrum[registros++];
        registros +=2;

        reg_ix=value_8_to_16( memoria_spectrum[registros+1],memoria_spectrum[registros] );
        registros +=4;

        reg_iy=value_8_to_16( memoria_spectrum[registros+1],memoria_spectrum[registros] );
        registros +=4;

        reg_sp=value_8_to_16( memoria_spectrum[registros+1],memoria_spectrum[registros] );
        registros +=4;

        reg_pc=value_8_to_16( memoria_spectrum[registros+1],memoria_spectrum[registros] );
        registros +=4;


        store_flags_shadow(memoria_spectrum[registros++]);
        reg_a_shadow=memoria_spectrum[registros++];
        registros +=2;

        reg_c_shadow=memoria_spectrum[registros++];
        reg_b_shadow=memoria_spectrum[registros++];
        registros +=2;

        reg_e_shadow=memoria_spectrum[registros++];
        reg_d_shadow=memoria_spectrum[registros++];
        registros +=2;

        reg_l_shadow=memoria_spectrum[registros++];
        reg_h_shadow=memoria_spectrum[registros++];
        registros +=2;


/*
        01, 00, 00, 00          IM
        01, 00, 00, 00          IFF1
        01, 00, 00, 00          IFF2
        00, 00, 00, 00          I
        11, 00, 00, 00          R
        80, 00, 00, 00          ?
*/

        im_mode=memoria_spectrum[registros] & 2;
        if (im_mode==1) im_mode=2;
        registros +=4;

        iff1.v=iff2.v=memoria_spectrum[registros] &1;
        registros +=8;

        reg_i=memoria_spectrum[registros];
        registros +=4;


        reg_r=memoria_spectrum[registros];
        reg_r_bit7=reg_r&128;
        registros +=4;




}


//Cargar snapshot .z80
void load_z80_snapshot(char *archivo)
{
	//Cabecera comun para todas versiones

	z80_byte z80_header[Z80_MAIN_HEADER_SIZE];

	//Cabecera adicional
	
	z80_byte z80_header_adicional[Z80_AUX_HEADER_SIZE];

	//Cabecera de cada bloque de datos en version 2 o 3
	z80_byte z80_header_bloque[3];

	FILE *ptr_z80file;
	z80_byte *buffer_lectura;

	int leidos;

	int comprimido;
	//int comprimido_orig;
	z80_int direccion_destino;


	z80_byte z80_version;

	//leer datos
        buffer_lectura=malloc(65536);
        if (buffer_lectura==NULL) cpu_panic("Cannot allocate memory when loading .z80 file");


	//Load File
	ptr_z80file=fopen(archivo,"rb");
        if (ptr_z80file==NULL) {
                debug_printf(VERBOSE_ERR,"Error opening %s",archivo);
                return;
        }

	//if (ptr_z80file) {
		leidos=fread(z80_header,1,Z80_MAIN_HEADER_SIZE,ptr_z80file);
		if (leidos!=Z80_MAIN_HEADER_SIZE) {
			debug_printf(VERBOSE_ERR,"Error reading %d bytes of header",Z80_MAIN_HEADER_SIZE);
			return;
		}

		//asignamos registros
		//#define value_8_to_16(h,l) ((h<<8)|l)

		comprimido=(z80_header[12]>>5)&1;
		//printf ("header[12]=%d\n",z80_header[12]);
		//comprimido_orig=comprimido;



		if (z80_header[6]==0 && z80_header[7]==0) {

			//Z80 version 2 o 3

			//leemos longitud de la cabecera adicional
			leidos=fread(z80_header_adicional,1,2,ptr_z80file);
			z80_int long_cabecera_adicional=value_8_to_16(z80_header_adicional[1],z80_header_adicional[0]);
			if (long_cabecera_adicional!= 23 && long_cabecera_adicional!= 54 && long_cabecera_adicional!= 55) {
				debug_printf(VERBOSE_ERR,"Header with %d bytes unknown",long_cabecera_adicional);
				return;
			}
			if (long_cabecera_adicional==23) {
				debug_printf(VERBOSE_INFO,".Z80 version 2 detected");
				z80_version=2;
			}
			else {
				debug_printf(VERBOSE_INFO,".Z80 version 3 detected");
				z80_version=3;
			}

			//leemos esa cabecera adicional
			debug_printf(VERBOSE_INFO,"Reading %d bytes of additional header",long_cabecera_adicional);
			 leidos=fread(&z80_header_adicional[2],1,long_cabecera_adicional,ptr_z80file);

			//leer tipo de maquina
			z80_byte maquina_leida=z80_header_adicional[4];


			z80_byte modify_hardware=z80_header_adicional[7]&128;


			debug_printf(VERBOSE_DEBUG,"Header machine type: %d Modify hardware flag: %d",maquina_leida,modify_hardware);

			if (load_sna_snapshot_must_change_machine() ) {

				switch (maquina_leida) {
					//If bit 7 of byte 37 is set, the hardware types are modified slightly: any 48K machine becomes a 16K machine, any 128K machines becomes a +2 and any +3 machine becomes a +2A.

					case 0:
					case 1:
						//48k
						current_machine_type=1;
						if (modify_hardware) current_machine_type=0;

					break;

					/*case 2:
						//Samram. Aunque hay snapshots mal generados de 3.00 a 3.02 de 128k
						//128k
						current_machine_type=6;
						//if (modify_hardware) current_machine_type=8;					
					break;*/

					case 3:
						if (z80_version==2) {
							//En v2, 128k
							current_machine_type=6;
							if (modify_hardware) current_machine_type=8;

						}

						if (z80_version==3) {
							debug_printf (VERBOSE_WARN,"Setting 48k machine but header says 48k + M.G.T.");
							//En v3, 48k + M.G.T.
							current_machine_type=1;
							if (modify_hardware) current_machine_type=0;
						}
					break;
					case 4:
						//128k
						current_machine_type=6;
						if (modify_hardware) current_machine_type=8;
					break;

					case 5:
						//128k + If.1
						current_machine_type=6;
						debug_printf (VERBOSE_ERR,"128k + If.1 is not emulated yet. Setting to Spectrum 128k");
					break;

					case 6:
						//128k + M.G.T.
						current_machine_type=6;
						debug_printf (VERBOSE_ERR,"128k + M.G.T. is not emulated yet. Setting to Spectrum 128k");
					break;

					case 7:
						//+3
						current_machine_type=MACHINE_ID_SPECTRUM_P3_40;
					break;

					case 8:
						//[mistakenly used by some versions of XZX-Pro to indicate a +3]
						current_machine_type=MACHINE_ID_SPECTRUM_P3_40;
					break;

					case 9:
						//Pentagon 128k
						current_machine_type=21;
					break;

					case 10:
						//Scorpion 256k
						current_machine_type=6;
						debug_printf (VERBOSE_ERR,"Scorpion 256k is not emulated yet. Setting to Spectrum 128k");
					break;

					case 11:
						//Didaktik-Kompakt
						current_machine_type=6;
						debug_printf (VERBOSE_ERR,"Didaktik-Kompakt is not emulated yet. Setting to Spectrum 128k");
					break;

					case 12:
						//+2
						current_machine_type=MACHINE_ID_SPECTRUM_P2;
					break;

					case 13:
						//+2A
						current_machine_type=MACHINE_ID_SPECTRUM_P2A_40;
					break;

					case 14:
					case 15:
						//TC2048
						debug_printf(VERBOSE_ERR,"Unsupported machine type TC2048");
						return;
					break;			

					case 128:
						//TS2068
						debug_printf(VERBOSE_ERR,"Unsupported machine type TS2068");
						return;
					break;									

					default:
						debug_printf(VERBOSE_ERR,"Unknown machine type %d",maquina_leida);
						return;
					break;
				}

            
				set_machine(NULL);
			}

            reset_cpu();

			reg_pc=value_8_to_16(z80_header_adicional[3],z80_header_adicional[2]);
			load_z80_snapshot_common_registers(z80_header);



			//registros del chip AY
			int reg_ay;
			for (reg_ay=0;reg_ay<16;reg_ay++) {
				//Meter valores en registros AY tal cual
                                ay_3_8912_registros[0][reg_ay]=z80_header_adicional[9+reg_ay];
			}



			//y ultimo valor enviado
			// 38      1       Last OUT to port 0xfffd (soundchip register number)
			ay_3_8912_registro_sel[0]=z80_header_adicional[8];


			//AY CHIP
			if ( (z80_header_adicional[7] &4) ) {
					debug_printf(VERBOSE_DEBUG,"AY Chip enabled on z80 snapshot");
					ay_chip_present.v=1;
			}

			else {
					debug_printf(VERBOSE_DEBUG,"AY Chip disabled on z80 snapshot");
					ay_chip_present.v=0;
			}

/* Esto no lo uso
if (long_cabecera_adicional>25) {
  z80_long_int z80_t_estados=z80_header_adicional[25] + 256*z80_header_adicional[26]+
                65536*z80_header_adicional[27];
                printf ("t estados en snapshot z80: %d\n",z80_t_estados);
      }
      */

			z80_byte numerobloque;
			z80_byte valor_puerto_32765;
			z80_int longitudbloque;
			//printf("current machine type: %d\n",current_machine_type);
			do {

			//leer datos
			//cabecera del bloque de 16kb
			leidos=fread(z80_header_bloque,1,3,ptr_z80file);
			comprimido=1;
			if (leidos>0) {
				numerobloque=z80_header_bloque[2];
				longitudbloque=value_8_to_16(z80_header_bloque[1],z80_header_bloque[0]);
				if (longitudbloque==65535) {
					//If length=0xffff, data is 16384 bytes long and not compressed
					longitudbloque=16384;
					comprimido=0;
				}

				if (MACHINE_IS_SPECTRUM_16_48) {
					//gestionar maquinas de 48 k
					switch (numerobloque) {
						case 4:
							direccion_destino=32768;
						break;

						case 5:
							direccion_destino=49152;
						break;

						case 8:
							direccion_destino=16384;
						break;

						case 0:
							//Carga en rom. lo ignoramos
						break;

						default:
							debug_printf(VERBOSE_ERR,"Z80 snapshot page number %d unknown",numerobloque);
							return;
						break;
					}
					debug_printf(VERBOSE_INFO,"Reading %d bytes of data at %d address",longitudbloque,direccion_destino);
					leidos=fread(buffer_lectura,1,longitudbloque,ptr_z80file);

					if (numerobloque!=0) {
						//0 es cargar rom. ignoramos
						load_z80_snapshot_bytes(buffer_lectura,leidos,direccion_destino,comprimido,NULL);
					}


				}
				else {
					//printf("leyendo bloque %d maquina 128k\n",numerobloque);
					//maquinas de 128k
					if ((numerobloque<3 || numerobloque>10) && numerobloque!=0) {
                                                        debug_printf(VERBOSE_ERR,"Page number %d unsupported");
                                                        return;
					}

					int orig_numerobloque=numerobloque;

					if (orig_numerobloque!=0) {
						numerobloque=numerobloque-3;
						//
						//puerto_32765=(puerto_32765&(255-7));
						//puerto_32765=puerto_32765 | numerobloque;
						//mem_page_ram_128k();

						//Paginamos la RAM correspondiente y leemos bloque de 16kb en la pagina 49152-65535

						valor_puerto_32765=(puerto_32765&(255-7));
						out_port_spectrum_no_time(32765,valor_puerto_32765 | numerobloque);
					}

					debug_printf(VERBOSE_INFO,"Reading %d bytes of data at 49152 address page number %d",longitudbloque,numerobloque);
					leidos=fread(buffer_lectura,1,longitudbloque,ptr_z80file);

					if (orig_numerobloque!=0) {
                    	load_z80_snapshot_bytes(buffer_lectura,leidos,49152,comprimido,NULL);
					}

				}
			}

			} while (leidos>0);

			//si modo 128k, dejar bien la paginacion
			if  (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
				out_port_spectrum_no_time(32765,z80_header_adicional[5]);


				if (long_cabecera_adicional==55) {
					if (MACHINE_IS_SPECTRUM_P2A_P3)
						out_port_spectrum_no_time(8189,z80_header_adicional[56]);
				}
			}



		}
		else {
			//.Z80 Version 1

			if (comprimido) debug_printf(VERBOSE_INFO,".Z80 compressed data");
			else debug_printf(VERBOSE_INFO,".Z80 non compressed data");

			debug_printf(VERBOSE_INFO,".Z80 Version 1 file detected");

			//esta variable no se usa mucho
			z80_version=1;

			current_machine_type=1;
			set_machine(NULL);
			reset_cpu();

			reg_pc=value_8_to_16(z80_header[7],z80_header[6]);
			load_z80_snapshot_common_registers(z80_header);



			leidos=fread(buffer_lectura,1,65536,ptr_z80file);
			//printf ("despues de carga\n");
			debug_printf(VERBOSE_INFO,"Readed %d bytes of data",leidos);
			//printf ("despues de debug_printf\n");

			//z80_byte byte_leido;

			direccion_destino=16384;

			load_z80_snapshot_bytes(buffer_lectura,leidos,direccion_destino,comprimido,NULL);


		}

	//}

	fclose(ptr_z80file);


}






//Comun para grabar registros en formato ZX y SP
void save_zxsp_snapshot_registers(z80_byte *header)
{


	header[2]=value_16_to_8l(49152);
	header[3]=value_16_to_8h(49152);
	header[4]=value_16_to_8l(16384);
	header[5]=value_16_to_8h(16384);


        header[6]=reg_c;
        header[7]=reg_b;
        header[8]=reg_e;
        header[9]=reg_d;
        header[10]=reg_l;
        header[11]=reg_h;

        header[12]=get_flags();
	header[13]=reg_a;



        header[14]=value_16_to_8l(reg_ix);
        header[15]=value_16_to_8h(reg_ix);
        header[16]=value_16_to_8l(reg_iy);
        header[17]=value_16_to_8h(reg_iy);

        header[18]=reg_c_shadow;
        header[19]=reg_b_shadow;
        header[20]=reg_e_shadow;
        header[21]=reg_d_shadow;
        header[22]=reg_l_shadow;
        header[23]=reg_h_shadow;



	header[24]=get_flags_shadow();

	header[25]=reg_a_shadow;

        header[26]=(reg_r&127) | (reg_r_bit7&128);

        header[27]=reg_i;

	header[28]=value_16_to_8l(reg_sp);
	header[29]=value_16_to_8h(reg_sp);
	header[30]=value_16_to_8l(reg_pc);
	header[31]=value_16_to_8h(reg_pc);

        header[34]=out_254  & 7;

	z80_byte bits_estado=(iff1.v) | (im_mode==2 ? 2 : 0);

	header[36]=bits_estado;

}


//Escribe bytes repeticion, si conviene, o bytes aislados
void save_zx_snapshot_escribe_repeticion(FILE *archivo,z80_byte byte_repeticion,z80_byte veces_repeticion,z80_byte byte_antes_repeticion)
{


			//si el byte de antes era justo 221 entonces...
			//221 de antes de la repetición,byte a repetir,221,221,byte a repetir,numero de veces-1
			if (byte_antes_repeticion==221) {
				fwrite(&byte_repeticion, 1, 1, archivo);
				veces_repeticion--;
			}


                        if (veces_repeticion>4 || (byte_repeticion==221 && veces_repeticion>=2) ) {
				z80_byte buffer[4];
				buffer[0]=221;
				buffer[1]=221;
				buffer[2]=byte_repeticion;
				buffer[3]=veces_repeticion;
                               	fwrite(buffer, 1, 4, archivo);
                        }

                        else {
                        	for (;veces_repeticion>0;veces_repeticion--)
                                	fwrite(&byte_repeticion, 1, 1, archivo);
                        }
}

//Escribe bytes repeticion, si conviene, o bytes aislados
z80_byte *save_zx_z88_snapshot_escribe_repeticion(z80_byte *destino,z80_byte byte_repeticion,z80_byte veces_repeticion,z80_byte byte_antes_repeticion)
{


                        //si el byte de antes era justo 221 entonces...
                        //221 de antes de la repetición,byte a repetir,221,221,byte a repetir,numero de veces-1
                        if (byte_antes_repeticion==221) {
				*destino++=byte_repeticion;
                                veces_repeticion--;
                        }


                        if (veces_repeticion>4 || (byte_repeticion==221 && veces_repeticion>=2) ) {
				*destino++=221;
				*destino++=221;
				*destino++=byte_repeticion;
				*destino++=veces_repeticion;
                        }

                        else {
                                for (;veces_repeticion>0;veces_repeticion--)
					*destino++=byte_repeticion;
                        }


	//devolvemos el puntero
	return destino;
}



//Bucle para guardar datos en ZX
void save_zx_snapshot_bytes_48k_128k(FILE *archivo,z80_int direccion,int si_128k,z80_byte ram_inicial)
{
	z80_byte byte_leido;

	z80_byte byte_repeticion;
	z80_byte veces_repeticion=0;

	z80_byte byte_antes_repeticion=0;

	//valores iniciales
	//valor inicial diferente
	if (si_128k) byte_repeticion=save_z80zx_snapshot_bytes_128k_peek(direccion,ram_inicial)^255;
	else byte_repeticion=peek_byte_no_time(direccion)^255;

	do {
		if (si_128k) byte_leido=save_z80zx_snapshot_bytes_128k_peek(direccion++,ram_inicial);
		else byte_leido=peek_byte_no_time(direccion++);

		if (byte_leido!=byte_repeticion) {
			//escribir bloque si conviene
			save_zx_snapshot_escribe_repeticion(archivo,byte_repeticion,veces_repeticion,byte_antes_repeticion);

			veces_repeticion=1;
			byte_antes_repeticion=byte_repeticion;
			byte_repeticion=byte_leido;
		}
		else {
			veces_repeticion++;
			//256 veces
			if (veces_repeticion==0) {
				save_zx_snapshot_escribe_repeticion(archivo,byte_repeticion,255,byte_antes_repeticion);
				veces_repeticion=1;
			}
		}

		if (direccion==0) {
			save_zx_snapshot_escribe_repeticion(archivo,byte_repeticion,veces_repeticion,byte_antes_repeticion);
			return;
		}

	} while(1);
}


//Bucle para guardar datos en ZX de Z88
//void save_zx_snapshot_bytes_z88(FILE *archivo,z80_byte bank)
z80_byte *save_zx_snapshot_bytes_z88(z80_byte *destino,z80_byte bank)
{
        z80_byte byte_leido;

        z80_byte byte_repeticion;
        z80_byte veces_repeticion=0;

        z80_byte byte_antes_repeticion=0;

	z80_int direccion=0;

        //valores iniciales
        //valor inicial diferente
        byte_repeticion=peek_byte_no_time_z88_bank_no_check_low(direccion,bank)^255;

        do {
                byte_leido=peek_byte_no_time_z88_bank_no_check_low(direccion++,bank);

                if (byte_leido!=byte_repeticion) {
                        //escribir bloque si conviene
                        //save_zx_snapshot_escribe_repeticion(archivo,byte_repeticion,veces_repeticion,byte_antes_repeticion);
                        destino=save_zx_z88_snapshot_escribe_repeticion(destino,byte_repeticion,veces_repeticion,byte_antes_repeticion);

                        veces_repeticion=1;
                        byte_antes_repeticion=byte_repeticion;
                        byte_repeticion=byte_leido;
                }
                else {
                        veces_repeticion++;
                        //256 veces
                        if (veces_repeticion==0) {
                                //save_zx_snapshot_escribe_repeticion(archivo,byte_repeticion,255,byte_antes_repeticion);
                                destino=save_zx_z88_snapshot_escribe_repeticion(destino,byte_repeticion,255,byte_antes_repeticion);
                                veces_repeticion=1;
                        }
                }

                if (direccion==16384) {
                        //save_zx_snapshot_escribe_repeticion(archivo,byte_repeticion,veces_repeticion,byte_antes_repeticion);
                        destino=save_zx_z88_snapshot_escribe_repeticion(destino,byte_repeticion,veces_repeticion,byte_antes_repeticion);
                        return destino;
                }

        } while(1);
}


//para compatibilidad con zxspectr
void save_zx_snapshot_header_pages(z80_byte *header)
{

        //para compatibilidad con zxspectr
        //paginas_actuales

        //ram mapeada normalmente, excepto en casos de ram en rom
        header[53]=(puerto_32765&7)+4;

	//si es modelo 128k, no se mapea ram en rom y todo es mas facil
	if (MACHINE_IS_SPECTRUM_128_P2)  {

	        //rom mapeada
		if ( (puerto_32765&16)==16) header[50]=3;
		else header[50]=0;

	        //ram
	        header[51]=5+4;
	        header[52]=2+4;
	}

	//si modelo +2a
	if (MACHINE_IS_SPECTRUM_P2A_P3)  {
		//ver si estamos ram en rom
		if ( (puerto_8189&1)==1) {
			//ram en rom

			//ver que combinacion es
			 z80_byte page_type;

		        page_type=(puerto_8189 >>1) & 3;

		        switch (page_type) {
                		case 0:
		                        header[50]=0;
		                        header[51]=1+4;
		                        header[52]=2+4;
		                        header[53]=3+4;
		                break;

		                case 1:
		                        header[50]=4;
		                        header[51]=5+4;
		                        header[52]=6+4;
		                        header[53]=7+4;
		                break;

		                case 2:
		                        header[50]=4;
		                        header[51]=5+4;
		                        header[52]=6+4;
		                        header[53]=3+4;
		                break;

		                case 3:
		                        header[50]=4+4;
		                        header[51]=7+4;
		                        header[52]=6+4;
		                        header[53]=3+4;
                		break;

        		}



		}

		else {
		//paginacion normal
			header[50]=( (puerto_32765>>4)&1) | ((puerto_8189>>1)&2);
	                //ram
        	        header[51]=5+4;
                	header[52]=2+4;

		}
	}

	//printf ("cabecera 128k: %d %d %d %d\n",header[50],header[51],header[52],header[53]);


}

z80_byte get_maquina_header(void)
{

	z80_byte maquina_header;

        switch (current_machine_type)
        {
                //0=Sinclair 16k
                case 0:
                        maquina_header=0;
                break;

                //1=Sinclair 48k
                case 1:
                        maquina_header=1;
                break;

                //2=Inves Spectrum+
                case 2:
                        maquina_header=2;
                break;

                //3=tk90x
                case 3:
                        maquina_header=11;
                break;

                //4=tk90xs
                case 4:
                        maquina_header=12;
                break;

                //5=tk95
                case 5:
                        maquina_header=13;
                break;

                //6=Sinclair 128k
                case 6:
                        maquina_header=3;
                break;
                //7=Sinclair 128k Español
                case 7:
                        maquina_header=10;
                break;

                //8=Amstrad +2
                case 8:
                        maquina_header=4;
                break;

                //9=Amstrad +2 - Frances
                case 9:
                        maquina_header=5;
                break;

                //10=Amstrad +2 - Spanish
                case 10:
                        maquina_header=6;
                break;

                //11=Amstrad +2A (ROM v4.0)
                case 11:
                        maquina_header=7;
                break;

                //12=Amstrad +2A (ROM v4.1)
                case 12:
                        maquina_header=8;
                break;

                //13=Amstrad +2A -Spanish
                case 13:
                        maquina_header=9;
                break;

		//14=ZX-Uno
		case 14:
			maquina_header=20;
		break;

		//Chloe 140SE
		case 15:
			maquina_header=21;
		break;

		//Chloe 280SE
		case 16:
			maquina_header=22;
		break;

		//17=Timex TS2068
		case 17:
			maquina_header=19;
		break;

		//18=Prism
		case 18:
			maquina_header=23;
		break;

                //20=Spectrum 48k Spanish
                case 20:
                        maquina_header=24;
                break;

                //20=zx80
                case 120:
                        maquina_header=14;
                break;

                //21=zx81
                case 121:
                        maquina_header=15;
                break;

                //22=jupiter ace
                case 122:
                        maquina_header=17;
                break;




		//30=Z88
		case 130:
			maquina_header=16;
		break;

		//40=CPC_464
		case 140:
			maquina_header=18;
		break;

                default:
			maquina_header=255;
                break;
        }


	return maquina_header;
}


void save_zx_z88_block_16kb(FILE *ptr_zxfile,z80_byte *buffer_z88_save,z80_byte bank)
{

	z80_int longitud;
	z80_byte *final_buffer;

                        //Cabecera bloque 16kb: bank, longitud
                        buffer_z88_save[0]=bank;
                        final_buffer=save_zx_snapshot_bytes_z88(&buffer_z88_save[3],bank);
                        longitud=final_buffer-buffer_z88_save;
                        //Meter en cabecera 16kb longitud bloque
                        z80_int longitud_en_header=longitud-3;
                        buffer_z88_save[1]=value_16_to_8l(longitud_en_header);
                        buffer_z88_save[2]=value_16_to_8h(longitud_en_header);
                        debug_printf (VERBOSE_INFO,"Saving 16 KB block with %d bytes of compressed data from bank %02XH",longitud_en_header,bank);
                        fwrite(buffer_z88_save,1,longitud,ptr_zxfile);


}


//Escribe bytes repeticion, si conviene, o bytes aislados
z80_byte *save_zx_snapshot_generic_escribe_repeticion(z80_byte *destino,z80_byte byte_repeticion,z80_byte veces_repeticion,z80_byte byte_antes_repeticion)
{


                        //si el byte de antes era justo 221 entonces...
                        //221 de antes de la repetición,byte a repetir,221,221,byte a repetir,numero de veces-1
                        if (byte_antes_repeticion==221) {
                                *destino++=byte_repeticion;
                                veces_repeticion--;
                        }


                        if (veces_repeticion>4 || (byte_repeticion==221 && veces_repeticion>=2) ) {
                                *destino++=221;
                                *destino++=221;
                                *destino++=byte_repeticion;
                                *destino++=veces_repeticion;
                        }

                        else {
                                for (;veces_repeticion>0;veces_repeticion--)
                                        *destino++=byte_repeticion;
                        }


        //devolvemos el puntero
        return destino;
}



z80_byte *save_zx_snapshot_bytes_generic_16kb(z80_byte *destino,z80_byte *puntero)
{
        z80_byte byte_leido;

        z80_byte byte_repeticion;
        z80_byte veces_repeticion=0;

        z80_byte byte_antes_repeticion=0;

        z80_int direccion=0;

        //valores iniciales
        //valor inicial diferente
        byte_repeticion=(*puntero)^255;

        do {
                byte_leido=puntero[direccion++];

                if (byte_leido!=byte_repeticion) {
                        //escribir bloque si conviene
                        destino=save_zx_snapshot_generic_escribe_repeticion(destino,byte_repeticion,veces_repeticion,byte_antes_repeticion);

                        veces_repeticion=1;
                        byte_antes_repeticion=byte_repeticion;
                        byte_repeticion=byte_leido;
                }
                else {
                        veces_repeticion++;
                        //256 veces
                        if (veces_repeticion==0) {
                                destino=save_zx_snapshot_generic_escribe_repeticion(destino,byte_repeticion,255,byte_antes_repeticion);
                                veces_repeticion=1;
                        }
                }

                if (direccion==16384) {
                        destino=save_zx_snapshot_generic_escribe_repeticion(destino,byte_repeticion,veces_repeticion,byte_antes_repeticion);
                        return destino;
                }

        } while(1);
}


void save_zx_generic_block_16kb(FILE *ptr_zxfile,z80_byte *buffer_generic_save,z80_byte *puntero,z80_int numero_bloque)
{

        z80_int longitud;
        z80_byte *final_buffer;

	const int size_mini_header=4;

                        //Cabecera bloque 16kb: numero_bloque(16 bit), longitud
                        buffer_generic_save[0]=value_16_to_8l(numero_bloque);
			buffer_generic_save[1]=value_16_to_8h(numero_bloque);

                        final_buffer=save_zx_snapshot_bytes_generic_16kb(&buffer_generic_save[size_mini_header],puntero);
                        longitud=final_buffer-buffer_generic_save;

                        //Meter en cabecera 16kb longitud bloque
                        z80_int longitud_en_header=longitud-size_mini_header;
                        buffer_generic_save[2]=value_16_to_8l(longitud_en_header);
                        buffer_generic_save[3]=value_16_to_8h(longitud_en_header);
                        debug_printf (VERBOSE_INFO,"Saving 16 KB block with %d bytes of compressed data from block %d",longitud_en_header,numero_bloque);
                        fwrite(buffer_generic_save,1,longitud,ptr_zxfile);


}



//Grabar Snapshot ZX
void save_zx_snapshot(char *filename)
{



	z80_byte header[ZX_HEADER_SIZE];

	//Inicializar todos los valores a 255, asi los que no se usan tendran ese valor
        int i;
        for (i=0;i<=293;i++) header[i]=255;





       FILE *ptr_zxfile;


	//Create header

	header[0]='Z';
	header[1]='X';


	//ZX Version 4
	//z80_byte snap_zx_version_save=4;

	debug_printf (VERBOSE_INFO,"Saving .zx snapshot version %d",snap_zx_version_save);


	//Version 1 solo soporta 48k
	if (snap_zx_version_save==1) {
		if (!MACHINE_IS_SPECTRUM_48) {
			debug_printf (VERBOSE_ERR,"Machine type not supported on snapshot version %d",snap_zx_version_save);
	                return;
		}
        }



        //Si version 2, no permitir zx80, 81
        //resto de maquinas spectrum se grabaran como 48k o 128k, al igual que se hacia en ZXSpectr
        if (MACHINE_IS_ZX8081 && snap_zx_version_save<3) {
		debug_printf (VERBOSE_ERR,"Machine type not supported on snapshot version %d",snap_zx_version_save);
		return;
	}

	//Si version inferior a 5, no permitir z88
	if ((MACHINE_IS_Z88) && snap_zx_version_save<5) {
		debug_printf (VERBOSE_ERR,"Machine type not supported on snapshot version %d",snap_zx_version_save);
                return;
        }

	//Si version inferior a 6, no permitir ACE ni otras
	if ((MACHINE_IS_ACE || MACHINE_IS_CPC || MACHINE_IS_ZXUNO || MACHINE_IS_TIMEX_TS2068 || MACHINE_IS_PRISM || MACHINE_IS_CHLOE || MACHINE_IS_PRISM || MACHINE_IS_SPECTRUM_48_SPA) && snap_zx_version_save<6) {
		debug_printf (VERBOSE_ERR,"Machine type not supported on snapshot version %d",snap_zx_version_save);
                return;
        }



	header[38]=snap_zx_version_save;

	save_zxsp_snapshot_registers(header);


        //Valores para zx version>=1
        if (snap_zx_version_save>=1) {
	        //brillo
	        header[44]=0;

        	//sonido
	        header[46]=1;

		//disparador automatico
		z80_byte bits_estado0=0;
		if (joystick_autofire_frequency!=0) bits_estado0=bits_estado0 | 64;
		header[47]=bits_estado0;

                //disparador frecuencia. Si es 0, no meter un 0. Para compatibilidad con ZXSpectr
                if (joystick_autofire_frequency!=0) header[45]=joystick_autofire_frequency;
                else header[45]=1;

	}

	//Valores para zx version>=2
	if (snap_zx_version_save>=2) {
        	//otros bits importantes
	        z80_byte bits_estado0=0;
        	//bit4 A 1 indica que el programa a cargar es de 128k (versión 2+)
	        //bits_estado0: 47
        	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) bits_estado0=bits_estado0 | 16;

	        //disparador activo
	        if (joystick_autofire_frequency!=0) bits_estado0=bits_estado0 | 64;

        	header[47]=bits_estado0;


	        //disparador frecuencia. Si es 0, no meter un 0. Para compatibilidad con ZXSpectr
        	if (joystick_autofire_frequency!=0) header[45]=joystick_autofire_frequency;
        	else header[45]=1;

	        header[48]=puerto_32765;
	        header[49]=puerto_8189;

		save_zx_snapshot_header_pages(header);

	        header[54]=ay_3_8912_registro_sel[0];

	        //chip sonido
        	//memcpy(&header[55],ay_3_8912_registros,16);
		copy_ay_registers_to_mem(&header[55]);

	}


        //Valores para zx version>=3
        if (snap_zx_version_save>=3) {
        	z80_byte maquina_header;

	        maquina_header=get_maquina_header();
        	if (maquina_header==255) {
	                debug_printf (VERBOSE_ERR,".ZX Snapshot not supported on machine %s",get_machine_name(current_machine_type));
        	        return;
	        }

		header[71]=maquina_header;

	}

	//Valores para zx version>=4
        if (snap_zx_version_save>=4) {

		//Generar bits_estado2
        	//z80_byte bits_estado2=header[72];
	        z80_byte bits_estado2=0;

		//bits_estado2=(nmi_generator_active.v) + (2*hsync_generator_active.v) + (4*rainbow_enabled.v) + (8*wrx_present.v) + (16*ram_in_8192.v);

		//z80_byte pruebas=(nmi_generator_active.v) + (2*hsync_generator_active.v) + (4*rainbow_enabled.v) + (8*wrx_present.v) + (16*ram_in_8192.v) + (32*ram_in_32768.v)+ (64*ram_in_49152.v) + (128*ay_chip_present.v);
		bits_estado2=(nmi_generator_active.v) | (hsync_generator_active.v<<1) | (rainbow_enabled.v<<2) | (wrx_present.v<<3) | (ram_in_8192.v<<4) | (ram_in_32768.v<<5) | (ram_in_49152.v<<6) | (ay_chip_present.v<<7);


		//printf ("bits_estado2: %d pruebas: %d\n",bits_estado2,pruebas);

	        header[72]=bits_estado2;

		//Generar bits_estado3
		z80_byte bits_estado3=0;


		//keymap de momento solo 0 o 1
		int bit_estado_keymap=(z88_cpc_keymap_type == 1 ? 128 : 0);

		bits_estado3=(video_zx8081_estabilizador_imagen.v) | (zx8081_vsync_sound.v <<2) | (chardetect_printchar_enabled.v <<3) | (stdout_simpletext_automatic_redraw.v << 4) | (chardetect_second_trap_sum32.v << 5) | (kempston_mouse_emulation.v << 6) | bit_estado_keymap;

		header[73]=bits_estado3;

		//valor con signo. TODO. no usado
		//char t=offset_zx8081_t_estados;
		//header[74]=t;

		//fecha grabacion
		time_t tiempo = time(NULL);
		struct tm tm = *localtime(&tiempo);

		//printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		header[75]=tm.tm_mday;
		header[76]=tm.tm_mon+1;

		z80_int year;
		year=tm.tm_year + 1900;

		header[77]=value_16_to_8l(year);
		header[78]=value_16_to_8h(year);

		header[79]=tm.tm_hour;
		header[80]=tm.tm_min;


		//zx80/81 ram
		z80_byte ram_zx8081=(ramtop_zx8081-16383)/1024;
		header[81]=ram_zx8081;

		//inves low ram poke desde menu
		header[82]=last_inves_low_ram_poke_menu;

		//inves ula delay
		//header[83]=inves_ula_delay_factor;


		//Second y Third trap char
		header[84]=value_16_to_8l(chardetect_second_trap_char_dir);
		header[85]=value_16_to_8h(chardetect_second_trap_char_dir);
		header[86]=value_16_to_8l(chardetect_third_trap_char_dir);
		header[87]=value_16_to_8h(chardetect_third_trap_char_dir);

		//stdout line width
		header[88]=chardetect_line_width;

		//stdout char filter
		header[89]=chardetect_char_filter;

		//joystick type emulation
		header[90]=joystick_emulation;

		//gunstick type emulation
		header[91]=gunstick_emulation;

	}


        //Valores para zx version>=5
        if (snap_zx_version_save>=5) {
		header[92]=(z88_internal_rom_size+1)/16384;
		header[93]=(z88_internal_ram_size+1)/16384;


		//Si hay algun slot tipo hibrido, cambiar numeracion
		z80_byte mem_types[3];
		mem_types[0]=z88_memory_slots[1].type;
		mem_types[1]=z88_memory_slots[2].type;
		mem_types[2]=z88_memory_slots[3].type;

		int i;
		for (i=0;i<3;i++) {
			if (mem_types[i]==4) {
				mem_types[i]=1;
			}
		}

		header[94]=(mem_types[0]) | (mem_types[1]<<2) | (mem_types[2]<<4);
		//header[94]=(z88_memory_slots[1].type) | (z88_memory_slots[2].type<<2) | (z88_memory_slots[3].type<<4);



		if (z88_memory_slots[1].size) header[95]=(z88_memory_slots[1].size+1)/16384;
		else header[95]=0;

		if (z88_memory_slots[2].size) header[96]=(z88_memory_slots[2].size+1)/16384;
		else header[96]=0;

		if (z88_memory_slots[3].size) header[97]=(z88_memory_slots[3].size+1)/16384;
		else header[97]=0;

		//Grabar registros del Blink
		//z80_int blink_pixel_base[4];
		header[98]=value_16_to_8l(blink_pixel_base[0]);
		header[99]=value_16_to_8h(blink_pixel_base[0]);

		header[100]=value_16_to_8l(blink_pixel_base[1]);
		header[101]=value_16_to_8h(blink_pixel_base[1]);

		header[102]=value_16_to_8l(blink_pixel_base[2]);
		header[103]=value_16_to_8h(blink_pixel_base[2]);

		header[104]=value_16_to_8l(blink_pixel_base[3]);
		header[105]=value_16_to_8h(blink_pixel_base[3]);

		//z80_int blink_sbr;
                header[106]=value_16_to_8l(blink_sbr);
                header[107]=value_16_to_8h(blink_sbr);

		header[108]=blink_com;
		header[109]=blink_int;

		header[110]=blink_sta;
		header[111]=blink_epr;

		header[112]=blink_tmk;
		header[113]=blink_tsta;

		header[114]=blink_mapped_memory_banks[0];
		header[115]=blink_mapped_memory_banks[1];
		header[116]=blink_mapped_memory_banks[2];
		header[117]=blink_mapped_memory_banks[3];

		header[118]=blink_tim[0];
		header[119]=blink_tim[1];
		header[120]=blink_tim[2];
		header[121]=blink_tim[3];
		header[122]=blink_tim[4];

		header[123]=blink_rxd;
		header[124]=blink_rxe;

		header[125]=blink_rxc;
		header[126]=blink_txd;

		header[127]=blink_txc;
		header[128]=blink_umk;

		header[129]=blink_uit;
	}

	 //Valores para zx version>=6
        if (snap_zx_version_save>=6) {

                //ace ram
		z80_byte ram_ace=((ramtop_ace-16383)/1024)+3;
                header[130]=ram_ace;


		header[131]=timex_port_f4;
		header[132]=timex_port_ff;

                //Generar bits_estado4
                z80_byte bits_estado4=0;

		bits_estado4=(ulaplus_presente.v) | (ulaplus_enabled.v <<1) | (timex_video_emulation.v<<2);


		header[133]=bits_estado4;

		header[134]=ulaplus_last_send_BF3B;
		header[135]=ulaplus_last_send_FF3B;
		header[136]=ulaplus_mode;

		//Guardar 64 bytes de paleta ulaplus
		int i;
		for (i=0;i<64;i++) header[137+i]=ulaplus_palette_table[i];
	}


        //Save header File
        ptr_zxfile=fopen(filename,"wb");
        if (!ptr_zxfile) {
		debug_printf (VERBOSE_ERR,"Error writing snapshot file %s",filename);
		return;
	}

	fwrite(header, 1, ZX_HEADER_SIZE, ptr_zxfile);


	//Escritura de datos
	if (MACHINE_IS_SPECTRUM_16_48) {
		//Escritura de 48k
		debug_printf (VERBOSE_INFO,"Saving 48kb block");
		save_zx_snapshot_bytes_48k_128k(ptr_zxfile,16384,0,0);
		}

	else if (MACHINE_IS_ZX8081) {
                //Escritura de zx8081
		int direccion=16384;
		if (ram_in_8192.v) direccion=8192;

                debug_printf (VERBOSE_INFO,"Saving %d kb block beginning from %d",(65536-direccion)/1024,direccion);
                save_zx_snapshot_bytes_48k_128k(ptr_zxfile,direccion,0,0);
	}

        else if (MACHINE_IS_ACE) {
                //Escritura de ace
                int direccion=8192;

                debug_printf (VERBOSE_INFO,"Saving %d kb block beginning from %d",(65536-direccion)/1024,direccion);
                save_zx_snapshot_bytes_48k_128k(ptr_zxfile,direccion,0,0);
        }

	else if (MACHINE_IS_Z88) {

		/* Formato datos grabacion Z88
		Se graban paginas (bancos de z88) de 16kb. Cada banco esta comprimido con mismo formato general que zx
		Al principio de cada bloque hay una cabecera:
		BYTE numero de banco
		16 BIT longitud de los datos comprimidos

		Al cargar se carga segun el numero de banco leido en la cabecera
		Se graban en orden: rom interna, ram interna, slot 1, slot 2, slot 3
		Aunque en principio se podrian incluso grabar desordenados, pues en la carga
		siempre lo hace leyendo el numero de banco que viene en la cabecera

		*/

		z80_byte *buffer_z88_save=malloc(20000);
		if (buffer_z88_save==NULL) {
			debug_printf (VERBOSE_ERR,"Error allocating memory buffer on save");
			return;
		}

		//Grabar ROM interna
		//calculo numero de bancos
		z80_byte bancos_total=(z88_internal_rom_size+1)/16384;

		int i;
		for (i=0;i<bancos_total;i++) {
			//save_zx_snapshot_bytes_z88(ptr_zxfile,i);
			//Cabecera bloque 16kb: bank, longitud
			z80_byte bank=i;
			save_zx_z88_block_16kb(ptr_zxfile,buffer_z88_save,bank);
		}

		//Grabar RAM interna
                //calculo numero de bancos
                bancos_total=(z88_internal_ram_size+1)/16384;

                for (i=0;i<bancos_total;i++) {
                        //save_zx_snapshot_bytes_z88(ptr_zxfile,0x20+i);
			z80_byte bank=0x20+i;
			save_zx_z88_block_16kb(ptr_zxfile,buffer_z88_save,bank);

                }


		//Los 3 slots y siempre que que size!=0 y no haya eprom ni flash en slot 3
		int slot;
		for (slot=1;slot<=3;slot++) {
			if (z88_memory_slots[slot].size!=0) {
				//Hay algo. Si es slot 3, que no sea eprom ni flash
				if (slot==3 && (z88_memory_slots[slot].type==2 || z88_memory_slots[slot].type==3 || z88_memory_slots[slot].type==4) ) {
					debug_printf (VERBOSE_DEBUG,"Do not save eprom/flash on slot 3");
				}

				else {

					//calculo numero de bancos
					bancos_total=(z88_memory_slots[slot].size+1)/16384;
					for (i=0;i<bancos_total;i++) {
						//save_zx_snapshot_bytes_z88(ptr_zxfile,0x40*slot+i);
						z80_byte bank=0x40*slot+i;
						save_zx_z88_block_16kb(ptr_zxfile,buffer_z88_save,bank);
					}
				}
			}
		}

		free(buffer_z88_save);
	}

	else if (MACHINE_IS_CPC_464) {
		fwrite(cpc_gate_registers,1,4,ptr_zxfile);
		fwrite(cpc_palette_table, 1, 16, ptr_zxfile);
		fwrite(cpc_ppi_ports, 1, 4, ptr_zxfile);
		fwrite(cpc_crtc_registers, 1, 32, ptr_zxfile);
		fwrite(&cpc_border_color,1,1,ptr_zxfile);
		fwrite(&cpc_crtc_last_selected_register,1,1,ptr_zxfile);

                z80_byte *buffer_cpc_save=malloc(20000);
                if (buffer_cpc_save==NULL) {
                        debug_printf (VERBOSE_ERR,"Error allocating memory buffer on save");
                        return;
                }


		//Grabar 64 KB
		int i;
                for (i=0;i<4;i++) {
			save_zx_generic_block_16kb(ptr_zxfile,buffer_cpc_save,cpc_ram_mem_table[i],i);
                }


	}

	else if (MACHINE_IS_TIMEX_TS2068) {
		z80_byte *buffer_timex_save=malloc(20000);
                if (buffer_timex_save==NULL) {
                        debug_printf (VERBOSE_ERR,"Error allocating memory buffer on save");
                        return;
                }


                //Grabar 48 KB
                int i;
                for (i=0;i<3;i++) {
                        save_zx_generic_block_16kb(ptr_zxfile,buffer_timex_save,timex_home_ram_mem_table[i],i);
                }


        }

	else if (MACHINE_IS_CHLOE_140SE) {
		z80_byte *buffer_chloe_save=malloc(20000);
                if (buffer_chloe_save==NULL) {
                        debug_printf (VERBOSE_ERR,"Error allocating memory buffer on save");
                        return;
                }


                //Grabar 128 KB
                int i;
                for (i=0;i<8;i++) {
                        save_zx_generic_block_16kb(ptr_zxfile,buffer_chloe_save,chloe_home_ram_mem_table[i],i);
                }
	}

        else if (MACHINE_IS_CHLOE_280SE) {
                z80_byte *buffer_chloe_save=malloc(20000);
                if (buffer_chloe_save==NULL) {
                        debug_printf (VERBOSE_ERR,"Error allocating memory buffer on save");
                        return;
                }

                //Grabar 64 KB EX RAM. Salto de bloque de 2 en dos (bloques internos de 8kb pero grabacion de 16kb)
                int i;
                for (i=0;i<8;i+=2) {
                        save_zx_generic_block_16kb(ptr_zxfile,buffer_chloe_save,chloe_ex_ram_mem_table[i],i);
                }

                //Grabar 64 KB Dock RAM. Salto de bloque de 2 en dos (bloques internos de 8kb pero grabacion de 16kb)
                for (i=0;i<8;i+=2) {
                        save_zx_generic_block_16kb(ptr_zxfile,buffer_chloe_save,chloe_dock_ram_mem_table[i],i+8);
                }

                //Grabar 128 KB Home RAM
                for (i=0;i<8;i++) {
                        save_zx_generic_block_16kb(ptr_zxfile,buffer_chloe_save,chloe_home_ram_mem_table[i],i+16);
                }
        }

	else if (MACHINE_IS_PRISM) {
		//Grabar bytes estado
		fwrite(&prism_rom_page,1,1,ptr_zxfile);
		fwrite(&prism_ae3b_registers[0],1,1,ptr_zxfile);
		fwrite(&prism_ula2_palette_control_colour,1,1,ptr_zxfile);
		fwrite(&prism_ula2_palette_control_index,1,1,ptr_zxfile);
		fwrite(prism_ula2_palette_control_rgb,1,3,ptr_zxfile);
		fwrite(prism_ula2_registers,1,16,ptr_zxfile);

		//Grabar paleta 2. 256 valores de 16 bits. Grabar en bucle para asegurarse que es little endian
		int i;
		z80_int color;
		z80_byte hi,lo;
		for (i=0;i<256;i++) {
			color=prism_palette_two[i];
			lo=value_16_to_8l(color);
			hi=value_16_to_8h(color);
			fwrite(&lo,1,1,ptr_zxfile);
			fwrite(&hi,1,1,ptr_zxfile);
		}

		z80_byte *buffer_prism_save=malloc(20000);
                if (buffer_prism_save==NULL) {
                        debug_printf (VERBOSE_ERR,"Error allocating memory buffer on save");
                        return;
                }

		//Grabar 512 KB RAM. Salto de bloque de 2 en dos (bloques internos de 8kb pero grabacion de 16kb)
                for (i=0;i<64;i+=2) {
                        save_zx_generic_block_16kb(ptr_zxfile,buffer_prism_save,prism_ram_mem_table[i],i);
                }

		//Grabar 32 kb de VRAM. Salto de bloque de 2 en dos (bloques internos de 8kb pero grabacion de 16kb)
		//El indice que se guarda en bloque es 240+numero vram
                for (i=0;i<4;i+=2) {
                        save_zx_generic_block_16kb(ptr_zxfile,buffer_prism_save,prism_vram_mem_table[i],240+i);
                }


	}



        else if (MACHINE_IS_ZXUNO) {
		//Guardar bytes estado
		fwrite(&last_port_FC3B,1,1,ptr_zxfile);
		fwrite(zxuno_ports,1,256,ptr_zxfile);
		fwrite(zxuno_spi_bus,1,8,ptr_zxfile);
		fwrite(&zxuno_spi_bus_index,1,1,ptr_zxfile);
		fwrite(&next_spi_read_byte,1,1,ptr_zxfile);
		fwrite(&zxuno_spi_status_register,1,1,ptr_zxfile);

		//Guardar 2 valores de 24 bits
		z80_byte buffer_spi_address[6];
		//last_spi_write_address
		buffer_spi_address[0]=last_spi_write_address & 0xFF;
		buffer_spi_address[1]=(last_spi_write_address>>8) & 0xFF;
		buffer_spi_address[2]=(last_spi_write_address>>16) & 0xFF;

		//last_spi_read_address
		buffer_spi_address[3]=last_spi_read_address & 0xFF;
		buffer_spi_address[4]=(last_spi_read_address>>8) & 0xFF;
		buffer_spi_address[5]=(last_spi_read_address>>16) & 0xFF;
		fwrite(buffer_spi_address,1,6,ptr_zxfile);


                z80_byte *buffer_zxuno_save=malloc(20000);
                if (buffer_zxuno_save==NULL) {
                        debug_printf (VERBOSE_ERR,"Error allocating memory buffer on save");
                        return;
                }


                //Grabar 512 KB
		//z80_byte *zxuno_sram_mem_table[ZXUNO_SRAM_PAGES];
                int i;
                for (i=0;i<ZXUNO_SRAM_PAGES;i++) {
                        save_zx_generic_block_16kb(ptr_zxfile,buffer_zxuno_save,zxuno_sram_mem_table_new[i],i);
                }


        }



	else if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
		//Escritura de 128k
		debug_printf (VERBOSE_INFO,"Saving 64kb block for RAMS 0-3");
		save_zx_snapshot_bytes_48k_128k(ptr_zxfile,0,1,0);
		debug_printf (VERBOSE_INFO,"Saving 64kb block for RAMS 4-7");
		save_zx_snapshot_bytes_48k_128k(ptr_zxfile,0,1,4);
	}



	fclose(ptr_zxfile);


}


//Grabar Snapshot SP
void save_sp_snapshot(char *filename)
{


        z80_byte header[SP_HEADER_SIZE];


	if (!(MACHINE_IS_SPECTRUM_16_48)) {
		debug_printf(VERBOSE_ERR,"SP snapshots are only for Spectrum 48k models");
		//Aqui se soporta 16kb,48kb, inves y modelos tk
		return;
	}


       FILE *ptr_spfile;


        //Create header

        header[0]='S';
        header[1]='P';



	//Se guardan en SP como 48kb en general. no especificar si 16kb, inves, 48k o tk....
        save_zxsp_snapshot_registers(header);


        //Save header File
        ptr_spfile=fopen(filename,"wb");
        if (!ptr_spfile) {
                debug_printf (VERBOSE_ERR,"Error writing snapshot file %s",filename);
                return;
        }

        fwrite(header, 1, SP_HEADER_SIZE, ptr_spfile);

        //Escritura de datos
	debug_printf (VERBOSE_INFO,"Saving 48kb block");
	fwrite(&memoria_spectrum[16384],1,49152,ptr_spfile);

        fclose(ptr_spfile);

}


//Comun para grabar registros en formato SNA 48k y 128k
void save_sna_snapshot_registers(z80_byte *header)
{

/*
   Offset   Size   Description
   ------------------------------------------------------------------------
   0        1      byte   I
   1        8      word   HL',DE',BC',AF'
   9        10     word   HL,DE,BC,IY,IX
   19       1      byte   Interrupt (bit 2 contains IFF2, 1=EI/0=DI)
   20       1      byte   R
   21       4      words  AF,SP
   25       1      byte   IntMode (0=IM0/1=IM1/2=IM2)
   26       1      byte   BorderColor (0..7, not used by Spectrum 1.7)
   27       49152  bytes  RAM dump 16384..65535
   ------------------------------------------------------------------------
   Total: 49179 bytes
*/

	header[0]=reg_i;
	header[1]=reg_l_shadow;
	header[2]=reg_h_shadow;	
	header[3]=reg_e_shadow;
	header[4]=reg_d_shadow;		
	header[5]=reg_c_shadow;
	header[6]=reg_b_shadow;
	header[7]=get_flags_shadow();
	header[8]=reg_a_shadow;

	header[9]=reg_l;
	header[10]=reg_h;
	header[11]=reg_e;
	header[12]=reg_d;		
	header[13]=reg_c;
	header[14]=reg_b;
	header[15]=value_16_to_8l(reg_iy);
	header[16]=value_16_to_8h(reg_iy);
	header[17]=value_16_to_8l(reg_ix);
	header[18]=value_16_to_8h(reg_ix);

	//   19       1      byte   Interrupt (bit 2 contains IFF2, 1=EI/0=DI)
	z80_byte bits_estado=(iff1.v) | (iff2.v ? 4 : 0);
	header[19]=bits_estado;

	header[20]=(reg_r&127) | (reg_r_bit7&128);

	header[21]=get_flags();
	header[22]=reg_a;

	header[23]=value_16_to_8l(reg_sp);
	header[24]=value_16_to_8h(reg_sp);

	header[25]=im_mode;
	header[26]=out_254  & 7;


}

void save_sna_snapshot_bytes_128k(FILE *ptr_sna_file,z80_byte pagina_entra)
{

	debug_printf (VERBOSE_INFO,"Writing 16Kb block from RAM page %d",pagina_entra);

	z80_byte valor_puerto_32765=(puerto_32765&(255-7));

	//Esto es una solucion un tanto fea pero funciona,
	//asi no tengo que andar mirando si es maquina 128k, plus2 o plus3, o zxuno, etc

	out_port_spectrum_no_time(32765,valor_puerto_32765 | pagina_entra);

	z80_int direccion_origen=49152;
	int l;
	z80_byte byte_leido;
	for (l=0;l<16384;l++) {
		byte_leido=peek_byte_no_time(direccion_origen++);
		fwrite(&byte_leido, 1, 1, ptr_sna_file);
	}
}


//Grabar Snapshot SNA
void save_sna_snapshot(char *filename)
{
/*
   Offset   Size   Description
   ------------------------------------------------------------------------
   0        1      byte   I
   1        8      word   HL',DE',BC',AF'
   9        10     word   HL,DE,BC,IY,IX
   19       1      byte   Interrupt (bit 2 contains IFF2, 1=EI/0=DI)
   20       1      byte   R
   21       4      words  AF,SP
   25       1      byte   IntMode (0=IM0/1=IM1/2=IM2)
   26       1      byte   BorderColor (0..7, not used by Spectrum 1.7)
   27       49152  bytes  RAM dump 16384..65535
   ------------------------------------------------------------------------
   Total: 49179 bytes
*/


    z80_byte header[SNA_48K_HEADER_SIZE];

	if (!MACHINE_IS_SPECTRUM) {
		debug_printf(VERBOSE_ERR,"SNA snapshots are only allowed on Spectrum machines");
		//Aqui se soporta 16kb,48kb, inves y modelos tk
		return;
	}


	FILE *ptr_spfile;


	if (MACHINE_IS_SPECTRUM_16_48) {
		//Meter PC en stack
		//Indicar como si fuera non_maskable_interrupt, pues originalmente los .sna venian en no-se-que interfaz
		//y se generaba snapshot al pulsar boton nmi
		push_valor(reg_pc,PUSH_VALUE_TYPE_NON_MASKABLE_INTERRUPT);
	}

    save_sna_snapshot_registers(header);


	//Save header 
	ptr_spfile=fopen(filename,"wb");
	if (!ptr_spfile) {
		debug_printf (VERBOSE_ERR,"Error writing snapshot file %s",filename);
		return;
	}

	fwrite(header, 1, SNA_48K_HEADER_SIZE, ptr_spfile);

    //Escritura de datos
	if (MACHINE_IS_SPECTRUM_16_48) {
		debug_printf (VERBOSE_INFO,"Saving 48kb block");
		fwrite(&memoria_spectrum[16384],1,49152,ptr_spfile);
	}


	else {
		//En 128k guardar mas cosas
		/*
		The 128K version of the .sna format is the same as above, with extensions to include the extra memory banks of the 128K/+2 machines, and fixes the problem with the PC being pushed onto the stack - now it is located in an extra variable in the file (and is not pushed onto the stack at all). The first 49179 bytes of the snapshot are otherwise exactly as described above, so the full description is:

		Offset   Size   Description
		------------------------------------------------------------------------
		0        27     bytes  SNA header (see above)
		27       16Kb   bytes  RAM bank 5 \
		16411    16Kb   bytes  RAM bank 2  } - as standard 48Kb SNA file
		32795    16Kb   bytes  RAM bank n / (currently paged bank)
		49179    2      word   PC
		49181    1      byte   port 0x7ffd setting
		49182    1      byte   TR-DOS rom paged (1) or not (0)
		49183    16Kb   bytes  remaining RAM banks in ascending order
		...
		------------------------------------------------------------------------
		Total: 131103 or 147487 bytes

		The third RAM bank saved is always the one currently paged, even if this is page 5 or 2 - in this case, the bank is actually included twice. The remaining RAM banks are saved in ascending order - e.g. if RAM bank 4 is paged in, the snapshot is made up of banks 5, 2 and 4 to start with, and banks 0, 1, 3, 6 and 7 afterwards. If RAM bank 5 is paged in, the snapshot is made up of banks 5, 2 and 5 again, followed by banks 0, 1, 3, 4, 6 and 7.
		*/


		//Fuse por ejemplo carga snapshots de 128kb como Pentagon 128k.... a saber...
		z80_byte puerto_32765_antes=puerto_32765;


		//Preparamos antes la cabecera pues hay que meter el puerto_32765 original
		/*
			49179    2      word   PC
			49181    1      byte   port 0x7ffd setting
			49182    1      byte   TR-DOS rom paged (1) or not (0)
			49183    16Kb   bytes  remaining RAM banks in ascending order
		*/		
		z80_byte header128[SNA_128K_HEADER_SIZE];
		header128[0]=value_16_to_8l(reg_pc);
		header128[1]=value_16_to_8h(reg_pc);
		header128[2]=puerto_32765_antes;
		header128[3]=0;		
		//TODO: usar el byte de TR-DOS rom paged (1) or not (0)

		//Suponemos primero pagina 0, para habilitar paginacion, por si estuviera deshabilitada
		puerto_32765=0;


		//grabar datos
		//grabar ram 5. 
		save_sna_snapshot_bytes_128k(ptr_spfile,5);

		//grabar ram 2. 
		save_sna_snapshot_bytes_128k(ptr_spfile,2);	

		//grabar ram N. luego la excluimos de la lista restante
		z80_byte ram_paginada=puerto_32765_antes & 7;
		save_sna_snapshot_bytes_128k(ptr_spfile,ram_paginada);


		fwrite(header128, 1, SNA_128K_HEADER_SIZE, ptr_spfile);					
				

		//Grabar RAMS 0,1,3,4,6,7. Si ram_paged es alguna de esas, no grabarla
		z80_byte paginas[6]={0,1,3,4,6,7};
		int i;
		for (i=0;i<6;i++) {
			z80_byte pagina_entra=paginas[i];
			if (pagina_entra!=ram_paginada) {
				save_sna_snapshot_bytes_128k(ptr_spfile,pagina_entra);
			}
		}

		//dejamos las paginas como estaban. Esto es una solucion un tanto fea pero funciona,
		//asi no tengo que andar mirando si es maquina 128k, plus2 o plus3, o zxuno, etc
		//printf ("puerto antes: %d\n",puerto_32765_antes);
		out_port_spectrum_no_time(32765,puerto_32765_antes);

	}


    fclose(ptr_spfile);

	if (MACHINE_IS_SPECTRUM_16_48) {
		//Sacar PC del stack
		reg_pc=pop_valor();
	}

	//Aviso de posible grabacion con error
	if (!MACHINE_IS_SPECTRUM_16_48 && !MACHINE_IS_SPECTRUM_128_P2) {
		menu_warn_message("SNA snapshot only work well on 48k and 128k/+2 models");
	}

}

void save_ace_snapshot_store_header(void)
{
	//Meter cabecera archivo .ace en direcciones de memoria ram del ace 2000H-21FFH

	//Primero meter a cero todas esas direcciones

	z80_int puntero;

	for (puntero=0x2000;puntero<0x2200;puntero++) memoria_spectrum[puntero]=0;

	/* Valores cabecera
Addr:   Defaults                Description

2000    01, 80, 00, 00      ?

2080    00, 80, 00, 00       Ramtop 4000 (3K), 8000(19K), C000(35K)
2084    00, 00, 00, 00       Debugger Data Address
2088    00, 00, 00, 00       Debugger Breakpoint Address
208C    03, 00, 00, 00       Frame Skip Rate (3)
2090    03, 00, 00, 00       Frames per TV Tick (3)
2094    FD, FD, 00, 00       ?
2098    XX, XX, XX, XX       Time emulator is running probably in milliseconds
209C    00, 00, 00, 00       Emulator Colours 0(white on Black), 1(green on Black),
                                             2(purple on Black),3(Black on White)
	*/


	memoria_spectrum[0x2000]=0x01;
	memoria_spectrum[0x2001]=0x80;

	//Ramtop
	//4000 si ramtop=3FFH
	//8000 si ramtop=7FFH
	//C000 si ramtop=BFFH

	z80_byte ramtop;
	ramtop=value_16_to_8h(ramtop_ace+1);


	memoria_spectrum[0x2081]=ramtop;

	//frameskip lo dejamos a 0
	//frames per tv tick lo ponemos a 1
	memoria_spectrum[0x2090]=0x1;

	memoria_spectrum[0x2094]=0xFD;
	memoria_spectrum[0x2095]=0xFD;


	//Escribir registros
/*
Addr:	last state             Registers

2100	50, 04, 00, 00		AF
	00, 00, 00, 00		BC
	E2, 26, 00, 00		DE
	28, 3C, 00, 00		HL
	00, 3C, 00, 00		IX
	C8, 04, 00, 00		IY
	FE, 7F, 00, 00		SP
	9D, 05, 00, 00		PC
	40, 20, 00, 00		AF'
	00, 01, 00, 00		BC'
	60, 00, 00, 00		DE'
	80, 26, 00, 00		HL'
	01, 00, 00, 00		IM
	01, 00, 00, 00		IFF1
	01, 00, 00, 00		IFF2
	00, 00, 00, 00		I
	11, 00, 00, 00		R
	80, 00, 00, 00		?
*/

	z80_int reg=0x2100;

        memoria_spectrum[reg++]=get_flags();
        memoria_spectrum[reg++]=reg_a;
	reg +=2;

        memoria_spectrum[reg++]=reg_c;
        memoria_spectrum[reg++]=reg_b;
	reg +=2;

        memoria_spectrum[reg++]=reg_e;
        memoria_spectrum[reg++]=reg_d;
	reg +=2;

        memoria_spectrum[reg++]=reg_l;
        memoria_spectrum[reg++]=reg_h;
	reg +=2;

        memoria_spectrum[reg++]=value_16_to_8l(reg_ix);
        memoria_spectrum[reg++]=value_16_to_8h(reg_ix);
	reg +=2;

        memoria_spectrum[reg++]=value_16_to_8l(reg_iy);
        memoria_spectrum[reg++]=value_16_to_8h(reg_iy);
	reg +=2;

        memoria_spectrum[reg++]=value_16_to_8l(reg_sp);
        memoria_spectrum[reg++]=value_16_to_8h(reg_sp);
	reg +=2;

        memoria_spectrum[reg++]=value_16_to_8l(reg_pc);
        memoria_spectrum[reg++]=value_16_to_8h(reg_pc);

        memoria_spectrum[reg++]=get_flags_shadow();
        memoria_spectrum[reg++]=reg_a_shadow;
        reg +=2;

        memoria_spectrum[reg++]=reg_c_shadow;
        memoria_spectrum[reg++]=reg_b_shadow;
        reg +=2;

        memoria_spectrum[reg++]=reg_e_shadow;
        memoria_spectrum[reg++]=reg_d_shadow;
        reg +=2;

        memoria_spectrum[reg++]=reg_l_shadow;
        memoria_spectrum[reg++]=reg_h_shadow;
        reg +=2;

/*
        01, 00, 00, 00          IM
        01, 00, 00, 00          IFF1
        01, 00, 00, 00          IFF2
        00, 00, 00, 00          I
        11, 00, 00, 00          R
        80, 00, 00, 00          ?
*/

	memoria_spectrum[reg]=(im_mode==2 ? 2 : 0);
	reg+=4;

	memoria_spectrum[reg]=iff1.v;
	reg +=4;

	memoria_spectrum[reg]=iff2.v;
	reg +=4;

	memoria_spectrum[reg]=reg_i;
	reg +=4;

	memoria_spectrum[reg]=(reg_r&127) | (reg_r_bit7&128);
	reg +=4;


	//Y el 80 del final que no sabemos que es
	memoria_spectrum[reg]=0x80;




}

void save_ace_snapshot_repeticion (FILE *file,z80_byte byte_repetir, z80_byte veces)
{

	debug_printf (VERBOSE_DEBUG,"Writing ED repetition. Byte=0x%02X Times=%d",byte_repetir,veces);

	if (veces>=3 || byte_repetir==0xED) {
		//ED xx yy    repeat byte yy, xx times.
		z80_byte buffer[3];

		buffer[0]=0xED;
		buffer[1]=veces;
		buffer[2]=byte_repetir;

		fwrite (buffer,1,3,file);

	}

	else {
		while (veces>0) {
			fwrite (&byte_repetir,1,1,file);
			veces--;
		}
	}
}


void old_save_ace_snapshot(char *filename)
{
	if (!(MACHINE_IS_ACE)) {
		debug_printf(VERBOSE_ERR,"ACE snapshots are only for Jupiter ace");
		return;
	}


	//printf ("desactivado save header\n");
	save_ace_snapshot_store_header();

	FILE *ptr_acefile;

	ptr_acefile=fopen(filename,"wb");
        if (!ptr_acefile) {
                debug_printf (VERBOSE_ERR,"Error writing snapshot file %s",filename);
                return;
        }

	//Bucle gestionando repeticiones
	z80_int puntero=0x2000;
	debug_printf (VERBOSE_INFO,"Saving %dKb block",(ramtop_ace+1-puntero)/1024);

	z80_byte byte_leido,byte_anterior;

	//byte_anterior no puede ser el mismo que el inicial
	byte_anterior=memoria_spectrum[puntero++];

	int repeticiones;

	int repeticion_con_final=0;


	while (puntero<=ramtop_ace) {
		repeticiones=0;
		byte_leido=memoria_spectrum[puntero++];
		debug_printf (VERBOSE_PARANOID,"Reading byte at 0x%04X = 0x%02X",puntero-1,byte_leido);

		//Si se repite
		if (byte_leido==byte_anterior) {
			repeticiones=2;
			int fin_repeticion=0;
			do {
				byte_leido=memoria_spectrum[puntero++];
				debug_printf (VERBOSE_PARANOID,"Reading byte at 0x%04X = 0x%02X",puntero-1,byte_leido);
				//si es el mismo que el anterior,seguir como maximo 240 veces
				if (byte_leido==byte_anterior) {
					repeticiones++;
					if (puntero>ramtop_ace) {
						fin_repeticion=1;
						repeticion_con_final=1;
					}

					else if (repeticiones==240) {
						//Metemos repeticion y seguimos dentro del bucle
						save_ace_snapshot_repeticion(ptr_acefile,byte_anterior,repeticiones);
						repeticiones=0;
					}
				}

				else fin_repeticion=1;

			} while (!fin_repeticion);

			if (repeticiones>=3) {
				save_ace_snapshot_repeticion(ptr_acefile,byte_anterior,repeticiones);
			}

			else {
				//Repeticiones < 3

				//Escribir bytes tal cual. Si son ED, generar bytes repeticion
				if (byte_anterior==0xED) {
					save_ace_snapshot_repeticion(ptr_acefile,byte_anterior,repeticiones);
				}
				else {
					while (repeticiones>0) {
						fwrite (&byte_anterior,1,1,ptr_acefile);
						repeticiones--;
					}
				}
			}

		}


		else {
			//No hay repeticion

			//Si es ED, repetir
			if (byte_anterior==0xED) save_ace_snapshot_repeticion(ptr_acefile,byte_anterior,1);

			else {
				//Escribirlo tal cual
				debug_printf (VERBOSE_PARANOID,"Writing alone byte 0x%02X",byte_anterior);
				fwrite (&byte_anterior,1,1,ptr_acefile);
			}
		}

		byte_anterior=byte_leido;
	}

	//Escribir el ultimo byte
	//Aqui se puede haber acabado con repeticion (por ejemplo, ceros hasta final de ram) o con bytes separados hasta final de ram,
	//O con repeticiones y el ultimo byte de la ram diferente
	if (repeticiones==0) fwrite (&byte_anterior,1,1,ptr_acefile);
	else {
		if (repeticion_con_final==0) fwrite (&byte_anterior,1,1,ptr_acefile);
	}

	z80_byte buffer_fin[2];
	buffer_fin[0]=0xED;
	buffer_fin[1]=0;


	//Finalizar con ED00
	fwrite (&buffer_fin,1,2,ptr_acefile);

        fclose(ptr_acefile);
}


void save_ace_snapshot(char *filename)
{
        if (!(MACHINE_IS_ACE)) {
                debug_printf(VERBOSE_ERR,"ACE snapshots are only for Jupiter ace");
                return;
        }


        //printf ("desactivado save header\n");
        save_ace_snapshot_store_header();

        FILE *ptr_acefile;

        ptr_acefile=fopen(filename,"wb");
        if (!ptr_acefile) {
                debug_printf (VERBOSE_ERR,"Error writing snapshot file %s",filename);
                return;
        }

        //Bucle gestionando repeticiones
	//Puntero a memoria usamos entero normal; si usamos z80_int y la ramtop del ace esta en 65535, dara la vuelta al contador
        int puntero=0x2000;


        debug_printf (VERBOSE_INFO,"Saving %dKb block",(ramtop_ace+1-puntero)/1024);

        z80_byte byte_leido,byte_anterior;

        //byte_anterior no puede ser el mismo que el inicial
        byte_anterior=memoria_spectrum[puntero++];

        int repeticiones=1;

        while (puntero<=ramtop_ace) {
                byte_leido=memoria_spectrum[puntero++];
                debug_printf (VERBOSE_PARANOID,"Reading byte at 0x%04X = 0x%02X",puntero-1,byte_leido);

		if (byte_anterior==byte_leido) {
			repeticiones++;
			if (repeticiones==240) {
				//Metemos repeticion y seguimos dentro del bucle
				save_ace_snapshot_repeticion(ptr_acefile,byte_anterior,repeticiones);
				repeticiones=0;
			}
		}

		else {
			//Byte diferente
			save_ace_snapshot_repeticion(ptr_acefile,byte_anterior,repeticiones);
			repeticiones=1;
		}

		byte_anterior=byte_leido;
	}

	//Ultima escritura
        //Escribir el ultimo byte
        //Aqui se puede haber acabado con repeticion (por ejemplo, ceros hasta final de ram) o con bytes separados hasta final de ram,
        //O con repeticiones y el ultimo byte de la ram diferente
        if (repeticiones==1) fwrite (&byte_leido,1,1,ptr_acefile);
        else {
		save_ace_snapshot_repeticion(ptr_acefile,byte_anterior,repeticiones);
        }

        z80_byte buffer_fin[2];
        buffer_fin[0]=0xED;
        buffer_fin[1]=0;


        //Finalizar con ED00
        fwrite (&buffer_fin,1,2,ptr_acefile);

        fclose(ptr_acefile);

}


//Funcion de grabacion de snapshot, lo normal es que se llame desde menu
void snapshot_save(char *filename)
{

	//if (strstr(filename,".zx")!=NULL  || strstr(filename,".ZX")!=NULL) {
	if (!util_compare_file_extension(filename,"zx") ) {
		debug_printf(VERBOSE_INFO,"Saving ZX snapshot %s",filename);
		save_zx_snapshot(filename);
	}

	else if (!util_compare_file_extension(filename,"sp") ) {
                debug_printf(VERBOSE_INFO,"Saving SP snapshot %s",filename);
                save_sp_snapshot(filename);
        }

	else if (!util_compare_file_extension(filename,"sna") ) {
                debug_printf(VERBOSE_INFO,"Saving SNA snapshot %s",filename);
                save_sna_snapshot(filename);
        }		

        else if (!util_compare_file_extension(filename,"zsf") ) {
                      debug_printf(VERBOSE_INFO,"Saving ZSF snapshot %s",filename);
                      save_zsf_snapshot(filename);
              }


	else if (!util_compare_file_extension(filename,"z80") ) {
                debug_printf(VERBOSE_INFO,"Saving Z80 snapshot %s",filename);
                save_z80_snapshot(filename);
        }

	else if (!util_compare_file_extension(filename,"p") ) {
                debug_printf(VERBOSE_INFO,"Saving P snapshot %s",filename);
                new_save_zx81_p_snapshot(filename);
        }

        else if (!util_compare_file_extension(filename,"o") ) {
                debug_printf(VERBOSE_INFO,"Saving O snapshot %s",filename);
                new_save_zx80_o_snapshot(filename);
        }

	else if (!util_compare_file_extension(filename,"ace") ) {
                debug_printf(VERBOSE_INFO,"Saving ACE snapshot %s",filename);
                save_ace_snapshot(filename);
        }





 	else {
                        debug_printf (VERBOSE_ERR,"Snapshot format of file %s not supported",filename);

	}

}


//Funcion de carga de snapshot, lo normal es que se llame desde menu o desde linea de comandos
void snapshot_load_name(char *nombre)
{



        if (nombre!=NULL) {
                //if (strstr(nombre,".p")!=NULL  || strstr(nombre,".P")!=NULL || strstr(nombre,".81")!=NULL ) {
                if (!util_compare_file_extension(nombre,"p") || !util_compare_file_extension(nombre,"81") ) {
                        current_machine_type=121;

                        set_machine(NULL);
			reset_cpu();

			set_snap_file_options(nombre);
                        new_load_zx81_p_snapshot(nombre);
                }
                else if (!util_compare_file_extension(nombre,"o") || !util_compare_file_extension(nombre,"80") ) {

			//Carga de algunos juegos da problemas con esto. No reiniciarlizar si ya estamos en ZX80
			if (!(MACHINE_IS_ZX80)) {
                	        current_machine_type=120;

        	                set_machine(NULL);
				reset_cpu();
			}

			else {
				debug_printf (VERBOSE_INFO,"We do not reset machine as we are already on ZX80 mode (load routine is not perfect)");
			}

			set_snap_file_options(nombre);
                        new_load_zx80_o_snapshot(nombre);
                }

		else if (!util_compare_file_extension(nombre,"z80") ) {
			set_snap_file_options(nombre);
			load_z80_snapshot(nombre);
		}

                else if (!util_compare_file_extension(nombre,"sna") ) {
                        set_snap_file_options(nombre);
                        load_sna_snapshot(nombre);
                }

                else if (!util_compare_file_extension(nombre,"snx") ) {
                        set_snap_file_options(nombre);
                        load_snx_snapshot(nombre);
                }                


                else if (!util_compare_file_extension(nombre,"zx") ) {
			set_snap_file_options(nombre);
                        load_zx_snapshot(nombre);
                }

                else if (!util_compare_file_extension(nombre,"sp") ) {
			set_snap_file_options(nombre);
                        load_sp_snapshot(nombre);
                }

                else if (!util_compare_file_extension(nombre,"zsf") ) {
			set_snap_file_options(nombre);
                        load_zsf_snapshot(nombre);
                }


                else if (!util_compare_file_extension(nombre,"nex") ) {
			set_snap_file_options(nombre);
                        load_nex_snapshot(nombre);
                }


                else if (!util_compare_file_extension(nombre,"spg") ) {
      set_snap_file_options(nombre);
                        load_spg_snapshot(nombre);
                }


                else if (!util_compare_file_extension(nombre,"rzx") ) {
      set_snap_file_options(nombre);
                        load_rzx_snapshot_file(nombre);
                }

		else if (!util_compare_file_extension(nombre,"z81") ) {
			debug_printf (VERBOSE_INFO,"Assume z81 snapshot is ZX81. We will hotswap later to ZX80 if needed");
                        current_machine_type=121;

                        set_machine(NULL);
                        reset_cpu();

                        set_snap_file_options(nombre);
			load_z81_snapshot(nombre);
		}


		else if (!util_compare_file_extension(nombre,"ace") ) {
                        set_snap_file_options(nombre);
                        load_ace_snapshot(nombre);
                }




                else {
                        debug_printf (VERBOSE_ERR,"Snapshot format of file %s not supported",nombre);
			//es necesario esto??
                        //snapfile=NULL;
                }

        }


}

void snapshot_load(void)
{
	snapshot_load_name(snapfile);
}

//Retorna texto %Y-%m-%d-%H-%M-%S, usado en quicksave y en dump zsf on panic
//texto tiene que tener tamanyo 40, aunque cabe con menos, pero mejor asi  .   char time_string[40];
void snapshot_get_date_time_string_common(char *texto,int todos_guiones)
{
struct timeval tv;
  struct tm* ptm;
  //long microseconds;


                    // 2015/01/01 11:11:11.999999 "
                    // 123456789012345678901234567
  //const int longitud_timestamp=27;

  /* Obtain the time of day, and convert it to a tm struct. */
  gettimeofday (&tv, NULL);
  ptm = localtime (&tv.tv_sec);
  /* Format the date and time, down to a single second. */
  char time_string[40];

  //buffer temporal para poderle indicar el sizeof
  if (todos_guiones) strftime (time_string, sizeof(time_string), "%Y-%m-%d-%H-%M-%S", ptm);
  else strftime (time_string, sizeof(time_string), "%Y/%m/%d %H:%M:%S", ptm);

  //copiar a texto final
  strcpy(texto,time_string);

  //printf ("texto fecha: %s\n",texto);
}


void snapshot_get_date_time_string(char *texto)
{
	//Para formatos de quicksave y dump zsf en panic. Todo guiones
	snapshot_get_date_time_string_common(texto,1);
}

void snapshot_get_date_time_string_human(char *texto)
{
	//Para formatos de texto incluidos por ejemplo en cabecera pzx
	snapshot_get_date_time_string_common(texto,0);
}

//Realiza quicksave y retorna nombre en char nombre, siempre que no sea NULL
void snapshot_quick_save(char *nombre)
{
  char final_name[PATH_MAX];


  
  char time_string[40];

  snapshot_get_date_time_string(time_string);

  if (snapshot_autosave_interval_quicksave_directory[0]==0) sprintf (final_name,"%s-%s.zsf",snapshot_autosave_interval_quicksave_name,time_string);

  else sprintf (final_name,"%s/%s-%s.zsf",snapshot_autosave_interval_quicksave_directory,snapshot_autosave_interval_quicksave_name,time_string);

  snapshot_save(final_name);

  if (nombre!=NULL) strcpy(nombre,final_name);
}

void autosave_snapshot_at_fixed_interval(void)
{
  if (snapshot_contautosave_interval_enabled.v==0) return;

  snapshot_autosave_interval_current_counter++;
  if (snapshot_autosave_interval_current_counter>=snapshot_autosave_interval_seconds) {
    snapshot_autosave_interval_current_counter=0;

    snapshot_quick_save(NULL);

  }
}

//Devuelve el offset al bloque de memoria, teniendo en cuenta que los primeros 6 estan desordenados:
//5,2,0,1,3,4,6,7,8,9,10,...,111
/*int load_nex_snapshot_get_ram_offset(int block)
{
	switch (block) {
		default:
			return block*16384;
		break;
	}
}*/

//funcion derivada y reducida de esxdos_handler_call_f_open
//Si variable forzar_filehandle_cero es diferente de 0, es siempre el 0
//Retorna file handle. Si <0, error
int load_nex_snapshot_open_esxdos(char *nombre_archivo,int forzar_filehandle_cero)
{
	/*
	;                                                                       // Open file. A=drive. HL=Pointer to null-
;                                                                       // terminated string containg path and/or
;                                                                       // filename. B=file access mode. DE=Pointer
;                                                                       // to BASIC header data/buffer to be filled
;                                                                       // with 8 byte PLUS3DOS BASIC header. If you
;                                                                       // open a headerless file, the BASIC type is
;                                                                       // $ff. Only used when specified in B.
;                                                                       // On return without error, A=file handle.
*/

	//Abrir para lectura
	char *fopen_mode="rb";

	//z80_byte modo_abrir=reg_b;


	//Ver si no se han abierto el maximo de archivos y obtener handle libre
    int free_handle;
    
    if (forzar_filehandle_cero==0) {
	    free_handle=esxdos_find_free_fopen();
    }

    else {
        free_handle=0;
    }

	if (free_handle==-1) {
		//esxdos_handler_error_carry(ESXDOS_ERROR_ENFILE);
		//esxdos_handler_old_return_call();
		return -1;
	}

	
	char fullpath[PATH_MAX];
	

	esxdos_fopen_files[free_handle].tiene_plus3dos_header.v=0;


	//esxdos_handler_pre_fileopen(nombre_archivo,fullpath);
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//TODO: si archivo a cargar .nex esta fuera del esxdos root dir, deberia dar error
	//Si empieza por /, es ruta absoluta
	if (nombre_archivo[0]=='/' || nombre_archivo[0]=='\\') strcpy (fullpath,nombre_archivo);
	else sprintf (fullpath,"%s/%s",directorio_actual,nombre_archivo);

	printf ("ESXDOS handler: fullpath file: %s\n",fullpath);



	//Abrir el archivo.
	esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix=fopen(fullpath,fopen_mode);


	if (esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix==NULL) {
		//esxdos_handler_error_carry(ESXDOS_ERROR_ENOENT);
		printf ("ESXDOS handler: Error from esxdos_handler_call_f_open file: %s\n",fullpath);
		//esxdos_handler_old_return_call();
		return -1;
	}
	else {
		


		//reg_a=free_handle;
		//esxdos_handler_no_error_uncarry();
		printf ("ESXDOS handler: Successfully esxdos_handler_call_f_open file: %s\n",fullpath);


		if (stat(fullpath, &esxdos_fopen_files[free_handle].last_file_buf_stat)!=0) {
						printf ("ESXDOS handler: Unable to get status of file %s\n",fullpath);
		}

		
		esxdos_handler_call_f_open_post(free_handle,nombre_archivo,fullpath);


	}

	return free_handle;


}



//Cargar snapshot nex
void load_nex_snapshot(char *archivo)
{

	debug_printf(VERBOSE_DEBUG,"Loading .nex snapshot %s",archivo);



	//buffer para la cabecera
	z80_byte nex_header[NEX_HEADER_SIZE];


        FILE *ptr_nexfile;


        int leidos;

        //Load File
        ptr_nexfile=fopen(archivo,"rb");
        if (ptr_nexfile==NULL) {
                debug_printf(VERBOSE_ERR,"Error opening %s",archivo);
                return;
        }

        leidos=fread(nex_header,1,NEX_HEADER_SIZE,ptr_nexfile);
        if (leidos!=NEX_HEADER_SIZE) {
                        debug_printf(VERBOSE_ERR,"Error reading %d bytes of header",NEX_HEADER_SIZE);
                        return;
        }
        

        //Ver si signatura correcta
        if (nex_header[0]!='N' || nex_header[1]!='e' || nex_header[2]!='x' || nex_header[3]!='t') {
                        debug_printf(VERBOSE_ERR,"Unknown NEX signature: 0x%x 0x%x 0x%x 0x%x",nex_header[0],nex_header[1],nex_header[2],nex_header[3]);
                        return;
        }


	//cambio a maquina tbblue, siempre 
	//if (!MACHINE_IS_TBBLUE) {
		
        current_machine_type=MACHINE_ID_TBBLUE;

		//temporalmente ponemos tbblue fast boot mode y luego restauramos valor anterior
		z80_bit antes_tbblue_fast_boot_mode;
		antes_tbblue_fast_boot_mode.v=tbblue_fast_boot_mode.v;
		tbblue_fast_boot_mode.v=1;

        set_machine(NULL);
        reset_cpu();

		tbblue_fast_boot_mode.v=antes_tbblue_fast_boot_mode.v;
	//}


	//Al cargar .nex lo pone en turbo x 4
	debug_printf(VERBOSE_DEBUG,"Setting turbo x 4 because it's the usual speed when loading .nex files from NextOS");

	z80_byte reg7=tbblue_registers[7];
	
	reg7 &=(255-3); //Quitar los dos bits bajos

	reg7 |=2;
	//(R/W)	07 => Turbo mode
	//bit 1-0 = Turbo (00 = 3.5MHz, 01 = 7MHz, 10 = 14MHz)

	tbblue_registers[7]=reg7;
	tbblue_set_emulator_setting_turbo();




	//desactivamos interrupciones. No esta en el formato pero supongo que es asi
	iff1.v=iff2.v=0;


	//check version. Permitir 1.0, 1.1 y 1.2 y avisar si mayor de 1.2

	char snap_version[5];
	//4	4	string with NEX file version, currently "V1.0", "V1.1" or "V1.2"
	snap_version[0]=nex_header[4];
	snap_version[1]=nex_header[5];
	snap_version[2]=nex_header[6];
	snap_version[3]=nex_header[7];
	snap_version[4]=0;

	//no imprimirlo por si no es una string normal 
	//printf ("Snapshot version: %s\n",snap_version);

	if (
		! 
		(
		!strcmp(snap_version,"V1.0") ||
		!strcmp(snap_version,"V1.1") ||
		!strcmp(snap_version,"V1.2") 
		)
	) {

		debug_printf (VERBOSE_ERR,"Unsupported snapshot version. Loading it anyway");
	}


	//8	1	RAM required: 0 = 768k, 1 = 1792k
	//En ZEsarUX, si activo los 2048 kb, es 1792 KB para el sistema. 
	//En ZEsarUX, si activo los 1024 kb, es 768 KB para el sistema. 
	//tbblue_extra_512kb_blocks 1 o 3
	if (nex_header[8]) {
		debug_printf(VERBOSE_DEBUG,"Uses 1792 kb");
		tbblue_extra_512kb_blocks=3;
	}
	else {
		debug_printf(VERBOSE_DEBUG,"Uses 768k kb");
		tbblue_extra_512kb_blocks=1;
	}
	
	//Ofset 8: Number of 16k Banks to Load: 0-112 (see also the byte array at offset 18, which must yield this count)
	//Que sentido tiene si ya hay un array en el offset 18?? Pasamos de esto

	//border
    out_254=nex_header[11] & 7;
    modificado_border.v=1;	

	//SP
	reg_sp=value_8_to_16(nex_header[13],nex_header[12]);

	//PC
	z80_int possible_pc=value_8_to_16(nex_header[15],nex_header[14]);

	if (possible_pc!=0) {
		reg_pc=possible_pc;
		debug_printf(VERBOSE_DEBUG,"Register PC: %04XH",reg_pc);
	}

	//modo timex
	set_timex_port_ff(nex_header[138]);
	debug_printf(VERBOSE_DEBUG,"Timex mode: %02XH",timex_port_ff);


	debug_printf(VERBOSE_DEBUG,"Mapping ram %d at C000H",nex_header[139]);
	tbblue_out_port_32765(nex_header[139]);

	//file handler address
	z80_int nex_file_handler=value_8_to_16(nex_header[141],nex_header[140]);
	debug_printf(VERBOSE_DEBUG,"File handler: %d",nex_file_handler);

	

	int cargar_paleta=0;
	z80_byte load_screen_blocks=nex_header[10];


	// Only Layer2 and Lo-Res screens expect the palette block (unless +128 flag set
	if ( (load_screen_blocks & 1) || (load_screen_blocks & 4) ) {
		cargar_paleta=1;
	}

	if (load_screen_blocks & 128) cargar_paleta=0;

	//Cargar paleta optional palette (for Layer2 or LoRes screen)
	if (cargar_paleta) {
		debug_printf(VERBOSE_DEBUG,"Loading palette");
		leidos=fread(tbblue_palette_layer2_second,1,512,ptr_nexfile);
	}

	//Cargar Layer2 loading screen
	if (load_screen_blocks & 1) {
		debug_printf(VERBOSE_DEBUG,"Loading Layer2 loading screen");
		int tbblue_layer2_offset=tbblue_get_offset_start_layer2();
		leidos=fread(&memoria_spectrum[tbblue_layer2_offset],1,49152,ptr_nexfile);
		//Asumimos que esta activo modo layer2 entonces
		//tbblue_out_port_layer2_value(1);
		//tbblue_registers[0x15]=4; //Layer priority L S U
	}	

	//classic ULA loading screen
	if (load_screen_blocks & 2) {
		debug_printf(VERBOSE_DEBUG,"Loading classic ULA loading screen");
		z80_byte *pant;
		pant=get_base_mem_pantalla();
		leidos=fread(pant,1,6912,ptr_nexfile);
	}		

	//LoRes loading screen
	if (load_screen_blocks & 4) {
		debug_printf(VERBOSE_DEBUG,"Loading LoRes loading screen");
		z80_byte *pant;
		pant=get_lores_pointer(0);
		leidos=fread(pant,1,12288,ptr_nexfile);

		//Asumimos modo lores
		//tbblue_registers[0x15]=128;
	}		


	//Timex HiRes (512x192) loading screen
	if (load_screen_blocks & 8) {
		debug_printf(VERBOSE_DEBUG,"Loading Timex HiRes loading screen");
		z80_byte *pant;
		pant=tbblue_ram_memory_pages[5*2];

		//primer bloque
		leidos=fread(pant,1,6144,ptr_nexfile);

		pant +=0x2000;

		//segundo bloque
		leidos=fread(pant,1,6144,ptr_nexfile);		
	}			

	//Timex HiCol (8x1) loading screen
	if (load_screen_blocks & 16) {
		debug_printf(VERBOSE_DEBUG,"Timex HiCol (8x1) loading screen");
		z80_byte *pant;
		pant=tbblue_ram_memory_pages[5*2];

		//primer bloque
		leidos=fread(pant,1,6144,ptr_nexfile);

		pant +=0x2000;

		//segundo bloque
		leidos=fread(pant,1,6144,ptr_nexfile);	
	}		

	//TODO: que modo activo de video esta? aparte del timex, no se puede saber si esta lores, o layer2, o ula normal
	//pruebo a activar el modo de la pantalla que carga pero no parece que ningun snapshot tenga una pantalla valida


	//16kiB raw memory bank data in predefined order: 5,2,0,1,3,4,6,7,8,9,10,...,111 (particular bank may be omitted completely)
	//Vamos a cargar los posibles 112 bloques en ram
	//Aunque no vayan a existir esos bloques, los cargamos todos y luego ya vemos
#define NEX_RAM_BLOCKS 112


	//Gestionar los primeros 6 bloques desordenados

	int array_bloques[6]={5,2,0,1,3,4};

	int i;
	for (i=0;i<6;i++) {
		//int offset_bloque;
		int bloque_cargar=array_bloques[i];
		z80_byte esta_presente=nex_header[18+bloque_cargar];
		if (esta_presente) {
			debug_printf(VERBOSE_DEBUG,"Loading ram block %d",bloque_cargar);
			z80_byte *destino=tbblue_ram_memory_pages[bloque_cargar*2];
			leidos=fread(destino,1,16384,ptr_nexfile);	
		}
	}

	//Leer el resto de bloques
	for (i=6;i<112;i++) {
		//int offset_bloque;
		int bloque_cargar=i;
		z80_byte esta_presente=nex_header[18+bloque_cargar];
		if (esta_presente) {
			debug_printf(VERBOSE_DEBUG,"Loading ram block %d",bloque_cargar);
			z80_byte *destino=tbblue_ram_memory_pages[bloque_cargar*2];
			leidos=fread(destino,1,16384,ptr_nexfile);	
		}
	}	

	//Gestionar file handler
	/*
	-.nex format. file handler:
“File handle address: 0 = NEX file is closed by the loader, 1..0x3FFF values (1 recommended) = NEX loader keeps NEX file open and does 
pass the file handle in BC register, 0x4000..0xFFFF values (for 0xC000..0xFFFF see also "Entry bank") = NEX loader 
keeps NEX file open and the file handle is written into memory at the desired address.”

If the word at offset is 0, I just simply close the file and I don’t do anything else. Set bc=255

If the value is between 1...0x3fff value, I keep the file open and the file handler number is copied to register BC

If the value is between 0x4000 and ffff, I write the file handler number at the address that poitnts this offset 140. Set bc to 255 

Y esto hacerlo después de marear toda la ram y cargar los bloques de memoria, lógicamente 

-Parámetro config de tipo background ZX desktop. Más tipos?
y parámetro de color del tipo de fondo sólido
	 */
	
	//por defecto hacemos que registro bc=255, error
	BC=255;

	if (nex_file_handler>0) {
		

		debug_printf(VERBOSE_DEBUG,"Uses NextOS file handler");
		
		//De momento asumimos error y escribimos 255 en la memoria tambien (en caso que nex_file_handler esta entre 0x4000 y ffff)
		//Asi le daremos un file handler erroneo si pasase cualquier cosa
		if (nex_file_handler>=0x4000 && nex_file_handler<0xffff) {
			poke_byte_no_time(nex_file_handler,255);
		}

		//Si no esta esxdos handler habilitado, avisar y no hacer nada mas
		if (esxdos_handler_enabled.v) {
			//Obtener offset actual sobre archivo snapshot abierto
			long initial_offset=ftell(ptr_nexfile);
			debug_printf(VERBOSE_DEBUG,"Current offset of .nex file after loading it: %ld",initial_offset);

			//Abrir este mismo archivo desde esxdos handler. Luego hacer seek hasta el offset que tenemos

			//Cerrarlo antes, por problemas de bloqueo en sistema operativo al tenerlo dos veces (en windows?)
			fclose(ptr_nexfile);


			int esx_file_handler=load_nex_snapshot_open_esxdos(archivo,0);
			debug_printf(VERBOSE_DEBUG,"file handle of esxdos open file: %d",esx_file_handler);

			if (esx_file_handler>=0) {
				//Hacer fseek
				if (fseek (esxdos_fopen_files[esx_file_handler].esxdos_last_open_file_handler_unix, initial_offset, SEEK_CUR)!=0) {
					debug_printf (VERBOSE_ERR,"ESXDOS handler: Error running fseek system call");
				}

				//Retornar BCDE
				long cur_offset=ftell(esxdos_fopen_files[esx_file_handler].esxdos_last_open_file_handler_unix);

				debug_printf(VERBOSE_DEBUG,"ESXDOS handler: offset is now at %ld",cur_offset);


				//If the value is between 1...0x3fff value, I keep the file open and the file handler number is copied to register BC

				//If the value is between 0x4000 and ffff, I write the file handler number at the address that poitnts this offset 140. Set bc to 255 
				if (nex_file_handler<=0x3fff) {
					BC=esx_file_handler;
					debug_printf(VERBOSE_DEBUG,"Setting BC register to value %04XH",BC);
				}
				else {
					//copiar handler en memoria
					debug_printf(VERBOSE_DEBUG,"Writing file handler to memory, address %04XH",nex_file_handler);
					poke_byte_no_time(nex_file_handler,esx_file_handler);
				}

			}

		}
		else {
			debug_printf (VERBOSE_ERR,"Snapshot uses NextOS file handler. You should enable esxdos handler before loading it");
			fclose(ptr_nexfile);
		}
	}
	else {
		fclose(ptr_nexfile);
	}

}

//Volcar snapshot cuando hay cpu panic
void snap_dump_zsf_on_cpu_panic(void)
{

	//Si volcar snapshot zsf cuando hay cpu_panic
	if (debug_dump_zsf_on_cpu_panic.v==0) return;

	//printf ("Intentando volcado zsf on panic\n");

	//Si ya se ha volcado snapshot zsf cuando hay cpu_panic, para evitar un segundo volcado (y siguientes) si se genera otro panic al hacer el snapshot
	if (dumped_debug_dump_zsf_on_cpu_panic.v) return;

	//printf ("Volcando zsf on panic\n");
	 dumped_debug_dump_zsf_on_cpu_panic.v=1;





 
	 char time_string[40];

  snapshot_get_date_time_string(time_string);

  sprintf (dump_snapshot_panic_name,"cpu_panic-%s.zsf",time_string);

  snapshot_save(dump_snapshot_panic_name);

}