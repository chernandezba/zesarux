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
   ZRCP functions
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

//con esto peta la compilacion en windows quejandose de winsock
//#include <errno.h>


#include "cpu.h"
#include "start.h"
#include "debug.h"
#include "utils.h"
#include "network.h"
#include "remote.h"
#include "compileoptions.h"
#include "ql.h"
#include "disassemble.h"
#include "zxvision.h"
#include "menu_items.h"
#include "menu_debug_cpu.h"
#include "audio.h"
#include "timer.h"
#include "screen.h"
#include "ay38912.h"
#include "charset.h"
#include "diviface.h"
#include "ula.h"
#include "superupgrade.h"
#include "tbblue.h"
#include "textspeech.h"
#include "tsconf.h"
#include "operaciones.h"
#include "snap.h"
#include "kartusho.h"
#include "ifrom.h"
#include "zxevo.h"
#include "settings.h"
#include "esxdos_handler.h"
#include "assemble.h"
#include "expression_parser.h"
#include "joystick.h"
#include "snap_zsf.h"
#include "autoselectoptions.h"
#include "zeng.h"
#include "zeng_online.h"
#include "ql_qdos_handler.h"
#include "tape.h"
#include "snap_ram.h"
#include "mmc.h"
#include "atomic.h"



z80_bit remote_ack_enter_cpu_step={0};


//Si se llama a end_emulator desde aqui
z80_bit remote_calling_end_emulator={0};

//solicitar comando de cerrar todos menus
int zrcp_command_close_all_menus=0;

#ifndef NETWORKING_DISABLED

#include <pthread.h>
#include <sys/types.h>

#ifdef MINGW
	#include <winsock2.h>

	//para usar socklen_t
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <arpa/inet.h>
#endif

#ifdef __FreeBSD__
#include <netinet/in.h>
#endif

#include <stdarg.h>

struct sockaddr_in adr;
unsigned int long_adr;
int sock_listen;
//int sock_connected_client=-1;

//int remote_salir_conexion_cliente;

z80_bit remote_protocol_ended={0};








void remote_cpu_enter_step(int misocket);
void remote_cpu_exit_step(int misocket);


//Usados en el prompt de ensamblado
z80_bit remote_protocol_assembling={0};
unsigned int direccion_assembling;




pthread_t thread_remote_protocol;

//Si el thread se ha inicializado correctamente
z80_bit thread_remote_inicializado={0};



#define EASTER_EGG_TOTAL_SPRITES 13

//Sprite GScape
z80_byte easter_egg_sprite_0[]={
2,26,
0x03,0x80,
0x05,0xC0,
0x07,0x80,
0x04,0x40,
0x03,0x80,
0x0D,0x40,
0x1E,0xE0,
0x1E,0xE0,
0x37,0x40,
0x35,0x40,
0x37,0x40,
0x37,0x40,
0x1A,0x20,
0x19,0xC0,
0x05,0xC0,
0x0D,0x40,
0x13,0x60,
0x1E,0xE0,
0x1E,0xE0,
0x1C,0xC0,
0x3D,0xC0,
0x39,0x80,
0x31,0x80,
0x50,0x40,
0x60,0xE0,
0x30,0x00
};


//Sprite Toi
z80_byte easter_egg_sprite_1[]={
3,32,
0x00,0x7E,0x00,
0x0E,0x81,0x60,
0x11,0x00,0x90,
0x2E,0x81,0x68,
0x2C,0x80,0x34,
0x4C,0x00,0x12,
0x48,0x00,0x12,
0x38,0x00,0x0C,
0x10,0x00,0x08,
0x11,0x86,0x08,
0x13,0xCF,0x04,
0x13,0xCF,0x04,
0x23,0xDF,0x8A,
0x26,0xF7,0x85,
0x24,0x63,0x85,
0x54,0x63,0x89,
0xA4,0x63,0x89,
0xA6,0xF7,0x06,
0x93,0x9E,0x04,
0x90,0x00,0x04,
0x70,0x00,0x08,
0x08,0x01,0x08,
0x09,0x02,0x10,
0x08,0xFC,0x10,
0x14,0x00,0x20,
0x12,0x00,0x50,
0x0B,0x01,0x88,
0x11,0xFF,0x08,
0x10,0x80,0x90,
0x10,0x80,0x60,
0x09,0x00,0x00,
0x06,0x00,0x00
};


//Sprite Sabre
z80_byte easter_egg_sprite_2[]={
6,40,
0x00,0x00,0x01,0x80,0x00,0x00,
0x00,0x00,0x07,0xE0,0x00,0x00,
0x00,0x00,0x1D,0xD8,0x00,0x00,
0x00,0x00,0x7E,0xC6,0x00,0x00,
0x00,0x01,0xBF,0x71,0x80,0x00,
0x00,0x06,0x7B,0xD1,0xE0,0x00,
0x00,0x1D,0x3D,0x77,0x18,0x00,
0x00,0x73,0x8A,0xCC,0x16,0x00,
0x01,0xF7,0xE0,0x00,0x31,0x80,
0x07,0xF9,0xBF,0xD9,0x60,0x60,
0x0D,0x5C,0x7C,0x57,0xC0,0x50,
0x1C,0xBF,0x03,0x57,0x00,0x48,
0x39,0x7E,0xE0,0x02,0x00,0x48,
0x34,0xFD,0x6B,0x6A,0x40,0x84,
0x28,0x7A,0xAE,0xEA,0x02,0x84,
0x33,0x1D,0xB7,0x69,0x27,0x04,
0x17,0x87,0x76,0xD5,0x3E,0x08,
0x1F,0xE0,0x7B,0xD4,0xF8,0x08,
0x0F,0xD4,0x00,0x00,0xC8,0x10,
0x07,0xB7,0xC0,0x02,0x40,0x20,
0x0B,0xEE,0xAB,0xD2,0x44,0xC0,
0x08,0xFB,0x6F,0x52,0x27,0x10,
0x0E,0x1F,0xFB,0x51,0xF8,0x10,
0x0A,0xC0,0xFF,0xFF,0x03,0x50,
0x0A,0xEF,0x00,0x00,0xF7,0x50,
0x0B,0xC7,0x00,0x00,0xF7,0x50,
0x0A,0xFF,0x00,0x00,0xE5,0x50,
0x0E,0xEF,0x00,0x00,0xF7,0x50,
0x0B,0xEF,0x00,0x00,0xE1,0x50,
0x0A,0xEB,0x00,0x00,0xE7,0x50,
0x0F,0xDD,0x00,0x00,0xF1,0x50,
0x0A,0xEF,0x00,0x00,0xF7,0x50,
0x0E,0xDF,0x00,0x00,0xF5,0x50,
0x0A,0xED,0x00,0x00,0xE3,0x50,
0x0B,0xEF,0x00,0x00,0xF7,0x70,
0x0E,0xEF,0x00,0x00,0xE7,0x60,
0x02,0xFF,0x00,0x00,0xF3,0x40,
0x01,0xFA,0x00,0x00,0xF5,0xC0,
0x00,0x6F,0x00,0x00,0xF6,0x00,
0x00,0x06,0x00,0x00,0x60,0x00
};


//Sprite WorldDes
z80_byte easter_egg_sprite_3[]={
6,33,
0x00,0x00,0x00,0x00,0x03,0xFE,
0x00,0x00,0x00,0x00,0x0F,0xFF,
0x00,0x00,0x00,0x00,0x2A,0xAB,
0x00,0x00,0x00,0x00,0xD5,0x57,
0x00,0x00,0x00,0x00,0xFF,0xFB,
0x00,0x00,0x00,0x00,0xFF,0xF7,
0x00,0x00,0x00,0x00,0x24,0x12,
0x00,0x00,0x00,0x01,0x7E,0xBF,
0x00,0x00,0x00,0x00,0x63,0x71,
0x00,0x00,0x00,0x01,0x7E,0xBF,
0x00,0x00,0x00,0x0F,0x7F,0x7F,
0x00,0x00,0x00,0x31,0x7E,0xBF,
0x00,0x00,0x00,0xC1,0x00,0x00,
0x00,0x00,0x01,0x01,0x77,0x77,
0x7F,0xFF,0xFF,0x83,0xAA,0xAA,
0xDF,0xE0,0x01,0xFF,0xDD,0xDD,
0xEF,0xFF,0xFF,0xFF,0x00,0x00,
0x77,0xFF,0xFF,0xFF,0x7E,0xBF,
0x3B,0xFF,0xFF,0xFF,0x63,0x71,
0x0F,0xFF,0xFE,0xAB,0x7E,0xBF,
0x00,0x00,0x1F,0xFF,0x7F,0x7F,
0x00,0x3F,0x00,0x0F,0x7E,0xBF,
0x00,0x1E,0x03,0x80,0x24,0x12,
0x00,0x30,0x0F,0xFF,0xFF,0xFB,
0x00,0x60,0x03,0x80,0x3F,0xF7,
0x00,0xC0,0x00,0x07,0xFF,0xFB,
0x01,0x80,0x00,0x07,0xD5,0x57,
0x03,0x00,0x00,0x01,0xAA,0xAB,
0x06,0x00,0x00,0x00,0xFF,0xFF,
0x0C,0x00,0x00,0x00,0x67,0xFE,
0x3E,0x00,0x00,0x01,0xF8,0x7C,
0x3E,0x00,0x00,0x01,0xF8,0x00,
0x00,0x00,0x00,0x00,0x00,0x00
};

//Sprite NightShift
z80_byte easter_egg_sprite_4[]={
3,56,
0x00,0x00,0x3C,
0x1E,0x01,0xC2,
0x35,0x86,0x12,
0x6A,0xF8,0xA9,
0x5D,0x55,0x75,
0xBE,0xAB,0xF9,
0x9F,0xFF,0xF5,
0xAF,0xAA,0xA9,
0x95,0x00,0x51,
0x4A,0x03,0x01,
0x40,0x0C,0xC3,
0x27,0xF8,0x32,
0x1C,0x00,0x0A,
0x08,0x05,0x44,
0x08,0x0A,0x84,
0x06,0x05,0x08,
0x04,0x0A,0x90,
0x06,0x17,0x10,
0x05,0x0E,0x30,
0x06,0x17,0x50,
0x05,0x0E,0x30,
0x06,0x17,0x50,
0x05,0x2E,0x30,
0x0A,0x1D,0x50,
0x0D,0x3E,0x30,
0x0A,0x1D,0x10,
0x0D,0x2E,0x28,
0x0A,0x1F,0x18,
0x15,0x2E,0x88,
0x1A,0x1F,0x18,
0x14,0x2F,0x88,
0x1A,0x57,0x58,
0x14,0x2F,0x8C,
0x2A,0x57,0xC4,
0x34,0x0B,0xAC,
0x28,0x15,0xC4,
0x34,0x0A,0xA2,
0x28,0x01,0x56,
0x50,0x00,0x02,
0x60,0x1F,0x86,
0x43,0xE0,0x72,
0x4E,0x1F,0x0A,
0x10,0xAF,0x46,
0x21,0x5E,0xB0,
0x00,0xBD,0x76,
0x11,0x5A,0xEC,
0x08,0xBD,0xD8,
0x04,0x5B,0xB0,
0x02,0x35,0x60,
0x01,0x01,0xC0,
0x00,0x3E,0x00,
0x00,0x4F,0x00,
0x00,0xBE,0x80,
0x00,0x4F,0x00,
0x00,0xBE,0x80,
0x00,0x41,0x00
};

//Sprite Franknstein
z80_byte easter_egg_sprite_5[]={
1,16,
0x3C,
0x7A,
0x84,
0xBB,
0xEF,
0xF0,
0x7E,
0x3C,
0x7E,
0x76,
0xF7,
0xFB,
0x3C,
0x76,
0x66,
0x77
};

//Sprite Exolon
z80_byte easter_egg_sprite_6[]={
3,31,
0x00,0x10,0x00,
0x00,0x20,0x00,
0x00,0x21,0xE0,
0x00,0xC7,0xF0,
0x06,0x6F,0x88,
0x0D,0xCF,0x70,
0x0B,0xB7,0x40,
0x1B,0x4B,0x78,
0x16,0xB5,0xB0,
0x02,0x7A,0xC0,
0x0F,0x7B,0x70,
0x1E,0xBB,0x00,
0x0E,0xEB,0x30,
0x01,0xF0,0x7F,
0x01,0xFD,0x86,
0x00,0x7D,0xF8,
0x01,0x81,0xB8,
0x00,0xFC,0x30,
0x02,0x00,0x00,
0x05,0x54,0x00,
0x0A,0xA0,0x00,
0x34,0x16,0x00,
0x03,0xEE,0x00,
0x0F,0x9E,0x00,
0x1F,0x0F,0x00,
0x15,0x0A,0x80,
0x7C,0x07,0x00,
0xF0,0x07,0x00,
0xE0,0x03,0x20,
0x70,0x07,0xE0,
0x3C,0x07,0x80
};

//Sprite Zorro
z80_byte easter_egg_sprite_7[]={
3,23,
0x00,0x0F,0x00,
0x00,0x0F,0x00,
0x00,0x3F,0xC0,
0x00,0x0E,0x00,
0x00,0x0F,0x00,
0x00,0x0F,0x00,
0x00,0x0E,0x00,
0x00,0x1F,0x80,
0x00,0x3F,0xC0,
0x00,0x3F,0xE0,
0x00,0x7F,0xF0,
0x00,0x7F,0x78,
0x00,0xFF,0x10,
0x00,0xFF,0x00,
0x01,0xFF,0x00,
0x01,0xFF,0x80,
0x03,0x9D,0xC0,
0x03,0x18,0xE0,
0x00,0x18,0x60,
0x00,0x30,0xC0,
0x00,0x60,0x80,
0x00,0x61,0xC0,
0x00,0x20,0x60
};


//Sprite SirFred
z80_byte easter_egg_sprite_8[]={
2,12,
0x0F,0x80,
0x07,0xC0,
0xC7,0xF8,
0x6F,0x9E,
0x3F,0xBC,
0x3F,0xF4,
0x3F,0xD0,
0x3F,0x80,
0x3F,0xEA,
0x63,0xFE,
0x47,0x7C,
0x02,0x00
};

//Sprite AquaPlane
z80_byte easter_egg_sprite_9[]={
3,14,
0x1C,0x00,0x00,
0x1F,0xC0,0x00,
0x0F,0xF8,0x00,
0x0F,0xFE,0x00,
0x07,0xFF,0x00,
0x05,0xFD,0xC0,
0x01,0x7C,0xE0,
0x05,0x7F,0xF0,
0x05,0x5F,0xF8,
0x03,0x5F,0xFC,
0x01,0xFF,0xF8,
0x00,0x79,0xC0,
0x00,0x00,0x04,
0x31,0x72,0xD0
};


//Sprite ChaseHQ invirtiendo ceros y unos
z80_byte easter_egg_sprite_10[]={
4,40,
0x00,0x00,0x00,0x00,
0x50,0x00,0x00,0x02,
0x40,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x09,0xA0,0x00,
0x00,0x33,0xA2,0x00,
0x00,0x0B,0xAC,0x80,
0x00,0x0B,0xAC,0x80,
0x00,0x03,0xEC,0x90,
0x00,0x4B,0xDD,0x90,
0x00,0x3F,0xDF,0xF0,
0x00,0x23,0xFF,0x08,
0x00,0xC0,0x3C,0x00,
0x00,0xB0,0x38,0x70,
0x00,0xBC,0x78,0xE8,
0x00,0xF0,0x38,0x10,
0x01,0x42,0x3C,0x20,
0x00,0x9F,0xBC,0xF0,
0x11,0x6B,0x9F,0x58,
0x11,0xFF,0x3F,0xFA,
0x18,0xFE,0xDD,0xFE,
0x19,0x7C,0xFE,0xFA,
0x0C,0xF8,0x1C,0xFE,
0x08,0x78,0x02,0x7A,
0x00,0xA8,0x15,0x16,
0x40,0x51,0x5A,0x8A,
0x60,0x20,0xFE,0x16,
0x40,0x13,0x01,0xE6,
0x40,0x21,0xFF,0xD6,
0x42,0x08,0xFC,0xAE,
0x41,0x25,0xC1,0xCE,
0x62,0x0B,0xFF,0xAE,
0x71,0x05,0xFF,0x4E,
0x42,0x82,0xAA,0x92,
0x35,0x41,0x45,0x1C,
0x77,0x80,0x00,0x5A,
0x67,0x40,0x00,0xDC,
0x67,0xA8,0x00,0xDA,
0x27,0xF5,0x14,0x54,
0x00,0x00,0x00,0x00
};


//Sprite FoxFightsBack
z80_byte easter_egg_sprite_11[]={
4,28,
0x01,0xFF,0xFF,0x80,
0x07,0xF1,0x1F,0xE0,
0x1F,0xF7,0x5F,0xF8,
0x3F,0xF7,0x5F,0xFC,
0x7F,0xF5,0x5F,0xFC,
0x7F,0xF1,0x1F,0xFE,
0xFF,0xFF,0xFF,0xFE,
0xFF,0xFF,0xFF,0xFF,
0xD5,0x54,0x45,0x47,
0xD5,0x46,0xEC,0x5F,
0xC5,0x56,0xED,0x5F,
0xD5,0x56,0xED,0x57,
0xD4,0x56,0xC5,0x47,
0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,
0xA8,0xAE,0x26,0xE3,
0xAA,0xAF,0x6A,0xEF,
0xDA,0xAF,0x6A,0xE7,
0xDA,0xAF,0x6A,0xEF,
0xD8,0x8E,0x26,0x23,
0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFD,0xFF,
0x7F,0x88,0xAD,0xFE,
0x7F,0xBA,0xAD,0xFC,
0x3F,0x9A,0xDD,0xF8,
0x1F,0xBA,0xAF,0xF8,
0x0F,0xB8,0xAD,0xFE,
0x03,0xFF,0xFF,0xFF
};

//Sprite Turrican
z80_byte easter_egg_sprite_12[]={
3,31,
0x00,0x7E,0x00,
0x00,0x83,0x00,
0x00,0x6B,0x80,
0x00,0x43,0x80,
0x00,0x06,0x80,
0x01,0xFC,0x80,
0x00,0xF1,0x00,
0x00,0x02,0x80,
0x81,0x11,0xC0,
0xFF,0xCB,0xE0,
0xFD,0x51,0xE0,
0x02,0x80,0x90,
0x07,0x03,0x70,
0x01,0x6B,0xA0,
0x00,0x01,0x40,
0x00,0x3E,0x00,
0x01,0xBF,0x00,
0x07,0xFF,0x00,
0x0F,0xDF,0x00,
0x17,0xDE,0x03,
0x3B,0x09,0x3F,
0x3C,0x07,0x7F,
0x20,0x0F,0x73,
0x1E,0x06,0xC3,
0x1C,0x00,0x01,
0x0E,0x00,0x00,
0x0E,0x00,0x00,
0x0F,0x00,0x00,
0x07,0x00,0x00,
0x1F,0x00,0x00,
0x3F,0x00,0x00
};


struct s_items_ayuda
{
  char *nombre_comando;
  char *nombres_alternativos;
  char *parametros;
  char *ayuda_comando;
};

struct s_items_ayuda items_ayuda[]={

  {"about",NULL,NULL,"Shows about message"},
  {"assemble","|a","[address] [instruction]","Assemble at address. If no instruction specified, "
                                        "opens assemble prompt"},
  {"ayplayer","|ayp","command parameter","Runs a command on the AY Player. command can be:\n"
	"get-author          Prints the author\n"
	"get-elapsed-track   Prints elapsed track time in 1/50 of seconds\n"
    "get-file            Prints the file being played\n"
    "get-id-file         Prints the number of order in playlist of the file being played\n"
	"get-misc            Prints misc information\n"
    "get-playlist        Prints the playlist\n"
    "get-total-files     Prints total files in playlist\n"
	"get-total-tracks    Prints total tracks\n"
	"get-track-length    Prints track length in 1/50 of seconds\n"
	"get-track-name      Prints track name\n"
	"get-track-number    Prints current track number\n"

	"load                Loads the .ay file indicated by parameter\n"
    "load-dir            Loads the directory to the playlist\n"
	"next-file           Go to next file\n"
    "play-id             Plays the file identificated by parameter id\n"
	"prev-file           Go to previous file\n"
	"next-track          Go to next track\n"
	"prev-track          Go to previous track\n"
	"stop                Stops playing\n"

},

	{"clear-membreakpoints",NULL,NULL,"Clear all memory breakpoints"},
    {"close-all-menus",NULL,NULL,"Close all open menus"},

	{"cpu-code-coverage",NULL,"action [parameter]","Sets cpu code coverage parameters. Action and parameters are the following:\n"
	"clear            Clear address list\n"
	"enabled yes|no   Enable or disable the cpu code coverage\n"
	"get              Get all run addresses\n"
	},


	{"cpu-history",NULL,"action [parameter] [parameter]","Runs cpu history actions. Action and parameters are the following:\n"
	"clear                      Clear the cpu history\n"
	"enabled       yes|no:      Enable or disable the cpu history\n"
	"get           index:       Get registers at position, being 0 the most recent item\n"
	"get-max-size               Return maximum allowed elements in history\n"
	"get-pc        start items: Return PC register from position start, being 0 the most recent item, total items. Goes backwards\n"
	"get-size                   Return total elements in history\n"
	"ignrephalt    yes|no:      Ignore repeated opcode HALT. Disabled by default. Parameter shared with cpu-transaction-log\n"
	"ignrepldxr    yes|no:      Ignore repeated opcode LDIR or LDDR. Disabled by default\n"
	"is-enabled                 Tells if the cpu history is enabled or not\n"
	"is-started                 Tells if the cpu history is started or not\n"
    "restore       index:       Restore registers from position, being 0 the most recent item\n"
	"started       yes|no:      Start recording cpu history. Requires it to be enabled first\n"
	"set-max-size  number:      Sets maximum allowed elements in history\n"
	},


  {"cpu-panic",NULL,"text","Triggers the cpu panic function with the desired text. Note: It sets cpu-step-mode before doing it, so it ensures the emulation is paused"},
  {"cpu-step","|cs",NULL,"Run single opcode cpu step. Note: if 'real video' and 'shows electron on debug' settings are enabled, display will be updated immediately"},
  {"cpu-step-over","|cso",NULL,"Runs until returning from the current opcode. In case if current opcode is RET or JP (with or without flag conditions) it will run a cpu-step instead of cpu-step-over"},
	{"cpu-transaction-log",NULL,"parameter value","Sets cpu transaction log parameters. Parameters and values are the following:\n"
	"logfile         name:   File to store the log\n"
	"enabled         yes|no: Enable or disable the cpu transaction log. Requires logfile to enable it\n"

	"autorotate      yes|no: Enables automatic rotation of the log file\n"
	"rotatefiles     number: Number of files to keep in rotation (1-999)\n"
	"rotatesize      number: Size in MB to rotate log file (0-9999). 0 means no rotate\n"
	"rotatelines     number: Size in lines to rotate log file (0-2147483647). 0 means no rotate\n"

	"truncate        yes|no: Truncate the log file. Requires value set to yes\n"
	"truncaterotated yes|no: Truncate the rotated log files. Requires value set to yes\n"

	"ignrephalt      yes|no: Ignore repeated opcode HALT. Disabled by default. Parameter shared with cpu-history\n"

	"datetime        yes|no: Enable datetime logging\n"
	"tstates         yes|no: Enable tstates logging\n"
	"address         yes|no: Enable address logging. Enabled by default\n"
	"opcode          yes|no: Enable opcode logging. Enabled by default\n"
	"registers       yes|no: Enable registers logging\n"
	},
  {"debug-analyze-command",NULL,"parameters","Just analyze the command and print its parameters"},

  {"disable-breakpoint","|db","index","Disable specific breakpoint"},
  {"disable-breakpoints",NULL,NULL,"Disable all breakpoints"},
  {"disassemble","|d","[address] [lines]","Disassemble at address. If no address specified, "
                                        "disassemble from PC register. If no lines specified, disassembles one line"},
	{"dump-nested-functions",NULL,NULL,"Shows internal nested core functions"},
	{"dump-scanline-buffer",NULL,NULL,"Shows internal scanline rainbow buffer, pixel and atribute byte pairs"},
  {"enable-breakpoint","|eb","index","Enable specific breakpoint"},
  {"enable-breakpoints",NULL,NULL,"Enable breakpoints"},
  {"enter-cpu-step","|encs",NULL,"Enter cpu step to step mode"},
	{"esxdoshandler-get-open-files","|esxgof",NULL,"Gets a list of open files and directories on the esxdos handler"},
  {"evaluate","|e","expression","Evaluate expression. It's the same parser as using breakpoints on the debug menu"},
  //{"evaluate-condition","|ec","condition","Evaluate condition. It's the same as using evaluate condition on the breakpoints debug menu"},
  {"exit-cpu-step","|ecs",NULL,"Exit cpu step to step mode"},
  {"exit-emulator",NULL,NULL,"Ends emulator"},

	{"extended-stack",NULL,"action [parameter]","Sets extended stack parameters, which allows you to see what kind of values are in the stack. Action and parameters are the following:\n"
	"clear              Clears the extended stack setting values to 0 and type to default\n"
	"enabled yes|no     Enable or disable the extended stack\n"
	"get     n [index]  Get n values. The index default value is the SP register\n"
	},


{"find-label",NULL,"label","Finds label on source code"},
  {"generate-nmi",NULL,NULL,"Generates a NMI"},
	{"get-audio-buffer-info",NULL,NULL,"Get audio buffer information"},
  {"get-breakpoints","|gb","[index] [items]","Get breakpoints list. If set index, returns item at index. If set items, returns number of items list starting from index parameter"},
	{"get-breakpointsactions","|gba","[index] [items]","Get breakpoints actions list. If set first, returns item at index. If set items, returns number of items list starting from index parameter"},
	{"get-breakpoints-optimized",NULL,"[index] [items]","Show which breakpoints are optimized or not. If set index, returns item at index. If set items, returns number of items list starting from index parameter"},
	  {"get-buildnumber",NULL,NULL,"Shows build number. Useful on beta version, this build number is the compilation date of ZEsarUX in Unix time format"},
	{"get-cpu-core-name",NULL,NULL,"Get emulation cpu core name"},
	{"get-cpu-frequency",NULL,NULL,"Get cpu frequency in HZ"},
	{"get-cpu-turbo-speed",NULL,NULL,"Get cpu turbo speed"},
  {"get-crc32",NULL,"start_address length","Calculate crc32 checksum starting at address for defined length. It uses current memory zone"},
  {"get-current-machine","|gcm",NULL,"Returns current machine name"},
	{"get-current-memory-zone","|gcmz",NULL,"Returns current memory zone"},
	{"get-debug-settings","|gds",NULL,"Get debug settings on remote command protocol. See command set-debug-settings"},

