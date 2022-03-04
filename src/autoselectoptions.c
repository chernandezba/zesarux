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

#include <string.h>
#include <stdio.h>

#include "autoselectoptions.h"
#include "debug.h"
#include "zx8081.h"
#include "cpu.h"
#include "screen.h"
#include "ay38912.h"
#include "joystick.h"
#include "tape.h"
#include "realjoystick.h"
#include "ulaplus.h"
#include "menu.h"
#include "utils.h"
#include "chardetect.h"

#ifdef COMPILE_STDOUT
	#include "scrstdout.h"
#endif

//Inicio funciones SSL
//Ya no se usan
/*
#ifdef COMPILE_SSL

#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL
#  include <CommonCrypto/CommonDigest.h>
#  define SHA1 CC_SHA1
#else
#  include <openssl/md5.h>
#endif

void return_md5sum_file(char *md5string,char *filename)
{
        int n;
        MD5_CTX c;
        char buf[512];

        int leidos;

        unsigned char out[MD5_DIGEST_LENGTH];

        FILE *ptr_file;

        MD5_Init(&c);

        //Open File
        ptr_file=fopen(filename,"rb");
        if (ptr_file==NULL) {
                md5string[0]=0;
                return;
        }

        do {

                leidos=fread(buf,1,512,ptr_file);
                if (leidos>0) MD5_Update(&c, buf, leidos);

        } while (leidos>0);

        MD5_Final(out, &c);

        for(n=0; n<MD5_DIGEST_LENGTH; n++)
                sprintf (&md5string[n*2],"%02x", out[n]);
                //printf("%02x", out[n]);

        md5string[n*2]=0;

        return;

}



#endif
//Fin funciones SSL
*/





//Para indicar que cinta/snapshot se ha detectado en la base de datos y se ha impreso en el footer el primer texto
int tape_options_set_first_message_counter=0;

//Para indicar que cinta/snapshot se ha detectado en la base de datos y se ha impreso en el footer el segundo texto
int tape_options_set_second_message_counter=0;

//Puntero a nombre del juego
char *mostrar_footer_game_name;

//Nombre del juego. donde se guarda el texto final
char texto_mostrar_footer_game_name[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH];

//texto mostrado en pantalla para primer mensaje.
//aunque en teoria deberia ser de 32, lo hacemos mayor porque se usa como buffer temporal y puede exceder 32
char mostrar_footer_first_message_mostrado[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH];

//texto entero para el primer mensaje
char mostrar_footer_first_message[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH];

//Indices de texto mostrado. Usados cuando hay que desplazar texto hacia la izquierda porque no cabe
int indice_first_message_mostrado,indice_second_message_mostrado;

//puntero a texto entero para el segundo mensaje
char *mostrar_footer_second_message;

//texto entero para el segundo mensaje
char texto_mostrar_footer_second_message[255];

//texto mostrado en pantalla para segundo mensaje.
//aunque en teoria deberia ser de 32, lo hacemos mayor porque se usa como buffer temporal y puede exceder 32
char mostrar_footer_second_message_mostrado[255];

void set_snaptape_filemachine_setreset(char *message,int machine)
{
                if (current_machine_type!=machine) {
                        debug_printf (VERBOSE_INFO,message);
                        current_machine_type=machine;
                        set_machine(NULL);
                        reset_cpu();
                }

}

//Auxiliar para obtener nombre e info del juego
//Separa las dos partes con un punto
//Si el juego tiene un . , se debe establecer manualmente mostrar_footer_game_name y mostrar_footer_game_info
void split_game_name_info(char *s)
{

	//split_game_name_info ("game with rainbow effects. Enabling Real Video");
	debug_printf (VERBOSE_INFO,"Detected %s",s);

	//Buscar hasta el primer punto
	int i=0;

	while (s[i]!=0 && s[i]!='.') {
		i++;
		//printf ("%d\n",i);
	}

	sprintf (texto_mostrar_footer_game_name,"%s",s);
	//Puntero apunta al buffer de texto
	mostrar_footer_game_name=texto_mostrar_footer_game_name;

	mostrar_footer_game_name[i]=0;

	//Hay info?
	if (s[i]!=0 && s[i+1]!=0) {
		//Si lo que hay es un espacio, avanzar a siguiente posicion
		if (s[i+1]==' ') i++;

		sprintf (texto_mostrar_footer_second_message,"%s",&s[i+1]);
		//Puntero apunta al buffer de texto
		mostrar_footer_second_message=texto_mostrar_footer_second_message;
	}

}

//seleccionar maquina en base a cinta o snapshot
//usado en quickload
void set_snaptape_filemachine(char *filename GCC_UNUSED)
{

	if (autoselect_snaptape_options.v==0) return;




//#ifdef COMPILE_SSL
//        char md5file[MD5_DIGEST_LENGTH*2+1];
//        return_md5sum_file(md5file,filename);
//        debug_printf (VERBOSE_INFO,"md5sum of file %s : %s",filename,md5file);
//#else
	char md5file[255];
	//Si no hay ssl, md5 nos lo inventamos...
	sprintf (md5file,"no ssl functions available");
//#endif



 
	/*

        if (!strcmp(md5file,"5b1c8a978c59983761558ce2f33635d5")) {
                set_snaptape_filemachine_setreset("Detected viewgiga. Resetting to 128k machine",6);
        }


        if (!strcmp(md5file,"7fdfef2ab78b1c2dc190d2ff1ce6a477")) {
                set_snaptape_filemachine_setreset("Detected Sgt. Helmet Zero. Resetting to 128k machine",6);
	}

	if (!strcmp(md5file,"b8a0cfa9ecad2c5e9590594f079679bd")) {
		set_snaptape_filemachine_setreset("Detected canciones. Resetting to +2A Spanish machine",13);
        }

	if (!strcmp(md5file,"5743d8482907b2ba1c068d60ee96a783")) {
		set_snaptape_filemachine_setreset("Detected sped52. Resetting to +2A Spanish machine",13);
        }


	if (!strcmp(md5file,"f6ac6979c60ff8b6a3d4f80c941082df")) {
		set_snaptape_filemachine_setreset("Detected listbasic. Resetting to +2A Spanish machine",13);
        }

        if (!strcmp(md5file,"65b6aa648c2b3dc6f9381d593980f86c")) {
		set_snaptape_filemachine_setreset("Detected MDA DEMO. Setting 128k machine",6);
        }

        if (!strcmp(md5file,"623e72f9b3365a7426966010622f0d00")) {
                set_snaptape_filemachine_setreset("Detected Song in Lines 5. Setting 128k machine",6);
        }


        if (!strcmp(md5file,"fb5e7d1726c145d14bb91b3cc5075c01")) {
                set_snaptape_filemachine_setreset("Detected Hercules. Setting 128k machine",6);
        }


        if (!strcmp(md5file,"bdb59a83d6984a7a4e6668c3b5ed772e")) {
                set_snaptape_filemachine_setreset("Detected Shock MegaDemo. Setting 48k machine",1);
        }


	if (!strcmp(md5file,"7f11eed294d66f6095d338fa40cdaa98")) {
                set_snaptape_filemachine_setreset("Detected Mescaline. Setting 128k machine",6);
        }


        if (!strcmp(md5file,"c3ba55396deef447a9e215e8b62b1651")) {
                set_snaptape_filemachine_setreset("Detected Animeeshon. Setting 128k machine",6);
        }


        if (!strcmp(md5file,"93baef584d99dd9a80866473df75c623")) {
                set_snaptape_filemachine_setreset("Detected Dark Apprehensions. Setting 128k machine",6);
        }


	if (!strcmp(md5file,"bae8f8007eb0da0f145c18966ee371c8")) {
                set_snaptape_filemachine_setreset("Detected Interlace. Setting 128k machine",6);
        }

	*/



}



