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
#include "expression_parser.h"
#include "utils.h"
#include "debug.h"
#include "operaciones.h"
#include "screen.h"
#include "mem128.h"
#include "scmp.h"
#include "m68k.h"
#include "prism.h"
#include "tbblue.h"
#include "hilow_datadrive.h"
#include "pd765.h"

/*


-Parser breakpoints
*función compilar ascii a tokens
*función inversa
*función evaluar desde tokens
*espacio en bytes de cada token fijo



Parser con tokens: token que identifique un valor, tendrá en su posición del array (estructura realmente) el valor entero almacenado directamente como int

Así cada elemento token puede ser una estructura tipo:

z80_byte tipo (variable, registro, operador, número, fin, etc)
z80_byte indice (si es registro , podría ser: 0 para registro A, 1 para registro B, etc)
z80_byte formato (para números, decir si se muestra como hexadecimal, binario, ascii etc)
z80_byte signo (signo del valor almacenado)
int  valor (guarda el valor absoluto en variable int)

Esto serán 11 bytes por cada token (en máquina de 64 bits)



Evaluar expresión:

Buscar si hay operador logico and, or, xor. Si lo hay, separar sub condiciones

Evaluar condiciones: buscar si hay comparadores =, <,>, <>. Si lo hay, evaluar cada uno de los valores
Cada condición genera 0 o 1

Evaluar valores: por orden, evaluar valores, variables  y posibles operadores de cálculo: +,-,*,/. & (and), | (or), ^ (xor)


NO existe el uso de paréntesis. 

Aquí lo difícil es el parser que convierte el ascii en tokens

Como interpretar un -/+ inicial? Tiene que ser un valor como tal y no una suma o resta. Pero no es solo inicial, sino también después de operadores, por ejemplo:
A>-3
-3 es un valor

Pero:
A>10-3
-3 es operador de resta y valor 3

Para saber si es valor con signo, hay que ver lo que hay antes:
-comparadores <>= etc
-inicio de string
-operador lógico and,or,xor
-operador de cálculo &|^* etc

En resto de casos, se trata de suma o resta y valor absoluto (o sea, cuando lo de la izquierda es un número o variable)


Al final esto dará un valor 0 o diferente de 0. A efectos de disparar breakpoint, este lo hará cuando el valor sea diferente de 0

 */


//Usado en la gestión de paréntesis:
token_parser_textos_indices tpti_parentesis[]={

	{TPI_P_ABRIR,"("},
	{TPI_P_CERRAR,")"},

    {TPI_FIN,""}
};

//funciones. Siempre acabarlas en (
token_parser_textos_indices tpti_funciones[]={

	{TPI_F_PEEK,"PEEK("},
	{TPI_F_PEEKW,"PEEKW("},
    {TPI_F_FPEEK,"FPEEK("},
	{TPI_F_IN,"IN("},
	{TPI_F_NOT,"NOT("},

	{TPI_F_ABS,"ABS("},
	{TPI_F_BYTE,"BYTE("},
	{TPI_F_WORD,"WORD("},
    {TPI_F_OPMWA,"OPMWA("},
    {TPI_F_OPMRA,"OPMRA("},
    {TPI_F_OPMWV,"OPMWV("},
    {TPI_F_OPMRV,"OPMRV("},    


    {TPI_FIN,""}
};

//Usado en la conversion de texto a tokens, variables:
token_parser_textos_indices tpti_variables[]={
    {TPI_V_MRA,"MRA"},
	{TPI_V_MRV,"MRV"},

	{TPI_V_MWV,"MWV"},
	{TPI_V_MWA,"MWA"},

	//Puertos
	{TPI_V_PRV,"PRV"},
	{TPI_V_PRA,"PRA"},

	{TPI_V_PWV,"PWV"},
	{TPI_V_PWA,"PWA"},

	{TPI_V_TSTATES,"TSTATES"},
	{TPI_V_TSTATESL,"TSTATESL"},
	{TPI_V_TSTATESP,"TSTATESP"},

	{TPI_V_SCANLINE,"SCANLINE"},


	{TPI_V_IFF1,"IFF1"},
	{TPI_V_IFF2,"IFF2"},

	{TPI_V_OUTFIRED,"OUTFIRED"},
	{TPI_V_INFIRED,"INFIRED"},
	{TPI_V_INTFIRED,"INTFIRED"},

	{TPI_V_ENTERROM,"ENTERROM"},
	{TPI_V_EXITROM,"EXITROM"},


    {TPI_V_SEG0,"SEG0"},
    {TPI_V_SEG1,"SEG1"},
    {TPI_V_SEG2,"SEG2"},
    {TPI_V_SEG3,"SEG3"},
    {TPI_V_SEG4,"SEG4"},
    {TPI_V_SEG5,"SEG5"},
    {TPI_V_SEG6,"SEG6"},
    {TPI_V_SEG7,"SEG7"},

    {TPI_V_HILOWMAPPED,"HILOWMAPPED"},

    {TPI_V_OPCODE1,"OPCODE1"},
    {TPI_V_OPCODE2,"OPCODE2"},
    {TPI_V_OPCODE3,"OPCODE3"},
    {TPI_V_OPCODE4,"OPCODE4"},


    {TPI_V_RAM,"RAM"},
    {TPI_V_ROM,"ROM"},

    {TPI_V_PD765_PCN,"PD765PCN"},
    	

    {TPI_FIN,""}
};

//Usado en la conversion de texto a tokens, registros:
token_parser_textos_indices tpti_registros[]={

	{TPI_R_PC,"PC"},
    {TPI_R_SP,"SP"},
    {TPI_R_USP,"USP"},
    {TPI_R_IX,"IX"},
    {TPI_R_IY,"IY"},	

	{TPI_R_A,"A"},
	{TPI_R_B,"B"},
	{TPI_R_C,"C"},
	{TPI_R_D,"D"},
	{TPI_R_E,"E"},
	{TPI_R_F,"F"},
	{TPI_R_H,"H"},
	{TPI_R_L,"L"},
	{TPI_R_I,"I"},
	{TPI_R_R,"R"},

        {TPI_R_AF,"AF"},
        {TPI_R_BC,"BC"},
        {TPI_R_DE,"DE"},
        {TPI_R_HL,"HL"},    

	{TPI_R_A_SHADOW,"A'"},
	{TPI_R_B_SHADOW,"B'"},
	{TPI_R_C_SHADOW,"C'"},
	{TPI_R_D_SHADOW,"D'"},
	{TPI_R_E_SHADOW,"E'"},
	{TPI_R_F_SHADOW,"F'"},
	{TPI_R_H_SHADOW,"H'"},
	{TPI_R_L_SHADOW,"L'"},


        {TPI_R_AF_SHADOW,"AF'"},
        {TPI_R_BC_SHADOW,"BC'"},
        {TPI_R_DE_SHADOW,"DE'"},
        {TPI_R_HL_SHADOW,"HL'"},   

        {TPI_R_FS,"FS"},
        {TPI_R_FZ,"FZ"},
        {TPI_R_FP,"FP"},
        {TPI_R_FV,"FV"},    
        {TPI_R_FH,"FH"},
        {TPI_R_FN,"FN"},
        {TPI_R_FC,"FC"},  




	//De motorola
        {TPI_R_D0,"D0"},     
        {TPI_R_D1,"D1"}, 
        {TPI_R_D2,"D2"}, 
        {TPI_R_D3,"D3"}, 
        {TPI_R_D4,"D4"}, 
        {TPI_R_D5,"D5"}, 
        {TPI_R_D6,"D6"}, 
        {TPI_R_D7,"D7"}, 

        {TPI_R_A0,"A0"},     
        {TPI_R_A1,"A1"}, 
        {TPI_R_A2,"A2"}, 
        {TPI_R_A3,"A3"}, 
        {TPI_R_A4,"A4"}, 
        {TPI_R_A5,"A5"}, 
        {TPI_R_A6,"A6"}, 
        {TPI_R_A7,"A7"},     

    //De SCMP
	{TPI_R_AC,"AC"},
    {TPI_R_ER,"ER"},
    {TPI_R_SR,"SR"},
    {TPI_R_P1,"P1"},
    {TPI_R_P2,"P2"},
    {TPI_R_P3,"P3"},  

    //De Tbblue
    {TPI_R_COPPERPC,"COPPERPC"},

             


    {TPI_FIN,""}
};