	{"get-io-ports",NULL,NULL,"Returns currently i/o ports used"},

  	{"get-membreakpoints",NULL,"[address] [items]","Get memory breakpoints list. If set address, returns item at address. If set items, returns number of enabled items list starting from address parameter"},
	{"get-machines",NULL,NULL,"Returns list of emulated machines"},
	{"get-memory-pages","|gmp","[verbose]","Returns current state of memory pages. Default output will be the same as on debug menu; verbose output gives a detailed description of every page"},
	{"get-memory-zones","|gmz",NULL,"Returns list of memory zones of this machine"},
	{"get-ocr",NULL,NULL,"Get OCR output text"},
	{"get-os",NULL,NULL,"Shows emulator operating system"},
  {"get-paging-state",NULL,NULL,"Shows paging state on Spectrum 128k machines: if using screen 5/7 and if paging enabled"},
  {"get-registers","|gr",NULL,"Get CPU registers"},
	{"get-snapshot",NULL,NULL,"Gets a zsf snapshot on console. Contents are shown as hexadecimal characters"},
	{"get-stack-backtrace",NULL,"[items]","Get last 16-bit values from the stack. If no items parameter, it shows 5 by default"},
	{"get-tstates",NULL,NULL,"Get the t-states counter"},
	{"get-tstates-partial",NULL,NULL,"Get the t-states partial counter. Shows text OVERFLOW if value overflows"},
	{"get-ui-io-ports",NULL,NULL,"Gets user interacton io ports values for 8 rows of keyboard and joystick. Format of the values received is the same as command set-ui-io-ports"},
	  {"get-version",NULL,NULL,"Shows emulator version"},

	{"get-video-driver",NULL,NULL,"Shows current video driver"},

#ifdef EMULATE_VISUALMEM
  {"get-visualmem-written-dump","|gvmwd","[compact]","Dumps all the visual memory written positions and values. Then, clear its contents. If parameter compact, will compress non zero values on the same line, for a maximum of 16"},
  {"get-visualmem-read-dump","|gvmrd","[compact]","Dumps all the visual memory read positions and values. Then, clear its contents. If parameter compact, will compress non zero values on the same line, for a maximum of 16"},
  {"get-visualmem-opcode-dump","|gvmod","[compact]","Dumps all the visual memory executed positions and values. Then, clear its contents. If parameter compact, will compress non zero values on the same line, for a maximum of 16"},
#endif
  {"hard-reset-cpu",NULL,NULL,"Hard resets the machine"},
  {"help","|?","[command]","Shows help screen or command help"},
	{"hexdump","|h","pointer length","Dumps memory at address, showing hex and ascii."},
	{"hexdump-internal",NULL,"pointer length [offset]","Dumps internal memory (hexadecimal and ascii) for a given memory pointer. "
							"Pointer can be:\n"
							"diviface_memory: where divide/divmmc firmware and ram is located\n"
							"emulator_memory: usually includes RAM+ROM emulated machine\n"
							"rainbow_buffer: where the real video memory buffer is located\n"
							"superupgrade_flash: where the superupgrade Flash is located\n"
							"superupgrade_ram: where the superupgrade RAM is located\n"
							"\n"
							"Use with care, pointer address is a memory address on the emulator program (not the emulated memory)"},
	{"ifrom-press-button",NULL,NULL,"Press button on the iFrom interface"},
	{"kartusho-press-button",NULL,NULL,"Press button on the Kartusho interface"},
	{"load-binary",NULL,"file addr len","Load binary file \"file\" at address \"addr\" with length \"len\", on the current memory zone. Set ln to 0 to load the entire file in memory"},
	{"load-source-code","|lsc","file","Load source file to be used on disassemble opcode functions"},
	{"ls",NULL,NULL,"Minimal command list"},
    {"mmc-reload",NULL,NULL,"Reload MMC file"},
	{"noop",NULL,NULL,"This command does nothing"},
    {"open-menu",NULL,NULL,"Triggers action to open menu (like F5)"},
    {"print-error",NULL,"message","Prints a error message exactly the same way as debug_printf(VERBOSE_ERR,..."},
	{"print-footer",NULL,"message","Prints message on footer"},
	{"put-snapshot",NULL,NULL,"Puts a zsf snapshot from console. Contents must be hexadecimal characters without spaces"},
{"qdos-get-open-files","|qlgof",NULL,"Gets a list of open files and directories on the QL QDOS handler"},

  {"quit","|exit|logout",NULL,"Closes connection"},
	{"read-memory",NULL,"[address] [length]","Dumps memory at address. "
	"If address not specified, dumps all memory for current memory zone: 64 kB for mapped memory on Z80, 16 kB for Spectrum 48kB ROM etc. "
	"And if you specify address but not length, only 1 byte is read"
	},
    {"realtape-open",NULL,"file","Inserts real tape"},
  {"reset-cpu",NULL,NULL,"Resets CPU"},
	{"reset-tstates-partial",NULL,NULL,"Resets the t-states partial counter"},
  {"run","|r","[verbose] [limit] [no-stop-on-data] [update-immediately]","Run cpu when on cpu step mode. Returns when a breakpoint is fired, data sent (for example keypress) or any other event which opens the menu.\n"
	"Set verbose parameter to get verbose output\n"
	"limit parameter is a number of opcodes to run before returning\n"
	"no-stop-on-data tells that the command will not return if data is sent to the socket (for example keypress on telnet client)\n"
	"update-immediately tells that the display must be updated after every opcode run; this requires 'real video' and 'shows electron on debug' settings enabled\n"
	"The parameters can be written in different order, for example:\nrun verbose\nor\nrun 100\nor\nrun verbose 100\n"
   "Notice this command does not run the usual cpu loop, instead it is controlled from ZRCP. If you close the connection, the run loop will die\n"
	 },
	{"save-binary",NULL,"file addr len","Save binary file \"file\" from address \"addr\" with length \"len\", from the current memory zone. Set ln to 0 to save the entire current memory zone"},
	{"save-binary-internal",NULL,"pointer length file [offset]","Dumps internal memory to file for a given memory pointer. "
				"Pointer can be any of the hexdump-internal command\n"
				"Use with care, pointer address is a memory address on the emulator program (not the emulated memory)"},

	{"save-screen",NULL,"file","Save screen to file. Currently bmp, scr and pbm file formats supported"},



	{"send-keys-ascii",NULL,"time asciichar1 [asciichar2] [asciichar3] ... ","Simulates sending some ascii keys on parameters asciichar, separated by spaces. Every key is separated in time by a non-press time. Time is in miliseconds, a normal value for Basic writing is 100 miliseconds"},
	{"send-keys-event",NULL,"key event [nomenu]","Simulates sending key press/release. See file utils.h, enum util_teclas for values. Event must be 0 for release, or different to 0 for press\n"
	                        "nomenu is an optional parameter, if set to non 0, tells the key is not sent when menu is open; by default is 0: send the key even if the menu is open"},

	{"send-keys-string",NULL,"time string","Simulates sending some keys on parameter string. Every key is separated in time by a non-press time. Time is in miliseconds, a normal value for Basic writing is 100 miliseconds"},
	{"set-breakpoint","|sb","index [condition]","Sets a breakpoint at desired index entry with condition. If no condition set, breakpoint will be handled as disabled\n"
	HELP_MESSAGE_CONDITION_BREAKPOINT
	},
	{"set-breakpointaction","|sba","index [action]","Sets a breakpoint action at desired index entry. The condition to fire the action is the one that matches the same index as on the breakpoints table\n"
	HELP_MESSAGE_BREAKPOINT_ACTION
	},
	{"set-cr",NULL,NULL,"Sends carriage return to every command output received, useful on Windows environments"},
	{"set-debug-settings","|sds","setting","Set debug settings on remote command protocol. It's a numeric value with bitmask with different meaning: \n"
				"Bit 0: show all cpu registers on cpu stepping or only pc+opcode.\nBit 1: show 8 next opcodes on cpu stepping.\n"
				"Bit 2: Do not consider a L preffix when searching source code labels.\n"
				"Bit 3: Show bytes when debugging opcodes.\n"
				"Bit 4: Repeat last command only by pressing enter.\n"
				"Bit 5: Step over interrupt when running cpu-step, cpu-step-over and run verbose. It's the same setting as Step Over Interrupt on menu\n"
		},

	{"set-ui-io-ports",NULL,"9-hex-values","Sets user interacton io ports values for 8 rows of keyboard and joystick. Bytes must be in hexadecimal and not separated\n"
	"The order of sending the 8 rows of keyboard is:\n"
	"\n"
	"              Bit   4    3    2    1    0     \n"
	"0xFEFE port. Keys:  V    C    X    Z    Sh    \n"
	"0xFDFE port. Keys:  G    F    D    S    A     \n"
	"0xFBFE port. Keys:  T    R    E    W    Q     \n"
	"0xF7FE port. Keys:  5    4    3    2    1     \n"
	"0xEFFE port. Keys:  6    7    8    9    0     \n"
	"0xDFFE port. Keys:  Y    U    I    O    P     \n"
	"0xBFFE port. Keys:  H    J    K    L    Enter \n"
	"0x7FFE port. Keys:  B    N    M    Simb Space \n"
	"\n"
	"And the last one (the 9th) is the joystick value:\n"
	"                    Fire Up   Down Left Right\n"
	"\n"
	"Keyboard bits have a 0 for a pressed key, and a 1 for an unpressed key\n"
	"Joystick bits have a 1 for a pressed direction, and a 0 for an unpressed direction\n"
	"\n"
	"So for example, sending the command:\n"
	"set-ui-io-ports efffffffffffffff00\n"
	"Will press key V (as EFH is 11101111 in binary)\n"
	"Or another example:\n"
	"set-ui-io-ports ffffffffffffffff01\n"
	"Will press joystick right direction\n"
	"\n"
	"Actually you only have to set/reset 5 lower bits of every keyboard rows, so:\n"
	"set-ui-io-ports ffffffffffffffff00\n"
	"has the same effect as:\n"
	"set-ui-io-ports 1f1f1f1f1f1f1f1f00\n"
	"\n"
	"Joystick value has always the same effect no matter which kind of joystick you selected, so for example, sending a 01 value will allways mean press right direction\n"

	},


	{"set-machine","|sm","machine_name","Set machine"},
	{"set-membreakpoint",NULL,"address type [items]","Sets a memory breakpoint starting at desired address entry for type. If items parameter is not set, the default is 1. type can be:\n"
		"0: Disabled\n"
		"1: Fired when reading memory\n"
		"2: Fired when writing memory\n"
		"3: Fired when reading or writing memory\n"
		},
	{"set-memory-zone","|smz","zone","Set memory zone number"},
  {"set-register","|sr","register=value","Changes register value. Example: set-register DE=3344H"},

	{"set-text-brightness",NULL,"brightness","Change text render brightness value (0-100)"},
	{"set-verbose-level",NULL,NULL,"Sets verbose level for console output"},
	{"set-window-zoom",NULL,"zoom","Sets window zoom"},
  {"smartload","|sl","file","Smart-loads a file. If the cpu is not in cpu-step mode, it will change to cpu-step before loading, and exit cpu-step mode after loading; if it was already on cpu-step mode, it will be on cpu-step mode after loading. Use with care, may produce unexpected behaviour when emulator is doing a machine reset for example"},
  {"snapshot-load",NULL,"file","Loads a snapshot. If the cpu is not in cpu-step mode, it will change to cpu-step before loading, and exit cpu-step mode after loading; if it was already on cpu-step mode, it will be on cpu-step mode after loading"},
  {"snapshot-save",NULL,"file","Saves a snapshot"},
  {"snapshot-inram-get-index",NULL,"position","Returns index to a RAM snapshot position, where 0 is the oldest position"},
  {"snapshot-inram-load",NULL,"position","Loads the RAM snapshot from position, where 0 is the oldest position"},
  {"speech-empty-fifo",NULL,NULL,"Empty speech fifo"},
  {"speech-send",NULL,"message","Sends message to speech"},



  {"tbblue-get-clipwindow",NULL,"ula|layer2|sprite|tilemap","Get clip window parameters. You need to tell which clip window. Only allowed on machine TBBlue"},
  {"tbblue-set-clipwindow",NULL,"ula|layer2|sprite|tilemap x1 x2 y1 y2","Set clip window parameters. You need to tell which clip window. Only allowed on machine TBBlue"},


 {"tbblue-get-palette",NULL,"ula|layer2|sprite first|second index [items]","Get palette colours at index, if not specified items parameters, returns only one. You need to tell which palette. Returned values are in hexadecimal format. Only allowed on machine TBBlue"},
 {"tbblue-get-pattern",NULL,"index 4|8 [items]","Get patterns at index, of type 4 or 8 bpp, if not specified items parameters, returns only one. Returned values are in hexadecimal format. Only allowed on machine TBBlue"},

{"tbblue-get-register",NULL,"index","Get TBBlue register at index"},

 {"tbblue-get-sprite",NULL,"index [items]","Get sprites at index, if not specified items parameters, returns only one. Returned values are in hexadecimal format. Only allowed on machine TBBlue"},

 {"tbblue-set-palette",NULL,"ula|layer2|sprite first|second index value","Sets palette values starting at desired starting index. You need to tell which palette. Values must be separated by one space each one"},
 {"tbblue-set-pattern",NULL,"index value","Sets pattern values starting at desired pattern index. Values must be separated by one space each one, you can only define one pattern maximum (so 256 values maximum)"},

{"tbblue-set-register",NULL,"index value","Set TBBlue register with value at index"},

 {"tbblue-set-sprite",NULL,"index value","Sets sprite values starting at desired sprite index. Values must be separated by one space each one, you can only define one sprite maximum (so 4 values maximum)"},

 {"tsconf-get-af-port",NULL,"index","Get TSConf XXAF port value"},

 {"tsconf-set-af-port",NULL,"index value","Set TSConf XXAF port value"},

	{"view-basic",NULL,NULL,"Gets Basic program listing"},
	{"write-memory","|wm","address value","Writes a sequence of bytes starting at desired address on memory. Bytes must be separated by one space each one"},
	{"write-memory-raw",NULL,"address values","Writes a sequence of bytes starting at desired address on memory. Bytes must be in hexadecimal and not separated"},
	{"write-port",NULL,"port value","Writes value at port"},
	{"zeng-is-master",NULL,NULL,"Tells if ZENG is configured as master or not"},


  {"zeng-online","|zo","command parameter","Related ZENG Online actions. Command can be:\n"
    "alive user_pass n uuid                          Tells the user is alive\n"
    "authorize-join creator_pass n perm              Authorize/deny permissions to first client join to room n. id is authorization id. perm are permissions, can be:\n"
    "                                                0: deny, 1: allow_send_keys, 2: allow_get_snapshot. Sum values for combinations\n"
    "create-room n name                              Creates a room n. It must be in non-created state. Returns the creator_password\n"
    "destroy-room creator_pass n                     Destroys room n\n"
    "disable                                         Disables ZENG Online\n"
	"enable                                          Enables ZENG Online\n"
    "get-keys user_pass n                            This command returns continuously (never ends) keys from room n, every one separated by NL character.\n"
    "                                                If there is not any key to get, it will block until one is generated\n"
    "                                                Returned format is: uuid key event nomenu\n"
    "get-join-queue-size creator_pass n              Returns the number of clients waiting for join on room n\n"
    "get-join-first-element-queue creator_pass n     Gets the first element in join waiting queue on room n\n"
    "get-key-profile creator_pass n p                Gets profile (p) keys for room n.\n"
    "get-key-profile-assign creator_pass n p         Gets assigned profile (p) keys for room n to user uuid.\n"
    "get-message user_pass n                         Gets the broadcast message from room\n"
    "get-message-id user_pass n                      Gets the broadcast message id from room\n"
    "get-snapshot user_pass n                        This command returns the last snapshot from room n, returns ERROR if no snapshot there. Requires user_pass\n"
    "get-snapshot-id user_pass n                     This command returns the last snapshot id from room n, returns ERROR if no snapshot there. Requires user_pass\n"
    "get-kicked-user user_pass n                     This command returns the last kicked user, returning its uuid\n"
    "is-enabled                                      Returns enabled status\n"
    "join n nickname uuid [creator_pass]             Joins to room n. Returns the user_password and the permissions. If parameter creator_pass is not set, it will need authorization from the master\n"
    "                                                uuid is a unique identifier for the client, usually can be the same as Statistics uuid\n"
    "kick creator_pass n uuid                        Kick user identified by uuid\n"
    "leave n user_pass uuid                          Leaves room n.\n"
    "list-rooms                                      Returns rooms list\n"
    "list-users user_pass n                          Gets the list of joined users on room n, user and uuid on separate lines\n"
    "put-snapshot creator_pass n data                Put a snapshot on room n, requieres creator_pass for that room. Data must be hexadecimal characters without spaces\n"
    "reset-autojoin creator_pass n                   Disables autojoin on room (n)\n"
    "reset-allow-messages creator_pass n             Disallows sending messages (allowed by default)\n"
    "send-keys user_pass n uuid key event nomenu     Simulates sending key press/release to room n.\n"
    "                                                uuid is a unique identifier for the client, usually can be the same as Statistics uuid\n"
    "                                                See file utils.h, enum util_teclas for key values\n"
    "                                                Event must be 0 for release, or different to 0 for press\n"
	"                                                nomenu if set to non 0, tells the key is not sent when menu is open\n"
    "send-message user_pass n nickname message       Sends broadcast message to room\n"
    "set-autojoin creator_pass n p                   Define permissions (p) for autojoin on room (n), this enables autojoin. Requires creator_pass of that room\n"
    "set-allow-messages creator_pass n               Allows sending messages (allowed by default)\n"
    "set-key-profile creator_pass n p k1 k2 k3 ...   Defines profile (p) keys for room n.\n"
    "set-key-profile-assign creator_pass n p [uuid]  Assigns profile (p) keys for room n to user uuid. Set uuid to blank \"\" or leave undefined to deassign.\n"
    "set-max-players creator_pass n m                Define max-players (m) for room (n). Requires creator_pass of that room\n"

},

 {"zxevo-get-nvram",NULL,"index","Get ZX-Evo NVRAM value at index"},

  {NULL,NULL,NULL,NULL}
};


//Si longitud=0, devuelve toda la memoria
void remote_get_memory(int misocket,unsigned int inicio,unsigned int longitud)
{

	menu_debug_set_memory_zone_attr();

	if (longitud==0) {
  	longitud=menu_debug_memory_zone_size;
	}

	while (longitud--) {
		//escribir_socket_format(misocket,"%02X",peek_byte_z80_moto(inicio++));
		escribir_socket_format(misocket,"%02X",menu_debug_get_mapped_byte(inicio++));
	}


}

void remote_help_command(int misocket,char *parametros)
{

  //char buffer_temporal[1024];

  int i;
  int encontrado=0;

  for (i=0;items_ayuda[i].nombre_comando!=NULL && !encontrado;i++) {
    if (!strcmp(parametros,items_ayuda[i].nombre_comando)) {
      escribir_socket (misocket,"Syntax: ");
      escribir_socket (misocket,items_ayuda[i].nombre_comando);
      if (items_ayuda[i].nombres_alternativos) escribir_socket (misocket,items_ayuda[i].nombres_alternativos);
      if (items_ayuda[i].parametros) {
        //escribir_socket(misocket,"\nParameters: ");
        escribir_socket(misocket," ");
        escribir_socket(misocket,items_ayuda[i].parametros);
      }
      escribir_socket (misocket,"\n\nDescription\n");
      escribir_socket (misocket,items_ayuda[i].ayuda_comando);
      encontrado=1;
    }
  }

  if (!encontrado) escribir_socket (misocket,"No help for that command");

}

//Inicio contando desde cero
void remote_get_breakpoints(int misocket,int inicio,int items)
{
  int i;

  escribir_socket (misocket,"Breakpoints: ");
  if (debug_breakpoints_enabled.v) escribir_socket (misocket,"On\n");
  else escribir_socket (misocket,"Off\n");

  for (i=inicio;i<MAX_BREAKPOINTS_CONDITIONS && i<inicio+items;i++) {

    if (debug_breakpoints_conditions_enabled[i]==0 || debug_breakpoints_enabled.v==0) {
      escribir_socket_format(misocket,"Disabled %d: ",i+1);
    }
    else {
      escribir_socket_format(misocket,"Enabled %d: ",i+1);
    }




			if (debug_breakpoints_conditions_array_tokens[i][0].tipo!=TPT_FIN) {

				//nuevo parser de breakpoints
				char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
				exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);
				escribir_socket_format(misocket,buffer_temp);
			}








    else {
      escribir_socket_format(misocket,"None");
    }

    escribir_socket(misocket,"\n");

  }
}

//Inicio contando desde cero
void remote_get_membreakpoints(int misocket,int inicio,int items)
{
  int i;

  escribir_socket (misocket,"Breakpoints: ");
  if (debug_breakpoints_enabled.v) escribir_socket (misocket,"On\n");
  else escribir_socket (misocket,"Off\n");


//Caso retornar estado del que solo miramos
	if (items==0) {
		escribir_socket_format(misocket,"%04XH : %d\n",inicio,mem_breakpoint_array[inicio]);
		return;
	}

 int total_activos=0;

  for (i=inicio;i<65536 && total_activos<items;i++) {

    z80_byte tipo=mem_breakpoint_array[i];

    if (tipo!=0) {
	escribir_socket_format(misocket,"%04XH : %d\n",i,tipo);
	total_activos++;
    }

  }
}

//Inicio contando desde cero
void remote_get_breakpoints_optimized(int misocket,int inicio,int items)
{
  int i;

  //escribir_socket (misocket,"Breakpoints: ");
  //if (debug_breakpoints_enabled.v) escribir_socket (misocket,"On\n");
  //else escribir_socket (misocket,"Off\n");

  for (i=inicio;i<MAX_BREAKPOINTS_CONDITIONS && i<inicio+items;i++) {


    if (optimized_breakpoint_array[i].optimized==0) {
      escribir_socket_format(misocket,"Not optimized %d",i+1);

    }
    else {
      escribir_socket_format(misocket,"Optimized %d: Type: %d Value: %d",
				i+1,optimized_breakpoint_array[i].operator,optimized_breakpoint_array[i].valor);
    }


    escribir_socket(misocket,"\n");

  }
}

void remote_get_memory_zones(int misocket)
{
  int i;
	char zone_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
	int readwrite;
	int size;

	escribir_socket(misocket,"Zone: -1 Name: Mapped memory\n");

	for (i=0;i<MACHINE_MAX_MEMORY_ZONES;i++) {
		size=machine_get_memory_zone_attrib(i, &readwrite);
		if (size>0) {
			machine_get_memory_zone_name(i, zone_name);
			escribir_socket_format(misocket,"Zone: %d Name: %s Size: %d R/W: %d\n",i,zone_name,size,readwrite);
		}
	}

}

void remote_get_breakpointsactions(int misocket,int inicio,int items)
{
  int i;


  for (i=inicio;i<MAX_BREAKPOINTS_CONDITIONS && i<inicio+items;i++) {
		escribir_socket_format(misocket,"%d: ",i+1);

    if (debug_breakpoints_actions_array[i][0]==0 ||
			!strcmp(debug_breakpoints_actions_array[i],"menu") ||
			!strcmp(debug_breakpoints_actions_array[i],"break")
		) {
			escribir_socket_format(misocket,"menu");
    }
    else {
      escribir_socket_format(misocket,debug_breakpoints_actions_array[i]);
    }

    escribir_socket(misocket,"\n");

  }
}


void remote_disable_breakpoint(int misocket,char *parametros)
{
  //Separar id indice y condicion
  //Buscar hasta espacio
  if (parametros[0]==0) {
    escribir_socket(misocket,"Error. No index set");
    return;
  }

  int indice=atoi(parametros);

  if (indice<1 || indice>MAX_BREAKPOINTS_CONDITIONS) {
    escribir_socket(misocket,"Error. Index out of range");
    return;
  }


  //debug_breakpoints_conditions_enabled[indice-1]=0;
  debug_breakpoints_conditions_disable(indice-1);
}


void remote_enable_breakpoint(int misocket,char *parametros)
{
  //Separar id indice y condicion
  //Buscar hasta espacio
  if (parametros[0]==0) {
    escribir_socket(misocket,"Error. No index set");
    return;
  }

  int indice=atoi(parametros);

  if (indice<1 || indice>MAX_BREAKPOINTS_CONDITIONS) {
    escribir_socket(misocket,"Error. Index out of range");
    return;
  }


  //debug_breakpoints_conditions_enabled[indice-1]=1;
  debug_breakpoints_conditions_enable(indice-1);
}

void remote_set_breakpoint(int misocket,char *parametros)
{
  //Separar id indice y condicion
  //Buscar hasta espacio
  if (parametros[0]==0) {
    escribir_socket(misocket,"Error. No parameters set");
    return;
  }
  int i;

  //Evaluar primer parametro
  int indice=atoi(parametros);

  if (indice<1 || indice>MAX_BREAKPOINTS_CONDITIONS) {
    escribir_socket(misocket,"Error. Index out of range");
    return;
  }

  //Buscar espacio del segundo parametro

  for (i=0;parametros[i]!=' ' && parametros[i];i++);


  //Si no hay espacio de segundo parametro, segundo parametro es nulo
  //Sera igual escribir:
  //set-breakpoint 1[_espacio_][_fin_linea]
  //o
  //set-breakpoint 1[_fin_linea]
  if (parametros[i]!=0) {
    i++;
  }


//Comprobar longitud condicion. Si condicion vacia, acabara desactivando el breakpoint
  int longitud=strlen(&parametros[i]);
  if (longitud>MAX_BREAKPOINT_CONDITION_LENGTH) {
    escribir_socket(misocket,"Error. Condition too long");
    return;
  }

  int result=debug_set_breakpoint(indice-1,&parametros[i]);
  if (result) escribir_socket(misocket,"Error. Error setting breakpoint");
}


