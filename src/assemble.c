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
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>



#include "cpu.h"
#include "assemble.h"
#include "debug.h"
#include "utils.h"



//Assembler. Ver http://www.z80.info/decoding.htm

//Retorna opcode y primer y segundo operador
/*
Formato entrada: 


OPCODE 
OPCODE OP1
OPCODE OP1,OP2  

Retorna puntero a primer parametro, util para comandos como DEFB 0,0,0,.....
*/
char *asm_return_op_ops(char *origen,char *opcode,char *primer_op,char *segundo_op)
{
	//Primero asumimos todos nulos
	*opcode=0;
	*primer_op=0;
	*segundo_op=0;

        char *puntero_primer_parametro;

	//Opcode empieza es hasta el espacio
	int i;
	for (i=0;origen[i] && origen[i]!=' ';i++) {
		*opcode=origen[i];
		opcode++;
	}

	*opcode=0;

	//Buscamos hasta algo diferente de espacio
	for (;origen[i]==' ';i++) {
        }


        puntero_primer_parametro=&origen[i];

	//Primer operador es hasta la ,
	for (;origen[i] && origen[i]!=',';i++) {
                *primer_op=origen[i];
                primer_op++;
        }

        *primer_op=0;
	
	if (origen[i]==',') i++;

	//Y ya hasta final de cadena
        for (;origen[i];i++) {
                *segundo_op=origen[i];
                segundo_op++;
        }

        *segundo_op=0;

        return puntero_primer_parametro;

}


/*
Tablas de opcodes para ensamblado:

-Nombre opcode: LD, INC, HALT, etc
-De cada tipo opcode, mascara opcode, tipo parametros que admite, cuantos parametros, y mascara parametro:
Ejemplos:   LD r,n .  LD RR,NN.    o HALT (sin parametros).

LD r,n. base opcode=6 (00000110). tipo parametros: r. mascara parametro 1: 00XXX000
Por ejemplo, si LD A,33 -> A en tabla r vale 7. 
Valor final:
00000110  OR 00111000 = 00111110 = 62

La mascara de operador no tiene mucho sentido el numero de bits en mascara, solo el primer bit de la derecha que esta a 1,
dado que meteremos el valor del operador final ahí con un OR, rotando tantos bits a la izquierda como corresponda

--Tipos parametros:
n
nn
dis
r
rp (bc,de,hl,sp)
rp2 (bc,de,hl,af)
cc (nz,z,nc,c,po,pe,p,m)
string tal cual (como "AF'" en "ex af,af'"), o como ("1" en "IM 1"), o como ("HL" en "JP HL")


-Casos especiales: EX AF,AF' -> un solo opcode sin parametros. Quiza en estos casos decir: opcode=EX. parametro 1=string=AF, parametro 2=string=AF'

-cada opcode en strings apartes:
char *asm_opcode_ld="LD";
char *asm_opcode_inc="INC",

en tabla opcodes:
{ asm_opcode_ld,r,n },
{ asm_opcode_ld,rr,nn } ,
{ asm_opcode_inc,r } ,
{ asm_opcode_inc,rr }

Para no tener que repetir strings (guardamos solo el char *)

*/

enum asm_tipo_parametro
{
        ASM_PARM_NONE,
	ASM_PARM_CONST, //caso de EX AF,AF'
	ASM_PARM_R,
	ASM_PARM_RP,
	ASM_PARM_RP2,
        ASM_PARM_CC,
        ASM_PARM_PARENTHESIS_N,
        ASM_PARM_PARENTHESIS_NN,
	ASM_PARM_RST,
	ASM_PARM_IM,
	ASM_PARM_BIT,
	ASM_PARM_N,
	ASM_PARM_NN,
	ASM_PARM_DIS
};

enum asm_tipo_parametro_entrada
{
	//ASM_PARM_IN_NONE,
        ASM_PARM_IN_R,
        ASM_PARM_IN_RP,
        ASM_PARM_IN_RP2,
        ASM_PARM_IN_CC,
        ASM_PARN_IN_PARENTHESIS_NUMERO,
	ASM_PARM_IN_NUMERO
};

struct s_tabla_ensamblado {
        char *texto_opcode;
        int mascara_opcode;
        int prefijo; //203,221,237,251

        int tipo_parametro_1;
        int desplazamiento_mascara_p1; //Cuantos bits a desplazar a la izquierda
	char *const_parm1; //Para EX AF,AF'

        int tipo_parametro_2;
        int desplazamiento_mascara_p2;
	char *const_parm2; //Para EX AF,AF'
};

typedef struct s_tabla_ensamblado tabla_ensamblado;

//char *ensamblado_opcode_nop="NOP";
//char *ensabmlado_opcode_ld="LD";