//Usado en la conversion de texto a tokens, operador logico:
token_parser_textos_indices tpti_operador_logico[]={

	//de tipo operador logico
	{TPI_OL_AND,"AND"},
	{TPI_OL_OR,"OR"},
	{TPI_OL_XOR,"XOR"},

    {TPI_FIN,""}
};


//Usado en la conversion de texto a tokens, operador condicional:
token_parser_textos_indices tpti_operador_condicional[]={

	{TPI_OC_IGUAL,"="},
	{TPI_OC_MENOR,"<"},
	{TPI_OC_MAYOR,">"},
	{TPI_OC_DIFERENTE,"<>"},

	{TPI_OC_MENOR_IGUAL,"<="},
	{TPI_OC_MAYOR_IGUAL,">="},    

    {TPI_FIN,""}
};


//Usado en la conversion de texto a tokens, operador calculo:
token_parser_textos_indices tpti_operador_calculo[]={

	{TPI_OC_SUMA,"+"},
	{TPI_OC_RESTA,"-"},
	{TPI_OC_MULTIPLICACION,"*"},
	{TPI_OC_DIVISION,"/"},
	{TPI_OC_AND,"&"},	
	{TPI_OC_OR,"|"},
	{TPI_OC_XOR,"^"},

    {TPI_FIN,""}
};    

//Dice si caracter es digito 0...9
int exp_par_is_digit(char c)
{
    if (c>='0' && c<='9') return 1;
    else return 0;
}

//Dice si caracter es digito hexadecimal 0...9...F
//modifica esletra si es letra a....f
int exp_par_is_hexadigit(char c,int *esletra)
{

    if (exp_par_is_digit(c)) {
        return 1;
    }

    if (letra_minuscula(c)>='a' && letra_minuscula(c)<='f') {
        *esletra=1;
        return 1;
    }

    else return 0;
}



//Dice si caracter es letra
int exp_par_is_letter(char c)
{
    if (letra_minuscula(c)>='a' && letra_minuscula(c)<='z') return 1;
    else return 0;
}

//Dice si una expresion es numérica, y dice donde acaba (caracter siguiente)
//Retorna 1 si lo es. 0 si no. -1 si hay error parseando
int exp_par_is_number(char *texto,int *final)
{
    /*
    Expresiones admitidas:
    numero decimal: 0123456789
    numero hexadecimal: 0123456789ABCDEFH
    numero binario: 01%
    numero ascii "A" o 'a'
     */

    //Primero probar ascii
    if (texto[0]=='"' || texto[0]=='\'') {
        //si final texto, error
        if (texto[1]==0) return -1;
        if (texto[2]=='"' || texto[2]=='\'') {
            *final=3;
            return 1;
        }
        else return -1; //no hay comillas de cierre
    }

    //Tendra que empezar con numero o hexa, si no, error
    int esletra=0;
    if (!exp_par_is_hexadigit(texto[0],&esletra)) {
        //printf ("numero no empieza con hexadigito\n");
        return -1;
    }

    //Parseamos hasta encontrar sufijo, si lo hay
    int i;
    for (i=0;texto[i];i++) {
        if (letra_minuscula(texto[i])=='h' || texto[i]=='%') {
            *final=i+1;
            return 1;
        }
        if (!exp_par_is_hexadigit(texto[i],&esletra)) break;

    }

    //final pero sin letra de sufijo

    //esletra?
    if (esletra) {
        //printf ("Habia letra hexa pero sin sufijo hexa. no es numero\n");
        return 0;
    }

    *final=i;
    return 1;

}

//Dice si el texto es uno de los contenidos en array de token_parser_textos_indices
//Retorna el indice si esta, -1 si no
int exp_par_is_token_parser_textos_indices(char *texto,token_parser_textos_indices *textos_indices)
{
    //printf ("llamando a exp_par_is_token_parser_textos_indices con texto [%s]\n",texto);

    int i=0;

    while (textos_indices->indice!=TPI_FIN) {
        //printf ("i %d\n",i);
        if (!strcasecmp(texto,textos_indices->texto)) {
            //printf ("es texto %s indice %d\n",textos_indices->texto,textos_indices->indice);
            return textos_indices->indice;
        }
        i++;
        textos_indices++;
    }

    return -1;
}

//Retorna 1 si el caracter es de abrir parentesis : ({[
int exp_par_is_parentesis_abrir(char caracter)
{
    if ( caracter=='{' || caracter=='(' || caracter=='[') return 1;
    else return 0;
}

//Retorna 1 si el caracter es de cerrar parentesis : )}]
int exp_par_is_parentesis_cerrar(char caracter)
{
    if ( caracter=='}' || caracter==')' || caracter==']') return 1;
    else return 0;
}

//Dice si una expresion es funcion, y dice donde acaba (caracter apuntando a cierre parentesis) y indice funcion
//Retorna 1 si lo es. 0 si no. -1 si hay error parseando
int exp_par_is_funcion(char *texto,int *final,enum token_parser_indice *indice_final)
{
    //Buscar hasta final letras
    char buffer_texto[MAX_PARSER_TEXTOS_INDICE_LENGTH];

    //int unparentesis=0;

    int i=0;
    while (*texto && (exp_par_is_letter(*texto) /* || ( (*texto)=='(' && unparentesis==0)*/ ) && i<MAX_PARSER_TEXTOS_INDICE_LENGTH)  {
        buffer_texto[i]=*texto;

        //if ( (*texto)=='(') unparentesis=1; //solo leer hasta el primer parentesis

        i++;
        texto++;
    }

    //Si no acaba la palabra con parentesis abrir
    //if ((*texto)!='(') return 0;
    if (!exp_par_is_parentesis_abrir(*texto)) return 0;

    if (i==MAX_PARSER_TEXTOS_INDICE_LENGTH) {
        //Final de buffer. error
        return -1;
    }

    //Meter parentesis
    buffer_texto[i++]='(';

    buffer_texto[i]=0;

    //printf ("probando tpti_funciones con [%s]\n",buffer_texto);
    int indice=exp_par_is_token_parser_textos_indices(buffer_texto,tpti_funciones);
    if (indice>=0) {
        //printf ("[%s] es funcion\n",buffer_texto);
        *final=strlen(buffer_texto)-1; //quitarle el parentesis 
        *indice_final=indice;
        return 1;
    }

 
    return 0;

}


//Dice si una expresion es operador, y dice donde acaba (caracter siguiente)
//Retorna 1 si lo es. 0 si no. -1 si hay error parseando
int exp_par_is_operador(char *texto,int *final)
{
    //cadena vacia
    if (texto[0]==0) return 0;

    //Considerar letras y tpti_operador_condicional y tpti_operador_calculo
    char primer_caracter;

    char buffer_texto[3];

    
    primer_caracter=*texto;
    buffer_texto[0]=primer_caracter;
    buffer_texto[1]=0;

    //considerar condicional <>
    if (texto[0]=='<' && texto[1]=='>') {
        buffer_texto[0]='<';
        buffer_texto[1]='>';
        buffer_texto[2]=0;        
    }

    //considerar condicionales <= >=
    if (
        (texto[0]=='<' || texto[0]=='>') &&
        texto[1]=='='
    ) {
        buffer_texto[0]=texto[0];
        buffer_texto[1]='=';
        buffer_texto[2]=0;        
    }    

    //tpti_operador_condicional
    if (exp_par_is_token_parser_textos_indices(buffer_texto,tpti_operador_condicional)>=0) {
        *final=strlen(buffer_texto);
        return 1;
    }

    //tpti_operador_calculo
    if (exp_par_is_token_parser_textos_indices(buffer_texto,tpti_operador_calculo)>=0) {
        *final=strlen(buffer_texto);
        return 1;
    }

    //letras hasta que no sean letras
    int i=0;
    while (exp_par_is_letter(texto[i])) {
        i++;
    }

    *final=i;
    return 1;

}

//Considerar caracteres auxiliares para registros: ' , (), pero no ciua
int exp_par_char_is_reg_aux(char c)
{
    //if (c=='\'' || c=='(' || c==')') return 1;
    if (c=='\'') return 1;  //no considerar () como parte de un registro
    else return 0;
}

//Considerar caracteres auxiliares para variables: 12,  como iff1 y iff2, seg0 etc, d0, d1 etc, pero no al principio
int exp_par_char_is_reg_aux_more(char c,int i)
{
    if (i==0) return 0;

    if (exp_par_is_digit(c)) return 1;
    else return 0;
}

