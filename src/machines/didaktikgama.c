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
#include <time.h>

#include "cpu.h"
#include "didaktikgama.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "zxvision.h"
#include "screen.h"
#include "ula.h"
#include "operaciones.h"
#include "start.h"




//Donde esta cada pagina de ram
z80_byte *didaktikgama_ram_mem_table[DIDAKTIKGAMA_TOTAL_RAM_PAGES];


//Direcciones actuales mapeadas
z80_byte *didaktikgama_memory_paged[4];

/*
 * Modelo del 8255 integrado en Didaktik Gama
 *
 * La ruta de E/S de operaciones.c llama a estas funciones para los puertos
 * oficiales 1Fh (A), 3Fh (B), 5Fh (C) y 7Fh (control). El byte alto de la
 * direccion del Z80 no cambia cual de los cuatro registros se selecciona.
 *
 * port_a, port_b y port_c son los latches escritos por la CPU. Un OUT guarda
 * el dato aunque el puerto sea de entrada; en ese caso el latch no conduce
 * los pines. input_a, input_b e input_c contienen los niveles que presenta
 * un dispositivo externo. Sin dispositivo conectado se inicializan a cero.
 * set_input_pins() permite cambiar esos niveles y get_output_pins() devuelve
 * los valores de salida junto con una mascara de los pines que los conducen.
 * Asi se mantienen separados el dato escrito y el nivel fisico de cada pin.
 *
 * mode conserva la ultima palabra de configuracion escrita en 7Fh con D7=1.
 * En modo 0, A y B tienen una direccion cada uno; C tiene direcciones
 * independientes para PC0-PC3 y PC4-PC7. Un IN lee los pines externos si
 * son entradas, o los latches si son salidas. Una nueva palabra de modo
 * pone a cero los latches de salida y el estado de handshake. Tras reset,
 * 9Bh configura los tres puertos como entradas. port_d guarda el ultimo
 * comando de control. El MHB8255 no ofrece un registro de control legible;
 * esta emulacion devuelve FFh para IN 7Fh.
 *
 * Un OUT a 7Fh con D7=0 es un comando BSR: D3-D1 eligen un bit del latch C
 * y D0 lo pone a uno o a cero. BSR no cambia las direcciones ni el modo.
 * En modos 1 y 2, ciertos BSR controlan tambien las habilitaciones INTE.
 *
 * En modo 1, A y B usan lineas de C para STB, ACK, IBF, OBF e INTR.
 * latch_a y latch_b guardan los bytes recibidos al activarse STB; los flags
 * ibf y obf indican el estado de las transferencias. En modo 2, A combina
 * entrada y salida con handshake; B conserva su modo independiente.
 * set_input_pins() detecta las transiciones externas de STB y ACK. Un IN
 * del puerto de entrada consume el byte recibido y libera su indicador IBF.
 *
 * PC0 selecciona los dos bancos de 32 KiB visibles en 8000h-FFFFh. En
 * modo 0 del grupo B, si PC0 es salida manda el bit 0 del latch C; si es
 * entrada, la resistencia a tierra del Gama impone banco 0. En modo 1 de B,
 * PC0 es INTR_B y su nivel selecciona el banco. Se actualiza el mapa al
 * cambiar la configuracion, escribir C o BSR, o cambiar el handshake de B.
 * La ROM y la RAM fija de 4000h-7FFFh no se paginan ni se copian.
 *
 * Ejemplo Z80: configurar A y B como entradas y C como salida; despues
 * seleccionar primero el banco 1 y luego el banco 0:
 *
 *     LD BC,007FH     ; Registro de control.
 *     LD A,092H       ; Modo 0: A/B entrada, C completo salida.
 *     OUT (C),A
 *     LD BC,005FH     ; Puerto C.
 *     LD A,001H       ; PC0=1: banco 1.
 *     OUT (C),A
 *     XOR A           ; PC0=0: banco 0.
 *     OUT (C),A
 *
 * El mismo cambio de banco puede hacerse sin escribir todo C: con BC=007FH,
 * cargar A=1 y ejecutar OUT (C),A activa PC0 por BSR; con A=0 lo desactiva.
 * Si se configura 9Bh (todos los puertos como entradas), OUT al puerto C o
 * BSR sobre PC0 solo modifica el latch; la resistencia a tierra mantiene
 * seleccionado el banco 0.
 *
 * Ejemplo de un dispositivo externo en modo 1 de B: escribir 86h en 7Fh
 * pone B como entrada con handshake; escribir 05h en 7Fh habilita INTE_B
 * mediante BSR de PC2.
 * El dispositivo presenta un byte en B y lleva PC2 (STB_B) de 1 a 0:
 * set_input_pins() lo copia a latch_b y activa IBF_B. Al devolver PC2 a 1,
 * INTR_B pasa a 1 y, por estar conectado a PC0, se selecciona el banco 1.
 * Un IN de 3Fh devuelve el byte recibido, limpia IBF_B y restaura el banco 0.
 * El programa que atienda esa interrupcion debe ejecutar el codigo desde
 * ROM o RAM fija si necesita seguir siendo visible durante la paginacion.
 *
 * Nota sobre BASIC: la ROM didaktikgama.rom incluida aqui se identifica como
 * Didaktik 1989. Su tabla de comandos envia OUT a la rutina de 38E7h. Esta
 * ejecuta el OUT solicitado y compara PC0 antes y despues leyendo 5Fh. Si
 * PC0 cambia y RAMTOP (5CB2h) alcanza 8000h, la rutina alterna los bancos
 * mediante 7Fh y copia datos de la zona paginada. Por tanto, la proteccion
 * puede activarse tanto con OUT 95 como con OUT 127 desde BASIC, siempre que
 * la escritura cambie PC0. Es codigo de la ROM ejecutado por la CPU emulada;
 * el 8255 solo selecciona el banco y nunca copia memoria por su cuenta.
 */