void remote_set_breakpointaction(int misocket,char *parametros)
{
  //Separar id indice y condicion
  //Buscar hasta espacio
  if (parametros[0]==0) {
    escribir_socket(misocket,"Error. No parameters set");
    return;
  }
  int i;

  //Evaluar primer parametro
  int indice=atoi(parametros);

  if (indice<1 || indice>MAX_BREAKPOINTS_CONDITIONS) {
    escribir_socket(misocket,"Error. Index out of range");
    return;
  }

  //Buscar espacio del segundo parametro

  for (i=0;parametros[i]!=' ' && parametros[i];i++);


  //Si no hay espacio de segundo parametro, segundo parametro es nulo
  //Sera igual escribir:
  //set-breakpointaction 1[_espacio_][_fin_linea]
  //o
  //set-breakpointaction 1[_fin_linea]
  if (parametros[i]!=0) {
    i++;
  }


//Comprobar longitud condicion. Si condicion vacia, acabara desactivando el breakpoint
  int longitud=strlen(&parametros[i]);
  if (longitud>MAX_BREAKPOINT_CONDITION_LENGTH) {
    escribir_socket(misocket,"Error. Action too long");
    return;
  }

  debug_set_breakpoint_action(indice-1,&parametros[i]);
}

int remote_eval_yes_no(char *text)
{
	if (!strcasecmp(text,"yes")) return 1;
	else return 0;
}

void remote_cpu_transaction_log(int misocket,char *parameter,char *value)
{
	if (!strcasecmp(parameter,"logfile")) {
		strcpy(transaction_log_filename,value);
	}

	//Comun para activar el logfile y tambien para truncar. Ambos requieren detener el core para hacer esto
	else if (!strcasecmp(parameter,"enabled") ||
					!strcasecmp(parameter,"truncate") ||
					!strcasecmp(parameter,"truncaterotated")
	) {

		//Si no esta definido logfile, no se permite activar (ni desactivar)
		//podria dejar permitido desactivar pero es absurdo, si no hay logfile no estara activado
		if (transaction_log_filename[0]==0) {
			escribir_socket(misocket,"Error. logfile not set");
			return;
		}


		//Pausar la emulacion para evitar que ese core transaction log este en ejecucion. Si eso pasa,
		//puede provocar segfault al desactivarlo, pues intenta llamar a debug_nested_core_call_previous y este mismo core ya ha desaparecido
		int antes_menu_event_remote_protocol_enterstep=menu_event_remote_protocol_enterstep.v;
			remote_cpu_enter_step(misocket);
			if (menu_event_remote_protocol_enterstep.v==0) return;


		if (!strcasecmp(parameter,"enabled")) {
			if (remote_eval_yes_no(value)) {
				set_cpu_core_transaction_log();
			}

			else {
				reset_cpu_core_transaction_log();
			}
		}

		if (!strcasecmp(parameter,"truncate")) {
			if (remote_eval_yes_no(value)) {
				transaction_log_truncate();
			}
		}

		if (!strcasecmp(parameter,"truncaterotated")) {
			if (remote_eval_yes_no(value)) {
				transaction_log_truncate_rotated();
			}
		}

		//Salir del cpu step si no estaba en ese modo
		if (!antes_menu_event_remote_protocol_enterstep) remote_cpu_exit_step(misocket);

	}

	else if (!strcasecmp(parameter,"autorotate")) {
		cpu_transaction_log_rotate_enabled.v=remote_eval_yes_no(value);
	}

	else if (!strcasecmp(parameter,"rotatefiles")) {
		if (transaction_log_set_rotate_number(parse_string_to_number(value))) {
			escribir_socket(misocket,"Error. Invalid value");
			return;
		}
	}

	else if (!strcasecmp(parameter,"rotatesize")) {
		if (transaction_log_set_rotate_size(parse_string_to_number(value))) {
			escribir_socket(misocket,"Error. Invalid value");
			return;
		}
	}

	else if (!strcasecmp(parameter,"rotatelines")) {
		if (transaction_log_set_rotate_lines(parse_string_to_number(value))) {
			escribir_socket(misocket,"Error. Invalid value");
			return;
		}
	}

	else if (!strcasecmp(parameter,"ignrephalt")) {
		cpu_trans_log_ignore_repeated_halt.v=remote_eval_yes_no(value);
	}

	else if (!strcasecmp(parameter,"datetime")) {
		cpu_transaction_log_store_datetime.v=remote_eval_yes_no(value);
	}

	else if (!strcasecmp(parameter,"tstates")) {
		cpu_transaction_log_store_tstates.v=remote_eval_yes_no(value);
	}

	else if (!strcasecmp(parameter,"address")) {
		cpu_transaction_log_store_address.v=remote_eval_yes_no(value);
	}

	else if (!strcasecmp(parameter,"opcode")) {
		cpu_transaction_log_store_opcode.v=remote_eval_yes_no(value);
	}

	else if (!strcasecmp(parameter,"registers")) {
		cpu_transaction_log_store_registers.v=remote_eval_yes_no(value);
	}

	else {
		escribir_socket(misocket,"Error. Unknown parameter");
	}


}



void remote_cpu_code_coverage(int misocket,char *parameter,char *value)
{
	/*if (!strcasecmp(parameter,"logfile????")) {
		strcpy(transaction_log_filename,value);
	}*/

	//printf ("%p\n",value);

	//Comun para activar el logfile y tambien para truncar. Ambos requieren detener el core para hacer esto
	if (!strcasecmp(parameter,"enabled") ) {




		//Pausar la emulacion para evitar que ese core transaction log este en ejecucion. Si eso pasa,
		//puede provocar segfault al desactivarlo, pues intenta llamar a debug_nested_core_call_previous y este mismo core ya ha desaparecido
		int antes_menu_event_remote_protocol_enterstep=menu_event_remote_protocol_enterstep.v;
			remote_cpu_enter_step(misocket);
			if (menu_event_remote_protocol_enterstep.v==0) return;



		if (!strcasecmp(parameter,"enabled")) {
			if (remote_eval_yes_no(value)) {
				if (cpu_code_coverage_enabled.v) escribir_socket(misocket,"Error. Already enabled");
				else set_cpu_core_code_coverage();
			}

			else {
				if (cpu_code_coverage_enabled.v==0) escribir_socket(misocket,"Error. Already disabled");
				reset_cpu_core_code_coverage();
			}
		}


		//Salir del cpu step si no estaba en ese modo
		if (!antes_menu_event_remote_protocol_enterstep) remote_cpu_exit_step(misocket);



	}

	else if (!strcasecmp(parameter,"get")) {
		if (cpu_code_coverage_enabled.v==0) escribir_socket(misocket,"Error. It's not enabled");
		else {
			int i;
			for (i=0;i<65536;i++) {
		  	if (cpu_code_coverage_array[i]) {
			    escribir_socket_format(misocket,"%04X ",i);
			  }
			}
			//escribir_socket(misocket,"\n");
		}
	}

	else if (!strcasecmp(parameter,"clear")) {
	cpu_code_coverage_clear();
	}



	else {
		escribir_socket(misocket,"Error. Unknown parameter");
	}


}



void remote_extended_stack(int misocket,char *parameter,char *value,char *second_value)
{

	//printf ("%p (%s)\n",second_value,second_value);


	//If comun para acciones que requieren detener el core momentaneamente
	if (!strcasecmp(parameter,"enabled") ) {


		//Pausar la emulacion para evitar que ese core transaction log este en ejecucion. Si eso pasa,
		//puede provocar segfault al desactivarlo, pues intenta llamar a debug_nested_core_call_previous y este mismo core ya ha desaparecido
		int antes_menu_event_remote_protocol_enterstep=menu_event_remote_protocol_enterstep.v;
		remote_cpu_enter_step(misocket);
		if (menu_event_remote_protocol_enterstep.v==0) return;



		if (!strcasecmp(parameter,"enabled")) {
			if (remote_eval_yes_no(value)) {
				if (extended_stack_enabled.v) escribir_socket(misocket,"Error. Already enabled");
				else set_extended_stack();
			}

			else {
				if (extended_stack_enabled.v==0) escribir_socket(misocket,"Error. Already disabled");
				reset_extended_stack();
			}
		}


		//Salir del cpu step si no estaba en ese modo
		if (!antes_menu_event_remote_protocol_enterstep) remote_cpu_exit_step(misocket);



	}

	else if (!strcasecmp(parameter,"get")) {
		if (extended_stack_enabled.v==0) escribir_socket(misocket,"Error. It's not enabled");
		else {
			int items=parse_string_to_number(value);

			z80_int registro_inicial_stack=reg_sp;

			//Ver si hay un segundo parametro
			if (second_value[0]!=0) registro_inicial_stack=parse_string_to_number(second_value);

			z80_int indice_l;
			z80_int indice_h;
			int i;

			for (i=0;i<items;i++) {
					z80_int valor_resultante;
					z80_byte tipo;

					//Para asegurarnos que los indices siempre estan en rango 0...65535 (el tamanyo del array)
					indice_l=registro_inicial_stack+i*2;
					indice_h=indice_l+1;


					//realmente el extended stack guarda valor y tipo para cada byte, aqui solo mostramos word
					valor_resultante=extended_stack_array_items[indice_l].valor+256*extended_stack_array_items[indice_h].valor;

					//Los dos bytes deberian tener el mismo tipo
					tipo=extended_stack_array_items[indice_l].tipo;
					escribir_socket_format(misocket,"%04XH %s\n",valor_resultante,extended_stack_get_string_type(tipo));
			}

		}
	}

	else if (!strcasecmp(parameter,"clear")) {
		extended_stack_clear();
	}


	else {
		escribir_socket(misocket,"Error. Unknown parameter");
	}


}


void remote_cpu_history(int misocket,char *parameter,char *value,char *value2)
{

	//printf ("(%s) (%s)\n",value,value2);

	//Comun para activar el core y para iniciarlo y otros. Ambos requieren detener el core para hacer esto
	if (!strcasecmp(parameter,"enabled") ||
	    !strcasecmp(parameter,"started") ||
		!strcasecmp(parameter,"set-max-size") ||
		!strcasecmp(parameter,"clear")

	) {


		//Pausar la emulacion para evitar que ese core transaction log este en ejecucion. Si eso pasa,
		//puede provocar segfault al desactivarlo, pues intenta llamar a debug_nested_core_call_previous y este mismo core ya ha desaparecido
		int antes_menu_event_remote_protocol_enterstep=menu_event_remote_protocol_enterstep.v;
			remote_cpu_enter_step(misocket);
			if (menu_event_remote_protocol_enterstep.v==0) return;



		if (!strcasecmp(parameter,"enabled")) {
			if (remote_eval_yes_no(value)) {
				if (cpu_history_enabled.v) escribir_socket(misocket,"Error. Already enabled");
				else set_cpu_core_history();
			}

			else {
				if (cpu_history_enabled.v==0) escribir_socket(misocket,"Error. Already disabled");
				reset_cpu_core_history();
			}
		}

		if (!strcasecmp(parameter,"started")) {
			if (cpu_history_enabled.v==0) escribir_socket(misocket,"Error. It's not enabled\n");
			else {
				if (remote_eval_yes_no(value)) cpu_history_started.v=1;
				else cpu_history_started.v=0;
			}
		}

		if (!strcasecmp(parameter,"set-max-size")) {
			if (cpu_history_enabled.v==0) escribir_socket(misocket,"Error. It's not enabled\n");
			else {
				int total=parse_string_to_number(value);

				if (cpu_history_set_max_size(total)!=0) {
					escribir_socket(misocket,"ERROR: Value out of range");
				}
			}
		}



		if (!strcasecmp(parameter,"clear")) {
			if (cpu_history_enabled.v==0) escribir_socket(misocket,"Error. It's not enabled\n");
			else {
				cpu_history_init_buffer();
			}
		}


		//Salir del cpu step si no estaba en ese modo
		if (!antes_menu_event_remote_protocol_enterstep) remote_cpu_exit_step(misocket);



	}

	else if (!strcasecmp(parameter,"get")) {
		if (cpu_history_enabled.v==0) escribir_socket(misocket,"Error. It's not enabled\n");
		else {
			int indice=parse_string_to_number(value);
			int total_elementos=cpu_history_get_total_elements();

			//Al solicitarlo, el 0 es el item mas reciente. el 1 es el anterior a este
			//en cpu_history_get_registers_element se pide como: 0 es el mas antiguo
			//Ejemplo: 10 elementos totales. Se pide por el 3.
			//indice_final=10-3-1=7
			//Ejemplo: 10 elementos totales. Se pide por el 9 (que sera el mas antiguo)
			//indice_final=10-9-1=0
			int indice_final=total_elementos-indice-1;


			char string_destino[1024];
			cpu_history_get_registers_element(indice_final,string_destino);
			escribir_socket(misocket,string_destino);
		}
	}

	else if (!strcasecmp(parameter,"restore")) {
		if (cpu_history_enabled.v==0) escribir_socket(misocket,"Error. It's not enabled\n");
		else {
			int indice=parse_string_to_number(value);
			int total_elementos=cpu_history_get_total_elements();

			//Al solicitarlo, el 0 es el item mas reciente. el 1 es el anterior a este
			//en cpu_history_get_registers_element se pide como: 0 es el mas antiguo
			//Ejemplo: 10 elementos totales. Se pide por el 3.
			//indice_final=10-3-1=7
			//Ejemplo: 10 elementos totales. Se pide por el 9 (que sera el mas antiguo)
			//indice_final=10-9-1=0
			int indice_final=total_elementos-indice-1;

			cpu_history_regs_bin_restore(indice_final);
		}
	}


	else if (!strcasecmp(parameter,"get-size")) {
		if (cpu_history_enabled.v==0) escribir_socket(misocket,"Error. It's not enabled\n");
		else {
			escribir_socket_format(misocket,"%d",cpu_history_get_total_elements() );
		}
	}


	else if (!strcasecmp(parameter,"get-max-size")) {
		if (cpu_history_enabled.v==0) escribir_socket(misocket,"Error. It's not enabled\n");
		else {
			escribir_socket_format(misocket,"%d",cpu_history_get_max_size() );
		}
	}

	else if (!strcasecmp(parameter,"get-pc")) {
		if (cpu_history_enabled.v==0) escribir_socket(misocket,"Error. It's not enabled\n");
		else {
		int total_elementos=cpu_history_get_total_elements();
			int indice=parse_string_to_number(value);
						//Al solicitarlo, el 0 es el item mas reciente. el 1 es el anterior a este
			indice=total_elementos-indice-1;


			int total=parse_string_to_number(value2);
			//int total=final-indice+1;
			if (total<0) {
				escribir_socket(misocket,"Error. Can't be negative");
			}
			else {
				//Validar que no se pidan mas de los que hay
				if (total>total_elementos) {
					//escribir_socket(misocket,"Error. End goes beyond total elements");
					total=total_elementos;
				}

					while (total) {
						char string_destino[1024];
						cpu_history_get_pc_register_element(indice--,string_destino);
						escribir_socket_format(misocket,"%s ",string_destino);
						total--;
					}

			}
		}
	}

	else if (!strcasecmp(parameter,"ignrephalt")) {
		cpu_trans_log_ignore_repeated_halt.v=remote_eval_yes_no(value);
	}

	else if (!strcasecmp(parameter,"ignrepldxr")) {
		cpu_trans_log_ignore_repeated_ldxr.v=remote_eval_yes_no(value);
	}

	else if (!strcasecmp(parameter,"is-enabled")) {
		escribir_socket_format(misocket,"%d",cpu_history_enabled.v);
	}

	else if (!strcasecmp(parameter,"is-started")) {
		escribir_socket_format(misocket,"%d",cpu_history_started.v);
	}

	else {
		escribir_socket(misocket,"Error. Unknown parameter");
	}


}



void remote_disassemble(int misocket,unsigned int direccion,int lineas,int mostrar_direccion)
{

	menu_debug_set_memory_zone_attr();

  char buffer_retorno[1024];
	//char buffer_codigo_fuente[1024];
	//char posible_siguiente_linea[1024]="";

  size_t longitud_opcode;

  //int lineas_orig=lineas;

	//int mostrar_codigo_fuente=1;

	int pos_source=-1;

	int guessed_next_pos_source=-1;

  while (lineas) {
		//mostrar_codigo_fuente=1;
    //direccion=adjust_address_space_cpu(direccion);

		direccion=adjust_address_memory_size(direccion);
		//int posicion_final_linea=-1;

		if (remote_tamanyo_archivo_raw_source_code) {
            pos_source=remote_disassemble_find_label(direccion);
            /*
			char buffer_label[128];

			if (remote_debug_settings & 4) {
				if (CPU_IS_MOTOROLA) sprintf(buffer_label,"%05X",direccion);
				else sprintf(buffer_label,"%04X",direccion);
			}
			else {
				if (CPU_IS_MOTOROLA) sprintf(buffer_label,"L%05X",direccion);
				else sprintf(buffer_label,"L%04X",direccion);
			}

			pos_source=remote_find_label_source_code(buffer_label);
            */
			//printf ("posicion para %s: %d\n",buffer_label,pos_source);
			if (pos_source!=-1) guessed_next_pos_source=pos_source;
		}


		int pos=0;
  	if (mostrar_direccion) {
			/*if (CPU_IS_MOTOROLA) {
				sprintf(buffer_retorno,"%05X ",direccion);
				pos+=6;
			}
  		else {
				sprintf(buffer_retorno,"%04X ",direccion);
				pos +=5;
			}*/

			menu_debug_print_address_memory_zone(buffer_retorno, direccion);
			pos +=7;

			//Quitar 0 del final
			//buffer_retorno[pos]=' ';
  	}

		escribir_socket_format(misocket,"%s ",buffer_retorno);
		pos=0;

  	debugger_disassemble(&buffer_retorno[pos],100,&longitud_opcode,direccion);

		//Mostrar bytes del opcode si conviene. Tenemos buffer para 4 bytes en spectrum y para 16 en QL
		if (remote_debug_settings & 8) {
#define MAX_OPCODE_LENGHT_MOTO 12
#define MAX_OPCODE_LENGHT_Z80 4

			char buffer_bytes_opcode[MAX_OPCODE_LENGHT_MOTO*2+1]; //24+1
															//  123456789012345678901234
			strcpy(buffer_bytes_opcode,"                        ");

			if (CPU_IS_MOTOROLA) buffer_bytes_opcode[MAX_OPCODE_LENGHT_MOTO*2]=0;
			else buffer_bytes_opcode[MAX_OPCODE_LENGHT_Z80*2]=0;

			//fijamos maximo
			int buffer_longitud=longitud_opcode;
			if (buffer_longitud>MAX_OPCODE_LENGHT_MOTO) buffer_longitud=MAX_OPCODE_LENGHT_MOTO;

			//printf ("longitud :%d fijo: %d\n",longitud_opcode,buffer_longitud);

			//Copiamos bytes
			int indice;
			for (indice=0;buffer_longitud>0;buffer_longitud--,indice++) {
				char buffer_temp[3];
				sprintf (buffer_temp,"%02X",peek_byte_z80_moto(direccion+indice));
				buffer_bytes_opcode[indice*2]=buffer_temp[0];
				buffer_bytes_opcode[indice*2+1]=buffer_temp[1];
			}

			escribir_socket_format(misocket,"%s ",buffer_bytes_opcode);

		}



  	direccion +=longitud_opcode;



		if (remote_tamanyo_archivo_raw_source_code) {

			char *puntero_source=NULL;

			if (pos_source!=-1 || guessed_next_pos_source!=-1) {

				//Intentamos mostrar la siguiente linea
				if (pos_source!=-1) {
		                        int indice=remote_parsed_source_code_indexes_pointer[pos_source];
                		        puntero_source=&remote_raw_source_code_pointer[indice];
				}

				else {
					//Mostrar guessed
		                        int indice=remote_parsed_source_code_indexes_pointer[guessed_next_pos_source];
                		        puntero_source=&remote_raw_source_code_pointer[indice];

				}
			}

			escribir_socket(misocket,buffer_retorno);

			if (puntero_source!=NULL) {
				//tabular y mostrar source
				int longitud=strlen(buffer_retorno);
				//Tabular hasta columna indicada. TODO configurable
				int espacios=50-longitud;
				while (espacios>0) {
					escribir_socket(misocket," ");
					espacios--;
				}

				escribir_socket(misocket,"|");
				escribir_socket(misocket,puntero_source);
			}

		}

		else escribir_socket(misocket,buffer_retorno);


		//Y apuntamos a la siguiente linea guessed
		if (guessed_next_pos_source!=-1) guessed_next_pos_source++;


  	lineas--;
  	if (lineas) escribir_socket(misocket,"\n");


  }
}

void remote_disable_multitask(void)
{

  menu_multitarea=0;

  audio_playing.v=0;

  timer_reset();
  //Pausa de 0.1 segundo
  usleep(100000);

}

void remote_enter_menu(void)
{
  menu_fire_event_open_menu();
}
void remote_disable_multitask_enter_menu(void)
{
  remote_enter_menu();
  remote_disable_multitask();
}

void remote_send_esc_close_menu(void)
{
  //Simulamos pulsar ESC
  salir_todos_menus=1;
  puerto_especial1 &=(255-1);
  //Pausa de 0.1 segundo
  usleep(100000);
  puerto_especial1 |=1;
}


void remote_footer_cpu_step(void)
{

	menu_footer_activity("STEP");
	//Dado que se ha quitado multitask y esta todo parado, refrescar pantalla para mostrar el footer
	scr_refresca_pantalla_solo_driver();

}

void remote_footer_cpu_step_clear(void)
{

	menu_delete_footer_activity();
	//no hace falta forzar refresco pantalla dado que se sale de este estado, menu esta cerrado y por tanto se ejecutara el core tal cual

}

//estado multitarea antes de cpu_step
int menu_multitarea_antes_cpu_step=0;

//Este modo deja la emulacion pausada
void remote_cpu_enter_step(int misocket)
{
	//Si ya estaba este modo, salir sin mas
	if (menu_event_remote_protocol_enterstep.v) return;

  //De momento solo simular pulsacion de tecla de menu, eso hace saltar el step

  //TODO: Pendiente de eliminar esta variable. Tiene sentido???
  //menu_espera_tecla_no_cpu_loop_flag_salir.v=1;

  //guardar estado multitarea
  menu_multitarea_antes_cpu_step=menu_multitarea;

  remote_ack_enter_cpu_step.v=0;


    //En caso de drivers stdout y null, no cerrar menu, que es un lio. si esta abierto, dara error
    if (
        !strcmp(scr_new_driver_name,"stdout") ||
        !strcmp(scr_new_driver_name,"simpletext") ||
        !strcmp(scr_new_driver_name,"null")
    ) {
        if (menu_abierto) {
            escribir_socket(misocket,"Error. Can not enter cpu step mode. You can try closing the menu");
            return;
        }
    }

    else {

  //Abrir menu si no estaba ya abierto
  remote_disable_multitask_enter_menu();
  //Cerrar menu
  remote_send_esc_close_menu();

    }

  //Pausa de 0.1 segundo. Esperamos a que se haya cerrado el menu
  usleep(100000);

  //Avisar que entramos en paso a paso y Abrir menu
  menu_event_remote_protocol_enterstep.v=1;
  menu_abierto=1;

  int contador_timeout;
  //Esperamos a que se haya recibido la señal de entrada en remote cpu-step
  //20 iteraciones=2 segundos
  for (contador_timeout=0;contador_timeout<20 && remote_ack_enter_cpu_step.v==0;contador_timeout++) {
    usleep(100000); //0.1 segundo
  }

  if (remote_ack_enter_cpu_step.v==0) {
    //Menu no se ha enterado de entrada en este modo. Error
    escribir_socket(misocket,"Error. Can not enter cpu step mode. You can try closing the menu");
    menu_event_remote_protocol_enterstep.v=0;
    return;
  }

	remote_footer_cpu_step();

}


//Muestra salida de registros y opcode cuando se hacen operaciones de cpu step, run, etc
void remote_get_regs_disassemble(int misocket)
{
  char buffer_retorno[2048];

  //Mostrar registros
	if (remote_debug_settings&1) {
  	print_registers(buffer_retorno);
  	escribir_socket (misocket,buffer_retorno);

	//Mostrar T-estados
	escribir_socket_format(misocket," TSTATES: %d",t_estados);

  	//Salto linea
  	escribir_socket (misocket,"\n");
	}

	/*else  {
		if (CPU_IS_MOTOROLA) {
			sprintf (buffer_retorno,"%05X ",get_pc_register());
		}
		else {
			sprintf (buffer_retorno,"%04X ",get_pc_register());
		}

		escribir_socket (misocket,buffer_retorno);
	}*/


  //Y desensamblar direccion donde apunta el PC
	int lineas=1;
	if (remote_debug_settings&2) lineas=8;
  remote_disassemble(misocket,get_pc_register(),lineas,1 );

}


void remote_cpu_exit_step_continue(void)
{
  menu_event_remote_protocol_enterstep.v=0;
  remote_send_esc_close_menu();
	remote_footer_cpu_step_clear();
}


void remote_cpu_exit_step_continue_restore_multitask(void)
{

	remote_cpu_exit_step_continue();

 //Restaurar estado multitarea
 menu_multitarea=menu_multitarea_antes_cpu_step;
}

void remote_cpu_exit_step(int misocket)
{
  if (menu_event_remote_protocol_enterstep.v==0) {
    escribir_socket(misocket,"Error. You are not in step to step mode");
    return;
  }
    remote_cpu_exit_step_continue_restore_multitask();
}


//Cosas que hay que hacer despues de ejecutar opcodes de cpu
void remote_cpu_after_core_loop(int misocket)
{
  if (menu_multitarea==0) {
    audio_playing.v=0;
    //audio_thread_finish();
  }

  //Si se ha disparado excepcion de breakpoint, quitar
  menu_breakpoint_exception.v=0;

  remote_get_regs_disassemble(misocket);
}


//Ejecuta core_loop y si parametro update esta activo, actualiza pantalla al momento y muestra electron si conviene
void remote_core_loop_if_update_immediately(int update,int nowait_endframe)
{
		if (update) screen_force_refresh=1; //Para que no haga frameskip y almacene los pixeles/atributos en buffer rainbow
    if (nowait_endframe) menu_debug_registers_run_cpu_opcode();
	else cpu_core_loop();
		if (update) {
			menu_debug_registers_show_scan_position();
			menu_refresca_pantalla();
		}
}

void remote_cpu_before_core_loop(void)
{
  //Limpiar algun flag que se haya quedado antes
  menu_abierto=0;
  menu_breakpoint_exception.v=0;
}