//Dice si una expresion es variable/registro, y dice donde acaba (caracter siguiente)
//Retorna 1 si lo es. 0 si no. -1 si hay error parseando
int exp_par_is_var_reg(char *texto,int *final)
{
   

    //Buscar hasta final letras
    char buffer_texto[MAX_PARSER_TEXTOS_INDICE_LENGTH];

    int i=0;
    while (*texto && (exp_par_is_letter(*texto) || exp_par_char_is_reg_aux(*texto) || exp_par_char_is_reg_aux_more(*texto,i) ) && i<MAX_PARSER_TEXTOS_INDICE_LENGTH)  {
        //consideramos que pueden acabar los registros con ' ()

        buffer_texto[i]=*texto;
        i++;
        texto++;
    }

    if (i==MAX_PARSER_TEXTOS_INDICE_LENGTH) {
        //Final de buffer. error
        return -1;
    }

    buffer_texto[i]=0;

    //printf ("probando tpti_variables\n");
    if (exp_par_is_token_parser_textos_indices(buffer_texto,tpti_variables)>=0) {
        //printf ("es variable\n");
        *final=strlen(buffer_texto);
        return 1;
    }

    if (exp_par_is_token_parser_textos_indices(buffer_texto,tpti_registros)>=0) {
        //printf ("es registro\n");
        *final=strlen(buffer_texto);
        return 1;
    }

    return 0;

}


//Parsear texto como variable o registro
//Devuelve 0 si no existe
int exp_par_parse_var_reg(char *texto,enum token_parser_tipo *tipo,enum token_parser_indice *indice_final)
{
   
    int indice;

    indice=exp_par_is_token_parser_textos_indices(texto,tpti_variables);
    if (indice>=0) {
        *tipo=TPT_VARIABLE;
        *indice_final=indice;
        return 1;
    }

    indice=exp_par_is_token_parser_textos_indices(texto,tpti_registros);
    if (indice>=0) {
        *tipo=TPT_REGISTRO;
        *indice_final=indice;
        return 1;
    }

    return 0;

}


//Parsear texto como operador
//Devuelve 0 si no existe
int exp_par_parse_operador(char *texto,enum token_parser_tipo *tipo,enum token_parser_indice *indice_final)
{
    int indice;

 
    indice=exp_par_is_token_parser_textos_indices(texto,tpti_operador_condicional);
    if (indice>=0) {
        *tipo=TPT_OPERADOR_CONDICIONAL;
        *indice_final=indice;
        return 1;
    }

    indice=exp_par_is_token_parser_textos_indices(texto,tpti_operador_calculo);
    if (indice>=0) {
        *tipo=TPT_OPERADOR_CALCULO;
        *indice_final=indice;
        return 1;
    }


    indice=exp_par_is_token_parser_textos_indices(texto,tpti_operador_logico);
    if (indice>=0) {
        *tipo=TPT_OPERADOR_LOGICO;
        *indice_final=indice;
        return 1;
    }

    return 0;

}


//Copia la cadena origen en destino, con longitud indicada. Agrega 0 al final
void exp_par_copy_string(char *origen,char *destino, int longitud)
{
    int i;

    for (i=0;i<longitud;i++) {
        destino[i]=origen[i];
    }

    destino[i]=0;
}




//Convierte expression de entrada en tokens. Devuelve <0 si error
int exp_par_exp_to_tokens(char *expression,token_parser *tokens)
{
    /*
    Expresion de entrada:

    NUMERO/VARIABLE/REGISTRO OPERADOR NUMERO/VARIABLE/REGISTRO OPERADOR .... NUMERO/VARIABLE/REGISTRO


    	TPT_NUMERO,
	TPT_VARIABLE, //mra, mrw, etc
	TPT_REGISTRO, //a, bc, de, etc
	TPT_OPERADOR_LOGICO, //and, or, xor
	TPT_OPERADOR_CONDICIONAL, //=, <,>, <>,
	TPT_OPERADOR_CALCULO //+,-,*,/. & (and), | (or), ^ (xor)

     */
    //Empezamos a leer texto y dividir secciones de NUMERO/VARIABLE/REGISTRO y OPERADOR

    int indice_token=0;

    enum token_parser_indice indice_funcion;
    int final;

    while (*expression) {
        //Si hay espacio, saltar
        if ( (*expression)==' ') expression++;
        else if ( exp_par_is_parentesis_abrir(*expression) ) { //si hay parentesis abrir o cerrar,saltar
                //printf ("abrir parentesis\n");
                tokens[indice_token].tipo=TPT_PARENTESIS;
                tokens[indice_token].indice=TPI_P_ABRIR;      
                expression++;
                indice_token++;          
        }
        else if ( exp_par_is_parentesis_cerrar(*expression) ) { //si hay parentesis abrir o cerrar,saltar
                //printf ("cerrar parentesis\n");
                tokens[indice_token].tipo=TPT_PARENTESIS;
                tokens[indice_token].indice=TPI_P_CERRAR;      
                expression++;
                indice_token++;          
        }      

        else if (exp_par_is_funcion(expression,&final,&indice_funcion)) {
            //meter funcion
            tokens[indice_token].tipo=TPT_FUNCION;
            tokens[indice_token].indice=indice_funcion;      

            indice_token++;    

            //Apuntamos al parentesis de abrir
            expression=&expression[final];                 

        }

        else {
            //Obtener numero
            
            int resultado;

            //Suponer primero que son variables/registros

            //Consideramos variable/registro
            //printf ("parsing variable/register from %s\n",expression);
            resultado=exp_par_is_var_reg(expression,&final);
            //printf ("resultado exp_par_is_var_reg %d\n",resultado);
            if (resultado>0) {
                //printf ("final index: %d\n",final);

                //Parsear expresion

                //Metemos en buffer temporal
                char buffer_temp[MAX_PARSER_TEXTOS_INDICE_LENGTH];
                exp_par_copy_string(expression,buffer_temp,final);

                enum token_parser_tipo tipo;
                enum token_parser_indice indice;

                if (!exp_par_parse_var_reg(buffer_temp,&tipo,&indice)) {
                    //printf ("return error exp_par_parse_var_reg\n");
                    return -1;
                }

                tokens[indice_token].tipo=tipo;
                tokens[indice_token].indice=indice;

           
            }

            else {

                //Consideramos numero

                //printf ("parsing number from %s\n",expression);
                int signo_valor=+1;

                //Ver si empieza por - o +
                if ( (*expression)=='-') {
                    signo_valor=-1;
                    expression++;
                }

                if ( (*expression)=='+') {
                    signo_valor=+1; //aunque ya viene asi por defecto antes, pero para que quede mas claro
                    expression++;
                }                

                resultado=exp_par_is_number(expression,&final);
            
                if (resultado<=0) {
                    //printf ("return number with error (evaluated [%s]\n",expression);
                    return -1; //error
                }

            
                //printf ("final index: %d\n",final);
                //Es un numero
                //printf ("end number: %c\n",expression[final]);


                //Metemos en buffer temporal
                char buffer_temp[MAX_PARSER_TEXTOS_INDICE_LENGTH];
                exp_par_copy_string(expression,buffer_temp,final);                

                //Parseamos numero
                enum token_parser_formato formato;
                int valor=parse_string_to_number_get_type(buffer_temp,&formato);

                //Meter valor en token
                tokens[indice_token].tipo=TPT_NUMERO;
                tokens[indice_token].formato=formato;
                tokens[indice_token].signo=signo_valor;

                //Meter valor
                tokens[indice_token].valor=valor;

            }

            

            //Siguiente expresion
            indice_token++;
            expression=&expression[final];

            //saltar espacios
            while ( (*expression)==' ') expression++;

            while ( exp_par_is_parentesis_abrir(*expression) ) { //si hay parentesis abrir o cerrar,saltar
                //printf ("abrir parentesis en bucle\n");
                tokens[indice_token].tipo=TPT_PARENTESIS;
                tokens[indice_token].indice=TPI_P_ABRIR;      
                expression++;
                indice_token++;          
            }
            while ( exp_par_is_parentesis_cerrar(*expression) ) { //si hay parentesis abrir o cerrar,saltar
                //printf ("cerrar parentesis en bucle\n");
                tokens[indice_token].tipo=TPT_PARENTESIS;
                tokens[indice_token].indice=TPI_P_CERRAR;      
                expression++;
                indice_token++;          
            }    

            //saltar espacios
            while ( (*expression)==' ') expression++;                          

            //Si no final, 
            if ( (*expression)!=0) {
                //Calcular operador
                //printf ("parsing operador from %s\n",expression);
                resultado=exp_par_is_operador(expression,&final);
                if (resultado==-1) {
                    //printf ("return operador with error\n");
                    return -1; //error
                }

                //printf ("final index: %d\n",final);

                //printf ("end number: %c\n",expression[final]);

                //Parsear expresion

                //Metemos en buffer temporal
                char buffer_temp[MAX_PARSER_TEXTOS_INDICE_LENGTH];
                exp_par_copy_string(expression,buffer_temp,final);

                enum token_parser_tipo tipo;
                enum token_parser_indice indice;

                if (!exp_par_parse_operador(buffer_temp,&tipo,&indice)) {
                    //printf ("return error exp_par_parse_operador [%s]\n",buffer_temp);
                    return -1;
                }

                tokens[indice_token].tipo=tipo;
                tokens[indice_token].indice=indice;                
       
            
                //Siguiente expresion
                indice_token++;
                expression=&expression[final];


            }

        }
    };

    //Poner final de token
    tokens[indice_token].tipo=TPT_FIN;

    return 0;

}





