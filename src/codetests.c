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

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "codetests.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"
#include "mem128.h"
#include "screen.h"
#include "tbblue.h"

#include "disassemble.h"
#include "assemble.h"
#include "expression_parser.h"
#include "audio.h"
#include "zeng.h"
#include "network.h"
#include "settings.h"
#include "atomic.h"
#include "zeng_online.h"

void codetests_repetitions(void)
{

	z80_byte repetitions0[]={1,2,3,4,5,6,7,8,9,10};
	z80_byte repetitions1[]={1,1,3,4,5,6,7,8,9,10};
	z80_byte repetitions2[]={1,1,1,4,5,6,7,8,9,10};
	z80_byte repetitions3[]={1,1,1,1,5,6,7,8,9,10};
	z80_byte repetitions4[]={1,1,1,1,1,6,7,8,9,10};

	//int util_get_byte_repetitions(z80_byte *memoria,int longitud,z80_byte *byte_repetido)

	int repeticiones[5];
	z80_byte byte_repetido[5];

	int i;
	z80_byte *puntero=NULL;

	for (i=0;i<5;i++) {
		if      (i==0) puntero=repetitions0;
		else if (i==1) puntero=repetitions1;
		else if (i==2) puntero=repetitions2;
		else if (i==3) puntero=repetitions3;
		else if (i==4) puntero=repetitions4;

		repeticiones[i]=util_get_byte_repetitions(puntero,10,&byte_repetido[i]);

		printf ("step %d repetitions: %d byte_repeated: %d\n",i,repeticiones[i],byte_repetido[i]);

		//Validar cantidad de valores repetidos y que byte repetido
		printf ("expected: repetitions: %d byte_repeated: 1\n",i+1);
		if (byte_repetido[i]!=1 || repeticiones[i]!=i+1) {
			printf ("error\n");
			exit(1);
		}
	}
}

void coretests_dumphex(z80_byte *ptr,int longitud)
{
	while (longitud) {
		printf ("%02X ",*ptr);
		ptr++;
		longitud--;
	}
}



//mostrar unos cuantos del inicio y del final
void coretests_dumphex_inicio_fin(z80_byte *ptr,int longitud,int max_mostrar)
{

	int mostrar;
	int cortado=0;
	if (longitud>max_mostrar*2) {
		mostrar=max_mostrar;
		cortado=1;
	}
	else mostrar=longitud;

	coretests_dumphex(ptr,mostrar);


	if (cortado) {
		printf (" ... ");
		coretests_dumphex(ptr+longitud-mostrar,mostrar);
	}

}

void coretests_compress_repetitions_write_arr(z80_byte *variable,int indice,z80_byte valor,int size_array)
{
	//El bucle que llama aqui se sale de array, tanto por arriba como por abajo, por eso lo controlo y no escribo en ese caso
	if (indice<0 || indice>=size_array) {
		//printf ("Out of array: %d\n",indice);
		return;
	}
	variable[indice]=valor;
}

void coretests_compress_repetitions(void)
{


#define MAX_COMP_REP_ARRAY 2048

	z80_byte repetitions[MAX_COMP_REP_ARRAY];

	z80_byte compressed_data[MAX_COMP_REP_ARRAY*2];

	int max_array=MAX_COMP_REP_ARRAY; //siempre menor o igual que MAX_COMP_REP_ARRAY. tamanyo de los datos a analizar

    int i;

	int max_veces=MAX_COMP_REP_ARRAY; //Siempre menor o igual que MAX_COMP_REP_ARRAY. cuantos bytes repetimos

	z80_byte magic_byte=0xDD;

    for (i=0;i<=max_veces;i++) {

		int j;

		//Inicializar con valores consecutivos
		printf ("Initializing with consecutive values\n");
		for (j=0;j<max_array;j++) {
			//repetitions[j]=j&255;
			coretests_compress_repetitions_write_arr(repetitions,j,j&255,MAX_COMP_REP_ARRAY);
		}

		printf ("Initializing with 0 from the left\n");
		//Meter valores "0" al principio
		for (j=0;j<=i;j++) {
			//repetitions[j]=0;
			coretests_compress_repetitions_write_arr(repetitions,j,0,MAX_COMP_REP_ARRAY);
		}

		printf ("Initializing with 1 from the right\n");
		//Meter valores "1" al final
		for (j=0;j<=i;j++) {
			coretests_compress_repetitions_write_arr(repetitions,max_array-1-j,1,MAX_COMP_REP_ARRAY);
			//repetitions[max_array-1-j]=1;
		}

                //repeticiones[i]=util_get_byte_repetitions(puntero,10,&byte_repetido[i]);
		printf ("step %d length: %d. 0's at beginning: %d. 1's at end: %d\n",i,max_array,i+1,i+1);

		coretests_dumphex_inicio_fin(repetitions,max_array,20);

		printf ("\n");

		int longitud_destino=util_compress_data_repetitions(repetitions,compressed_data,max_array,magic_byte);

		printf ("compressed length: %d\n",longitud_destino);

		//coretests_dumphex(compressed_data,longitud_destino);
		coretests_dumphex_inicio_fin(compressed_data,longitud_destino,20);
		printf ("\n");



		//Validar, pero solo para iteraciones < 256. mas alla de ahi, dificil de calcular
		int limite=256-4-magic_byte;
		//A partir de 33 con magic_byte=0xDD falla el calculo porque hay:
		//D8 D9 DA DB DC DD 1 1 1 1 1 1 1 1 ....... Ese DD aislado hay que escaparlo como 1 sola repeticion
		//que se traduce en:
		//D8 D9 DA DB DC DD DD DD 01 DD DD 01 22

		if (i<limite) {
			//Validacion solo de longitud comprimida. El contenido, hacer una validacion manual
			int valor_esperado_comprimido=max_array;
			if (i>3) valor_esperado_comprimido=max_array-(i-3)*2;

			printf ("Expected length: %d\n",valor_esperado_comprimido);

			if (valor_esperado_comprimido!=longitud_destino) {
                	        printf ("error\n");
                        	exit(1);
	                }
		}

		printf ("\n");
    }


}

void coretests_read_file_memory(char *filename,z80_byte *memoria)
{
		long long int tamanyo;
		tamanyo=get_file_size(filename);


                FILE *ptr_file;
                ptr_file=fopen(filename,"rb");

                if (!ptr_file) {
                        printf ("Unable to open file %s",filename);
                        exit(1);
                }




                fread(memoria,1,tamanyo,ptr_file);


                fclose(ptr_file);
}

void coretests_compress_uncompress_repetitions_aux(char *filename)
{
	z80_byte *memoria_file_orig;
	z80_byte *memoria_file_compressed;
	z80_byte *memoria_file_uncompressed;

	long long int tamanyo=get_file_size(filename);

	//Memoria para lectura, comprimir y descomprimir
	//tamanyo, tamanyo*2, tamanyo*2

	memoria_file_orig=malloc(tamanyo);
	if (memoria_file_orig==NULL) {
		printf("Error allocating memory\n");
		exit(1);
	}

        memoria_file_compressed=malloc(tamanyo*2);
        if (memoria_file_compressed==NULL) {
                printf("Error allocating memory\n");
                exit(1);
        }

        memoria_file_uncompressed=malloc(tamanyo*2);
        if (memoria_file_uncompressed==NULL) {
                printf("Error allocating memory\n");
                exit(1);
        }

	coretests_read_file_memory(filename,memoria_file_orig);

/*
extern int util_compress_data_repetitions(z80_byte *origen,z80_byte *destino,int longitud,z80_byte magic_byte);

extern int util_uncompress_data_repetitions(z80_byte *origen,z80_byte *destino,int longitud,z80_byte magic_byte);
*/

	z80_byte magic_byte=0xDD;

	printf ("Original size: %lld\n",tamanyo);

	int longitud_comprimido=util_compress_data_repetitions(memoria_file_orig,memoria_file_compressed,tamanyo,magic_byte);
	int porcentaje;
	if (tamanyo==0) porcentaje=100;
	else porcentaje=(longitud_comprimido*100)/tamanyo;
	printf ("Compressed size: %d (%d%%)\n",longitud_comprimido,porcentaje);

	int longitud_descomprido=util_uncompress_data_repetitions(memoria_file_compressed,memoria_file_uncompressed,longitud_comprimido,magic_byte);
	printf ("Uncompressed size: %d\n",longitud_descomprido);

	int error=0;

	//Primera comprobacion de tamanyo
	if (tamanyo!=longitud_descomprido) {
		printf ("Original size and uncompressed size doesnt match\n");
		error=1;
	}

	//Y luego comparar byte a byte
	int i;
	for (i=0;i<tamanyo;i++) {
		z80_byte byte_orig,byte_uncompress;
		byte_orig=memoria_file_orig[i];
		byte_uncompress=memoria_file_uncompressed[i];
		if (byte_orig!=byte_uncompress) {
			printf("Difference in offset %XH. Original byte: %02XH Uncompressed byte: %02XH\n",i,byte_orig,byte_uncompress);
			error++;
		}

		if (error>=10) {
			printf ("And more errors.... showing only first 10\n");
			exit(1);
		}
	}


	if (error) {
		exit(1);
	}

	else {
		printf ("Compress/Uncompress ok\n");
	}

}