void remote_cpu_step(int misocket) {
  //char buffer_retorno[1024];
  //Ejecutar una instruccion
  if (menu_event_remote_protocol_enterstep.v==0) {
    escribir_socket(misocket,"Error. You must first enter cpu-step mode");
    return;
  }
  debug_core_lanzado_inter.v=0;

  remote_cpu_before_core_loop();

	remote_core_loop_if_update_immediately(1,1);

  if (debug_core_lanzado_inter.v && (remote_debug_settings&32)) {
	  debug_run_until_return_interrupt();
  }



  remote_cpu_after_core_loop(misocket);


}

void remote_cpu_step_over(int misocket) {

  //char buffer_retorno[1024];
  //Si apunta PC a instrucciones RET o JP, hacer un cpu-step
  if (si_cpu_step_over_jpret()) {
	  debug_printf(VERBOSE_DEBUG,"Running only cpu-step as current opcode is JP or RET");
	  remote_cpu_step(misocket);
	  return;
  }


  if (menu_event_remote_protocol_enterstep.v==0) {
    escribir_socket(misocket,"Error. You must first enter cpu-step mode");
    return;
  }

  remote_cpu_before_core_loop();

  debug_cpu_step_over();


  remote_cpu_after_core_loop(misocket);


}



void remote_cpu_run_loop(int misocket,int verbose,int limite,int datos_vuelve,int actualiza_al_momento)
{

	char buf[30];


	//Hacer el socket que no se quede bloqueado si no hay datos.

	if (datos_vuelve) {
	#ifdef MINGW
	  u_long on = 1;
    ioctlsocket(misocket, FIONBIO, &on);
	#else
		int flags = fcntl(misocket, F_GETFL, 0);
		fcntl(misocket, F_SETFL, flags | O_NONBLOCK);
	#endif

	}

	int total_instrucciones=0;

	int salir=0;
	while (!salir) {
	  if (verbose) {
	    remote_get_regs_disassemble(misocket);
	    escribir_socket(misocket,"\n");
	  }
	  debug_core_lanzado_inter.v=0;


		remote_core_loop_if_update_immediately(actualiza_al_momento,0);

	  if (debug_core_lanzado_inter.v && (remote_debug_settings&32)) {
			debug_run_until_return_interrupt();
	  }

		//Si se vuelve cuando hay datos en el socket. Con esto, usa mucha mas cpu (46% respecto a un 19% si no se esta mirando el socket)
		if (datos_vuelve) {
			#ifdef MINGW
				int leidos=recv(misocket,buf,30,0);
				//int leidos = read(misocket, buf, 30);
				if (leidos>0) {
					salir=1;
				}
			#else
				//int leidos = read(misocket, buf, 30);
				int leidos=recv(misocket,buf,30,0);
				if (leidos>0) {
					salir=1;
				}
			#endif
		}


	  total_instrucciones++;
	  if (limite) {
	    if (total_instrucciones==limite) {
	      escribir_socket_format(misocket,"Returning after %d opcodes\n",limite);
	      salir=1;
	    }
	  }
      //Si se abre menu o salta excepcion, salir
	  if (menu_abierto || menu_breakpoint_exception.v) salir=1;
	}


	//Dejar el socket tranquilito como estaba antes
	if (datos_vuelve) {
	#ifdef MINGW
		u_long on = 0;
    	ioctlsocket(misocket, FIONBIO, &on);
	#else
		int flags = fcntl(misocket, F_GETFL, 0);
		fcntl(misocket, F_SETFL, flags ^ O_NONBLOCK);
	#endif
	}



}



//Ejecutar hasta siguiente punto de paro o cualquier otro evento que abra el menu
//Variables: verbose: si se muestra desensamblado en cada instruccion,
//limite: si se ejecutan N instrucciones y se finaliza. Si es 0, no tiene limite (hasta apertura de menu o breakpoint)
void remote_cpu_run(int misocket,int verbose,int limite,int datosvuelve,int update_immediately)
{

  if (menu_event_remote_protocol_enterstep.v==0) {
    escribir_socket(misocket,"Error. You must first enter cpu-step mode");
    return;
  }



  remote_cpu_before_core_loop();

  //Parar cuando se produzca algun evento de apertura de menu, como un breakpoint

//Hacemos esto normal y luego en menu, el menu_event_remote_protocol_enterstep refresca la pantalla por si solo cada 1 segundo
  remote_cpu_run_loop(misocket,verbose,limite,datosvuelve,update_immediately);

  debug_printf(VERBOSE_DEBUG,"Exiting run command");

	//Si se ha saltado un breakpoint, decirlo
	if (menu_breakpoint_exception.v) {
		if (debug_if_breakpoint_action_menu(catch_breakpoint_index)) {
			escribir_socket_format(misocket,"Breakpoint fired: %s\n",catch_breakpoint_message);
			}
	}


  remote_cpu_after_core_loop(misocket);

}

void remote_evaluate(int misocket,char *texto)
{

  //char salida[MAX_BREAKPOINT_CONDITION_LENGTH];


	char buffer_salida[256]; //mas que suficiente
	char string_detoken[MAX_BREAKPOINT_CONDITION_LENGTH];


	int result=exp_par_evaluate_expression(texto,buffer_salida,string_detoken);
	if (result==0) {
		escribir_socket(misocket,buffer_salida);
	}

	else if (result==1) {
		escribir_socket(misocket,buffer_salida);
	}

	else {
		escribir_socket_format(misocket,"%s parsed string: %s",buffer_salida,string_detoken);
	}

}


void remote_esxdos_gof(int misocket)
{
	int i;

	for (i=0;i<ESXDOS_MAX_OPEN_FILES;i++) {
		if (esxdos_fopen_files[i].open_file.v) {
			if (esxdos_fopen_files[i].is_a_directory.v) {
				escribir_socket_format(misocket,"%d (dir) Name: %s\n",i,esxdos_fopen_files[i].esxdos_handler_last_dir_open);
			}
			else {
				escribir_socket_format(misocket,"%d (file) Name: %s Full Path: %s\n",i,esxdos_fopen_files[i].debug_name,esxdos_fopen_files[i].debug_fullpath);
			}
		}
	}

}

void remote_qdos_gof(int misocket)
{
	int i;

	for (i=0;i<QLTRAPS_MAX_OPEN_FILES;i++) {
		if (qltraps_fopen_files[i].open_file.v) {
			if (qltraps_fopen_files[i].es_dispositivo) {
				escribir_socket_format(misocket,"%d (dir) Name: %s\n",i,qltraps_fopen_files[i].qltraps_handler_last_dir_open);
			}
			else {
				escribir_socket_format(misocket,"%d (file) Name: %s Full Path: %s\n",i,qltraps_fopen_files[i].debug_name,qltraps_fopen_files[i].debug_fullpath);
			}
		}
	}

}

/*void easter_egg_put_char(int x,int y,int color,unsigned char c)
{

  z80_bit inverse,f;

inverse.v=0;
f.v=0;
//128 y 129 corresponden a franja de menu y a letra enye minuscula
if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) caracter='?';
scr_putsprite_comun(&char_set[(caracter-32)*8],x,y,inverse,tinta,papel,f);



y=y*8;

for (line=0;line<8;line++,y++) {
  byte_leido=*puntero++;
  if (inverse.v==1) byte_leido = byte_leido ^255;
  for (bit=0;bit<8;bit++) {
        if (byte_leido & 128 ) color=tinta;
        else color=papel;

        //simular modo fast para zx81
        if (MACHINE_IS_ZX8081) {
                if (fast_mode.v==1 && video_fast_mode_emulation.v==1 && video_fast_mode_next_frame_black==LIMIT_FAST_FRAME_BLACK) color=0;
        }

        byte_leido=(byte_leido&127)<<1;

        //este scr_putpixel_zoom_rainbow tiene en cuenta los timings de la maquina (borde superior, por ejemplo)
        if (rainbow_enabled.v==1) scr_putpixel_zoom_rainbow((x*8)+bit+margenx_izq,y+margeny_arr,color);

        else scr_putpixel_zoom((x*8)+bit,y,color);

   }
}



}*/

void remote_easter_egg_refresca_pantalla(void)
{
  //sin color oscuro gris
  menu_abierto=0;
  //printf ("%p\n",scr_refresca_pantalla_solo_driver);
  scr_refresca_pantalla_solo_driver();
  menu_abierto=1;
  scr_actualiza_tablas_teclado(); //Xwindows por ejemplo requiere esto
}

z80_int *easter_egg_front_display;
z80_int *easter_egg_back_display;

void easter_egg_asigna_memoria_pantalla(void)
{
  //Asignamos las dos pantallas para visualizacion del easter egg
  //Una front y la otra back
  easter_egg_front_display=malloc(2*256*192); //16 bits, 256x192 resolucion

  easter_egg_back_display=malloc(2*256*192);

  if (easter_egg_front_display==NULL || easter_egg_back_display==NULL)
  {
    cpu_panic("Can not allocate memory for easter egg");
  }

}

void easter_egg_libera_memoria_pantalla(void)
{
  free(easter_egg_back_display);
  free(easter_egg_front_display);
}

void easter_egg_flush_display(void)
{
  //Mezcla las dos pantallas y las visualiza en la ventana
  //Back es la de fondo
  //Front es la de delante
  //Si color en front es 65535, es transparente y se muestra el back

  z80_int *back,*front;

  back=easter_egg_back_display;
  front=easter_egg_front_display;

  int x,y;
  z80_int color_final;

  for (y=0;y<192;y++) {
    for (x=0;x<256;x++) {
      color_final=*front;
      if (color_final==65535) color_final=*back;
      scr_putpixel_zoom(x,y,color_final);

      front++;
      back++;
    }
  }

  remote_easter_egg_refresca_pantalla();
}

void easter_egg_putpixel_front(int x,int y,z80_int color)
{
  int offset;
  offset=y*256+x;
  easter_egg_front_display[offset]=color;
}

void easter_egg_putpixel_back(int x,int y,z80_int color)
{
  int offset;
  offset=y*256+x;
  easter_egg_back_display[offset]=color;
}

z80_int easter_egg_getpixel_front(int x,int y)
{
  int offset;
  offset=y*256+x;
  return easter_egg_front_display[offset];
}

//Dibuja franjas de colores en el background, de color oscuro. Excluir el negro y azul, para que se vea mejor texto
void easter_egg_rainbow_back(void)
{
  int x,y;
  z80_int color=2;

  for (y=0;y<192;y++) {
    for (x=0;x<256;x++) {
      easter_egg_putpixel_back(x,y,color+16); //+16 porque son los colores oscuros
     }
    color++;
    if (color==8) color=2; //Negro ni azul no
  }

}

void easter_egg_transparent_front(void)
{
  int x,y;
  for (y=0;y<192;y++) {
    for (x=0;x<256;x++) {
      easter_egg_putpixel_front(x,y,65535);
    }
  }
}

void easter_egg_black_front(void)
{
  int x,y;
  for (y=0;y<192;y++) {
    for (x=0;x<256;x++) {
      easter_egg_putpixel_front(x,y,0);
    }
  }
}

z80_int easter_egg_get_random(void)
{
  ay_randomize(0);

  //randomize_noise es valor de 16 bits
  return randomize_noise[0];
}

void easter_egg_random_pixels_front(void)
{
  int x,y;
  z80_int color,color_final;

  int iteraciones;

  int max_iteraciones=50*10; //50 frames, 10 segundos

  int valor_si_transparente=0;

  //5 segundos
  for (iteraciones=0;iteraciones<max_iteraciones;iteraciones++) {

    for (y=0;y<192;y++) {
      for (x=0;x<256;x++) {
        color=easter_egg_get_random() % EMULATOR_TOTAL_PALETTE_COLOURS;
        color_final=color;

        //A partir de mitad de proceso, colores pueden salir transparentes
        if (iteraciones>=max_iteraciones/2) {
          //Formula 1: A medida que avanzan las iteraciones, van desapareciendo zonas de pixeles
          //if ((x+y)/2==valor_si_transparente) color_final=65535;

          //Formula 2: colores van desapareciendo segun random
          if (color_final<200) color_final=65535; //200 puesto a ojo ;)
        }



        //solo poner el pixel si no es transparente el que habia
        if (easter_egg_getpixel_front(x,y)!=65535) easter_egg_putpixel_front(x,y,color_final);
      }

      //Un poco mas de random al tema
      int entropia;
      for (entropia=0;entropia<y;entropia++) easter_egg_get_random();
    }

    if (iteraciones>=max_iteraciones/2) valor_si_transparente++;

    easter_egg_flush_display();
    usleep(1000000/50);
  }
}

//Dibujar solo una linea de un caracter (no los 8 pixeles de alto)
//scanline va de 0 a 7
void easter_scanline_putchar_front(z80_byte caracter,int x,int y,int scanline,z80_int color)
{
    z80_byte *puntero_caracter;
    int offset;
    z80_byte byte_leido;

    if (!si_valid_char(caracter)) caracter='?';

    offset=(caracter-32)*8;

    puntero_caracter=&char_set_dos[offset+scanline];
    byte_leido=*puntero_caracter;

    //Hacemos bold
    //z80_byte byte_bold=byte_leido<<1;
    //byte_leido |=byte_bold;


    int bit;

    for (bit=0;bit<8;bit++) {
      if (byte_leido&128) easter_egg_putpixel_front(x+bit,y,color);

      byte_leido=byte_leido<<1;
    }
}

//x,y en coordenadas pixeles
//ancho en caracteres (1=8 pixeles)
//alto en pixeles
void easter_egg_put_sprite_front(z80_byte *sprite, int x, int y, int ancho, int alto, z80_int color)
{

  int xorig=x;
  int anchoorig=ancho;
  z80_byte byte_leido;

  for (;alto>0;alto--,y++) {
    x=xorig;
    ancho=anchoorig;
    for (;ancho>0;ancho--,x+=8) {
      int bit;

      byte_leido=*sprite;
      sprite++;
      for (bit=0;bit<8;bit++) {
        if (byte_leido&128) {
          int xfinal=x+bit;
          if (xfinal>=0 && xfinal<=255 && y>=0 && y<=191) {  //Podemos jugar con sprites que salen de los limites de la pantalla
            easter_egg_putpixel_front(xfinal,y,color);
          }
        }
        byte_leido=byte_leido<<1;
      }
    }
  }

}

void easter_egg_scroll_vertical_front(void)
{
  int x,y;
  z80_int color;
  for (y=0;y<191;y++) {
    for (x=0;x<256;x++) {
      color=easter_egg_getpixel_front(x,y+1);
      easter_egg_putpixel_front(x,y,color);
    }
  }

  //Ultima linea transparente
  for (x=0;x<256;x++) easter_egg_putpixel_front(x,191,65535);
}

void easter_egg_scroll_horizontal_continuo_front(void)
{
  //Copiar primera linea de la izquierda que va a desaparecer
  z80_int linea_copia[192];
  z80_int color;

  int x,y;
  for (y=0;y<192;y++) linea_copia[y]=easter_egg_getpixel_front(0,y);

  //Scroll
  for (x=0;x<255;x++) {
    for (y=0;y<192;y++) {
      color=easter_egg_getpixel_front(x+1,y);
      easter_egg_putpixel_front(x,y,color);
    }
  }

  //Y poner linea a la derecha
  for (y=0;y<192;y++) easter_egg_putpixel_front(255,y,linea_copia[y]);
}

void easter_egg_star_wars_write_line(char *texto,int longitud_texto,int x,int y,int scanline, z80_int color)
{

	//Convertir buffer inicial a caracteres finales para conversión de cirílicos

	z80_byte buffer_final[MAX_ANCHO_LINEAS_GENERIC_MESSAGE];

	int longitud_final=util_convert_utf_charset(texto,buffer_final,longitud_texto);
	//Al final el texto convertido desde utf ocupara menos en pantalla en caso de haber caracteres de mas de 1 byte
	//y como formatea el texto antes por cada linea, habran lineas bastante vacias cuando haya caracteres utf

  //for (;longitud_texto;longitud_texto--) {
	  int i=0;
  for (;longitud_final;longitud_final--) {
	  //printf ("%d ",buffer_final[i]);
    easter_scanline_putchar_front(buffer_final[i],x,y,scanline,color);
    x+=8;
    //texto++;
	i++;
  }

  //printf ("\n");
}



z80_byte *easter_egg_retorna_numero_sprite(int numero_sprite,int *ancho,int *alto)
{
  z80_byte *sprite;

  switch (numero_sprite)
  {
    case 0:
      sprite=easter_egg_sprite_0;
    break;

    case 1:
      sprite=easter_egg_sprite_1;
    break;

    case 2:
      sprite=easter_egg_sprite_2;
    break;

    case 3:
      sprite=easter_egg_sprite_3;
    break;

    case 4:
      sprite=easter_egg_sprite_4;
    break;

    case 5:
      sprite=easter_egg_sprite_5;
    break;

    case 6:
      sprite=easter_egg_sprite_6;
    break;

    case 7:
      sprite=easter_egg_sprite_7;
    break;

    case 8:
      sprite=easter_egg_sprite_8;
    break;

    case 9:
      sprite=easter_egg_sprite_9;
    break;

    case 10:
      sprite=easter_egg_sprite_10;
    break;

    case 11:
      sprite=easter_egg_sprite_11;
    break;

    case 12:
      sprite=easter_egg_sprite_12;
    break;


    default:
      sprite=easter_egg_sprite_0;
    break;
  }
  *ancho=*sprite;
  sprite++;
  *alto=*sprite;
  sprite++;

  return sprite;

}
int sprite_actual=0;

void easter_egg_show_random_sprite(void)
{
  int x,y;

    x=((easter_egg_get_random() ) % 200)+10;
    y=((easter_egg_get_random() ) % 100)+20;

    //int numero_sprite=easter_egg_get_random() % EASTER_EGG_TOTAL_SPRITES;
    int  numero_sprite=sprite_actual  % EASTER_EGG_TOTAL_SPRITES;
    sprite_actual++;
    //printf ("numero sprite: %d\n",numero_sprite);
    int ancho,alto;

    z80_byte *sprite=easter_egg_retorna_numero_sprite(numero_sprite,&ancho,&alto);

    z80_int color=easter_egg_get_random() % EMULATOR_TOTAL_PALETTE_COLOURS;

    easter_egg_put_sprite_front(sprite,x,y,ancho,alto,color);
}

#define EASTER_TEXT_SCROLL_PAUSE (1000000/7)
#define EASTER_STAR_WARS_TEXT_COLOR 15
#define EASTER_REBOTE_PELOTA_PAUSA (1000000/1000)

void easter_egg_star_wars_text_one_line(char *texto)
{
  int scanline;

    for (scanline=0;scanline<8;scanline++) {
      easter_egg_star_wars_write_line(texto,strlen(texto),0,191,scanline,EASTER_STAR_WARS_TEXT_COLOR);  //Siempre escribir en ultima linea 191
      easter_egg_flush_display();
      usleep(EASTER_TEXT_SCROLL_PAUSE); //0.25 s de pausa
      easter_egg_scroll_vertical_front();
    }

}

int mostrar_sprite_random_contador=0;

z80_bit easter_egg_mostrar_sprite={0};

void easter_egg_star_wars_text(char *texto_orig)
{

    char *texto=util_malloc_max_texto_generic_message("Can not allocate memory for easter egg");
    strcpy(texto, texto_orig);
  //void menu_generic_message_tooltip(char *titulo, int tooltip_enabled, int mostrar_cursor, generic_message_tooltip_return *retorno, const char * texto_format , ...)

    //Funcion derivada de menu_generic_message_tooltip
  	//texto que contiene cada linea con ajuste de palabra. Al trocear las lineas aumentan
  	char buffer_lineas[MAX_LINEAS_TOTAL_GENERIC_MESSAGE][MAX_ANCHO_LINEAS_GENERIC_MESSAGE];

  	const int max_ancho_texto=30;

  	//Primera linea que mostramos en la ventana
  	//int primera_linea=0;

  	int indice_linea=0;  //Numero total de lineas??
  	int indice_texto=0;
  	int ultimo_indice_texto=0;
  	int longitud=strlen(texto);

  	//int indice_segunda_linea;

  	//int texto_no_cabe=0;

  	do {
  		indice_texto+=max_ancho_texto;


  		//printf ("indice_linea: %d\n",indice_linea);

  		//Controlar final de texto
  		if (indice_texto>=longitud) indice_texto=longitud;

  		//Si no, miramos si hay que separar por espacios
  		else indice_texto=menu_generic_message_aux_wordwrap(texto,ultimo_indice_texto,indice_texto);

  		//Separamos por salto de linea, filtramos caracteres extranyos
  		indice_texto=menu_generic_message_aux_filter(texto,ultimo_indice_texto,indice_texto);

  		//copiar texto
  		int longitud_texto=indice_texto-ultimo_indice_texto;


  		//snprintf(buffer_lineas[indice_linea],longitud_texto,&texto[ultimo_indice_texto]);


  		menu_generic_message_aux_copia(&texto[ultimo_indice_texto],buffer_lineas[indice_linea],longitud_texto);
  		buffer_lineas[indice_linea++][longitud_texto]=0;
  		//printf ("copiado %d caracteres desde %d hasta %d: %s\n",longitud_texto,ultimo_indice_texto,indice_texto,buffer_lineas[indice_linea-1]);


  		//printf ("texto: %s\n",buffer_lineas[indice_linea-1]);

  		if (indice_linea==MAX_LINEAS_TOTAL_GENERIC_MESSAGE) {
                          //cpu_panic("Max lines on menu_generic_message reached");
  			debug_printf(VERBOSE_INFO,"Max lines on menu_generic_message reached (%d)",MAX_LINEAS_TOTAL_GENERIC_MESSAGE);
  			//finalizamos bucle
  			indice_texto=longitud;
  		}

  		ultimo_indice_texto=indice_texto;
  		//printf ("ultimo indice: %d %c\n",ultimo_indice_texto,texto[ultimo_indice_texto]);

  	} while (indice_texto<longitud);


  	debug_printf (VERBOSE_INFO,"Read %d lines (word wrapped)",indice_linea);


    int linea_leida;

    for (linea_leida=0;linea_leida<indice_linea;linea_leida++) {
      easter_egg_star_wars_text_one_line(buffer_lineas[linea_leida]);

      //Y si se da el caso, mostrar un sprite al azar
      if ((mostrar_sprite_random_contador%3)==0 && easter_egg_mostrar_sprite.v==1) easter_egg_show_random_sprite();
      mostrar_sprite_random_contador++;
    }



    //easter_egg_star_wars_text_one_line("Hola que tal");
    //easter_egg_star_wars_text_one_line("esto es una prueba");

    free(texto);

}


//Derivado de menu_about_read_file
void easter_egg_read_file(char *aboutfile)
{


  char *about_file=util_malloc_max_texto_generic_message("Can not allocate memory for reading the file");

  debug_printf (VERBOSE_INFO,"Loading %s File",aboutfile);
  FILE *ptr_aboutfile;
//ptr_aboutfile=fopen(aboutfile,"rb");
  open_sharedfile(aboutfile,&ptr_aboutfile);

  if (!ptr_aboutfile)
  {
    easter_egg_star_wars_text("Sorry, I can not read the file... maybe you have deleted it?");
  }
  else {

    int leidos=fread(about_file,1,MAX_TEXTO_GENERIC_MESSAGE,ptr_aboutfile);
    debug_printf (VERBOSE_INFO,"Read %d bytes of file: %s",leidos,aboutfile);

    if (leidos==MAX_TEXTO_GENERIC_MESSAGE) {
      debug_printf (VERBOSE_ERR,"Read max text buffer: %d bytes. Showing only these",leidos);
      leidos--;
    }

    about_file[leidos]=0;


    fclose(ptr_aboutfile);

    easter_egg_star_wars_text(about_file);
  }

  free(about_file);
}

void easter_egg_star_wars(void)
{

  easter_egg_mostrar_sprite.v=0;

  easter_egg_star_wars_text("A long time ago, in a galaxy far, far away....");
  easter_egg_star_wars_text("\n\n");
  easter_egg_star_wars_text("Just kidding... this is not the usual Star Wars scroller... you have just found the ZEsarUX easter egg! \n"
                            "So, this could be my first demo for the Speccy, but this demo does not run on a real Speccy, "
                            "it runs from the inside of the emulator, this is not a Spectrum binary program. \n"

                            "You may notice every pixel in this demo can have its own color, and I can use here the total "
                            "palette colours of the emulator.\n"
                            "This demo have two planes, one background with the rainbow stripes and one foreground, with the scroller. "
                            "I can also do some fast and curious effects like this:\n\n");



  int contador;
  int veces_scroll_horizontal=1;

  for (contador=0;contador<256*veces_scroll_horizontal;contador++) {  //multiple de 256 para que deje la pantalla como estaba antes
    easter_egg_flush_display();
    usleep(1000000/50); //20 ms de pausa
    easter_egg_scroll_horizontal_continuo_front();
  }

  easter_egg_star_wars_text("\nWell maybe it's not very impressive, but I'm not an expert Spectrum demo programmer, I'm better at making "
                            "Spectrum emulators ;)\n");


  easter_egg_star_wars_text("I can also draw some sprites...");
  easter_egg_mostrar_sprite.v=1;

  easter_egg_star_wars_text("\n\n\n");


  easter_egg_star_wars_text("Do you recognize all the sprites? They all come from different Spectrum games.\n\n");

  easter_egg_star_wars_text("So... I don't want you to stay here reading lots of text, so if you want, you can read the following and the "
                            "usual acknowledgements list... I expect you enjoyed this simple? demo ;)\n"
                            "\nACKNOWLEDGEMENTS:\n\n");
  easter_egg_read_file("ACKNOWLEDGEMENTS");

  //Scroll final
  int y;
  for (y=0;y<192;y++) {
    easter_egg_flush_display();
    usleep(EASTER_TEXT_SCROLL_PAUSE);
    easter_egg_scroll_vertical_front();
  }
}