/*
Funciones de paso de tokens a string
*/

//Convierte tokens en string
void exp_par_tokens_to_exp(token_parser *tokens,char *expression,int maximo)
{
	int i=0;
    int dest_string=0;
    char buffer_temporal_binario[34]; //32 bits, prefijo y 0 del final

	while (tokens[i].tipo!=TPT_FIN && maximo) {
        int esnumero=0;
        int espacio=0;
        int esfuncion=0;
        enum token_parser_tipo tipo=tokens[i].tipo;

        token_parser_textos_indices *indice_a_tabla;

        switch (tipo) {
            case TPT_NUMERO:
                esnumero=1;
            break;

            case TPT_PARENTESIS:
                indice_a_tabla=tpti_parentesis;
            break;

	        case TPT_VARIABLE: //mra,mrw, etc
                indice_a_tabla=tpti_variables;
            break;

	        case TPT_REGISTRO: //a, bc, de, etc
                indice_a_tabla=tpti_registros;
            break;


	        case TPT_OPERADOR_LOGICO:  //and, or, xor
                indice_a_tabla=tpti_operador_logico;
                espacio=1;
            break;

            case TPT_OPERADOR_CONDICIONAL:  //=, <,>, <>,
                indice_a_tabla=tpti_operador_condicional;
            break;

            case TPT_OPERADOR_CALCULO: //+,-,*,/. & (and), | (or), ^ (xor)
                indice_a_tabla=tpti_operador_calculo;
            break;

            case TPT_FUNCION: 
                indice_a_tabla=tpti_funciones;
                esfuncion=1;
            break;            

            case TPT_FIN:
                //esto se gestiona desde el while y por tanto no se llega nunca aqui. Lo pongo para que no se queje el compilador
            break;

        }

        if (esnumero) {

           int valor_final=tokens[i].valor;

            //aplicar signo
            valor_final *=tokens[i].signo;        
            
           enum token_parser_formato formato;
           formato=tokens[i].formato;

           switch (formato) {

               case TPF_HEXADECIMAL:
                sprintf(&expression[dest_string],"%XH",valor_final); 
               break;   

               case TPF_DECIMAL:
               sprintf(&expression[dest_string],"%d",valor_final); 
               break;

               case TPF_ASCII:
               //controlar rango, por si acaso
               if (valor_final>=32 && valor_final<=126) sprintf(&expression[dest_string],"\'%c\'",valor_final); 
               else sprintf(&expression[dest_string],"%d",valor_final); 
               break;     

               case TPF_BINARIO:  
                util_ascii_to_binary(valor_final,buffer_temporal_binario,33);
                strcpy(&expression[dest_string],buffer_temporal_binario);
               break;      

               default:
               //cualquier otra cosa, decimal (aunque aqui no deberia llegar nunca)
               sprintf(&expression[dest_string],"%d",valor_final); 
               break;

               	//TPF_DECIMAL,
	            //TPF_HEXADECIMAL,
	            //TPF_BINARIO,
	            //TPF_ASCII
               
           }
           
        }

        else {
            enum token_parser_indice indice=tokens[i].indice; //indice a buscar

            //buscar texto que corresponda con ese indice
            int j;
            for (j=0;indice_a_tabla[j].indice!=TPI_FIN;j++) {
                if (indice_a_tabla[j].indice==indice) break;
            }

            //Meter en buffer el texto
            char buf_tok_str[MAX_PARSER_TEXTOS_INDICE_LENGTH];
            
            strcpy(buf_tok_str,indice_a_tabla[j].texto);

            //Si es funcion, eliminar ultimo caracter que sera "("
            if (esfuncion) {
                int l=strlen(buf_tok_str);
                if (l) buf_tok_str[l-1]=0;
            }            

            if (!espacio) sprintf(&expression[dest_string],"%s",buf_tok_str);
            else sprintf(&expression[dest_string]," %s ",buf_tok_str);

            //printf ("***MRA=%d \n",TPI_V_MRA);
        }

        int longitud=strlen(&expression[dest_string]);
        dest_string +=longitud;    

            
		i++;
        maximo--;
	}

    //Esto solo tiene sentido si cadena de entrada era TPT_FIN tal cual,
    //por que si no, cualquiera de los sprintf de antes habran metido el 0 final
    expression[dest_string]=0;

}


//Retorna valor de opcode segun si tipo opcode1,2,3 o 4
int exp_par_opcode(int longitud)
{

	int resultado=0;
    int i;

	//printf ("longitud en bytes de la condicion opcode: %d\n",longitud);
	unsigned int copia_reg_pc=get_pc_register();

	for (i=0;i<longitud;i++) {
		z80_byte valor_leido=peek_byte_z80_moto(copia_reg_pc);
		resultado=resultado<<8;
		resultado |=valor_leido;

		copia_reg_pc++;
	}



	return resultado;

}

