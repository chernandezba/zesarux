/*
    disassemble.c: Fuse's disassembler
    Copyright (c) 2002-2003 Darren Salt, Philip Kendall

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
#include <string.h>

#include "cpu.h"
#include "operaciones.h"
#include "disassemble.h"
#include "debug.h"
#include "m68k.h"
#include "utils.h"
#include "scmp.h"
#include "zxvision.h"
#include "tbblue.h"
#include "utils_text_adventure.h"

//Indica 1 al desensamblador que el peek_byte debe usar direcciones de memoria de spectrum. modo normal
//Indica 0 al desensamblador que debe usar direcciones del array de desensamblaje, usado en cpu statistics
z80_bit disassemble_peek_si_spectrum_ram;

//Indica 1 al desensamblador que los valores se deben mostrar (caso normal)
//Indica 0  al desensamblador que los valores no se deben mostrar, usado en cpu statistics
z80_bit disassemble_show_value;

//Indica cuantas veces se ha llamado a disassemble_main dentro de ella misma,
//usado para evitar errores de pila al leer sentencias seguidas de 221 o 253
int disassemble_ddfd_anidado;

z80_byte disassemble_array[DISASSEMBLE_ARRAY_LENGTH];

z80_byte disassemble_peek_byte(int address)
{
	if (disassemble_peek_si_spectrum_ram.v==1) {
		address=adjust_address_memory_size(address);
		return menu_debug_get_mapped_byte(address);
		//return peek_byte_no_time(address);
	}
	else return disassemble_array[address];
}

int debugger_output_base=16;

/* Used to flag whether we're after a DD or FD prefix */
enum hl_type { USE_HL, USE_IX, USE_IY };

static void disassemble_main( int address, char *buffer,
			      size_t buflen, size_t *length,
			      enum hl_type use_hl );
static void disassemble_00xxxxxx( int address, char *buffer,
				  size_t buflen, size_t *length,
				  enum hl_type use_hl );
static void disassemble_00xxx010( int address, char *buffer,
				  size_t buflen, size_t *length,
				  enum hl_type use_hl );
static void disassemble_00xxx110( int address, char *buffer,
				  size_t buflen, size_t *length,
				  enum hl_type use_hl );
static void disassemble_11xxxxxx( int address, char *buffer,
				  size_t buflen, size_t *length,
				  enum hl_type use_hl );
static void disassemble_11xxx001( z80_byte b, char *buffer,
				  size_t buflen, size_t *length,
				  enum hl_type use_hl );
static void disassemble_11xxx011( int address, char *buffer,
				  size_t buflen, size_t *length,
				  enum hl_type use_hl );
static void disassemble_11xxx101( int address, char *buffer,
				  size_t buflen, size_t *length,
				  enum hl_type use_hl );
static void disassemble_cb( int address, char *buffer,
			    size_t buflen, size_t *length );
static void disassemble_ed( int address, char *buffer,
			    size_t buflen, size_t *length );
static void disassemble_ddfd_cb( int address, char offset,
				 enum hl_type use_hl, char *buffer,
				 size_t buflen, size_t *length );

static void get_byte( char *buffer, size_t buflen, z80_byte b );
static void get_word( char *buffer, size_t buflen, int address );
static void get_offset( char *buffer, size_t buflen, z80_int address,
			z80_byte offset );

static const char *reg_pair( z80_byte b, enum hl_type use_hl );
static const char *hl_ix_iy( enum hl_type use_hl );
static void ix_iy_offset( char *buffer, size_t buflen, enum hl_type use_hl,
			  z80_byte offset );

static int source_reg( int address, enum hl_type use_hl,
		       char *buffer, size_t buflen );
static int dest_reg( int address, enum hl_type use_hl,
		     char *buffer, size_t buflen );
static int single_reg( int i, enum hl_type use_hl, z80_byte offset,
		       char *buffer, size_t buflen );

static const char *addition_op( z80_byte b );
static const char *condition( z80_byte b );
static const char *rotate_op( z80_byte b );
static const char *bit_op( z80_byte b );
static int bit_op_bit( z80_byte b );

struct s_tbblue_extended_string_opcode {
	char text[32];
	z80_byte opcode;
  int sumar_longitud;
};

#define TOTAL_TBBLUE_EXTENDED_OPCODES 31

struct s_tbblue_extended_string_opcode tbblue_extended_string_opcode[TOTAL_TBBLUE_EXTENDED_OPCODES]={
	{"SWAPNIB",0x23,0},
  {"MIRROR A",0x24,0},
  {"LD HL,SP",0x25,0},
  {"TEST N",0x27,1},
	{"MUL",0x30,0},
  {"ADD HL,A",0x31,0},
  {"ADD DE,A",0x32,0},
  {"ADD BC,A",0x33,0},
  {"ADD HL,NN",0x34,2},
  {"ADD DE,NN",0x35,2},
  {"ADD BC,NN",0x36,2},
  {"INC DEHL",0x37,0},
  {"DEC DEHL",0x38,0},
  {"ADD DEHL,A",0x39,0},
  {"ADD DEHL,BC",0x3A,0}, 
  {"ADD DEHL,NN",0x3B,2},
  {"SUB DEHL,A",0x3C,0}, 
  {"SUB DEHL,BC",0x3D,0}, 
  {"PUSH NN",0x8A,2},
  {"OUTINB",0x90,0},
  {"NEXTREG N,N",0x91,2},
  {"NEXTREG N,A",0x92,1},
  {"PIXELDN",0x93,0},
  {"PIXELAD",0x94,0},
  {"SETAE",0x95,0},
  {"LDIX",0xA4,0},
  {"LDWS",0xA5,0},
  {"LDIRX",0xB4,0},
  {"LDDX",0xAC,0},
  {"LDDRX",0xBC,0},
  {"LDPIRX",0xB7,0}
};

