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
   AY Player related stuff
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "cpu.h"
#include "audio_ayplayer.h"
#include "audio.h"
#include "debug.h"
#include "autoselectoptions.h"
#include "utils.h"
#include "operaciones.h"
#include "cpc.h"
#include "ay38912.h"
#include "zvfs.h"
#include "start.h"
#include "menu_items.h"
#include "menu_filesel.h"



z80_byte *audio_ay_player_mem;

int audio_ay_player_file_size=0;

//Longitud de la cancion en 1/50s
z80_int ay_song_length=0;
//Contador actual de transcurrido
z80_int ay_song_length_counter=0;

z80_byte ay_player_pista_actual=1;

z80_bit ay_player_playing={0};

//Cerrar emulador al reproducir todas canciones de archivo AY
z80_bit ay_player_exit_emulator_when_finish={0};

//Limite en 1/50 segundos para canciones que tienen duracion infinita (0)
z80_int ay_player_limit_infinite_tracks=0;

//Limite en 1/50 segundos para cualquier cancion. Si es 0, no limitar
z80_int ay_player_limit_any_track=0;

//Tiempo en segundos para sumarle a la duración de cada pista, por algunos pocos casos
//en que la duración está mal (ejemplo pista 9 de toi acid game)
z80_int ay_player_add_to_track=0;

//Ir a primera pista cuando se llega al final
//En versiones previas a la 10.10 esto venia activado por defecto
z80_bit ay_player_repeat_file={0};

//Reproducir en modo cpc
z80_bit ay_player_cpc_mode={0};

//Mostrar info de cancion tambien en consola
z80_bit ay_player_show_info_console={0};

//Modo aleatorio
z80_bit ay_player_shuffle_mode={0};

//Para detectar silencio y pasar a la siguiente pista
//Este funciona diferente que el resto de detectores de silencio,
//basicamente lo que hace es que si el sample de audio enviado a la tarjeta de sonido es igual al anterior, asume silencio e incrementa un contador
z80_bit ay_player_silence_detection={1};

int ay_player_silence_detection_counter=0;

//Si pausado
z80_bit ay_player_paused={0};


//Retorna valor de 16 bit apuntado como big endian
z80_int audio_ay_player_get_be_word(int index)
{
	return ((audio_ay_player_mem[index])*256 ) + audio_ay_player_mem[index+1];
}


//Retorna valor a indice teniendo como entrada indice y un puntero de 16 bits absoluto en ese indice inicial
int audio_ay_player_get_abs_index(int initial_index)
{
	int final_index;

	final_index=initial_index+audio_ay_player_get_be_word(initial_index);

	return final_index;
}

z80_byte ay_player_total_songs(void)
{
	return audio_ay_player_mem[16]+1;
}

z80_byte ay_player_first_song(void)
{
	return audio_ay_player_mem[17]+1;
}

z80_byte ay_player_version(void)
{
	return audio_ay_player_mem[9];
}

z80_byte ay_player_release_file_version(void)
{
	return audio_ay_player_mem[8];
}

int ay_player_size(void)
{
	return audio_ay_player_file_size;
}

void ay_player_set_footer(char *texto1,char *texto2)
{
    sprintf (mostrar_footer_first_message,"%s",texto1);

	//Texto mostrado
	sprintf (mostrar_footer_first_message_mostrado,"%s",mostrar_footer_first_message);

	mostrar_footer_second_message=texto2;
	//mostrar_footer_second_message=NULL;


	indice_first_message_mostrado=indice_second_message_mostrado=0;

	//Cortar a 32
	tape_options_corta_a_32(mostrar_footer_first_message_mostrado);

	tape_options_set_first_message_counter=4;

	autoselect_options_put_footer();

}

char ay_player_file_author[1024]="";
char ay_player_file_misc[1024]="";
char ay_player_file_song_name[1024]="";
char ay_player_second_footer[1024]="";