//calcula valor de token, si es numero, variable o registro
int exp_par_calculate_numvarreg(token_parser *token)
{

    enum token_parser_tipo tipo=token->tipo;
    enum token_parser_indice indice=token->indice;

        int resultado=0; //asumimos cero

        switch (tipo) {
            case TPT_NUMERO:
                resultado=(token->valor) * (token->signo);
            break;

	        case TPT_VARIABLE: //mra,mrw, etc
                switch (indice) {
                    

//Variables de la MMU
	//Memoria
    case TPI_V_MRA: return debug_mmu_mra; break;
	case TPI_V_MRV: return debug_mmu_mrv; break;

	case TPI_V_MWV: return debug_mmu_mwv; break;
	case TPI_V_MWA: return debug_mmu_mwa; break;

	//Puertos
	case TPI_V_PRV: return debug_mmu_prv; break;
	case TPI_V_PRA: return debug_mmu_pra; break;

	case TPI_V_PWV: return debug_mmu_pwv; break;
	case TPI_V_PWA: return debug_mmu_pwa; break;

	//T-estados
	case TPI_V_TSTATES: return t_estados; break;
	case TPI_V_TSTATESL: return t_estados % screen_testados_linea; break;
	case TPI_V_TSTATESP: return debug_t_estados_parcial; break;

	case TPI_V_SCANLINE: return t_scanline_draw; break;



	//interrupciones
	case TPI_V_IFF1: return iff1.v; break;
	case TPI_V_IFF2: return iff2.v; break;

	//se acaba de lanzar un out
	case TPI_V_OUTFIRED: return debug_fired_out; break;
	//se acaba de lanzar un in
	case TPI_V_INFIRED: return debug_fired_in; break;
	//se acaba de generar una interrupcion
	case TPI_V_INTFIRED: return debug_fired_interrupt; break;	


    case TPI_V_ENTERROM:

	
		if (debug_enterrom==1) {
			debug_enterrom++;
			return 1;
		}
		return 0;
	

    break;

    case TPI_V_EXITROM:

	
		if (debug_exitrom==1) {
			debug_exitrom++;
			return 1;
		}
		return 0;
	

    break;
    


    case TPI_V_OPCODE1:
        return exp_par_opcode(1);
    break;

    case TPI_V_OPCODE2:
        return exp_par_opcode(2);
    break;

    case TPI_V_OPCODE3:
        return exp_par_opcode(3);
    break;

    case TPI_V_OPCODE4:
        return exp_par_opcode(4);
    break;


    //bancos memoria Z88
    case TPI_V_SEG0:
        if (MACHINE_IS_Z88) return blink_mapped_memory_banks[0];
        if (MACHINE_IS_TBBLUE) return debug_paginas_memoria_mapeadas[0];
    break;

    case TPI_V_SEG1:
        if (MACHINE_IS_Z88) return blink_mapped_memory_banks[1];
        if (MACHINE_IS_TBBLUE) return debug_paginas_memoria_mapeadas[1];
    break;

    case TPI_V_SEG2:
        if (MACHINE_IS_Z88) return blink_mapped_memory_banks[2];
        if (MACHINE_IS_TBBLUE) return debug_paginas_memoria_mapeadas[2];
    break;

    case TPI_V_SEG3:
        if (MACHINE_IS_Z88) return blink_mapped_memory_banks[3];
        if (MACHINE_IS_TBBLUE) return debug_paginas_memoria_mapeadas[3];
    break;          

    case TPI_V_SEG4:
        if (MACHINE_IS_TBBLUE) return debug_paginas_memoria_mapeadas[4];
    break;           

    case TPI_V_SEG5:
        if (MACHINE_IS_TBBLUE) return debug_paginas_memoria_mapeadas[5];
    break;           

    case TPI_V_SEG6:
        if (MACHINE_IS_TBBLUE) return debug_paginas_memoria_mapeadas[6];
    break;           

    case TPI_V_SEG7:
        if (MACHINE_IS_TBBLUE) return debug_paginas_memoria_mapeadas[7];
    break;           

    case TPI_V_HILOWMAPPED:
        return hilow_mapped_rom.v; //sirve tambien para ram, se mapean ram y rom al mismo tiempo
    break;

    case TPI_V_ROM:
	    //ram mapeada en 49152-65535 de Spectrum
	    if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) return (debug_paginas_memoria_mapeadas[0] & 127);

    break;
	
    case TPI_V_RAM:
	//ram mapeada en 49152-65535 de Spectrum
	    if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
		    return debug_paginas_memoria_mapeadas[3];
		    //TODO. condiciones especiales para mapeo de paginas del +2A tipo ram en rom
	    }

    case TPI_V_PD765_PCN:
        return pd765_pcn;
    break;

	//ram mapeada en 49152-65535 de Prism
        if (MACHINE_IS_PRISM) {
                return prism_retorna_ram_entra()*2;
	    }

    break;    

                    default:
                        //Para que no se queje el compilador por demas valores enum no tratados
                    break;


                }
  
            break;

/*
//si (NN)
	if (registro[0]=='(') {
		int s=strlen(registro);
		if (s>2) {
			if (registro[s-1]==')') {
				char buf_direccion[MAX_BREAKPOINT_CONDITION_LENGTH];
				//copiar saltando parentesis inicial
				sprintf (buf_direccion,"%s",&registro[1]);
				//quitar parentesis final
				//(16384) -> s=7 -> buf_direccion=16384). () -> s=2 ->buf_direccion=) .
				buf_direccion[s-2]=0;
				//printf ("buf_direccion: %s\n",buf_direccion);
				z80_int direccion=parse_string_to_number(buf_direccion);
				return peek_byte_no_time(direccion);
			}
		}
	}

	

	//enterrom, exitrom

//Avisa cuando se ha entrado o salido de rom. Solo salta una vez el breakpoint
//0: no esta en rom
//1: esta en rom y aun no ha saltado breakpoint
//2: esta en rom y ya ha saltado breakpoint
//int debug_enterrom=0;

//0: no ha salido de rom
//1: ha salido de rom y aun no ha saltado breakpoint
//2: ha salido de rom y ya ha saltado breakpoint
//int debug_exitrom=0;


	if (!strcasecmp(registro,"enterrom")) {
		if (debug_enterrom==1) {
			debug_enterrom++;
			return 1;
		}
		return 0;
	}

	if (!strcasecmp(registro,"exitrom")) {
		if (debug_exitrom==1) {
			debug_exitrom++;
			return 1;
		}
		return 0;
	}
 */



	        case TPT_REGISTRO: //a, bc, de, etc


                //Registros mk14
                if (CPU_IS_SCMP) {
                    switch (indice) {
                    case TPI_R_PC: return scmp_m_PC.w.l; break;

                    case TPI_R_AC: return scmp_m_AC; break;

                    case TPI_R_ER: return scmp_m_ER; break;

                    case TPI_R_SR: return scmp_m_SR; break;

                    case TPI_R_P1: return scmp_m_P1.w.l; break;

                    case TPI_R_P2: return scmp_m_P2.w.l; break;

                    case TPI_R_P3: return scmp_m_P3.w.l; break;



                    default:
                        //Para que no se queje el compilador por demas valores enum no tratados
                    break;

                    }
                }
                //Fin registros mk14

                //Registros motorola
                if (CPU_IS_MOTOROLA) {
                    switch (indice) {
                    case TPI_R_PC: return get_pc_register(); break;
                    case TPI_R_SP: return m68k_get_reg(NULL, M68K_REG_SP); break;
                    case TPI_R_USP: return m68k_get_reg(NULL, M68K_REG_USP); break;

                    case TPI_R_D0: return m68k_get_reg(NULL, M68K_REG_D0); break;
                    case TPI_R_D1: return m68k_get_reg(NULL, M68K_REG_D1); break;
                    case TPI_R_D2: return m68k_get_reg(NULL, M68K_REG_D2); break;
                    case TPI_R_D3: return m68k_get_reg(NULL, M68K_REG_D3); break;
                    case TPI_R_D4: return m68k_get_reg(NULL, M68K_REG_D4); break;
                    case TPI_R_D5: return m68k_get_reg(NULL, M68K_REG_D5); break;
                    case TPI_R_D6: return m68k_get_reg(NULL, M68K_REG_D6); break;
                    case TPI_R_D7: return m68k_get_reg(NULL, M68K_REG_D7); break;

                    case TPI_R_A0: return m68k_get_reg(NULL, M68K_REG_A0); break;
                    case TPI_R_A1: return m68k_get_reg(NULL, M68K_REG_A1); break;
                    case TPI_R_A2: return m68k_get_reg(NULL, M68K_REG_A2); break;
                    case TPI_R_A3: return m68k_get_reg(NULL, M68K_REG_A3); break;
                    case TPI_R_A4: return m68k_get_reg(NULL, M68K_REG_A4); break;
                    case TPI_R_A5: return m68k_get_reg(NULL, M68K_REG_A5); break;
                    case TPI_R_A6: return m68k_get_reg(NULL, M68K_REG_A6); break;
                    case TPI_R_A7: return m68k_get_reg(NULL, M68K_REG_A7); break;       



                    default:
                        //Para que no se queje el compilador por demas valores enum no tratados
                    break;

                    }
                }
               //Fin registros motorola                


                /*/if (indice==TPI_R_A) return reg_a;
                if (indice==TPI_R_BC) return reg_bc;
                if (indice==TPI_R_DE) return reg_de;*/
                switch (indice) {
    case TPI_R_PC: return reg_pc; break;
    case TPI_R_SP: return reg_sp; break;
    case TPI_R_IX: return reg_ix; break;
    case TPI_R_IY: return reg_iy; break;	

	case TPI_R_A: return reg_a; break;
	case TPI_R_B: return reg_b; break;
	case TPI_R_C: return reg_c; break;
	case TPI_R_D: return reg_d; break;
	case TPI_R_E: return reg_e; break;
	case TPI_R_F: return Z80_FLAGS; break;
	case TPI_R_H: return reg_h; break;
	case TPI_R_L: return reg_l; break;
	case TPI_R_I: return reg_i; break;
	case TPI_R_R: return (reg_r&127)|(reg_r_bit7&128); break;

        case TPI_R_AF: return REG_AF; break;
        case TPI_R_BC: return reg_bc; break;
        case TPI_R_DE: return reg_de; break;
        case TPI_R_HL: return reg_hl; break;



	case TPI_R_A_SHADOW: return reg_a_shadow; break;
	case TPI_R_B_SHADOW: return reg_b_shadow; break;
	case TPI_R_C_SHADOW: return reg_c_shadow; break;
	case TPI_R_D_SHADOW: return reg_d_shadow; break;
	case TPI_R_E_SHADOW: return reg_e_shadow; break;
	case TPI_R_F_SHADOW: return Z80_FLAGS_SHADOW; break;
	case TPI_R_H_SHADOW: return reg_h_shadow; break;
	case TPI_R_L_SHADOW: return reg_l_shadow; break;

        case TPI_R_AF_SHADOW: return REG_AF_SHADOW; break;
        case TPI_R_BC_SHADOW: return REG_BC_SHADOW; break;
        case TPI_R_DE_SHADOW: return REG_DE_SHADOW; break;
        case TPI_R_HL_SHADOW: return REG_HL_SHADOW; break;

        case TPI_R_FS: return ( Z80_FLAGS & FLAG_S ? 1 : 0); break;
        case TPI_R_FZ: return ( Z80_FLAGS & FLAG_Z ? 1 : 0); break;

        case TPI_R_FP: 
        case TPI_R_FV: 
            return ( Z80_FLAGS & FLAG_PV ? 1 : 0);
        break;

        case TPI_R_FH: return ( Z80_FLAGS & FLAG_H ? 1 : 0); break;
        case TPI_R_FN: return ( Z80_FLAGS & FLAG_N ? 1 : 0); break;
        case TPI_R_FC: return ( Z80_FLAGS & FLAG_C ? 1 : 0); break;


        case TPI_R_COPPERPC: return tbblue_copper_get_pc(); break;


                    default:
                        //Para que no se queje el compilador por demas valores enum no tratados
                    break;
                }




            break;

         
            


	        case TPT_OPERADOR_LOGICO:  //and, or, xor
            case TPT_OPERADOR_CONDICIONAL:  //=, <,>, <>,
            case TPT_OPERADOR_CALCULO: //+,-,*,/. & (and), | (or), ^ (xor)
            case TPT_PARENTESIS:
            case TPT_FUNCION:
            case TPT_FIN:
                //esto no se llega nunca aqui. Lo pongo para que no se queje el compilador
            break;



    }

    return resultado;
}