void debugger_handle_extended_tbblue_opcodes(char *buffer, unsigned int address, int *sumar_longitud)
{
	if (MACHINE_IS_TBBLUE) {
		if (!strcmp(buffer,"NOPD")) {
			if (disassemble_peek_byte(address)==237) {
				z80_byte opcode=disassemble_peek_byte(address+1);

				int i;
				for (i=0;i<TOTAL_TBBLUE_EXTENDED_OPCODES;i++) {
					if (tbblue_extended_string_opcode[i].opcode==opcode) {
						strcpy(buffer,tbblue_extended_string_opcode[i].text);
            *sumar_longitud=tbblue_extended_string_opcode[i].sumar_longitud;
					}

				}
			}
		}
	}
}


void debugger_disassemble_crear_rep_spaces(char *origen)
{
	//Quita espacios repetidos de la cadena de texto
	char *destino;

	destino=origen;

	int repetido=0;
	char caracter;
	while (*origen) {
		caracter=*origen;

		if (caracter!=' ') {
			repetido=0;
			*destino=caracter;
			destino++;
		}

		else {
			//No Habia otro?
			if (repetido==0) {	
				repetido=1;
				*destino=caracter;
                 	       destino++;
	                }	

			else {
				//Habia otro
				//No hacemos nada
			}

		}

		origen++;

	}

	*destino=0;
}
		