//Retorna 0 si ok
int audio_ay_player_load(char *filename)
{
    //Validar extension, si por error se carga otro archivo de diferente extension
    //(por ejemplo por error al seleccionar Recent Files), puede petar el emulador

    if (util_compare_file_extension(filename,"ay")) {
        debug_printf(VERBOSE_ERR,"AY file does not have .ay extension");
        return 1;
    }

	//Leemos archivo en memoria
	int total_mem=get_file_size(filename);

    audio_ay_player_file_size=total_mem;


    FILE *ptr_ayfile;
    ptr_ayfile=fopen(filename,"rb");

    if (!ptr_ayfile) {
	  debug_printf(VERBOSE_ERR,"Unable to open ay file");
	  return 1;
	}

	//Si estaba asignado, desasignar
	if (audio_ay_player_mem!=NULL) free(audio_ay_player_mem);

	audio_ay_player_mem=malloc(total_mem);
	if (audio_ay_player_mem==NULL) cpu_panic("Error allocating memory for ay file");

	//z80_byte *puntero_lectura;
	//puntero_lectura=audio_ay_player_mem;


    int leidos=fread(audio_ay_player_mem,1,total_mem,ptr_ayfile);

	if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading ay file");
        free(audio_ay_player_mem);
        audio_ay_player_mem=NULL;
        return 1;
    }


    fclose(ptr_ayfile);

    //Mostrar información de la canción
	int indice_autor=audio_ay_player_get_abs_index(12);
	int indice_misc=audio_ay_player_get_abs_index(14);
	z80_byte file_version=ay_player_version();

	debug_printf (VERBOSE_INFO,"Version: %d",file_version);
	debug_printf (VERBOSE_INFO,"Author: %s",&audio_ay_player_mem[indice_autor]);
	debug_printf (VERBOSE_INFO,"Misc: %s",&audio_ay_player_mem[indice_misc]);
	debug_printf (VERBOSE_INFO,"Total songs: %d",ay_player_total_songs() );
	debug_printf (VERBOSE_INFO,"First song: %d",ay_player_first_song() );

    //sprintf (ay_player_file_author_misc,"%s - %s",&audio_ay_player_mem[indice_autor],&audio_ay_player_mem[indice_misc]);
    sprintf (ay_player_file_author,"%s",&audio_ay_player_mem[indice_autor]);
    sprintf (ay_player_file_misc,"%s",&audio_ay_player_mem[indice_misc]);


    if (ay_player_show_info_console.v) {
        printf("Playing AY file: %s\n",filename);
	    printf ("Version: %d\n",file_version);
	    printf ("Author: %s\n",&audio_ay_player_mem[indice_autor]);
	    printf ("Misc: %s\n",&audio_ay_player_mem[indice_misc]);
	    printf ("Total songs: %d\n",ay_player_total_songs() );
	    printf ("First song: %d\n",ay_player_first_song() );
    }


	if (file_version>3) {
		debug_printf(VERBOSE_ERR,"File version>3 not supported yet (file version: %d)",file_version);
		return 1;
	}


    return 0;
}

//Retorna indice a estructura canciones
int audio_ay_player_song_str(void)
{
	return audio_ay_player_get_abs_index(18);
	//return 20;

	/*
	Que es psongstructure???
	18   PSongsStructure:smallint;	//Relative pointer to song structure
	  end;

	20  Next is song structure (repeated NumOfSongs + 1 times):

	*/
}

void ay_player_poke(z80_int destino,z80_byte valor)
{
	if (MACHINE_IS_SPECTRUM) {
		if (destino<16384) memoria_spectrum[destino]=valor;
		else poke_byte_no_time(destino,valor);

		//printf ("poke %04X valor %02X\n",destino,valor);
	}

	if (MACHINE_IS_CPC) {
		poke_byte_no_time_cpc(destino,valor);
	}
}

void ay_player_copy_mem(int origen_archivo,z80_int destino,z80_int longitud)
{
	for (;longitud;longitud--) {
		//memoria_spectrum[destino++]=audio_ay_player_mem[origen_archivo++];
		ay_player_poke(destino++,audio_ay_player_mem[origen_archivo++]);
	}
}


void ay_player_copy_to_rom(z80_byte *origen,z80_int destino,z80_int longitud)
{
	for (;longitud;longitud--) {
		ay_player_poke(destino++,*origen);
		//printf ("poke %d value %02X\n",destino,*origen);

		origen++;
	}
}

void ay_player_mem_set(z80_int destino,z80_byte valor,z80_int longitud)
{
	for (;longitud;longitud--) {
		ay_player_poke(destino++,valor);
	}
}