//calcula valor resultante de aplicar operador, puede ser 
int exp_par_calculate_operador(int valor_izquierda,int valor_derecha,enum token_parser_tipo tipo,enum token_parser_indice indice)
{

    int resultado=0; //asumimos cero
    
    /*
    	TPT_OPERADOR_LOGICO, //and, or, xor
	TPT_OPERADOR_CONDICIONAL, //=, <,>, <>,
	TPT_OPERADOR_CALCULO //+,-,*,/. & (and), | (or), ^ (xor)
     */

    //printf ("exp_par_calculate_operador tipo %d indice %d\n",tipo,indice);

    switch (tipo) {
            case TPT_NUMERO:
            case TPT_PARENTESIS:
	        case TPT_VARIABLE: //mra,mrw, etc
	        case TPT_REGISTRO: //a, bc, de, etc
            case TPT_FUNCION:
            case TPT_FIN:
                //esto no se llega nunca aqui. Lo pongo para que no se queje el compilador
            break;


	        case TPT_OPERADOR_LOGICO:  //and, or, xor

                if (indice==TPI_OL_AND) {   
                    if (valor_izquierda && valor_derecha) return 1;
                }

                if (indice==TPI_OL_OR) {   
                    if (valor_izquierda || valor_derecha) return 1;
                }                

                if (indice==TPI_OL_XOR) {
                    if (valor_izquierda && valor_derecha) return 0;  
                    else if (!valor_izquierda && valor_derecha) return 1;  
                    else if (valor_izquierda && !valor_derecha) return 1;  
                    else return 0; //ambos a 0
                }                   

            break;

            case TPT_OPERADOR_CONDICIONAL:  //=, <,>, <>,
            //printf ("operaodr condicional\n"); 
                if (indice==TPI_OC_MAYOR) {
                    //printf ("operaodr mayor\n");    
                    if (valor_izquierda>valor_derecha) return 1;
                }
                if (indice==TPI_OC_MENOR) {   
                    if (valor_izquierda<valor_derecha) return 1;
                }

                if (indice==TPI_OC_MAYOR_IGUAL) {
                    if (valor_izquierda>=valor_derecha) return 1;
                }
                if (indice==TPI_OC_MENOR_IGUAL) {   
                    if (valor_izquierda<=valor_derecha) return 1;
                }                

                if (indice==TPI_OC_IGUAL) {   
                    if (valor_izquierda==valor_derecha) return 1;
                }           

                if (indice==TPI_OC_DIFERENTE) {  
                    if (valor_izquierda!=valor_derecha) return 1;
                }                          

            break;

            case TPT_OPERADOR_CALCULO: //+,-,*,/. & (and), | (or), ^ (xor)

                if (indice==TPI_OC_SUMA) {
                    //printf ("sumando %d y %d\n",valor_izquierda,valor_derecha);
                    return valor_izquierda + valor_derecha;
                }         

                if (indice==TPI_OC_RESTA) {
                    return valor_izquierda - valor_derecha;
                }          

                if (indice==TPI_OC_MULTIPLICACION) {
                    //printf ("multiplicando %d y %d\n",valor_izquierda,valor_derecha);
                    return valor_izquierda * valor_derecha;
                }         

                if (indice==TPI_OC_DIVISION) {

                    //controlar division por cero
                    if (valor_derecha==0) {
                        //retornamos valor 16 bits maximo
                        return 0xffff;
                    }
                    else return valor_izquierda / valor_derecha;
                }        

                if (indice==TPI_OC_AND) {
                    return valor_izquierda & valor_derecha;
                }     

                if (indice==TPI_OC_OR) {
                    return valor_izquierda | valor_derecha;
                }          

                if (indice==TPI_OC_XOR) {
                    return valor_izquierda ^ valor_derecha;
                }                                                                          

            break;



    }

    return resultado;
}


//calcula valor resultante de aplicar funcion
int exp_par_calculate_funcion(int valor,enum token_parser_tipo tipo,enum token_parser_indice indice)
{

    int resultado=0; //asumimos cero
    

    switch (tipo) {
            case TPT_NUMERO:
            case TPT_PARENTESIS:
	        case TPT_VARIABLE: //mra,mrw, etc
	        case TPT_REGISTRO: //a, bc, de, etc
            case TPT_OPERADOR_LOGICO:
            case TPT_OPERADOR_CONDICIONAL:
            case TPT_OPERADOR_CALCULO:
            case TPT_FIN:
                //esto no se llega nunca aqui. Lo pongo para que no se queje el compilador
            break;


            case TPT_FUNCION:
                switch (indice) {
                    case TPI_F_PEEK:
        				return peek_byte_z80_moto(valor);
                    break;

                    case TPI_F_PEEKW:
        				return peek_byte_z80_moto(valor)+256*peek_byte_z80_moto(valor+1);
                    break;

                    case TPI_F_FPEEK:
        				return far_peek_byte(valor);
                    break;
                    
                    case TPI_F_IN:
                    if (MACHINE_IS_SPECTRUM) {
                        return lee_puerto_spectrum_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }
                    //zx80,81 no conviene pues la lectura de puerto genera vsync


                    else if (MACHINE_IS_Z88) {
                        return lee_puerto_z88_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }

                    else if (MACHINE_IS_ACE) {
                        return lee_puerto_ace_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }

                    else if (MACHINE_IS_CPC) {
                        return lee_puerto_cpc_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }

                    else if (MACHINE_IS_PCW) {
                        return lee_puerto_pcw_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }

                    else if (MACHINE_IS_SAM) {
                        return lee_puerto_sam_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }                                                            

                    else if (MACHINE_IS_MSX) {
                        return lee_puerto_msx1_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }

                    else if (MACHINE_IS_COLECO) {
                        return lee_puerto_coleco_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }

                    else if (MACHINE_IS_SG1000) {
                        return lee_puerto_sg1000_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }

                    else if (MACHINE_IS_SMS) {
                        return lee_puerto_sms_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }                    

                    else if (MACHINE_IS_SVI) {
                        return lee_puerto_svi_no_time(value_16_to_8h(valor),value_16_to_8l(valor));
                    }                                                            

                    else return 255;
                    break;


                    case TPI_F_NOT:
        				return !valor;
                    break;       

                    case TPI_F_ABS:
        				if (valor<0) return -valor;
                        else return valor;
                    break;         

                    case TPI_F_BYTE:
        				return valor & 0xFF;
                    break;          

                    case TPI_F_WORD:
        				return valor & 0xFFFF;
                    break;

                    case TPI_F_OPMWA:
                        if (debug_mmu_mwa==(unsigned int)valor || anterior_debug_mmu_mwa==(unsigned int)valor) return 1;
                        else return 0;
                    break;

                    case TPI_F_OPMRA:
                        if (debug_mmu_mra==(unsigned int)valor || anterior_debug_mmu_mra==(unsigned int)valor) return 1;
                        else return 0;
                    break;    

                    case TPI_F_OPMWV:
                        if (debug_mmu_mwv==(unsigned int)valor || anterior_debug_mmu_mwv==(unsigned int)valor) return 1;
                        else return 0;
                    break;

                    case TPI_F_OPMRV:
                        if (debug_mmu_mrv==(unsigned int)valor || anterior_debug_mmu_mrv==(unsigned int)valor) return 1;
                        else return 0;
                    break;                                      

                    default:
                        //Para que no se queje el compilador por demas valores enum no tratados
                    break;    
                    
                }
            break;



    }

    return resultado;
}