//seleccionar opciones en base a cinta o snapshot
//Aqui se llama desde:
//-justo despues de insertar cinta
//-en carga snapshot, justo antes de cargar snapshot

//Que cosas se resetean al cargar cinta/snapshot?
//En set_machine_params (se llama cuando hay cambio de maquina o tambien desde smartload) se hace:
//video_zx8081_lnctr_adjust.v=0;
//video_zx8081_estabilizador_imagen.v=1;
//disable_wrx();
//zx8081_vsync_sound.v=0;
//minimo_duracion_vsync=DEFAULT_MINIMO_DURACION_VSYNC;

void set_snaptape_fileoptions(char *filename)
{

	if (autoselect_snaptape_options.v==0) return;

//#ifdef COMPILE_SSL
//	char md5file[MD5_DIGEST_LENGTH*2+1];
//	return_md5sum_file(md5file,filename);
//	debug_printf (VERBOSE_INFO,"md5sum of file %s : %s",filename,md5file);
//#else
	char md5file[255];
	//Si no hay ssl, md5 nos lo inventamos...
        sprintf (md5file,"no ssl functions available");
//#endif


	//Indices de desplazamiento a cero
	indice_first_message_mostrado=indice_second_message_mostrado=0;

	//Por defecto dejar los contadores a cero
	tape_options_set_first_message_counter=0;
	tape_options_set_second_message_counter=0;

	int mostrar_footer=1;
	mostrar_footer_game_name=NULL;
	mostrar_footer_second_message=NULL;

	//printf ("set_snaptape_fileoptions\n");

        //Aplicar configuracion segun si existe archivo .config
        char nombre_archivo_configuracion[PATH_MAX];
        sprintf (nombre_archivo_configuracion,"%s.config",filename);
        if (si_existe_archivo(nombre_archivo_configuracion)) {
                debug_printf (VERBOSE_INFO,"Parsing custom configuration file %s",nombre_archivo_configuracion);
                parse_custom_file_config(nombre_archivo_configuracion);
        }



	//
	// Cambios en joystick. Solo si joystick esta detectado
	//

	/*

		else if (!strcmp(md5file,"03f575af86144cde6efb7f2dba36c3bb") && realjoystick_present.v) {
        	        split_game_name_info ("Mazogs. Setting realjoystick buttons to keys. Setting joystick to none");
			realjoystick_clear_keys_array();
			realjoystick_copy_event_button_key(REALJOYSTICK_EVENT_UP,0,'w');
			realjoystick_copy_event_button_key(REALJOYSTICK_EVENT_DOWN,1,'x');
			realjoystick_copy_event_button_key(REALJOYSTICK_EVENT_LEFT,2,'a');
			realjoystick_copy_event_button_key(REALJOYSTICK_EVENT_RIGHT,3,'d');

			//faltan al menos L y R para poder jugar
			realjoystick_copy_event_button_key(REALJOYSTICK_EVENT_AUX1,4,'l');
			realjoystick_copy_event_button_key(REALJOYSTICK_EVENT_AUX2,5,'r');

			//Asignacion manual de numero de boton a letra. Desactivado
		        //realjoystick_keys_array[4].asignado.v=1;
		        //realjoystick_keys_array[4].button=4;
        		//realjoystick_keys_array[4].button_type=0;
		        //realjoystick_keys_array[4].caracter='l';

		        //realjoystick_keys_array[5].asignado.v=1;
		        //realjoystick_keys_array[5].button=5;
        		//realjoystick_keys_array[5].button_type=0;
	        	//realjoystick_keys_array[5].caracter='r';


			//establecemos joystick tipo ninguno, porque por defecto, esta en cursor+shift,
			//y si se mueve el joystick en este modo, se enviara tecla cursor + alguna de las 4 letras anteriores, y no funcionara
			joystick_emulation=0;

			mostrar_footer_game_name="Mazogs";
		}

		else if (!strcmp(md5file,"9cd5af6a9a9c61b2ea272f4f35058928") && realjoystick_present.v) {
	                split_game_name_info ("3d monster maze. Setting realjoystick buttons to keys. Setting joystick to cursor joystick");
			realjoystick_clear_keys_array();

			//letra 'c'
			realjoystick_copy_event_button_key(REALJOYSTICK_EVENT_AUX1,0,'c');
			//Asignacion manual de numero de boton a letra. Desactivado
	                //realjoystick_keys_array[0].asignado.v=1;
        	        //realjoystick_keys_array[0].button=4;
	                //realjoystick_keys_array[0].button_type=0;
        	        //realjoystick_keys_array[0].caracter='c';

			joystick_emulation=JOYSTICK_CURSOR;
		}

	*/


	//
	// Fin cambios en joystick
	//

/*

	else if (!strcmp(md5file,"0d7e4bbb6ffda3a80b138ff88c32af71")) {
		split_game_name_info ("ZX Music Interpreter. Enabling Tape Sound, Real Video");
		zx8081_vsync_sound.v=1;
		enable_rainbow();

	}


	else if (!strcmp(md5file,"2c48e1e90a1ed07ee251754e7a026cc2")) {
                split_game_name_info ("ZX Beatles. Enabling Tape Sound, Real Video");
                zx8081_vsync_sound.v=1;
		enable_rainbow();

        }

	else if (!strcmp(md5file,"9b88f41d8b67dd6bedf1d31972dcda7a")) {
                debug_printf (VERBOSE_INFO,"Detected OverScan Demo. Enabling Real Video and AY Chip");
		enable_rainbow();
		ay_chip_present.v=1;
		mostrar_footer_game_name="OverScan";
	}




	else if (!strcmp(md5file,"e24ed8361218b5905cf7a41f7db207be")) {
                split_game_name_info ("Arkanoid. Enabling Real Video (needed for idle bus)");
                enable_rainbow();
        }


	else if (!strcmp(md5file,"94da7c48e88dddb63f268b5eb12dd111") ||
	    !strcmp(md5file,"d6109cf492e2b6b64bd7d0e12c8ca800") ) {
		split_game_name_info ("BIFROST* ENGINE. Enabling Real Video");
                enable_rainbow();

        }

	else if (!strcmp(md5file,"13dda4aceaaddc32ebe2196b92dfc4fa")) {
		split_game_name_info ("ZXodus Engine. Enabling Real Video");
                enable_rainbow();
        }

	else if (!strcmp(md5file,"68f0bff47bd58c1d241d54076b4b99dc")) {
                split_game_name_info ("Knights & Demons DX. Enabling Real Video");
                enable_rainbow();
        }

        else if (!strcmp(md5file,"bdb59a83d6984a7a4e6668c3b5ed772e")) {
                split_game_name_info ("Shock MegaDemo. Enabling Real Video and AY Chip");
                enable_rainbow();
		ay_chip_present.v=1;
        }


	//Some rainbow games & demos

//7e5a02a5086c5f04c6331e44f4cfe28a  Bozxle.tap
//e6486e5842f71d4d9154ea14f84cfaa8  ColorPRINT48.tzx
//f25e4159a7a69f7113d856cc8a75265e  endlessforms0.tap
//b205fa5e8ec08d9cd0bf8a7b155befc3  RainbowProcessor.tap
//c6c70cf11e2e96cdbf4c09b4412630b3  Rotatrix (TK90X).tzx
//5e8f8f1456833c23dc625db2ed4db565  Rotatrix.tzx
//c3e7a660f4ce7b1f7dcdff39c5ce0ed9  STARTIP1.TAP
//dea011a55f3a629c61a88ec6369c88fc  thelosttapesofalbion0.tap
//64e5647478ff7c263e5e360a4e179db4  over128.z80
//9fa069661be0fc6d4ff22156a0268210  over48.z80
//053ee0857a91b0f4902a0b5b80dee375  BorderTrix.tap



	else if (
	!strcmp(md5file,"7e5a02a5086c5f04c6331e44f4cfe28a") ||
	!strcmp(md5file,"e6486e5842f71d4d9154ea14f84cfaa8") ||
	!strcmp(md5file,"f25e4159a7a69f7113d856cc8a75265e") ||
	!strcmp(md5file,"b205fa5e8ec08d9cd0bf8a7b155befc3") ||
	!strcmp(md5file,"c6c70cf11e2e96cdbf4c09b4412630b3") ||
	!strcmp(md5file,"5e8f8f1456833c23dc625db2ed4db565") ||
	!strcmp(md5file,"c3e7a660f4ce7b1f7dcdff39c5ce0ed9") ||
	!strcmp(md5file,"dea011a55f3a629c61a88ec6369c88fc") ||
        !strcmp(md5file,"64e5647478ff7c263e5e360a4e179db4") ||
        !strcmp(md5file,"9fa069661be0fc6d4ff22156a0268210") ||
	!strcmp(md5file,"053ee0857a91b0f4902a0b5b80dee375")
	) {
		split_game_name_info ("Game with rainbow effects. Enabling Real Video");
                enable_rainbow();
        }



	else if (!strcmp(md5file,"04c6a5e9d8c1834d171352942cc34a8e")) {
                split_game_name_info ("Super Wonder Boy. Enabling Real Video");
                enable_rainbow();
        }


	else if (!strcmp(md5file,"449435fa4b9bfdf7e7a24f0d7d574302")) {
                split_game_name_info ("CAC10. Enabling Real Video");
                enable_rainbow();
	}

	else if (!strcmp(md5file,"00768c4874441fabefbf7cab38b10818")) {
                split_game_name_info ("Defenda (Quicksilva). Enabling Real Video and Quicksilva QS Sound board.");
		enable_rainbow();
		ay_chip_present.v=1;
	}


	else if (!strcmp(md5file,"35c6d91970899efe6417c3f27ed29dd0") ||
	    !strcmp(md5file,"e6400944886fdd7cbbdc7b84d954b628")) {

		split_game_name_info ("Toddy Forth. Enabling Real Video, Bi-Pak ZON-X81 Sound, RAM Pack on 2000H, 8000H and C000H");
                enable_rainbow();
                ay_chip_present.v=1;
                ram_in_8192.v=1;
                enable_ram_in_49152();

	}

	else if (!strcmp(md5file,"3a912c2dececc98a57fbac9072999524")) {
		//Un ejemplo de juego que no establece nombre ni info
                debug_printf (VERBOSE_INFO,"Detected aydemo. Enabling Bi-Pak ZON-X81 Sound");
                ay_chip_present.v=1;
        }

	else if (!strcmp(md5file,"079bd9dd140b03e26b2e8ca3bd0f403a")) {
		//Un ejemplo de juego que solo establece nombre pero no info
                debug_printf (VERBOSE_INFO,"Detected Dancing Demon. Enabling Bi-Pak ZON-X81 Sound");
                ay_chip_present.v=1;
		mostrar_footer_game_name="Dancing Demon";
        }


	else if (!strcmp(md5file,"638260d62acc4c82d8ec206168815da2")) {
		//Un ejemplo de juego que solo establece nombre pero no info, usando funcion split_game_name_info
                split_game_name_info ("Pink Panther.");
                ay_chip_present.v=1;
        }




	else if (!strcmp(md5file,"56f211a86c6ce8097d027c7a4e581c5e")) {
                split_game_name_info ("Asteroids (Quicksilva). Enabling Quicksilva QS Sound board");
                ay_chip_present.v=1;
        }


	else if (!strcmp(md5file,"a2f7022fe77f9de3b20e945b8c880b50")) {
                split_game_name_info ("Breakout (Macronics 1980). Enabling Real Video");
                enable_rainbow();
        }

	else if (!strcmp(md5file,"5fe93279e662fdb120aea9661a389c90")) {
                split_game_name_info ("Double Breakout (Macronics 1980). Enabling Real Video");
                enable_rainbow();
        }



	else if (!strcmp(md5file,"f79bab280162d997f8817bb49b22187c")) {
                split_game_name_info ("Super Invasion 1K (Beam Software 1981). Enabling Real Video");
                enable_rainbow();
        }

	else if (!strcmp(md5file,"41708156d0a7558bd6cf26d61b8bb910")) {
                split_game_name_info ("Super Invasion 2K (Beam Software 1981). Enabling Real Video");
                enable_rainbow();
        }

	else if (!strcmp(md5file,"ff3524e2ba0e77095b4084e966e36dbb")) {
                split_game_name_info ("Space Invaders 1K (Macronics 1981) Reconstruction. Enabling Real Video.");
                enable_rainbow();
        }

	else if (!strcmp(md5file,"d2188bc44493e4d89179c56e96c5943f")) {
                split_game_name_info ("Space Invaders 3K (Macronics 1981). Enabling Real Video.");
                enable_rainbow();
        }

	else if (!strcmp(md5file,"0c796f61f0db896e708f789196bf9263")) {
                split_game_name_info ("SOUND. Enabling Tape Sound, Real Video");
                zx8081_vsync_sound.v=1;
		enable_rainbow();
        }

	else if (!strcmp(md5file,"6097b80383b6396b49c898e972d079e4")) {
                split_game_name_info ("Orquesta. Enabling Tape Sound, Real Video");
                zx8081_vsync_sound.v=1;
                enable_rainbow();


        }


	else if (!strcmp(md5file,"9567ffe4588ef4b7935076af3faef2fe")) {
                split_game_name_info ("Orquesta rwa. Enabling Tape Sound, Real Video");
                zx8081_vsync_sound.v=1;
                enable_rainbow();

	}


	else if (!strcmp(md5file,"b8f10ccea1713a53ed3e4def694ee725")) {
                split_game_name_info ("MAGIC FLOOR. Enabling Real Video, WRX");
                enable_rainbow();
                enable_wrx();
        }

	else if (!strcmp(md5file,"0bcaf318a48c26a9a056dc45d00c79dd")) {
                split_game_name_info ("JBRACING. Enabling Real Video, WRX");
                enable_rainbow();
                enable_wrx();
        }


        else if (!strcmp(md5file,"310bb9c4718be8e78553da09aa4949cf")) {
                split_game_name_info ("BEAMRIDER. Enabling Real Video, WRX, RAM in 8000H");
                enable_rainbow();
                enable_wrx();
		enable_ram_in_32768();
        }

	//Ejemplo de juego con . en el nombre
        else if (!strcmp(md5file,"d3f70909961601f6120e9e22d51f8636")) {
		debug_printf (VERBOSE_INFO,"Detected game H.E.R.O. Enabling Real Video, WRX, RAM in 8000H and C000H. Changing vsync minimum length");

		mostrar_footer_game_name="H.E.R.O.";
		mostrar_footer_second_message="Enabling Real Video, WRX, RAM in 8000H and C000H. Changing vsync minimum length";

                enable_rainbow();
                enable_wrx();
		enable_ram_in_49152();
		minimo_duracion_vsync=160;
        }



        else if (!strcmp(md5file,"5c15fc10d644eaa439a264d39d27898e")) {
                split_game_name_info ("STARFIGHT. Enabling Real Video, WRX");
                enable_rainbow();
                enable_wrx();
        }




	else if (!strcmp(md5file,"d018bb60f78caad93f51b2f97fa54233")) {
                split_game_name_info ("hr invaders. Enabling Real Video, WRX, RAM Pack, Joystick Zebra");
		enable_rainbow();
                ram_in_8192.v=1;
                enable_wrx();
		joystick_emulation=JOYSTICK_ZEBRA;

	}

        else if (!strcmp(md5file,"55d1b367c05a5f9259da78aad2b072da")) {
                split_game_name_info ("MikroGen Frogs. Setting joystick MikroGen");
		joystick_emulation=JOYSTICK_MIKROGEN;
	}

	else if (!strcmp(md5file,"a1495a5a88c593c9c8796762eb805153")) {
                split_game_name_info ("rezurrection. Enabling Real Video, WRX, RAM Pack, disabling horizontal stabilization");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_wrx();
		video_zx8081_estabilizador_imagen.v=0;
		//offset_zx8081_t_estados=3;
        }

	else if (!strcmp(md5file,"37258e692b0d21c0ce4149553a134e72")) {
                split_game_name_info ("Wall_wrx. Enabling Real Video, WRX");
                enable_rainbow();
                enable_wrx();
                //offset_zx8081_t_estados=1;
        }



	else if (!strcmp(md5file,"dfed160e05e728f78e98f95be0b13442")) {
                split_game_name_info ("ZX-PAINT. Enabling Real Video, WRX, RAM Pack on 2000H and 8000H");
                enable_rainbow();
                ram_in_8192.v=1;
		enable_ram_in_32768();
                enable_wrx();
        }

	else if (!strcmp(md5file,"00e5f41cdbb8c6e31fa1f6eed3baf2c8")) {
                split_game_name_info ("FSCAPES. Enabling Real Video, WRX, RAM Pack on 2000H, 8000H and C000H");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_ram_in_49152();
                enable_wrx();

        }

	else if (!strcmp(md5file,"4dce9f4d2fc7d69cf59c369938f61de8")) {
                split_game_name_info ("FOURIER. Enabling Real Video, WRX, RAM Pack on 2000H and 8000H");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_ram_in_32768();
                enable_wrx();
        }

	else if (!strcmp(md5file,"6c5f01f907bac2e82a02391cd5e30179")) {
                split_game_name_info ("Voyage of Peril. Enabling Real Video, WRX, RAM Pack on 2000H and 8000H");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_ram_in_32768();
                enable_wrx();

        }

	else if (!strcmp(md5file,"252e10e0ac890abf4045fa467d1a5cb8")) {
                split_game_name_info ("3DFrac. Enabling Real Video, WRX, RAM Pack on 2000H and 8000H");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_ram_in_32768();
                enable_wrx();
        }

	else if (!strcmp(md5file,"b284c430067c7350dce09ac74e15c4db")) {
                split_game_name_info ("Biplot. Enabling Real Video, WRX, RAM Pack");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_wrx();
		video_zx8081_lnctr_adjust.v=1;

        }




	//some other wrx
        else if (!strcmp(md5file,"b284c430067c7350dce09ac74e15c4db") ||
	    !strcmp(md5file,"bd5bfd9a24dec98e56cad48804cd7684") ||
            !strcmp(md5file,"1eb79331ab9a975602149bf324cf1604")


	) {
                split_game_name_info ("Game with WRX. Enabling Real Video, WRX, RAM Pack");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_wrx();
        }





//8ec7b8cd5c682bbfd4fa1fc5b29d0255  tapes/zx81/hires/wrx/NU3.P
//cfdf935dde293c85766d0f5f735ef253  tapes/zx81/hires/wrx/NU4.P
//d9c5e4b08ae79869fe8898529209c061  tapes/zx81/hires/wrx/NU5.P
//ebc6ac140491b9662a19cbba06e63886  tapes/zx81/hires/wrx/NU9.P

	else if (
	!strcmp(md5file,"8ec7b8cd5c682bbfd4fa1fc5b29d0255") ||
	!strcmp(md5file,"cfdf935dde293c85766d0f5f735ef253") ||
	!strcmp(md5file,"d9c5e4b08ae79869fe8898529209c061") ||
	!strcmp(md5file,"ebc6ac140491b9662a19cbba06e63886")
	) {
                split_game_name_info ("Set NU*. Enabling Real Video, WRX");
                enable_rainbow();
                enable_wrx();
		//offset_zx8081_t_estados=1;
        }


	else if (!strcmp(md5file,"1d872460b37591bf6ddcd3b18575cd43")) {
                split_game_name_info ("Nuclear Invaders (WRX). Enabling Real Video, Bi-Pak ZON-X81 Sound, WRX, other hacks");
                enable_rainbow();
		ay_chip_present.v=1;
                enable_wrx();
		//offset_zx8081_t_estados=1;
        }



	else if (!strcmp(md5file,"162128d2a1b8478e602eea20dbc57a1e")) {
                split_game_name_info ("PokerHR. Enabling Real Video, RAM Pack (Uses UDG)");
                enable_rainbow();
                ram_in_8192.v=1;
        }



	else if (!strcmp(md5file,"94ab7863916860f39906a943c830df5c")) {
                split_game_name_info ("HiRes Galaxian. Enabling Real Video, RAM Pack (Uses UDG)");
                enable_rainbow();
                ram_in_8192.v=1;
        }

	else if (!strcmp(md5file,"799841a9ac4d549e9a5fc75733957d32")) {
                split_game_name_info ("Airport HR. Enabling Real Video, RAM Pack (Uses UDG)");
                enable_rainbow();
                ram_in_8192.v=1;
        }

	else if (!strcmp(md5file,"3bff922d9aaf6d95d2c0ca6f0588d3bf")) {
                split_game_name_info ("interc-u.p. Enabling Real Video, RAM Pack (Uses UDG)");
                enable_rainbow();
                ram_in_8192.v=1;
        }



	else if (!strcmp(md5file,"318ccb1ef3e937807b84644c7f696d3b")) {
                split_game_name_info ("Noice-SomewhatLessLimitedCapabilities. Enabling Real Video, enabling lnctr video adjust");
                enable_rainbow();
                video_zx8081_lnctr_adjust.v=1;
        }

	else if (!strcmp(md5file,"34a9c4af6022c59a9f961580b065ef64")) {
                split_game_name_info ("REVENGE. Enabling Real Video, enabling lnctr video adjust");
                enable_rainbow();
                video_zx8081_lnctr_adjust.v=1;
        }

	else if (!strcmp(md5file,"5de8a521ebfde9a8ca35e3a047da73f7")) {
                split_game_name_info ("CRUSH. Enabling Real Video, enabling lnctr video adjust");
                enable_rainbow();
                video_zx8081_lnctr_adjust.v=1;
        }

	else if (!strcmp(md5file,"cb03d99b26b0d04336ded001595ce3c4")) {
                split_game_name_info ("BLOCKBUSTER. Enabling Real Video, WRX, RAM pack");
                enable_rainbow();
		enable_wrx();
		ram_in_8192.v=1;
        }


	else if (!strcmp(md5file,"02d1ab0cf7d0251c12dafa9838d84af5")) {
                split_game_name_info ("No Limits demo. Enabling Real Video, enabling lnctr video adjust");
                enable_rainbow();
                video_zx8081_lnctr_adjust.v=1;
        }



	else if (!strcmp(md5file,"202d2fe74dbd7c72ee0d6acff6bc3cb1")) {
                split_game_name_info ("Spirograph. Enabling Real Video, WRX, RAM Pack");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_wrx();
		//video_zx8081_decremento_x_cuando_mayor=16;
        }

	else if (!strcmp(md5file,"e96ab74436dfa4be1716099b5883a828")) {
                split_game_name_info ("Julia Sets. Enabling Real Video, WRX, RAM Pack");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_wrx();
        }



	else if (!strcmp(md5file,"847e65f95303da26bd401826a8e542e0")) {
                split_game_name_info ("MAND-FFP. Enabling Real Video, WRX, RAM Pack");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_wrx();
        }

	else if (!strcmp(md5file,"490534dd9b8a73fb62be44072e91aa54")) {
                split_game_name_info ("MACRO-Life 16. Enabling Real Video, WRX, RAM Pack");
                enable_rainbow();
                ram_in_8192.v=1;
                enable_wrx();
		//video_zx8081_decremento_x_cuando_mayor=16;
        }



	else if (!strcmp(md5file,"e9a2b79b316f09a7a01adc09b3585072")) {
                split_game_name_info ("wrx1k1. Enabling Real Video, WRX");
                enable_rainbow();
                enable_wrx();
        }

	else if (!strcmp(md5file,"e369469f3944bafd48cea670829771b2")) {
                split_game_name_info ("HRDEMO3. Enabling Real Video, WRX, RAM Pack");
                enable_rainbow();
                enable_wrx();
		ram_in_8192.v=1;
        }

	else if (!strcmp(md5file,"16feff02683324ee0d0dd827e39da415")) {
                split_game_name_info ("ZXBART. Enabling Real Video, WRX");
                enable_rainbow();
                enable_wrx();
		//offset_zx8081_t_estados=1;
        }


	else if (!strcmp(md5file,"493817d98b6d96356df1e1c3f93866e9")) {
                split_game_name_info ("HighRes. Enabling Real Video");
                enable_rainbow();
		video_zx8081_lnctr_adjust.v=1;
        }

	else if (!strcmp(md5file,"2e4901caa2e103cf08f074c296fb30a1")) {
                split_game_name_info ("HighResolution A1. Enabling Real Video, enabling lnctr video adjust");
		enable_rainbow();
		video_zx8081_lnctr_adjust.v=1;
	}


	else if (!strcmp(md5file,"95323ed253c423a78e34b3064440dfb7")) {
                split_game_name_info ("Sunbucket. Enabling Real Video");
                enable_rainbow();

        }

	else if (!strcmp(md5file,"3c9a8d64d659fbcdf196aa3fa45c2c74")) {
                split_game_name_info ("Nirvana Engine. Enabling Real Video");
                enable_rainbow();

        }


	else if (!strcmp(md5file,"d85c1aa060b2e15218653555a1ef4046")) {
                split_game_name_info ("El Stompo. Enabling Real Video");
                enable_rainbow();
        }


	else if (!strcmp(md5file,"d4a16c48c6d9b59ad23f285d54da3057") ||
	    !strcmp(md5file,"028b6fc0a4014d9bf8205275289ddd0d")) {
                split_game_name_info ("Dreamwalker. Enabling Real Video");
                enable_rainbow();
        }


//65b6aa648c2b3dc6f9381d593980f86c  tapes/spectrum/rainbow/MDA_DEMO.TAP

	else if (!strcmp(md5file,"65b6aa648c2b3dc6f9381d593980f86c")) {
		split_game_name_info ("MDA DEMO. Enabling Real Video");
		enable_rainbow();
	}


        else if (!strcmp(md5file,"623e72f9b3365a7426966010622f0d00")) {
                split_game_name_info ("Song in Lines 5. Enabling Real Video");
                enable_rainbow();

        }


        else if (!strcmp(md5file,"fb5e7d1726c145d14bb91b3cc5075c01")) {
                split_game_name_info ("Hercules. Enabling Real Video");
                enable_rainbow();
        }



	else if (!strcmp(md5file,"20f8175d7b08e23d94167d6003b3a16f")) {
                split_game_name_info ("Target Plus. Enabling Gunstick - Kempston");
		gunstick_emulation=GUNSTICK_KEMPSTON;
	}

	else if (!strcmp(md5file,"1edee139362708bbbeeffa123f046fe8")) {
                split_game_name_info ("Bestial Warrior. Enabling Gunstick - Kempston");
                gunstick_emulation=GUNSTICK_KEMPSTON;
        }

	//programas que usan magnum light phaser

//6bf66b0972ca6f91412981d96cf73116  American Turbo King (1989)(Mastertronic)[Lightgun].tap
//6c9958a804f352eb42bf431e07bb5484  Billy The Kid - Lightgun (1989)(Mastertronic).tap
//59859057a5376c890c5cc2e617157e0f  Bronx Street Cop - Joystick (1989)(Mastertronic).tap
//aade525753c4572d11e10af9cb344e18  SINCLACT.TAP
//051c1e304acd4d08c3d7cc9fd60efa2b  Sinclair Action Pack - Side A.tzx
//7d30da869544aed29181749bcbdf16b0  Sinclair Action Pack - Side B.tzx

	else if (
	!strcmp(md5file,"6bf66b0972ca6f91412981d96cf73116") ||
	!strcmp(md5file,"6c9958a804f352eb42bf431e07bb5484") ||
	!strcmp(md5file,"59859057a5376c890c5cc2e617157e0f") ||
	!strcmp(md5file,"15791c249f842589fa279dfd1f382e72") ||
	!strcmp(md5file,"aade525753c4572d11e10af9cb344e18") ||
	!strcmp(md5file,"051c1e304acd4d08c3d7cc9fd60efa2b") ||
	!strcmp(md5file,"7d30da869544aed29181749bcbdf16b0")

	) {
                split_game_name_info ("Game that uses Magnum Light Phaser - AYCHIP");
                gunstick_emulation=GUNSTICK_AYCHIP;
		//mejor con realvideo
		enable_rainbow();
	}


	else if (!strcmp(md5file,"15791c249f842589fa279dfd1f382e72")) {
                split_game_name_info ("Jungle Warfare. Uses Magnum Light Phaser - AYCHIP only with bright");
		gunstick_emulation=GUNSTICK_AYCHIP;
		gunstick_solo_brillo=1;
		//mejor con realvideo
		enable_rainbow();
	}

	else if (!strcmp(md5file,"6c9561f56463ca1f67d1c0aa9e0f6131")) {
                split_game_name_info ("QS Armoury. Uses Magnum Light Phaser - AYCHIP, offset Y");
		gunstick_emulation=GUNSTICK_AYCHIP;
		gunstick_range_x=32;
		gunstick_range_y=8;
		//gunstick_y_offset=24;
		//mejor con realvideo
		enable_rainbow();
	}



	else if (!strcmp(md5file,"fc3891eaefa94544c01aa2b975df1811")) {
                split_game_name_info ("Solo. Enabling Gunstick - Sinclair 1");
                gunstick_emulation=GUNSTICK_SINCLAIR_1;
	}

	else if (!strcmp(md5file,"a294d38a0c44c11afb135ebc1d79e8e6")) {
                split_game_name_info ("Trigger. Enabling Gunstick - Sinclair 1");
                gunstick_emulation=GUNSTICK_SINCLAIR_1;
        }


//064792146542ada54d8177fa36f25d39  tapes/spectrum/mouse/kempston_mouse_interface_driver.z80
//9f26f8265931eca1d713142676276d03  tapes/spectrum/mouse/The OCP Art Studio (Datel) - Side A.tzx
//f09b92fc3fa4e58aa21f0a029a73e308  tapes/spectrum/mouse/The OCP Art Studio (Datel) - Side B.tzx



	//programas que usan kempston mouse
	else if (
        !strcmp(md5file,"064792146542ada54d8177fa36f25d39") ||
        !strcmp(md5file,"9f26f8265931eca1d713142676276d03") ||
        !strcmp(md5file,"f09b92fc3fa4e58aa21f0a029a73e308")
	) {
		split_game_name_info ("program that uses Kempston Mouse. Enabling it");
		kempston_mouse_emulation.v=1;
	}

	//Traps para juegos de texto
#ifdef COMPILE_STDOUT
	else if (
	!strcmp(md5file,"5a39494d370da1b2a653856c8d52d2dc") ||
	!strcmp(md5file,"24b545b9a62d692add2b38446ceaa1ee") ||
	!strcmp(md5file,"126b9236415a72aaf95870217c550454")
	) {

                split_game_name_info ("La Aventura Original. Setting stdout second trap address");
		chardetect_second_trap_char_dir=31103;
		trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
		chardetect_line_width=42;
		chardetect_char_filter=CHAR_FILTER_AD42;
		chardetect_line_width_wait_space.v=1;
	}

	else if (
	!strcmp(md5file,"007de995e26d68bdcbf4062892d40ee5") ||
	!strcmp(md5file,"24e2bfa51d56ff5b046961de28ba1cbf") ||
	!strcmp(md5file,"ad554d6625f5e9f4064c041b604593c7")
	) {
                split_game_name_info ("La Diosa de Cozumel. Setting stdout second trap address");
                chardetect_second_trap_char_dir=30974;
		trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
		chardetect_line_width=42;
		chardetect_char_filter=CHAR_FILTER_AD42;
		chardetect_line_width_wait_space.v=1;
	}

	else if (
	!strcmp(md5file,"a3f96f06d46921a3ed676363accbbb4f") ||
	!strcmp(md5file,"134214780aafda49411bb61bc31b63e2")
	) {
		split_game_name_info ("Chichen Itza. Setting stdout second trap address");
                chardetect_second_trap_char_dir=30548;
		trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
		chardetect_line_width=42;
		chardetect_char_filter=CHAR_FILTER_AD42;
		chardetect_line_width_wait_space.v=1;
        }

	//templos sagrados misma direccion que chichen itza
        else if (
        !strcmp(md5file,"dfc1e4b50a7b0f4193380aeab7c0b961") ||
        !strcmp(md5file,"b453cc07d1a1f1fe7a993cbe8257302f")
        ) {
                split_game_name_info ("Los templos sagrados. Setting stdout second trap address");
                chardetect_second_trap_char_dir=30548;
		trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
		chardetect_line_width=42;
		chardetect_char_filter=CHAR_FILTER_AD42;
		chardetect_line_width_wait_space.v=1;
        }


	else if (!strcmp(md5file,"dafbd989acae0efe46875833700a1dfc")) {
		split_game_name_info ("Jabato. Setting stdout second trap address");
                chardetect_second_trap_char_dir=31125;
		trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
                chardetect_line_width=42;
		chardetect_char_filter=CHAR_FILTER_AD42;
		chardetect_line_width_wait_space.v=1;
        }



*/



	//Hobbit v1.0. tzx. utiliza dos traps
/*        else if (
        !strcmp(md5file,"957e4e8be02ad8d1d36dbeeefa6fc8bd") ||
        !strcmp(md5file,"84228f97e581e5bd730587bbfa04483d") ||
	!strcmp(md5file,"3a09fe06a679644738b2d20470df9912")



        ) {*/
	/*
		second_trap entra como common 1:
                PUSH AF
                PUSH BC
                PUSH DE
                PUSH HL
                LD L,A
                LD H,0
                ADD HL,HL
                ADD HL,HL
                ADD HL,HL
                LD DE,XXXX
                ADD HL,DE



		third_trap trap es: (no asignado a ninguna rutina de deteccion, dado que es un tercer trap, y no esta soportado en autodeteccion)
		PUSH AF
		PUSH BC
		PUSH DE
		PUSH HL
		SUB 20H
		LD L,A
		LD H,0
		ADD HL,HL
		ADD HL,HL
		ADD HL,HL
		LD DE,3D00H
		ADD HL,DE
	*/
/*
                split_game_name_info ("The Hobbit v10. Setting stdout second and third trap address");
                chardetect_second_trap_char_dir=30652;
                chardetect_third_trap_char_dir=30317;
                chardetect_line_width=42;
		trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
		chardetect_char_filter=CHAR_FILTER_HOBBIT;
		chardetect_line_width_wait_space.v=1;
        }

	//Hobbit v1.2
	else if (
        !strcmp(md5file,"3c359e2e8618e371cf385b9302bfe687") ||
        !strcmp(md5file,"3cf6e2b08ff52146b9850a51f431f80f")

        ) {
                split_game_name_info ("The Hobbit v12. Setting stdout second and third trap address");
                chardetect_second_trap_char_dir=34761;
                chardetect_third_trap_char_dir=34426;
                chardetect_line_width=42;
                trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
                chardetect_char_filter=CHAR_FILTER_HOBBIT;
                chardetect_line_width_wait_space.v=1;
        }


#endif



	//Inves ULA Out port test
        else if (!strcmp(md5file,"dff9f35491af3db8c8a5b3faf1ad85f0")) {
                split_game_name_info ("Inves ULA Out port test. Enabling Real Video");
                enable_rainbow();
        }



	else if (!strcmp(md5file,"41e787acc20178e5901e7a6eac13ec3a")) {
                split_game_name_info ("Hires interlaced. Enabling Real Video and Interlace");
		enable_interlace();
	}


	else if (!strcmp(md5file,"7667b4a37940c3f7c567a3b8671c9eaf")) {
                split_game_name_info ("Interlace Test. Enabling Real Video and Interlace");
                enable_interlace();
        }

	else if (!strcmp(md5file,"bae8f8007eb0da0f145c18966ee371c8")) {
                split_game_name_info ("Demo Interlace. Enabling Real Video and Interlace");
                enable_interlace();
        }


        else if (!strcmp(md5file,"7f11eed294d66f6095d338fa40cdaa98")) {
		split_game_name_info ("Mescaline. Enabling Real Video and Gigascreen");
		enable_gigascreen();
        }

	else if (!strcmp(md5file,"ec6a3b4b712b5400a36a65c1c48103e0")) {
                split_game_name_info ("Paralactika. Enabling Real Video and Gigascreen");
                enable_gigascreen();
        }

	else if (!strcmp(md5file,"5b1c8a978c59983761558ce2f33635d5")) {
                split_game_name_info ("viewgiga. Enabling Real Video and Gigascreen");
                enable_gigascreen();
        }


	else if (!strcmp(md5file,"c3ba55396deef447a9e215e8b62b1651")) {
                split_game_name_info ("Animeeshon. Enabling Real Video and Gigascreen");
                enable_gigascreen();
		        }


        else if (!strcmp(md5file,"93baef584d99dd9a80866473df75c623")) {
                split_game_name_info ("Dark Apprehensions. Enabling Real Video and Gigascreen");
                enable_gigascreen();
		        }


	else if (!strcmp(md5file,"3c777acda39438a3aade54c783a9ba2c")) {
                split_game_name_info ("Gigascreen test. Enabling Real Video and Gigascreen");
                enable_gigascreen();
        }

*/


	else {
		debug_printf (VERBOSE_INFO,"No detected options for %s",filename);
		//No se ha detectado cinta/snap a cargar
		mostrar_footer=0;
		//Si habia algun texto en footer, borrarlo
		menu_putstring_footer(0,2,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
		menu_footer_bottom_line();
	}


	if (mostrar_footer) {
		//mostrar_footer_game_name contendra el nombre del juego
		//mostrar_footer_first_message contiene el texto entero para el primer mensaje
		//mostrar_footer_first_message_mostrado contiene el texto entero para el primer mensaje (maximo 32)
		if (mostrar_footer_game_name!=NULL) {

			//Si hay segundo mensaje, ultimo caracter es ":"
			if (mostrar_footer_second_message!=NULL) {
				sprintf (mostrar_footer_first_message,"Applied options for %s:",mostrar_footer_game_name);
			}

			else {
				sprintf (mostrar_footer_first_message,"Applied options for %s",mostrar_footer_game_name);
			}

		}
		else {
			sprintf (mostrar_footer_first_message,"Applied options for known game");
		}

		//Texto mostrado
		sprintf (mostrar_footer_first_message_mostrado,"%s",mostrar_footer_first_message);

		//Cortar a 32
		tape_options_corta_a_32(mostrar_footer_first_message_mostrado);

		tape_options_set_first_message_counter=4;

		autoselect_options_put_footer();

		
	}


}


void put_footer_first_message(char *mensaje)
{

	//Indices de desplazamiento a cero
	indice_first_message_mostrado=indice_second_message_mostrado=0;

	//Por defecto dejar los contadores a cero
	tape_options_set_first_message_counter=0;
	tape_options_set_second_message_counter=0;

        strcpy(mostrar_footer_first_message,mensaje);
       

		//Texto mostrado
		sprintf (mostrar_footer_first_message_mostrado,"%s",mostrar_footer_first_message);

		//Cortar a 32
		tape_options_corta_a_32(mostrar_footer_first_message_mostrado);

		tape_options_set_first_message_counter=4;

		autoselect_options_put_footer();

		
	
}

//Pone el footer segun lo que corresponda, si primer mensaje o segundo mensaje activo
void autoselect_options_put_footer(void)
{
	if (tape_options_set_first_message_counter!=0) {

                //De momento borrar lo que haya
                menu_putstring_footer(0,2,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

                //Y mostrarlo
                menu_putstring_footer(0,2,mostrar_footer_first_message_mostrado,WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);

		return;
	}

	if (tape_options_set_second_message_counter!=0) {

                //De momento borrar lo que haya
                menu_putstring_footer(0,2,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

		//Y mostrarlo
                menu_putstring_footer(0,2,mostrar_footer_second_message_mostrado,WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);

		return;
	}

}


void tape_options_corta_a_32(char *s)
{
	                int longitud_mensaje=strlen(s);
                        //Ver si longitud excede 32
                        if (longitud_mensaje>32) {
                                s[32]=0;
                        }

}

//Desplazar texto mostrado
void desplaza_texto(int *indice, char *cadena_entera, char *cadena_mostrada)
{

	int i=*indice;
	i++;

	*indice=i;
	//printf ("indice: %d\n",i);
	//Y copiamos 32 caracteres
	sprintf (cadena_mostrada,"%s",&cadena_entera[i]);
	cadena_mostrada[32]=0;
}


//Primer mensaje ya ha pasado 4 segundos en pantalla. Desplazar si no ha cabido entero o borrarlo
void delete_tape_options_set_first_message(void)
{
	//De momento borrar lo que haya
	menu_putstring_footer(0,2,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);


        //Hacer que el first message se desplace a la izquierda si no ha cabido entero
        int longitud_mostrada=strlen(mostrar_footer_first_message_mostrado);
        int longitud_real=strlen(&mostrar_footer_first_message[indice_first_message_mostrado]);
        if (longitud_real>longitud_mostrada) {
			desplaza_texto(&indice_first_message_mostrado,mostrar_footer_first_message,mostrar_footer_first_message_mostrado);

                        //Metemos contador a 1 segundo
                        tape_options_set_first_message_counter=1;

			autoselect_options_put_footer();

			

                        return;
	}



	//No hay que desplazar primer mensaje. Mostrar segundo mensaje, si hay
	if (mostrar_footer_second_message!=NULL) {
		sprintf (mostrar_footer_second_message_mostrado,"%s",mostrar_footer_second_message);
		tape_options_corta_a_32(mostrar_footer_second_message_mostrado);

		tape_options_set_second_message_counter=4;

		autoselect_options_put_footer();

	

	}

	//No hay segundo mensaje
	else {

		//Aunque para Z88 no se llama a funcion de set_snaptape_fileoptions, y no haria falta repintar zona footer Z88,
		//lo hacemos por si en un futuro para alguna eprom de Z88 se llama aqui
		menu_footer_bottom_line();
	}
}

//Segundo mensaje ya ha pasado 4 segundos en pantalla. Desplazar si no ha cabido entero o borrarlo
void delete_tape_options_set_second_message(void)
{
	//De momento borrar lo que haya
        menu_putstring_footer(0,2,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

	//Hacer que el second message se desplace a la izquierda si no ha cabido entero
	int longitud_mostrada=strlen(mostrar_footer_second_message_mostrado);

	debug_printf (VERBOSE_DEBUG,"second message: index: %d text: %s",indice_second_message_mostrado,&mostrar_footer_second_message[indice_second_message_mostrado]);

	int longitud_real=strlen(&mostrar_footer_second_message[indice_second_message_mostrado]);
	if (longitud_real>longitud_mostrada) {
			desplaza_texto(&indice_second_message_mostrado,mostrar_footer_second_message,mostrar_footer_second_message_mostrado);

			//Metemos contador a 1 segundo
			tape_options_set_second_message_counter=1;

			//Y mostrarlo
			autoselect_options_put_footer();



			return;
	}

	//Aunque para Z88 no se llama a funcion de set_snaptape_fileoptions, y no haria falta repintar zona footer Z88,
        //lo hacemos por si en un futuro para alguna eprom de Z88 se llama aqui
        menu_footer_bottom_line();
}