//Retorna 0 si ok
int audio_ay_player_play_song(z80_byte song)
{
    ay_player_silence_detection_counter=0;

	if (audio_ay_player_mem==NULL) {
		debug_printf(VERBOSE_ERR,"No song loaded");
		return 1;
	}

	if (song<1) {
        debug_printf (VERBOSE_ERR,"Song number must be >0");
        return 1;
	}

	z80_byte version=ay_player_version();

	//Puntero a estructura inicial
	int song_struct=audio_ay_player_song_str();

	//Puntero a cancion concreta
	int offset=(song-1)*4;

	song_struct +=offset;

	//Mostrar nombre cancion
	int indice_nombre=audio_ay_player_get_abs_index(song_struct);
	debug_printf (VERBOSE_INFO,"Song %d name: %s",song,&audio_ay_player_mem[indice_nombre]);



	int song_data=audio_ay_player_get_abs_index(song_struct+2);


    if (ay_player_cpc_mode.v==0) {
        //Si maquina actual ya es spectrum, no cargar de nuevo rom
        if (current_machine_type!=MACHINE_ID_SPECTRUM_48) {
            //Resetear a maquina spectrum 48
            current_machine_type=MACHINE_ID_SPECTRUM_48;
            set_machine(NULL);
        }

    }


    else {
        if (current_machine_type!=MACHINE_ID_CPC_464) {
            current_machine_type=MACHINE_ID_CPC_464;
            set_machine(NULL);
        }

    }


	cold_start_cpu_registers();
	reset_cpu();

    if (ay_player_cpc_mode.v==1) {
        cpc_gate_registers[2] |=(4|8); //Mapear todo RAM
        cpc_set_memory_pages();
    }

	sprintf (ay_player_file_song_name,"%s",&audio_ay_player_mem[indice_nombre]);

	sprintf (ay_player_second_footer,"%s - %s",ay_player_file_author,ay_player_file_misc);


	ay_player_set_footer(ay_player_file_song_name,ay_player_second_footer);

	ay_song_length=audio_ay_player_get_be_word(song_data+4);

	if (ay_song_length==0) ay_song_length=ay_player_limit_infinite_tracks;

	if (ay_player_limit_any_track>0) {
		if (ay_song_length>ay_player_limit_any_track || ay_song_length==0) ay_song_length=ay_player_limit_any_track;
	}


	//printf ("Song length: %d 1/50s (%d s)\n",ay_song_length,ay_song_length/50);

    //printf("Fade length: %d 1/50s\n",audio_ay_player_get_be_word(song_data+6));

	ay_song_length_counter=0;
	ay_player_playing.v=1;
    ay_player_paused.v=0;

	//Cargar registros
	/*
	12   HiReg,LoReg:byte;		//HiReg and LoReg for all common registers:
					//AF,AF',HL,HL',DE,DE',BC,BC',IX and IY
					*/

	//Mostrar hexadecimal registros
	int offset_to_registers=song_data+8;

	/*
	int i;
	printf ("Dump registers:\n");
	for (i=0;i<20;i++){
		printf("%02d %02XH - ",i,audio_ay_player_mem[offset_to_registers+i]);
	}

	printf ("\n");
*/

	/*
	reg_a=0xff;
	Z80_FLAGS=0xff;
	BC=HL=DE=0xffff;
	reg_ix=reg_iy=reg_sp=0xffff;

	reg_h_shadow=reg_l_shadow=reg_b_shadow=reg_c_shadow=reg_d_shadow=reg_e_shadow=reg_a_shadow=Z80_FLAGS_SHADOW=0xff;
	reg_i=0;
	reg_r=reg_r_bit7=0;
	*/
    //AF,AF',HL,HL',DE,DE',BC,BC',IX and IY

    //No se en que version de ay se usan los registros... Al  menos en 0 no se usan!
    //En 1,2,3 parece que si...

    //Quiza solo lee registro A??
	reg_a=audio_ay_player_mem[offset_to_registers+0];
	Z80_FLAGS=audio_ay_player_mem[offset_to_registers+1];
	/*reg_a_shadow=audio_ay_player_mem[offset_to_registers+2];
	Z80_FLAGS_SHADOW=audio_ay_player_mem[offset_to_registers+3];

	reg_h=audio_ay_player_mem[offset_to_registers+4];
	reg_l=audio_ay_player_mem[offset_to_registers+5];
	reg_h_shadow=audio_ay_player_mem[offset_to_registers+6];
	reg_l_shadow=audio_ay_player_mem[offset_to_registers+7];

	reg_d=audio_ay_player_mem[offset_to_registers+8];
	reg_e=audio_ay_player_mem[offset_to_registers+9];
	reg_d_shadow=audio_ay_player_mem[offset_to_registers+10];
	reg_e_shadow=audio_ay_player_mem[offset_to_registers+11];

	reg_b=audio_ay_player_mem[offset_to_registers+12];
	reg_c=audio_ay_player_mem[offset_to_registers+13];
	reg_b_shadow=audio_ay_player_mem[offset_to_registers+14];
	reg_c_shadow=audio_ay_player_mem[offset_to_registers+15];

	reg_ix=audio_ay_player_mem[offset_to_registers+16]*256+audio_ay_player_mem[offset_to_registers+17];
	reg_iy=audio_ay_player_mem[offset_to_registers+18]*256+audio_ay_player_mem[offset_to_registers+19];*/

	//Volcar desde song_data en adelante

	/*
	int jj;
	printf ("--volcado desde song_data--\n");
	for (jj=0;jj<64;jj++)
	{
		if ((jj%16)==0) printf ("\n");
		printf ("%02X ",audio_ay_player_mem[song_data+jj]);

	}

	printf ("--fin volcado--\n");
	*/

	int ppoints,pdata;

	//if (version==0) {
	if (version==0 || version==1 || version==2 || version==3)  {
			ppoints=audio_ay_player_get_abs_index(song_data+10);
			pdata=audio_ay_player_get_abs_index(song_data+12);
	}

	else {
		ppoints=audio_ay_player_get_abs_index(offset_to_registers+20);
	//int ppoints=audio_ay_player_get_abs_index(offset_to_registers+10);

		pdata=audio_ay_player_get_abs_index(offset_to_registers+22);
	//int pdata=audio_ay_player_get_abs_index(offset_to_registers+12);

}

	reg_sp=audio_ay_player_get_be_word(ppoints);
	z80_int reg_init=audio_ay_player_get_be_word(ppoints+2);
	z80_int reg_inter=audio_ay_player_get_be_word(ppoints+4);

	debug_printf (VERBOSE_DEBUG,"SP: %04XH init=%04XH inter:%04XH",reg_sp,reg_init,reg_inter);

	/*if (reg_init==0) {
			debug_printf(VERBOSE_ERR,"AY files with init=0 not supported yet");
			return 1;
	}*/

	/*if (reg_inter==0) {
		debug_printf(VERBOSE_ERR,"AY files with inter=0 not supported yet");
		return 1;
	}*/


	//a) Fill #0000-#00FF range with #C9 value
	//b) Fill #0100-#3FFF range with #FF value
	//c) Fill #4000-#FFFF range with #00 value
	//d) Place to #0038 address #FB value
	//memset(memoria_spectrum+0x0000,0xc9,0x0100);
	//memset(memoria_spectrum+0x0100,0xff,0x3f00);
	//memset(memoria_spectrum+0x4000,0x00,0xc000);

    ay_player_mem_set(0x0000,0xc9,0x0100);
    ay_player_mem_set(0x0100,0xff,0x3f00);
    ay_player_mem_set(0x4000,0x00,0xc000);



	//memoria_spectrum[0x38]=0xfb;         /* ei */
	ay_player_poke(0x38,0xfb);/* ei */
    /*
        f) if INTERRUPT equal to ZERO then place at ZERO address next player:

        DI
        CALL    INIT
    LOOP:	IM      2
        EI
        HALT
        JR      LOOP
        */

    /*	g) if INTERRUPT not equal to ZERO then place at ZERO address next player:

        DI
        CALL    INIT
        LOOP:	IM      1
        EI
        HALT
        CALL INTERRUPT
        JR      LOOP
    */

    z80_byte player[256];

    if (reg_inter==0) {
        //ay_player_copy_to_rom(z80_byte *origen,z80_int destino,z80_int longitud)

        player[0]=243;

        player[1]=205;
        player[2]=value_16_to_8l(reg_init);
        player[3]=value_16_to_8h(reg_init);

        player[4]=237;
        player[5]=94;

        player[6]=251;

        player[7]=118;  //HALT


        player[8]=24;
        player[9]=256-6;

        //Hueco para llegar a posicion 13
        player[10]=201;

        player[11]=201;
        player[12]=201;

    }
    else {

        player[0]=243;

        player[1]=205;
        player[2]=value_16_to_8l(reg_init);
        player[3]=value_16_to_8h(reg_init);

        player[4]=237;
        player[5]=86;

        player[6]=251;

        player[7]=118;  //HALT

        player[8]=205; //CALL INTER
        player[9]=value_16_to_8l(reg_inter);
        player[10]=value_16_to_8h(reg_inter);

        player[11]=24;
        player[12]=256-9;



    }

    //Para poder hacer la pausa
    //DI.
    //LOOP: JR LOOP
    player[13]=243;
    player[14]=24;
    player[15]=256-2;

    //Para despausarlo
    //EI
    //RET
    player[16]=251;
    player[17]=201;

    ay_player_copy_to_rom(player,0,18);

	//	h) Load all blocks for this song
	//		m) Load to PC ZERO value
	reg_pc=0;

    //Ir leyendo bloques

    int bloque=0;
    z80_int bloque_direccion,bloque_longitud,bloque_offset;
    do {
        bloque_direccion=audio_ay_player_get_be_word(pdata);
        bloque_longitud=audio_ay_player_get_be_word(pdata+2);
        bloque_offset=audio_ay_player_get_be_word(pdata+4);

        /*
          In case Address+Length > 65536, DeliAY decreases the size to make it
            == 65536.
        */

        int final=bloque_direccion+bloque_longitud;
        if (final>65535) {
            //printf("Adjusting block length\n");
            bloque_longitud=65536-bloque_direccion;
        }


        if (bloque_direccion!=0) {
            debug_printf (VERBOSE_DEBUG,"Block: %d address: %04XH length: %d offset in ay file: %d",
                bloque,bloque_direccion,bloque_longitud,bloque_offset);

            z80_int origen_archivo=pdata+4+bloque_offset;
            ay_player_copy_mem(origen_archivo,bloque_direccion,bloque_longitud);
            //memcpy(&memoria_spectrum[bloque_direccion],&audio_ay_player_mem[origen_archivo],bloque_longitud);

            /*
            e) if INIT equal to ZERO then place to first CALL instruction address of
                first AY file block instead of INIT (see next f) and g) steps)
                */
            if (reg_init==0 && bloque==0) reg_pc=bloque_direccion;
        }

        bloque++;
        pdata +=6;

    } while (bloque_direccion!=0);


        //	i) Load all common lower registers with LoReg value (including AF register)
    //		j) Load all common higher registers with HiReg value
    //		k) Load into I register 3 (this player version)
    reg_i=3;
    //		l) load to SP stack value from points data of this song

    //		n) Disable Z80 interrupts and set IM0 mode
    iff1.v=iff2.v=0;
    im_mode=0;
    //		o) Emulate resetting of AY chip
    //		p) Start Z80 emulation*/

    if (ay_player_show_info_console.v) {
        z80_byte minutos_total,segundos_total,decimas_segundos_total;

        ay_player_get_duration_current_song(&minutos_total,&segundos_total,&decimas_segundos_total);

        printf ("Playing song %d name: %s. Duration: %02d:%02d.%02d\n",song,&audio_ay_player_mem[indice_nombre],
            minutos_total,segundos_total,decimas_segundos_total);

    }

    //printf("PC register: %04XH\n",reg_pc);


    return 0;

}

