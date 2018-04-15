#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "ql.h"
#include "m68k.h"
#include "debug.h"
#include "utils.h"
#include "menu.h"
#include "operaciones.h"
#include "screen.h"


#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif

//extern unsigned char puerto_49150;


char ql_mdv1_root_dir[PATH_MAX]="";
char ql_mdv2_root_dir[PATH_MAX]="";
char ql_flp1_root_dir[PATH_MAX]="";


unsigned char *memoria_ql;
unsigned char ql_mc_stat;

unsigned char ql_pc_intr;

int ql_microdrive_floppy_emulation=0;


#define QL_STATUS_IPC_IDLE 0
#define QL_STATUS_IPC_WRITING 1

unsigned char ql_ipc_last_write_value=0;

//Ultimo comando recibido
unsigned char ql_ipc_last_command=0;

//Alterna bit ready o no leyendo el serial bit de ipc
int ql_ipc_reading_bit_ready=0;

//Ultimo parametro de comando recibido
unsigned char ql_ipc_last_command_parameter;

int ql_ipc_last_write_bits_enviados=0;
int ql_estado_ipc=QL_STATUS_IPC_IDLE;
int ql_ipc_bytes_received=0;

unsigned char ql_ipc_last_nibble_to_read[32];
int ql_ipc_last_nibble_to_read_mascara=8;
int ql_ipc_last_nibble_to_read_index=0;
int ql_ipc_last_nibble_to_read_length=1;


unsigned char temp_pcintr;

void ql_load_binary_file(FILE *ptr_file,unsigned int direccion, unsigned int longitud);

//Valores que me invento para gestionar pulsaciones de teclas no ascii
#define QL_KEYCODE_F1 256
#define QL_KEYCODE_F2 257
#define QL_KEYCODE_UP 258
#define QL_KEYCODE_DOWN 259
#define QL_KEYCODE_LEFT 260
#define QL_KEYCODE_RIGHT 261


//ql_keyboard_table[0] identifica a fila 7 F4     F1      5     F2     F3     F5      4      7
//...
//ql_keyboard_table[7] identifica a fila 0 Shift   Ctrl    Alt      x      v      /      n      ,

//Bits se cuentan desde la izquierda:

// 1      2      4     8      16     32      64    128
//F4     F1      5     F2     F3     F5      4      7

z80_byte ql_keyboard_table[8]={
	255,
	255,
	255,
	255,
	255,
	255,
	255,
	255
};




void ql_footer_mdflp_operating(void)
{
	generic_footertext_print_operating("MDFLP");
}



//adicionales
int ql_pressed_backspace=0;

//Nota: esta matrix de teclas y su numeracion de cada fila está documentada erroneamente en la info del QL de manera ascendente (de 0 a 7),
//mientras que lo correcto, cuando se habla de filas en resultado de comandos ipc, es descendente, tal y como está a continuación:

// ================================== matrix ============================
//        0      1      2      3      4      5      6      7
//  +-------------------------------------------------------
// 7|    F4     F1      5     F2     F3     F5      4      7     ql_keyboard_table[0]
// 6|   Ret   Left     Up    Esc  Right      \  Space   Down     ql_keyboard_table[1]
// 5|     ]      z      .      c      b  Pound      m      '     ql_keyboard_table[2]
// 4|     [   Caps      k      s      f      =      g      ;     ql_keyboard_table[3]
// 3|     l      3      h      1      a      p      d      j     ql_keyboard_table[4]
// 2|     9      w      i    Tab      r      -      y      o     ql_keyboard_table[5]
// 1|     8      2      6      q      e      0      t      u     ql_keyboard_table[6]
// 0| Shift   Ctrl    Alt      x      v      /      n      ,     ql_keyboard_table[7]


//Retorna fila y columna para una tecla pulsada mirando ql_keyboard_table. No se puede retornar por ahora mas de una tecla a la vez
//Se excluye shift, ctrl y alt de la respuesta
//Retorna fila -1 y columna -1 si ninguna tecla pulsada

void ql_return_columna_fila_puertos(int *columna,int *fila)
{

	int c,f;
	c=-1;
	f=-1;

	int i;
	int rotacion;

	z80_byte valor_puerto;
	int salir=0;
	for (i=0;i<8 && salir==0;i++){
		valor_puerto=ql_keyboard_table[i];
		//Si shift ctrl y alt quitarlos
		if (i==7) valor_puerto |=1+2+4;

		//Ver si hay alguna tecla pulsada

		for (rotacion=0;rotacion<8 && salir==0;rotacion++) {
			if ((valor_puerto&1)==0) {
				c=rotacion;
				f=7-i;
				salir=1;
				//printf ("c: %d f: %d\n",c,f);
			}
			else {
				valor_puerto=valor_puerto>>1;
			}
		}
	}

	*columna=c;
	*fila=f;
}

//Retorna caracter ascii segun tecla pulsada en ql_keyboard_table
//Miramos segun tabla de tecla a puertos (ql_tabla_teclado_letras)
int ql_return_ascii_key_pressed(void)
{
	//temp
	//if ((ql_keyboard_table[4]&1)==0) return 'l';
	//if ((ql_keyboard_table[4]&16)==0) return 'a';

	int letra;
	int indice=0;

	for (letra='a';letra<='z';letra++) {
		if ((*ql_tabla_teclado_letras[indice].puerto & ql_tabla_teclado_letras[indice].mascara)==0) {
			//printf ("letra: %c\n",letra);
			return letra;
		}

		indice++;
	}

	indice=0;
	for (letra='0';letra<='9';letra++) {
		if ((*ql_tabla_teclado_numeros[indice].puerto & ql_tabla_teclado_numeros[indice].mascara)==0) {
			//printf ("numero: %c\n",letra);
			return letra;
		}

		indice++;
	}

	//Otras teclas
	//Enter
	if ((ql_keyboard_table[1]&1)==0) return 10;

	//Punto
	if ((ql_keyboard_table[2]&4)==0) return '.';

	//Coma
	if ((ql_keyboard_table[7]&128)==0) return ',';

	//Espacio
	if ((ql_keyboard_table[1]&64)==0) return 32;


	//F1
	if ((ql_keyboard_table[0]&2)==0) return QL_KEYCODE_F1;

	//F2
	if ((ql_keyboard_table[0]&8)==0) return QL_KEYCODE_F2;

// 1|   Ret   Left     Up    Esc  Right      \  Space   Down
	if ((ql_keyboard_table[1]&4)==0) return QL_KEYCODE_UP;

	if ((ql_keyboard_table[1]&128)==0) return QL_KEYCODE_DOWN;

	if ((ql_keyboard_table[1]&2)==0) return QL_KEYCODE_LEFT;

	if ((ql_keyboard_table[1]&16)==0) return QL_KEYCODE_RIGHT;


	return 0;
}

void ql_ipc_reset(void)
{
	ql_ipc_last_write_value=0;
	ql_ipc_last_write_bits_enviados=0;
	ql_estado_ipc=QL_STATUS_IPC_IDLE;
	ql_ipc_last_command=0;
	ql_ipc_bytes_received=0;
	ql_ipc_last_nibble_to_read_mascara=8;
	ql_ipc_last_nibble_to_read_index=0;
	ql_ipc_last_nibble_to_read_length=1;
	ql_ipc_reading_bit_ready=0;
}

void ql_debug_port(unsigned int Address)
{
	return;
	switch (Address) {
		case 0x18000:
		printf ("	PC_CLOCK		Real-time clock (Long word)\n\n");
		break;

		case 0x18002:
		printf ("	PC_TCTRL		Transmit control register\n\n");
		break;

		case 0x18003:
		printf ("	PC_IPCWR		IPC port - write only\n\n");
		break;

		case 0x18020:
		printf ("	PC_IPCRD		IPC port - read only\n");
		printf ("	PC_MCTRL		Microdrive control register - write only\n\n");
		break;

		case 0x18021:
		printf ("	PC_INTR		Interrupt register\n\n");
		//usleep(20000);
		break;

		case 0x18022:
		printf ("	PC_TDATA		Transmit register - write only\n");
		printf ("	PC_TRAK1		Read microdrive track 1\n\n");
		break;

		case 0x18023:
		printf ("	PC_TRAK2		Read microdrive track 2\n\n");
		break;

		case 0x18063:
		printf ("	MC_STAT		Master chip status register\n\n");
		break;

		default:
		printf ("Unknown i/o port %08XH\n",Address);
		break;
	}
}





//unsigned char temp_read_ipc;
unsigned char ql_read_ipc(void)
{


//Temporal
//temp_read_ipc ^=64;

	unsigned char valor_retorno=0;

	//printf ("Valor temporal reading ipc: %d\n",ql_ipc_last_nibble_to_read[0]);
/*
        ql_ipc_last_nibble_to_read_index;
        ql_ipc_last_nibble_to_read_length;
*/

	//Ir alternando valor retornado
	if (ql_ipc_reading_bit_ready==0) {
		ql_ipc_reading_bit_ready=1;
		return 0;
	}

	//else ql_ipc_reading_bit_ready=0;



	if (ql_ipc_last_nibble_to_read[ql_ipc_last_nibble_to_read_index]&ql_ipc_last_nibble_to_read_mascara) valor_retorno |=128; //Valor viene en bit 7


	//Solo mostrar este debug si hay tecla pulsada

	//if (ql_pulsado_tecla() )printf ("Returning ipc: %XH. Index nibble: %d mask: %d\n",valor_retorno,ql_ipc_last_nibble_to_read_index,ql_ipc_last_nibble_to_read_mascara);

	if (ql_ipc_last_nibble_to_read_mascara!=1) ql_ipc_last_nibble_to_read_mascara=ql_ipc_last_nibble_to_read_mascara>>1;
	else {

		//Siguiente byte
		ql_ipc_last_nibble_to_read_mascara=8;
		ql_ipc_last_nibble_to_read_index++;
		if (ql_ipc_last_nibble_to_read_index>=ql_ipc_last_nibble_to_read_length) ql_ipc_last_nibble_to_read_index=0; //Si llega al final, dar la vuelta
		//if (ql_ipc_last_nibble_to_read_index>=ql_ipc_last_nibble_to_read_length) ql_ipc_last_nibble_to_read_index=ql_ipc_last_nibble_to_read_length; //dejarlo al final
	}
	//Para no perder nunca el valor. Rotamos mascara


	//if (ql_ipc_last_nibble_to_read_index>1) sleep(2);

	//if (valor_retorno) sleep(1);

	return valor_retorno;
	//return 0;  //De momento eso



	/*
	* Receiving data from the IPC is done by writing %1110 to pc_ipcwr for each bit
* of the data, once again waiting for bit 6 at pc_ipcrd to go to zero, and
* then reading bit 7 there as the data bit. The data is received msb first.
*/
}



int ql_pulsado_tecla(void)
{

	if (menu_abierto) return 0;

	//Si backspace
	if (ql_pressed_backspace) return 1;

	z80_byte acumulado;

	acumulado=255;

	int i;
	for (i=0;i<8;i++) acumulado &=ql_keyboard_table[i];

	if (acumulado==255) return 0;
	return 1;
	/*

	acumulado=menu_da_todas_teclas();


					//Hay tecla pulsada
					if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA ) {
						return 1;
					}
	return 0;*/
}

//unsigned char temp_stat_cmd;
//unsigned char temp_contador_tecla_pulsada;

//int temp_columna=0;



//Para gestionar repeticiones
int ql_mantenido_pulsada_tecla=0;
int ql_mantenido_pulsada_tecla_timer=0;

/*
// ================================== matrix ============================
//        0      1      2      3      4      5      6      7
//  +-------------------------------------------------------
// 7|    F4     F1      5     F2     F3     F5      4      7
// 6|   Ret   Left     Up    Esc  Right      \  Space   Down
// 5|     ]      z      .      c      b  Pound      m      '
// 4|     [   Caps      k      s      f      =      g      ;
// 3|     l      3      h      1      a      p      d      j
// 2|     9      w      i    Tab      r      -      y      o
// 1|     8      2      6      q      e      0      t      u
// 0| Shift   Ctrl    Alt      x      v      /      n      ,
*/