/* A very thin wrapper to avoid exposing the USE_HL constant */
void
debugger_disassemble( char *buffer, size_t buflen, size_t *length,
		      unsigned int address )
{
	disassemble_peek_si_spectrum_ram.v=1;
	disassemble_show_value.v=1;
	disassemble_ddfd_anidado=0;

	//Caso para MK14
	if (CPU_IS_SCMP) {
		unsigned char op=disassemble_peek_byte(address);
		unsigned char arg=disassemble_peek_byte(address+1);
		int longitud=scmp_CPU_DISASSEMBLE( address , op, arg, buffer);
		*length=longitud;
		return;
	}

	//Caso para QL
	if (CPU_IS_MOTOROLA) {
		char buffer_temporal[256];
		//address=adjust_address_space_cpu(address);
		int longitud=m68k_disassemble(buffer_temporal, address, M68K_CPU_TYPE_68000);
		buffer_temporal[buflen-1]=0; //forzar maximo longitud

		//Quitamos espacios de mas
		debugger_disassemble_crear_rep_spaces(buffer_temporal);

		strcpy(buffer,buffer_temporal);
		*length=longitud;
		return;
	}

	//Caso para copper
	if (menu_debug_memory_zone==MEMORY_ZONE_NUM_TBBLUE_COPPER) {
		//Casos WAIT, MOVE, NOOP, HALT


    *length=2;

		z80_byte op=disassemble_peek_byte(address);
		z80_byte arg=disassemble_peek_byte(address+1);

    //Special case of "value 0 to port 0" works as "no operation" (duration 1 CLOCK)
    if (op==0 && arg==0) {
      strcpy(buffer,"NOOP");
      return;
    }

    //Special case of "WAIT 63,511" works as "halt" instruction
    if (op==255 && arg==255) {
      strcpy(buffer,"HALT");
      return;
    }

		if (op&128) {
			//wait
			int raster=arg|((op&1)<<8);
			int horiz=((op>>1)&63);

			sprintf (buffer,"WAIT %d,%d",raster,horiz);
		}
		else {
			//move
			int registro=op&127;
			int value=arg;
			sprintf (buffer,"MOVE %d,%d",registro,value);
		}

		
		return;
	}

  //Caso para contacts de daad
  if (menu_debug_memory_zone==MEMORY_ZONE_NUM_DAAD_CONDACTS) {

		z80_byte op=disassemble_peek_byte(address);
		z80_byte arg1=disassemble_peek_byte(address+1);
    z80_byte arg2=disassemble_peek_byte(address+2);

    //Palabra del vocabulario
    char buffer_vocabulary[10];
    //por defecto
    buffer_vocabulary[0]=0;

    /*
    Por otro lado, el valor del opcode le tienes que hacer AND 0x7F, porque solo los 7 bits bajos son el opcode, el bit alto indica si el 
primer parámetro tiene indirección, cosa que en lo que a ti afecta, solo te supone poner el parametro 1 entre corechetes o no.
  */

    int indireccion=0;

    if (op>127) {
      op -=128;
      indireccion=1;
    }

    int num_parametros=daad_contacts_array[op].parametros;
    char *nombre_condact=daad_contacts_array[op].nombre;

  z80_byte arg_vocabulary=arg1;
  if (indireccion) arg_vocabulary=util_daad_get_flag_value(arg_vocabulary);

//Si parametros son vocabularios
	//{1,"NOUN2  "}, //  69 $45
	if (op==69) {
		util_daad_locate_word(arg_vocabulary,2,buffer_vocabulary);
	} 		

  //{1,"ADJECT1"}, //  16 $10
  //{1,"ADJECT2"}, //  70 $46
  	if (op==16 || op==70) {
		util_daad_locate_word(arg_vocabulary,3,buffer_vocabulary);
	} 	


  	//{1,"ADVERB "}, //  17 $11
    if (op==17) {
		util_daad_locate_word(arg_vocabulary,1,buffer_vocabulary);
	} 

    //{1,"PREP   "}, //  68 $44	
	if (op==68) {
		util_daad_locate_word(arg_vocabulary,4,buffer_vocabulary);
	} 	

  int vocabulario_encontrado=0;

  if (buffer_vocabulary[0]!=0 && buffer_vocabulary[0]!='?') vocabulario_encontrado=1;

  char buffer_parametro1[32];
  if (indireccion) sprintf (buffer_parametro1,"@%d",arg1);
  else {
    //Skip utiliza parametro en complemento a 2
    if (op==116) sprintf (buffer_parametro1,"%d",(char) arg1);  
    else sprintf (buffer_parametro1,"%d",arg1);  
  }



    if (num_parametros==0) {
      sprintf (buffer,"%s",nombre_condact);
    }

    else if (num_parametros==1) {
      if (vocabulario_encontrado) sprintf (buffer,"%s %s (%s)",nombre_condact,buffer_vocabulary,buffer_parametro1);
      else sprintf (buffer,"%s %s",nombre_condact,buffer_parametro1);
    }    

    else {
      sprintf (buffer,"%s %s %3d",nombre_condact,buffer_parametro1,arg2);
    }   

    *length=1+num_parametros; 

    return;
  }


  //Caso para contacts de daad
  if (menu_debug_memory_zone==MEMORY_ZONE_NUM_PAWS_CONDACTS) {

		z80_byte op=disassemble_peek_byte(address);
		z80_byte arg1=disassemble_peek_byte(address+1);
    z80_byte arg2=disassemble_peek_byte(address+2);

    //Palabra del vocabulario
    char buffer_vocabulary[10];
    //por defecto
    buffer_vocabulary[0]=0;

    /*
    Por otro lado, el valor del opcode le tienes que hacer AND 0x7F, porque solo los 7 bits bajos son el opcode, el bit alto indica si el 
primer parámetro tiene indirección, cosa que en lo que a ti afecta, solo te supone poner el parametro 1 entre corechetes o no.
  */

    int indireccion=0;

    if (op>127) {
      op -=128;
      indireccion=1;
    }

    int num_parametros=paws_contacts_array[op].parametros;
    char *nombre_condact=paws_contacts_array[op].nombre;

  z80_byte arg_vocabulary=arg1;
  if (indireccion) arg_vocabulary=util_daad_get_flag_value(arg_vocabulary);

//Si parametros son vocabularios
	//{1,"NOUN2  "}, //  69 $45
	if (op==69) {
		util_paws_locate_word(arg_vocabulary,2,buffer_vocabulary);
	} 		

  //{1,"ADJECT1"}, //  16 $10
  //{1,"ADJECT2"}, //  70 $46
  	if (op==16 || op==70) {
		util_paws_locate_word(arg_vocabulary,3,buffer_vocabulary);
	} 	


  	//{1,"ADVERB "}, //  17 $11
    if (op==17) {
		util_paws_locate_word(arg_vocabulary,1,buffer_vocabulary);
	} 

    //{1,"PREP   "}, //  68 $44	
	if (op==68) {
		util_paws_locate_word(arg_vocabulary,4,buffer_vocabulary);
	} 	

  int vocabulario_encontrado=0;

  if (buffer_vocabulary[0]!=0 && buffer_vocabulary[0]!='?') vocabulario_encontrado=1;

  char buffer_parametro1[32];
  if (indireccion) sprintf (buffer_parametro1,"@%d",arg1);
  else {
    //Skip utiliza parametro en complemento a 2
    if (op==116) sprintf (buffer_parametro1,"%d",(char) arg1);  
    else sprintf (buffer_parametro1,"%d",arg1);  
  }



    if (num_parametros==0) {
      sprintf (buffer,"%s",nombre_condact);
    }

    else if (num_parametros==1) {
      if (vocabulario_encontrado) sprintf (buffer,"%s %s (%s)",nombre_condact,buffer_vocabulary,buffer_parametro1);
      else sprintf (buffer,"%s %s",nombre_condact,buffer_parametro1);
    }    

    else {
      sprintf (buffer,"%s %s %3d",nombre_condact,buffer_parametro1,arg2);
    }   

    *length=1+num_parametros; 

    return;
  }



	disassemble_main( address, buffer, buflen, length, USE_HL );

	//gestionar casos de opcodes extendidos de Next
  int sumar_longitud=0;
	debugger_handle_extended_tbblue_opcodes(buffer,address,&sumar_longitud);
  *length +=sumar_longitud;
}