char ay_player_filename_playing[PATH_MAX]="";

z80_byte ayplayer_antes_pausa_ay_3_8912_reg_mixer[MAX_AY_CHIPS];


void ay_player_pause(void)
{

    int i;
    for (i=0;i<MAX_AY_CHIPS;i++) {
        ayplayer_antes_pausa_ay_3_8912_reg_mixer[i]=ay_3_8912_registros[i][7];
        ay_3_8912_registros[i][7]=255; //Silenciar canales
    }

    //call_address(13);

    push_valor(reg_pc,PUSH_VALUE_TYPE_CALL);
    reg_pc=13;


}

void ay_player_unpause(void)
{
    //Restaurar canales
    int i;
    for (i=0;i<MAX_AY_CHIPS;i++) {
        ay_3_8912_registros[i][7]=ayplayer_antes_pausa_ay_3_8912_reg_mixer[i];
    }

    //jp 16
    reg_pc=16;
}



void ay_player_pause_unpause(void)
{
    if (ay_player_paused.v==0) {
        ay_player_pause();
        ay_player_paused.v=1;
    }
    else {
        ay_player_unpause();
        ay_player_paused.v=0;
    }
}

void ay_player_load_and_play(char *filename)
{
    strcpy(ay_player_filename_playing,filename);

	if (audio_ay_player_load(filename)) {
		audio_ay_player_mem=NULL;
		return;
	}



    ay_player_pista_actual=1;
    audio_ay_player_play_song(ay_player_pista_actual);
}