struct x_tabla_columna_fila
{
        int columna;
        int fila;
};

struct x_tabla_columna_fila ql_col_fil_numeros[]={
	{5,1}, //0
	{3,3},
	{1,1}, //2
	{1,3},
	{6,7},  //4
	{2,7},
	{2,1},  //6
	{7,7},
	{0,1}, //8
	{0,2}
};


/*
// ================================== matrix ============================
//        0      1      2      3      4      5      6      7
//  +-------------------------------------------------------
// 7|    F4     F1      5     F2     F3     F5      4      7
// 6|   Ret   Left     Up    Esc  Right      \  Space   Down
// 5|     ]      z      .      c      b  Pound      m      '
// 4|     [   Caps      k      s      f      =      g      ;
// 3|     l      3      h      1      a      p      d      j
// 2|     9      w      i    Tab      r      -      y      o
// 1|     8      2      6      q      e      0      t      u
// 0| Shift   Ctrl    Alt      x      v      /      n      ,
*/
struct x_tabla_columna_fila ql_col_fil_letras[]={
	{4,3}, //A
	{4,5},
	{3,5},
	{6,3},
	{4,1}, //E
	{4,4},
	{6,4},
	{2,3}, //H
	{2,2},
	{7,3},
	{2,4}, //K
	{0,3},
	{6,5},
	{6,0}, //N
	{7,2},
	{5,3},
	{3,1}, //Q
	{4,2},
	{3,4},
	{6,1}, //T
	{7,1},
	{4,0},
	{1,2}, //W
	{3,0},
	{6,2},
	{1,5} //Z
};


//Returna fila y columna para una tecla dada.
void ql_return_fila_columna_tecla(int tecla,int *columna,int *fila)
{
	int c;
	int f;
	int indice;

	//Por defecto
	c=-1;
	f=-1;

	if (tecla>='0' && tecla<='9') {
		indice=tecla-'0';
		c=ql_col_fil_numeros[indice].columna;
		f=ql_col_fil_numeros[indice].fila;
	}

	else if (tecla>='a' && tecla<='z') {
		indice=tecla-'a';
		c=ql_col_fil_letras[indice].columna;
		f=ql_col_fil_letras[indice].fila;
	}

	else if (tecla==32) {
		c=6;
		f=6;
	}

	else if (tecla==10) {
		c=0;
		f=6;
	}

	else if (tecla==QL_KEYCODE_F1) {
		c=1;
		f=7;
	}

	else if (tecla==QL_KEYCODE_F2) {
		c=3;
		f=7;
	}

	else if (tecla=='.') {
		c=2;
		f=5;
	}

	else if (tecla==',') {
		c=7;
		f=0;
	}

	//        0      1      2      3      4      5      6      7
	//  +-------------------------------------------------------
	// 6|   Ret   Left     Up    Esc  Right      \  Space   Down

	else if (tecla==QL_KEYCODE_UP) {
		c=2;
		f=6;
	}

	else if (tecla==QL_KEYCODE_DOWN) {
		c=7;
		f=6;
	}


	else if (tecla==QL_KEYCODE_LEFT) {
		c=1;
		f=6;
	}


	else if (tecla==QL_KEYCODE_RIGHT) {
		c=4;
		f=6;
	}


	*columna=c;
	*fila=f;
}

/*
89l6ihverantyd


*/




//Mete en valores ipc de vuelta segun teclas pulsadas
/*Desde la rom, cuando se genera una PC_INTR, se pone a leer de ipc lo siguiente y luego ya llama a write_ipc comando 8:


Read PC_INTR pulsado tecla
Returning ipc: 0H. Index nibble: 0 mask: 1
Returning ipc: 0H. Index nibble: 0 mask: 8
Returning ipc: 80H. Index nibble: 0 mask: 4
Write ipc command 1 pressed key
Returning ipc: 80H. Index nibble: 0 mask: 8
Returning ipc: 80H. Index nibble: 0 mask: 4
Returning ipc: 80H. Index nibble: 0 mask: 2
Returning ipc: 80H. Index nibble: 0 mask: 1

Returning ipc: 80H. Index nibble: 0 mask: 8
Returning ipc: 80H. Index nibble: 0 mask: 4
Returning ipc: 80H. Index nibble: 0 mask: 2
Returning ipc: 80H. Index nibble: 0 mask: 1

Returning ipc: 80H. Index nibble: 0 mask: 8
Returning ipc: 80H. Index nibble: 0 mask: 4
Returning ipc: 80H. Index nibble: 0 mask: 2
Returning ipc: 80H. Index nibble: 0 mask: 1

Returning ipc: 80H. Index nibble: 0 mask: 8
Returning ipc: 80H. Index nibble: 0 mask: 4
Returning ipc: 80H. Index nibble: 0 mask: 2
Returning ipc: 80H. Index nibble: 0 mask: 1

Returning ipc: 80H. Index nibble: 0 mask: 8
QL Trap ROM: Tell one key pressed
Returning ipc: 80H. Index nibble: 0 mask: 4
Returning ipc: 80H. Index nibble: 0 mask: 2
Returning ipc: 80H. Index nibble: 0 mask: 1


--Aparentemente estas lecturas anteriores no afectan al valor de la tecla

Lectura de teclado comando. PC=00002F8EH
letra: a
no repeticion


*/


//int temp_flag_reves=0;
void ql_ipc_write_ipc_teclado(void)
{
	/*
	* This returns one nibble, plus up to 7 nibble/byte pairs:
	* first nibble, ms bit: set if final last keydef is still held
	* first nibble, ls 3 bits: count of keydefs to follow.
	* then, for each of the 0..7 keydefs:
	* nibble, bits are 3210=lsca: lost keys (last set only), shift, ctrl and alt.
	* byte, bits are 76543210=00colrow: column and row as keyrow table.
	* There is a version of the IPC used on the thor that will also return keydef
	* values for a keypad. This needs looking up
	*/

	/*Devolveremos una tecla. Esto es:
	* first nibble, ms bit: set if final last keydef is still held
	* first nibble, ls 3 bits: count of keydefs to follow.
	valor 1
	//nibble, bits are 3210=lsca: lost keys (last set only), shift, ctrl and alt.
	valor 0
	//byte, bits are 76543210=00colrow: column and row as keyrow table
// ================================== matrix ============================
//        0      1      2      3      4      5      6      7
//  +-------------------------------------------------------
// 7|    F4     F1      5     F2     F3     F5      4      7
// 6|   Ret   Left     Up    Esc  Right      \  Space   Down
// 5|     ]      z      .      c      b  Pound      m      '
// 4|     [   Caps      k      s      f      =      g      ;
// 3|     l      3      h      1      a      p      d      j
// 2|     9      w      i    Tab      r      -      y      o
// 1|     8      2      6      q      e      0      t      u
// 0| Shift   Ctrl    Alt      x      v      /      n      ,


	//F1 es columna 1, row 0
	valor es 7654=00co=0000=0
	valor es lrow=1000 =8
	col=001 row=000
	00 001 000

	*/


	int columna;
	int fila;


int i;
	//Si tecla no pulsada
	//if ((puerto_49150&1)) {
	if (!ql_pulsado_tecla()) {
		for (i=0;i<ql_ipc_last_nibble_to_read_length;i++) ql_ipc_last_nibble_to_read[i]=0;
	}


ql_return_columna_fila_puertos(&columna,&fila);
int tecla_shift=0;
int tecla_control=0;
int tecla_alt=0;


if ( (columna>=0 && fila>=0) || ql_pressed_backspace) {
	if (ql_mantenido_pulsada_tecla==0 || (ql_mantenido_pulsada_tecla==1 && ql_mantenido_pulsada_tecla_timer>=50) )  {
		if (ql_mantenido_pulsada_tecla==0) {
			ql_mantenido_pulsada_tecla=1;
			ql_mantenido_pulsada_tecla_timer=0;
		}


		if (ql_pressed_backspace) {
			//CTRL + flecha izquierda
			tecla_control=1;
			// 6|   Ret   Left     Up    Esc  Right      \  Space   Down
			fila=6;
			columna=1;

		}

		//printf ("------fila %d columna %d\n",fila,columna);
		z80_byte byte_tecla=((fila&7)<<3) | (columna&7);



		ql_ipc_last_nibble_to_read[2]=(byte_tecla>>4)&15;
		ql_ipc_last_nibble_to_read[3]=(byte_tecla&15);

		if ((ql_keyboard_table[7]&1)==0) tecla_shift=1;
		if ((ql_keyboard_table[7]&2)==0) tecla_control=1;
		if ((ql_keyboard_table[7]&4)==0) tecla_alt=1;


		ql_ipc_last_nibble_to_read[1]=0;  //lsca
		if (tecla_shift) ql_ipc_last_nibble_to_read[1] |=4;
		if (tecla_control) ql_ipc_last_nibble_to_read[1] |=2;
		if (tecla_alt) ql_ipc_last_nibble_to_read[1] |=1;


	}
	else {
		//debug_printf (VERBOSE_PARANOID,"Repeating key");
		ql_ipc_last_nibble_to_read[0]=ql_ipc_last_nibble_to_read[1]=ql_ipc_last_nibble_to_read[2]=ql_ipc_last_nibble_to_read[3]=ql_ipc_last_nibble_to_read[4]=ql_ipc_last_nibble_to_read[5]=0;
	}
}
else {
	//debug_printf (VERBOSE_PARANOID,"Unknown key");
	ql_mantenido_pulsada_tecla=0;
	ql_ipc_last_nibble_to_read[0]=ql_ipc_last_nibble_to_read[1]=ql_ipc_last_nibble_to_read[2]=ql_ipc_last_nibble_to_read[3]=ql_ipc_last_nibble_to_read[4]=0;
}


				ql_ipc_last_nibble_to_read_mascara=8;
				ql_ipc_last_nibble_to_read_index=0;
				ql_ipc_last_nibble_to_read_length=5; //5;

					//printf ("Ultimo pc_intr: %d\n",temp_pcintr);

				for (i=0;i<ql_ipc_last_nibble_to_read_length;i++) {
					//debug_printf (VERBOSE_PARANOID,"Return IPC values:[%d] = %02XH",i,ql_ipc_last_nibble_to_read[i]);
				}

}



void ql_ipc_write_ipc_read_keyrow(int row)
{

/*
kbdr_cmd equ    9       keyboard direct read
* kbdr_cmd requires one nibble which selects the row to be read.
* The top bit of this is ignored (at least on standard IPC's...).
* It responds with a byte whose bits indicate which of the up to eight keys on
* the specified row of the keyrow table are held down. */

//De momento nada

	//z80_byte temp_resultado=0;

	//if (ql_pulsado_tecla()) temp_resultado++;
	z80_byte resultado_row;

	resultado_row=ql_keyboard_table[row&7] ^ 255;
	//Bit a 1 para cada tecla pulsada
	//row numerando de 0 a 7
	/*
	// ================================== matrix ============================
	//        0      1      2      3      4      5      6      7
	//  +-------------------------------------------------------
	// |    F4     F1      5     F2     F3     F5      4      7
	// |   Ret   Left     Up    Esc  Right      \  Space   Down
	// |     ]      z      .      c      b  Pound      m      '
	// |     [   Caps      k      s      f      =      g      ;
	// |     l      3      h      1      a      p      d      j
	// |     9      w      i    Tab      r      -      y      o
	// |     8      2      6      q      e      0      t      u
	// | Shift   Ctrl    Alt      x      v      /      n      ,
	Por ejemplo, para leer si se pulsa Space, tenemos que leer row 1, y ver luego si bit 6 está a 1 (40H)
	*/

	if (menu_abierto)  resultado_row=255;

	debug_printf (VERBOSE_PARANOID,"Reading ipc command 9: read keyrow. row %d returning %02XH",row,resultado_row);

		ql_ipc_last_nibble_to_read[0]=(resultado_row>>4)&15;
		ql_ipc_last_nibble_to_read[1]=resultado_row&15;
			ql_ipc_last_nibble_to_read_mascara=8;
			ql_ipc_last_nibble_to_read_index=0;
			ql_ipc_last_nibble_to_read_length=2;


}