//Devuelve el indice donde hay final de paréntesis. Se le empieza token con {
//Se va abriendo y cerrando hasta igualar nivel paréntesis
//Devuelve -1 si no se detecta final
int exp_par_final_parentesis(token_parser *tokens,int longitud_tokens)
{
    int i=0;
    int nivel_parentesis=0;

    //Avanzamos el índice. suponemos que habia {
    i++;
    nivel_parentesis=1;

    for (;i<longitud_tokens && tokens[i].tipo!=TPT_FIN;i++) {
        if (tokens[i].tipo==TPT_PARENTESIS) {
            if (tokens[i].indice==TPI_P_ABRIR) {
              nivel_parentesis++;  
            }
            else if (tokens[i].indice==TPI_P_CERRAR) {
                nivel_parentesis--;
                //final?
                if (nivel_parentesis==0) {
                    return i;
                }
            }
        }
    }

    //final y no hay cierre conveniente
    return -1;

}


//Calcula la expresion identificada por tokens. Funcion recursiva
//final identifica al siguiente token despues del final. Poner valor alto para que no tenga final y detecte token de fin
//error_code puede ser 0 o 1
int exp_par_evaluate_token(token_parser *tokens,int longitud_tokens,int *error_code)
{
/*
Evaluar expresión:

Buscar si hay operador logico and, or, xor. Si lo hay, separar sub condiciones

Evaluar condiciones: buscar si hay comparadores =, <,>, <>. Si lo hay, evaluar cada uno de los valores
Cada condición genera 0 o 1

Evaluar valores: por orden, evaluar valores, variables  y posibles operadores de cálculo: +,-,*,/. & (and), | (or), ^ (xor)
 */
    //printf ("evaluando tokens hasta longitud %d\n",final);

    *error_code=0; //asumimos ok

    if (longitud_tokens==0) {
        //expresion vacia. no deberia suceder. retornar 0
        //printf ("expresion vacia\n");
        *error_code=1;
        return 0;
    }

    //Ver si hay operadores logicos
    //int i;

    int i=0;

    int nivel_parentesis=0;


    //debug mostrar tokens
    /*
     printf ("--exp_par_evaluate_token. longitud %d. dump:\n",longitud_tokens);
    exp_par_debug_dump_tokens(tokens,longitud_tokens);
    printf ("--end dump\n");

    char buffer_destino[1024];
    exp_par_tokens_to_exp(tokens,buffer_destino,longitud_tokens);
    printf ("--exp_par_evaluate_token. expression to parse: [%s]\n",buffer_destino);
    */
    
    //fin debug mostrar tokens

    //int calculado_izquierda=0;
    int valor_izquierda;
    int valor_derecha;
    //int pos_inicial=0;


    //Empieza con parentesis?. temp desactivado
    /*if (tokens[i].tipo==TPT_PARENTESIS && tokens[i].indice==TPI_P_ABRIR) {
        //buscar hasta cierre
        int final_par=exp_par_final_parentesis(tokens,longitud_tokens);
        printf ("cierre parentesis en indice %d\n",final_par);
        if (final_par<0) {
            *error_code=1;
            return 0;
        }
        else {
            //calculamos valor entre llaves
            i++;
            calculado_izquierda=1;

            //debug parentesis
            char buffer_destino[1024];
            exp_par_tokens_to_exp(&tokens[i],buffer_destino,final_par-1);
            printf ("evaluar parentesis: [%s]\n",buffer_destino);
            //fin debug parentesis


            int otro_err_code;
            valor_izquierda=exp_par_evaluate_token(&tokens[i],final_par-1,&otro_err_code);
            
            if ( otro_err_code <0) {
                *error_code=otro_err_code;
                return 0; //ha habido error
            }

            //Alternativa 1: avanzamos indice puntero, cambiando pos_inicial
            //pos_inicial=final_par+1; //seguimos evaluacion desde despues parentesis

            //Alternativa 2: avanzamos puntero tokens, descartando el resto
            i=final_par+1;
            longitud_tokens -=i;
            tokens=&tokens[i];
            printf ("vuelta de evaluar parentesis. valor_izquierda=%d longitud restante: %d indice actual: %d\n",valor_izquierda,longitud_tokens,i);
        }
    }*/

    nivel_parentesis=0;
    for (i=0;i<longitud_tokens && tokens[i].tipo!=TPT_FIN;i++) {

        //Al separar por operador, ver que no estemos dentro de parentesis
        if (tokens[i].tipo==TPT_PARENTESIS && tokens[i].indice==TPI_P_ABRIR) nivel_parentesis++;
        if (tokens[i].tipo==TPT_PARENTESIS && tokens[i].indice==TPI_P_CERRAR) nivel_parentesis--;

        if (tokens[i].tipo==TPT_OPERADOR_LOGICO && !nivel_parentesis ) {
            //Evaluar parte izquierda y derecha y aplicar operador
            
            

            int errorcode1,errorcode2;

            int longitud_izquierda=i;
            int longitud_derecha=longitud_tokens-longitud_izquierda-1;
            //printf ("operador logico\n");

            valor_izquierda=exp_par_evaluate_token(tokens,longitud_izquierda,&errorcode1);
            valor_derecha=exp_par_evaluate_token(&tokens[i+1],longitud_derecha,&errorcode2);

            *error_code=errorcode1 | errorcode2; //cualquiera de los dos puede ser 0 o 1, hacer OR de los dos

            return exp_par_calculate_operador(valor_izquierda,valor_derecha,tokens[i].tipo,tokens[i].indice);
        }
    }


    nivel_parentesis=0;
    //printf ("inicio check condicionales\n");
    for (i=0;i<longitud_tokens && tokens[i].tipo!=TPT_FIN;i++) {

        //Al separar por operador, ver que no estemos dentro de parentesis
        if (tokens[i].tipo==TPT_PARENTESIS && tokens[i].indice==TPI_P_ABRIR) {
            //printf ("abrir parentesis en %d\n",i);
            nivel_parentesis++;
        }
        if (tokens[i].tipo==TPT_PARENTESIS && tokens[i].indice==TPI_P_CERRAR) {
            //printf ("cerrar parentesis en %d\n",i);
            nivel_parentesis--;
        }

        if (tokens[i].tipo==TPT_OPERADOR_CONDICIONAL) {
            //printf ("token es condicional y parentesis=%d\n",nivel_parentesis);
        }

        if (tokens[i].tipo==TPT_OPERADOR_CONDICIONAL && !nivel_parentesis) {
            //Evaluar parte izquierda y derecha y aplicar operador
            


            int errorcode1,errorcode2;

            int longitud_izquierda=i;
            int longitud_derecha=longitud_tokens-longitud_izquierda-1;            

            //printf ("operador condicionales\n");

            valor_izquierda=exp_par_evaluate_token(tokens,longitud_izquierda,&errorcode1);
            valor_derecha=exp_par_evaluate_token(&tokens[i+1],longitud_derecha,&errorcode2);

            *error_code=errorcode1 | errorcode2; //cualquiera de los dos puede ser 0 o 1, hacer OR de los dos


            //printf ("condicional [%d] y [%d]\n",valor_izquierda,valor_derecha);
            int resul=exp_par_calculate_operador(valor_izquierda,valor_derecha,tokens[i].tipo,tokens[i].indice);

            //printf ("resultado condicional %d\n",resul);
            return resul;
        }
    }

    //printf ("fin check condicionales\n");
    

    nivel_parentesis=0;
    for (i=0;i<longitud_tokens && tokens[i].tipo!=TPT_FIN;i++) {
   
        //Al separar por operador, ver que no estemos dentro de parentesis
        if (tokens[i].tipo==TPT_PARENTESIS && tokens[i].indice==TPI_P_ABRIR) nivel_parentesis++;
        if (tokens[i].tipo==TPT_PARENTESIS && tokens[i].indice==TPI_P_CERRAR) nivel_parentesis--;

        //Para sumas y restas, mas prioridad que dividir o multiplicar

        if (tokens[i].tipo==TPT_OPERADOR_CALCULO && 
                !nivel_parentesis &&
                (tokens[i].indice==TPI_OC_SUMA || tokens[i].indice==TPI_OC_RESTA)
        ) {
            //Evaluar parte izquierda y derecha y aplicar operador
            


            int errorcode1,errorcode2;

            int longitud_izquierda=i;
            int longitud_derecha=longitud_tokens-longitud_izquierda-1;            

            //printf ("operador suma/resta\n");

            valor_izquierda=exp_par_evaluate_token(tokens,longitud_izquierda,&errorcode1);
            valor_derecha=exp_par_evaluate_token(&tokens[i+1],longitud_derecha,&errorcode2);

            *error_code=errorcode1 | errorcode2; //cualquiera de los dos puede ser 0 o 1, hacer OR de los dos


            return exp_par_calculate_operador(valor_izquierda,valor_derecha,tokens[i].tipo,tokens[i].indice);
        }   

    }

    nivel_parentesis=0;
    for (i=0;i<longitud_tokens && tokens[i].tipo!=TPT_FIN;i++) {

        //Al separar por operador, ver que no estemos dentro de parentesis
        if (tokens[i].tipo==TPT_PARENTESIS && tokens[i].indice==TPI_P_ABRIR) nivel_parentesis++;
        if (tokens[i].tipo==TPT_PARENTESIS && tokens[i].indice==TPI_P_CERRAR) nivel_parentesis--;        
   
        //Pero no sumas y restas

        if (tokens[i].tipo==TPT_OPERADOR_CALCULO &&
        !nivel_parentesis &&
        (tokens[i].indice!=TPI_OC_SUMA && tokens[i].indice!=TPI_OC_RESTA)
        ) {
            //Evaluar parte izquierda y derecha y aplicar operador
            


            int errorcode1,errorcode2;

            int longitud_izquierda=i;
            int longitud_derecha=longitud_tokens-longitud_izquierda-1;            

            //printf ("operador multi/divi etc\n");

            valor_izquierda=exp_par_evaluate_token(tokens,longitud_izquierda,&errorcode1);
            valor_derecha=exp_par_evaluate_token(&tokens[i+1],longitud_derecha,&errorcode2);

            *error_code=errorcode1 | errorcode2; //cualquiera de los dos puede ser 0 o 1, hacer OR de los dos


            return exp_par_calculate_operador(valor_izquierda,valor_derecha,tokens[i].tipo,tokens[i].indice);
        }   

    }

    if (tokens[0].tipo==TPT_FUNCION) {
        //printf ("es funcion\n");
    //buscar hasta cierre
        int final_par=exp_par_final_parentesis(&tokens[1],longitud_tokens-1);
        //printf ("cierre funcion en indice %d\n",final_par);
        if (final_par<0) {
            *error_code=1;
            return 0;
        }
        else {
            //calculamos valor entre llaves
            //int longitud_dentro_llaves=final_par-1; //el pa

            //debug parentesis
            char buffer_destino[1024];
            exp_par_tokens_to_exp(&tokens[2],buffer_destino,final_par-1);
            //printf ("evaluar funcion para valor: [%s]\n",buffer_destino);
            //fin debug parentesis


            int errorcode2;
            valor_izquierda=exp_par_evaluate_token(&tokens[2],final_par-1,&errorcode2);

            *error_code=errorcode2;

            
            if ( *error_code ) {
                return 0; //ha habido error
            }

            int valor_resultante=exp_par_calculate_funcion(valor_izquierda,TPT_FUNCION,tokens[0].indice);
            return valor_resultante;
        }
    }

    i=0;
    if (tokens[i].tipo==TPT_PARENTESIS && tokens[i].indice==TPI_P_ABRIR) {
        //buscar hasta cierre
        int final_par=exp_par_final_parentesis(tokens,longitud_tokens);
        //printf ("cierre parentesis en indice %d\n",final_par);
        if (final_par<0) {
            *error_code=1;
            return 0;
        }
        else {
            //calculamos valor entre llaves
            i++;
            //calculado_izquierda=1;

            //debug parentesis
            /*
            char buffer_destino[1024];
            exp_par_tokens_to_exp(&tokens[i],buffer_destino,final_par-1);
            printf ("evaluar parentesis: [%s]\n",buffer_destino);
            */
            //fin debug parentesis


            int errorcode2;
            int valor_parentesis=exp_par_evaluate_token(&tokens[i],final_par-1,&errorcode2);

            *error_code=errorcode2;

            
            if ( *error_code ) {
                return 0; //ha habido error
            }
        
            return valor_parentesis;
        }
    }



    if ( (tokens[0].tipo==TPT_NUMERO || tokens[0].tipo==TPT_VARIABLE || tokens[0].tipo==TPT_REGISTRO)
             )
    {
            //printf ("es variable\n");
            //tiene que ser numero
            int resultado=exp_par_calculate_numvarreg(&tokens[0]);
            //printf("resultado variable: %d\n",resultado);
            return resultado;
    }


    //Aqui no deberia llegar nunca
    //printf ("fin evaluar token sin coincidir ninguno. posible token de fin\n");
    *error_code=1;
    return 0;

}