void coretests_compress_uncompress_repetitions_zip_aux(char *filename)
{
	z80_byte *memoria_file_orig;
	z80_byte *memoria_file_compressed;
	z80_byte *memoria_file_uncompressed;

	long long int tamanyo=get_file_size(filename);

	//Memoria para lectura, comprimir y descomprimir
	//tamanyo, tamanyo*2, tamanyo*2

	memoria_file_orig=malloc(tamanyo);
	if (memoria_file_orig==NULL) {
		printf("Error allocating memory\n");
		exit(1);
	}

        /*memoria_file_compressed=malloc(tamanyo*2);
        if (memoria_file_compressed==NULL) {
                printf("Error allocating memory\n");
                exit(1);
        }

        memoria_file_uncompressed=malloc(tamanyo*2);
        if (memoria_file_uncompressed==NULL) {
                printf("Error allocating memory\n");
                exit(1);
        }*/

	coretests_read_file_memory(filename,memoria_file_orig);

/*
extern int util_compress_data_repetitions(z80_byte *origen,z80_byte *destino,int longitud,z80_byte magic_byte);

extern int util_uncompress_data_repetitions(z80_byte *origen,z80_byte *destino,int longitud,z80_byte magic_byte);
*/

	//z80_byte magic_byte=0xDD;

	printf ("Original size: %lld\n",tamanyo);

	//int longitud_comprimido=util_compress_data_repetitions(memoria_file_orig,memoria_file_compressed,tamanyo,magic_byte);


    int longitud_comprimido;
    memoria_file_compressed=util_compress_memory_zip(memoria_file_orig,tamanyo,&longitud_comprimido,"prueba.raw");

    //printf("Snapshot uncompressed: %d compressed: %d\n",tamanyo,longitud_comprimido);



	int porcentaje;
	if (tamanyo==0) porcentaje=100;
	else porcentaje=(longitud_comprimido*100)/tamanyo;
	printf ("Compressed size: %d (%d%%)\n",longitud_comprimido,porcentaje);

	//int longitud_descomprido=util_uncompress_data_repetitions(memoria_file_compressed,memoria_file_uncompressed,longitud_comprimido,magic_byte);

    int longitud_descomprimido;
    memoria_file_uncompressed=util_uncompress_memory_zip(memoria_file_compressed,longitud_comprimido,&longitud_descomprimido,"prueba.raw");

	printf ("Uncompressed size: %d\n",longitud_descomprimido);


	int error=0;

	//Primera comprobacion de tamanyo
	if (tamanyo!=longitud_descomprimido) {
		printf ("Original size and uncompressed size doesnt match\n");
		error=1;
	}

	//Y luego comparar byte a byte
	int i;
	for (i=0;i<tamanyo;i++) {
		z80_byte byte_orig,byte_uncompress;
		byte_orig=memoria_file_orig[i];
		byte_uncompress=memoria_file_uncompressed[i];
		if (byte_orig!=byte_uncompress) {
			printf("Difference in offset %XH. Original byte: %02XH Uncompressed byte: %02XH\n",i,byte_orig,byte_uncompress);
			error++;
		}

		if (error>=10) {
			printf ("And more errors.... showing only first 10\n");
			exit(1);
		}
	}


	if (error) {
		exit(1);
	}

	else {
		printf ("Compress/Uncompress ok\n");
	}

}

void coretests_compress_uncompress_repetitions(char *archivo)
{
	printf ("Testing compression routine with file %s\n",archivo);
	coretests_compress_uncompress_repetitions_aux(archivo);
}

void coretests_compress_uncompress_repetitions_zip(char *archivo)
{
	printf ("Testing zip compression routine with file %s\n",archivo);
	coretests_compress_uncompress_repetitions_zip_aux(archivo);
}

void codetests_tbblue_get_horizontal_raster(void)
{


	screen_testados_linea=224;

	int i;
	for (i=0;i<69888;i++) {
		t_estados=i;
		int estados_en_linea=t_estados % screen_testados_linea;
		int linea=t_estados/screen_testados_linea;
		int horiz=tbblue_get_current_raster_horiz_position();

		printf ("t-total %5d line %3d t_states %3d. horiz: %3d\n",i,linea,estados_en_linea,horiz );
		if (horiz!=estados_en_linea/4) {
			printf ("Error\n");
			exit(1);
		}
	}
}

/*void codetests_cut_line(void)
{

	//	extern void menu_util_cut_line_at_spaces(int posicion_corte, char *texto,char *linea1, char *linea2);

	char linea1[200];
	char linea2[200];

	char *entrada="Hola como estas yo bien y tu";

	int corte;

	for (corte=0;corte<30;corte++) {
		menu_util_cut_line_at_spaces(corte,entrada,linea1,linea2);
		printf ("\nEntrada: [%s]\nCorte en %d\nLinea 1: [%s]\nLinea 2: [%s]\n",entrada,corte,linea1,linea2);
	}

	exit(0);


}*/



void codetests_tbblue_layers(void)
{
//tbblue_get_string_layer_prio
//+extern char *tbblue_get_string_layer_prio(int layer,z80_byte prio);


	int layer, prio;

	for (prio=0;prio<8;prio++) {
		printf ("Priority %d\n",prio);
		for (layer=0;layer<3;layer++) {
			printf ("Layer %d : %s\n",layer,tbblue_get_string_layer_prio(layer,prio));
		}
	}
}

void codetests_assembler_print(char *s1,char *s2,char *s3, char *s4)
{
	printf ("%s\nOpcode: [%s]\nFirst op: [%s]\nSecond op: [%s]\n\n",s1,s2,s3,s4);
}

void codetests_assemble_opcode(char *instruccion,z80_byte *destino)
{
	int longitud=assemble_opcode(16384,instruccion,destino);
        printf ("Longitud opcode: %d\n",longitud);
	if (longitud) {
		printf ("Codigo generado: ");
	}

	while (longitud) {
		printf ("%02XH ",*destino);
		destino++;
		longitud--;
	};

	if (longitud) {
                printf ("\n");
	}
}