void easter_egg_rebote_pelota(void)
{

  //Primer flush. Para que cuando se hace putpixel directo, tengamos antes la mezcla de front-back aqui
  easter_egg_flush_display();

  int x,y;
    //Rebote pelota
    x=((get_pc_register() ^ easter_egg_get_random()) +17 ) % 256; //x random segun pc. +17 por meter un random
    y=(get_pc_register() ^ easter_egg_get_random() ) % 192; //y random segun pc
    //mejor x,y fijados asi sabemos como acabara
    //x=33;
    //y=99;

    int incx=+1;
    int incy=+1;
    int contador=0;

    z80_int color=(get_pc_register() ^ easter_egg_get_random() ) % EMULATOR_TOTAL_PALETTE_COLOURS;

    while (contador<50000) //Con esto llenamos la pantalla
    {
      //Hacemos putpixel normal sin front-back
      //easter_egg_putpixel_front(x,y,color);
      scr_putpixel_zoom(x,y,color);
      color++;
      if (color==EMULATOR_TOTAL_PALETTE_COLOURS) color=0;

      x +=incx;
      y +=incy;

      if (x==0 || x==255) {
        incx=-incx;
      }

      if (y==0 || y==191) {
        incy=-incy;
      }


      usleep(EASTER_REBOTE_PELOTA_PAUSA);


      //hacemos refresco normal sin front-back
      //easter_egg_flush_display();

      //Refresco de vez en cuando
      if ( (contador%100)==0) remote_easter_egg_refresca_pantalla();

      contador++;
    }

}

void remote_easter_egg_mostrar(void)
{

  easter_egg_asigna_memoria_pantalla();

  easter_egg_rainbow_back();

  //5 segundos de pixeles aleatorios
  easter_egg_black_front();
  easter_egg_random_pixels_front();


  easter_egg_transparent_front();



  easter_egg_star_wars();

  easter_egg_rebote_pelota();


  easter_egg_libera_memoria_pantalla();

}

void remote_easter_egg(int misocket)
{

  if (!si_complete_video_driver()) {
    escribir_socket(misocket,"Error. You need to try this using a full video driver...");
    return;
  }

    //Entramos en el mismo modo que cpu-step para poder congelar la emulacion
    remote_cpu_enter_step(misocket);
    if (menu_event_remote_protocol_enterstep.v==0) return;

    zrcp_easter_egg_running.v=1;

    remote_easter_egg_mostrar();
    sleep(5);


    remote_cpu_exit_step(misocket);

    zrcp_easter_egg_running.v=0;

}

//Retorna puntero a fin de comando (0) o salto de linea, o bien, caracter de despues del espacio
char *find_space_or_end(char *s)
{
  while (*s && *s!=' ' && *s!='\r') {
    s++;
  }
  if (*s==' ') s++;
  return s;
}

void remote_cerrar_conexion(void)
{

    //printf("Remote cerrar conexion\n");

    //TODO
    //gestionar cada sock_connected_client de cada conexion
    //realmente para que? si ya se cierra la conexion porque se cierra el emulador...
    //Esto por ejemplo si desactivamos ZRCP desde menu y hay conexiones activas, no se cierran si no hacemos nada aqui...
    //Aqui se deberian cerrar tanto los sockets como finalizar los threads que los gestionan
    return;


/*
    //Solo hacer close de eso cuando habia una conexión activa
    if (sock_connected_client>=0) {
#ifdef MINGW
	closesocket(sock_connected_client);
#else
	close(sock_connected_client);
#endif

	//#ifdef MINGW
	//WSACleanup();
	//#endif
    }

    sock_connected_client=-1;
*/
}



//Parseo de parametros de comando.
#define REMOTE_MAX_PARAMETERS_COMMAND 20
//array de punteros a comando y sus argumentos
//char *remote_command_argv[REMOTE_MAX_PARAMETERS_COMMAND];
//int remote_command_argc;



//Separar comando con codigos 0 y rellenar array de parametros
void remote_parse_commands_argvc(char *texto,int *remote_command_argc, char **remote_command_argv)
{

  *remote_command_argc=util_parse_commands_argvc_comillas(texto, remote_command_argv, REMOTE_MAX_PARAMETERS_COMMAND);

}

void remote_dump_nested_core_functions(int misocket)
{

	char buffer[10000];


	debug_dump_nested_functions(buffer);

	escribir_socket(misocket,buffer);
}

//Escribe en destino las dos cadenas, la segunda tabulada en la posicion column, y tabulada mediante espacios
/*void remote_print_strings_tabbed(char *destino, char *string1, char *string2, int column)
{
	//Primero llenar de espacios hasta columna
	int i;
	for (i=0;i<column;i++) {
		destino[i]=' ';
	}

	//Escribe las dos cadenas
	sprintf(destino,"%s",string1);

	//Quitar 0 final
	destino[strlen(string1)]=' ';

	//Escribe cadena 2
	sprintf(&destino[column],"%s",string2);
}
*/

void remote_fill_string_spaces(char *string, int length)
{
	int i;

	for (i=0;i<length;i++) {
		*string=' ';
		string++;
	}

	*string=0;
}

int remote_get_max_length_command(void)
{
	int salto_tabulacion=0;
	int i;

	//Calcular primero salto de tabulacion ideal, leyendo mayor longitud de texto
	for (i=0;items_ayuda[i].nombre_comando!=NULL;i++) {
		int l=strlen(items_ayuda[i].nombre_comando);
		if (l>salto_tabulacion) salto_tabulacion=l;
	}

	return salto_tabulacion;
}

void remote_print_no_zero_ending(char *s,char *string_to_add)
{
	while (*string_to_add) {
		*s=*string_to_add;

		s++;
		string_to_add++;
	}
}

//Retorna lista de comandos con ayuda simple en misma linea
void remote_help_no_parameters_command(int misocket)
{

		//Mostrar comando y al lado ayuda del mismo, reducida hasta el siguiente punto

		//Calcular primero maxima longitud comando
		int salto_tabulacion=remote_get_max_length_command();

		//darle dos espacios mas

		salto_tabulacion +=2;

		char buffer_texto[1024];

		char buffer_ayuda[1024];

    int i;
    for (i=0;items_ayuda[i].nombre_comando!=NULL;i++) {

				//llenar string con espacios minimo hasta salto_tabulacion
				remote_fill_string_spaces(buffer_texto,salto_tabulacion);



				//Meter en buffer_ayuda hasta siguiente . o final
				int j;

				for (j=0;items_ayuda[i].ayuda_comando[j]!=0 && items_ayuda[i].ayuda_comando[j]!='.';j++) {
					buffer_ayuda[j]=items_ayuda[i].ayuda_comando[j];
				}

				buffer_ayuda[j]=0;

				//Meter en buffer texto las dos cadenas
				remote_print_no_zero_ending(buffer_texto,items_ayuda[i].nombre_comando);

				sprintf(&buffer_texto[salto_tabulacion],"%s",buffer_ayuda);
				escribir_socket(misocket,buffer_texto);


        escribir_socket (misocket,"\n");
    }

    escribir_socket_format(misocket,"\nTotal commands: %d\n",i);

}

//Retorna lista de comandos tabulados en varias columnas
void remote_simple_help(int misocket)
{


		int salto_tabulacion=remote_get_max_length_command();

		//y darle dos espacios mas
		salto_tabulacion +=2;

		const int columnas_total=4;
		int columna=0;
		char buffer_texto[1024];
		int i;

    for (i=0;items_ayuda[i].nombre_comando!=NULL;i++) {
				if (columna==0) remote_fill_string_spaces(buffer_texto,columnas_total*salto_tabulacion);

				int pos_columna=columna*salto_tabulacion;


				remote_print_no_zero_ending(&buffer_texto[pos_columna],items_ayuda[i].nombre_comando);
				//sprintf(&buffer_texto[pos_columna],"%s",items_ayuda[i].nombre_comando);
				//Quitar 0 del final
				buffer_texto[pos_columna+strlen(items_ayuda[i].nombre_comando)]=32;

				columna++;

				//Si final de columna o final de comandos
				if (columna==columnas_total) {
					escribir_socket(misocket,buffer_texto);
					escribir_socket (misocket,"\n");
					columna=0;
				}


        //escribir_socket(misocket,items_ayuda[i].nombre_comando);
        //escribir_socket (misocket,"\n");
    }

		//Si ha acabado en medio de columnas
		if (columna!=0) {
			escribir_socket(misocket,buffer_texto);
			escribir_socket (misocket,"\n");
		}

}

//Copia de origen hasta destino finalizado con espacio o 0
void remote_copy_string_spc(char *origen, char *destino)
{

  if (origen!=NULL) {
    while (*origen!=0 && *origen!=32) {
      *destino=*origen;

      destino++;
      origen++;
    }
  }

  *destino=0;
}






void remote_ayplayer(int misocket,char *command,char *command_parm)
{

	//Comandos que requieren que este en ejecucion el player
	if (
		!strcasecmp(command,"prev-track") ||
		!strcasecmp(command,"stop") ||
		!strcasecmp(command,"next-track") ||
        !strcasecmp(command,"prev-file") ||
        !strcasecmp(command,"next-file") ||
		!strcasecmp(command,"get-track-name") ||
		!strcasecmp(command,"get-author") ||
		!strcasecmp(command,"get-misc") ||
		!strcasecmp(command,"get-track-number") ||
		!strcasecmp(command,"get-total-tracks") ||
        !strcasecmp(command,"get-id-file") ||
		!strcasecmp(command,"get-elapsed-track") ||
        !strcasecmp(command,"get-file") ||
		!strcasecmp(command,"get-track-length")
		) {
		if (!menu_audio_new_ayplayer_si_mostrar()) {
			escribir_socket_format(misocket,"ERROR. Player not running\n");
			return;
		}

		if (!strcasecmp(command,"prev-track")) ay_player_previous_track();
		if (!strcasecmp(command,"stop")) ay_player_stop_player();
		if (!strcasecmp(command,"next-track")) ay_player_next_track();
        if (!strcasecmp(command,"prev-file")) ay_player_previous_file();
        if (!strcasecmp(command,"next-file")) ay_player_next_file();
		if (!strcasecmp(command,"get-track-name")) {
			escribir_socket(misocket,ay_player_file_song_name);
		}

        if (!strcasecmp(command,"get-file")) {
            escribir_socket(misocket,ay_player_filename_playing);
        }

		if (!strcasecmp(command,"get-author")) {
			escribir_socket(misocket,ay_player_file_author);
		}

		if (!strcasecmp(command,"get-misc")) {
			escribir_socket(misocket,ay_player_file_misc);
		}

		if (!strcasecmp(command,"get-track-number")) {
			escribir_socket_format(misocket,"%d",ay_player_pista_actual);
		}

		if (!strcasecmp(command,"get-total-tracks")) {
			escribir_socket_format(misocket,"%d",ay_player_total_songs());
		}




		if (!strcasecmp(command,"get-id-file")) {
			escribir_socket_format(misocket,"%d",ay_player_playlist_item_actual);
		}


		if (!strcasecmp(command,"get-elapsed-track")) {
			escribir_socket_format(misocket,"%d",ay_song_length_counter);
		}

		if (!strcasecmp(command,"get-track-length")) {
			escribir_socket_format(misocket,"%d",ay_song_length);
		}

	}

    if (!strcasecmp(command,"get-total-files")) {
        escribir_socket_format(misocket,"%d",ay_player_playlist_get_total_elements());
    }

	if (!strcasecmp(command,"load-dir")) {
		ay_player_add_directory_playlist(command_parm);
	}

	if (!strcasecmp(command,"load")) {
		ay_player_add_file(command_parm);
	}

    if (!strcasecmp(command,"get-playlist")) {

        if (ay_player_playlist_get_total_elements()!=0) {

            //Recorrer toda la playlist
            ay_player_playlist_item *playitem=ay_player_first_item_playlist;

            int i=0;
            while (playitem!=NULL) {
                char nombre_archivo[PATH_MAX];

                util_get_file_no_directory(playitem->nombre,nombre_archivo);

                escribir_socket_format(misocket,"%d %s\n",i,nombre_archivo);

                playitem=playitem->next_item;
                i++;
            }

        }
    }


	if (!strcasecmp(command,"play-id")) {
        int identificador=parse_string_to_number(command_parm);
		ay_player_play_this_item(identificador);
	}


}

void remote_hexdump_internal(int misocket,z80_byte *inicio,int longitud,int offset)
{
	//Formato:
	//000284d0  20 20 20 20 20 4d 4f 56  45 2e 57 20 20 20 44 31  |     MOVE.W   D1|
	//printf ("%p %p\n",inicio,memoria_spectrum);
	inicio +=offset;

	for (;longitud>0;longitud -=16) {
		escribir_socket_format(misocket,"%XH ",offset);
		offset +=16;

		//escribir_socket_format(misocket,"%d ",longitud);
		//sleep(1);
		int longitud_parcial=longitud;

		if (longitud_parcial>16) longitud_parcial=16;

		z80_byte *puntero_linea;
		puntero_linea=inicio;
		//Volcar 16 bytes hexa
		for (;longitud_parcial>0;longitud_parcial--) {
			escribir_socket_format(misocket,"%02X ",*puntero_linea);
			puntero_linea++;
		}
		//z80_byte c=(z80_byte)*memoria_spectrum;

		//escribir_socket_format(misocket,"%02XH ",*puntero_linea);
		//Llenar los que falten

		if (longitud<16) {
			int espacios=16-longitud;
			for (;espacios;espacios--) escribir_socket(misocket,"   ");
		}

		escribir_socket(misocket," |");


		puntero_linea=(z80_byte *) inicio;

		longitud_parcial=longitud;

		if (longitud_parcial>16) longitud_parcial=16;

		//Volcar 16 bytes ascii
		for (;longitud_parcial>0;longitud_parcial--) {
			unsigned char c=*puntero_linea;
			if (c<32 || c>126) c='.';
			escribir_socket_format(misocket,"%c",c);
			puntero_linea++;
		}

		escribir_socket(misocket,"|\n");

		inicio +=16;

	}
}



void remote_hexdump(int misocket,int inicio,int longitud)
{
	//Formato:
	//000284d0  20 20 20 20 20 4d 4f 56  45 2e 57 20 20 20 44 31  |     MOVE.W   D1|
	//printf ("%p %p\n",inicio,memoria_spectrum);
	//inicio +=offset;

	menu_debug_set_memory_zone_attr();

	for (;longitud>0;longitud -=16) {
		char buffer_direccion[MAX_LENGTH_ADDRESS_MEMORY_ZONE+1];
		menu_debug_print_address_memory_zone(buffer_direccion,inicio);

		escribir_socket_format(misocket,"%sH ",buffer_direccion);
		//offset +=16;

		//escribir_socket_format(misocket,"%d ",longitud);
		//sleep(1);
		int longitud_parcial=longitud;

		if (longitud_parcial>16) longitud_parcial=16;

		int puntero_linea;
		puntero_linea=inicio;
		//Volcar 16 bytes hexa
		for (;longitud_parcial>0;longitud_parcial--) {
			//escribir_socket_format(misocket,"%02X ",peek_byte_z80_moto(puntero_linea));
			escribir_socket_format(misocket,"%02X ",menu_debug_get_mapped_byte(puntero_linea));
			puntero_linea++;
		}
		//z80_byte c=(z80_byte)*memoria_spectrum;

		//escribir_socket_format(misocket,"%02XH ",*puntero_linea);
		//Llenar los que falten

		if (longitud<16) {
			int espacios=16-longitud;
			for (;espacios;espacios--) escribir_socket(misocket,"   ");
		}

		escribir_socket(misocket," |");


		puntero_linea=inicio;

		longitud_parcial=longitud;

		if (longitud_parcial>16) longitud_parcial=16;

		//Volcar 16 bytes ascii
		for (;longitud_parcial>0;longitud_parcial--) {
			//unsigned char c=peek_byte_z80_moto(puntero_linea);
			unsigned char c=menu_debug_get_mapped_byte(puntero_linea);
			if (c<32 || c>126) c='.';
			escribir_socket_format(misocket,"%c",c);
			puntero_linea++;
		}

		escribir_socket(misocket,"|\n");

		inicio +=16;

	}
}

//Retorna 0 si no reconocido puntero. Retorna puntero en variable "puntero"
int return_internal_pointer(char *s,z80_byte **puntero)
{

	z80_byte *inicio;
	inicio=NULL;
	int retorno=1;

	if (!strcasecmp(s,"emulator_memory")) 				inicio=memoria_spectrum;
	else if (!strcasecmp(s,"diviface_memory")) 		inicio=diviface_memory_pointer;
	else if (!strcasecmp(s,"rainbow_buffer")) 		inicio=(z80_byte *)rainbow_buffer;
	else if (!strcasecmp(s,"superupgrade_ram")) 	inicio=superupgrade_ram_memory_pointer;
	else if (!strcasecmp(s,"superupgrade_flash")) inicio=superupgrade_rom_memory_pointer;

	else retorno=0;

	*puntero=inicio;

	return retorno;
}


//Retorna puntero a array de clip window, segun
//tipo : ula|layer2|sprite|tilemap
//Retorna NULL si hay algun error
z80_byte *remote_return_clipwindow(char *tipo)
{
//z80_byte clip_windows[4][4];

  if (!strcmp(tipo,"ula")) {
    return clip_windows[TBBLUE_CLIP_WINDOW_ULA];
  }
  else   if (!strcmp(tipo,"layer2")) {
    return clip_windows[TBBLUE_CLIP_WINDOW_LAYER2];
  }
  else   if (!strcmp(tipo,"sprite")) {
    return clip_windows[TBBLUE_CLIP_WINDOW_SPRITES];
  }
  else   if (!strcmp(tipo,"tilemap")) {
    return clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP];
  }

  return NULL;

}

//Retorna puntero a array de paleta, segun
//tipo : ula|layer2|sprite
//firstsecond: first|second
//Retorna NULL si hay algun error
z80_int *remote_return_palette(char *tipo, char *firstsecond)
{

  int segundo=0;

  if (!strcmp(firstsecond,"first")) segundo=0;
  else if (!strcmp(firstsecond,"second")) segundo=1;
  else return NULL;
  /*
  z80_int tbblue_palette_ula_first[256];
z80_int tbblue_palette_ula_second[256];
z80_int tbblue_palette_layer2_first[256];
z80_int tbblue_palette_layer2_second[256];
z80_int tbblue_palette_sprite_first[256];
z80_int tbblue_palette_sprite_second[256];
*/

  if (!strcmp(tipo,"ula")) {
    if (segundo) return tbblue_palette_ula_second;
    else return tbblue_palette_ula_first;
  }
  else   if (!strcmp(tipo,"layer2")) {
    if (segundo) return tbblue_palette_layer2_second;
    else return tbblue_palette_layer2_first;
  }
  else   if (!strcmp(tipo,"sprite")) {
    if (segundo) return tbblue_palette_sprite_second;
    else return tbblue_palette_sprite_first;
  }

  return NULL;

}

int remote_assemble(int misocket,char *texto,unsigned int direccion)
{
	z80_byte destino_ensamblado[MAX_DESTINO_ENSAMBLADO];


				int longitud_destino=assemble_opcode(direccion,texto,destino_ensamblado);

				if (longitud_destino==0) {
					escribir_socket_format(misocket,"Error. Invalid opcode: %s\n",texto);
					return 0;
				}

				else {

					menu_debug_set_memory_zone_attr();
					unsigned int direccion_escribir=direccion;
					int i;
					for (i=0;i<longitud_destino;i++) {
						menu_debug_write_mapped_byte(direccion_escribir++,destino_ensamblado[i]);
					}

					remote_disassemble(misocket,direccion,1,1);
					escribir_socket(misocket,"\n");
				}
		return longitud_destino;
}


#ifdef EMULATE_VISUALMEM

void remote_visualmem_generic_compact(int misocket, z80_byte *buffer, int final_visualmem)
{

	int digitos_max=menu_debug_get_total_digits_hexa(final_visualmem-1);
	int longitud_linea=0;

	int i;
	for (i=0;i<final_visualmem;i++) {
		//Posicion no 0
		if (buffer[i]) {
			if (longitud_linea==0) {
				//Nueva linea
				escribir_socket_format(misocket,"%0*XH ",digitos_max,i);
			}
			escribir_socket_format(misocket,"%u ",buffer[i]);
			buffer[i]=0;

			longitud_linea++;
			if (longitud_linea==16) {
				escribir_socket_format(misocket,"\n");
				longitud_linea=0;
			}
		}

		else {
			//Posicion 0. Si la linea estaba empezada, salto de linea
			if (longitud_linea) {
				escribir_socket_format(misocket,"\n");
				longitud_linea=0;
			}
		}
	}
}
#endif

//Leer todos los parametros en un solo array de char
//Maximo es 65536*4, para permitir comando largo write-mapped-memory que pueda escribir en 64kb de memoria,
//teniendo en cuenta que un numero como maximo ocupa 3 caracteres + 1 espacio
//Damos 10 bytes de mas de margen por si acaso
//char parametros[MAX_LENGTH_PROTOCOL_COMMAND];


//char *buffer_lectura_socket;

//para poder repetir comando anterior solo pulsando enter
//char *buffer_lectura_socket_anterior;

//char *parametros;