void ql_write_ipc(unsigned char Data)
{
	/*
	* Commands and data are sent msb first, by writing a byte containg %11x0 to
	* location pc_ipcwr ($18023), where the "x" is one data bit. Bit 6 at location
	* pc_ipcrd ($18020) is then examined, waiting for it to go zero to indicate
	* that the bit has been received by the IPC.
	*/
	/*
	* Receiving data from the IPC is done by writing %1110 to pc_ipcwr for each bit
* of the data, once again waiting for bit 6 at pc_ipcrd to go to zero, and
* then reading bit 7 there as the data bit. The data is received msb first.
	*/

	//Si dato tiene formato: 11x0  (8+4+1)
	if ((Data&13)!=12) return;

	int bitdato=(Data>>1)&1;
	//printf ("Escribiendo bit ipc: %d\n",bitdato);
	ql_ipc_last_write_value=ql_ipc_last_write_value<<1;
	ql_ipc_last_write_value |=bitdato;
	ql_ipc_last_write_bits_enviados++;
	if (ql_ipc_last_write_bits_enviados==4) {
			switch (ql_estado_ipc) {
			  case QL_STATUS_IPC_IDLE:
					ql_ipc_last_command=ql_ipc_last_write_value&15;
					//printf ("Resultante ipc command: %d (%XH)\n",ql_ipc_last_command,ql_ipc_last_command); //Se generan 4 bits cada vez
					ql_ipc_last_write_bits_enviados=0;

					//Actuar segun comando
					switch (ql_ipc_last_command)
					{


						case 0:
						//*rset_cmd equ    0       resets the IPC software
							ql_ipc_reset();
						break;

						case 1:
/*
stat_cmd equ    1       report input status
* returns a byte, the bits of which are:
ipc..kb equ     0       set if data available in keyboard buffer, or key held
ipc..so equ     1       set if sound is still being generated
*               2       set if kbd shift setting has changed, with key held
*               3       set if key held down
ipc..s1 equ     4       set if input is pending from RS232 channel 1
ipc..s2 equ     5       set if input is pending from RS232 channel 2
ipc..wp equ     6       return state of p26, currently not connected
*               7       was set if serial transfer was being zapped, now zero
*/

							ql_ipc_last_nibble_to_read[0]=0; //ipc..kb equ     0       set if data available in keyboard buffer, or key held


							//Decir tecla pulsada

							//temp
							//temp_stat_cmd++;
							//ql_ipc_last_nibble_to_read[0]=temp_stat_cmd;
							ql_ipc_last_nibble_to_read[0]=15; //Devolver valor entre 8 y 15 implica que acabara leyendo el teclado
						        ql_ipc_last_nibble_to_read_mascara=8;
						        ql_ipc_last_nibble_to_read_index=0;
						        ql_ipc_last_nibble_to_read_length=1;


							//Si tecla no pulsada
							if (!ql_pulsado_tecla()) ql_ipc_last_nibble_to_read[0]=4;

							else {
								//debug_printf (VERBOSE_DEBUG,"Write ipc command 1: Report input status pressed key");
							}
							//if ((puerto_49150&1)) ql_ipc_last_nibble_to_read[0]=4;

							//printf ("Valor a retornar: %d\n",ql_ipc_last_nibble_to_read[0]&15);

							//sleep(1);

						break;

						case 8:

							//debug_printf (VERBOSE_DEBUG,"Write ipc command 8: Read key. PC=%08XH",get_pc_register() );

							ql_ipc_write_ipc_teclado();


						break;

						case 9:
							//debug_printf (VERBOSE_ERR,"Write ipc command 9: Reading keyrow. Not implemented");
							/*
							kbdr_cmd equ    9       keyboard direct read
* kbdr_cmd requires one nibble which selects the row to be read.
* The top bit of this is ignored (at least on standard IPC's...).
* It responds with a byte whose bits indicate which of the up to eight keys on
* the specified row of the keyrow table are held down. */

							ql_estado_ipc=QL_STATUS_IPC_WRITING;


						break;

						case 10:
						/*
						inso_cmd equ    10      initiate sound process
* This requires no less than 64 bits of data. it starts sound generation.
* Note that the 16 bit values below need to have their ls 8 bits sent first!
* 8 bits pitch 1
* 8 bits pitch 2
* 16 bits interval between steps
* 16 bits duration (0=forever)
* 4 bits signed step in pitch
* 4 bits wrap
* 4 bits randomness (none unless msb is set)
* 4 bits fuziness (none unless msb is set)
*/
							debug_printf (VERBOSE_PARANOID,"ipc command 10 inso_cmd initiate sound process");
							//ql_estado_ipc=QL_STATUS_IPC_WRITING; Mejor lo desactivo porque si no se queda en estado writing y no sale de ahi
						break;

						//baud_cmd
						case 13:
						/*
						baud_cmd equ    13      change baud rate
* This expects one nibble, of which the 3 lsbs select the baud rate for both
* serial channels. The msb is ignored. Values 7 down to zero correspond to baud
* rates 75/300/600/1200/2400/4800/9600/19200.
* The actual clock rate is supplied from the PC to the IPC, but this command is
* also needed in the IPC for timing out transfers!
						*/
						ql_estado_ipc=QL_STATUS_IPC_WRITING;
						break;


						case 15:
							//El que se acaba enviando para leer del puerto ipc. No hacer nada
						break;

						default:
							//debug_printf (VERBOSE_ERR,"Write ipc command %d. Not implemented",ql_ipc_last_command);
						break;
					}
				break;


			case QL_STATUS_IPC_WRITING:
				ql_ipc_last_command_parameter=ql_ipc_last_write_value&15;
				//printf ("Parametro recibido de ultimo comando %d: %d\n",ql_ipc_last_command,ql_ipc_last_command_parameter);
				//Segun ultimo comando
					switch (ql_ipc_last_command) {
						case 9:
						/*
						kbdr_cmd equ    9       keyboard direct read
* kbdr_cmd requires one nibble which selects the row to be read.
* The top bit of this is ignored (at least on standard IPC's...).
* It responds with a byte whose bits indicate which of the up to eight keys on
* the specified row of the keyrow table are held down. */
//ql_ipc_write_ipc_read_keyrow();
							//printf ("Parametro recibido de ultimo comando read keyrow %d: %d\n",ql_ipc_last_command,ql_ipc_last_command_parameter);
							ql_estado_ipc=QL_STATUS_IPC_IDLE;
							ql_ipc_last_write_bits_enviados=0;
							ql_ipc_write_ipc_read_keyrow(ql_ipc_last_command_parameter);


						break;


						case 10:
							debug_printf (VERBOSE_PARANOID,"parameter sound %d: %d",ql_ipc_bytes_received,ql_ipc_last_command_parameter);
							ql_ipc_bytes_received++;
							if (ql_ipc_bytes_received>=8) {
								debug_printf (VERBOSE_PARANOID,"End receiving ipc parameters");
								ql_estado_ipc=QL_STATUS_IPC_IDLE;
								ql_ipc_last_write_bits_enviados=0;
								ql_ipc_bytes_received=0;
							}
						break;

						case 13:
							ql_estado_ipc=QL_STATUS_IPC_IDLE;
							//Fin de parametros de comando. Establecer baud rate y dejar status a idle de nuevo para que la siguiente escritura se interprete como comando inicial
						break;
					}
			break;
			}
			//sleep(2);
	}


}




unsigned char ql_lee_puerto(unsigned int Address)
{



	unsigned char valor_retorno=0;

	//temporal
	//return 255;

	switch (Address) {

		case 	0x18020:
			//printf ("Leyendo IPC. PC=%06XH\n",get_pc_register());
			//temp
			//return 0;
			return ql_read_ipc();

		break;


		case 0x18021:
			//printf ("Read PC_INTR		Interrupt register. Value: %02XH\n\n",ql_pc_intr);


                        //temp solo al pulsar enter
                        ////puerto_49150    db              255  ; H                J         K      L    Enter ;6
                        //if ((puerto_49150&1)==0) {
/*
* read addresses
pc_intr equ     $18021  bits 4..0 set as pending level 2 interrupts
pc.intre equ    1<<4    external interrupt register
pc.intrf equ    1<<3    frame interrupt register
pc.intrt equ    1<<2    transmit interrupt register
pc.intri equ    1<<1    interface interrupt register
pc.intrg equ    1<<0    gap interrupt register
*/

/*
Esto se lee en la rom asi:
L00352 MOVEM.L D7/A5-A6,-(A7)       *#: ext2int entry
XL00352 EQU L00352
       MOVEA.L A7,A5
       MOVEA.L #$00028000,A6
       MOVE.B  $00018021,D7
       LSR.B   #1,D7
       BCS     L02ABC
       LSR.B   #1,D7
       BCS     L02CCC               * 8049 interrupt

	Por tanto la 8049 interrupt se interpreta cuando bit 1 activo
*/

			ql_pc_intr=0;
			//if ((puerto_49150&1)==0) ql_pc_intr |=2;
			if (ql_pulsado_tecla() ) {
				//debug_printf (VERBOSE_DEBUG,"Read PC_INTR pressed key");
				ql_pc_intr |=2;
			}
			//printf ("------------Retornando %d\n",ql_pc_intr);
		//	return ql_pc_intr;
			return 134; //Con pruebas, acabo viendo que retornar este valor acaba provocando lectura de teclado

			temp_pcintr++;
			//printf ("------------Retornando %d\n",temp_pcintr);
			return temp_pcintr;
			//if ((puerto_49150&1)==0) ql_pc_intr |=31;
			if (ql_pulsado_tecla() ) ql_pc_intr |=31;
			//usleep(5000000);
			//sleep(1);
			return ql_pc_intr;
			//return 31;

			//}

			//else return 255;
			//return 255;
		break;

	}


	return valor_retorno;
}