void codetests_assembler(void)
{
	//void asm_return_op_ops(char *origen,char *opcode,char *primer_op,char *segundo_op)
	/*
	char buf_opcode[100];
	char buf_primer_op[100];
	char buf_segundo_op[100];

	asm_return_op_ops("NOP",buf_opcode,buf_primer_op,buf_segundo_op);
	codetests_assembler_print("NOP",buf_opcode,buf_primer_op,buf_segundo_op);

	asm_return_op_ops("PUSH AF",buf_opcode,buf_primer_op,buf_segundo_op);
	codetests_assembler_print("PUSH AF",buf_opcode,buf_primer_op,buf_segundo_op);

	asm_return_op_ops("EX DE,HL",buf_opcode,buf_primer_op,buf_segundo_op);
	codetests_assembler_print("EX DE,HL",buf_opcode,buf_primer_op,buf_segundo_op);

	asm_return_op_ops("PUSH   AF",buf_opcode,buf_primer_op,buf_segundo_op);
	codetests_assembler_print("PUSH   AF",buf_opcode,buf_primer_op,buf_segundo_op);

	asm_return_op_ops("EX     DE,HL   ",buf_opcode,buf_primer_op,buf_segundo_op);
	codetests_assembler_print("EX     DE,HL   ",buf_opcode,buf_primer_op,buf_segundo_op);*/


	printf ("Assembling\n");

	//int assemble_opcode(char *texto,z80_byte *destino)
	z80_byte destino_ensamblado[MAX_DESTINO_ENSAMBLADO];


/*	codetests_assemble_opcode("NOP",destino_ensamblado);

	codetests_assemble_opcode("NOP 33",destino_ensamblado);

	codetests_assemble_opcode("LD A,2",destino_ensamblado);
	codetests_assemble_opcode("LD B,B",destino_ensamblado);
	codetests_assemble_opcode("LD D,A",destino_ensamblado);
	codetests_assemble_opcode("LD (HL),(HL)",destino_ensamblado); //TODO Incorrecto. debe salir error

	codetests_assemble_opcode("LD BC,2",destino_ensamblado);
	codetests_assemble_opcode("LD HL,260",destino_ensamblado);
	codetests_assemble_opcode("LD IX,260",destino_ensamblado);
	codetests_assemble_opcode("LD IY,260",destino_ensamblado);

	codetests_assemble_opcode("EXX",destino_ensamblado);
	codetests_assemble_opcode("EX AF,AF'",destino_ensamblado);
	codetests_assemble_opcode("EX AF,BC",destino_ensamblado);
	codetests_assemble_opcode("PUSH DE",destino_ensamblado);
	codetests_assemble_opcode("PUSH DE,AF",destino_ensamblado);
*/

	//Prueba ensamblando todas instrucciones

	//Primero sin opcode
	int i;

	//z80_byte origen_ensamblado[256];


	char texto_desensamblado[256];

	int paso_prefijo;

	for (paso_prefijo=0;paso_prefijo<7;paso_prefijo++) { //Si prefijo, 221, 253, 237, 203, 221+203, 253+203
		if (paso_prefijo) {
			printf ("Paso prefijo %d\n----------------\n\n",paso_prefijo);
		}
	for (i=0;i<256;i++) {
		//Primero metemos 4 bytes y desensamblamos
		//Evitar opcode prefijos
		if (i==203 || i==221 || i==237 || i==253) continue;

		//Metemos el primer byte con ese valor y 3 mas de relleno
		int inicio_array=0;

		if (paso_prefijo==1) {
			disassemble_array[inicio_array]=221;
			inicio_array++;
		}

		if (paso_prefijo==2) {
			disassemble_array[inicio_array]=253;
			inicio_array++;
		}

		if (paso_prefijo==3) {
			disassemble_array[inicio_array]=237;
			inicio_array++;
		}

		if (paso_prefijo==4) {
			disassemble_array[inicio_array]=203;
			inicio_array++;
		}

		if (paso_prefijo==5) {
			disassemble_array[inicio_array]=221;
			disassemble_array[inicio_array+1]=203;
			inicio_array+=2;
		}

		if (paso_prefijo==6) {
			disassemble_array[inicio_array]=253;
			disassemble_array[inicio_array+1]=203;
			inicio_array+=2;
		}



		if (paso_prefijo==5 || paso_prefijo==6) {
			disassemble_array[inicio_array]=0; //desplazamiento
			disassemble_array[inicio_array+1]=i; //instruccion
			disassemble_array[inicio_array+2]=0; //0x6e;
			disassemble_array[inicio_array+3]=0; //0xab;
		}

		else {
			disassemble_array[inicio_array]=i;

			disassemble_array[inicio_array+1]=0; //0x3e;
			disassemble_array[inicio_array+2]=0; //0x6e;
			disassemble_array[inicio_array+3]=0; //0xab;
		}

		//Desensamblamos
		size_t longitud_opcode_desensamblado;
		debugger_disassemble_array(texto_desensamblado,255,&longitud_opcode_desensamblado,0);

		//printf ("Ensamblando Opcode %d : %s\n",i,texto_desensamblado);

		//Evitar nop con prefijo
		if (paso_prefijo && !strcasecmp(texto_desensamblado,"NOP")) continue;
		//Evitar nop con prefijo
		if (paso_prefijo && !strcasecmp(texto_desensamblado,"NOPD")) continue;

		//Evitar segundo y demas neg
		if (paso_prefijo && !strcasecmp(texto_desensamblado,"NEG") && i>=76) continue;

		//Evitar segundo im0
		if (paso_prefijo && !strcasecmp(texto_desensamblado,"IM 0") && i==78) continue;

		//Evitar segundo retn y demas
		if (paso_prefijo && !strcasecmp(texto_desensamblado,"RETN") && i>=85) continue;

		//Evitar segundo LD (NN),HL
		if (paso_prefijo && !strcasecmp(texto_desensamblado,"LD (NNNN),HL") && i==99) continue;

		//Evitar instruccion sin IX o IY (Ejemplo: DD + LD BC,NNNNN)
		if (paso_prefijo && paso_prefijo<=2 && !strstr(texto_desensamblado,"IX") && !strstr(texto_desensamblado,"IY")) continue;

		//Evitar im0,1,2 >=102, que estan repetidos
		if (paso_prefijo && !strcasecmp(texto_desensamblado,"IM 0") && (i==102 || i==110)) continue;
		if (paso_prefijo && !strcasecmp(texto_desensamblado,"IM 1") && (i==118)) continue;
		if (paso_prefijo && !strcasecmp(texto_desensamblado,"IM 2") && (i==126)) continue;

		//Evitar segundo LD HL,(NN)
		if (paso_prefijo && !strcasecmp(texto_desensamblado,"LD HL,(NNNN)") && i==107) continue;

		//Evitar de momento instrucciones "raras" dd/fd+203 < 64
		//if ( (paso_prefijo==5 || paso_prefijo==6) && i<255) continue;

		//Las otras raras de DD/FD+CB. Solo las que tienen 3 bits mas bajos a 6
		if (paso_prefijo==5 || paso_prefijo==6) {
			//printf ("%d\n",i&7);
			if ((i&7)!=6) continue;
		}




		//Ensamblar
		printf ("Ensamblando Opcode %d : %s\n",i,texto_desensamblado);
		int direccion_destino=16384;

		//Casos especiales
		if (!strcmp(texto_desensamblado,"DJNZ NNNN")) {
			disassemble_array[inicio_array+1]=3;
			strcpy(texto_desensamblado,"DJNZ 16389"); //16384+2+3;
		}

		if (!strcmp(texto_desensamblado,"JR NNNN")) {
			disassemble_array[inicio_array+1]=256-4;
			strcpy(texto_desensamblado,"JR 16382"); //16384+2-4;
		}

		if (!strcmp(texto_desensamblado,"JR NZ,NNNN")) {
			disassemble_array[inicio_array+1]=256-4;
			strcpy(texto_desensamblado,"JR NZ,16382"); //16384+2-4;
		}

		if (!strcmp(texto_desensamblado,"JR Z,NNNN")) {
			disassemble_array[inicio_array+1]=256-4;
			strcpy(texto_desensamblado,"JR Z,16382"); //16384+2-4;
		}

		if (!strcmp(texto_desensamblado,"JR NC,NNNN")) {
			disassemble_array[inicio_array+1]=3;
			strcpy(texto_desensamblado,"JR NC,16389"); //16384+2+3;
		}

		if (!strcmp(texto_desensamblado,"JR C,NNNN")) {
			disassemble_array[inicio_array+1]=3;
			strcpy(texto_desensamblado,"JR C,16389"); //16384+2+3;
		}

		if (!strcmp(texto_desensamblado,"LD (NNNN),HL")) {
			disassemble_array[inicio_array+1]=4;
			disassemble_array[inicio_array+2]=64;
			strcpy(texto_desensamblado,"LD (16388),HL");
		}

		if (strstr(texto_desensamblado,"RST")) { //al desensamblar lo mete como valor hexadecimal (RST 16->RST 10)
			int rstvalor=((i>>3) & 7)*8;
			sprintf(texto_desensamblado,"RST %d",rstvalor);
		}

		int longitud_destino=assemble_opcode(direccion_destino,texto_desensamblado,destino_ensamblado);

		if (longitud_destino==0) {
			printf ("Error longitud=0\n");
			return;
		}

		else if (longitud_destino!=(int)longitud_opcode_desensamblado) {
			printf ("Sizes do not match\n");
		}

		else {
			printf ("OK. Dump original and destination:\n");
			int j;
			for (j=0;j<(int)longitud_opcode_desensamblado;j++) {
				z80_byte byte_origen=disassemble_array[j];
				z80_byte byte_destino=destino_ensamblado[j];
				printf ("orig: %02XH dest: %02XH .  ",byte_origen,byte_destino);
				if (byte_origen!=byte_destino) {
					printf ("\nDo not match bytes\n");
					return;
				}
			}

			printf ("\n");
		}


		//usleep(50000);
	}
	}

	printf ("Assemble tests OK\n");


}

int codetests_expression_parser_print_tokens(token_parser *tokens)
{
	exp_par_debug_dump_tokens(tokens,MAX_PARSER_TOKENS_NUM);

	printf ("**text from tokens: \n");
	char buffer_destino[1024];

	exp_par_tokens_to_exp(tokens,buffer_destino,MAX_PARSER_TOKENS_NUM);
	printf ("[%s]\n\n",buffer_destino);

	printf ("Resultado expresion tokens\n");
	int error_code;
	int resultado=exp_par_evaluate_token(tokens,MAX_PARSER_TOKENS_NUM,&error_code);
	printf ("%d\n",resultado);
	return resultado;
}

void codetests_expression_parser_expect(char *string,int expected_value)
{

	printf ("\n\n\n*****Text [%s] expect to be [%d]\n",string,expected_value);

	//Mis tokens de salida
	token_parser tokens[MAX_PARSER_TOKENS_NUM];
	int result;
	int resultado_evaluar;

	printf ("\nText to token: %s\n",string);
	result=exp_par_exp_to_tokens(string,tokens);
	printf ("result token: %d\n",result);
	if (result>=0) resultado_evaluar=codetests_expression_parser_print_tokens(tokens);
	else {
		printf ("ERROR!\n");
		exit(1);
	}

	if (resultado_evaluar!=expected_value) {
		printf ("*****ERROR text [%s] is NOT [%d]. IS [%d]\n",string,expected_value,resultado_evaluar);
		exit(1);
	}
	else {
		printf ("*****OK text [%s] is [%d]\n",string,expected_value);
	}


}

z80_byte codetests_expression_parser_peek_byte_no_time(z80_int dir)
{
	//para testeo
	//devolver nibble bajo de direccion
	return dir & 0xFF;
}

