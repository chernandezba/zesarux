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

#ifndef QL_H
#define QL_H

//Para no hacer include de cpu.h, que provoca errores de compilacion por registros iguales en core de z80 y motorola (como reg_b)
//estos dos solo necesarios temporalmente para funciones legacy, como peek_byte_no_time_legacy_ql por ejemplo
typedef unsigned char z80_byte;
typedef unsigned short z80_int;

//Tipos propios para motorola, para intentar evitar z80_
typedef unsigned char moto_byte; // 8 bits
typedef unsigned short moto_int; // 16 bits
typedef unsigned int moto_long;



struct s_moto_bit {
        unsigned int v:1;
};


typedef struct s_moto_bit moto_bit;


#define QL_MEM_LIMIT ((1024*256)-1)
//#define QL_MEM_LIMIT ((1024*1024)-1)

extern  unsigned char *memoria_ql;



extern unsigned char (*ql_readbyte_no_ports_function)(unsigned int Address);
extern unsigned char ql_readbyte_no_ports_vacio(unsigned int Address);


//En principio no hay border. ponemos solo de ejemplo
#define QL_LEFT_BORDER_NO_ZOOM 8
#define QL_TOP_BORDER_NO_ZOOM 8

#define QL_LEFT_BORDER QL_LEFT_BORDER_NO_ZOOM*zoom_x
#define QL_TOP_BORDER QL_TOP_BORDER_NO_ZOOM*zoom_y

//#define QL_LEFT_BORDER 0
//#define QL_TOP_BORDER 0
//Resolucion maxima es 512*256. Para que sea pantalla cuadrada, haremos 512x512 poniendo pixeles doble de alto
#define QL_DISPLAY_WIDTH 512
#define QL_DISPLAY_HEIGHT 512


#define m68k_read_memory_8 GetMemB
#define m68k_read_memory_16 GetMemW
#define m68k_read_memory_32 GetMemL

#define m68k_write_memory_8 SetMemB
#define m68k_write_memory_16 SetMemW
#define m68k_write_memory_32 SetMemL



extern unsigned char ql_readbyte_no_ports(unsigned int Address);
extern void ql_writebyte_no_ports(unsigned int Address,unsigned char valor);
extern void ql_writebyte(unsigned int Address, unsigned char Data);
extern unsigned char ql_readbyte(unsigned int Address);


//Funciones legacy solo para interceptar posibles llamadas a poke, peek etc en caso de motorola
//la mayoria de estas vienen del menu, lo ideal es que en el menu se usen peek_byte_z80_moto , etc
extern void poke_byte_legacy_ql(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_legacy_ql(z80_int dir,z80_byte valor);
extern z80_byte peek_byte_legacy_ql(z80_int dir);
extern z80_byte peek_byte_no_time_legacy_ql(z80_int dir);
extern z80_byte lee_puerto_legacy_ql(z80_byte h,z80_byte l);
extern void out_port_legacy_ql(z80_int puerto,z80_byte value);
extern z80_byte fetch_opcode_legacy_ql(void);


extern void motorola_get_flags_string(char *texto);




#endif