ay_player_playlist_item *ay_player_first_item_playlist;
int ay_player_playlist_total_elements=0;

void ay_player_playlist_init(void)
{
    ay_player_first_item_playlist=NULL;
    ay_player_playlist_total_elements=0;
}

int ay_player_playlist_get_total_elements(void)
{
    return ay_player_playlist_total_elements;
}

void ay_player_playlist_add(char *archivo)
{
    //Asignar memoria
    ay_player_playlist_item *new_item;

    new_item=util_malloc(sizeof(ay_player_playlist_item),"Can not allocate new playlist item");

    strcpy(new_item->nombre,archivo);

    new_item->marcado=0;

    //Es el ultimo y por tanto no tiene siguiente
    new_item->next_item=NULL;



    //Si es el primero
    if (ay_player_first_item_playlist==NULL) {
        ay_player_first_item_playlist=new_item;
    }

        else {
        //Agregarlo al ultimo
        ay_player_playlist_item *last_item=ay_player_first_item_playlist;

        while (last_item->next_item!=NULL) {
            last_item=last_item->next_item;
        }
        last_item->next_item=new_item;
    }

    ay_player_playlist_total_elements++;
}

ay_player_playlist_item *ay_player_search_item(int position)
{

    if (position<0 || position>ay_player_playlist_total_elements-1) {
        debug_printf(VERBOSE_ERR,"Can not search beyond total items: Position asked: %d",position);
        return NULL;
    }


    ay_player_playlist_item *item_to_search=ay_player_first_item_playlist;



    int i;

    for (i=0;i<position;i++) {
        item_to_search=item_to_search->next_item;
    }

    return item_to_search;

}