void codetests_expression_parser(void)
{
	//void exp_par_exp_to_tokens(char *expression,token_parser *tokens)
	//algunas inicializaciones de registros
	current_machine_type=MACHINE_ID_SPECTRUM_48;
	reg_a=45;
	reg_bc=40000;
	reg_de=30000;
	peek_byte_no_time=codetests_expression_parser_peek_byte_no_time;

	//Basicos
	codetests_expression_parser_expect("0",0);
	codetests_expression_parser_expect("1",1);

	//Diferentes bases
	codetests_expression_parser_expect("0%",0);
	codetests_expression_parser_expect("1%",1);

	codetests_expression_parser_expect("0H",0);
	codetests_expression_parser_expect("1H",1);

	codetests_expression_parser_expect("101%",5);
	codetests_expression_parser_expect("AAB0H",0xaab0);
	codetests_expression_parser_expect("\"A\"",'A');
	codetests_expression_parser_expect("'b'",'b');

	codetests_expression_parser_expect("1111111111111111111111111111111%",2147483647);

	codetests_expression_parser_expect("11111111111111111111111111111111%",(unsigned int)4294967295);

	codetests_expression_parser_expect("110%+AAH+'c'+33",6+0xaa+'c'+33);

	//Unos cuantos de numeros negativos con signo
	codetests_expression_parser_expect("-2",-2);
	codetests_expression_parser_expect("-2+5",3);
	codetests_expression_parser_expect("-2-6",-8);
	codetests_expression_parser_expect("10+(-3)",7);
	codetests_expression_parser_expect("10-(-3)",13);

	//esta sintacticamente no deberia ser posible, pero lo es
	codetests_expression_parser_expect("10--3",13);

	codetests_expression_parser_expect("10-3",7);

	//Unos cuantos de numeros positivos con signo
	codetests_expression_parser_expect("+2",+2);
	codetests_expression_parser_expect("+2+5",7);
	codetests_expression_parser_expect("+2-6",-4);
	codetests_expression_parser_expect("10+(+3)",13);
	codetests_expression_parser_expect("10-(+3)",7);

	//esta sintacticamente no deberia ser posible, pero lo es
	codetests_expression_parser_expect("10++3",13);

	codetests_expression_parser_expect("10+3",13);



	//Sumas, restas, multiplicaciones y divisiones
	codetests_expression_parser_expect("1+1",2);
	codetests_expression_parser_expect("2*3",6);
	codetests_expression_parser_expect("2*3+1",7);
	codetests_expression_parser_expect("1+2*3",7);


	codetests_expression_parser_expect("10/2",5);
	codetests_expression_parser_expect("4+10/2",9);

	//Comparadores
	codetests_expression_parser_expect("1=1",1);
	codetests_expression_parser_expect("1<2",1);
	codetests_expression_parser_expect("20>1",1);

	codetests_expression_parser_expect("1<=2",1);
	codetests_expression_parser_expect("20>=1",1);

	codetests_expression_parser_expect("20>=20",1);
	codetests_expression_parser_expect("20<=20",1);


	codetests_expression_parser_expect("(6-20)+2",-12);

	//Este no lo pasa, debido a que acaba calculando: 6 -    20+2  ->   6-   22 -> -16
	//codetests_expression_parser_expect("6-20+2",-12);

	//Este tampoco lo pasa, acaba calculando: 10 -      1-1 -> 10 - 0 -> 10
	//codetests_expression_parser_expect("10-1-1",8);


	codetests_expression_parser_expect("(10-1)-1",8);

	//Operaciones mas complejas
	codetests_expression_parser_expect("3*(6+7)",39);
	codetests_expression_parser_expect("3*(6+7)+4",43);
	codetests_expression_parser_expect("3*(6+7)+4=43",1);
	codetests_expression_parser_expect("3*(6+7)+4=99",0);

	codetests_expression_parser_expect("3*[6+7]+4=99",0);
	codetests_expression_parser_expect("3*{6+7}+4=99",0);


	codetests_expression_parser_expect("(((((5)))))",5);
    codetests_expression_parser_expect("(( ((2)) + 4))*((5))",30);


	//Este no lo pasa, debido a que acaba calculando:   6    -    20+2  -> 6-22 -> -16
    //codetests_expression_parser_expect(" 2*3 - 4*5 + 6/3 ",-12);


	codetests_expression_parser_expect("( 2*3 - 4*5  ) + 6/3 ",-12);


	codetests_expression_parser_expect("A",45);
	codetests_expression_parser_expect("AH",10);

	codetests_expression_parser_expect("NOT(1)",0);
	codetests_expression_parser_expect("NOT(0)",1);

	codetests_expression_parser_expect("BC+DE",70000);

	codetests_expression_parser_expect("(BC+DE)",70000);

	codetests_expression_parser_expect("(BC+DE)&FFFFH",4464);


	codetests_expression_parser_expect("3*((BC+DE)&FFFFH)",3*4464);
	codetests_expression_parser_expect("3*((BC+DE)&FFFFH)=(2+1)*((BC+DE)&FFFFH)",1);


	codetests_expression_parser_expect("3*((BC+DE)&FFFFH)=(2+1)*((BC+DE)&FFFFH) AND (1=1)",1);
	codetests_expression_parser_expect("3*((BC+DE)&FFFFH)=(2+1)*((BC+DE)&FFFFH) AND (bc=de+10000)",1);
	codetests_expression_parser_expect("3*((BC+DE)&FFFFH)=(2+1)*((BC+DE)&FFFFH) AND (bc=de+10001)",0);
	codetests_expression_parser_expect("3*((BC+DE)&FFFFH)=(2+1)*((BC+DE)&FFFFH) AND (bc=de+10000) AND (bc-de=10000)",1);
	codetests_expression_parser_expect("3*((BC+DE)&FFFFH)=(2+1)*((BC+DE)&FFFFH) AND (bc=de+10000) AND (bc-de=10001)",0);


	codetests_expression_parser_expect("3*((BC+DE)&FFFFH)=(2+1)*((BC+DE)&FFFFH) AND bc=de+10000 AND bc-de=10000",1);
	codetests_expression_parser_expect("3  * ( (  BC +  DE ) &  FFFFH  )=( 2 + 1  )  *  (  (  BC  +  DE  )  &  FFFFH) AND    bc   =   de +  10000 AND bc - de =  10000",1);


	codetests_expression_parser_expect("3  * ( (  BC +  DE ) &  FFFFH  )",3*4464);

	codetests_expression_parser_expect("3*((BC+DE)&FFFFH)=(999999+1)*((BC+DE)&FFFFH) OR (bc=de+10000)",1);
	codetests_expression_parser_expect("3*((BC+DE)&FFFFH)=(999999+1)*((BC+DE)&FFFFH) OR (bc=de+9999)",0);

	codetests_expression_parser_expect("NOT(1)",0);
	codetests_expression_parser_expect("NOT(NOT(1))",1);

	codetests_expression_parser_expect("NOT(3*((BC+DE)&FFFFH))",0);



	codetests_expression_parser_expect("NOT(NOT(3*((BC+DE)&FFFFH)))",1);


	codetests_expression_parser_expect("peek(FF01H)",1);
	codetests_expression_parser_expect("3*peek(FF02H)",6);
	codetests_expression_parser_expect("3*peek(FF02H)+NOT(0)",7);


	codetests_expression_parser_expect("2+(      peek(FF02H)     &    2    )",4);


	codetests_expression_parser_expect("2+(      peek(FF02H+33)     &    7    )",2+3);


	codetests_expression_parser_expect("2+(      peek(    FF02H   )      &    2    )",4);


	codetests_expression_parser_expect("A*BC",1800000);
	codetests_expression_parser_expect("A*BC+3",1800000+3);
	codetests_expression_parser_expect("99*(A*BC+3)",99*(1800000+3));
	codetests_expression_parser_expect("(99*(A*BC+3)) & FFFFH",7913);

	printf ("\nOK ALL expression parser TESTS OK\n");

}


void codetests_mid_test(void)
{

	//Prueba mid
	z80_byte midi_buffer[2048];

	//Metemos cabecera bloque
	int indice=0;

	int division=50;
	int pistas=2;

	//Cabecera archivo
	indice +=mid_mete_cabecera(&midi_buffer[indice],pistas,division);


	int inicio_pista;
	int canal;
	int longitud_pista;



	//Inicio pista 0
	inicio_pista=indice;

	indice +=mid_mete_inicio_pista(&midi_buffer[indice],division);

	canal=0;

	//Nota
	indice +=mid_mete_nota(&midi_buffer[indice],0,division,canal,60,0x40);
	indice +=mid_mete_nota(&midi_buffer[indice],0,division,canal,61,0x40);
	indice +=mid_mete_nota(&midi_buffer[indice],0,division,canal,62,0x40);

	//Final de pista
	indice +=mid_mete_evento_final_pista(&midi_buffer[indice]);

	//Indicar longitud de pista
	longitud_pista=indice-inicio_pista;

	mid_mete_longitud_pista(&midi_buffer[inicio_pista],longitud_pista);



	//Inicio pista 1
	inicio_pista=indice;

	indice +=mid_mete_inicio_pista(&midi_buffer[indice],division);

	canal=1;

	//Nota
	indice +=mid_mete_nota(&midi_buffer[indice],0,division,canal,63,0x40);
	indice +=mid_mete_nota(&midi_buffer[indice],0,division,canal,64,0x40);
	indice +=mid_mete_nota(&midi_buffer[indice],50,division,canal,65,0x40);

	//Final de pista
	indice +=mid_mete_evento_final_pista(&midi_buffer[indice]);

	//Indicar longitud de pista
	longitud_pista=indice-inicio_pista;

	mid_mete_longitud_pista(&midi_buffer[inicio_pista],longitud_pista);



	//Grabar a disco
FILE *ptr_midfile;

     ptr_midfile=fopen("salida.mid","wb");
     if (!ptr_midfile) {
                        printf("can not write midi file\n");
                        return;
      }

    fwrite(midi_buffer, 1, indice, ptr_midfile);


      fclose(ptr_midfile);
}