void ql_out_port(unsigned int Address, unsigned char Data)
{

	int anterior_video_mode;

	switch (Address) {

  	case    0x18003:
			//printf ("Escribiendo IPC. Valor: %02XH PC=%06XH\n",Data,get_pc_register() );
			ql_write_ipc(Data);

		break;

		case 0x18020:
			//printf ("Writing on 18020H - Microdrive control register - write. Data: %02XH\n",Data);
		break;

		case 0x18021:
		  //printf ("Escribiendo pc_intr. Valor: %02XH\n",Data);
/*
*pc_intr equ    $18021  7..5 masks and 4..0 to clear interrupt
*/

//Es una mascara??
			ql_pc_intr=ql_pc_intr&(Data^255);

			ql_pc_intr=Data;

			//sleep(5);
		break;

		case 0x18063:
			//MC_STAT		Master chip status register
			anterior_video_mode=(ql_mc_stat>>3)&1;

			ql_mc_stat=Data;

			int video_mode=(ql_mc_stat>>3)&1;

			if (video_mode!=anterior_video_mode) {
				//0=512x256
				//1=256x256
				/*
					0 = 4 colour (mode 4) =512x256
				  1 = 8 colour (mode 8) =256x256
				*/
				if (video_mode==0) screen_print_splash_text(10,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Setting mode 4 512x256");
				else screen_print_splash_text(10,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Setting mode 8 256x256");
			}

		break;
	}

}





void ql_writebyte(unsigned int Address, unsigned char Data)
{
	Address&=QL_MEM_LIMIT;

	if (Address>=0x18000 && Address<=0x1BFFF) {

		if (Address==0x18003) {
                //printf ("writing i/o %X value %X\n",Address,Data);
								ql_ipc_reading_bit_ready=0;
								ql_debug_port(Address);
								//sleep(1);
		}
		else {
			ql_debug_port(Address);
		}

		ql_out_port(Address,Data);


#ifdef EMULATE_VISUALMEM
		//Escribimos en visualmem a partir de direccion 18000H
		set_visualmembuffer(Address);

#endif

    return; //Espacio i/o
  }

	if (Address<0x18000 || Address>QL_MEM_LIMIT) return;
  //memoria_ql[Address&0xfffff]=Data;

  unsigned char valor=Data;
	//memoria_ql[Address&0xfffff]=valor;
  memoria_ql[Address]=valor;

#ifdef EMULATE_VISUALMEM
	//printf ("addr: %d\n",Address);
	//Escribimos en visualmem a partir de direccion 18000H
	set_visualmembuffer(Address);

#endif

}

unsigned char ql_readbyte(unsigned int Address)
{
	Address&=QL_MEM_LIMIT;

	if (Address>=0x18000 && Address<=0x1BFFF) {

		if (Address==0x18020) {
	    //printf ("Reading i/o %X\n",Address);
			ql_debug_port(Address);
			//sleep(1);
		}


		else {
			ql_debug_port(Address);
		}


		unsigned char valor=ql_lee_puerto(Address);
		//printf ("return value: %02XH\n",valor);
#ifdef EMULATE_VISUALMEM
                //Escribimos en visualmem a partir de direccion 18000H
                set_visualmemreadbuffer(Address);

#endif
		return valor;
	}

	//if (Address==0x00028000) printf ("Leyendo parte despues del boot\n");

  if (Address>QL_MEM_LIMIT) return(0);

#ifdef EMULATE_VISUALMEM
                //Escribimos en visualmem a partir de direccion 18000H
                set_visualmemreadbuffer(Address);

#endif

	//unsigned char valor=memoria_ql[Address&0xfffff];
	unsigned char valor=memoria_ql[Address];
	return valor;
}

unsigned char ql_readbyte_no_ports(unsigned int Address)
{
	Address&=QL_MEM_LIMIT;
	unsigned char valor=memoria_ql[Address];
	return valor;

}

void ql_writebyte_no_ports(unsigned int Address,unsigned char valor)
{
	Address&=QL_MEM_LIMIT;
	memoria_ql[Address]=valor;

}

//extern void    disass(char *buf, uint16 *inst_stream);

char texto_disassemble[1000];

/* Usado en prueba_ql.c
void dump_screen(void)
{
        printf ("Inicio pantalla\n");

        unsigned char *mem;

        mem=&memoria_ql[128*1024];

        int x,y;

        for (y=0;y<256;y++) {
                for (x=0;x<128;x++) {
                        printf ("%02X",*mem);
                        mem++;
                }
                printf ("\n");
        }

        printf ("\n");
}

*/


void disassemble(void)
{

/*
        uint8 *mem;

        int i;
        mem=&memoria_ql[M68K_REG_PC & QL_MEM_LIMIT];
        printf ("%02X%02X%02X%02X%02X%02X%02X%02X ",mem[0],mem[1],mem[2],mem[3],mem[4],mem[5],mem[6],mem[7]);

        uint16 buffer[16];

        uint16 temp;
        for (i=0;i<16;i++) {
                temp=(*mem)*256;
                mem++;
                temp |=(*mem);
                mem++;
                buffer[i]=temp;
        }

        disass(texto_disassemble,buffer);

        printf ("%08XH %s\n",pc,texto_disassemble);
*/
}




void print_regs(void)
{
/*
                printf ("PC: %08X usp: %08X ssp: %08X ",pc,usp,ssp);
        int i;

        for (i=0;i<8;i++) {
                printf ("D%d: %08X ",i,reg[i]);
        }
        for (;i<16;i++) {
                printf ("A%d: %08X ",i-8,reg[i]);
        }



        printf ("\n");
*/
}





unsigned int GetMemB(unsigned int address)
{
        return(ql_readbyte(address));
}
/* Fetch word, address may not be word-aligned */
unsigned int  GetMemW(unsigned int address)
{
#ifdef CHKADDRESSERR
    if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 1);
#endif
        return((ql_readbyte(address)<<8)|ql_readbyte(address+1));
}
/* Fetch dword, address may not be dword-aligned */
unsigned int GetMemL(unsigned int address)
{
#ifdef CHKADDRESSERR
    if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 1);
#endif
        return((GetMemW(address)<<16) | GetMemW(address+2));
}
/* Write byte to address */
void SetMemB (unsigned int address, unsigned int value)
{
        ql_writebyte(address,value);
}
/* Write word, address may not be word-aligned */
void SetMemW(unsigned int address, unsigned int value)
{
#ifdef CHKADDRESSERR
if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 0);
#endif
        ql_writebyte(address,(value>>8)&255);
        ql_writebyte(address+1, (value&255));
}
/* Write dword, address may not be dword-aligned */
void SetMemL(unsigned int address, unsigned int value)
{
#ifdef CHKADDRESSERR
    if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 0);
#endif
        SetMemW(address, (value>>16)&65535);
        SetMemW(address+2, (value&65535));
}


unsigned int m68k_read_disassembler_16 (unsigned int address)
{
	return GetMemW(address);
}


unsigned int m68k_read_disassembler_32 (unsigned int address)
{
	return GetMemL(address);
}




//Funciones legacy solo para interceptar posibles llamadas a poke, peek etc en caso de motorola
//la mayoria de estas vienen del menu, lo ideal es que en el menu se usen peek_byte_z80_moto , etc

void poke_byte_legacy_ql(z80_int dir GCC_UNUSED,z80_byte valor GCC_UNUSED)
{
	debug_printf(VERBOSE_ERR,"Calling poke_byte function on a QL machine. TODO fix it!");
}
void poke_byte_no_time_legacy_ql(z80_int dir GCC_UNUSED,z80_byte valor GCC_UNUSED)
{
	debug_printf(VERBOSE_ERR,"Calling poke_byte_no_time function on a QL machine. TODO fix it!");
}
z80_byte peek_byte_legacy_ql(z80_int dir GCC_UNUSED)
{
	debug_printf(VERBOSE_ERR,"Calling peek_byte function on a QL machine. TODO fix it!");
	return 0;
}
z80_byte peek_byte_no_time_legacy_ql(z80_int dir GCC_UNUSED)
{
	//debug_printf(VERBOSE_ERR,"Calling peek_byte_no_time function on a QL machine. TODO fix it!");
	return 0;
}
z80_byte lee_puerto_legacy_ql(z80_byte h GCC_UNUSED,z80_byte l GCC_UNUSED)
{
	debug_printf(VERBOSE_ERR,"Calling lee_puerto function on a QL machine. TODO fix it!");
	return 0;
}
void out_port_legacy_ql(z80_int puerto GCC_UNUSED,z80_byte value GCC_UNUSED)
{
	debug_printf(VERBOSE_ERR,"Calling out_port function on a QL machine. TODO fix it!");
}
z80_byte fetch_opcode_legacy_ql(void)
{
	debug_printf(VERBOSE_ERR,"Calling fetch_opcode function on a QL machine. TODO fix it!");
	return 0;
}



//Numero de canal ficticio para archivos que se abran mdvx_ o flpx_, para distinguirlos de los que gestiona el sistema
//#define OLD_QL_ID_CANAL_INVENTADO_MICRODRIVE 32


//Canal inventado solo para cuando se abre "mdv"
//#define QL_ID_CANAL_INVENTADO_2_MICRODRIVE 150



struct s_qltraps_fopen {

        /* Para archivos */
        FILE *qltraps_last_open_file_handler_unix;
        //z80_byte temp_qltraps_last_open_file_handler;

        //Usado al hacer fstat
        struct stat last_file_buf_stat;


        /* Para directorios */
        //usados al leer directorio
        //z80_byte qltraps_handler_filinfo_fattrib;
        struct dirent *qltraps_handler_dp;
        DIR *qltraps_handler_dfd; //    =NULL;
        //ultimo directorio leido al listar archivos
        char qltraps_handler_last_dir_open[PATH_MAX];

        //para telldir
        unsigned int contador_directorio;

        //para io.file. indica que la siguiente lectura debe retornar eof
        int next_eof_ptr_io_fline;

        //Indica que se ha abierto el dispositivo entero "mdv1_", "mdv2_",etc. Se usa en dir mdv1_
        int es_dispositivo;

        char ql_file_name[1024];


        /* Comun */
        //Indica a 1 que el archivo/directorio esta abierto. A 0 si no
        z80_bit open_file;

        z80_bit is_a_directory;
};

struct s_qltraps_fopen qltraps_fopen_files[QLTRAPS_MAX_OPEN_FILES];

//char ql_nombre_archivo_load[255];

//#define QLTRAPS_MAX_OPEN_FILES 64
//#define QLTRAPS_START_FILE_NUMBER 32

void qltraps_init_fopen_files_array(void)
{
	int i;
	for (i=0;i<QLTRAPS_MAX_OPEN_FILES;i++) {
		qltraps_fopen_files[i].open_file.v=0;
	}
}

//Ver si el numero del canal del fichero esta en el rango que gestiona este trap de emulacion
int qltrap_if_file_in_range(unsigned int channel)
{
	unsigned int rangomin=QLTRAPS_START_FILE_NUMBER;
	unsigned int rangomax=QLTRAPS_START_FILE_NUMBER+QLTRAPS_MAX_OPEN_FILES-1;
	if (channel<rangomin || channel>rangomax) return 0;
	return 1;
}


//Retorna indice al array. Si -1, no encontrado/no abierto
int qltraps_find_open_file(unsigned int channel)
{
	if (!qltrap_if_file_in_range(channel)) return -1;

	unsigned int indice=channel-QLTRAPS_START_FILE_NUMBER;
	if (qltraps_fopen_files[indice].open_file.v) return indice;
	else return -1;
}

//Si el id del canal del fichero esta abierto por nuestro gestor de traps de ql
/*int old_qltrap_if_file_open(unsigned int channel)
{
	debug_printf(VERBOSE_DEBUG,"Lets see if file %d has been opened by the emulator traps",channel);

	//Ver primero si el id del canal esta en el rango

	if (!qltrap_if_file_in_range(channel)) {
		debug_printf(VERBOSE_DEBUG,"File %d is out of range of ql traps",channel);
		return 0;
	}

	if (channel==OLD_QL_ID_CANAL_INVENTADO_MICRODRIVE) {
		debug_printf(VERBOSE_DEBUG,"File %d has been opened by the emulator traps",channel);
		return 1;
	}
	else {
		debug_printf(VERBOSE_DEBUG,"File %d has NOT been opened by the emulator traps",channel);
		return 0;
	}
}*/


//Retorna contador a array de estructura de archivo vacio. Retorna -1 si no hay
int qltraps_find_free_fopen(void)
{
	int i;

	for (i=0;i<QLTRAPS_MAX_OPEN_FILES;i++) {
		if (qltraps_fopen_files[i].open_file.v==0) {
			debug_printf (VERBOSE_DEBUG,"QL TRAPS: Free handle: %d",i+QLTRAPS_START_FILE_NUMBER);
			return i;
		}
	}

	return -1;
}




//Archivo que se est abiendo, cargando, etc. TODO: no soporta abrir mas de un archivo a ala vez
//char ql_full_path_load[PATH_MAX];

void ql_debug_force_breakpoint(char *message)
{
  catch_breakpoint_index=0;
  menu_breakpoint_exception.v=1;
  menu_abierto=1;
  sprintf (catch_breakpoint_message,"%s",message);
  printf ("Abrimos menu\n");
}