void ay_player_mark_unmark_this_item(int position)
{


    ay_player_playlist_item *item_to_search=ay_player_search_item(position);
    if (item_to_search==NULL) return;

    item_to_search->marcado ^=1;

}

void ay_player_playlist_remove(int position)
{

    if (position<0 || position>ay_player_playlist_total_elements-1) {
        debug_printf(VERBOSE_ERR,"Can not delete beyond total items: Position asked: %d",position);
        return;
    }

    ay_player_playlist_item *item_to_delete=ay_player_first_item_playlist;
    ay_player_playlist_item *previous_item=NULL;
    ay_player_playlist_item *next_item=NULL;


    if (position==0) {
        ay_player_first_item_playlist=ay_player_first_item_playlist->next_item;
    }

    else {

        int i;

        for (i=0;i<position;i++) {
            previous_item=item_to_delete;
            item_to_delete=item_to_delete->next_item;
            next_item=item_to_delete->next_item;
        }

        //Apuntar al anterior el siguiente, siempre que no sea el primero
        previous_item->next_item=next_item;

    }

    //Liberar memoria del current
    free(item_to_delete);

    ay_player_playlist_total_elements--;

}

void ay_player_playlist_remove_all(void)
{

    ay_player_playlist_item *item_to_delete=ay_player_first_item_playlist;


    while (item_to_delete!=NULL) {

        debug_printf(VERBOSE_INFO,"Removing %s",item_to_delete->nombre);

        ay_player_playlist_item *next_item=item_to_delete->next_item;

        //Liberar memoria del current
        free(item_to_delete);

        item_to_delete=next_item;
    }


    ay_player_playlist_total_elements=0;

    ay_player_first_item_playlist=NULL;

}

void ay_player_load_playlist(char *archivo_playlist)
{

    int longitud_archivo=get_file_size(archivo_playlist);

    z80_byte *buffer_temporal=util_malloc(longitud_archivo+1,"Can not allocate memory for reading playlist");

    int total_leidos=util_load_file_bytes((z80_byte *)buffer_temporal,archivo_playlist,longitud_archivo);

    if (total_leidos<=0) return;

    buffer_temporal[total_leidos]=0;


    //leer linea a linea
    char buffer_linea[PATH_MAX+1];

    char *mem=(char *)buffer_temporal;


    do {
        int leidos;
        char *next_mem;

        next_mem=util_read_line(mem,buffer_linea,total_leidos,PATH_MAX+1,&leidos);
        debug_printf(VERBOSE_INFO,"Reading playlist file, line: [%s]",buffer_linea);
        ay_player_playlist_add(buffer_linea);
        total_leidos -=leidos;


        mem=next_mem;

    } while (total_leidos>0);


    free(buffer_temporal);


}

//Guardar una playlist. Indica en marked_items si solo se guardan los items marcados (si valor no 0) o todos
void ay_player_save_playlist(char *destination_file,int marked_items,int append)
{


    FILE *ptr_destination_file;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;

    if (append) {
        if (zvfs_fopen_write_append(destination_file,&in_fatfs,&ptr_destination_file,&fil)<0) {
            debug_printf (VERBOSE_ERR,"Can not open %s",destination_file);
            return;
        }
    }

    else {

        if (zvfs_fopen_write(destination_file,&in_fatfs,&ptr_destination_file,&fil)<0) {
            debug_printf (VERBOSE_ERR,"Can not open %s",destination_file);
            return;
        }

    }


    //Recorrer toda la playlist
    ay_player_playlist_item *playitem=ay_player_first_item_playlist;

    while (playitem!=NULL) {

        int grabar=0;

        if (!marked_items) grabar=1;
        else {
            grabar=playitem->marcado;
        }

        if (grabar) {

            char nombre_archivo[PATH_MAX+1]; //+1 para el salto de linea

            sprintf(nombre_archivo,"%s\n",playitem->nombre);

            int longitud_linea=strlen(nombre_archivo);
            zvfs_fwrite(in_fatfs,(z80_byte *)nombre_archivo,longitud_linea,ptr_destination_file,&fil);
        }



        playitem=playitem->next_item;

    }

    zvfs_fclose(in_fatfs,ptr_destination_file,&fil);



}