void codetests_zeng(void)
{
	zeng_key_presses elemento;

	int i;
	int tecla=0;

	for (i=0;i<ZENG_FIFO_SIZE;i++) {
		elemento.tecla=tecla;
		elemento.pressrelease=1;

		if (zeng_fifo_add_element(&elemento)) {
			printf ("Error adding zeng\n");
			exit(1);
		}

		tecla++;
	}

	//Elemento adicional que no cabe
	if (!zeng_fifo_add_element(&elemento)) {
			printf ("Error adding zeng. fifo full but does not warn\n");
			exit(1);
	}

	//Quitar dos
	zeng_fifo_read_element(&elemento);
	zeng_fifo_read_element(&elemento);

	//Leer elementos
	for (i=0;i<ZENG_FIFO_SIZE-2;i++) {


		if (zeng_fifo_read_element(&elemento)) {
			printf ("Error reading zeng\n");
			exit(1);
		}

		printf ("Element %d tecla %d pressrelease %d\n",i,elemento.tecla,elemento.pressrelease);
	}

	//Agregar dos
	elemento.tecla=100;
	elemento.pressrelease=0;
	zeng_fifo_add_element(&elemento);

	elemento.tecla=200;
	elemento.pressrelease=1;
	zeng_fifo_add_element(&elemento);

	//Leer elementos
	for (i=0;i<2;i++) {


		if (zeng_fifo_read_element(&elemento)) {
			printf ("Error reading zeng\n");
			exit(1);
		}

		printf ("Element %d tecla %d pressrelease %d\n",i,elemento.tecla,elemento.pressrelease);
	}

}


void codetests_https()
{
	//http://www.zx81.nl/files.html
	int http_code;
	char *mem;
	char *orig_mem;
	char *mem_after_headers;
	int total_leidos;
	//int retorno=zsock_http("www.google.es","/",&http_code,&mem,&total_leidos,&mem_after_headers,0,"",1);

	char redirect_url[NETWORK_MAX_URL];

	int retorno=zsock_http("archive.org","/download/World_of_Spectrum_June_2017_Mirror/World%20of%20Spectrum%20June%202017%20Mirror.zip/World%20of%20Spectrum%20June%202017%20Mirror/sinclair/games/m/Mandroid.tzx.zip",
				&http_code,&mem,&total_leidos,&mem_after_headers,0,"",1,redirect_url,0,"");

	if (retorno<0) {
		printf ("Error zsock_http\n");
		exit(1);
	}

	orig_mem=mem;

	if (retorno==0 && mem!=NULL) printf ("Response\n%s\n",mem);

	//leer linea a linea hasta fin cabecera
	char buffer_linea[1024];
	int i=0;
	int salir=0;
	do {
		int leidos;
		char *next_mem;
		if (*mem=='\n') {
			//esto puede que no pase, linea con solo salto linea tendra un cr antes,
			//por tanto la deteccion de esa linea se leera abajom cuando buffer linea vacia
			salir=1;
			mem++;
			printf ("salir con salto linea inicial\n");
		}
		else {
			next_mem=util_read_line(mem,buffer_linea,total_leidos,1024,&leidos);
			total_leidos -=leidos;

			if (buffer_linea[0]==0) {
				salir=1;
				printf ("salir con linea vacia final\n");
				mem=next_mem;
			}
			else {
				printf ("cabecera %d: %s\n",i,buffer_linea);
				i++;
				mem=next_mem;
			}

			if (total_leidos<=0) salir=1;
		}
	} while (!salir);

	printf ("respuesta despues cabeceras:\n%s\n",mem);


	if (orig_mem!=NULL) free (orig_mem);

	//peticion saltando cabeceras
	//printf ("Request skipping headers\n");
	//retorno=zsock_http("www.google.es","/",&http_code,&mem,&total_leidos,&mem_after_headers,1,"",1);
	//if (mem_after_headers!=NULL) printf ("Answer after headers:\n%s\n",mem_after_headers);

	//if (mem!=NULL) free (mem);

}


void codetests_https_sni()
{
	//http://www.zx81.nl/files.html
	int http_code;
	char *mem;
	char *orig_mem;
	char *mem_after_headers;
	int total_leidos;
	//int retorno=zsock_http("www.google.es","/",&http_code,&mem,&total_leidos,&mem_after_headers,0,"",1);

	char redirect_url[NETWORK_MAX_URL];

	int retorno=zsock_http("spectrumcomputing.co.uk","/index.php",
				&http_code,&mem,&total_leidos,&mem_after_headers,0,"",1,redirect_url,0,"spectrumcomputing.co.uk");

	if (retorno<0) {
		printf ("Error zsock_http\n");
		exit(1);
	}

	orig_mem=mem;

	if (retorno==0 && mem!=NULL) printf ("Response\n%s\n",mem);

	//leer linea a linea hasta fin cabecera
	char buffer_linea[1024];
	int i=0;
	int salir=0;
	do {
		int leidos;
		char *next_mem;
		if (*mem=='\n') {
			//esto puede que no pase, linea con solo salto linea tendra un cr antes,
			//por tanto la deteccion de esa linea se leera abajom cuando buffer linea vacia
			salir=1;
			mem++;
			printf ("salir con salto linea inicial\n");
		}
		else {
			next_mem=util_read_line(mem,buffer_linea,total_leidos,1024,&leidos);
			total_leidos -=leidos;

			if (buffer_linea[0]==0) {
				salir=1;
				printf ("salir con linea vacia final\n");
				mem=next_mem;
			}
			else {
				printf ("cabecera %d: %s\n",i,buffer_linea);
				i++;
				mem=next_mem;
			}

			if (total_leidos<=0) salir=1;
		}
	} while (!salir);

	printf ("respuesta despues cabeceras:\n%s\n",mem);


	if (orig_mem!=NULL) free (orig_mem);

	//peticion saltando cabeceras
	//printf ("Request skipping headers\n");
	//retorno=zsock_http("www.google.es","/",&http_code,&mem,&total_leidos,&mem_after_headers,1,"",1);
	//if (mem_after_headers!=NULL) printf ("Answer after headers:\n%s\n",mem_after_headers);

	//if (mem!=NULL) free (mem);

}


void codetests_http()
{
	//http://www.zx81.nl/files.html
	int http_code;
	char *mem;
	char *orig_mem;
	char *mem_after_headers;
	int total_leidos;
	char redirect_url[NETWORK_MAX_URL];
	int retorno=zsock_http("www.zx81.nl","/files.html",&http_code,&mem,&total_leidos,&mem_after_headers,0,"",0,redirect_url,0,"");
	orig_mem=mem;

	if (retorno==0 && mem!=NULL) printf ("Response\n%s\n",mem);

	//leer linea a linea hasta fin cabecera
	char buffer_linea[1024];
	int i=0;
	int salir=0;
	do {
		int leidos;
		char *next_mem;
		if (*mem=='\n') {
			//esto puede que no pase, linea con solo salto linea tendra un cr antes,
			//por tanto la deteccion de esa linea se leera abajom cuando buffer linea vacia
			salir=1;
			mem++;
			printf ("salir con salto linea inicial\n");
		}
		else {
			next_mem=util_read_line(mem,buffer_linea,total_leidos,1024,&leidos);
			total_leidos -=leidos;

			if (buffer_linea[0]==0) {
				salir=1;
				printf ("salir con linea vacia final\n");
				mem=next_mem;
			}
			else {
				printf ("cabecera %d: %s\n",i,buffer_linea);
				i++;
				mem=next_mem;
			}

			if (total_leidos<=0) salir=1;
		}
	} while (!salir);

	printf ("respuesta despues cabeceras:\n%s\n",mem);


	if (orig_mem!=NULL) free (orig_mem);

	//peticion saltando cabeceras
	printf ("Request skipping headers\n");

	retorno=zsock_http("www.zx81.nl","/files.html",&http_code,&mem,&total_leidos,&mem_after_headers,1,"",0,redirect_url,0,"");
	if (mem_after_headers!=NULL) printf ("Answer after headers:\n%s\n",mem_after_headers);

	if (mem!=NULL) free (mem);

}

void codetests_messages_debug(char *s)
{
        printf ("%s\n",s);
		fflush(stdout);
}


#ifdef USE_PTHREADS

pthread_t thread_codetests;
z_atomic_semaphore codetest_semaforo;