void interpreta_comando(char *comando,int misocket,char *buffer_lectura_socket_anterior,int *remote_salir_conexion_cliente,char *ip_source_address)
{

	char buffer_retorno[2048];
    char *parametros;
    int remote_command_argc;

    //punteros de parametros
    char *remote_command_argv[REMOTE_MAX_PARAMETERS_COMMAND];

	int longitud_comando=strlen(comando);

	if (longitud_comando<DEBUG_MAX_MESSAGE_LENGTH) {
		debug_printf (VERBOSE_DEBUG,"Remote command: length: %d [%s]",longitud_comando,comando);
	}

	else {
		debug_printf (VERBOSE_DEBUG,"Remote command: length: %d",longitud_comando);
	}

	//Si enter y setting de repetir comando anterior solo pulsando enter
	if (  (comando[0]==0  || comando[0]=='\n' || comando[0]=='\r')
		&&
		(remote_debug_settings&16)
		) {
		strcpy(comando,buffer_lectura_socket_anterior);
		debug_printf (VERBOSE_DEBUG,"Repeating last command: length: %d [%s]",strlen(comando),comando);
		escribir_socket_format(misocket,"Repeating: %s\n",comando);
	}

	else {
		//Si no, copiar a texto comando anterior el actual
		strcpy(buffer_lectura_socket_anterior,comando);
	}

	//printf ("%d %d %d %d",comando[0],comando[1],comando[2],comando[3]);

	//No hacer caso de comandos con enter, cr, o espacios
	//Si comando nulo, volver
	int i;
	int comando_ignorar=1;

	i=0;
	while (comando[i]) {
		char c=comando[i];
		if (c!=10 && c!=13) comando_ignorar=0;
		i++;
	}

	if (comando_ignorar) {
		//En caso de salir de assemble
		remote_protocol_assembling.v=0;
		return;
	}

	if (comando[0]==0 || (comando[0]==10 && comando[1]==0) ) {
		//debug_printf(VERBOSE_DEBUG,"Command is null");
		//sleep (1);
		return;
	}





    //Interpretar comando hasta espacio o final de linea, o sea sin tener en cuenta parametros
    char comando_sin_parametros[1024];

    for (i=0;comando[i] && comando[i]!=' ' && comando[i]!='\n' && comando[i]!='\r';i++) {
      comando_sin_parametros[i]=comando[i];
    }

    comando_sin_parametros[i]=0;

	parametros=&comando[i];

	//parametros[0]=0;
	int pindex=0;
	if (comando[i]==' ') {
		i++;
		parametros++;
		for (;comando[i] && comando[i]!='\n' && comando[i]!='\r';i++,pindex++) {
			//printf ("searching for end command index: %d\n",i);
			//parametros[pindex]=comando[i];
		}
	}

	parametros[pindex]=0;

	debug_printf (VERBOSE_DEBUG,"Remote command without parameters: length: %d [%s]",strlen(comando_sin_parametros),comando_sin_parametros);

	if (strlen(parametros)<DEBUG_MAX_MESSAGE_LENGTH) {
		debug_printf (VERBOSE_DEBUG,"Remote command parameters: length: %d [%s]",strlen(parametros),parametros);
	}
	else {
		debug_printf (VERBOSE_DEBUG,"Remote command parameters: length: %d",strlen(parametros));
	}


	//Si en modo assembling. Juntamos comando y parametros
	if (remote_protocol_assembling.v) {

		//Si cadena vacia
		//if (comando_sin_parametros[0]==0) {
		//	remote_protocol_assembling.v=0;
		//}

		//else {
			char comando_final[2048];
			sprintf (comando_final,"%s %s",comando_sin_parametros,parametros);
			int longitud=remote_assemble(misocket,comando_final,direccion_assembling);
			//printf ("longitud ensamblado: %d\n",longitud);
			if (longitud==0) {
				//Error
				remote_protocol_assembling.v=0;
			}
			else {
				direccion_assembling +=longitud;
			}
		//}

		return;
	}

    //Si limitados comandos a zeng online cuando zeng online server activado
    if (zeng_online_enabled && zeng_online_server_allow_zrcp_only_zeng_online.v) {
        //Permitir help, about, zeng-online y poco mas
        if (!
            (
                !strcmp(comando_sin_parametros,"help") ||
                !strcmp(comando_sin_parametros,"?") ||
                !strcmp(comando_sin_parametros,"about") ||
                !strcmp(comando_sin_parametros,"quit") ||
                !strcmp(comando_sin_parametros,"get-version") ||
                !strcmp(comando_sin_parametros,"zeng-online") ||
                !strcmp(comando_sin_parametros,"zo")
                //util_string_starts_with(comando_sin_parametros,"zeng-online")
            )
            )

        {
		    escribir_socket (misocket,"Unknown or not allowed command");
            return;
	    }
    }

	//Ver cada comando
	if (!strcmp(comando_sin_parametros,"help") || !strcmp(comando_sin_parametros,"?")) {
		if (parametros[0]==0) {
			escribir_socket(misocket,"Available commands:\n");
			remote_help_no_parameters_command(misocket);


			escribir_socket(misocket,"\nYou can get descriptive help for every command with: help command");

			escribir_socket(misocket,"\nNote 1: When help shows an argument in brackets [], it means the argument is optional, and when written, "
			                  	"you must not write these brackets\n"

								  "Note 2: Some commands allows to specify parameters in \"\" \n"


							);
    }

		else remote_help_command(misocket,parametros);
	}


	else if (!strcmp(comando_sin_parametros,"about")) {
		escribir_socket (misocket,"ZEsarUX remote command protocol");
	}

  else if (!strcmp(comando_sin_parametros,"assemble") || !strcmp(comando_sin_parametros,"a")) {

		//No utilizo remote_parse_commands pues el parametro 2 en adelante puede contener espacios ("INC BC")
	if (parametros[0]==0) {
		escribir_socket(misocket,"ERROR. No parameters set");
	}

	/*else {

		direccion=parse_string_to_number(parametros);

		menu_debug_set_memory_zone_attr();

		//Ver si hay espacio
		char *s=find_space_or_end(parametros);
		while (*s) {*/

		else {
			unsigned int direccion=parse_string_to_number(parametros);
			char *s=find_space_or_end(parametros);
      if (*s) {
				remote_assemble(misocket,s,direccion);
			}
			else {
				direccion_assembling=direccion;
				remote_protocol_assembling.v=1; //Entrar en modo assembly
			}
  	}
	}

  //ATDT easter egg
	else if (comando_sin_parametros[0]=='A' && comando_sin_parametros[1]=='T' && comando_sin_parametros[2]=='D' && comando_sin_parametros[3]=='T') {
		escribir_socket (misocket,"NO CARRIER");
	}

  else if (!strcmp(comando_sin_parametros,"ayplayer") || !strcmp(comando_sin_parametros,"ayp")) {
    remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

    if (remote_command_argc<1) {
      escribir_socket(misocket,"ERROR. Needs one parameter at least");
      return;
    }


    remote_ayplayer(misocket,remote_command_argv[0],remote_command_argv[1]);
  }


 else if (!strcmp(comando_sin_parametros,"clear-membreakpoints")) {
	 clear_mem_breakpoints();
  }

 else if (!strcmp(comando_sin_parametros,"close-all-menus")) {

    if (menu_abierto) {
        //Ir a menu principal
        puerto_especial2 &=(255-16); //pulsar F5
        usleep(100000); //pausa de 0.1 segundos

        puerto_especial2 |=16; //liberar F5
        usleep(100000); //pausa de 0.1 segundos
    }


    int timeout=10;
    //Si esta menu abierto con un tooltip, es necesario hacer esto al menos dos veces

    while (menu_abierto && timeout) {

        //Y cerrar
        puerto_especial1 &=(255-1); //Pulsar ESC
        usleep(100000); //pausa de 0.1 segundos

        puerto_especial1 |=1; //Liberar ESC
        usleep(100000); //pausa de 0.1 segundos

        timeout--;
    }

    /*
    TODO: lo mejor seria usar zrcp_command_close_all_menus pero tiene varios problemas:
    - requiere evento para leer tecla: movimiento raton por ejemplo
    - no cierra menus cuando hay un tooltip abierto
    - no va dentro de debug cpu (probablemente porque espera tambien pulsacion tecla)

    zrcp_command_close_all_menus=1;
    */

   /*
    if (menu_abierto) {
        zrcp_command_close_all_menus=1;
        usleep(100000); //pausa de 0.1 segundos
    }
    */

    if (menu_abierto) {
        escribir_socket(misocket,"ERROR. Can not close all menus");
    }
  }

  else if (!strcmp(comando_sin_parametros,"cpu-code-coverage") ) {
    remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

    if (remote_command_argc<1) {
      escribir_socket(misocket,"ERROR. Needs at least one parameter");
      return;
    }

	//Asignar punteros. Si no existen parametros adicionales, cadenas vacias

	//En este caso un parametros adicionales. Inicializados a cadenas vacias
	char *second_parameter="";
	//second_parameter=NULL;


	if (remote_command_argc>1) second_parameter=remote_command_argv[1];


    remote_cpu_code_coverage(misocket,remote_command_argv[0],second_parameter);
  }


  else if (!strcmp(comando_sin_parametros,"cpu-history") ) {
    remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

    if (remote_command_argc<1) {
      escribir_socket(misocket,"ERROR. Needs at least one parameter");
      return;
    }

	//Asignar punteros. Si no existen parametros adicionales, cadenas vacias

	//En este caso dos parametros adicionales. Inicializados a cadenas vacias
	char *second_parameter="";
	//second_parameter=NULL;

	char *third_parameter="";
	//third_parameter=NULL;

	if (remote_command_argc>1) second_parameter=remote_command_argv[1];
	if (remote_command_argc>2) third_parameter=remote_command_argv[2];

	//El primero y segundo parametro no seran nunca cero porque ya estamos haciendo comprobacion por remote_command_argc antes
    remote_cpu_history(misocket,remote_command_argv[0],second_parameter,third_parameter);
  }






  else if (!strcmp(comando_sin_parametros,"cpu-panic")) {
	//Entramos en el mismo modo que cpu-step para poder congelar la emulacion
	remote_cpu_enter_step(misocket);
	if (menu_event_remote_protocol_enterstep.v==0) return;

	cpu_panic(parametros);
  }

  else if (!strcmp(comando_sin_parametros,"cpu-step") || !strcmp(comando_sin_parametros,"cs")) {
    remote_cpu_step(misocket);
  }

  else if (!strcmp(comando_sin_parametros,"cpu-step-over") || !strcmp(comando_sin_parametros,"cso")) {
    remote_cpu_step_over(misocket);
  }


  else if (!strcmp(comando_sin_parametros,"cpu-transaction-log") ) {
    remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

    if (remote_command_argc<2) {
      escribir_socket(misocket,"ERROR. Needs two parameters");
      return;
    }


    remote_cpu_transaction_log(misocket,remote_command_argv[0],remote_command_argv[1]);
  }


	else if (!strcmp(comando_sin_parametros,"debug-analyze-command")) {
		//Parseamos los parametros porque me sirven para debugar
		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

		int i;

		for (i=0;i<remote_command_argc;i++) {
			//printf ("Parametro %d: [%s]\n",i,remote_command_argv[i]);
			escribir_socket_format(misocket,"Parameter %d: [%s]\n",i,remote_command_argv[i]);
		}
	}

  else if (!strcmp(comando_sin_parametros,"disable-breakpoint") || !strcmp(comando_sin_parametros,"db")) {
    if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. You must enable breakpoints first");
    else remote_disable_breakpoint(misocket,parametros);
  }

  //disable-breakpoints
    else if (!strcmp(comando_sin_parametros,"disable-breakpoints")) {
      if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. Already disabled");
      else {
              debug_breakpoints_enabled.v=0;
              //debug_cpu_core_loop.v=0;
              breakpoints_disable();
      }
    }


  else if (!strcmp(comando_sin_parametros,"disassemble") || !strcmp(comando_sin_parametros,"d")) {
    unsigned int direccion;
    int lineas=1;
    if (parametros[0]==0) {
      direccion=get_pc_register();
    }
    else {

      direccion=parse_string_to_number(parametros);
      //Ver si hay espacio
      char *s=find_space_or_end(parametros);
      if (*s) lineas=parse_string_to_number(s);

    }
    remote_disassemble(misocket,direccion,lineas,1);

  }

	else if (!strcmp(comando_sin_parametros,"dump-nested-functions")) {
		remote_dump_nested_core_functions(misocket);
	}

	else if (!strcmp(comando_sin_parametros,"dump-scanline-buffer")) {

		int limite=screen_testados_linea*2; //2 pixeles por cada t-estado

		limite /=8; //array es de bloques de 8 pixeles (espacio de atributo)

		limite *=2; //y son pares de 8-pixel (byte) y atributo

		int i;
		for (i=0;i<limite;i++) escribir_socket_format (misocket,"%02X ",scanline_buffer[i]);


	}

  else if (!strcmp(comando_sin_parametros,"easter-egg")) {
    remote_easter_egg(misocket);
  }


  else if (!strcmp(comando_sin_parametros,"enable-breakpoint") || !strcmp(comando_sin_parametros,"eb")) {
    if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. You must enable breakpoints first");
    else remote_enable_breakpoint(misocket,parametros);
  }

  //enable-breakpoints
    else if (!strcmp(comando_sin_parametros,"enable-breakpoints")) {
      if (debug_breakpoints_enabled.v==0) {
              debug_breakpoints_enabled.v=1;
              breakpoints_enable();
      }


      else {
        escribir_socket (misocket,"Error. Already enabled");
      }

    }

  else if (!strcmp(comando_sin_parametros,"enter-cpu-step") || !strcmp(comando_sin_parametros,"encs")) {
    remote_cpu_enter_step(misocket);
  }

	else if (!strcmp(comando_sin_parametros,"esxdoshandler-get-open-files") || !strcmp(comando_sin_parametros,"esxgof")) {
		if (esxdos_handler_enabled.v==0) escribir_socket(misocket,"Error. Handler is not enabled");
		else remote_esxdos_gof(misocket);
  }


  else if (!strcmp(comando_sin_parametros,"evaluate") || !strcmp(comando_sin_parametros,"e")) {
    if (parametros[0]==0) {
      escribir_socket(misocket,"Error. No expression");
    }
    else {
      remote_evaluate(misocket,parametros);
    }
  }


   /*else if (!strcmp(comando_sin_parametros,"evaluate-condition") || !strcmp(comando_sin_parametros,"ec")) {
    if (parametros[0]==0) {
      escribir_socket(misocket,"Error. No expression");
    }
    else {
      int result=debug_breakpoint_condition_loop(parametros,1);
      escribir_socket_format(misocket,"Result: %s -> %s",parametros,(result ? "True" : "False " ));
    }
  }*/



  else if (!strcmp(comando_sin_parametros,"exit-cpu-step") || !strcmp(comando_sin_parametros,"ecs")) {
    remote_cpu_exit_step(misocket);
  }

  else if (!strcmp(comando_sin_parametros,"exit-emulator")) {
		escribir_socket (misocket,"Sayonara baby\n");
		sleep(1);

		remote_calling_end_emulator.v=1;
		end_emulator_autosave_snapshot();
  }


  else if (!strcmp(comando_sin_parametros,"extended-stack") ) {

	  if (!CPU_IS_Z80) {
		  escribir_socket(misocket,"ERROR. CPU is not Z80");
		  return;
	  }

    remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

    if (remote_command_argc<1) {
      escribir_socket(misocket,"ERROR. Needs at least one parameter");
      return;
    }



	//Asignar punteros. Si no existen parametros adicionales, cadenas vacias

	//En este caso solo un parametro adicional
	char *third_parameter="";
	//third_parameter=NULL;

	if (remote_command_argc>2) third_parameter=remote_command_argv[2];


	//El primero y segundo parametro no seran nunca cero porque ya estamos haciendo comprobacion por remote_command_argc antes
    remote_extended_stack(misocket,remote_command_argv[0],remote_command_argv[1],third_parameter);
  }



  else if (!strcmp(comando_sin_parametros,"find-label")) {
		if (remote_tamanyo_archivo_raw_source_code==0) {
			escribir_socket (misocket,"ERROR. No source code loaded");
		}
		else {
			int linea=remote_find_label_source_code(parametros);
			if (linea!=-1) {
				int indice=remote_parsed_source_code_indexes_pointer[linea];
        char *puntero=&remote_raw_source_code_pointer[indice];
				escribir_socket (misocket,puntero);
			}
			else {
				escribir_socket (misocket,"ERROR. Not found");
			}
		}
  }


  else if (!strcmp(comando_sin_parametros,"generate-nmi")) {
	generate_nmi();
  }


	else if (!strcmp(comando_sin_parametros,"get-audio-buffer-info")) {
		int tamanyo,posicion;
		audio_get_buffer_info(&tamanyo,&posicion);
		escribir_socket_format(misocket,"Total size: %d Current size: %d\n",tamanyo,posicion);
	}

//get-breakpoints, estado global y lista cada uno, si hay y si esta enabled
  else if (!strcmp(comando_sin_parametros,"get-breakpoints") || !strcmp(comando_sin_parametros,"gb")) {
	int inicio=1;
	int items=MAX_BREAKPOINTS_CONDITIONS;

		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
                if (remote_command_argc>0) {
			inicio=parse_string_to_number(remote_command_argv[0]);
			if (inicio<1 || inicio>MAX_BREAKPOINTS_CONDITIONS) {
				escribir_socket (misocket,"ERROR. Index out of range");
				return;
			}
			items=1;
                }

                if (remote_command_argc>1) {
			items=parse_string_to_number(remote_command_argv[1]);
                }


    remote_get_breakpoints(misocket,inicio-1,items);
  }

	//get-breakpoints, estado global y lista cada uno, si hay y si esta enabled
	  else if (!strcmp(comando_sin_parametros,"get-breakpointsactions") || !strcmp(comando_sin_parametros,"gba")) {
        int inicio=1;
        int items=MAX_BREAKPOINTS_CONDITIONS;

                remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
                if (remote_command_argc>0) {
                        inicio=parse_string_to_number(remote_command_argv[0]);
			if (inicio<1 || inicio>MAX_BREAKPOINTS_CONDITIONS) {
				escribir_socket (misocket,"ERROR. Index out of range");
				return;
			}
			items=1;
                }

                if (remote_command_argc>1) {
                        items=parse_string_to_number(remote_command_argv[1]);
                }


	    remote_get_breakpointsactions(misocket,inicio-1,items);
	  }

	  else if (!strcmp(comando_sin_parametros,"get-breakpoints-optimized") ) {
        int inicio=1;
	int items=MAX_BREAKPOINTS_CONDITIONS;

		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
                if (remote_command_argc>0) {
			inicio=parse_string_to_number(remote_command_argv[0]);
			if (inicio<1 || inicio>MAX_BREAKPOINTS_CONDITIONS) {
				escribir_socket (misocket,"ERROR. Index out of range");
				return;
			}
			items=1;
                }

                if (remote_command_argc>1) {
			items=parse_string_to_number(remote_command_argv[1]);
                }


    remote_get_breakpoints_optimized(misocket,inicio-1,items);
	  }

	else if (!strcmp(comando_sin_parametros,"get-buildnumber")) {
                escribir_socket (misocket,BUILDNUMBER);
	}


	else if (!strcmp(comando_sin_parametros,"get-cpu-core-name")) {
		if (cpu_core_loop_name!=NULL) escribir_socket(misocket,cpu_core_loop_name);
	}

	else if (!strcmp(comando_sin_parametros,"get-cpu-frequency")) {
		escribir_socket_format(misocket,"%d",get_cpu_frequency() );
	}

	else if (!strcmp(comando_sin_parametros,"get-cpu-turbo-speed")) {
		escribir_socket_format(misocket,"%d",cpu_turbo_speed );
	}



  else if (!strcmp(comando_sin_parametros,"get-crc32")) {

    remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

    if (remote_command_argc<2) {
      escribir_socket(misocket,"ERROR. Needs two parameters");
      return;
    }

    /*if (return_internal_pointer(remote_command_argv[0],&inicio)==0) {
      escribir_socket(misocket,"ERROR. Unknown pointer");
      return;
    }*/

    int start_address=parse_string_to_number(remote_command_argv[0]);
    int length=parse_string_to_number(remote_command_argv[1]);

    if (length<1) {
      escribir_socket(misocket,"ERROR. Length must be >0");
      return;
    }


    //Copiar contenido memoria segun memory zone activa a buffer de memoria temporal
    z80_byte *memoria_temporal;
    memoria_temporal=malloc(length);
    if (memoria_temporal==NULL) cpu_panic("Can not allocate memory for crc32 calculation");


    menu_debug_set_memory_zone_attr();
    int longitud_copiar=length;
    int i;

    for (i=0;longitud_copiar>0;i++,longitud_copiar--) {
      z80_byte byte_leido=menu_debug_get_mapped_byte(start_address+i);
      memoria_temporal[i]=byte_leido;
    }


    //z80_long_int crc32=util_crc32_calculation(memoria_spectrum,16384);
    z80_long_int crc32=util_crc32_calculation(0,memoria_temporal,length);
    escribir_socket_format (misocket,"%08x",crc32);

    free(memoria_temporal);

  }




  else if (!strcmp(comando_sin_parametros,"get-current-machine") || !strcmp(comando_sin_parametros,"gcm")) {
    escribir_socket (misocket,get_machine_name(current_machine_type));
  }

	else if (!strcmp(comando_sin_parametros,"get-current-memory-zone") || !strcmp(comando_sin_parametros,"gcmz")) {
		menu_debug_set_memory_zone_attr();

		char zone_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
		int zone=menu_get_current_memory_zone_name_number(zone_name);
		//machine_get_memory_zone_name(menu_debug_memory_zone,buffer_name);

		escribir_socket_format (misocket,"Zone number: %d Name: %s Size: %d (%d KB)", zone,zone_name,
			menu_debug_memory_zone_size,menu_debug_memory_zone_size/1024);

	}

	else if (!strcmp(comando_sin_parametros,"get-debug-settings") || !strcmp(comando_sin_parametros,"gds")) {
		sprintf(buffer_retorno,"%d",remote_debug_settings);
		escribir_socket(misocket,buffer_retorno);
	}



	else if (!strcmp(comando_sin_parametros,"get-io-ports") ) {
        char *stats_buffer=util_malloc_max_texto_generic_message("Can not allocate memory for io ports");

		debug_get_ioports(stats_buffer);
		escribir_socket(misocket,stats_buffer);

        free(stats_buffer);
	}

	else if (!strcmp(comando_sin_parametros,"get-machines")) {
		escribir_socket (misocket,string_machines_list_description);
	}

	else if (!strcmp(comando_sin_parametros,"get-memory-pages") || !strcmp(comando_sin_parametros,"gmp")) {

                remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

                debug_memory_segment segmentos[MAX_DEBUG_MEMORY_SEGMENTS];
                int total_segmentos=debug_get_memory_pages_extended(segmentos);

                int verbose=0;

                if (remote_command_argc>0) {
                 if (!strcmp(remote_command_argv[0],"verbose")) {
                    verbose=1;
                  }
                }


                 int i;

                for (i=0;i<total_segmentos;i++) {

                  if (verbose) {
                    escribir_socket_format(misocket,"Segment %d\nLong name: %s\nShort name: %s\nStart: %XH\nEnd: %XH\n\n",
            i+1,segmentos[i].longname, segmentos[i].shortname, segmentos[i].start, segmentos[i].start+segmentos[i].length-1);
                  }

                  else {
                    escribir_socket_format(misocket,"%s ",segmentos[i].shortname);
                  }
                }


		//char buffer_temporal[MAX_TEXT_DEBUG_GET_MEMORY_PAGES+1];
		//debug_get_memory_pages(buffer_temporal);
		//escribir_socket (misocket,buffer_temporal);
	}

  else if (!strcmp(comando_sin_parametros,"get-membreakpoints") ) {
        int inicio=0;
        int items=65536;

                remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
                if (remote_command_argc>0) {
                        inicio=parse_string_to_number(remote_command_argv[0]);
                        if (inicio<0 || inicio>65535) {
                                escribir_socket (misocket,"ERROR. Address out of range");
                                return;
                        }
                        items=0; //decirle que solo el que vamos a mirar
                }

                if (remote_command_argc>1) {
                        items=parse_string_to_number(remote_command_argv[1]);
			if (items<0 || items>65536) {
                                escribir_socket (misocket,"ERROR. Items out of range");
                                return;
                        }
                }


    remote_get_membreakpoints(misocket,inicio,items);
  }

	else if (!strcmp(comando_sin_parametros,"get-memory-zones") || !strcmp(comando_sin_parametros,"gmz")) {
		remote_get_memory_zones(misocket);
	}



	else if (!strcmp(comando_sin_parametros,"get-ocr")) {
		char buffer_ocr[8192];
		ocr_get_text(buffer_ocr,1);
		escribir_socket (misocket,buffer_ocr);
	}



	else if (!strcmp(comando_sin_parametros,"get-os")) {
		escribir_socket (misocket,COMPILATION_SYSTEM);
	}

  else if (!strcmp(comando_sin_parametros,"get-paging-state")) {
    char buffer[32];
    debug_get_paging_screen_state(buffer);
    escribir_socket (misocket,buffer);
  }

  else if (!strcmp(comando_sin_parametros,"get-registers") || !strcmp(comando_sin_parametros,"gr")) {
    print_registers(buffer_retorno);
    escribir_socket (misocket,buffer_retorno);
  }

	else if (!strcmp(comando_sin_parametros,"get-snapshot")) {
		z80_byte *buffer_temp;
		buffer_temp=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM); //16 MB es mas que suficiente
		if (buffer_temp==NULL) cpu_panic("Can not allocate memory for get-snapshot");

		z80_byte *puntero=buffer_temp;
		int longitud;

  		save_zsf_snapshot_file_mem(NULL,puntero,&longitud,0);

		//printf ("longitud: %d\n",longitud);

		int i;
		for (i=0;i<longitud;i++) {
			escribir_socket_format(misocket,"%02X",buffer_temp[i]);
		}

	 	free(buffer_temp);
	}

	else if (!strcmp(comando_sin_parametros,"get-stack-backtrace")) {

    int items=5;

    remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
    if (remote_command_argc>0) {
      items=parse_string_to_number(remote_command_argv[0]);
      if (items<1) {
        escribir_socket(misocket,"ERROR. Items must be >0");
        return;
      }
    }

		if (CPU_IS_MOTOROLA) {

		}
		else {
			int i;


			for (i=0;i<items;i++) {
				z80_int valor=peek_byte_z80_moto(reg_sp+i*2)+256*peek_byte_z80_moto(reg_sp+1+i*2);
				escribir_socket_format(misocket,"%04XH ",valor);
			}
		}
	}

	else if (!strcmp(comando_sin_parametros,"get-tstates")) {
		escribir_socket_format (misocket,"%d",t_estados);
	}

	else if (!strcmp(comando_sin_parametros,"get-tstates-partial")) {
		char buffer_partial[32];
		debug_get_t_estados_parcial(buffer_partial);
		escribir_socket (misocket,buffer_partial);
	}

	//Este comando no se usa en la funcion de juegos online, aunque lo hago como complemento a set-ui-io-ports, por si resulta util para algo
	else if (!strcmp(comando_sin_parametros,"get-ui-io-ports") ) {

//;                    Bits:  4    3    2    1    0     ;desplazamiento puerto
//puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
//puerto_65022   db    255  ; G    F    D    S    A     ;1
//puerto_64510    db              255  ; T    R    E    W    Q     ;2
//puerto_63486    db              255  ; 5    4    3    2    1     ;3
//puerto_61438    db              255  ; 6    7    8    9    0     ;4
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H                J         K      L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7

		escribir_socket_format(misocket,"%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			puerto_65278,puerto_65022,puerto_64510,puerto_63486,
			puerto_61438,puerto_57342,puerto_49150,puerto_32766,
			puerto_especial_joystick);

	}


	else if (!strcmp(comando_sin_parametros,"get-version")) {
		char buffer[30];
		util_get_emulator_version_number(buffer);

		escribir_socket (misocket,buffer);
	}




	else if (!strcmp(comando_sin_parametros,"get-video-driver")) {
                escribir_socket (misocket,scr_new_driver_name);
	}

#ifdef EMULATE_VISUALMEM
	else if (!strcmp(comando_sin_parametros,"get-visualmem-written-dump") || !strcmp(comando_sin_parametros,"gvmwd") ) {

		int salida_compacta=0;
		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
		if (remote_command_argc>0) {
			if (!strcmp(remote_command_argv[0],"compact")) salida_compacta=1;
		}

		int final_visualmem=65536;
		if (MACHINE_IS_QL) final_visualmem=QL_MEM_LIMIT+1;

		if (salida_compacta) {
			remote_visualmem_generic_compact(misocket,visualmem_buffer,final_visualmem);
			return;
		}

		int digitos_max=menu_debug_get_total_digits_hexa(final_visualmem-1);

		int i;
		for (i=0;i<final_visualmem;i++) {
			if (visualmem_buffer[i]) {
				escribir_socket_format(misocket,"%0*XH %u\n",digitos_max,i,visualmem_buffer[i]);
				clear_visualmembuffer(i);
			}
		}
	}

	else if (!strcmp(comando_sin_parametros,"get-visualmem-read-dump") || !strcmp(comando_sin_parametros,"gvmrd")) {

		int salida_compacta=0;
		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
		if (remote_command_argc>0) {
			if (!strcmp(remote_command_argv[0],"compact")) salida_compacta=1;
		}

		int final_visualmem=65536;
		if (MACHINE_IS_QL) final_visualmem=QL_MEM_LIMIT+1;

		if (salida_compacta) {
			remote_visualmem_generic_compact(misocket,visualmem_read_buffer,final_visualmem);
			return;
		}

		int digitos_max=menu_debug_get_total_digits_hexa(final_visualmem-1);

		int i;
		for (i=0;i<final_visualmem;i++) {
			if (visualmem_read_buffer[i]) {
				escribir_socket_format(misocket,"%0*XH %u\n",digitos_max,i,visualmem_read_buffer[i]);
				clear_visualmemreadbuffer(i);
			}
		}
	}

	else if (!strcmp(comando_sin_parametros,"get-visualmem-opcode-dump") || !strcmp(comando_sin_parametros,"gvmod")) {

		int salida_compacta=0;
		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
		if (remote_command_argc>0) {
			if (!strcmp(remote_command_argv[0],"compact")) salida_compacta=1;
		}

		int final_visualmem=65536;
		if (MACHINE_IS_QL) final_visualmem=QL_MEM_LIMIT+1;

		if (salida_compacta) {
			remote_visualmem_generic_compact(misocket,visualmem_opcode_buffer,final_visualmem);
			return;
		}

		int digitos_max=menu_debug_get_total_digits_hexa(final_visualmem-1);

		int i;
		for (i=0;i<final_visualmem;i++) {
			if (visualmem_opcode_buffer[i]) {
				escribir_socket_format(misocket,"%0*XH %u\n",digitos_max,i,visualmem_opcode_buffer[i]);
				clear_visualmemopcodebuffer(i);
			}
		}
	}