z80_byte didaktikgama_8255_port_a=0;
z80_byte didaktikgama_8255_port_b=0;
z80_byte didaktikgama_8255_port_c=0;
z80_byte didaktikgama_8255_port_d=0;
static z80_byte didaktikgama_8255_mode=0x9b;
// Niveles de entrada sin periféricos conectados al MHB8255 NMOS.
static z80_byte didaktikgama_8255_input_a=0;
static z80_byte didaktikgama_8255_input_b=0;
static z80_byte didaktikgama_8255_input_c=0;
static z80_byte didaktikgama_8255_latch_a=0;
static z80_byte didaktikgama_8255_latch_b=0;
static z80_bit didaktikgama_8255_ibf_a={0};
static z80_bit didaktikgama_8255_ibf_b={0};
static z80_bit didaktikgama_8255_obf_a={0};
static z80_bit didaktikgama_8255_obf_b={0};
static z80_bit didaktikgama_8255_ack_a={0};
static z80_bit didaktikgama_8255_ack_b={0};
static z80_bit didaktikgama_8255_inte_a_in={0};
static z80_bit didaktikgama_8255_inte_a_out={0};
static z80_bit didaktikgama_8255_inte_b={0};

static int didaktikgama_8255_mode_a(void)
{
    int mode=(didaktikgama_8255_mode >> 5) & 3;
    return mode>1 ? 2 : mode;
}

static int didaktikgama_8255_intr_a(void)
{
    int mode=didaktikgama_8255_mode_a();
    int input=(mode==2 || (didaktikgama_8255_mode & 0x10));
    int output=(mode==2 || !(didaktikgama_8255_mode & 0x10));
    return mode && ((input && didaktikgama_8255_inte_a_in.v &&
                     didaktikgama_8255_ibf_a.v && (didaktikgama_8255_input_c & 0x10)) ||
                    (output && didaktikgama_8255_inte_a_out.v &&
                     didaktikgama_8255_ack_a.v && (didaktikgama_8255_input_c & 0x40)));
}

static int didaktikgama_8255_intr_b(void)
{
    if (!(didaktikgama_8255_mode & 4) || !didaktikgama_8255_inte_b.v) return 0;
    if (didaktikgama_8255_mode & 2)
        return didaktikgama_8255_ibf_b.v && (didaktikgama_8255_input_c & 4);
    return didaktikgama_8255_ack_b.v && (didaktikgama_8255_input_c & 4);
}