void debugger_disassemble_array (char *buffer, size_t buflen, size_t *length, unsigned address )
{
	disassemble_peek_si_spectrum_ram.v=0;
	disassemble_show_value.v=0;
	disassemble_ddfd_anidado=0;
	disassemble_main( address, buffer, buflen, length, USE_HL );

	//gestionar casos de opcodes extendidos de Next
  int sumar_longitud=0;
	debugger_handle_extended_tbblue_opcodes(buffer,address,&sumar_longitud);
  *length +=sumar_longitud;

}


/* Disassemble one instruction */
static void
disassemble_main( int address, char *buffer, size_t buflen,
		  size_t *length, enum hl_type use_hl )
{

  z80_byte b;
  char buffer2[40], buffer3[40];

  b = disassemble_peek_byte( address );

  if( b < 0x40 ) {
    disassemble_00xxxxxx( address, buffer, buflen, length, use_hl );
  } else if( b == 0x76 ) {
    snprintf( buffer, buflen, "HALT" ); *length = 1;
  } else if( b < 0x80 ) {

    if( ( b & 0x07 ) == 0x06 ) {		 /* LD something,(HL) */
      dest_reg( address, USE_HL, buffer2, 40 );
      source_reg( address, use_hl, buffer3, 40 );
      *length = ( use_hl == USE_HL ? 1 : 2 );
    } else if( ( ( b >> 3 ) & 0x07 ) == 0x06 ) { /* LD (HL),something */
      dest_reg( address, use_hl, buffer2, 40 );
      source_reg( address, USE_HL, buffer3, 40 );
      *length = ( use_hl == USE_HL ? 1 : 2 );
    } else {				/* Does not involve (HL) at all */
      dest_reg( address, use_hl, buffer2, 40 );
      source_reg( address, use_hl, buffer3, 40 );
      *length = 1;
    }
    /* Note LD (HL),(HL) does not exist */

    snprintf( buffer, buflen, "LD %s,%s", buffer2, buffer3 );

  } else if( b < 0xc0 ) {
    *length = 1 + source_reg( address, use_hl, buffer2, 40 );
    snprintf( buffer, buflen, addition_op( b ), buffer2 );
  } else {
    disassemble_11xxxxxx( address, buffer, buflen, length, use_hl );
  }


}

/* Disassemble something of the form 00xxxxxx */
static void
disassemble_00xxxxxx( int address, char *buffer, size_t buflen,
		      size_t *length, enum hl_type use_hl )
{
  const char *opcode_00xxx000[] = {
    "NOP", "EX AF,AF'", "DJNZ ", "JR ", "JR NZ,", "JR Z,", "JR NC,", "JR C,"
  };
  const char *opcode_00xxx111[] = {
    "RLCA", "RRCA", "RLA", "RRA", "DAA", "CPL", "SCF", "CCF"
  };
  char buffer2[40], buffer3[40];

  z80_byte b = disassemble_peek_byte( address );

  switch( b & 0x0f ) {

  case 0x00: case 0x08:
    if( b <= 0x08 ) {
      snprintf( buffer, buflen, "%s", opcode_00xxx000[ b >> 3 ] ); *length = 1;
    } else {
      get_offset( buffer2, 40, address + 2, disassemble_peek_byte( address + 1 ) );
      snprintf( buffer, buflen, "%s%s", opcode_00xxx000[ b >> 3 ], buffer2 );
      *length = 2;
    }
    break;

  case 0x01:
    get_word( buffer2, 40, address + 1 );
    snprintf( buffer, buflen, "LD %s,%s", reg_pair( b, use_hl ), buffer2 );
    *length = 3;
    break;

  case 0x02:
    disassemble_00xxx010( address, buffer, buflen, length, use_hl );
    break;

  case 0x03:
    snprintf( buffer, buflen, "INC %s", reg_pair( b, use_hl ) ); *length = 1;
    break;

  case 0x04: case 0x0c:
    *length = 1 + dest_reg( address, use_hl, buffer2, 40 );
    snprintf( buffer, buflen, "INC %s", buffer2 );
    break;

  case 0x05: case 0x0d:
    *length = 1 + dest_reg( address, use_hl, buffer2, 40 );
    snprintf( buffer, buflen, "DEC %s", buffer2 );
    break;

  case 0x06: case 0x0e:
    *length = 2 + dest_reg( address, use_hl, buffer2, 40 );
    get_byte( buffer3, 40, disassemble_peek_byte( address + *length - 1 ) );
    snprintf( buffer, buflen, "LD %s,%s", buffer2, buffer3 );
    break;

  case 0x07: case 0x0f:
    snprintf( buffer, buflen, "%s", opcode_00xxx111[ b >> 3 ] ); *length = 1;
    break;

  case 0x09:
    snprintf( buffer, buflen, "ADD %s,%s", hl_ix_iy( use_hl ),
	      reg_pair( b, use_hl ) );
    *length = 1;
    break;

  case 0x0a:
    disassemble_00xxx110( address, buffer, buflen, length, use_hl );
    break;

  case 0x0b:
    snprintf( buffer, buflen, "DEC %s", reg_pair( b, use_hl ) );
    *length = 1;
    break;

  }
}

