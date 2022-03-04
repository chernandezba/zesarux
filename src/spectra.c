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
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>



#include "cpu.h"
#include "spectra.h"
#include "debug.h"
#include "utils.h"
#include "menu.h"
#include "screen.h"
#include "operaciones.h"
#include "ulaplus.h"
#include "mem128.h"


z80_bit spectra_enabled={0};
z80_byte spectra_display_mode_register=0;

z80_byte *spectra_ram=NULL;

//rutinas de poke byte antes de cambiarlas
//void (*original_spectra_poke_byte_no_time)(z80_int dir,z80_byte valor);
//void (*original_spectra_poke_byte)(z80_int dir,z80_byte valor);

int spectra_nested_id_poke_byte;
int spectra_nested_id_poke_byte_no_time;


//Hacer poke a shadow ram de spectra si conviene
void spectra_poke_shadow_ram(z80_int dir,z80_byte valor)
{

	//Mode 128k
	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
		z80_byte ram_write=0;

		z80_int offset;

		//Writing on address 16384-32767
                if (dir>=16384 && dir<=32767) {
			ram_write=5;
                }

		//Segment C000H
		else if (dir>=49152) {
			//See mapped ram 
			//TODO: Do not look at special paging modes RAM in ROM of Spectrum +2A
			//puerto_32765 has the value for port 32765
			ram_write=puerto_32765&7;
		}

		//Write on RAM5
		if (ram_write==5) {
			//Same behaviour as Spectrum 48k
                        //Get offset in range 0-16383
                        offset=dir&16383;

                        //Look at which spectra shadow ram
                        if (spectra_display_mode_register&64) offset+=16384;

                        //Write in spectra memory
                        spectra_ram[offset]=valor;

		}


		//Write on RAM7
		if (ram_write==7) {
                        //Get offset in range 0-16383
                        offset=dir&16383;
                        
                        //Look at which spectra shadow ram
                        if ((spectra_display_mode_register&64)==0) offset+=16384;
                        
                        //Write in spectra memory
                        spectra_ram[offset]=valor;

		}

		//debug
		//if (ram_write==5 || ram_write==7) {
		//	printf ("Writing ram %d spectra offset: %d\n",ram_write,offset);
		//}


	}

	else {
		//Mode 48k
		//printf ("usando poke de spectra: dir : %d\n",dir);
		if (dir>=16384 && dir<=32767) {
			//printf ("Poke en zona 16384-32767\n");

			//Get offset in range 0-16383
			z80_int offset=dir&16383;

			//Look at which spectra shadow ram
			if (spectra_display_mode_register&64) offset+=16384;

			//Write in spectra memory
			spectra_ram[offset]=valor;

		}

	}

}


z80_byte poke_byte_no_time_spectrum_spectra(z80_int dir,z80_byte valor)
{
	spectra_poke_shadow_ram(dir,valor);
	//original_spectra_poke_byte_no_time(dir,valor);
	debug_nested_poke_byte_no_time_call_previous(spectra_nested_id_poke_byte_no_time,dir,valor);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;

}


z80_byte poke_byte_spectrum_spectra(z80_int dir,z80_byte valor)
{
	spectra_poke_shadow_ram(dir,valor);
	//original_spectra_poke_byte(dir,valor);
	debug_nested_poke_byte_call_previous(spectra_nested_id_poke_byte,dir,valor);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;

}


void spectra_set_poke(void)
{
	debug_printf (VERBOSE_DEBUG,"Setting Spectra poke functions");
	//original_spectra_poke_byte_no_time=poke_byte_no_time;
	//original_spectra_poke_byte=poke_byte;
	//poke_byte_no_time=poke_byte_no_time_spectrum_spectra;
	//poke_byte=poke_byte_spectrum_spectra;
        spectra_nested_id_poke_byte=debug_nested_poke_byte_add(poke_byte_spectrum_spectra,"Spectra poke_byte");
        spectra_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(poke_byte_no_time_spectrum_spectra,"Spectra poke_byte_no_time");
}