#endif






  else if (!strcmp(comando_sin_parametros,"hard-reset-cpu")) {
          hard_reset_cpu();
  }


	else if (!strcmp(comando_sin_parametros,"hexdump") || !strcmp(comando_sin_parametros,"h")) {
		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

		if (remote_command_argc!=2) {
			escribir_socket(misocket,"ERROR. Needs two parameters");
			return;
		}

		int inicio=parse_string_to_number(remote_command_argv[0]);
		int longitud=parse_string_to_number(remote_command_argv[1]);

        //temp simular un comando que va lento
        //sleep(10);

        //printf("Despues sleep\n");

		remote_hexdump(misocket,inicio,longitud);

        //printf("Despues remote_hexdump\n");

	}

	else if (!strcmp(comando_sin_parametros,"hexdump-internal")) {
		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

		if (remote_command_argc<2) {
			escribir_socket(misocket,"ERROR. Needs two parameters minimum");
			return;
		}


		z80_byte *inicio;


		if (return_internal_pointer(remote_command_argv[0],&inicio)==0) {
			escribir_socket(misocket,"ERROR. Unknown pointer");
			return;
		}

		if (inicio==NULL) {
			escribir_socket(misocket,"ERROR. Pointer is null");
			return;
		}

		int longitud=parse_string_to_number(remote_command_argv[1]);

		int offset=0;
		if (remote_command_argc>2) offset=parse_string_to_number(remote_command_argv[2]);

		remote_hexdump_internal(misocket,inicio,longitud,offset);

	}

	else if (!strcmp(comando_sin_parametros,"kartusho-press-button")) {
			kartusho_press_button();
	}

	else if (!strcmp(comando_sin_parametros,"ifrom-press-button")) {
			ifrom_press_button();
	}


	else if (!strcmp(comando_sin_parametros,"load-binary")) {
                remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);




                if (remote_command_argc<3) {
                        escribir_socket(misocket,"ERROR. Needs three parameters");
                        return;
                }

		//int load_binary_file(char *binary_file_load,int valor_leido_direccion,int valor_leido_longitud)

		char *archivo;
		archivo=remote_command_argv[0];

		int valor_leido_direccion=parse_string_to_number(remote_command_argv[1]);

		int valor_leido_longitud=parse_string_to_number(remote_command_argv[2]);

		int retorno=load_binary_file(archivo,valor_leido_direccion,valor_leido_longitud);

		if (retorno) escribir_socket(misocket,"ERROR loading file");


	}


	else if (!strcmp(comando_sin_parametros,"load-source-code") || !strcmp(comando_sin_parametros,"lsc")) {
		int retorno=remote_load_source_code(parametros);

        if (retorno) {
            escribir_socket(misocket,"ERROR loading source code");
        }
	}

	else if (!strcmp(comando_sin_parametros,"ls")) {
			remote_simple_help(misocket);
	}

	else if (!strcmp(comando_sin_parametros,"mmc-reload")) {
        if (mmc_enabled.v==0) {
            escribir_socket(misocket,"ERROR. MMC is not enabled");
        }

        else {
            if (mmc_read_file_to_memory()==0) {
                escribir_socket(misocket,"OK. MMC file reloaded");
            }
            else {
               escribir_socket(misocket,"ERROR reloading MMC");
            }
        }
	}

	else if (!strcmp(comando_sin_parametros,"noop")) {
		//No hacer absolutamente nada
	}

	else if (!strcmp(comando_sin_parametros,"open-menu")) {
		menu_fire_event_open_menu();
	}

	else if (!strcmp(comando_sin_parametros,"print-error") ) {

        remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

        if (remote_command_argc<1) {
                escribir_socket(misocket,"ERROR. No message set");
                return;
        }

		debug_printf(VERBOSE_ERR,"%s",remote_command_argv[0]);

	}

	else if (!strcmp(comando_sin_parametros,"print-footer") ) {

		if (parametros[0]==0) {
			escribir_socket(misocket,"ERROR. No message set");
		}

		else {

			char *s=parametros;
			//int parametros_recibidos=0;
			int i=0;

			char mensaje[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH];

			while (*s && i<AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH-1) {
				mensaje[i++]=*s;
				s++;
			}

			//printf ("i: %d\n",i);
			mensaje[i]=0;

			put_footer_first_message(mensaje);

			//Y enviarlo a speech
			textspeech_print_speech(parametros);


		}

	}

    //Este comando se usa en ZENG
	else if (!strcmp(comando_sin_parametros,"put-snapshot") ) {
		/*
		Se puede enviar un snapshot por script asi:
		#!/usr/bin/env bash

		( sleep 1 ; echo -n "put-snapshot " ; cat pruebasnap.txt ; sleep 1 ) | telnet localhost 10000
		//Teniendo en cuenta que pruebasnap.txt contiene el volcado que se ha generado con get-snapshot
		 */
		z80_byte valor;
		if (parametros[0]==0) {
			escribir_socket(misocket,"ERROR. No parameters set");
		}

		else {

			char *s=parametros;
			int parametros_recibidos=0;

			z80_byte *buffer_destino;
			buffer_destino=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2);
			if (buffer_destino==NULL) cpu_panic("Can not allocate memory for put-snapshot");


			while (*s) {
				char buffer_valor[4];
				buffer_valor[0]=s[0];
				buffer_valor[1]=s[1];
				buffer_valor[2]='H';
				buffer_valor[3]=0;
				//printf ("%s\n",buffer_valor);
				valor=parse_string_to_number(buffer_valor);
				//printf ("valor: %d\n",valor);

				buffer_destino[parametros_recibidos++]=valor;
				//menu_debug_write_mapped_byte(direccion++,valor);

				s++;
				if (*s) s++;
			}

			//Enviarlo como snapshot, pero al final de un frame de pantalla
			//load_zsf_snapshot_file_mem(NULL,buffer_destino,parametros_recibidos);
			pending_zrcp_put_snapshot_buffer_destino=buffer_destino;
			pending_zrcp_put_snapshot_longitud=parametros_recibidos;



		}

	}


	else if (!strcmp(comando_sin_parametros,"qdos-get-open-files") || !strcmp(comando_sin_parametros,"qlgof")) {
		if (!MACHINE_IS_QL) escribir_socket(misocket,"Error. Machine is not QL");
		else remote_qdos_gof(misocket);
  }

	else if (!strcmp(comando_sin_parametros,"read-memory")) {
		unsigned int inicio=0;
		unsigned int longitud=0;

		if (parametros[0]!=0) {

			inicio=parse_string_to_number(parametros);
			longitud=1;
			//Ver si hay espacio
      char *s=find_space_or_end(parametros);
      if (*s) longitud=parse_string_to_number(s);

		}
		remote_get_memory(misocket,inicio,longitud);

	}


	else if (!strcmp(comando_sin_parametros,"read-mapped-memory")) {
		escribir_socket(misocket,"This command is no longer supported. Use read-memory command and set memory zone if needed");
	}

    else if (!strcmp(comando_sin_parametros,"realtape-open")) {

        remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
        if (remote_command_argc<1) {
            escribir_socket(misocket,"ERROR. No parameter set");
            return;
        }


        //Si estamos en cpu-step-mode, cargar tal cual y volver sin tocar modo
        //Si no, entrar cpu-step-mode y luego quitar cpu-step-mode

        z80_bit antes_menu_event_remote_protocol_enterstep;

        antes_menu_event_remote_protocol_enterstep.v=menu_event_remote_protocol_enterstep.v;


        if (antes_menu_event_remote_protocol_enterstep.v==0) {
            //Asegurarnos que congelamos el emulador: abrir menu con mutitarea desactivada
            //Entramos en el mismo modo que cpu-step para poder congelar la emulacion
            remote_cpu_enter_step(misocket);
            if (menu_event_remote_protocol_enterstep.v==0) return;
        }



        strcpy(menu_realtape_name,remote_command_argv[0]);
        realtape_name=menu_realtape_name;
        realtape_insert();

        if (antes_menu_event_remote_protocol_enterstep.v==0) {
            remote_cpu_exit_step(misocket);
        }

    }


  else if (!strcmp(comando_sin_parametros,"reset-cpu")) {
          reset_cpu();
  }

	else if (!strcmp(comando_sin_parametros,"reset-tstates-partial")) {
		debug_t_estados_parcial=0;
	}

  else if (!strcmp(comando_sin_parametros,"run") || !strcmp(comando_sin_parametros,"r")) {
    int verbose=0;
    int limit=0;

    int par=0;
		int datosvuelve=1;
		int update_immediately=0;

    remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

    //ver cada parametro. pueden venir en diferente orden
    for (par=0;par<remote_command_argc;par++) {
      if (!strcasecmp(remote_command_argv[par],"verbose")) verbose=1;
      else if (!strcasecmp(remote_command_argv[par],"no-stop-on-data")) datosvuelve=0;
			else if (!strcasecmp(remote_command_argv[par],"update-immediately")) update_immediately=1;
      else limit=parse_string_to_number(remote_command_argv[par]);
    }


    //if (parametros[0]!=0) verbose=1;
	char texto_evento_data[100];
	if (datosvuelve) strcpy(texto_evento_data,"key press or data sent, ");
	else texto_evento_data[0]=0;

    if (limit==0) escribir_socket_format(misocket,"Running until a breakpoint, %smenu opening or other event\n",texto_evento_data);
    else escribir_socket_format(misocket,"Running until a breakpoint, %smenu opening, %d opcodes run, or other event\n",texto_evento_data,limit);
    remote_cpu_run(misocket,verbose,limit,datosvuelve,update_immediately);
  }


	else if (!strcmp(comando_sin_parametros,"save-binary")) {
		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);


		if (remote_command_argc<3) {
				escribir_socket(misocket,"ERROR. Needs three parameters");
				return;
		}


		char *archivo;
		archivo=remote_command_argv[0];

		int valor_leido_direccion=parse_string_to_number(remote_command_argv[1]);

		int valor_leido_longitud=parse_string_to_number(remote_command_argv[2]);

		int retorno=save_binary_file(archivo,valor_leido_direccion,valor_leido_longitud);

		if (retorno) escribir_socket(misocket,"ERROR loading file");


	}


	else if (!strcmp(comando_sin_parametros,"save-binary-internal")) {
		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

		if (remote_command_argc<3) {
			escribir_socket(misocket,"ERROR. Needs three parameters minimum");
			return;
		}


		z80_byte *inicio;

		if (return_internal_pointer(remote_command_argv[0],&inicio)==0) {
			escribir_socket(misocket,"ERROR. Unknown pointer");
			return;
		}

		if (inicio==NULL) {
			escribir_socket(misocket,"ERROR. Pointer is null");
			return;
		}

		int longitud=parse_string_to_number(remote_command_argv[1]);

		char *binaryfile;

		binaryfile=remote_command_argv[2];

		int offset=0;
		if (remote_command_argc>3) offset=parse_string_to_number(remote_command_argv[3]);

		inicio +=offset;

		FILE *ptr_binaryfile;
				ptr_binaryfile=fopen(binaryfile,"wb");
				if (!ptr_binaryfile)
			{
						debug_printf (VERBOSE_ERR,"Unable to open file");
				}
			else {

							int i;
							for (i=0;i<longitud;i++) {
											fwrite(inicio,1,1,ptr_binaryfile);
											inicio++;
							}

						 fclose(ptr_binaryfile);
			}


	}


	else if (!strcmp(comando_sin_parametros,"save-screen") ) {

		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
		if (remote_command_argc<1) {
			escribir_socket(misocket,"ERROR. No parameter set");
			return;
		}


		save_screen(remote_command_argv[0]);


  }

	else if (!strcmp(comando_sin_parametros,"send-keys-event")) {

		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

		if (remote_command_argc<2) {
			escribir_socket(misocket,"ERROR. Needs two parameters minimum");
			return;
		}




		int tecla=parse_string_to_number(remote_command_argv[0]);
		int evento=parse_string_to_number(remote_command_argv[1]);
		int nomenu=0;

		if (remote_command_argc==3) nomenu=parse_string_to_number(remote_command_argv[2]);

		int enviar=1;
		if (nomenu && menu_abierto) enviar=0;


		//Enviar la tecla pero que no vuelva a entrar por zeng
		if (enviar) {
			debug_printf (VERBOSE_PARANOID,"Processing from ZRCP command send-keys-event: key %d event %d",tecla,evento);

            debug_printf (VERBOSE_PARANOID,"Info joystick: fire: %d up: %d down: %d left: %d right: %d",
                UTIL_KEY_JOY_FIRE,UTIL_KEY_JOY_UP,UTIL_KEY_JOY_DOWN,UTIL_KEY_JOY_LEFT,UTIL_KEY_JOY_RIGHT);

            //Si tecla especial de reset todas teclas. usado en driver curses
            if (tecla==UTIL_KEY_RESET_ALL) {
                //printf("Reset todas teclas\n");
                reset_keyboard_ports();
            }

            else {
			    util_set_reset_key_continue_after_zeng(tecla,evento);
            }
		}


	}


	else if (!strcmp(comando_sin_parametros,"send-keys-string")) {
		//remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

		/*if (remote_command_argc<2) {
			escribir_socket(misocket,"ERROR. Needs two parameters minimum");
			return;
		}*/

		z80_byte tecla;

		//No separamos en argv porque interesa tener toda la string a enviar, con sus espacios y todo
		int pausa=parse_string_to_number(parametros);

		char *s=find_space_or_end(parametros);
		if ( (*s)==0) {
			escribir_socket(misocket,"ERROR. Needs two parameters minimum");
			return;
		}

		for (;*s;s++) {
			tecla=*s;
			ascii_to_keyboard_port(tecla);
			usleep(pausa*1000);
			reset_keyboard_ports();
			usleep(pausa*1000);
		}


	}

	else if (!strcmp(comando_sin_parametros,"send-keys-ascii")) {
		remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

		if (remote_command_argc<2) {
			escribir_socket(misocket,"ERROR. Needs two parameters minimum");
			return;
		}

		z80_byte tecla;

		int pausa=parse_string_to_number(remote_command_argv[0]);

		int i;
		for (i=1;i<remote_command_argc;i++) {
			tecla=parse_string_to_number(remote_command_argv[i]);
			//printf ("Enviando tecla %d\n",tecla);
			ascii_to_keyboard_port(tecla);
			usleep(pausa*1000);
			reset_keyboard_ports();
			usleep(pausa*1000);
		}


	}

//set-breakpoint - si breakpoint activo. id numero y texto
else if (!strcmp(comando_sin_parametros,"set-breakpoint") || !strcmp(comando_sin_parametros,"sb")) {
  if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. You must enable breakpoints first");
  else remote_set_breakpoint(misocket,parametros);
}

else if (!strcmp(comando_sin_parametros,"set-breakpointaction") || !strcmp(comando_sin_parametros,"sba")) {
  if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. You must enable breakpoints first");
  else remote_set_breakpointaction(misocket,parametros);
}

else if (!strcmp(comando_sin_parametros,"set-cr")) {
	enviar_cr=1;
}

else if (!strcmp(comando_sin_parametros,"set-debug-settings") || !strcmp(comando_sin_parametros,"sds")) {
	if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
	else remote_debug_settings=parse_string_to_number(parametros);
}

//Este comando se usa (o se usara) en la funcion de juegos online
else if (!strcmp(comando_sin_parametros,"set-ui-io-ports") ) {
		z80_byte valor;
		if (parametros[0]==0) {
			escribir_socket(misocket,"ERROR. No parameters set");
		}

		else {

			char *s=parametros;
			int parametros_recibidos=0;

#define ZRCP_CMD_SET_UI_IO_PORTS_TOTAL_PARAMS 9


			z80_byte buffer_destino[ZRCP_CMD_SET_UI_IO_PORTS_TOTAL_PARAMS];

			while (*s) {
				char buffer_valor[4];
				buffer_valor[0]=s[0];
				buffer_valor[1]=s[1];
				buffer_valor[2]='H';
				buffer_valor[3]=0;
				//printf ("%s\n",buffer_valor);
				valor=parse_string_to_number(buffer_valor);
				//printf ("valor: %d\n",valor);

				buffer_destino[parametros_recibidos++]=valor;
				//menu_debug_write_mapped_byte(direccion++,valor);

				s++;
				if (*s) s++;
			}

			//Ver si total de parametros recibidos correctos
			if (parametros_recibidos!=ZRCP_CMD_SET_UI_IO_PORTS_TOTAL_PARAMS) {
				escribir_socket_format(misocket,"ERROR. Number of bytes received different than %d",ZRCP_CMD_SET_UI_IO_PORTS_TOTAL_PARAMS);
			}
			else {
				//Meterlo en los puertos tal cual
//;                    Bits:  4    3    2    1    0     ;desplazamiento puerto
//puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
//puerto_65022   db    255  ; G    F    D    S    A     ;1
//puerto_64510    db              255  ; T    R    E    W    Q     ;2
//puerto_63486    db              255  ; 5    4    3    2    1     ;3
//puerto_61438    db              255  ; 6    7    8    9    0     ;4
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H                J         K      L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7
				puerto_65278=buffer_destino[0];
				puerto_65022=buffer_destino[1];
				puerto_64510=buffer_destino[2];
				puerto_63486=buffer_destino[3];
				puerto_61438=buffer_destino[4];
				puerto_57342=buffer_destino[5];
				puerto_49150=buffer_destino[6];
				puerto_32766=buffer_destino[7];
				puerto_especial_joystick=buffer_destino[8];

				//0xFEFE,0xFDFE,0xFBFE,0xF7FE,0xEFFE,0xDFFE,0xBFFE,0x7FFE
			}


		}

	}


else if (!strcmp(comando_sin_parametros,"set-machine") || !strcmp(comando_sin_parametros,"sm")) {
	if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
	else {

		//Entramos en el mismo modo que cpu-step para poder congelar la emulacion
		remote_cpu_enter_step(misocket);
		if (menu_event_remote_protocol_enterstep.v==0) return;

		if (set_machine_type_by_name(parametros)) {
						 //Error.
						escribir_socket_format(misocket,"ERROR. Unknown machine %s",parametros);
		}

		else {
			set_machine(NULL);
			cold_start_cpu_registers();
			reset_cpu();
		}

		remote_cpu_exit_step(misocket);
	}

}

else if (!strcmp(comando_sin_parametros,"set-membreakpoint") ) {
  if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. You must enable breakpoints first");
  else {

                                remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

                                if (remote_command_argc<2) {
                                        escribir_socket(misocket,"ERROR. Needs two parameters minimum");
                                        return;
                                }

  	int index_int=parse_string_to_number(remote_command_argv[0]);
  	if (index_int<0 || index_int>65536) {
		escribir_socket(misocket,"ERROR. Address out of range");
		return;
	}

	int type=parse_string_to_number(remote_command_argv[1]);
	if (type<0 || type>255) {
                escribir_socket(misocket,"ERROR. Type out of range");
                return;
        }

	int items=1;

	if (remote_command_argc>=3) {
		items=parse_string_to_number(remote_command_argv[2]);
	}



	for (;items>0;items--) {
		debug_set_mem_breakpoint(index_int++,type);
	}
  }
}

else if (!strcmp(comando_sin_parametros,"set-memory-zone") || !strcmp(comando_sin_parametros,"smz") ) {
	if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
	else {

		int readwrite;

		int zone=parse_string_to_number(parametros);

		if (zone==-1) {
			menu_debug_show_memory_zones=0;
			menu_debug_memory_zone=-1;
		}

		else {

			menu_debug_show_memory_zones=1;

			unsigned int size=machine_get_memory_zone_attrib(zone,&readwrite);
			if (size<=0) {
						 //Error.
						escribir_socket_format(misocket,"ERROR. Unknown zone %d",zone);
					}

			else {
				//escribir_socket_format(misocket,"Setting zone to %d\n",zone);
				menu_debug_memory_zone=zone;
			}
		}

	}

}

	else if (!strcmp(comando_sin_parametros,"set-text-brightness")) {
		if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
	        else {
			int bri=parse_string_to_number(parametros);
			if (bri<0 || bri>100) {
				escribir_socket_format(misocket,"ERROR. Invalid brightness value %d",bri);
			}

			else screen_text_brightness=bri;
		}
	}

	else if (!strcmp(comando_sin_parametros,"set-register") || !strcmp(comando_sin_parametros,"sr")) {
		if (debug_change_register(parametros)) {
			escribir_socket (misocket,"Error changing register");
		}
		else {
			print_registers(buffer_retorno);
			escribir_socket (misocket,buffer_retorno);
		}
	}

	else if (!strcmp(comando_sin_parametros,"set-verbose-level")) {
		if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");

		else {
			int i=parse_string_to_number(parametros);

			if (i<0 || i>4) {
				escribir_socket (misocket,"ERROR. Invalid Verbose level");
			}

			else verbose_level=i;
		}
	}

	else if (!strcmp(comando_sin_parametros,"set-window-zoom") ) {
		if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
		else {

			//Entramos en el mismo modo que cpu-step para poder congelar la emulacion
			remote_cpu_enter_step(misocket);
			if (menu_event_remote_protocol_enterstep.v==0) return;

			int z=parse_string_to_number(parametros);

			screen_set_window_zoom(z);


			remote_cpu_exit_step(misocket);
		}

	}