/* Disassemble something of the form 00xxx010 */
static void
disassemble_00xxx010( int address, char *buffer, size_t buflen,
		      size_t *length, enum hl_type use_hl )
{
  char buffer2[40];
  z80_byte b = disassemble_peek_byte( address );

  switch( b >> 4 ) {

  case 0: case 1:
    snprintf( buffer, buflen, "LD (%s),A", reg_pair( b, use_hl ) );
    *length = 1;
    break;

  case 2:
    get_word( buffer2, 40, address + 1 );
    snprintf( buffer, buflen, "LD (%s),%s", buffer2, hl_ix_iy( use_hl ) );
    *length = 3;
    break;

  case 3:
    get_word( buffer2, 40, address + 1 );
    snprintf( buffer, buflen, "LD (%s),A", buffer2 ); *length = 3;
    break;
  }
}

/* Disassemble something of the form 00xxx110 */
static void
disassemble_00xxx110( int address, char *buffer, size_t buflen,
		      size_t *length, enum hl_type use_hl )
{
  char buffer2[40];
  z80_byte b = disassemble_peek_byte( address );

  switch( b >> 4 ) {

  case 0: case 1:
    snprintf( buffer, buflen, "LD A,(%s)", reg_pair( b, use_hl ) );
    *length = 1;
    break;

  case 2:
    get_word( buffer2, 40, address + 1 );
    snprintf( buffer, buflen, "LD %s,(%s)", hl_ix_iy( use_hl ), buffer2 );
    *length = 3;
    break;

  case 3:
    get_word( buffer2, 40, address + 1 );
    snprintf( buffer, buflen, "LD A,(%s)", buffer2 ); *length = 3;
    break;
  }
}

/* Disassemble something of the form 11xxxxxx */
static void
disassemble_11xxxxxx( int address, char *buffer, size_t buflen,
		      size_t *length, enum hl_type use_hl )
{
  char buffer2[40];
  z80_byte b = disassemble_peek_byte( address );

  switch( b & 0x07 ) {

  case 0x00:
    snprintf( buffer, buflen, "RET %s", condition( b ) ); *length = 1;
    break;

  case 0x01:
    disassemble_11xxx001( b, buffer, buflen, length, use_hl );
    break;

  case 0x02:
    get_word( buffer2, 40, address + 1 );
    snprintf( buffer, buflen, "JP %s,%s", condition( b ), buffer2 );
    *length = 3;
    break;

  case 0x03:
    disassemble_11xxx011( address, buffer, buflen, length, use_hl );
    break;

  case 0x04:
    get_word( buffer2, 40, address + 1 );
    snprintf( buffer, buflen, "CALL %s,%s", condition( b ), buffer2 );
    *length = 3;
    break;

  case 0x05:
    disassemble_11xxx101( address, buffer, buflen, length, use_hl );
    break;

  case 0x06:
    get_byte( buffer2, 40, disassemble_peek_byte( address + 1 ) );
    snprintf( buffer, buflen, addition_op( b ), buffer2 );
    *length = 2;
    break;

  case 0x07:
    snprintf( buffer, buflen, "RST %X", 0x08 * ( ( b >> 3 ) - 0x18 ) );
    *length = 1;
    break;
  }
}

/* Disassemble something for the form 11xxx001 */
static void
disassemble_11xxx001( z80_byte b, char *buffer, size_t buflen,
		      size_t *length, enum hl_type use_hl )
{
  switch( ( b >> 3 ) - 0x18 ) {

  case 0x00: case 0x02: case 0x04:
    snprintf( buffer, buflen, "POP %s", reg_pair( b, use_hl ) ); *length = 1;
    break;

  case 0x01: snprintf( buffer, buflen, "RET" ); *length = 1; break;
  case 0x03: snprintf( buffer, buflen, "EXX" ); *length = 1; break;

  case 0x05:
    snprintf( buffer, buflen, "JP %s", hl_ix_iy( use_hl ) ); *length = 1;
    break;

  case 0x06: snprintf( buffer, buflen, "POP AF" ); *length = 1; break;

  case 0x07:
    snprintf( buffer, buflen, "LD SP,%s", hl_ix_iy( use_hl ) ); *length = 1;
    break;
  }
}

/* Disassemble something for the form 11xxx011 */
static void
disassemble_11xxx011( int address, char *buffer, size_t buflen,
		      size_t *length, enum hl_type use_hl )
{
  char buffer2[40];
  z80_byte b = disassemble_peek_byte( address );

  switch( ( b >> 3 ) - 0x18 ) {

  case 0x00:
    get_word( buffer2, 40, address + 1 );
    snprintf( buffer, buflen, "JP %s", buffer2 ); *length = 3;
    break;

  case 0x01:
    if( use_hl != USE_HL ) {
      char offset = disassemble_peek_byte( address + 1 );
      disassemble_ddfd_cb( address+2, offset, use_hl, buffer, buflen,
			   length );
      (*length) += 2;
    } else {
      disassemble_cb( address+1, buffer, buflen, length ); (*length)++;
    }
    break;

  case 0x02:
    get_byte( buffer2, 40, disassemble_peek_byte( address + 1 ) );
    snprintf( buffer, buflen, "OUT (%s),A", buffer2 ); *length = 2;
    break;

  case 0x03:
    get_byte( buffer2, 40, disassemble_peek_byte( address + 1 ) );
    snprintf( buffer, buflen, "IN A,(%s)", buffer2 ); *length = 2;
    break;

  case 0x04:
    snprintf( buffer, buflen, "EX (SP),%s", hl_ix_iy( use_hl ) ); *length = 1;
    break;

  case 0x05:
    /* Note: does not get modified by DD or FD */
    snprintf( buffer, buflen, "EX DE,HL" ); *length = 1;
    break;

  case 0x06:
    snprintf( buffer, buflen, "DI" ); *length = 1;
    break;

  case 0x07:
    snprintf( buffer, buflen, "EI" ); *length = 1;
    break;
  }
}