void codetests_simple_atomic(void)
{

        z_atomic_reset(&codetest_semaforo);
        printf("Semaforo despues reset: %d\n",codetest_semaforo);


		while(z_atomic_test_and_set(&codetest_semaforo)) {
			printf ("Esperando a adquirir lock en secondary pthread\n");
		}

        printf("Semaforo despues set: %d\n",codetest_semaforo);


		z_atomic_reset(&codetest_semaforo);
        printf("Semaforo despues reset: %d\n",codetest_semaforo);
}



void *thread_codetests_function(void *nada GCC_UNUSED)
{
	while (1) {
		//Adquirir lock
		/*while(z_atomic_test_and_set(&codetest_semaforo)) {
			printf ("  Esperando a adquirir lock en secondary pthread\n");
		}*/


		//printf("Message from secondary pthread\n");
		debug_printf(VERBOSE_DEBUG,"Message from secondary pthread\n");
		usleep(1000);
		//printf ("hola\n");

		//Liberar lock
		z_atomic_reset(&codetest_semaforo);


		//Pausa de test
		usleep(1000);
	}
}



void codetests_atomic(void)
{

    z_atomic_reset(&codetest_semaforo);
	scr_messages_debug=codetests_messages_debug;
	verbose_level=VERBOSE_PARANOID;
	scr_set_driver_name("");


		//Inicializar thread

	if (pthread_create( &thread_codetests, NULL, &thread_codetests_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create codetests pthread");
		exit(1);
	}


	//Empezar a escribir debug info en este pthread y en el otro
	while (1) {
		//Adquirir lock
		/*while(z_atomic_test_and_set(&codetest_semaforo)) {
			printf ("  Esperando a adquirir lock en primary pthread\n");
		}*/


		//printf("Message from primary pthread\n");
		debug_printf(VERBOSE_DEBUG,"Message from primary pthread\n");
		usleep(1000);
		//printf ("hola\n");

		//Liberar lock
		z_atomic_reset(&codetest_semaforo);


		//Pausa de test
		usleep(1000);
	}

}


pthread_t pthread_zengonline_put_snapshot_thread;
pthread_t pthread_zengonline_get_snapshot_thread;

    char  *codetest_putsnap_string_mysnap1="Hola que tal";
    char  *codetest_putsnap_string_mysnap2="Yo muy bien";
    char  *codetest_putsnap_string_mysnap3="Y tu como vas";

void *thread_codetests_putsnap_function(void *nada GCC_UNUSED)
{
    //printf("Put snap thread\n");



    while (1) {
        printf("put snapshot 1\n");
        zengonline_put_snapshot(0,(z80_byte *)codetest_putsnap_string_mysnap1,strlen(codetest_putsnap_string_mysnap1)+1);

        printf("put snapshot 2\n");
        zengonline_put_snapshot(0,(z80_byte *)codetest_putsnap_string_mysnap2,strlen(codetest_putsnap_string_mysnap2)+1);

        printf("put snapshot 3\n");
        zengonline_put_snapshot(0,(z80_byte *)codetest_putsnap_string_mysnap3,strlen(codetest_putsnap_string_mysnap3)+1);
    }
    return NULL;
}

void *thread_codetests_getsnap_function(void *nada GCC_UNUSED)
{
    //printf("Get snap thread\n");

    char buffer_get_snap[1024];

    while (1) {
        zengonline_get_snapshot(0,(z80_byte *)buffer_get_snap);
        //printf("Desde un lector thread, snapshot leido: %s\n",buffer_get_snap);

        //Tiene que coincidir con alguna de las 3 strings
        if (
            !strcmp(buffer_get_snap,codetest_putsnap_string_mysnap1) ||
            !strcmp(buffer_get_snap,codetest_putsnap_string_mysnap2) ||
            !strcmp(buffer_get_snap,codetest_putsnap_string_mysnap3)
        )
        {
            //coincide
        }
        else {
            printf("Snapshot no es el esperado, leido (entre corchetes): [%s]\n",buffer_get_snap);
            printf("Error!\n");
            exit(1);
        }
    }
    return NULL;
}

void codetests_zengonline_putget_snapshot(void)
{

    //printf("Wait 10 seconds..\n");

	if (pthread_create( &pthread_zengonline_put_snapshot_thread, NULL, &thread_codetests_putsnap_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create pthread_zengonline_put_snapshot_thread");
		exit(1);
	}
    //Esperar 1 segundo a que se genere al menos 1 snapshot
    sleep(1);



    //muchos leyendo
    int i;
    for (i=0;i<100;i++) {
        if (pthread_create( &pthread_zengonline_get_snapshot_thread, NULL, &thread_codetests_getsnap_function, NULL) ) {
            debug_printf(VERBOSE_ERR,"Can not create pthread_zengonline_get_snapshot_thread");
            exit(1);
        }

    }

    printf("Created %d threads reading, 1 writing. Wait 10 seconds...\n",i);

    sleep(10);
}

void *thread_codetests_network_function(void *nada GCC_UNUSED)
{
	while (1) {
		printf ("Abriendo conexion desde thread secundario\n");
		fflush(stdout);
		int sock=z_sock_open_connection("google.es",80,0,"");
		printf ("socket para thread secundario: %d\n",sock);
		fflush(stdout);

		usleep(200000);
		if (sock>=0) {
			printf ("cerrando socket %d\n",sock);
			fflush(stdout);
			z_sock_close_connection(sock);
		}
		else {
			printf ("socket no abierto en thread secundario\n");
			fflush(stdout);
		}


	}
}


pthread_t thread_network_codetests;

void codetests_network_atomic(void)
{
		//Inicializar thread

	if (pthread_create( &thread_network_codetests, NULL, &thread_codetests_network_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create codetests network pthread");
		exit(1);
	}

	scr_messages_debug=codetests_messages_debug;
	verbose_level=VERBOSE_PARANOID;
	scr_set_driver_name("");

	//Empezar a abrir conexiones tcpen este pthread y en el otro
	while (1) {

		printf ("Abriendo conexion desde thread primario\n");
		fflush(stdout);
		int sock=z_sock_open_connection("google.es",80,0,"");
        //int sock=z_sock_open_connection("spectrumcomputing.co.uk",443,1,"spectrumcomputing.co.uk");
		printf ("socket para thread primario: %d\n",sock);
		fflush(stdout);

		usleep(200000);
		if (sock>=0) {
			printf ("cerrando socket %d\n",sock);
			fflush(stdout);
			z_sock_close_connection(sock);
		}
		else {
			printf ("socket no abierto en thread primario\n");
			fflush(stdout);
		}

	}

}

void codetests_open_sockets_infinite(void)
{
	int i;

	for (i=0;i<9999;i++) {
		printf ("Creating socket %d\n",i);
		int socket=z_sock_assign_socket();
		if (socket<0) {
			printf ("Error: %s\n",z_sock_get_error(socket));
			return;
		}
	}
}


#else
//Funcion vacia para compilacion en entornos sin threads
void codetests_zengonline_putget_snapshot(void)
{
}


#endif


/*
void codetests_get_background_f_key(void)
{


	int i;

	for (i=1;i<=10;i++) {


		z80_byte *puntero;
		int mascara;

		puntero=menu_get_port_puerto_especial(i);
		mascara=menu_get_mask_puerto_especial(i);


		printf ("puerto: %p mascara: %d\n",puntero,mascara);
	}
}
*/

void codetests_tbblue_set_ram_blocks(void)
{

	int i;

	for (i=0;i<3000;i++) {
		tbblue_set_ram_blocks(i);
		printf ("ram: %04dKB blocks: %d\n",i,tbblue_extra_512kb_blocks);
	}

}


/*
extern float aproximate_frequency_from_ql_pitch(int pitch);

void codetests_get_note_table_ql(void)
{
    int i;
    int columna=0;

    for (i=0;i<256;i++) {
        int frecuencia=get_note_frequency_from_ql_pitch(i);
        if (frecuencia==-1) {
            //Obtener mediante aproximacion
            float frecuencia_float=aproximate_frequency_from_ql_pitch(i);
            //printf ("aprox ql pitch: %3d frecuencia: %f\n",i,frecuencia_float);

            //Aproximar frecuencia. Si mayor *.5-> *+1
            int entero_frecuencia=frecuencia_float;
            //printf("entero: %d\n",entero_frecuencia);

            float resta=frecuencia_float-entero_frecuencia;
            if (resta>0.5) entero_frecuencia++;
            //printf("entero rounded: %d\n",entero_frecuencia);

            //printf ("aprox ql pitch: %3d frecuencia: %f rounded: %d\n",i,frecuencia_float,entero_frecuencia);

            printf("%f,",frecuencia_float);
            //printf("%d,",entero_frecuencia);

        }
        else {
            printf ("%d,",frecuencia);
        }

            columna++;

            if (columna==10) {
                printf("\n");
                columna=0;
            }


    }

    printf("\n");

}
*/

void codetests_cosine_table(void)
{
    int i;

    for (i=0;i<360;i++) {
        printf ("cosine %3d %5d\n",i,util_get_cosine(i));
    }

    for (i=0;i<360;i++) {
        printf ("sine %3d %5d\n",i,util_get_sine(i));
    }
}


z80_byte buffer_get_pixel[6912];
void codetests_get_pixel_color_scr(void)
{

    init_screen_addr_table();
    memset(buffer_get_pixel,0,6912);
    estado_parpadeo.v=0;

    int x,y;

    int pasos;

    //Con rutina sin optimizar: 12.5 segundos
    //Con rutina optimizada: 5.7 segundos
    for (pasos=0;pasos<10000;pasos++) {

        for (x=0;x<256;x++) {
            for (y=0;y<192;y++) {
                //printf("%d %d\n",x,y);
                int color=util_get_pixel_color_scr(buffer_get_pixel,x,y);

                //Esto solo para que no se queje el compilador de variable no usada
                color++;
            }
        }

    }

}

char *codetests_scanf_history_array[UTIL_SCANF_HISTORY_MAX_LINES]={
    NULL
};


void codetests_scanf_history(void)
{
    //compruebo que cadena inicial llegue al final, y que luego rote

    int total_elelements=UTIL_SCANF_HISTORY_MAX_LINES-1;

    util_scanf_history_insert(codetests_scanf_history_array,"1234");
    printf("\n");
    util_scanf_history_insert(codetests_scanf_history_array,"4567");
    printf("\n");

    int i;

    //Inserto total_elements-2, asi con los anteriores anterior ya he llenado la lista
    for (i=0;i<total_elelements-2;i++) {
        util_scanf_history_insert(codetests_scanf_history_array,"9876");
        printf("\n");
    }

    //ultima cadena tiene que ser la inicial
    if (strcmp(codetests_scanf_history_array[total_elelements-1],"1234")) {
        printf ("error. last element is not initial\n");
        exit(1);
    }

    //inserto de nuevo. ultimo elemento tiene que ser el segundo
    util_scanf_history_insert(codetests_scanf_history_array,"4444");
    printf("\n");

    if (strcmp(codetests_scanf_history_array[total_elelements-1],"4567")) {
        printf ("error. last element is rotated properly\n");
        exit(1);
    }

    //Y primer elemento tiene que ser el ultimo insertado
    if (strcmp(codetests_scanf_history_array[0],"4444")) {
        printf ("error. first element is not what expected\n");
        exit(1);
    }
}

int codetests_sqrt_aux(int valor)
{
    int result_type;

    int square=util_sqrt(valor,&result_type);
    printf("Square of %d is %d\n",valor,square);

    //Tipo resultado: 0 exacto, 1 aproximado, -1 valor negativo
    if (result_type==0) printf("Exact\n");
    else if (result_type==1) printf("Aproximate\n");
    else if (result_type==-1) printf("Negative error\n");
    printf("\n");

    return square;
}

void codetests_sqrt(void)
{

    //primer mostrar tabla
    int i;

    for (i=0;i<=100;i++) {
        codetests_sqrt_aux(i);
        //printf("Square root of %d: %d\n",i,util_sqrt(i));
    }

    //luego comprobar algunos valores exactos
    int square;

    square=codetests_sqrt_aux(25);
    if (square!=5) {
        printf ("error calculating square root\n");
        exit(1);
    }

    square=codetests_sqrt_aux(144);
    if (square!=12) {
        printf ("error calculating square root\n");
        exit(1);
    }

    square=codetests_sqrt_aux(1089);
    if (square!=33) {
        printf ("error calculating square root\n");
        exit(1);
    }

    //Y una prueba sin parametro de tipo resultado
    square=util_sqrt(10000,NULL);
    printf("Square of 10000 is %d\n",square);
    if (square!=100) {
        printf ("error calculating square root\n");
        exit(1);
    }
    printf("\n");

    //Y prueba con valor negativo
    int result_type;
    square=util_sqrt(-1,&result_type);
    printf("Square of -1 is %d\n",square);
    if (result_type==-1) {
        printf("Ok negative no result\n");
    }
    else {
        printf ("error returning negative square root\n");
        exit(1);
    }


}

void codetests_acosine(void)
{
    int i;

    for (i=0;i<=10000;i+=1000) {
        printf("Acosine of %d: %d\n",i,util_get_acosine(i));
    }

    if (util_get_acosine(8660)!=30) {
        printf ("error calculating acosine\n");
        exit(1);
    }

    if (util_get_acosine(-349)!=92) {
        printf ("error calculating acosine\n");
        exit(1);
    }

}

void codetests_debug_printf_exclude_include(void)
{

    //Preservar settings de filtros

    int antes_debug_mascara_modo_exclude_include=debug_mascara_modo_exclude_include;

    int antes_debug_mascara_clase_exclude=debug_mascara_clase_exclude;

    int antes_debug_mascara_clase_include=debug_mascara_clase_include;


    //Probar exclusiones primero
    debug_mascara_modo_exclude_include=VERBOSE_MASK_CLASS_TYPE_EXCLUDE;

    debug_mascara_clase_exclude=0;

    if (debug_printf_check_exclude_include(VERBOSE_CLASS_ANYTHINGELSE)==0) {
        printf("error exclude anything else when mask=0\n");
        exit(1);
    }
    if (debug_printf_check_exclude_include(VERBOSE_CLASS_DSK)==0) {
        printf("error exclude class dsk when mask=0\n");
        exit(1);
    }

    debug_mascara_clase_exclude=VERBOSE_CLASS_DSK;

    if (debug_printf_check_exclude_include(VERBOSE_CLASS_ANYTHINGELSE)==0) {
        printf("error exclude anything else when mask=dsk\n");
        exit(1);
    }
    if (debug_printf_check_exclude_include(VERBOSE_CLASS_DSK)==1) {
        printf("error exclude class dsk when mask=dsk\n");
        exit(1);
    }

    //Probar inclusiones
    debug_mascara_modo_exclude_include=VERBOSE_MASK_CLASS_TYPE_INCLUDE;

    debug_mascara_clase_include=0;

    if (debug_printf_check_exclude_include(VERBOSE_CLASS_ANYTHINGELSE)==1) {
        printf("error include anything else when mask=0\n");
        exit(1);
    }
    if (debug_printf_check_exclude_include(VERBOSE_CLASS_DSK)==1) {
        printf("error include class dsk when mask=0\n");
        exit(1);
    }

    debug_mascara_clase_include=VERBOSE_CLASS_DSK;

    if (debug_printf_check_exclude_include(VERBOSE_CLASS_ANYTHINGELSE)==1) {
        printf("error include anything else when mask=dsk\n");
        exit(1);
    }
    if (debug_printf_check_exclude_include(VERBOSE_CLASS_DSK)==0) {
        printf("error include class dsk when mask=dsk\n");
        exit(1);
    }

    //Dejamos luego settings iniciales
    debug_mascara_modo_exclude_include=antes_debug_mascara_modo_exclude_include;

    debug_mascara_clase_exclude=antes_debug_mascara_clase_exclude;

    debug_mascara_clase_include=antes_debug_mascara_clase_include;

}

void codetests_tbblue_divmmc_masks(void)
{

    /*
    int i;

    for (i=0;i<8;i++) {

        int direccion=8*i;
        int mascara=tbblue_get_mask_divmmc_entry_point(direccion);
        printf("dir %04XH mask: %02XH\n",direccion,mascara);


        switch (i) {
            case 0:
                if (mascara!=1) { printf("Error\n"); exit(1); }
            break;

            case 1:
                if (mascara!=2) { printf("Error\n"); exit(1); }
            break;

            case 2:
                if (mascara!=4) { printf("Error\n"); exit(1); }
            break;

            case 3:
                if (mascara!=8) { printf("Error\n"); exit(1); }
            break;

            case 4:
                if (mascara!=16) { printf("Error\n"); exit(1); }
            break;

            case 5:
                if (mascara!=32) { printf("Error\n"); exit(1); }
            break;

            case 6:
                if (mascara!=64) { printf("Error\n"); exit(1); }
            break;

            case 7:
                if (mascara!=128) { printf("Error\n"); exit(1); }
            break;

        }
    }

    */
}


void codetests_ay_playlist(void)
{
    ay_player_playlist_init();

    #define CODETESTS_AY_PLAYLIST_ARCHIVO1 "david"
    #define CODETESTS_AY_PLAYLIST_ARCHIVO2 "oscar"
    #define CODETESTS_AY_PLAYLIST_ARCHIVO3 "diego"
    #define CODETESTS_AY_PLAYLIST_ARCHIVO4 "marisa"

    char buffer_temp[PATH_MAX];

    //Add and check
    ay_player_playlist_add(CODETESTS_AY_PLAYLIST_ARCHIVO1);

    ay_player_playlist_get_item(0,buffer_temp);

    printf("Item at position 0: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO1)) {
        printf("Error\n");
        exit(1);
    }

    //Add and check two
    ay_player_playlist_add(CODETESTS_AY_PLAYLIST_ARCHIVO2);

    ay_player_playlist_get_item(0,buffer_temp);

    printf("Item at position 0: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO1)) {
        printf("Error\n");
        exit(1);
    }

    ay_player_playlist_get_item(1,buffer_temp);

    printf("Item at position 1: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO2)) {
        printf("Error\n");
        exit(1);
    }

    //Add and check three
    ay_player_playlist_add(CODETESTS_AY_PLAYLIST_ARCHIVO3);

    ay_player_playlist_get_item(0,buffer_temp);

    printf("Item at position 0: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO1)) {
        printf("Error\n");
        exit(1);
    }

    ay_player_playlist_get_item(1,buffer_temp);

    printf("Item at position 1: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO2)) {
        printf("Error\n");
        exit(1);
    }

    ay_player_playlist_get_item(2,buffer_temp);

    printf("Item at position 2: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO3)) {
        printf("Error\n");
        exit(1);
    }


    //Add and check four
    ay_player_playlist_add(CODETESTS_AY_PLAYLIST_ARCHIVO4);

    ay_player_playlist_get_item(0,buffer_temp);

    printf("Item at position 0: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO1)) {
        printf("Error\n");
        exit(1);
    }

    ay_player_playlist_get_item(1,buffer_temp);

    printf("Item at position 1: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO2)) {
        printf("Error\n");
        exit(1);
    }

    ay_player_playlist_get_item(2,buffer_temp);

    printf("Item at position 2: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO3)) {
        printf("Error\n");
        exit(1);
    }

    ay_player_playlist_get_item(3,buffer_temp);

    printf("Item at position 3: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO4)) {
        printf("Error\n");
        exit(1);
    }


    //Delete first
    printf("Delete pos 0\n");
    ay_player_playlist_remove(0);

    ay_player_playlist_get_item(0,buffer_temp);

    printf("Item at position 0: %s\n",buffer_temp);

    //Tiene que ser el segundo que habia
    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO2)) {
        printf("Error\n");
        exit(1);
    }

    //Delete second
    printf("Delete pos 1\n");
    ay_player_playlist_remove(1);

    //El primero era el mismo de antes
    ay_player_playlist_get_item(0,buffer_temp);

    printf("Item at position 0: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO2)) {
        printf("Error\n");
        exit(1);
    }

    //El segundo pasara a ser el cuarto
    ay_player_playlist_get_item(1,buffer_temp);

    printf("Item at position 1: %s\n",buffer_temp);

    //Tiene que ser el segundo que habia
    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO4)) {
        printf("Error\n");
        exit(1);
    }

    //Delete second (and the last)
    printf("Delete pos 1\n");
    ay_player_playlist_remove(1);

    //El primero era el mismo de antes
    ay_player_playlist_get_item(0,buffer_temp);

    printf("Item at position 0: %s\n",buffer_temp);

    if (strcmp(buffer_temp,CODETESTS_AY_PLAYLIST_ARCHIVO2)) {
        printf("Error\n");
        exit(1);
    }


    printf("Delete pos 0\n");
    ay_player_playlist_remove(0);

    int total=ay_player_playlist_get_total_elements();

    printf("Total: %d\n",total);

    if (total!=0) {
        printf("Error getting total\n");
        exit(1);
    }

    //Agregar 10000 en bucle y comprobarlos
    int i;

    printf("Inserting 10000...\n");
    for (i=0;i<10000;i++) {
        sprintf(buffer_temp,"file%d",i);
        ay_player_playlist_add(buffer_temp);
    }

    printf("Checking 10000...\n");
    //Comprobarlos
    for (i=0;i<10000;i++) {
        char buffer_item_actual[PATH_MAX];
        sprintf(buffer_temp,"file%d",i);

        ay_player_playlist_get_item(i,buffer_item_actual);
        if (strcmp(buffer_temp,buffer_item_actual)) {
            printf("Items do not match\n");
            exit(1);
        }
    }

    //Deleting 10000
    printf("Deleting 10000...\n");
    for (i=0;i<10000;i++) {
        ay_player_playlist_remove(0);
    }

    total=ay_player_playlist_get_total_elements();

    printf("Total: %d\n",total);

    if (total!=0) {
        printf("Error getting total\n");
        exit(1);
    }
}