void core_ql_trap_one(void)
{

  //Ver pagina 173. 18.14 Trap Keys

  debug_printf (VERBOSE_PARANOID,"Trap 1. D0=%02XH D1=%02XH A0=%08XH A1=%08XH A6=%08XH PC=%05XH is : ",
    m68k_get_reg(NULL,M68K_REG_D0),m68k_get_reg(NULL,M68K_REG_D1),m68k_get_reg(NULL,M68K_REG_A0),
    m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A6),m68k_get_reg(NULL,M68K_REG_PC));

  switch(m68k_get_reg(NULL,M68K_REG_D0)) {

      case 0x00:
        debug_printf (VERBOSE_PARANOID,"Trap 1: MT.INF");
      break;

      case 0x01:
        debug_printf (VERBOSE_PARANOID,"Trap 1: MT.CJOB");
      break;

      case 0x0C:
        debug_printf (VERBOSE_PARANOID,"Trap 1: MT.ALLOC");
      break;  

      case 0x0D:
        debug_printf (VERBOSE_PARANOID,"Trap 1: MT.LNKFR");
      break;      

      case 0x10:
        debug_printf (VERBOSE_PARANOID,"Trap 1: MT.DMODE");
        //ql_debug_force_breakpoint("despues DMODE");
      break;

      case 0x11:
        debug_printf (VERBOSE_PARANOID,"Trap 1: MT.IPCOM");
      break;

      case 0x16:
        debug_printf (VERBOSE_PARANOID,"Trap 1: MT.ALBAS allocate BASIC area");
      break;

      case 0x17:
        debug_printf (VERBOSE_PARANOID,"Trap 1: MT.REBAS release BASIC area");
      break;


      default:
        debug_printf (VERBOSE_PARANOID,"Unknown trap");
      break;

    }

}

unsigned int pre_io_open_a[8];
unsigned int pre_io_open_d[8];

unsigned int pre_io_close_a[8];
unsigned int pre_io_close_d[8];

unsigned int pre_io_sstrg_a[8];
unsigned int pre_io_sstrg_d[8];

unsigned int pre_fs_headr_a[8];
unsigned int pre_fs_headr_d[8];

unsigned int pre_fs_load_a[8];
unsigned int pre_fs_load_d[8];


unsigned int pre_fs_mdinf_a[8];
unsigned int pre_fs_mdinf_d[8];


unsigned int pre_io_fline_a[8];
unsigned int pre_io_fline_d[8];

void ql_store_a_registers(unsigned int *destino, int ultimo)
{
  if (ultimo>=0) destino[0]=m68k_get_reg(NULL,M68K_REG_A0);
  if (ultimo>=1) destino[1]=m68k_get_reg(NULL,M68K_REG_A1);
  if (ultimo>=2) destino[2]=m68k_get_reg(NULL,M68K_REG_A2);
  if (ultimo>=3) destino[3]=m68k_get_reg(NULL,M68K_REG_A3);
  if (ultimo>=4) destino[4]=m68k_get_reg(NULL,M68K_REG_A4);
  if (ultimo>=5) destino[5]=m68k_get_reg(NULL,M68K_REG_A5);
  if (ultimo>=6) destino[6]=m68k_get_reg(NULL,M68K_REG_A6);
  if (ultimo>=7) destino[7]=m68k_get_reg(NULL,M68K_REG_A7);
}

void ql_store_d_registers(unsigned int *destino, int ultimo)
{
  if (ultimo>=0) destino[0]=m68k_get_reg(NULL,M68K_REG_D0);
  if (ultimo>=1) destino[1]=m68k_get_reg(NULL,M68K_REG_D1);
  if (ultimo>=2) destino[2]=m68k_get_reg(NULL,M68K_REG_D2);
  if (ultimo>=3) destino[3]=m68k_get_reg(NULL,M68K_REG_D3);
  if (ultimo>=4) destino[4]=m68k_get_reg(NULL,M68K_REG_D4);
  if (ultimo>=5) destino[5]=m68k_get_reg(NULL,M68K_REG_D5);
  if (ultimo>=6) destino[6]=m68k_get_reg(NULL,M68K_REG_D6);
  if (ultimo>=7) destino[7]=m68k_get_reg(NULL,M68K_REG_D7);
}



void ql_restore_a_registers(unsigned int *origen, int ultimo)
{
  if (ultimo>=0) m68k_set_reg(M68K_REG_A0,origen[0]);
  if (ultimo>=1) m68k_set_reg(M68K_REG_A1,origen[1]);
  if (ultimo>=2) m68k_set_reg(M68K_REG_A2,origen[2]);
  if (ultimo>=3) m68k_set_reg(M68K_REG_A3,origen[3]);
  if (ultimo>=4) m68k_set_reg(M68K_REG_A4,origen[4]);
  if (ultimo>=5) m68k_set_reg(M68K_REG_A5,origen[5]);
  if (ultimo>=6) m68k_set_reg(M68K_REG_A6,origen[6]);
  if (ultimo>=7) m68k_set_reg(M68K_REG_A7,origen[7]);
}


void ql_restore_d_registers(unsigned int *origen, int ultimo)
{
  if (ultimo>=0) m68k_set_reg(M68K_REG_D0,origen[0]);
  if (ultimo>=1) m68k_set_reg(M68K_REG_D1,origen[1]);
  if (ultimo>=2) m68k_set_reg(M68K_REG_D2,origen[2]);
  if (ultimo>=3) m68k_set_reg(M68K_REG_D3,origen[3]);
  if (ultimo>=4) m68k_set_reg(M68K_REG_D4,origen[4]);
  if (ultimo>=5) m68k_set_reg(M68K_REG_D5,origen[5]);
  if (ultimo>=6) m68k_set_reg(M68K_REG_D6,origen[6]);
  if (ultimo>=7) m68k_set_reg(M68K_REG_D7,origen[7]);
}


void core_ql_trap_two(void)
{

  //int reg_a0;

  //Ver pagina 173. 18.14 Trap Keys

  debug_printf (VERBOSE_PARANOID,"Trap 1. D0=%02XH A0=%08XH A1=%08XH PC=%05XH is : ",
    m68k_get_reg(NULL,M68K_REG_D0),m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_PC));

  switch(m68k_get_reg(NULL,M68K_REG_D0)) {

      case 1:
        debug_printf(VERBOSE_PARANOID,"Trap 1. IO.OPEN");
        //Open a channel. IO.OPEN Guardo todos registros A y D yo internamente de D2,D3,A2,A3 para restaurarlos despues de que se hace el trap de microdrive
        ql_store_a_registers(pre_io_open_a,7);
        ql_store_d_registers(pre_io_open_d,7);
      break;

      case 2:
        debug_printf(VERBOSE_PARANOID,"Trap 1. IO.CLOSE");
        ql_store_a_registers(pre_io_close_a,7);
        ql_store_d_registers(pre_io_close_d,7);
      break;

      default:
        debug_printf (VERBOSE_PARANOID,"Unknown trap");
      break;

    }

}

void ql_get_file_header(unsigned int indice_canal,unsigned int destino)
{
  /*
  pagina 38. 7.0 Directory Device Drivers
  Each file is assumed to have a 64-byte header (the logical beginning of file is set to byte 64, not byte zero). This header should be formatted as follows:

  00  long        file length
  04  byte        file access key (not yet implemented - currently always zero)
  05  byte        file type
  06  8 bytes     file type-dependent information
  0E  2+36 bytes  filename
  34 long         reserved for update date (not yet implemented)
  38 long         reserved for reference date (not yet implemented)
  3c long         reserved for backup date (not yet implemented)

  The current file types allowed are: 2, which is a relocatable object file;
  1, which is an executable program;
  255 is a directory;
  and 0 which is anything else

  In the case of file type 1,the first longword of type-dependent information holds
  the default size of the data space for the program.

  Ejecutable quiere decir binario??
  Si es un basic, es tipo 0?

  */

  debug_printf(VERBOSE_DEBUG,"Returning header for file on address %05XH",destino);

  //Inicializamos cabecera a 0
  int i;
  for (i=0;i<64;i++) ql_writebyte(destino+i,0);

  //unsigned int tamanyo=get_file_size(nombre);

	unsigned int tamanyo=qltraps_fopen_files[indice_canal].last_file_buf_stat.st_size;

  //Guardar tamanyo big endian
  ql_writebyte(destino+0,(tamanyo>>24)&255);
  ql_writebyte(destino+1,(tamanyo>>16)&255);
  ql_writebyte(destino+2,(tamanyo>>8)&255);
  ql_writebyte(destino+3,tamanyo&255);

  //Tipo
  ql_writebyte(destino+5,0); //ejecutable 1

  //Nombre. de momento me lo invento para ir rapido
  ql_writebyte(destino+0xe,0); //longitud nombre en big endian
  ql_writebyte(destino+0xf,4); //longitud nombre en big endian

  ql_writebyte(destino+0x10,'p');
  ql_writebyte(destino+0x11,'e');
  ql_writebyte(destino+0x12,'p');
  ql_writebyte(destino+0x13,'e');

  //Falta el "default size of the data space for the program" ?


}

void core_ql_trap_three(void)
{

//Ver pagina 173. 18.14 Trap Keys

  debug_printf (VERBOSE_PARANOID,"Trap 3. D0=%02XH A0=%08XH A1=%08XH A6=%08XH PC=%05XH is : ",
    m68k_get_reg(NULL,M68K_REG_D0),m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),
    m68k_get_reg(NULL,M68K_REG_A6),m68k_get_reg(NULL,M68K_REG_PC));

  switch(m68k_get_reg(NULL,M68K_REG_D0)) {
    case 0x2:
      debug_printf(VERBOSE_PARANOID,"Trap 3: IO.FLINE. fetch a line of bytes");
      	      //Guardar registros
      ql_store_a_registers(pre_io_fline_a,7);
      ql_store_d_registers(pre_io_fline_d,7);
    break;

    case 0x4:
      debug_printf (VERBOSE_PARANOID,"Trap 3: IO.EDLIN");
    break;

    case 0x7:
      debug_printf (VERBOSE_PARANOID,"Trap 3: IO.SSTRG");
      //Guardar registros
      ql_store_a_registers(pre_io_sstrg_a,7);
      ql_store_d_registers(pre_io_sstrg_d,7);
    break;

    case 0x45:
    	debug_printf (VERBOSE_PARANOID,"Trap 3: FS.MDINF");

    	      //Guardar registros
      ql_store_a_registers(pre_fs_mdinf_a,7);
      ql_store_d_registers(pre_fs_mdinf_d,7);
    break;

    
    break;

    case 0x47:
      debug_printf (VERBOSE_PARANOID,"Trap 3: FS.HEADR");
      //Guardar registros
      ql_store_a_registers(pre_fs_headr_a,7);
      ql_store_d_registers(pre_fs_headr_d,7);


    break;

    case 0x48:
      debug_printf (VERBOSE_PARANOID,"Trap 3: FS.LOAD. Lenght: %d Channel: %d Address: %05XH"
          ,m68k_get_reg(NULL,M68K_REG_D2),m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1)  );
      //D2.L length of file. A0 channellD. A1 base address for load

      //ql_debug_force_breakpoint("En FS.LOAD");

      //Guardar registros
      ql_store_a_registers(pre_fs_load_a,7);
      ql_store_d_registers(pre_fs_load_d,7);


    break;

    default:
      debug_printf (VERBOSE_PARANOID,"Trap 3: unknown");
    break;

  }
}

//Dado una ruta de QL tipo mdv1_programa , retorna mdv1 y programa por separados
void ql_split_path_device_name(char *ql_path, char *ql_device, char *ql_file)
{
  //Buscar hasta _
  int i;
  for (i=0;ql_path[i] && ql_path[i]!='_';i++);
  if (ql_path[i]==0) {
    //No encontrado
    ql_device[0]=0;
    ql_file[0]=0;
  }

  //Copiar desde inicio hasta aqui en ql_device
  int iorig=i;
  //Vamos del final hacia atras
  ql_device[i]=0;
  i--;
  char c;
  for (;i>=0;i--) {
    c=letra_minuscula(ql_path[i]);
    ql_device[i]=c;
  }

  //Restauramos indice y vamos de ahi+1 al final. Si nombre de archivo contiene un _, sustituir por .
  //Y pasar a minusculas todo
  i=iorig+1;
  int destino=0;

  for (;ql_path[i];i++,destino++) {
    c=letra_minuscula(ql_path[i]);
    if (c=='_') c='.'; //TODO: Deberia hacer eso solo para la primera _ desde la derecha
    ql_file[destino]=c;
  }

  ql_file[destino]=0;

  debug_printf(VERBOSE_DEBUG,"Source path: %s Device: %s File: %s",ql_path,ql_device,ql_file);
}