/* Disassemble something for the form 11xxx101 */
static void
disassemble_11xxx101( int address, char *buffer, size_t buflen,
		      size_t *length, enum hl_type use_hl )
{
  char buffer2[40];
  z80_byte b = disassemble_peek_byte( address );
z80_byte opc=( b >> 3 ) - 0x18;


  //switch( ( b >> 3 ) - 0x18 ) {
  switch( opc ) {

  case 0x00: case 0x02: case 0x04:
    snprintf( buffer, buflen, "PUSH %s", reg_pair( b, use_hl ) ); *length = 1;
    break;

  case 0x01:
    get_word( buffer2, 40, address + 1 );
    snprintf( buffer, buflen, "CALL %s", buffer2 ); *length = 3;
    break;

  case 0x03:
  case 0x07:
	//Prefijo 221/253
	//Proteccion para evitar desbordamientos de pila cuando hay valores del prefijo repetidos
	//printf ("%d\n",disassemble_ddfd_anidado);
	if ( disassemble_ddfd_anidado>0) {
		snprintf( buffer, buflen, "NOPD" );
		debug_printf (VERBOSE_DEBUG,"Reached maximum DD/FD prefixes");
		*length=0; //Al volver la longitud se incrementara
	}
	else {
		disassemble_ddfd_anidado++;
		//printf ("anidar\n");
		if (opc==0x03) disassemble_main( address+1, buffer, buflen, length, USE_IX );
		if (opc==0x07) disassemble_main( address+1, buffer, buflen, length, USE_IY );
		(*length)++;
		//printf ("desanidar\n");
		disassemble_ddfd_anidado--;
	}
    break;

  case 0x05:
    disassemble_ed( address+1, buffer, buflen, length ); (*length)++;
    break;

  case 0x06:
    snprintf( buffer, buflen, "PUSH AF" ); *length = 1;
    break;

/*
  case 0x07:
    disassemble_main( address+1, buffer, buflen, length, USE_IY ); (*length)++;
    break;
*/

  }
}

/* Disassemble an instruction after a CB prefix */
static void
disassemble_cb( int address, char *buffer, size_t buflen,
		size_t *length )
{
  char buffer2[40];
  z80_byte b = disassemble_peek_byte( address );

  source_reg( address, USE_HL, buffer2, 40 );

  if( b < 0x40 ) {
    snprintf( buffer, buflen, "%s %s", rotate_op( b ), buffer2 );
    *length = 1;
  } else {
    snprintf( buffer, buflen, "%s %d,%s", bit_op( b ), bit_op_bit( b ),
	      buffer2 );
    *length = 1;
  }
}