void codetests_multiply_8bits(void)
{
    //comprobar todos los posibles dos operadores
    int i,j;

    for (i=0;i<256;i++) {
        for (j=0;j<256;j++) {
            z80_int resultado=util_multiply_8bits(i,j);
            printf("%d * %d = %d\n",i,j,resultado);
            if (resultado!=i*j) {
                printf("Error multiply\n");
                exit(1);
            }
        }
    }
}

void codetests_stl(void)
{
    char buffer_linea[256];

    util_stl_print_vertex(buffer_linea,3,4,5,0,10);

    printf("%s\n",buffer_linea);


    FILE *ptr_stl=fopen("prueba.stl","wb");
    if (!ptr_stl) {
            printf("Unable to create sample stl file\n");
            exit(1);
    }

    util_stl_cube(ptr_stl,10,20,0,0,1,1,1,1);
}

void codetests_main(int main_argc,char *main_argv[])
{

	if (main_argc>2) {
		printf ("\nRunning compress/uncompress repetitions code\n");
		coretests_compress_uncompress_repetitions(main_argv[2]);
        coretests_compress_uncompress_repetitions_zip(main_argv[2]);
		exit(0);
	}

	scr_messages_debug=codetests_messages_debug;
	verbose_level=VERBOSE_PARANOID;
	scr_set_driver_name("");

	printf ("\nRunning expression parser tests\n");
	codetests_expression_parser();


	//printf ("\nRunning mid tests\n");
	//codetests_mid_test();


	//int lineas=get_file_lines("pruebatrans.log.1x");
	//printf ("lineas: %d\n",lineas);


	//printf ("Note: %d\n",get_mid_number_note("C0"));
	//printf ("Note: %d\n",get_mid_number_note("G9"));
	//printf ("Note: %d\n",get_mid_number_note("KK"));
	//printf ("Note: %d\n",get_mid_number_note(""));

	printf ("\nRunning assembler tests\n");
	codetests_assembler();

	printf ("\nRunning zeng tests\n");
	init_network_tables();
	codetests_zeng();

	//printf ("error: %s\n",z_sock_get_error(Z_ERR_NUM_READ_SOCKET));

	//codetests_open_sockets_infinite();

	//printf ("\nRunning zsock http tests\n");
	//init_network_tables();
	//codetests_http();

	//printf ("\nRunning zsock https tests\n");
	//init_network_tables();
	//codetests_https();

    //codetests_https_sni();

	//int r=z_sock_close_connection(44);
	//if (r<0) printf ("Error: %s\n",z_sock_get_error(r));

//#ifdef USE_PTHREADS
//	printf ("\nRunning atomic tests\n");
//  init_network_tables();
//	codetests_atomic();
//#endif

//    codetests_simple_atomic();



//#ifdef USE_PTHREADS
//	printf ("\nRunning network atomic tests\n");
// init_network_tables();
//	codetests_network_atomic();
//#endif

	printf ("\nRunning tbblue layers strings\n");
	codetests_tbblue_layers();

	printf ("\nRunning repetitions code\n");
	codetests_repetitions();

	printf ("\nRunning compress repetitions code\n");
	coretests_compress_repetitions();

	printf ("\nRunning get raster tbblue horizontal\n");
	codetests_tbblue_get_horizontal_raster();


	printf ("\nRunning code tests tbblue_set_ram_blocks\n");
	codetests_tbblue_set_ram_blocks();

    printf("\nRunning cosine table tests\n");
    codetests_cosine_table();

    printf("\nRunning zxvision scanf history tests\n");
    codetests_scanf_history();

    printf("\nRunning square root tests\n");
    codetests_sqrt();

    printf("\nRunning acosine tests\n");
    codetests_acosine();

    printf("\nRunning debug printf exclude/include class tests\n");
    codetests_debug_printf_exclude_include();

    printf("\nRunning tbblue divmmc masks\n");
    codetests_tbblue_divmmc_masks();

    //printf("\nRunning ay playlist codetests\n");
    //codetests_ay_playlist();

    //printf("\nRunning zeng online put-get snapshot tests\n");
    //codetests_zengonline_putget_snapshot();

    printf("\nRunning multiply 8 bits code tests\n");
    codetests_multiply_8bits();

    //printf("\nRunning codetests stl\n");
    //codetests_stl();

    //temporal crear dsk
    //dsk_create("/tmp/maspruebas.dsk",40,1,9,512);

    //Este es solo un test para probar velocidad, no valida realmente que funcione
    //printf("\nRunning int util_get_pixel_color_scr time tests\n");
    //codetests_get_pixel_color_scr();

    //printf("\nRunning get note table ql test\n");
    //codetests_get_note_table_ql();


	//printf ("\nRunning getting background F-key\n");
	//codetests_get_background_f_key();


	//prueba crear paleta
	/*
	int i;

	int valor_paleta=0;

	for (i=0;i<256;i++) {
		printf ("%d %d %d Untitled\n",(valor_paleta>>16 & 0xFF),(valor_paleta>>8 & 0xFF),(valor_paleta  & 0xFF)) ;

		valor_paleta +=0x010101;
	}
	*/

	exit(0);
}