//Dado un device y un nombre de archivo, retorna ruta a archivo en filesystem final
//Retorna 1 si error
int ql_return_full_path(char *device, char *file, char *fullpath)
{
  char *sourcepath;

  if (!strcasecmp(device,"mdv1")) sourcepath=ql_mdv1_root_dir;
  else if (!strcasecmp(device,"mdv2")) sourcepath=ql_mdv2_root_dir;
  else if (!strcasecmp(device,"flp1")) sourcepath=ql_flp1_root_dir;
  else return 1;

  if (sourcepath[0])  sprintf(fullpath,"%s/%s",sourcepath,file);
  else sprintf(fullpath,"%s",file); //Ruta definida como vacia


  return 0;
}


//Dice si la ruta que se le ha pasado corresponde a un mdv1_, o mdv2_, o flp1_
int ql_si_ruta_parametro(char *texto,char *ruta)
{
  char *encontrado;

  encontrado=util_strcasestr(texto, ruta);
  if (encontrado) return 1;

  return 0;
}

//Dice si la ruta que se le ha pasado corresponde a un mdv1_, o mdv2_, o flp1_
int ql_si_ruta_mdv_flp(char *texto)
{
  char *encontrado;

  char *buscar_mdv1="mdv1_";
  encontrado=util_strcasestr(texto, buscar_mdv1);
  if (encontrado) return 1;

  char *buscar_mdv2="mdv2_";
  encontrado=util_strcasestr(texto, buscar_mdv2);
  if (encontrado) return 1;

  char *buscar_flp1="flp1_";
  encontrado=util_strcasestr(texto, buscar_flp1);
  if (encontrado) return 1;

  return 0;
}

//int temp_fs_line=0;



//FILE *ptr_io_fline=NULL;
//int next_eof_ptr_io_fline=0;

//Leer archivo linea a linea. Retorna bytes leidos, y valor de retorno. 
//Si hay eof, se debe retornar solo el eof y sin bytes leidos

unsigned int ql_read_io_fline(unsigned int canal,unsigned int puntero_destino,unsigned int *valor_retorno)
{

	FILE *ptr_archivo;

	//Si habia final de fichero, retornar solo eso
	if (qltraps_fopen_files[canal].next_eof_ptr_io_fline) {
		debug_printf (VERBOSE_PARANOID,"Returning eof");
		qltraps_fopen_files[canal].next_eof_ptr_io_fline=0;
		*valor_retorno=-10;
		return 0;
	}

	//Por defecto
	*valor_retorno=0;

	//Si no esta abierto el archivo, abrir
	/*if (ptr_io_fline==NULL) {
		debug_printf (VERBOSE_PARANOID,"Loading file at address %08XH",ql_full_path_load,puntero_destino);
		ptr_io_fline=fopen(ql_full_path_load,"rb");

		if (ptr_io_fline==NULL) {
			debug_printf(VERBOSE_PARANOID,"Error opening file %s",ql_full_path_load);
          		//Retornar Not complete (NC)

			*valor_retorno=-1;
			return 0; //bytes leidos 0
     		
		}

		next_eof_ptr_io_fline=0;
	}*/

	ptr_archivo=qltraps_fopen_files[canal].qltraps_last_open_file_handler_unix;

	unsigned int total_leidos=0;
	//Ir leyendo hasta codigo 10 o final de fichero
	int salir=0;

	while (!salir) {
		int bytes_leidos=fgetc(ptr_archivo);
		//Si negativo, asumimos final de fichero
		if (bytes_leidos<0) {
			qltraps_fopen_files[canal].next_eof_ptr_io_fline=1;
			salir=1;
		}

		if (!salir) {

			//printf ("Escribiendo byte %d (%c) direccion %XH\n",bytes_leidos,(bytes_leidos>32 && bytes_leidos<128 ? bytes_leidos : '.'),puntero_destino);

			ql_writebyte(puntero_destino++,bytes_leidos);
			total_leidos++;

		}

		//Si salto de linea, salir
		if (bytes_leidos==10) salir=1;
	}


	return total_leidos;
	

}


void ql_load_binary_file(FILE *ptr_file,unsigned int valor_leido_direccion, unsigned int valor_leido_longitud)
{




int leidos=1;
                                                z80_byte byte_leido;
                                                while (valor_leido_longitud>0 && leidos>0) {
                                                        leidos=fread(&byte_leido,1,1,ptr_file);
                                                        if (leidos>0) {
                                                                //poke_byte_no_time(valor_leido_direccion,byte_leido);
                                                                poke_byte_z80_moto(valor_leido_direccion,byte_leido);
                                                                valor_leido_direccion++;
                                                                valor_leido_longitud--;
                                                        }
                                                }
}


