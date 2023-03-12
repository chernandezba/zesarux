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

#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include "cpu.h"


enum token_parser_tipo {
	TPT_FIN, //fin de expresion
	TPT_PARENTESIS, //parentesis
	TPT_NUMERO,
	TPT_VARIABLE, //mra, mrw, etc
	TPT_REGISTRO, //a, bc, de, etc
	TPT_OPERADOR_LOGICO, //and, or, xor
	TPT_OPERADOR_CONDICIONAL, //=, <,>, <>,
	TPT_OPERADOR_CALCULO, //+,-,*,/. & (and), | (or), ^ (xor)
	TPT_FUNCION //peek(, peekw (, not() etc
};

enum token_parser_indice {
	TPI_FIN, //para indicar final de array


	//tipo parentesis
	TPI_P_ABRIR,
	TPI_P_CERRAR,

	//de tipo variable
  	TPI_V_MRA,
	TPI_V_MRV,

	TPI_V_MWV,
	TPI_V_MWA,

	//Puertos
	TPI_V_PRV,
	TPI_V_PRA,

	TPI_V_PWV,
	TPI_V_PWA,

	TPI_V_TSTATES,
	TPI_V_TSTATESL,
	TPI_V_TSTATESP,

	TPI_V_SCANLINE,


	TPI_V_IFF1,
	TPI_V_IFF2,

	TPI_V_OUTFIRED,
	TPI_V_INFIRED,
	TPI_V_INTFIRED,

	TPI_V_ENTERROM,
	TPI_V_EXITROM,

	TPI_V_SEG0,
	TPI_V_SEG1,
	TPI_V_SEG2,
	TPI_V_SEG3,
	TPI_V_SEG4,
	TPI_V_SEG5,
	TPI_V_SEG6,
	TPI_V_SEG7,

    TPI_V_HILOWMAPPED,

	TPI_V_RAM,
	TPI_V_ROM,

    TPI_V_PD765_PCN,

	TPI_V_OPCODE1,
	TPI_V_OPCODE2,
	TPI_V_OPCODE3,
	TPI_V_OPCODE4,

	//de tipo registro

	TPI_R_PC,
    TPI_R_SP,
	TPI_R_USP, //de motorola
    TPI_R_IX,
    TPI_R_IY,	

	TPI_R_A,
	TPI_R_B,
	TPI_R_C,
	TPI_R_D,
	TPI_R_E,
	TPI_R_F,
	TPI_R_H,
	TPI_R_L,
	TPI_R_I,
	TPI_R_R,

        TPI_R_AF,
        TPI_R_BC,
        TPI_R_DE,
        TPI_R_HL,

	TPI_R_A_SHADOW,
	TPI_R_B_SHADOW,
	TPI_R_C_SHADOW,
	TPI_R_D_SHADOW,
	TPI_R_E_SHADOW,
	TPI_R_F_SHADOW,
	TPI_R_H_SHADOW,
	TPI_R_L_SHADOW,

        TPI_R_AF_SHADOW,
        TPI_R_BC_SHADOW,
        TPI_R_DE_SHADOW,
        TPI_R_HL_SHADOW,

        TPI_R_FS,
        TPI_R_FZ,
        TPI_R_FP,
        TPI_R_FV,    
        TPI_R_FH,
        TPI_R_FN,
        TPI_R_FC,


	//De motorola
	TPI_R_D0,
	TPI_R_D1,
	TPI_R_D2,
	TPI_R_D3,
	TPI_R_D4,
	TPI_R_D5,
	TPI_R_D6,
	TPI_R_D7,

	TPI_R_A0,
	TPI_R_A1,
	TPI_R_A2,
	TPI_R_A3,
	TPI_R_A4,
	TPI_R_A5,
	TPI_R_A6,
	TPI_R_A7,	

	//De SCMP
	TPI_R_AC,
    TPI_R_ER,
    TPI_R_SR,
    TPI_R_P1,
    TPI_R_P2,
    TPI_R_P3,

	//De Tbblue
	TPI_R_COPPERPC,

	//de tipo operador logico
	TPI_OL_AND,
	TPI_OL_OR,
	TPI_OL_XOR,
	//de tipo condicional
	TPI_OC_IGUAL,
	TPI_OC_MENOR,
	TPI_OC_MAYOR,
	TPI_OC_DIFERENTE,

	TPI_OC_MENOR_IGUAL,
	TPI_OC_MAYOR_IGUAL,

	//de tipo operador calculo
	TPI_OC_SUMA,
	TPI_OC_RESTA,
	TPI_OC_MULTIPLICACION,
	TPI_OC_DIVISION,
	TPI_OC_AND,	
	TPI_OC_OR,
	TPI_OC_XOR,

	TPI_F_PEEK,
	TPI_F_PEEKW,
    TPI_F_FPEEK,
	TPI_F_IN,
	TPI_F_NOT,
	TPI_F_ABS,
	TPI_F_BYTE,
	TPI_F_WORD,
	TPI_F_OPMWA,
	TPI_F_OPMRA,
	TPI_F_OPMWV,
	TPI_F_OPMRV

};

enum token_parser_formato {
	TPF_DECIMAL,
	TPF_HEXADECIMAL,
	TPF_BINARIO,
	TPF_ASCII
};

struct s_token_parser {

	enum token_parser_tipo tipo; //(variable, registro, operador, número, fin, etc)
	enum token_parser_indice indice; //(si es registro , podría ser: 0 para registro A, 1 para registro B, etc)
	enum token_parser_formato formato; //(para números, decir si se muestra como hexadecimal, binario, ascii etc)
	int signo; //(signo del valor almacenado)
	int  valor; //(guarda el valor absoluto en variable int)

};

typedef struct s_token_parser token_parser;



//Usados en la conversion de texto a tokens
#define MAX_PARSER_TEXTOS_INDICE_LENGTH 40
//metemos 40 para que se pueda poner de manera holgada una cadena binaria de 32 bits
struct s_token_parser_textos_indices {
	enum token_parser_indice indice;
	char texto[MAX_PARSER_TEXTOS_INDICE_LENGTH];
};

typedef struct s_token_parser_textos_indices token_parser_textos_indices;

#define MAX_PARSER_TOKENS_NUM 100

extern int exp_par_exp_to_tokens(char *expression,token_parser *tokens);
extern void exp_par_tokens_to_exp(token_parser *tokens,char *expression,int maximo);
int exp_par_evaluate_token(token_parser *tokens,int final,int *error_code);

extern void exp_par_debug_dump_tokens(token_parser *tokens,int longitud);

extern int exp_par_evaluate_expression(char *entrada,char *salida,char *string_detoken);

extern int exp_par_is_number(char *texto,int *final);

extern int exp_par_evaluate_expression_to_number(char *entrada);

#endif