void ay_player_playlist_get_item(int position,char *nombre)
{

    ay_player_playlist_item *current_item=ay_player_search_item(position);

    if (current_item==NULL) return;

    strcpy(nombre,current_item->nombre);


}

int ay_player_playlist_item_actual=0;



void ay_player_play_current_item(void)
{
    char nombre_archivo[PATH_MAX];
    ay_player_playlist_get_item(ay_player_playlist_item_actual,nombre_archivo);

    ay_player_load_and_play(nombre_archivo);
}

void ay_player_set_random_item(void)
{
    int total_elements=ay_player_playlist_get_total_elements();

    int valor_random=util_get_random();

    //Por si acaso evitar división por cero
    if (total_elements==0) ay_player_playlist_item_actual=0;
    else ay_player_playlist_item_actual=valor_random % total_elements;
}

//El primero o es el 0 o random
void ay_player_start_playing_all_items(void)
{
    ay_player_playlist_item_actual=0;

    //Modo random
    if (ay_player_shuffle_mode.v) {
        ay_player_set_random_item();
    }

    ay_player_play_current_item();
}

void ay_player_play_this_item(int item)
{
    if (item>=ay_player_playlist_get_total_elements() || item<0) ay_player_stop_player();
    else {
        ay_player_playlist_item_actual=item;
        ay_player_play_current_item();
    }
}

void ay_player_next_file(void)
{
    //play el siguiente en la playlist
    int total_elements=ay_player_playlist_get_total_elements();

    //Modo random
    if (ay_player_shuffle_mode.v) {
        ay_player_set_random_item();

        ay_player_play_current_item();

    }

    else {
        //Siguiente pista
        if (ay_player_playlist_item_actual<total_elements-1) {
            ay_player_playlist_item_actual++;
            ay_player_play_current_item();
        }

        //Llegado al final
        else {
            ay_player_stop_player();

            if (ay_player_exit_emulator_when_finish.v) end_emulator_autosave_snapshot();
        }

    }

    ayplayer_force_refresh=1;
}

void ay_player_previous_file(void)
{


    if (ay_player_playlist_item_actual>0) {
        ay_player_playlist_item_actual--;
    }

    ay_player_play_current_item();

    ayplayer_force_refresh=1;


}

void ay_player_add_file(char *archivo)
{

    ay_player_playlist_add(archivo);

    if (ay_player_playing.v==0) {
        //play el ultimo
        int total_elements=ay_player_playlist_get_total_elements();
        ay_player_playlist_item_actual=total_elements-1;
        ay_player_play_current_item();
    }

/*
*al agregar a playlist:
  si reproduciendose, no hacer nada
  si no reproduciendose, reproducir el siguiente
  */

}

void ay_player_next_track(void)
{

	if (audio_ay_player_mem==NULL) return;

    if (ay_player_pista_actual<ay_player_total_songs() ) {
        ay_player_pista_actual++;

    }

	else {
			if (ay_player_repeat_file.v) ay_player_pista_actual=1;
			else {
				ay_player_next_file();
				return;
			}
	}

	audio_ay_player_play_song(ay_player_pista_actual);

    ayplayer_force_refresh=1;
}


void ay_player_previous_track (void)
{

	if (audio_ay_player_mem==NULL) return;

	if (ay_player_pista_actual==1) ay_player_pista_actual=ay_player_total_songs();
	else ay_player_pista_actual--;

	audio_ay_player_play_song(ay_player_pista_actual);

    ayplayer_force_refresh=1;
}

//A los 10 segundos de silencio, saltar siguiente pista
#define AYPLAYER_SILENCE_MAX_COUNTER 10

//A donde se llama desde timer
void ay_player_playing_timer(void)
{
	if (audio_ay_player_mem==NULL) return;
	if (ay_player_playing.v==0) return;
    if (ay_player_paused.v) return;


	ay_song_length_counter++;


    //Si hay detector de silencio de ayplayer
    if (ay_player_silence_detection.v) {
        //printf("Contador silencio: %d\n",ay_player_silence_detection_counter);

        //ay_player_silence_detection_counter++;

       if (ay_player_silence_detection_counter>AYPLAYER_SILENCE_MAX_COUNTER*FRECUENCIA_SONIDO) {
        debug_printf(VERBOSE_INFO,"Jump to next track because there is silence");
        ay_player_next_track();
       }
    }

	//Si es infinito, no saltar nunca a siguiente cancion
	if (ay_song_length==0) return;

	//printf ("Contador cancion: %d limite: %d  (%d s)\n",ay_song_length_counter,ay_song_length,ay_song_length/50);

	if (ay_song_length_counter>ay_song_length+ay_player_add_to_track*50) {
		ay_player_next_track();
	}

}