void ql_rom_traps(void)
{




    //Traps de intercepcion de teclado
    //F1 y F2 iniciales
    /*
    Para parar y simular F1:

sb 2 pc=4AF6H
set-register pc=4AF8H  (saltamos el trap 3)
set-register D1=E8H    (devolvemos tecla f1)
(o para simular F2: set-register D1=ECH)
  */

    //Si pulsado F1 o F2
    //Trap ya no hace falta. Esto era necesario cuando la lectura de teclado no funcionaba y esta era la unica manera de simular F1 o F2
    /*
    if ( get_pc_register()==0x4af6 &&
        ((ql_keyboard_table[0]&2)==0 || (ql_keyboard_table[0]&8)==0)
        )     {


          debug_printf(VERBOSE_DEBUG,"QL Trap ROM: Read F1 or F2");

      //Saltar el trap 3
      m68k_set_reg(M68K_REG_PC,0x4AF8);

      //Si F1
      if ((ql_keyboard_table[0]&2)==0) m68k_set_reg(M68K_REG_D1,0xE8);
      //Pues sera F2
      else m68k_set_reg(M68K_REG_D1,0xEC);

    }
*/



    //Saltar otro trap que hace mdv1 boot
    /*
    04B4C trap    #$2
04B4E tst.l   D0
04B50 rts


TRAP #2 D0=$1
Open a channel
IO.OPEN


A0=00004BE4 :

L04BE4 DC.W    $0009
       DC.B    'MDV1_BOOT'
       DC.B    $00


saltamos ese trap
set-register pc=04B50h

    */
    //TODO: saltar esta llamada de manera mas elegante
    /*if ( get_pc_register()==0x04B4C) {
      debug_printf(VERBOSE_DEBUG,"QL Trap ROM: Skipping MDV1 boot");
      m68k_set_reg(M68K_REG_PC,0x04B50);
    }*/


    /* Lectura de tecla */
    if (get_pc_register()==0x02D40) {
      //Decir que hay una tecla pulsada
      //Temp solo cuando se pulsa enter
      //if ((puerto_49150&1)==0) {
      if (ql_pulsado_tecla()) {
        //debug_printf(VERBOSE_DEBUG,"QL Trap ROM: Tell one key pressed");
        m68k_set_reg(M68K_REG_D7,0x01);
      }
      else {
        //printf ("no tecla pulsada\n");
        ql_mantenido_pulsada_tecla=0;
      }
    }

    //Decir 1 tecla pulsada
    if (get_pc_register()==0x02E6A) {
      //if ((puerto_49150&1)==0) {
      if (ql_pulsado_tecla()) {
        //debug_printf(VERBOSE_DEBUG,"QL Trap ROM: Tell 1 key pressed");
        m68k_set_reg(M68K_REG_D1,1);
        //L02E6A MOVE.B  D1,D5         * d1=d5=d4 : number of bytes in buffer
      }
      else {
        m68k_set_reg(M68K_REG_D1,0); //0 teclas pulsadas
      }
    }



/*
Info rapida de como funcionan los traps:
Detectar primero si se llama a trap 1, 2 o 3 , y llamar a core_ql_trap_one, two o three segun detectado
En esas funciones , cuando es alguna funcion de qdos que estamos gestionando, se guardan los registros A y D del Motorola, para su posterior uso

Esos traps en la rom acaban saltando a unas direcciones mas altas, y son las que posteriormente intercepto, 
con la funcion exacta del qdos, como ejemplo:


//Trap2, IO.OPEN
if (get_pc_register()==0x032B4 && m68k_get_reg(NULL,M68K_REG_D0)==1) {

Cuando salta ahi, los registros A y D se han modificado algunos desde que he detectado el trap, por eso los guardo antes.
Cuando se detecta la funcion exacta del qdos, como este con D0=1, restauro los registros que tenia salvados antes para obtener 
las variables de entrada (tal y como entraban al principio del trap). Con esos registros restaurados ya se qué hace la llamada a qdos.
Se realiza la función adecuada a esa llamada: abrir fichero, cerrar, leer, etc

Para volver del trap despues de haberlo interceptado, cambio el registro pc a una instruccion rte que hau en 53H de la rom
Ajusto también el stack de salida para que vuelva tal cual deberia del trap inicial (digamos que evito algun push y algun salto)
Y el registro D0 siempre contiene el codigo de error/ok de retorno del trap
Con esto ya se vuelve del trap: un tanto chapucero pero funciona

Esto probado con la rom ql_js.rom, con otras, es probable que falle.

*/



    if (get_pc_register()==0x0031e) {
      core_ql_trap_one();
    }





//00324 bsr     336
//Interceptar trap 2
/*
trap 2 salta a:
00324 bsr     336
*/
  if (get_pc_register()==0x00324) {
    core_ql_trap_two();
  }



    //Interceptar trap 3
    /*
    trap 3 salta a:
    0032A bsr     336
    */
    if (get_pc_register()==0x0032a) {
      core_ql_trap_three();
    }





    //Interceptar trap 2, con d0=1, cuando ya sabemos la direccion
    //IO.OPEN
/*
command@cpu-step> cs
PC: 032B4 SP: 2846E USP: 3FFC0 SR: 2000 :  S         A0: 0003FDEE A1: 0003EE00 A2: 00006906 A3: 00000670 A4: 00000012 A5: 0002846E A6: 00028000 A7: 0002846E D0: 00000001 D1: FFFFFFFF D2: 00000058 D3: 00000001 D4: 00000001 D5: 00000000 D6: 00000000 D7: 0000007F
032B4 subq.b  #1, D0

*/

  //Nota: lo normal seria que no hagamos este trap a no ser que se habilite emulacion de ql_microdrive_floppy_emulation.
  //Pero, si lo hacemos asi, si no habilitamos emulacion de micro&floppy, al pasar del menu de inicio (F1,F2) buscara el archivo BOOT, y como no salta el
  //trap, se queda bloqueado

    if (get_pc_register()==0x032B4 && m68k_get_reg(NULL,M68K_REG_D0)==1) {
      //en A0
      char ql_nombre_archivo_load[255];
      int reg_a0=m68k_get_reg(NULL,M68K_REG_A0);
      int longitud_nombre=peek_byte_z80_moto(reg_a0)*256+peek_byte_z80_moto(reg_a0+1);
      reg_a0 +=2;
      debug_printf (VERBOSE_PARANOID,"Lenght channel name: %d",longitud_nombre);

      char c;
      int i=0;
      for (;longitud_nombre;longitud_nombre--) {
        c=peek_byte_z80_moto(reg_a0++);
        ql_nombre_archivo_load[i++]=c;
        //printf ("%c",c);
      }
      //printf ("\n");
      ql_nombre_archivo_load[i++]=0;


      debug_printf (VERBOSE_PARANOID,"Channel name: %s",ql_nombre_archivo_load);
      



       


      //Hacer que si es mdv1_ ... volver

      //A7=0002846EH
      //A7=0002847AH
      //Incrementar A7 en 12
      //set-register pc=5eh. apunta a un rte

      int es_dispositivo=0;

      int hacer_trap=0;


      if (ql_si_ruta_mdv_flp(ql_nombre_archivo_load)) hacer_trap=1;

      if (!hacer_trap) {
      	if (
      		ql_si_ruta_parametro(ql_nombre_archivo_load,"mdv") ||
      		ql_si_ruta_parametro(ql_nombre_archivo_load,"flp")
      	    ) {
      		hacer_trap=1;
      		es_dispositivo=1;
      	}
      }
      

      if (hacer_trap) {

        debug_printf (VERBOSE_PARANOID,"Returning from trap without opening anything because file is mdv1, mdv2 or flp1");

        //ql_debug_force_breakpoint("En IO.OPEN");

/*
069CC movea.l A1, A0                              |L069CC MOVEA.L A1,A0
069CE move.w  (A6,A1.l), -(A7)                    |       MOVE.W  $00(A6,A1.L),-(A7)
069D2 trap    #$4                                 |       TRAP    #$04
>069D4 trap    #$2                                 |       TRAP    #$02
069D6 moveq   #$3, D3                             |       MOVEQ   #$03,D3
069D8 add.w   (A7)+, D3                           |       ADD.W   (A7)+,D3
069DA bclr    #$0, D3                             |       BCLR    #$00,D3
069DE add.l   D3, ($58,A6)                        |       ADD.L   D3,$0058(A6)

Es ese trap 2 el que se llama al hacer lbytes mdv...

Y entra asi:
command@cpu-step> run
Running until a breakpoint, menu opening or other event
PC: 069D4 SP: 3FFC0 USP: 3FFC0 SR: 0000 :

A0: 00000D88 A1: 00000D88 A2: 00006906 A3: 00000668 A4: 00000012 A5: 00000670 A6: 0003F068 A7: 0003FFC0 D0: 00000001 D1: FFFFFFFF D2: 00000058 D3: 00000001 D4: 00000001 D5: 00000000 D6: 00000000 D7: 00000000
069D4 trap    #$2

*/

        //D2,D3,A2,A3 se tienen que preservar, segun dice el trap.
        //Segun la info general de los traps, tambien se deben guardar de D4 a D7 y A4 a A6. Directamente guardo todos los D y A excepto A7

        ql_restore_d_registers(pre_io_open_d,7);
        ql_restore_a_registers(pre_io_open_a,6);
        //ql_restore_a_registers(pre_io_open_a,7);



        //Volver de ese trap
        m68k_set_reg(M68K_REG_PC,0x5e);
        //Ajustar stack para volver
        int reg_a7=m68k_get_reg(NULL,M68K_REG_A7);
        reg_a7 +=12;
        m68k_set_reg(M68K_REG_A7,reg_a7);





        //Como decir no error
        /*
        pg 11 qltm.pdf
        When the TRAP operation is complete, control is returned to the program at the location following the TRAP instruction,
        with an error key in all 32 bits of D0. This key is set to zero if the operation has been completed successfully,
        and is set to a negative number for any of the system-defined errors (see section 17.1 for a list of the meanings
        of the possible error codes). The key may also be set to a positive number, in which case that number is a pointer
        to an error string, relative to address $8000. The string is in the usual Qdos form of a word giving the length of
        the string, followed by the characters.
        */


        //No error.
        m68k_set_reg(M68K_REG_D0,0);

        char ql_io_open_device[PATH_MAX];
        char ql_io_open_file[PATH_MAX];

        char ql_nombrecompleto[PATH_MAX];

        if (!es_dispositivo) {

   	     ql_split_path_device_name(ql_nombre_archivo_load,ql_io_open_device,ql_io_open_file);

        	ql_return_full_path(ql_io_open_device,ql_io_open_file,ql_nombrecompleto);


        //Para siguientes io.fline
        //ptr_io_fline=NULL;

        	if (!si_existe_archivo(ql_nombrecompleto)) {
          		debug_printf(VERBOSE_PARANOID,"File %s not found",ql_nombrecompleto);
          		//Retornar Not found (NF)
          		m68k_set_reg(M68K_REG_D0,-7);
          		return;
        	}
	}

        

	//Metemos channel id (A0) inventado
	//m68k_set_reg(M68K_REG_A0,QL_ID_CANAL_INVENTADO_MICRODRIVE);

	//Obtenemos canal disponible
	int canal=qltraps_find_free_fopen();
	if (canal<0) {
		//No hay disponibles. Error.
  		m68k_set_reg(M68K_REG_D0,QDOS_ERROR_CODE_NC);
  		return;
	}

	//Se ha retornado indice al array. Canal sera sumando 
	m68k_set_reg(M68K_REG_A0,canal+QLTRAPS_START_FILE_NUMBER);


	//Resetear eof 
	qltraps_fopen_files[canal].next_eof_ptr_io_fline=0;


	strcpy(qltraps_fopen_files[canal].ql_file_name,ql_nombre_archivo_load);

	qltraps_fopen_files[canal].es_dispositivo=es_dispositivo;



	if (!es_dispositivo) {
		
		
	
		//Indicar file handle
		FILE *archivo;
		archivo=fopen(ql_nombrecompleto,"rb");
		if (archivo==NULL) {
        		debug_printf(VERBOSE_PARANOID,"File %s not found",ql_nombrecompleto);
	  		//Retornar Not found (NF)
  			m68k_set_reg(M68K_REG_D0,-7);
  			return;
		}


		qltraps_fopen_files[canal].qltraps_last_open_file_handler_unix=archivo;

		//Le hacemos un stat
		if (stat(ql_nombrecompleto, &qltraps_fopen_files[canal].last_file_buf_stat)!=0) {
			debug_printf (VERBOSE_DEBUG,"QLTRAPS handler: Unable to get status of file %s",ql_nombrecompleto);
		}

	}


	//Indicamos en array que esta abierto
	qltraps_fopen_files[canal].open_file.v=1;

	

        //D1= Job ID. TODO. Parece que da error "error in expression" porque no se asigna un job id valido?
        //Parece que D1 entra con -1, que quiere decir "the channel will be associated with the current job"
        //m68k_set_reg(M68K_REG_D1,0); //Valor de D1 inventado. Da igual, tambien fallara
        /*

        */

        return;

      }


      //Aqui se llama despues de hacer "load" de programa basic, hace IO.FLINE y luego hace IO.OPEN de "mdv" sin mas
      /*if (ql_si_ruta_parametro(ql_nombre_archivo_load,"mdv")) {

        debug_printf (VERBOSE_PARANOID,"Returning from trap without opening anything because file is mdv");

        ql_restore_d_registers(pre_io_open_d,7);
        ql_restore_a_registers(pre_io_open_a,6);
        //ql_restore_a_registers(pre_io_open_a,7);



        //Volver de ese trap
        m68k_set_reg(M68K_REG_PC,0x5e);
        //Ajustar stack para volver
        int reg_a7=m68k_get_reg(NULL,M68K_REG_A7);
        reg_a7 +=12;
        m68k_set_reg(M68K_REG_A7,reg_a7);
        

        //No error.
        m68k_set_reg(M68K_REG_D0,0);


        //Metemos channel id (A0) inventado. TODO: quiza otro canal diferente para estos casos??
        m68k_set_reg(M68K_REG_A0,QL_ID_CANAL_INVENTADO_2_MICRODRIVE);

        

        return;

      }*/

    }



    //IO.CLOSE
    if (get_pc_register()==0x032B4 && m68k_get_reg(NULL,M68K_REG_D0)==2) {


    	ql_restore_d_registers(pre_io_close_d,7);
        ql_restore_a_registers(pre_io_close_a,6);

    	debug_printf (VERBOSE_DEBUG,"IO.CLOSE. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) );

    	//Si canal es el segundo ficticio 
        /*if (m68k_get_reg(NULL,M68K_REG_A0)==QL_ID_CANAL_INVENTADO_2_MICRODRIVE) {
      		debug_printf (VERBOSE_DEBUG,"Returning IO.CLOSE from our second microdrive channel without error");

       	 	ql_restore_d_registers(pre_io_close_d,7);
        	ql_restore_a_registers(pre_io_close_a,6);
       


        	//Volver de ese trap
        	m68k_set_reg(M68K_REG_PC,0x5e);
        	//Ajustar stack para volver
        	int reg_a7=m68k_get_reg(NULL,M68K_REG_A7);
        	reg_a7 +=12;
        	m68k_set_reg(M68K_REG_A7,reg_a7);


        	//No error.
        	m68k_set_reg(M68K_REG_D0,0);

        	return;


       }*/
      	//Si canal es el mio ficticio 
       	int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));

        if (indice_canal>=0  ) {
        	debug_printf (VERBOSE_DEBUG,"Closing file/device %s",qltraps_fopen_files[indice_canal].ql_file_name);

        	//Si no es dispositivo, fclose
        	if (!qltraps_fopen_files[indice_canal].es_dispositivo) {
        		fclose(qltraps_fopen_files[indice_canal].qltraps_last_open_file_handler_unix);
        	}

        	//Liberar ese item del array
        	qltraps_fopen_files[indice_canal].open_file.v=0;


        	//Volver de ese trap
        	m68k_set_reg(M68K_REG_PC,0x5e);
        	//Ajustar stack para volver
        	int reg_a7=m68k_get_reg(NULL,M68K_REG_A7);
        	reg_a7 +=12;
        	m68k_set_reg(M68K_REG_A7,reg_a7);


        	//No error.
        	m68k_set_reg(M68K_REG_D0,0);


       }

    }






    //Quiza Trap 3 FS.HEADR acaba saltando a 0337C move.l  A0, D7
    if (get_pc_register()==0x0337C && m68k_get_reg(NULL,M68K_REG_D0)==0x47 && ql_microdrive_floppy_emulation) {
        debug_printf (VERBOSE_PARANOID,"FS.HEADR. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) );

        //Si canal es el mio ficticio 100
        int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        if (indice_canal>=0 ) {
          //Devolver cabecera. Se supone que el sistema operativo debe asignar espacio para la cabecera? Posiblemente si.
          //Forzamos meter cabecera en espacio de memoria de pantalla a ver que pasa
          //ql_get_file_header(ql_full_path_load,m68k_get_reg(NULL,M68K_REG_A1));

          //TODO completar esto
          ql_get_file_header(indice_canal,m68k_get_reg(NULL,M68K_REG_A1));

          //ql_get_file_header(ql_nombre_archivo_load,131072); //131072=pantalla

          ql_restore_d_registers(pre_fs_headr_d,7);
          ql_restore_a_registers(pre_fs_headr_a,6);

          //Volver de ese trap
          m68k_set_reg(M68K_REG_PC,0x5e);
          unsigned int reg_a7=m68k_get_reg(NULL,M68K_REG_A7);
          reg_a7 +=12;
          m68k_set_reg(M68K_REG_A7,reg_a7);

          //No error.
          m68k_set_reg(M68K_REG_D0,0);

          //D1.W length of header read. A1 top of read buffer
          m68k_set_reg(M68K_REG_D1,64);

          //Decimos que A1 es A1 top of read buffer
          unsigned int reg_a1=m68k_get_reg(NULL,M68K_REG_A1);
          reg_a1 +=64;
          m68k_set_reg(M68K_REG_A1,reg_a1);


          //Le decimos en A1 que la cabecera esta en la memoria de pantalla
          //m68k_set_reg(M68K_REG_A1,131072);

        }
    }

    //Trap 3 FS.MDINF 
    if (get_pc_register()==0x0337C && m68k_get_reg(NULL,M68K_REG_D0)==0x45 && ql_microdrive_floppy_emulation) {
        debug_printf (VERBOSE_PARANOID,"FS.MDINF. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) );

        //Si canal es el mio ficticio 100
        int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        if (indice_canal>=0 ) {

        	//printf ("Mi canal MDINF\n");


        	ql_restore_d_registers(pre_fs_mdinf_d,7);
          	ql_restore_a_registers(pre_fs_mdinf_a,6);

        	//Retornamos :
        	//D1.L empty/good sectors. The number of empty sectors is in the most significant word (msw) of D1, 
        	//the total available on the medium is in the least significant word (lsw). A sector is 512 bytes.
        	//de momento ,MDV files in QLAY format. Thee files must be exactly 174930 bytes 174930/512->aprox 341
        	m68k_set_reg(M68K_REG_D1,341); //0 sectores libres, 341 sectores ocupados

        	
          
          //Volver de ese trap
          m68k_set_reg(M68K_REG_PC,0x5e);
          unsigned int reg_a7=m68k_get_reg(NULL,M68K_REG_A7);
          reg_a7 +=12;
          m68k_set_reg(M68K_REG_A7,reg_a7);

          //No error.
          m68k_set_reg(M68K_REG_D0,0);

          //D1.W length of header read. A1 top of read buffer
          //m68k_set_reg(M68K_REG_D1,64);

          //Devolver medium name
          //A1 end of medium name  (entrada: A1 ptr to 10 byte buffer)

          unsigned int reg_a1=m68k_get_reg(NULL,M68K_REG_A1);
          unsigned int puntero=reg_a1+m68k_get_reg(NULL,M68K_REG_A6);

          reg_a1 +=10;
          m68k_set_reg(M68K_REG_A1,reg_a1);

          ql_writebyte(puntero++,'Z');
          ql_writebyte(puntero++,'E');
          ql_writebyte(puntero++,'s');
          ql_writebyte(puntero++,'a');
          ql_writebyte(puntero++,'r'); //5
          ql_writebyte(puntero++,'U');
          ql_writebyte(puntero++,'X');
          ql_writebyte(puntero++,'M');
          ql_writebyte(puntero++,'D');
          ql_writebyte(puntero++,' '); //10





        }
    }




	//Trap 3 IO.FLINE
    if (get_pc_register()==0x0337C && m68k_get_reg(NULL,M68K_REG_D0)==0x2 && ql_microdrive_floppy_emulation) {
        debug_printf (VERBOSE_PARANOID,"IO.FLINE. Channel ID=%d Base of buffer A1=%08XH A3=%08XH A6=%08XH",
        		m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),m68k_get_reg(NULL,M68K_REG_A6) );


        //Si canal es el segundo ficticio 100
        /*if (m68k_get_reg(NULL,M68K_REG_A0)==QL_ID_CANAL_INVENTADO_2_MICRODRIVE) {
        	debug_printf (VERBOSE_DEBUG,"Returning IO.FLINE from second microdrive channel (just \"mdv\") with EOF");
        	m68k_set_reg(M68K_REG_D0,-10);
          	debug_printf (VERBOSE_DEBUG,"IO.FLINE - returning EOF");
          	m68k_set_reg(M68K_REG_D1,0);  //0 byte leido

          	return;
        }*/

        //Si canal es el mio ficticio 100
        int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        if (indice_canal>=0) {

        	
        	debug_printf (VERBOSE_PARANOID,"Returning IO.FLINE from our microdrive channel without error");


        	//Si es un dispositivo entero
        	if (qltraps_fopen_files[indice_canal].es_dispositivo) {
        		debug_printf (VERBOSE_DEBUG,"Returning IO.FLINE from full device channel (just \"%s\") with EOF",
        			qltraps_fopen_files[indice_canal].ql_file_name);

        		m68k_set_reg(M68K_REG_D0,QDOS_ERROR_CODE_EF);
          		debug_printf (VERBOSE_DEBUG,"IO.FLINE - returning EOF");
          		m68k_set_reg(M68K_REG_D1,0);  //0 byte leido
      			return;
        	}

        	 //Indicar actividad en md flp
        	ql_footer_mdflp_operating();


          	/*
          	D0=$2 IO.FLINE fetch a line of characters terminated by ASCII <LF> ($A)
		D0=$3 IO.FSTRG fetch a string of bytes
          	*/

          	/*
          	Entrada:
          	D2.W length of buffer
          	D3.W timeout
          	A0 channel ID
          	A1 base of buffer

          	Salida:
          	D1.W nr. of bytes fetched
          	A1 updated ptr to buffer

          	Errores:
          	NC not complete
          	NO channel not open
          	EF end of file
          	B0 buffer overflow (fetch line only)

          	*/

        	//Dudas!! Donde se guarda los datos leidos? En A1+A6??
        	//Registro de salida A1 a donde debe apuntar??
        	//unsigned int puntero_destino=m68k_get_reg(NULL,M68K_REG_A1)+m68k_get_reg(NULL,M68K_REG_A6);

        	//O a A1 a secas
        	//depende de si se ha llamado trap4 o no

          	ql_restore_d_registers(pre_io_fline_d,7);
          	ql_restore_a_registers(pre_io_fline_a,6);

          	unsigned int puntero_destino=m68k_get_reg(NULL,M68K_REG_A1)+m68k_get_reg(NULL,M68K_REG_A6);


          	debug_printf (VERBOSE_DEBUG,"IO.FLINE - Channel ID=%d Base of buffer A1=%08XH A3=%08XH A6=%08XH dest pointer: %08XH",
        		m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),
        		m68k_get_reg(NULL,M68K_REG_A6),puntero_destino );


                  

          	unsigned int valor_retorno;
          	unsigned int leidos=ql_read_io_fline(indice_canal,puntero_destino,&valor_retorno);

          	

          	m68k_set_reg(M68K_REG_D0,valor_retorno);

          	unsigned int registro_a1=m68k_get_reg(NULL,M68K_REG_A1);
          	registro_a1 +=leidos;
          	m68k_set_reg(M68K_REG_A1,registro_a1);

          	//printf ("Leidos: %d\n",leidos);
          	m68k_set_reg(M68K_REG_D1,leidos);

  	

        	
          
          //Volver de ese trap
          m68k_set_reg(M68K_REG_PC,0x5e);
          unsigned int reg_a7=m68k_get_reg(NULL,M68K_REG_A7);
          reg_a7 +=12;
          m68k_set_reg(M68K_REG_A7,reg_a7);


         



        }
    }