void spectra_restore_poke(void)
{
	debug_printf (VERBOSE_DEBUG,"Restoring poke functions before enabling Spectra");
	//poke_byte_no_time=original_spectra_poke_byte_no_time;
	//poke_byte=original_spectra_poke_byte;
        debug_nested_poke_byte_del(spectra_nested_id_poke_byte);
        debug_nested_poke_byte_no_time_del(spectra_nested_id_poke_byte_no_time);
}

void spectra_enable(void)
{

	//Si no esta activo y si maquina es spectrum
	if (spectra_enabled.v==0 && MACHINE_IS_SPECTRUM) {
		debug_printf (VERBOSE_INFO,"Enabling Spectra");


		//asignar los 32kb de ram

		if (spectra_ram==NULL) {
			spectra_ram=malloc(32768);
			if (spectra_ram==NULL) cpu_panic ("Error allocating Spectra RAM");
		}

		//Copiar contenido de 16384-32767 en las dos rams de spectra, para que 
		//al habilitar spectra no aparezca la pantalla en negro (porque no tiene datos)
		z80_int i;
		z80_byte leido;
		for (i=0;i<16384;i++) {
			leido=peek_byte_no_time(16384+i);
			spectra_ram[i]=leido;
			spectra_ram[i+16384]=leido;
		}

		spectra_set_poke();

		spectra_enabled.v=1;

	        //son excluyentes
        	//disable_interlace();
		disable_scanlines();
	        disable_gigascreen();
		disable_ulaplus();
		disable_timex_video();

	        //necesita real video
        	enable_rainbow();

		//modo 0. no tocar. esto se resetea al hacer un reset cpu
		//spectra_display_mode_register=0;

	}

}

void spectra_disable(void)
{

	if (spectra_enabled.v) {
		debug_printf (VERBOSE_INFO,"Disabling Spectra");

		spectra_restore_poke();
        	spectra_enabled.v=0;

	}
}


z80_byte spectra_read(void)
{
	return spectra_display_mode_register;
}

void spectra_return_text_mode(z80_byte mode_register,char *text)
{


        int spectra_line_height=mode_register&3;

        int spectra_basicextra_colors=(mode_register>>2)&1;

        int spectra_doublebyte_colors=(mode_register>>3)&1;

        int spectra_half_cell=(mode_register>>7)&1;

	char width;
	width=(spectra_half_cell ? '4' : '8');

	char height;
	if (spectra_line_height==0) height='8';
	if (spectra_line_height==1) height='4';
	if (spectra_line_height==2) height='2';
	if (spectra_line_height==3) height='1';

	char singledouble;
	singledouble=(spectra_doublebyte_colors ? 'D' : 'S');

	char basicextra;
	basicextra=(spectra_basicextra_colors ? 'E' : 'B');

	sprintf (text,"%cx%c%c%c",width,height,singledouble,basicextra);
}

	


void spectra_write(z80_byte value)
{
	//bits de modo:
	//7: Full/Half Cell
	//6: Shadow bank 0/1
	//5: Display bank 0/1
	//4: Standard border (0), Enhanced border(1)
	//3: Single byte colour (0), Double byte colour(1)
	//2: Basic colours (0), Extra colours(1)
	//1,0: Line height: 00=Row, 01=Quad, 10=Dual, 11=Single

	//Para ver el numero de modo quitamos bits 6,5
	z80_byte mascara_modo=255-64-32;

	if ( (spectra_display_mode_register & mascara_modo)!=(value&mascara_modo)) {
		char mensaje[200];
		char nombremodo[10];
		spectra_return_text_mode(value,nombremodo);

		sprintf (mensaje,"Setting Spectra Video mode %d (%s)",value&mascara_modo,nombremodo);
		screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,mensaje);
	}

	spectra_display_mode_register=value;
}