/* Disassemble an instruction after an ED prefix */
static void
disassemble_ed( int address, char *buffer, size_t buflen,
		size_t *length )
{
  z80_byte b;
  char buffer2[40];

  const char *opcode_01xxx111[] = {
    "LD I,A", "LD R,A", "LD A,I", "LD A,R", "RRD", "RLD", "NOPD", "NOPD"
  };

  /* Note 0xbc to 0xbf removed before this table is used */
  const char *opcode_101xxxxx[] = {
    "LDI",  "CPI",  "INI",  "OUTI", "NOPD", "NOPD", "NOPD", "NOPD",
    "LDD",  "CPD",  "IND",  "OUTD", "NOPD", "NOPD", "NOPD", "NOPD",
    "LDIR", "CPIR", "INIR", "OTIR", "NOPD", "NOPD", "NOPD", "NOPD",
    "LDDR", "CPDR", "INDR", "OTDR"
  };

  /* The order in which the IM x instructions appear */
  const int im_modes[] = { 0, 0, 1, 2 };

  b = disassemble_peek_byte( address );

  if( b < 0x40 || b > 0xbb ) {
    snprintf( buffer, buflen, "NOPD" ); *length = 1;
  } else if( b < 0x80 ) {

    switch( b & 0x0f ) {

    case 0x00: case 0x08:
      if( b == 0x70 ) {
	snprintf( buffer, buflen, "IN F,(C)" ); *length = 1;
      } else {
	dest_reg( address, USE_HL, buffer2, 40 );
	snprintf( buffer, buflen, "IN %s,(C)", buffer2 ); *length = 1;
      }
      break;

    case 0x01: case 0x09:
      if( b == 0x71 ) {
	snprintf( buffer, buflen, "OUT (C),0" ); *length = 1;
      } else {
	dest_reg( address, USE_HL, buffer2, 40 );
	snprintf( buffer, buflen, "OUT (C),%s", buffer2 ); *length = 1;
      }
      break;

    case 0x02:
      snprintf( buffer, buflen, "SBC HL,%s", reg_pair( b, USE_HL ) );
      *length = 1;
      break;

    case 0x03:
      get_word( buffer2, 40, address + 1 );
      snprintf( buffer, buflen, "LD (%s),%s", buffer2, reg_pair( b, USE_HL ) );
      *length = 3;
      break;

    case 0x04: case 0x0c:
      snprintf( buffer, buflen, "NEG" ); *length = 1;
      break;

    case 0x05: case 0x0d:
      if( b == 0x4d ) {
	snprintf( buffer, buflen, "RETI" ); *length = 1;
      } else {
	snprintf( buffer, buflen, "RETN" ); *length = 1;
      }
      break;

    case 0x06: case 0x0e:
      snprintf( buffer, buflen, "IM %d", im_modes[ ( b >> 3 ) & 0x03 ] );
      *length = 1;
      break;

    case 0x07: case 0x0f:
      snprintf( buffer, buflen, "%s", opcode_01xxx111[ ( b >> 3 ) & 0x07 ] );
      *length = 1;
      break;

    case 0x0a:
      snprintf( buffer, buflen, "ADC HL,%s", reg_pair( b, USE_HL ) );
      *length = 1;
      break;

    case 0x0b:
      get_word( buffer2, 40, address + 1 );
      snprintf( buffer, buflen, "LD %s,(%s)", reg_pair( b, USE_HL ), buffer2 );
      *length = 3;
      break;

    }
  } else if( b < 0xa0 ) {
    snprintf( buffer, buflen, "NOPD" ); *length = 1; *length = 1;
  } else {
    /* Note: 0xbc to 0xbf already removed */
    snprintf( buffer, buflen, "%s", opcode_101xxxxx[ b & 0x1f ] ); *length = 1;
  }
}

/* Disassemble an instruction after DD/FD CB prefixes */
static void
disassemble_ddfd_cb( int address, char offset,
		     enum hl_type use_hl, char *buffer, size_t buflen,
		     size_t *length )
{
  z80_byte b = disassemble_peek_byte( address );
  char buffer2[40], buffer3[40];

  if( b < 0x40 ) {
    if( ( b & 0x07 ) == 0x06 ) {
      ix_iy_offset( buffer2, 40, use_hl, offset );
      snprintf( buffer, buflen, "%s %s", rotate_op( b ), buffer2 );
      *length = 1;
    } else {
      source_reg( address, USE_HL, buffer2, 40 );
      ix_iy_offset( buffer3, 40, use_hl, offset );
      snprintf( buffer, buflen, "LD %s,%s %s", buffer2,
		rotate_op( b ), buffer3 );
      *length = 1;
    }
  } else if( b < 0x80 ) {
    ix_iy_offset( buffer2, 40, use_hl, offset );
    snprintf( buffer, buflen, "BIT %d,%s", ( b >> 3 ) & 0x07, buffer2 );
    *length = 1;
  } else {
    if( ( b & 0x07 ) == 0x06 ) {
      ix_iy_offset( buffer2, 40, use_hl, offset );
      snprintf( buffer, buflen, "%s %d,%s", bit_op( b ), bit_op_bit( b ),
		buffer2 );
      *length = 1;
    } else {
      source_reg( address, USE_HL, buffer2, 40 );
      ix_iy_offset( buffer3, 40, use_hl, offset );
      snprintf( buffer, buflen, "LD %s,%s %s", buffer2, bit_op( b ), buffer3 );
      *length = 1;
    }
  }
}

/* Get a text representation of a one-byte number */
static void
get_byte( char *buffer, size_t buflen, z80_byte b )
{
	if (disassemble_show_value.v==1)  snprintf( buffer, buflen, debugger_output_base == 10 ? "%d" : "%02X", b );
	else snprintf( buffer, buflen, "NN" );
}

/* Get a text representation of an (LSB) two-byte number */
static void
get_word( char *buffer, size_t buflen, int address )
{
  z80_int w;

  w  = disassemble_peek_byte( address + 1 ); w <<= 8;
  w += disassemble_peek_byte( address     );

  if (disassemble_show_value.v==1)  snprintf( buffer, buflen, debugger_output_base == 10 ? "%d" : "%04X", w);
  else snprintf( buffer, buflen, "NNNN");
}

/* Get a text representation of ( 'address' + 'offset' ) */
static void
get_offset( char *buffer, size_t buflen, z80_int address,
	    z80_byte offset )
{
  address += ( offset >= 0x80 ? offset-0x100 : offset );
  if (disassemble_show_value.v==1) snprintf( buffer, buflen, debugger_output_base == 10 ? "%d" : "%04X", address );
  else snprintf( buffer, buflen, "NNNN");
}

/* Select the appropriate register pair from BC, DE, HL (or IX, IY) or
   SP, depending on bits 4 and 5 of the opcode */