//Poner antes los que dependen de constantes que no registros variables
//Poner antes que los nn, los registros
tabla_ensamblado array_tabla_ensamblado[]={
        {"NOP",0,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},

        //LD
        {"LD",2,0,  ASM_PARM_CONST,0,"(BC)", ASM_PARM_CONST, 0,"A"},   //LD (BC),A
        {"LD",10,0,  ASM_PARM_CONST,0,"A", ASM_PARM_CONST, 0,"(BC)"},   //LD A,(BC)
        {"LD",18,0,  ASM_PARM_CONST,0,"(DE)", ASM_PARM_CONST, 0,"A"},   //LD (DE),A
        {"LD",26,0,  ASM_PARM_CONST,0,"A", ASM_PARM_CONST, 0,"(DE)"},   //LD A,(DE)
        {"LD",34,0,  ASM_PARM_PARENTHESIS_NN,0,NULL, ASM_PARM_CONST, 0,"HL"},   //LD (NNNN),HL
		{"LD",42,0,  ASM_PARM_CONST, 0,"HL", ASM_PARM_PARENTHESIS_NN,0,NULL, },   //LD HL,(NNNN)

		//Tiene que estar estos antes, si no, LD A,I lo interceptaria como LD A,N
		{"LD",87,237, ASM_PARM_CONST,0,"A",   ASM_PARM_CONST,0,"I"},   //LD A,I
		{"LD",95,237, ASM_PARM_CONST,0,"A", ASM_PARM_CONST,0,"R"  },   //LD A,R


	{"LD",50,0,  ASM_PARM_PARENTHESIS_NN,0,NULL, ASM_PARM_CONST, 0,"A"},   //LD (NNNN),A
	{"LD",58,0,  ASM_PARM_CONST, 0,"A", ASM_PARM_PARENTHESIS_NN,0,NULL },   //LD A,(NNNN)

        {"LD",64,0, ASM_PARM_R,3,NULL,  ASM_PARM_R,  0,NULL},   //LD r,r   

        {"LD",1,0,    ASM_PARM_RP,4,NULL, ASM_PARM_NN, 0,NULL},   //LD rp,NN     
        {"LD",6,0,  ASM_PARM_R,3,NULL,  ASM_PARM_N,  0,NULL},   //LD r,n           


        {"INC",3,0,    ASM_PARM_RP,4,NULL, ASM_PARM_NONE, 0,NULL},   //INC rp        
        {"INC",4,0,  ASM_PARM_R,3,NULL,  ASM_PARM_NONE,  0,NULL},   //INC r
        {"DEC",5,0,  ASM_PARM_R,3,NULL,  ASM_PARM_NONE,  0,NULL},   //DEC r
     

        {"RLCA",7,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},
        {"EX",8,0,  ASM_PARM_CONST,0,"AF", ASM_PARM_CONST, 0,"AF'"},   //EX AF,AF'
        {"ADD",9,0, ASM_PARM_CONST,0,"HL", ASM_PARM_RP,4,NULL}, //ADD HL,rp


        {"DEC",11,0,    ASM_PARM_RP,4,NULL, ASM_PARM_NONE, 0,NULL},   //DEC rp

        {"RRCA",15,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},
        {"DJNZ",16,0, ASM_PARM_DIS,0,NULL, ASM_PARM_NONE,0,NULL},

        {"RLA",23,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},

        {"JR",24,0, ASM_PARM_DIS,0,NULL, ASM_PARM_NONE,0,NULL},
        {"RRA",31,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},

        {"JR",32,0, ASM_PARM_CC,3,NULL, ASM_PARM_DIS,0,NULL},     

        {"DAA",39,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   
	{"CPL",47,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},  
	{"SCF",55,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   
	{"CCF",63,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL}, 

	{"HALT",118,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   

	{"ADD",128,0, ASM_PARM_CONST,0,"A",  ASM_PARM_R,  0,NULL},   //ADD A,r  
	{"ADC",136,0, ASM_PARM_CONST,0,"A",  ASM_PARM_R,  0,NULL},   //ADC A,r   

	{"SUB",144,0, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   //SUB r 
	{"SBC",152,0, ASM_PARM_CONST,0,"A",  ASM_PARM_R,  0,NULL},   //SBC A,r   
	{"AND",160,0, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   //AND r 
	{"XOR",168,0, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   //XOR r 
	{"OR",176,0, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   //OR r 
	{"CP",184,0, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   //CP r 

	{"RET",192,0, ASM_PARM_CC,  3,NULL, ASM_PARM_NONE,0,NULL},   //RET cc 
	{"POP",193,0,  ASM_PARM_RP2,4,NULL, ASM_PARM_NONE, 0,NULL},   //POP rp2   
	{"JP",194,0, ASM_PARM_CC,3,NULL, ASM_PARM_NN,0,NULL},    //JP CC,NN   
	{"JP",195,0, ASM_PARM_NN,0,NULL, ASM_PARM_NONE, 0,NULL },    //JP NN  
	{"CALL",196,0, ASM_PARM_CC,3,NULL, ASM_PARM_NN,0,NULL},    //CALL CC,NN    

	{"PUSH",197,0,  ASM_PARM_RP2,4,NULL, ASM_PARM_NONE, 0,NULL},   //PUSH rp2  

	{"ADD",198,0, ASM_PARM_CONST,0,"A",  ASM_PARM_N,  0,NULL},   //ADD A,N   

	{"RST",199,0,  ASM_PARM_RST,3,NULL, ASM_PARM_NONE, 0,NULL},   //RST n   

	{"RET",201,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},    //RET
	{"CALL",205,0, ASM_PARM_NN,0,NULL, ASM_PARM_NONE,0,NULL},   //CALL NN
	{"ADC",206,0, ASM_PARM_CONST,0,"A",  ASM_PARM_N,  0,NULL},   //ADC A,N  

	//Este OUT tiene que estar antes del otro
	{"OUT",65,237, ASM_PARM_CONST,0,"(C)",  ASM_PARM_R,  3,NULL},   //OUT (C),r
	{"OUT",211,0,  ASM_PARM_PARENTHESIS_N,0,NULL, ASM_PARM_CONST, 0,"A"},   //OUT (N),A

	{"SUB",214,0, ASM_PARM_N,  0,NULL, ASM_PARM_NONE,0,NULL  },   //SUB N  

	{"EXX",217,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   


	//Este IN tiene que estar antes del otro
	{"IN",64,237, ASM_PARM_R,  3,NULL, ASM_PARM_CONST,0,"(C)"},   //IN r,(C) 

	{"IN",219,0,  ASM_PARM_CONST, 0,"A", ASM_PARM_PARENTHESIS_N,0,NULL },   //IN A,(N)


	{"SBC",222,0, ASM_PARM_CONST,0,"A",  ASM_PARM_N,  0,NULL},   //SBC A,N 

	{"EX",227,0,  ASM_PARM_CONST, 0,"(SP)", ASM_PARM_CONST,0,"HL" },   //EX (SP),HL

	{"AND",230,0, ASM_PARM_N,  0,NULL, ASM_PARM_NONE,0,NULL  },   //AND N  	

	{"JP",233,0, ASM_PARM_CONST,  0,"HL", ASM_PARM_NONE,0,NULL  },   //JP HL	
	{"JP",233,0, ASM_PARM_CONST,  0,"(HL)", ASM_PARM_NONE,0,NULL  },   //JP (HL)

	{"EX",235,0,  ASM_PARM_CONST, 0,"DE", ASM_PARM_CONST,0,"HL" },   //EX DE,HL	

	{"XOR",238,0, ASM_PARM_N,  0,NULL, ASM_PARM_NONE,0,NULL  },   //XOR N  

	{"DI",243,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},  //DI

	{"OR",246,0, ASM_PARM_N,  0,NULL, ASM_PARM_NONE,0,NULL  },   //OR N  


	{"LD",249,0,  ASM_PARM_CONST, 0,"SP", ASM_PARM_CONST,0,"HL" },   //LD SP,HL

	{"EI",251,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},  //EI	

	{"CP",254,0, ASM_PARM_N,  0,NULL, ASM_PARM_NONE,0,NULL  },   //CP N 





	//ED opcodes
	{"SBC",66,237, ASM_PARM_CONST,0,"HL",  ASM_PARM_RP,  4,NULL},   //SBC HL,rp
	{"LD",67,237, ASM_PARM_PARENTHESIS_NN,0,NULL,  ASM_PARM_RP,  4,NULL},   //LD (NN),rp

	{"NEG",68,237, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   //NEG
	{"RETN",69,237, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   //RETN
	{"IM",70,237,  ASM_PARM_IM,3,NULL, ASM_PARM_NONE, 0,NULL},   //IM 0,1,2
	{"LD",71,237, ASM_PARM_CONST,0,"I",  ASM_PARM_CONST,0,"A"},   //LD I,A 	

	{"ADC",74,237, ASM_PARM_CONST,0,"HL",  ASM_PARM_RP,  4,NULL},   //ADC HL,rp
	{"LD",75,237,  ASM_PARM_RP,  4,NULL,  ASM_PARM_PARENTHESIS_NN,0,NULL},   //LD rp,(NN)

	{"RETI",77,237, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   //RETN
	{"LD",79,237, ASM_PARM_CONST,0,"R",  ASM_PARM_CONST,0,"A"},   //LD R,A 	

	{"RRD",103,237, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},
	{"RLD",111,237, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},

	{"IN",112,237, ASM_PARM_CONST,0,"F",  ASM_PARM_CONST,0,"(C)"},   //IN F,(C)
	{"OUT",113,237, ASM_PARM_CONST,0,"(C)", ASM_PARM_CONST,0,"0"  },   //OUT (C),0
	
	{"LDI",160,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   
	{"CPI",161,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL}, 
	{"INI",162,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},
	{"OUTI",163,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   

	{"LDD",168,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   
	{"CPD",169,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL}, 
	{"IND",170,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},
	{"OUTD",171,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL}, 	

	{"LDIR",176,237, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL}, 
	{"CPIR",177,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL}, 
	{"INIR",178,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},
	{"OTIR",179,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   


	{"LDDR",184,237, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},   
	{"CPDR",185,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL}, 
	{"INDR",186,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},
	{"OTDR",187,237,  ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL}, 	

	//CB opcodes
	{"RLC",0,203, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   
	{"RRC",8,203, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   
	{"RL",16,203, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   
	{"RR",24,203, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   
	{"SLA",32,203, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   
	{"SRA",40,203, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL}, 
	{"SLL",48,203, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL},   
	{"SRL",56,203, ASM_PARM_R,  0,NULL, ASM_PARM_NONE,0,NULL}, 
	{"BIT",64,203, ASM_PARM_BIT,3,NULL, ASM_PARM_R,  0,NULL }, //BIT x,r  
	{"RES",128,203, ASM_PARM_BIT,3,NULL, ASM_PARM_R,  0,NULL }, //RES x,r  
	{"SET",192,203, ASM_PARM_BIT,3,NULL, ASM_PARM_R,  0,NULL }, //RES x,r  


        {NULL,0,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL}
};

char *asm_parametros_tipo_r[]={
	"B","C","D","E","H","L","(HL)","A",
	NULL
};

char *asm_parametros_tipo_rp[]={
	"BC","DE","HL","SP",
	NULL
};

char *asm_parametros_tipo_rp2[]={
	"BC","DE","HL","AF",
	NULL
};

char *asm_parametros_tipo_cc[]={
	"NZ","Z","NC","C","PO","PE","P","M",
	NULL
};

//Buscar en el array de char* de parametros si coincide y su valor
char *assemble_find_array_params(char *texto_buscar,char *tabla[],unsigned int *valor)
{
	int i;

	for (i=0;tabla[i]!=NULL;i++) {
		if (!strcasecmp(texto_buscar,tabla[i])) {
			*valor=i;
			return tabla[i];
		}
	}

	return NULL;
}




//Devuelve tipo de parametro en entrada, de enum asm_tipo_parametro_entrada
//Si no es de ninguna tabla, retornara numero (N o NN)
int assemble_find_param_type(char *buf_op,unsigned int *valor)
{
	char *tabla;

        //Buscar primero en tabla R
        tabla=assemble_find_array_params(buf_op,asm_parametros_tipo_r,valor);
	if (tabla!=NULL) return ASM_PARM_IN_R;

        tabla=assemble_find_array_params(buf_op,asm_parametros_tipo_rp,valor);
	if (tabla!=NULL) return ASM_PARM_IN_RP;

        tabla=assemble_find_array_params(buf_op,asm_parametros_tipo_rp2,valor);
	if (tabla!=NULL) return ASM_PARM_IN_RP2;

        tabla=assemble_find_array_params(buf_op,asm_parametros_tipo_cc,valor);
	if (tabla!=NULL) return ASM_PARM_IN_CC;

        //Tipo (NN)
        if (buf_op[0]=='(') return ASM_PARN_IN_PARENTHESIS_NUMERO;

	//final
	return ASM_PARM_IN_NUMERO;
}


//Si coincide el tipo de parametro de entrada con el de la tabla
int asm_check_parameter_in_table(enum asm_tipo_parametro_entrada tipo_parametro_entrada,enum asm_tipo_parametro tipo_en_tabla)
{

        //


	//Para cada caso
	switch (tipo_parametro_entrada) {
		case ASM_PARM_IN_R:
			if (tipo_en_tabla==ASM_PARM_R) return 1;


			else return 0;
		break;

		case ASM_PARM_IN_RP:
		case ASM_PARM_IN_RP2:
			//TODO: controlar AF y SP. En caso de RP, no permite AF. En caso de RP2, no permite SP
			if (tipo_en_tabla==ASM_PARM_RP || tipo_en_tabla==ASM_PARM_RP2) return 1;
			else return 0;
		break;

		case ASM_PARM_IN_CC:
			if (tipo_en_tabla==ASM_PARM_CC) return 1;

                        
			else return 0;
		break;   

		case ASM_PARN_IN_PARENTHESIS_NUMERO:
			if (tipo_en_tabla==ASM_PARM_PARENTHESIS_N || tipo_en_tabla==ASM_PARM_PARENTHESIS_NN) return 1;
			else return 0;
		break;                             

		case ASM_PARM_IN_NUMERO:
			if (tipo_en_tabla==ASM_PARM_N || tipo_en_tabla==ASM_PARM_NN || tipo_en_tabla==ASM_PARM_DIS || tipo_en_tabla==ASM_PARM_RST || tipo_en_tabla==ASM_PARM_IM || tipo_en_tabla==ASM_PARM_BIT) return 1;
			else return 0;
		break;

	}

	return 0;

}

int assemble_sustituir_ixiy_despl_aux(int es_prefijo_ix_iy,char *buf_op,char *valor_desplazamiento)
{
	if (buf_op[0]=='(') {
		if (letra_mayuscula(buf_op[1])=='I') {
			if (letra_mayuscula(buf_op[2])!='X' && letra_mayuscula(buf_op[2])!='Y') return es_prefijo_ix_iy;

			if (buf_op[3]!='+' && buf_op[3]!='-') return es_prefijo_ix_iy;

			if (letra_mayuscula(buf_op[2])=='X') {
				es_prefijo_ix_iy=221;
			}
			else if (letra_mayuscula(buf_op[2])=='Y') {
				es_prefijo_ix_iy=253;
			}			

			else return es_prefijo_ix_iy;

			//Es IX o IY. Leer desplazamiento. Guardamos valor desplazamiento en buffer aparte
			char buffer_numero[256];
			int i;

			//numero empieza en buf_opn[3]
			for (i=0;buf_op[i+3]!=0 && buf_op[i+3]!=')';i++) {
				buffer_numero[i]=buf_op[i+3];
			}

			buffer_numero[i]=0;
			//printf ("Parsear numero [%s] de desplazamiento ix/iy\n",buffer_numero);

			int valor_desplazamiento_nochar=parse_string_to_number(buffer_numero);
			*valor_desplazamiento=valor_desplazamiento_nochar;	

			//Y cambiar por (HL)
			strcpy(buf_op,"(HL)");	

			//printf ("Es IX/IY+d\n");	

		}
	}

	return es_prefijo_ix_iy;
}

int assemble_sustituir_ixiy_despl(int es_prefijo_ix_iy,char *buf_primer_op,char *buf_segundo_op,char *valor_desplazamiento)
{

	es_prefijo_ix_iy=assemble_sustituir_ixiy_despl_aux(es_prefijo_ix_iy,buf_primer_op,valor_desplazamiento);
	es_prefijo_ix_iy=assemble_sustituir_ixiy_despl_aux(es_prefijo_ix_iy,buf_segundo_op,valor_desplazamiento);

	return es_prefijo_ix_iy;

}

int assemble_sustituir_ixiy(int es_prefijo_ix_iy,char *buf_primer_op,char *buf_segundo_op)
{

	if (!strcasecmp(buf_primer_op,"IX")) {
		es_prefijo_ix_iy=221;
		strcpy(buf_primer_op,"HL");
	}	

	if (!strcasecmp(buf_segundo_op,"IX")) {
		es_prefijo_ix_iy=221;
		strcpy(buf_segundo_op,"HL");
	}	

	if (!strcasecmp(buf_primer_op,"IY")) {
		es_prefijo_ix_iy=253;
		strcpy(buf_primer_op,"HL");
	}	

	if (!strcasecmp(buf_segundo_op,"IY")) {
		es_prefijo_ix_iy=253;
		strcpy(buf_segundo_op,"HL");
	}	


	if (!strcasecmp(buf_primer_op,"IXL") || !strcasecmp(buf_primer_op,"IX_L")) {
		es_prefijo_ix_iy=221;
		strcpy(buf_primer_op,"L");
	}	

	if (!strcasecmp(buf_segundo_op,"IXL") || !strcasecmp(buf_segundo_op,"IX_L")) {
		es_prefijo_ix_iy=221;
		strcpy(buf_segundo_op,"L");
	}	

	if (!strcasecmp(buf_primer_op,"IXH") || !strcasecmp(buf_primer_op,"IX_H")) {
		es_prefijo_ix_iy=221;
		strcpy(buf_primer_op,"H");
	}	

	if (!strcasecmp(buf_segundo_op,"IXH") || !strcasecmp(buf_segundo_op,"IX_H")) {
		es_prefijo_ix_iy=221;
		strcpy(buf_segundo_op,"H");
	}	


	if (!strcasecmp(buf_primer_op,"IYL") || !strcasecmp(buf_primer_op,"IY_L")) {
		es_prefijo_ix_iy=253;
		strcpy(buf_primer_op,"L");
	}	

	if (!strcasecmp(buf_segundo_op,"IYL") || !strcasecmp(buf_segundo_op,"IY_L")) {
		es_prefijo_ix_iy=253;
		strcpy(buf_segundo_op,"L");
	}		


	if (!strcasecmp(buf_primer_op,"IYH") || !strcasecmp(buf_primer_op,"IY_H")) {
		es_prefijo_ix_iy=253;
		strcpy(buf_primer_op,"H");
	}	

	if (!strcasecmp(buf_segundo_op,"IYH") || !strcasecmp(buf_segundo_op,"IY_H")) {
		es_prefijo_ix_iy=253;
		strcpy(buf_segundo_op,"H");
	}	

	return es_prefijo_ix_iy;

}


int assemble_opcode_defm(char *texto,z80_byte *destino)
{
	//Asumimos que el primer parámetro empieza just despues de "defm " " 
	//posicion 6
	int indice=6;

	int longitud_total=0;

	while (texto[indice]!=0 && texto[indice]!='"') {
		*destino=texto[indice++];

		destino++;
		longitud_total++;

	}

	//printf ("longitud: %d\n",longitud_total);
	return longitud_total;
}


int assemble_opcode_defs(char *texto,z80_byte *destino)
{
	//Asumimos que el primer parámetro empieza just despues de "defs " 
	//posicion 5
	int indice=5;


	int longitud_total=parse_string_to_number(&texto[indice]);

	if (longitud_total<=0 || longitud_total>MAX_DESTINO_ENSAMBLADO) return 0; //Error

	int i;

	for (i=0;i<longitud_total;i++) {
		
		*destino=0; //meter ceros en destino
		destino++;

	}

	//printf ("longitud: %d\n",longitud_total);
	return longitud_total;
}

int assemble_opcode_defb_defw(int si_defw,char *texto,z80_byte *destino)
{
	//Asumimos que el primer parámetro empieza just despues de "defb " 
	//posicion 5
	int indice=5;
	char buffer_numero[255];

	int longitud_total=0;

	while (texto[indice]!=0) {
		int indice_buffer_numero=0;
		while (texto[indice]!=0 && texto[indice]!=',') {
			buffer_numero[indice_buffer_numero++]=texto[indice++];
		}
		buffer_numero[indice_buffer_numero]=0;
		
		int numero=parse_string_to_number(buffer_numero);
		//printf ("numero: [%s] [%d]\n",buffer_numero,numero);

		*destino=numero & 0xFF;
		destino++;
		longitud_total++;

		if (si_defw) {
			numero=numero>>8;
			*destino=numero & 0xFF;
			destino++;
			longitud_total++;			
		}

		if (texto[indice]!=0) indice++;
	}

	//printf ("longitud: %d\n",longitud_total);
	return longitud_total;
}

//Ensamblado de instruccion.
//Retorna longitud de opcode. 0 si error
//Lo ensambla en puntero indicado destino
//Maximo 255 bytes 
//direccion_destino es necesario para instrucciones tipo jr dis, djnz dis, etc
int assemble_opcode(int direccion_destino,char *texto,z80_byte *destino)
{

	if (CPU_IS_MOTOROLA || CPU_IS_SCMP) {
		debug_printf(VERBOSE_ERR,"Sorry, no assembler for that cpu yet");
		return 0;
	}

        //printf ("\n\nAssemble %s\n",texto);

        int longitud_instruccion=0;
        //Parsear opcode y parametros

	char buf_opcode[256];
	char buf_primer_op[256];
	char buf_segundo_op[256];

        asm_return_op_ops(texto,buf_opcode,buf_primer_op,buf_segundo_op);

        int parametros_entrada=0;
        if (buf_segundo_op[0]!=0) parametros_entrada++;
        if (buf_primer_op[0]!=0) parametros_entrada++;       

        //Aqui tenemos ya el numero de parametros

        //tipo de parametros de la instruccion. Tener en cuenta que algunos pueden ser n y nn a la vez, o rp y rp2 a la vez, etc
	//Si coincide con algun tipo de parametro conocido, y si no, se trata como numero


	//Casos especiales defb, defw, defm, defs
	if (parametros_entrada) {
		if (!strcasecmp("defb",buf_opcode)) {
			return assemble_opcode_defb_defw(0,texto,destino);
		}

		if (!strcasecmp("defw",buf_opcode)) {
			return assemble_opcode_defb_defw(1,texto,destino);
		}
 
		if (!strcasecmp("defm",buf_opcode)) {
			return assemble_opcode_defm(texto,destino);
		}		

		if (!strcasecmp("defs",buf_opcode)) {
			return assemble_opcode_defs(texto,destino);
		}				

	}


        //Recorrer array de ensamblado
        int i;

	int salir=0;
	int encontrado_indice=-1;

	//Si registro IX o IY, alterar prefijo. Tener cuidado con instrucciones de DD/FD + CB
	int es_prefijo_ix_iy=0;

	es_prefijo_ix_iy=assemble_sustituir_ixiy(es_prefijo_ix_iy,buf_primer_op,buf_segundo_op);

	//Si es IX+d	

	unsigned int valor_parametro_1=0; //Valor del parametro 1 cuando no es N ni NN
	unsigned int valor_parametro_2=0; //Valor del parametro 2 cuando no es N ni NN

	char desplazamiento_ixiy; //El valor +d o -d de (IX+d)
	es_prefijo_ix_iy=assemble_sustituir_ixiy_despl(es_prefijo_ix_iy,buf_primer_op,buf_segundo_op,&desplazamiento_ixiy);


        for (i=0;array_tabla_ensamblado[i].texto_opcode!=NULL && !salir;i++) {
                //printf ("%s\n",array_tabla_ensamblado[i].texto_opcode);
                if (!strcasecmp(buf_opcode,array_tabla_ensamblado[i].texto_opcode)) {
                        //printf ("Match opcode\n");
                        
	
			//Contar numero de parametros en array
			int parametros_array=0;

			if (array_tabla_ensamblado[i].tipo_parametro_1!=ASM_PARM_NONE) parametros_array++;
			if (array_tabla_ensamblado[i].tipo_parametro_2!=ASM_PARM_NONE) parametros_array++;

			if (parametros_array==parametros_entrada) {
				//comprobar los tipos de parametros que coincidan
				if (array_tabla_ensamblado[i].tipo_parametro_1!=ASM_PARM_NONE) {

					//Parametros de tipo constante
					if (array_tabla_ensamblado[i].tipo_parametro_1==ASM_PARM_CONST) {
						if (strcasecmp(buf_primer_op,array_tabla_ensamblado[i].const_parm1)) {
							//printf ("No coincide tipo const\n");
							continue;
						}
					}

					else {
		
						enum asm_tipo_parametro_entrada tipo_parametro_entrada_1;

						tipo_parametro_entrada_1=assemble_find_param_type(buf_primer_op,&valor_parametro_1);
				
						//Que coincidan los tipos
						if (!asm_check_parameter_in_table(tipo_parametro_entrada_1,array_tabla_ensamblado[i].tipo_parametro_1)) {
							//printf ("No coinciden los tipos en parm1\n");

							//Ver si se ha detectado como registro C pero es condicion CC
							if (!strcasecmp(buf_primer_op,"C") && array_tabla_ensamblado[i].tipo_parametro_1==ASM_PARM_CC) {
								//printf ("Si que coincide. Condicion C\n");
								valor_parametro_1=3; //cuarto operador en lista NZ,Z,NC,C,....
							}
							else continue;
						}
					}
				}


				if (array_tabla_ensamblado[i].tipo_parametro_2!=ASM_PARM_NONE) {
					//Parametros de tipo constante
                                        if (array_tabla_ensamblado[i].tipo_parametro_2==ASM_PARM_CONST) {
						if (strcasecmp(buf_segundo_op,array_tabla_ensamblado[i].const_parm2)) {
                                                        //printf ("No coincide tipo const\n");
                                                        continue;
                                                }
                                        }

					else {

						enum asm_tipo_parametro_entrada tipo_parametro_entrada_2;

        	                                tipo_parametro_entrada_2=assemble_find_param_type(buf_segundo_op,&valor_parametro_2);

                	                        //Que coincidan los tipos
                        	                if (!asm_check_parameter_in_table(tipo_parametro_entrada_2,array_tabla_ensamblado[i].tipo_parametro_2)) {
                                	                //printf ("No coinciden los tipos en parm2\n");
                                                	continue;
                                        	}
					}
				}

                        	//printf ("Match opcode and parameters type\n");

				longitud_instruccion=1;


				//Salir del bucle 
                        	salir=1;
				encontrado_indice=i;
			}
                }
        }

        //printf ("Indice: %d\n\n",encontrado_indice);

        if (encontrado_indice==-1) {
                //printf ("No match\n");
        }

	else {


		longitud_instruccion=1;

		//Ensamblarlo
		//Prefijo ix o iy
		if (es_prefijo_ix_iy) {
			*destino=es_prefijo_ix_iy;
			destino++;
			longitud_instruccion++;		
		}

		//Prefijo
		if (array_tabla_ensamblado[encontrado_indice].prefijo) {
			*destino=array_tabla_ensamblado[encontrado_indice].prefijo;
			destino++;
			longitud_instruccion++;		
		}

		//meter base opcode y mascara parametros
		z80_byte opcode_final=array_tabla_ensamblado[encontrado_indice].mascara_opcode;


		//Para parametro 1 y 2
		int paso_parametro;

		int tipo_parametro_tabla;
		int desplazamiento_mascara_tabla;
		char *buffer_operador;
		unsigned int valor_parametro;

		for (paso_parametro=1;paso_parametro<=2;paso_parametro++) {

		if (paso_parametro==1) {
			tipo_parametro_tabla=array_tabla_ensamblado[encontrado_indice].tipo_parametro_1;
			desplazamiento_mascara_tabla=array_tabla_ensamblado[encontrado_indice].desplazamiento_mascara_p1;
			buffer_operador=buf_primer_op;
			valor_parametro=valor_parametro_1;
		}

		else {
			tipo_parametro_tabla=array_tabla_ensamblado[encontrado_indice].tipo_parametro_2;
			desplazamiento_mascara_tabla=array_tabla_ensamblado[encontrado_indice].desplazamiento_mascara_p2;
			buffer_operador=buf_segundo_op;
			valor_parametro=valor_parametro_2;
		}		

		if (tipo_parametro_tabla!=ASM_PARM_NONE) {
			if (tipo_parametro_tabla==ASM_PARM_N || tipo_parametro_tabla==ASM_PARM_NN
                        || tipo_parametro_tabla==ASM_PARM_DIS || tipo_parametro_tabla==ASM_PARM_PARENTHESIS_N
                        || tipo_parametro_tabla==ASM_PARM_PARENTHESIS_NN
                        ) {
				//Parseamos valor
                                if (tipo_parametro_tabla==ASM_PARM_PARENTHESIS_N || tipo_parametro_tabla==ASM_PARM_PARENTHESIS_NN) {
                                        //Saltar el parentesis en (NN)
                                        valor_parametro=parse_string_to_number(&buffer_operador[1]);
                                }
                        
				else valor_parametro=parse_string_to_number(buffer_operador);

				//Si es N
				if (tipo_parametro_tabla==ASM_PARM_N || tipo_parametro_tabla==ASM_PARM_PARENTHESIS_N) {
					//En destino+1
					destino[1]=valor_parametro & 0xFF;
					longitud_instruccion++;
				}

				//Si es NN
				if (tipo_parametro_tabla==ASM_PARM_NN || tipo_parametro_tabla==ASM_PARM_PARENTHESIS_NN) {
					//En destino+1
					destino[1]=valor_parametro & 0xFF;
					destino[2]=(valor_parametro>>8) & 0xFF;
					longitud_instruccion+=2;
				}

                                //Si es DIS
				if (tipo_parametro_tabla==ASM_PARM_DIS) {
					//En destino+1
                                        //Calcular desplazamiento
                                        int dir_final=direccion_destino+2;
                                        int incremento=valor_parametro-dir_final;
                                        char incremento_8bit=incremento;


					destino[1]=incremento_8bit;
					longitud_instruccion++;
				}
			}

			else {
				//NO: qui se entra tambien cuando tipo es CONST. Pero como valor y desplazamiento valen 0, es un OR de 0
				if (tipo_parametro_tabla!=ASM_PARM_CONST) {


					//Si es RST
					if (tipo_parametro_tabla==ASM_PARM_RST) {
						valor_parametro=parse_string_to_number(buffer_operador);
						//printf ("Valor parametro 1 en RST: %d\n",valor_parametro_1);
						valor_parametro /=8;
						 
					}

					//Si es IM
					if (tipo_parametro_tabla==ASM_PARM_IM) {
						valor_parametro=parse_string_to_number(buffer_operador);
						//0,0,1,2,0,0,1,2 
						if (valor_parametro==1) valor_parametro=2;
						else if (valor_parametro==2) valor_parametro=3;
						//Usamos los valores oficiales 0,0,1,2 (el "segundo" 0 corresponderia a im0 no documentado)
						 
					}			

					//Si es BIT
					if (tipo_parametro_tabla==ASM_PARM_BIT) {
						valor_parametro=parse_string_to_number(buffer_operador);
					}									

					//En caso de IX+d
					int es_ddfd_cb=0;
					if (es_prefijo_ix_iy && !strcasecmp(buffer_operador,"(HL)")) {
						//En caso de IX/IY y prefijo CB. Desplazamiento va antes y luego va instruccion
						if (array_tabla_ensamblado[encontrado_indice].prefijo==203) {
							//printf ("Opcode con DD/FD+CB\n");
							valor_parametro <<= desplazamiento_mascara_tabla;
							opcode_final |= valor_parametro;							
							//printf ("opcode_final: %02X %02X\n",desplazamiento_ixiy,opcode_final);
							destino[1]=opcode_final;
							//destino[2]=opcode_final;

							opcode_final=desplazamiento_ixiy; //Acabara escribiendo esto mas abajo
							longitud_instruccion++;

							es_ddfd_cb=1; //Para que esto mas abajo no haga OR con nada

						}

						else {
							destino[1]=desplazamiento_ixiy;
							longitud_instruccion++;
							valor_parametro=6; //b,c,d,e,h,l,(hl),a ->numero de orden 6 para (hl)
						}
					}

					if (!es_ddfd_cb) {
                        //printf ("OR con valor %d desplazado %d\n",valor_parametro_1,array_tabla_ensamblado[encontrado_indice].desplazamiento_mascara_p1);
						valor_parametro <<= desplazamiento_mascara_tabla;
						opcode_final |= valor_parametro;
					}
				}

				
			}
		}

		} //for paso parametro

		


		*destino=opcode_final;
		//printf ("opcode final: %d\n",opcode_final);


	}
        return longitud_instruccion;

}