static z80_byte didaktikgama_8255_read_c(void)
{
    z80_byte mask=(didaktikgama_8255_mode & 8) ? 0 : 0xf0;
    z80_byte value;
    int mode_a=didaktikgama_8255_mode_a();

    if (!(didaktikgama_8255_mode & 1)) mask |= 0x0f;
    value=(didaktikgama_8255_port_c & mask) |
          (didaktikgama_8255_input_c & ~mask & 0xfe);

    if (mode_a) {
        value=(value & ~0x08) | (didaktikgama_8255_intr_a() ? 0x08 : 0);
        if (mode_a==2 || (didaktikgama_8255_mode & 0x10)) {
            value=(value & ~0x30) | (didaktikgama_8255_input_c & 0x10) |
                  (didaktikgama_8255_ibf_a.v ? 0x20 : 0);
        }
        if (mode_a==2 || !(didaktikgama_8255_mode & 0x10)) {
            value=(value & ~0xc0) | (didaktikgama_8255_input_c & 0x40) |
                  (didaktikgama_8255_obf_a.v ? 0 : 0x80);
        }
    }
    if (didaktikgama_8255_mode & 4) {
        value=(value & ~0x07) | (didaktikgama_8255_intr_b() ? 1 : 0) |
              (didaktikgama_8255_input_c & 4);
        if (didaktikgama_8255_mode & 2)
            value |= didaktikgama_8255_ibf_b.v ? 2 : 0;
        else value |= didaktikgama_8255_obf_b.v ? 0 : 2;
    }
    return value;
}

void didaktikgama_init_memory_tables(void)
{
    //Primero rom y luego ram

    int i;
    z80_byte *puntero=memoria_spectrum;

    //saltar la rom
    puntero +=16384;

    //saltamos la ram 16384-32767
    puntero +=16384;


    for (i=0;i<DIDAKTIKGAMA_TOTAL_RAM_PAGES;i++) {
        didaktikgama_ram_mem_table[i]=puntero;
        puntero +=32768;
    }


}

void didaktikgama_set_normal_pages(void)
{
    didaktikgama_memory_paged[0]=memoria_spectrum;
    didaktikgama_memory_paged[1]=&memoria_spectrum[16384];
    didaktikgama_memory_paged[2]=didaktikgama_ram_mem_table[0];
    didaktikgama_memory_paged[3]=didaktikgama_ram_mem_table[0]+16384;

    // Solo la RAM fija de pantalla en 4000h-7FFFh tiene contienda con la ULA.
    contend_pages_actual[0]=0;
    contend_pages_actual[1]=1;
    contend_pages_actual[2]=0;
    contend_pages_actual[3]=0;

}

void didaktikgama_malloc_mem_machine(void)
{
    //16 KB ROM, 80 KB RAM

    malloc_machine((16+DIDAKTIKGAMA_RAM_SIZE)*1024);
    random_ram(memoria_spectrum+DIDAKTIKGAMA_ROM_SIZE*1024,DIDAKTIKGAMA_RAM_SIZE*1024);

    didaktikgama_init_memory_tables();
    didaktikgama_set_normal_pages();
}

void didaktikgama_mem_page_ram(void)
{

    // PC0 tiene una resistencia a tierra cuando se configura como entrada.
    z80_byte page_entra=(didaktikgama_8255_mode & 4) ? didaktikgama_8255_intr_b() :
                          ((didaktikgama_8255_mode & 1) ? 0 : (didaktikgama_8255_port_c & 1));

    didaktikgama_memory_paged[2]=didaktikgama_ram_mem_table[page_entra];
    didaktikgama_memory_paged[3]=didaktikgama_ram_mem_table[page_entra]+16384;

    printf("Paging ram %d on 8000h-ffffh\n",page_entra);

}

void didaktikgama_reset(void)
{
    didaktikgama_8255_port_a=0;
    didaktikgama_8255_port_b=0;
    didaktikgama_8255_port_c=0;
    didaktikgama_8255_port_d=0;
    didaktikgama_8255_mode=0x9b;
    didaktikgama_8255_latch_a=0;
    didaktikgama_8255_latch_b=0;
    didaktikgama_8255_ibf_a.v=didaktikgama_8255_ibf_b.v=0;
    didaktikgama_8255_obf_a.v=didaktikgama_8255_obf_b.v=0;
    didaktikgama_8255_ack_a.v=didaktikgama_8255_ack_b.v=0;
    didaktikgama_8255_inte_a_in.v=didaktikgama_8255_inte_a_out.v=didaktikgama_8255_inte_b.v=0;
    didaktikgama_set_normal_pages();

}