//Trap 3 IO.SSTRG
    if (get_pc_register()==0x0337C && m68k_get_reg(NULL,M68K_REG_D0)==0x7 && ql_microdrive_floppy_emulation) {
        debug_printf (VERBOSE_PARANOID,"IO.SSTRG. Channel ID=%d Base of buffer A1=%08XH A3=%08XH A6=%08XH D2=%08XH",
        		m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),
        		m68k_get_reg(NULL,M68K_REG_A6),m68k_get_reg(NULL,M68K_REG_D2) );

        //Si canal es el mio ficticio 100
        int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        if (indice_canal>=0 ) {

        	
        	debug_printf (VERBOSE_PARANOID,"Returning IO.SSTRG from our microdrive channel without error");




          	/*
          	Entrada:
          	D2.W nr of bytes to be sent
          	D3.W timeout
          	A0 channel ID
          	A1 base of buffer

          	Salida:
          	D1.W nr. of bytes sent
          	A1 updated ptr to buffer

          	Errores:
          	NC not complete
          	NO channel not open
          	EF end of file
          	B0 buffer overflow (fetch line only)

          	*/

        	
        	

        	//O a A1 a secas
        	//depende de si se ha llamado trap4 o no

          	ql_restore_d_registers(pre_io_sstrg_d,7);
          	ql_restore_a_registers(pre_io_sstrg_a,6);

          	unsigned int puntero_origen=m68k_get_reg(NULL,M68K_REG_A1)+m68k_get_reg(NULL,M68K_REG_A6);


          	debug_printf (VERBOSE_PARANOID,"IO.SSTRG - restoreg registers. Channel ID=%d Base of buffer A1=%08XH A3=%08XH A6=%08XH D2=%08XH",
        		m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),
        		m68k_get_reg(NULL,M68K_REG_A6),m68k_get_reg(NULL,M68K_REG_D2) );


        

          	//Mostrar parte del mensaje enviado en SSTRG
          	//unsigned int puntero_destino=m68k_get_reg(NULL,M68K_REG_A1)+m68k_get_reg(NULL,M68K_REG_A6);
          	char buffer_mensaje[256];
          	int longitud=m68k_get_reg(NULL,M68K_REG_D2);
          	if (longitud>32) longitud=32;

          	int i=0;
          	char byte_leido;

          	while (longitud) {
          		byte_leido=ql_readbyte(puntero_origen);
          		if (byte_leido>=32) {
          			buffer_mensaje[i]=byte_leido;
          			i++;
          		}
          		else {
          			sprintf(&buffer_mensaje[i],"%02XH ",byte_leido);

          			i+=4;
          		}
          		
          		puntero_origen++;
          	}

          	buffer_mensaje[i]=0;

          	debug_printf (VERBOSE_PARANOID,"IO.SSTRG - message sent: %s",buffer_mensaje);
          

          	//bytes enviados 
          	m68k_set_reg(M68K_REG_D1,m68k_get_reg(NULL,M68K_REG_D2) );


          	//Aumentar puntero A1
          	unsigned int registro_a1=m68k_get_reg(NULL,M68K_REG_A1);
          	registro_a1 +=m68k_get_reg(NULL,M68K_REG_D2);
          	m68k_set_reg(M68K_REG_A1,registro_a1);



        	  //No error. 
          	m68k_set_reg(M68K_REG_D0,0);

  	

        	
          
          //Volver de ese trap
          m68k_set_reg(M68K_REG_PC,0x5e);
          unsigned int reg_a7=m68k_get_reg(NULL,M68K_REG_A7);
          reg_a7 +=12;
          m68k_set_reg(M68K_REG_A7,reg_a7);



        }
    }



    //FS.LOAD
    if (get_pc_register()==0x0337C && m68k_get_reg(NULL,M68K_REG_D0)==0x48 && ql_microdrive_floppy_emulation) {
        debug_printf (VERBOSE_PARANOID,"FS.LOAD. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) );

        //Si canal es el mio ficticio 100
        int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        if (indice_canal>=0 ) {

          ql_restore_d_registers(pre_fs_load_d,7);
          ql_restore_a_registers(pre_fs_load_a,6);

          unsigned int longitud=m68k_get_reg(NULL,M68K_REG_D2);


            debug_printf (VERBOSE_PARANOID,"Loading file at address %05XH with lenght: %d",m68k_get_reg(NULL,M68K_REG_A1),longitud);
            //void load_binary_file(char *binary_file_load,int valor_leido_direccion,int valor_leido_longitud)

             //Indicar actividad en md flp
        	ql_footer_mdflp_operating();

            //longitud la saco del propio archivo, ya que no me llega bien de momento pues no retornaba bien fs.headr
            //int longitud=get_file_size(ql_nombre_archivo_load);
        	FILE *ptr_file;
        	ptr_file=qltraps_fopen_files[indice_canal].qltraps_last_open_file_handler_unix;
            ql_load_binary_file(ptr_file,m68k_get_reg(NULL,M68K_REG_A1),longitud);



          //Volver de ese trap
          m68k_set_reg(M68K_REG_PC,0x5e);
          unsigned int reg_a7=m68k_get_reg(NULL,M68K_REG_A7);
          reg_a7 +=12;
          m68k_set_reg(M68K_REG_A7,reg_a7);

          //No error.
          m68k_set_reg(M68K_REG_D0,0);

          //m68k_set_reg(M68K_REG_D0,-7);


          //Decimos que A1 es A1 top address after load
          unsigned int reg_a1=m68k_get_reg(NULL,M68K_REG_A1);
          reg_a1 +=longitud;
          m68k_set_reg(M68K_REG_A1,reg_a1);


          //Le decimos en A1 que la cabecera esta en la memoria de pantalla
          //m68k_set_reg(M68K_REG_A1,131072);

        }
    }


}