static const char *
reg_pair( z80_byte b, enum hl_type use_hl )
{
  switch( ( b >> 4 ) & 0x03 ) {
  case 0: return "BC";
  case 1: return "DE";
  case 2: return hl_ix_iy( use_hl );
  case 3: return "SP";
  }
  return "* INTERNAL ERROR *";	/* Should never happen */
}

/* Get whichever of HL, IX or IY is in use here */
static const char *
hl_ix_iy( enum hl_type use_hl )
{
  switch( use_hl ) {
  case USE_HL: return "HL";
  case USE_IX: return "IX";
  case USE_IY: return "IY";
  }
  return "* INTERNAL ERROR *";	/* Should never happen */
}

/* Get a text representation of '(IX+03)' or similar things */
static void
ix_iy_offset( char *buffer, size_t buflen, enum hl_type use_hl,
	      z80_byte offset )
{


if (disassemble_show_value.v==1) {

  if( offset < 0x80 ) {
    snprintf( buffer, buflen, debugger_output_base == 10 ? "(%s+%d)" : "(%s+%02X)", hl_ix_iy( use_hl ), offset );
  }


  else {
    snprintf( buffer, buflen, debugger_output_base == 10 ? "(%s-%d)" : "(%s-%02X)", hl_ix_iy( use_hl ), 0x100 - offset );
  }

}

else {
	snprintf( buffer, buflen, "(%s+dd)", hl_ix_iy( use_hl ) );
}


}

/* Get an 8-bit register, based on bits 0-2 of the opcode at 'address' */
static int
source_reg( int address, enum hl_type use_hl, char *buffer,
	    size_t buflen )
{
  return single_reg( disassemble_peek_byte( address ) & 0x07, use_hl,
		     disassemble_peek_byte( address + 1 ), buffer, buflen );
}

/* Get an 8-bit register, based on bits 3-5 of the opcode at 'address' */
static int
dest_reg( int address, enum hl_type use_hl, char *buffer,
	  size_t buflen )
{
  return single_reg( ( disassemble_peek_byte( address ) >> 3 ) & 0x07, use_hl,
		     disassemble_peek_byte( address + 1 ), buffer, buflen );
}

/* Get an 8-bit register name, including (HL). Also substitutes
   IXh, IXl and (IX+nn) and the IY versions if appropriate */
static int
single_reg( int i, enum hl_type use_hl, z80_byte offset,
	    char *buffer, size_t buflen )
{
  char buffer2[40];

  if( i == 0x04 && use_hl != USE_HL ) {
    snprintf( buffer, buflen, "%sh", hl_ix_iy( use_hl ) );
    return 0;
  } else if( i == 0x05 && use_hl != USE_HL ) {
    snprintf( buffer, buflen, "%sl", hl_ix_iy( use_hl ) );
    return 0;
  } else if( i == 0x06 && use_hl != USE_HL ) {
    ix_iy_offset( buffer2, 40, use_hl, offset );
    snprintf( buffer, buflen, "%s", buffer2 );
    return 1;
  } else {
    const char *regs[] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
    snprintf( buffer, buflen, "%s", regs[i] );
    return 0;
  }
}

/* Various lookup tables for opcodes */

/* Addition/subtraction opcodes:
   10xxxrrr: 'xxx' selects the opcode and 'rrr' the register to be added
   11xxx110: 'xxx' selects the opcode and add a constant
*/
static const char *
addition_op( z80_byte b )
{
  const char *ops[] = { "ADD A,%s", "ADC A,%s", "SUB %s", "SBC A,%s",
			"AND %s",   "XOR %s",   "OR %s",  "CP %s"     };
  return ops[ ( b >> 3 ) & 0x07 ];
}

/* Conditions for jumps, etc:
   11xxx000: RET condition
   11xxx010: JP condition,nnnn
   11xxx100: CALL condition,nnnn
*/
static const char *
condition( z80_byte b )
{
  const char *conds[] = { "NZ", "Z", "NC", "C", "PO", "PE", "P", "M" };
  return conds[ ( b >> 3 ) & 0x07 ];
}

/* Shift/rotate operations:
   CB 00xxxrrr: 'xxx' selects the opcode and 'rrr' the register
   DD/FD CB <offset> 00xxxrrr: the documented rotate/shifts on (IX+nn) etc
                               and the undocumented rotate-and-store opcodes
*/
static const char *
rotate_op( z80_byte b )
{
  const char *ops[] = { "RLC", "RRC", "RL", "RR", "SLA", "SRA", "SLL", "SRL" };
  return ops[ b >> 3 ];
}

/* Bit operations:
   CB oobbbrrr: 'oo' (not 00) selects operation
                'bbb' selects bit
                'rrr' selects register
   DD/FD CB <offset> oobbbrrr: the documented bit ops on (IX+nn) etc and the
                               undocumented bit-op-and store
*/
static const char *
bit_op( z80_byte b )
{
  const char *ops[] = { "BIT", "RES", "SET" };
  return ops[ ( b >> 6 ) - 1 ];
}

/* Which bit is used by a BIT, RES or SET with this opcode (bits 3-5) */
static int
bit_op_bit( z80_byte b )
{
  return ( b >> 3 ) & 0x07;
}