void poke_byte_no_time_didaktikgama(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

    if (dir>16383) {
		poke_byte_spectrum_sync_rainbow(dir);
#ifdef EMULATE_VISUALMEM

        set_visualmembuffer(dir);

#endif
        dir = dir & 16383;
        puntero=didaktikgama_memory_paged[segmento]+dir;

        *puntero=valor;
    }
}


void poke_byte_didaktikgama(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

#ifdef EMULATE_CONTEND
    if (contend_pages_actual[segmento]) {
        //printf ("t_estados: %d\n",t_estados);
        t_estados += contend_table[ t_estados ];
    }
#endif
    t_estados += 3;



    if (dir>16383) {
		poke_byte_spectrum_sync_rainbow(dir);
#ifdef EMULATE_VISUALMEM

        set_visualmembuffer(dir);

#endif
		dir = dir & 16383;
		puntero=didaktikgama_memory_paged[segmento]+dir;

//		printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
		*puntero=valor;
	}
}


z80_byte peek_byte_no_time_didaktikgama(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

	dir = dir & 16383;
	puntero=didaktikgama_memory_paged[segmento]+dir;

	return *puntero;
}


z80_byte peek_byte_didaktikgama(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

#ifdef EMULATE_CONTEND
    if (contend_pages_actual[segmento]) {
            t_estados += contend_table[ t_estados ];
    }
#endif
    t_estados += 3;



	dir = dir & 16383;
	puntero=didaktikgama_memory_paged[segmento]+dir;

	return *puntero;
}


void didaktikgama_8255_set_input_pins(z80_byte a,z80_byte b,z80_byte c)
{
    z80_byte old_c=didaktikgama_8255_input_c;
    int mode_a=didaktikgama_8255_mode_a();

    didaktikgama_8255_input_a=a;
    didaktikgama_8255_input_b=b;
    didaktikgama_8255_input_c=c;

    if (mode_a==2 || (mode_a==1 && (didaktikgama_8255_mode & 0x10))) {
        if ((old_c & 0x10) && !(c & 0x10)) {
            didaktikgama_8255_latch_a=a;
            didaktikgama_8255_ibf_a.v=1;
        }
    }
    if (mode_a==2 || (mode_a==1 && !(didaktikgama_8255_mode & 0x10))) {
        if ((old_c & 0x40) && !(c & 0x40)) {
            didaktikgama_8255_obf_a.v=0;
            didaktikgama_8255_ack_a.v=1;
        }
    }
    if (didaktikgama_8255_mode & 4) {
        if (didaktikgama_8255_mode & 2) {
            if ((old_c & 4) && !(c & 4)) {
                didaktikgama_8255_latch_b=b;
                didaktikgama_8255_ibf_b.v=1;
            }
        }
        else if ((old_c & 4) && !(c & 4)) {
            didaktikgama_8255_obf_b.v=0;
            didaktikgama_8255_ack_b.v=1;
        }
        didaktikgama_mem_page_ram();
    }
}

void didaktikgama_8255_get_output_pins(z80_byte *a,z80_byte *mask_a,
                                        z80_byte *b,z80_byte *mask_b,
                                        z80_byte *c,z80_byte *mask_c)
{
    int mode_a=didaktikgama_8255_mode_a();

    *a=didaktikgama_8255_port_a;
    *b=didaktikgama_8255_port_b;
    *c=didaktikgama_8255_read_c();
    *mask_a=(mode_a==2) ?
        ((didaktikgama_8255_input_c & 0x40) ? 0 : 0xff) :
        ((didaktikgama_8255_mode & 0x10) ? 0 : 0xff);
    *mask_b=(didaktikgama_8255_mode & 2) ? 0 : 0xff;
    *mask_c=(didaktikgama_8255_mode & 8) ? 0 : 0xf0;
    if (!(didaktikgama_8255_mode & 1)) *mask_c |= 0x0f;
    if (mode_a) {
        *mask_c=(*mask_c & ~0x08) | 0x08;
        if (mode_a==2 || (didaktikgama_8255_mode & 0x10))
            *mask_c=(*mask_c & ~0x30) | 0x20;
        if (mode_a==2 || !(didaktikgama_8255_mode & 0x10))
            *mask_c=(*mask_c & ~0xc0) | 0x80;
    }
    if (didaktikgama_8255_mode & 4) *mask_c=(*mask_c & ~0x07) | 0x03;
}