void ay_player_stop_player(void)
{
	ay_player_playing.v=0;
	audio_ay_player_mem=NULL;

    ay_player_playlist_item_actual=0;

	//set_machine(NULL);

    //cargar la rom normal para eliminar el "invento" que se hace en la rom para poder cargar los archivos .ay
    //no interesa hacer un set_machine(NULL) porque eso borra layer de menu y acaba
    //borrando las ventanas, y no se refrescan

    rom_load(NULL);
	cold_start_cpu_registers();
	reset_cpu();
}

void ay_player_get_duration_current_song(z80_byte *minutos_total,z80_byte *segundos_total,z80_byte *decimas_segundos_total)
{
    *minutos_total=ay_song_length/60/50;
    *segundos_total=(ay_song_length/50)%60;
    *decimas_segundos_total=(ay_song_length % 50)*2; //1 frame de video=20ms=2 décimas de segundo
}



int ay_player_add_directory_playlist_alphasort(const struct dirent **d1, const struct dirent **d2)
{

	//printf ("menu_filesel_alphasort %s %s\n",(*d1)->d_name,(*d2)->d_name );

	//compara nombre
	return (strcasecmp((*d1)->d_name,(*d2)->d_name));
}

int ay_player_add_directory_playlist_filter_func(const struct dirent *d GCC_UNUSED)
{
    //d->d_name tiene el nombre sin el directorio
	int tipo_archivo=get_file_type((char *)d->d_name);

    //printf("tipo_archivo: %d name: %s\n",tipo_archivo,d->d_name);

	//Si no es archivo ni link, no ok

	if (tipo_archivo  == 0) {
        //printf("NOOK filesel is not a directory, file or link: %s\n",d->d_name);
		debug_printf (VERBOSE_DEBUG,"Item is not a directory, file or link");
		return 0;
	}

    if (!util_compare_file_extension((char *)d->d_name,"ay")) return 1;

    return 0;
}

void ay_player_add_directory_playlist(char *directorio)
{

    char directorio_actual[PATH_MAX];
    zvfs_getcwd(directorio_actual,PATH_MAX);

    zvfs_chdir(directorio);


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


    struct dirent **namelist;

	struct dirent *nombreactual;

    int n;

    // Si unidad actual es la mmc montada
    //if (fatfs_disk_zero_memory!=NULL)
    if (menu_current_drive_mmc_image.v)
    {
        //TODO: no he probado agregar una playlist que esté en imagen mmc
        n = menu_filesel_readdir_mmc_image(".", &namelist, ay_player_add_directory_playlist_filter_func, ay_player_add_directory_playlist_alphasort);
    }

    else {

#ifndef MINGW
	n = scandir(".", &namelist, ay_player_add_directory_playlist_filter_func, ay_player_add_directory_playlist_alphasort);
#else
	//alternativa scandir, creada por mi
	n = scandir_mingw(".", &namelist, ay_player_add_directory_playlist_filter_func, ay_player_add_directory_playlist_alphasort);
#endif

    }

    if (n < 0) {
		debug_printf (VERBOSE_ERR,"Error reading directory contents: %s",strerror(errno));
	}

    else {
        int i;

	//printf("total elementos directorio: %d\n",n);

        for (i=0;i<n;i++) {
            nombreactual=namelist[i];

            char directorio_actual[PATH_MAX];
            zvfs_getcwd(directorio_actual,PATH_MAX);
            char archivo_actual[PATH_MAX];

            sprintf(archivo_actual,"%s/%s",directorio_actual,nombreactual->d_name);

            debug_printf(VERBOSE_INFO,"Add AY file to playlist: %s",archivo_actual);

            ay_player_playlist_add(archivo_actual);

        }

		free(namelist);

    }

    //Restaurar directorio
    zvfs_chdir(directorio_actual);

}

void ay_player_get_elapsed_current_song(z80_byte *minutos,z80_byte *segundos,z80_byte *decimas_segundos)
{
    *minutos=ay_song_length_counter/60/50;
    *segundos=(ay_song_length_counter/50)%60;
    *decimas_segundos=(ay_song_length_counter % 50)*2; //1 frame de video=20ms=2 décimas de segundo
}