//funcion para hacer dump de los tokens por pantalla. Se utiliza para debugar, y no en el proceso normal del programa
void exp_par_debug_dump_tokens(token_parser *tokens,int longitud)
{

	int i=0;

	printf ("Printing tokens\n");
	while (tokens[i].tipo!=TPT_FIN && longitud) {
		printf ("%d: tipo: %d indice: %d formato: %d signo: %d valor: %d\n",
			i,
			tokens[i].tipo,
			tokens[i].indice,
			tokens[i].formato,
			tokens[i].signo,
			tokens[i].valor
		);
		i++;
        longitud--;
	}

}

//Evalua expresion de entrada y la muestra en salida
//Retorna:
//0 si ok
//1 si error parseando
//2 si error evaluando
//En salida retorna valor numerico o mensaje de error
//en string_detoken retorna la cadena resultado de pasar el token parseado a string
int exp_par_evaluate_expression(char *entrada,char *salida,char *string_detoken)
{


	//Mis tokens de salida
	token_parser tokens[MAX_PARSER_TOKENS_NUM];
	int result;

	//printf ("\nText to token: %s\n",string_texto);
	result=exp_par_exp_to_tokens(entrada,tokens);
	//printf ("result: %d\n",result);
	if (result>=0) {
			//Pasamos primero a string de nuevo
			//char string_detoken[MAX_BREAKPOINT_CONDITION_LENGTH];
			exp_par_tokens_to_exp(tokens,string_detoken,MAX_PARSER_TOKENS_NUM);
			
			int error_code;  
			
			
			int resultado=exp_par_evaluate_token(tokens,MAX_PARSER_TOKENS_NUM,&error_code);
            if (error_code) {
                strcpy(salida,"Error evaluating");
                return 2;
            }
            else {
			    sprintf(salida,"%d",resultado);
            }


	}
	else {
		strcpy(salida,"Error parsing");
        string_detoken[0]=0; //Ya que no lo ha parseado, cadena detoken vacia
        return 1;
	}

    return 0;
}




//Evalua expresion de entrada y la retorna como valor
//Puede retornar valor 0 si expresion con error
//Usado por ejemplo en breakpoint condition "putv" en que no hace falta que evaluemos si expresion con error o no
int exp_par_evaluate_expression_to_number(char *entrada)
{


	//Mis tokens de salida
	token_parser tokens[MAX_PARSER_TOKENS_NUM];
	int result;

	//printf ("\nText to token: %s\n",string_texto);
	result=exp_par_exp_to_tokens(entrada,tokens);
	//printf ("result: %d\n",result);
	if (result>=0) {
			
			int error_code;  
		
			int resultado=exp_par_evaluate_token(tokens,MAX_PARSER_TOKENS_NUM,&error_code);
            if (error_code) {
                //Error evaluating
                return 0;
            }
            else {
                //OK
                return resultado;
            }


	}
	else {
        //Error parsing
        return 0;

	}

}