void didaktikgama_out_port(z80_int puerto, z80_byte value)
{
    switch (puerto & 0x60) {
        case 0x00:
            didaktikgama_8255_port_a=value;
            if (didaktikgama_8255_mode_a() &&
                (didaktikgama_8255_mode_a()==2 || !(didaktikgama_8255_mode & 0x10))) {
                didaktikgama_8255_obf_a.v=1;
                didaktikgama_8255_ack_a.v=0;
            }
            break;
        case 0x20:
            didaktikgama_8255_port_b=value;
            if ((didaktikgama_8255_mode & 4) && !(didaktikgama_8255_mode & 2)) {
                didaktikgama_8255_obf_b.v=1;
                didaktikgama_8255_ack_b.v=0;
                didaktikgama_mem_page_ram();
            }
            break;
        case 0x40:
            didaktikgama_8255_port_c=value;
            didaktikgama_mem_page_ram();
            break;
        case 0x60:
            didaktikgama_8255_port_d=value;
            if (value & 0x80) {
                didaktikgama_8255_mode=value;
                didaktikgama_8255_port_a=0;
                didaktikgama_8255_port_b=0;
                didaktikgama_8255_port_c=0;
                didaktikgama_8255_ibf_a.v=didaktikgama_8255_ibf_b.v=0;
                didaktikgama_8255_obf_a.v=didaktikgama_8255_obf_b.v=0;
                didaktikgama_8255_ack_a.v=didaktikgama_8255_ack_b.v=0;
                didaktikgama_8255_inte_a_in.v=didaktikgama_8255_inte_a_out.v=didaktikgama_8255_inte_b.v=0;
                didaktikgama_mem_page_ram();
            }
            else {
                // El modo BSR modifica un solo bit del puerto C.
                z80_byte bit=1 << ((value >> 1) & 7);
                if (value & 1) didaktikgama_8255_port_c |= bit;
                else didaktikgama_8255_port_c &= ~bit;
                if ((didaktikgama_8255_mode & 4) && bit==4) didaktikgama_8255_inte_b.v=value & 1;
                if (didaktikgama_8255_mode_a() && bit==0x10) didaktikgama_8255_inte_a_in.v=value & 1;
                if (didaktikgama_8255_mode_a() && bit==0x40) didaktikgama_8255_inte_a_out.v=value & 1;
                didaktikgama_mem_page_ram();
            }
            break;
    }
}


z80_byte didaktikgama_in_port(z80_int puerto)
{
    switch (puerto & 0x60) {
        case 0x00:
            if (didaktikgama_8255_mode_a()==2 ||
                (didaktikgama_8255_mode_a()==1 && (didaktikgama_8255_mode & 0x10))) {
                didaktikgama_8255_ibf_a.v=0;
                return didaktikgama_8255_latch_a;
            }
            if ((didaktikgama_8255_mode & 0x60)==0 && (didaktikgama_8255_mode & 0x10))
                return didaktikgama_8255_input_a;
            return didaktikgama_8255_port_a;
        case 0x20:
            if ((didaktikgama_8255_mode & 6)==6) {
                didaktikgama_8255_ibf_b.v=0;
                didaktikgama_mem_page_ram();
                return didaktikgama_8255_latch_b;
            }
            if (!(didaktikgama_8255_mode & 4) && (didaktikgama_8255_mode & 2))
                return didaktikgama_8255_input_b;
            return didaktikgama_8255_port_b;
        case 0x40: return didaktikgama_8255_read_c();
        case 0x60: return 0xff;
    }
    return 0xff;
}