else if (!strcmp(comando_sin_parametros,"smartload") || !strcmp(comando_sin_parametros,"sl")) {

	remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
	if (remote_command_argc<1) {
		escribir_socket(misocket,"ERROR. No parameter set");
		return;
	}


		//Si estamos en cpu-step-mode, cargar tal cual y volver sin tocar modo
		//Si no, entrar cpu-step-mode y luego quitar cpu-step-mode

		z80_bit antes_menu_event_remote_protocol_enterstep;

		antes_menu_event_remote_protocol_enterstep.v=menu_event_remote_protocol_enterstep.v;


		if (antes_menu_event_remote_protocol_enterstep.v==0) {
    		//Asegurarnos que congelamos el emulador: abrir menu con mutitarea desactivada
    		//Entramos en el mismo modo que cpu-step para poder congelar la emulacion
            remote_cpu_enter_step(misocket);
            if (menu_event_remote_protocol_enterstep.v==0) return;
		}


    //if (quickload(parametros)) {
    //  escribir_socket (misocket,"Error. Unknown file format");
    //}

    strcpy(quickload_file,remote_command_argv[0]);
    quickfile=quickload_file;

    if (quickload(quickload_file)) {
      escribir_socket (misocket,"Error. Unknown file format");
    }

	if (antes_menu_event_remote_protocol_enterstep.v==0) {
        remote_cpu_exit_step(misocket);
	}

  }

    else if (!strcmp(comando_sin_parametros,"snapshot-inram-get-index") ) {
        //Retorna el indice al elemento N, donde 0 es el mas antiguo

        remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

        if (remote_command_argc<1) {
                escribir_socket(misocket,"ERROR. Needs one parameter");
                return;
        }

        int index_int=parse_string_to_number(remote_command_argv[0]);

        int indice_retorno=snapshot_in_ram_get_element(index_int);

        escribir_socket_format(misocket,"%d",indice_retorno);
    }

    else if (!strcmp(comando_sin_parametros,"snapshot-inram-load") ) {
        //Carga el snapshot N de memoria, donde 0 es el mas antiguo

        remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

        if (remote_command_argc<1) {
            escribir_socket(misocket,"ERROR. Needs one parameter");
            return;
        }

        int index_int=parse_string_to_number(remote_command_argv[0]);

        int resultado=snapshot_in_ram_load(index_int);

        if (resultado<0) escribir_socket_format(misocket,"Error loading snapshot from ram position %d",index_int);

    }




  else if (!strcmp(comando_sin_parametros,"snapshot-load") ) {

		//Si estamos en cpu-step-mode, cargar tal cual y volver sin tocar modo
		//Si no, entrar cpu-step-mode y luego quitar cpu-step-mode

		z80_bit antes_menu_event_remote_protocol_enterstep;

		antes_menu_event_remote_protocol_enterstep.v=menu_event_remote_protocol_enterstep.v;


		if (antes_menu_event_remote_protocol_enterstep.v==0) {
    	//Asegurarnos que congelamos el emulador: abrir menu con mutitarea desactivada
    	//Entramos en el mismo modo que cpu-step para poder congelar la emulacion
    	remote_cpu_enter_step(misocket);
    	if (menu_event_remote_protocol_enterstep.v==0) return;
		}


    strcpy(snapshot_load_file,parametros);
    snapfile=snapshot_load_file;
    snapshot_load();


		if (antes_menu_event_remote_protocol_enterstep.v==0) {
    	remote_cpu_exit_step(misocket);
		}
  }

  else if (!strcmp(comando_sin_parametros,"snapshot-save") ) {
    snapshot_save(parametros);
  }

  else if (!strcmp(comando_sin_parametros,"speech-empty-fifo") ) {
  	//Vaciamos cola speech
  	textspeech_empty_speech_fifo();
  }

  else if (!strcmp(comando_sin_parametros,"speech-send") ) {
  	textspeech_print_speech(parametros);
  }

  else if (!strcmp(comando_sin_parametros,"tbblue-get-clipwindow") ) {

    if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
    else {
      remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
      if (remote_command_argc<1) {
        escribir_socket(misocket,"ERROR. Needs one parameter");
        return;
      }


      z80_byte *clipwindow;
      clipwindow=remote_return_clipwindow(remote_command_argv[0]);
      if (clipwindow==NULL) {
        escribir_socket(misocket,"ERROR. Unknown clip window");
        return;
      }

      int i;
      for (i=0;i<4;i++) {
        escribir_socket_format(misocket,"%d ",clipwindow[i]);
      }




    }
  }


  else if (!strcmp(comando_sin_parametros,"tbblue-set-clipwindow") ) {

    if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
    else {
      remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
      if (remote_command_argc<5) {
        escribir_socket(misocket,"ERROR. Needs five parameters");
        return;
      }


      z80_byte *clipwindow;
      clipwindow=remote_return_clipwindow(remote_command_argv[0]);
      if (clipwindow==NULL) {
        escribir_socket(misocket,"ERROR. Unknown clip window");
        return;
      }

      int i;
      for (i=0;i<4;i++) {
        clipwindow[i]=parse_string_to_number(remote_command_argv[1+i]);
      }




    }
  }

    else if (!strcmp(comando_sin_parametros,"tbblue-get-palette") ) {

		if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
		else {
			remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
			if (remote_command_argc<3) {
				escribir_socket(misocket,"ERROR. Needs three parameter minimum");
				return;
			}

			z80_byte index=parse_string_to_number(remote_command_argv[2]);

      z80_int *paleta;
      paleta=remote_return_palette(remote_command_argv[0],remote_command_argv[1]);
      if (paleta==NULL) {
        escribir_socket(misocket,"ERROR. Unknown palette");
        return;
      }

			int items=1;
			if (remote_command_argc>3) items=parse_string_to_number(remote_command_argv[3]);
			//if (index<0 || index>255) escribir_socket(misocket,"ERROR. Out of range");

			for (;items;items--) {
				//z80_byte color=tbsprite_palette[index++];
        z80_int color=paleta[index++];  //primera paleta
				escribir_socket_format(misocket,"%03X ",color);
			}

	        }


    }

        else if (!strcmp(comando_sin_parametros,"tbblue-get-pattern") ) {

			if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
			else {

				remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

				if (remote_command_argc<2) {
						escribir_socket(misocket,"ERROR. Needs two parameters minimum");
						return;
				}

				int index_int=parse_string_to_number(remote_command_argv[0]);

				int totalitems=1;

				int bpp=parse_string_to_number(remote_command_argv[1]);

				if (bpp!=4 && bpp!=8) {
					escribir_socket_format(misocket,"ERROR. Invalid value for bpp: %d",bpp);
					return;
				}

				if (remote_command_argc>2) totalitems=parse_string_to_number(remote_command_argv[2]);

				int max_pattern=TBBLUE_MAX_PATTERNS;
				int total_size=TBBLUE_SPRITE_8BPP_SIZE;

				if (bpp==4) {
					max_pattern *=2;
					total_size=TBBLUE_SPRITE_4BPP_SIZE;
				}


				//Esto solo usado para 4bpp
				//int indice_4bpp=index_int/2;
                int indice_4bpp=index_int;
				//int offset_1_pattern=index_int %2;


				if (index_int<0 || index_int>=max_pattern) escribir_socket(misocket,"ERROR. Out of range");
				else {
					for (;totalitems;totalitems--) {
						int i;
						for (i=0;i<total_size;i++) {

							z80_byte index_color;

							if (bpp==8) {
								index_color=tbsprite_pattern_get_value_index_8bpp(index_int,i);
							}
							else {
								index_color=tbsprite_pattern_get_value_index_4bpp(indice_4bpp,i);
							}

							escribir_socket_format(misocket,"%02X ",index_color);
						}
						escribir_socket(misocket,"\n");
						index_int++;
						if (index_int==TBBLUE_MAX_PATTERNS) index_int=0;
					}
				}
			}

        }


        else if (!strcmp(comando_sin_parametros,"tbblue-get-register") ) {

          if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
          else {
            if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
            else {

               int index=parse_string_to_number(parametros);
               if (index<0 || index>255) escribir_socket(misocket,"ERROR. Out of range");
               else {
                 z80_byte value=tbblue_get_value_port_register(index);
                 escribir_socket_format(misocket,"%02XH",value);
               }
            }
          }

        }


        else if (!strcmp(comando_sin_parametros,"tbblue-get-sprite") ) {

					if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
					else {

							remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

							if (remote_command_argc<1) {
											escribir_socket(misocket,"ERROR. Needs one parameter minimum");
											return;
							}

							int index_int=parse_string_to_number(remote_command_argv[0]);

							int totalitems=1;

							if (remote_command_argc>1) totalitems=parse_string_to_number(remote_command_argv[1]);

							if (index_int<0 || index_int>=TBBLUE_MAX_SPRITES) escribir_socket(misocket,"ERROR. Out of range");
							else {
								for (;totalitems;totalitems--) {
									int i;

									int tamanyo_sprite=4;
									if (tbsprite_new_sprites[index_int][3] & 64) tamanyo_sprite++;

									for (i=0;i<tamanyo_sprite;i++) {
											z80_byte value_sprite=tbsprite_new_sprites[index_int][i];
											escribir_socket_format(misocket,"%02X ",value_sprite);
									}
									escribir_socket(misocket,"\n");
									index_int++;
									if (index_int==TBBLUE_MAX_SPRITES) index_int=0;
								}
							}
					}

				}



	else if (!strcmp(comando_sin_parametros,"tbblue-set-palette")) {
		z80_byte index;
		z80_int valor;
		if (parametros[0]==0) {
			escribir_socket(misocket,"ERROR. No parameters set");
		}

    //ula|layer2|sprite first|second index value
		else {

      //remote_copy_string_spc
      char nombrepaleta[100];
      char tipopaleta[100];
      char textoindice[100];

      char *s;
      s=parametros;

      remote_copy_string_spc(s,nombrepaleta);
      s=find_space_or_end(s);

      remote_copy_string_spc(s,tipopaleta);
      s=find_space_or_end(s);

      remote_copy_string_spc(s,textoindice);
      s=find_space_or_end(s);

      //printf ("%s-%s-%s\n",nombrepaleta,tipopaleta,textoindice);

      z80_int *paleta;
      paleta=remote_return_palette(nombrepaleta,tipopaleta);
      if (paleta==NULL) {
        escribir_socket(misocket,"ERROR. Unknown palette");
        return;
      }


			index=parse_string_to_number(textoindice);

			//Ver si hay espacio
			//s=find_space_or_end(parametros);
			while (*s) {
				valor=parse_string_to_number(s);

        paleta[index++]=valor & 0x1FF;

				s=find_space_or_end(s);
			}


		}

	}


	else if (!strcmp(comando_sin_parametros,"tbblue-set-pattern")) {
		int index_int;
		z80_byte valor;
		if (parametros[0]==0) {
			escribir_socket(misocket,"ERROR. No parameters set");
		}

		else {

			index_int=parse_string_to_number(parametros);

			if (index_int<0 || index_int>=TBBLUE_MAX_PATTERNS) escribir_socket(misocket,"ERROR. Out of range");

			z80_byte i=0;

			//Ver si hay espacio
			char *s=find_space_or_end(parametros);
			while (*s) {
				valor=parse_string_to_number(s);
				//tbsprite_patterns[index_int][i++]=valor;
				tbsprite_pattern_put_value_index_8bpp(index_int,i,valor);
				i++;

				s=find_space_or_end(s);
			}


		}

	}

   else if (!strcmp(comando_sin_parametros,"tbblue-set-register") ) {

          if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
          else {

              remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

              if (remote_command_argc<2) {
                      escribir_socket(misocket,"ERROR. Needs two parameters");
                      return;
              }

              int index_int=parse_string_to_number(remote_command_argv[0]);

              int value=parse_string_to_number(remote_command_argv[1]);


              if (index_int<0 || index_int>255 || value<0 || value>255) escribir_socket(misocket,"ERROR. Out of range");
              else {
                //tbblue_set_register_port(index_int);
                //tbblue_set_value_port(value);
								tbblue_set_value_port_position(index_int,value);
              }
          }

  }


	else if (!strcmp(comando_sin_parametros,"tbblue-set-sprite")) {
		int index_int;
		z80_byte valor;
		if (parametros[0]==0) {
			escribir_socket(misocket,"ERROR. No parameters set");
		}

		else {

			index_int=parse_string_to_number(parametros);

			if (index_int<0 || index_int>=TBBLUE_MAX_SPRITES) escribir_socket(misocket,"ERROR. Out of range");

			z80_byte i=0;

			//Ver si hay espacio
			char *s=find_space_or_end(parametros);
			while (*s) {
				valor=parse_string_to_number(s);
				//tbsprite_sprites[index_int][i]=valor;
                tbblue_write_sprite_value(index_int,i,valor);
                i++;
				if (i==4) i=0;

				s=find_space_or_end(s);
			}


		}

	}






				else if (!strcmp(comando_sin_parametros,"tsconf-get-af-port") ) {

			  	if (!MACHINE_IS_TSCONF) escribir_socket(misocket,"ERROR. Machine is not TSConf");
					else {
						if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
						else {

							 int index=parse_string_to_number(parametros);
							 if (index<0 || index>255) escribir_socket(misocket,"ERROR. Out of range");
							 else {
								 z80_byte value=tsconf_get_af_port(index);
								 escribir_socket_format(misocket,"%02XH",value);
							 }
						}
					}

				}



				else if (!strcmp(comando_sin_parametros,"tsconf-set-af-port") ) {

			  	if (!MACHINE_IS_TSCONF) escribir_socket(misocket,"ERROR. Machine is not TSConf");
					else {

              remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

              if (remote_command_argc<2) {
                      escribir_socket(misocket,"ERROR. Needs two parameters");
                      return;
              }

							int index_int=parse_string_to_number(remote_command_argv[0]);

            	int value=parse_string_to_number(remote_command_argv[1]);

							if (index_int<0 || index_int>255 || value<0 || value>255) escribir_socket(misocket,"ERROR. Out of range");

							else {
								tsconf_write_af_port(index_int,value);
							}


					}

				}





        else if (!strcmp(comando_sin_parametros,"quit") || !strcmp(comando_sin_parametros,"exit") || !strcmp(comando_sin_parametros,"logout")) {
          //if (menu_event_remote_protocol_enterstep.v) remote_cpu_exit_step(misocket);

          escribir_socket (misocket,"Sayonara baby\n");
					*remote_salir_conexion_cliente=1;
					sleep(1);

					remote_cerrar_conexion();

        }

	else if (!strcmp(comando_sin_parametros,"view-basic")) {
                char *results_buffer=util_malloc_max_texto_generic_message("Can not allocate memory for showing basic");

				debug_view_basic(results_buffer);
				escribir_socket(misocket,results_buffer);

                free(results_buffer);
	}

/*
como generar volcado hexadecimal de archivo binario:
hexdump -v -e '16/1 "%02xH " " "'
*/
else if (!strcmp(comando_sin_parametros,"write-memory") || !strcmp(comando_sin_parametros,"wm")) {
	unsigned int direccion;
	z80_byte valor;
	if (parametros[0]==0) {
		escribir_socket(misocket,"ERROR. No parameters set");
	}

	else {

		direccion=parse_string_to_number(parametros);

		menu_debug_set_memory_zone_attr();

		//Ver si hay espacio
		char *s=find_space_or_end(parametros);
		while (*s) {
			valor=parse_string_to_number(s);
			//poke_byte_z80_moto(direccion++,valor);
			//printf ("poke addr %X value %x\n",direccion,valor);
			menu_debug_write_mapped_byte(direccion++,valor);

			s=find_space_or_end(s);
		}


	}

}

else if (!strcmp(comando_sin_parametros,"write-mapped-memory") || !strcmp(comando_sin_parametros,"wmm")) {
	escribir_socket(misocket,"This command is no longer supported. Use write-memory command and set memory zone if needed");
}

else if (!strcmp(comando_sin_parametros,"write-mapped-memory-raw") ) {
	escribir_socket(misocket,"This command is no longer supported. Use write-memory-raw command and set memory zone if needed");
}

else if (!strcmp(comando_sin_parametros,"write-memory-raw") ) {
	unsigned int direccion;
	z80_byte valor;
	if (parametros[0]==0) {
		escribir_socket(misocket,"ERROR. No parameters set");
	}

	else {

		direccion=parse_string_to_number(parametros);

		menu_debug_set_memory_zone_attr();

		//Ver si hay espacio
		char *s=find_space_or_end(parametros);
		while (*s) {
			char buffer_valor[4];
			buffer_valor[0]=s[0];
			buffer_valor[1]=s[1];
			buffer_valor[2]='H';
			buffer_valor[3]=0;
			//printf ("%s\n",buffer_valor);
			valor=parse_string_to_number(buffer_valor);
			//printf ("valor: %d\n",valor);
			//poke_byte_z80_moto(direccion++,valor);
			menu_debug_write_mapped_byte(direccion++,valor);

			s++;
			if (*s) s++;
		}
		/*else {
			escribir_socket(misocket,"ERROR. No value set");
		}*/

	}

}

else if (!strcmp(comando_sin_parametros,"write-port") ) {

	if (!CPU_IS_Z80) escribir_socket(misocket,"ERROR. CPU is not Z80");
	else {

              remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);

              if (remote_command_argc<2) {
                      escribir_socket(misocket,"ERROR. Needs two parameters");
                      return;
              }

			z80_int port=parse_string_to_number(remote_command_argv[0]);

            z80_byte value=parse_string_to_number(remote_command_argv[1]);

			out_port(port,value);



	}
}


	else if (!strcmp(comando_sin_parametros,"zeng-is-master") ) {
		escribir_socket_format(misocket,"%d",zeng_i_am_master);
	}

	else if (!strcmp(comando_sin_parametros,"zeng-online") || !strcmp(comando_sin_parametros,"zo")) {
        remote_parse_commands_argvc(parametros,&remote_command_argc,remote_command_argv);
        zeng_online_parse_command(misocket,remote_command_argc,remote_command_argv,ip_source_address);

	}


		else if (!strcmp(comando_sin_parametros,"zxevo-get-nvram") ) {

			  	if (!MACHINE_IS_ZXEVO) escribir_socket(misocket,"ERROR. Machine is not ZX-Evo");
					else {
						if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
						else {

							 int index=parse_string_to_number(parametros);
							 if (index<0 || index>255) escribir_socket(misocket,"ERROR. Out of range");
							 else {
								 z80_byte value=zxevo_nvram[index];
								 escribir_socket_format(misocket,"%02X",value);
							 }
						}
					}

				}

	else {
		escribir_socket (misocket,"Unknown command");
	}



}

//Retorna 0 si ok
//diferente de 0 si error
int remote_initialize_port(void)
{

	sock_listen=crear_socket_TCP();

	//printf("sock_listen: %d\n",sock_listen);

//#ifndef MINGW
  if (sock_listen<0) {
    debug_printf (VERBOSE_ERR,"Error creating socket TCP for remote command protocol");
    return 1;
  }

	//Evitar errores "Unable to bind: Address already in use" cuando se cierra el emulador con una conexion en curso
	int opcion_socket=1;

	setsockopt(sock_listen,SOL_SOCKET,SO_REUSEADDR,&opcion_socket,sizeof(int));

//#endif


	int result=assignar_adr_internet(sock_listen,NULL,remote_protocol_port);

//#ifndef MINGW
  if (result<0) {
    debug_printf(VERBOSE_ERR,"Error allocating socket address for remote command protocol");
    return 1;
  }
//#endif

	result=listen(sock_listen,SOMAXCONN);

//#ifndef MINGW
  if (result<0) {
    debug_printf (VERBOSE_ERR,"Error running listen for remote command protocol");
    return 1;
  }
//#endif


  return 0;
}

void remote_get_client_ip(int elsocket,char *ipsource)
{


    //Inicialmente vacia
    ipsource[0]=0;

//Estructura del socket client, obtinguda per saber el nom del client
struct sockaddr_in sockaddr_client;

 socklen_t long_sockaddr;
	char *direccio_client; //Adreça IP del client en format text


	//Obtenir socket client
	long_sockaddr=sizeof(sockaddr_client);
	if (getpeername(elsocket,(struct sockaddr *)&sockaddr_client,&long_sockaddr)<0) {
		debug_printf (VERBOSE_DEBUG,"ZRCP: Error getting client IP");
		return;
	}



	//debug_printf (VERBOSE_DEBUG,"ncdd_servidor_fill: %x",sockaddr_client.sin_addr.s_addr);


	//Obtenir direccio IP client, en format text ("A.B.C.D")
	direccio_client=inet_ntoa(sockaddr_client.sin_addr);
	if (direccio_client==NULL) {
		debug_printf (VERBOSE_DEBUG,"ZRCP: Error getting client IP string");
		return;
	}



    strcpy(ipsource,direccio_client);






}

z_atomic_semaphore zrcp_command_semaforo;

void zrcp_sem_init(void)
{
	z_atomic_reset(&zrcp_command_semaforo);
}

/*
struct s_zrcp_new_connection_parms
{
    int sock_connected_client;
};
*/

void *zrcp_handle_new_connection(void *entrada)
{

	//Ese puntero que nos llega es realmente el valor del numero de socket
    int sock_connected_client=(int)entrada;

	//printf("sock_connected_client en zrcp_handle_new_connection: %d\n",sock_connected_client);
    //char *buffer_lectura_socket=((struct s_zrcp_new_connection_parms *)entrada)->buffer_lectura_socket;

    //Asignar memoria para los buffers de recepcion
    char *buffer_lectura_socket=malloc(MAX_LENGTH_PROTOCOL_COMMAND);

    char *buffer_lectura_socket_anterior=malloc(MAX_LENGTH_PROTOCOL_COMMAND);

    if (buffer_lectura_socket==NULL || buffer_lectura_socket_anterior==NULL) cpu_panic("Can not allocate buffer for ZRCP");


    //Inicializarlo a cadena vacia
    buffer_lectura_socket_anterior[0]=0;



    debug_printf (VERBOSE_DEBUG,"Received remote command connection petition");

    //obtener direccion ip origen
    char ip_source_address[MAX_IP_SOURCE_ADDRESS_LENGTH+1];
    remote_get_client_ip(sock_connected_client,ip_source_address);

    debug_printf (VERBOSE_DEBUG,"ZRCP: Client IP: %s",ip_source_address);
    //printf("ZRCP: Client IP: %s\n",ip_source_address);

    //Enviamos mensaje bienvenida
    escribir_socket(sock_connected_client,"Welcome to ZEsarUX remote command protocol (ZRCP)\nWrite help for available commands\n");


    int remote_salir_conexion_cliente=0;

    while (!remote_salir_conexion_cliente) {

        char prompt[1024];
        if (menu_event_remote_protocol_enterstep.v) sprintf (prompt,"\n%s@cpu-step> ",remote_prompt_command_string);
        else if (remote_protocol_assembling.v) sprintf (prompt,"assemble at %XH> ",direccion_assembling);
        else sprintf (prompt,"\n%s> ",remote_prompt_command_string);
        if (escribir_socket(sock_connected_client,prompt)<0) remote_salir_conexion_cliente=1;

        int indice_destino=0;

        if (!remote_salir_conexion_cliente) {

            //Leer socket
            int leidos;
            int salir_bucle=0;
            do {
                leidos=leer_socket(sock_connected_client, &buffer_lectura_socket[indice_destino], MAX_LENGTH_PROTOCOL_COMMAND-1);
                debug_printf (VERBOSE_DEBUG,"ZRCP: Read block %d bytes index: %d",leidos,indice_destino);

                /*
                RETURN VALUES
                These calls return the number of bytes received, or -1 if an error occurred.

                For TCP sockets, the return value 0 means the peer has closed its half side of the connection.
                */

                if (leidos==0) remote_salir_conexion_cliente=1;

                //Controlar tambien si da <0 bytes al leer. En ese caso salir tambien
                //Nota: Esto lo he observado en Windows a veces (en un caso retornó -8)
                if (leidos<0) remote_salir_conexion_cliente=1;

                if (leidos>0) {

                    indice_destino +=leidos;
                    //Si acaba con final de string, salir

                    //printf ("%d %d %d %d\n",buffer_lectura_socket[0],buffer_lectura_socket[1],buffer_lectura_socket[2],buffer_lectura_socket[3]);

                    if (buffer_lectura_socket[indice_destino-1]=='\r' || buffer_lectura_socket[indice_destino-1]=='\n' || buffer_lectura_socket[indice_destino-1]==0) {
                        salir_bucle=1;
                    }
                }


            } while (leidos>0 && salir_bucle==0);


            //No hacer nada de esto si es <=0
            if (leidos>0) {
                buffer_lectura_socket[indice_destino]=0;
                debug_printf (VERBOSE_DEBUG,"Remote command. Length Read text: %d",indice_destino);

                //Para que no pete el emulador si el verbose level esta al menos 3 y el comando recibido excede el maximo que puede mostrar debug_printf
                if (strlen(buffer_lectura_socket)<DEBUG_MAX_MESSAGE_LENGTH) {
                    debug_printf (VERBOSE_DEBUG,"Remote command. Read text: [%s]",buffer_lectura_socket);
                }

                //Adquirir lock
                //Pero solo si no esta activado zeng-online, en ese caso se permiten multiples conexiones
                if (!zeng_online_enabled) {
                    while(z_atomic_test_and_set(&zrcp_command_semaforo)) {
                        //Pausa de 0.05 segundo
                        usleep(50000);
                        //printf("Esperando a liberar lock en zrcp_handle_new_connection\n");
                    }
                }

                interpreta_comando(buffer_lectura_socket,sock_connected_client,
					buffer_lectura_socket_anterior,&remote_salir_conexion_cliente,ip_source_address);

                //Liberar lock
                z_atomic_reset(&zrcp_command_semaforo);
            }

        } //Fin if (!remote_salir_conexion_cliente)

    } //Fin while (!remote_salir_conexion_cliente)

    //Liberar buffers de recepcion
    free(buffer_lectura_socket);
    free(buffer_lectura_socket_anterior);

    //printf("Fin thread de cliente\n");


#ifdef MINGW
	closesocket(sock_connected_client);
	//desactivo esto ya que esto implica que no se va a usar mas los windows sockets, cosa no cierta (se pueden usar en zeng por ejemplo)
	//ademas no estamos llamando a WSAStartup al inicio
	//Se deberia hacer el WSACleanup al finalizar el emulador
	//WSACleanup();
#else
	//printf("Closing socket %d\n",sock_connected_client);
    //int retorno=
    close(sock_connected_client);
    //printf("Retorno close: %d\n",retorno);
#endif

  return NULL;
}



void thread_remote_protocol_function_aux_new_conn(int sock_connected_client)
{
    //struct s_zrcp_new_connection_parms parametros_thread;
    //parametros_thread.sock_connected_client=sock_connected_client;

	//printf("sock_connected_client en thread_remote_protocol_function_aux_new_conn: %d\n",sock_connected_client);
    //parametros_thread.buffer_lectura_socket=buffer_lectura_socket;

    //TODO: deberia llevar un control de cada thread que se crea?
    //porque pierdo el control despues de crearlo...,
    //aparte de que hago malloc pero no libero en ningun sitio (aunque este malloc es de pthread_t que es un entero, unos pocos bytes),
    //pero es un memory leak

    //Nota: no uso pthread_t temp_thread (sin hacer malloc) porque eso asignaria memoria temporal solo para esta funcion, en el stack,
    //pero luego se reusaria, y no tengo claro si ese pthread_t tiene que estar en un sitio "seguro"
    //Por tanto hago malloc y se que nadie liberara la memoria (a no ser que haga yo un free)
    pthread_t *temp_thread=util_malloc(sizeof(pthread_t),"Can not allocate memory for new thread for ZRCP connection");

    //pthread_t temp_thread;

	//Nota: al pasar el parametro de sock_connected_client, o bien creo una estructura con un malloc, para cada conexion,
	//que ademas deberia llevar yo un control para hacer su free correspondiente, o bien
	//hago esto, en el que el valor del socket lo envio como si fuese un puntero,
	//y eso funciona, siempre considerando las limitaciones de un valor de un puntero, por ejemplo en entornos de 32 bits,
	//un puntero tiene tamaño de 32 bits,
	//y no podria enviar un valor de socket que excediese el rango de 32 bits (si intentase enviar un long de 64 bits por ejemplo)
	//Nota 2: podrias pensar que serviria asignar una estructura en el stack , pero eso no vale, porque
	//al finalizar esta funcion, el stack se libera , y cuando el thread vaya a mirar esa estructura,
	//la memoria donde estaba, esta liberada y a saber entonces que lee...
    if (pthread_create( temp_thread, NULL, &zrcp_handle_new_connection, (void *)sock_connected_client) ) {
        debug_printf(VERBOSE_ERR,"Error running handling new ZRCP connection");
    }

    //y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(*temp_thread);
}

void *thread_remote_protocol_function(void *nada)
{

	if (remote_initialize_port()) return NULL;

	fflush(stdout);

	while (1)
	{
		long_adr=sizeof(adr);

        int sock_connected_client=-1;

		if (!remote_protocol_ended.v) {
			//printf ("ANTES accept\n");
			sock_connected_client=accept(sock_listen,(struct sockaddr *)&adr,&long_adr);
            //printf("sock_connected_client: %d\n",sock_connected_client);
		}

		else {
			//Si ha finalizado, volver
			return NULL;
		}


		if (sock_connected_client<0) {
			debug_printf (VERBOSE_DEBUG,"Remote command. Error running accept on socket %d",
				sock_listen);
			//printf ("Remote command. Error running accept on socket %d. More info: %s\n",
			//	sock_listen,strerror(errno));
			//Esto se dispara cuando desactivamos el protocolo desde el menu, y no queremos que salte error
			//Aunque tambien se puede dar en otros momentos por culpa de fallos de conexion, no queremos que moleste al usuario
			//como mucho lo mostramos en verbose debug

			//remote_salir_conexion_cliente=1;
			sleep(1);
		}


		else {

            thread_remote_protocol_function_aux_new_conn(sock_connected_client);

		}

		//debug_printf (VERBOSE_DEBUG,"Remote command. Exiting connection");

        //printf ("Remote command. Exiting connection\n");

		//Salir del modo step si estaba activado
		//if (menu_event_remote_protocol_enterstep.v) remote_cpu_exit_step_continue();

	} //Fin while(1)


	//para que no se queje el compilador de variable no usada
	nada=0;
	nada++;


	return NULL;
}



void init_remote_protocol(void)
{

  if (remote_protocol_enabled.v==0) return;

	debug_printf (VERBOSE_INFO,"Starting ZEsarUX remote protocol (ZRCP) listener on port %d",remote_protocol_port);


    /*
    //Asignar memoria para los buffers de recepcion
    buffer_lectura_socket=malloc(MAX_LENGTH_PROTOCOL_COMMAND);

    buffer_lectura_socket_anterior=malloc(MAX_LENGTH_PROTOCOL_COMMAND);

    if (buffer_lectura_socket==NULL || buffer_lectura_socket_anterior==NULL) cpu_panic("Can not allocate buffer for ZRCP");


    //Inicializarlo a cadena vacia
    buffer_lectura_socket_anterior[0]=0;*/




	thread_remote_inicializado.v=0;

	if (pthread_create( &thread_remote_protocol, NULL, &thread_remote_protocol_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create remote protocol pthread");
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_remote_protocol);

	thread_remote_inicializado.v=1;
	remote_protocol_ended.v=0;

    zrcp_sem_init();

}

void enable_and_init_remote_protocol(void)
{
	remote_protocol_enabled.v=1;
	init_remote_protocol();
}

void end_remote_protocol(void)
{

  if (remote_protocol_enabled.v==0) return;
  if (thread_remote_inicializado.v==0) return;

	debug_printf(VERBOSE_INFO,"Ending remote command protocol listener");

	//remote_salir_conexion_cliente=1;
	remote_protocol_ended.v=1;

	remote_cerrar_conexion();
#ifdef MINGW
	closesocket(sock_listen);
	//desactivo esto ya que esto implica que no se va a usar mas los windows sockets, cosa no cierta (se pueden usar en zeng por ejemplo)
	//ademas no estamos llamando a WSAStartup al inicio
	//Se deberia hacer el WSACleanup al finalizar el emulador
	//WSACleanup();
#else
	close(sock_listen);
#endif



  pthread_cancel(thread_remote_protocol);



}

#else

//No hacer nada, no hay pthreads disponibles
void init_remote_protocol(void)
{
	if (remote_protocol_enabled.v==0) return;
	debug_printf(VERBOSE_INFO,"Not enabling remote protocol because pthreads support is not compiled");
}

void end_remote_protocol(void)
{

}


void enable_and_init_remote_protocol(void)
{

}

#endif
