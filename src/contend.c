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
#include <string.h>


#include "contend.h"
#include "cpu.h"
#include "debug.h"
#include "screen.h"
#include "compileoptions.h"
#include "ulaplus.h"
#include "zxuno.h"
#include "ula.h"
#include "tbblue.h"


int contend_patron_65432100[] = { 5, 4, 3, 2, 1, 0, 0, 6 };

int contend_patron_76543210[] = { 5, 4, 3, 2, 1, 0, 7, 6 };

int contend_patron_no_contend[] = { 0,0,0,0,0,0,0,0};

z80_byte *contend_pages_128k_p2a;

//Memory banks 1,3,5 and 7 are contended. Para 128k, p2
//                                 0,1,2,3,4,5,6,7
z80_byte contend_pages_128k[] = {  0,1,0,1,0,1,0,1 };


//Para +2A, contend solo en 4,5,6,7
//                                0,1,2,3,4,5,6,7
z80_byte contend_pages_p2a[] = {  0,0,0,0,1,1,1,1 };


//Para Chloe, contend solo en 5,7
//				  0,1,2,3,4,5,6,7
z80_byte contend_pages_chloe[]={  0,0,0,0,0,1,0,1 };


//Para Chloe, contend solo en 2,5
//				  0,1,2,3,4,5,6,7,8,9
z80_byte contend_pages_chrome[]={  0,0,1,0,0,1,0,0,0,0 };


//Para tsconf, contend solo en 2,5
//				  0,1,2,3,4,5,6,7,8,9
//z80_byte contend_pages_tsconf[]={  0,0,1,0,0,1,0,0,0,0 };


//Indica si las paginas actuales mapeadas tienen contend o no (a 0 o a 1)
z80_byte contend_pages_actual[4];



//z80_byte contend_table[MAX_CONTEND_TABLE];
//z80_byte contend_table_no_mreq[MAX_CONTEND_TABLE];

//Tablas cuando cpu speed es X1
z80_byte contend_table_speed_one[CONTEND_TABLE_SIZE_ONE_SPEED];
z80_byte contend_table_no_mreq_speed_one[CONTEND_TABLE_SIZE_ONE_SPEED];

//Tablas para cuando cpu speed es > X1 y estan a 0 siempre
z80_byte contend_table_speed_higher[MAX_CONTEND_TABLE];
z80_byte contend_table_no_mreq_speed_higher[MAX_CONTEND_TABLE];


z80_byte *contend_table;
z80_byte *contend_table_no_mreq;


//Funciones de contended memory y timing para las diferentes maquinas
//Punteros a funciones de cada maquina
void (*contend_read)(z80_int direccion,int time);
void (*contend_read_no_mreq)(z80_int direccion,int time);
void (*contend_write_no_mreq)(z80_int direccion,int time);


void (*ula_contend_port_early)( z80_int port );
void (*ula_contend_port_late)( z80_int port );

int (*port_from_ula) (z80_int puerto);

z80_bit contend_enabled;



//zx80, zx81 - no contended
void contend_read_zx8081(z80_int direccion GCC_UNUSED,int time)
{
	//No contention

        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_zx8081(z80_int direccion GCC_UNUSED,int time)
{
	//No contention

        //Y sumamos estados normales
        t_estados += time;

}


void contend_write_no_mreq_zx8081(z80_int direccion GCC_UNUSED,int time)
{

	//No contention

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_zx8081( z80_int port GCC_UNUSED )
{
  t_estados++;
}

void ula_contend_port_late_zx8081( z80_int port GCC_UNUSED )
{
        t_estados += 2;

}


//Jupiter Ace - no contended
void contend_read_ace(z80_int direccion GCC_UNUSED,int time)
{
        //No contention

        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_ace(z80_int direccion GCC_UNUSED,int time)
{
        //No contention

        //Y sumamos estados normales
        t_estados += time;

}


void contend_write_no_mreq_ace(z80_int direccion GCC_UNUSED,int time)
{

        //No contention

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_ace( z80_int port GCC_UNUSED )
{
  t_estados++;
}

void ula_contend_port_late_ace( z80_int port GCC_UNUSED )
{
        t_estados += 2;

}



//CPC 464
void contend_read_cpc(z80_int direccion GCC_UNUSED,int time)
{
        //No contention

        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_cpc(z80_int direccion GCC_UNUSED,int time)
{
        //No contention

        //Y sumamos estados normales
        t_estados += time;

}


void contend_write_no_mreq_cpc(z80_int direccion GCC_UNUSED,int time)
{

        //No contention

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_cpc( z80_int port GCC_UNUSED )
{
  t_estados++;
}

void ula_contend_port_late_cpc( z80_int port GCC_UNUSED )
{
        t_estados += 2;

}


//SAM COUPE
void contend_read_sam(z80_int direccion GCC_UNUSED,int time)
{
        //No contention

        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_sam(z80_int direccion GCC_UNUSED,int time)
{
        //No contention

        //Y sumamos estados normales
        t_estados += time;

}


void contend_write_no_mreq_sam(z80_int direccion GCC_UNUSED,int time)
{

        //No contention

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_sam( z80_int port GCC_UNUSED )
{
  t_estados++;
}

void ula_contend_port_late_sam( z80_int port GCC_UNUSED )
{
        t_estados += 2;

}



//48kb

void contend_read_48k(z80_int direccion,int time)
{
/*
0100000000000000 -> 16384
1000000000000000 -> 32768
1100000000000000 -> 49152
Contention solo entre 16384 y 49151
*/
#ifdef EMULATE_CONTEND
	if ( (direccion&49152)==16384) {
		t_estados += contend_table[ t_estados ];
	}
#endif

	//Y sumamos estados normales
	t_estados += time;

}

void contend_read_no_mreq_48k(z80_int direccion,int time)
{
/*
0100000000000000 -> 16384
1000000000000000 -> 32768
1100000000000000 -> 49152
Contention solo entre 16384 y 49151
*/
#ifdef EMULATE_CONTEND
        if ( (direccion&49152)==16384) {
                t_estados += contend_table_no_mreq[ t_estados ];
        }
#endif

        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_48k(z80_int direccion,int time)
{
/*
0100000000000000 -> 16384
1000000000000000 -> 32768
1100000000000000 -> 49152
Contention solo entre 16384 y 49151
*/
#ifdef EMULATE_CONTEND
        if ( (direccion&49152)==16384) {
                t_estados += contend_table_no_mreq[ t_estados ];
        }
#endif

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_48k( z80_int port )
{
#ifdef EMULATE_CONTEND

        if ( (port&49152)==16384) {
                t_estados += contend_table_no_mreq[ t_estados ];
        }
#endif

	t_estados++;
}

void ula_contend_port_late_48k( z80_int port )
{
#ifdef EMULATE_CONTEND
  if( port_from_ula( port ) ) {

    t_estados += contend_table_no_mreq[ t_estados ];
    t_estados += 2;

  }
  else {

        if ( (port&49152)==16384) {
      		t_estados += contend_table_no_mreq[ t_estados ]; t_estados++;
		t_estados += contend_table_no_mreq[ t_estados ]; t_estados++;
		t_estados += contend_table_no_mreq[ t_estados ];
	}
	else {
	      t_estados += 2;
	}
 }
#else
	t_estados += 2;
#endif

}



//Timex

void contend_read_timex(z80_int direccion,int time)
{
/*
0100000000000000 -> 16384
1000000000000000 -> 32768
1100000000000000 -> 49152
Contention solo entre 16384 y 49151
*/
#ifdef EMULATE_CONTEND
	if ( (direccion&49152)==16384) {
		t_estados += contend_table[ t_estados ];
	}
#endif

	//Y sumamos estados normales
	t_estados += time;

}

void contend_read_no_mreq_timex(z80_int direccion,int time)
{
/*
0100000000000000 -> 16384
1000000000000000 -> 32768
1100000000000000 -> 49152
Contention solo entre 16384 y 49151
*/
#ifdef EMULATE_CONTEND
        if ( (direccion&49152)==16384) {
                t_estados += contend_table_no_mreq[ t_estados ];
        }
#endif

        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_timex(z80_int direccion,int time)
{
/*
0100000000000000 -> 16384
1000000000000000 -> 32768
1100000000000000 -> 49152
Contention solo entre 16384 y 49151
*/
#ifdef EMULATE_CONTEND
        if ( (direccion&49152)==16384) {
                t_estados += contend_table_no_mreq[ t_estados ];
        }
#endif

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_timex( z80_int port )
{
#ifdef EMULATE_CONTEND

        if ( (port&49152)==16384) {
                t_estados += contend_table_no_mreq[ t_estados ];
        }
#endif

	t_estados++;
}

void ula_contend_port_late_timex( z80_int port )
{
#ifdef EMULATE_CONTEND
  if( port_from_ula( port ) ) {

    t_estados += contend_table_no_mreq[ t_estados ];
    t_estados += 2;

  }
  else {

        if ( (port&49152)==16384) {
      		t_estados += contend_table_no_mreq[ t_estados ]; t_estados++;
		t_estados += contend_table_no_mreq[ t_estados ]; t_estados++;
		t_estados += contend_table_no_mreq[ t_estados ];
	}
	else {
	      t_estados += 2;
	}
 }
#else
	t_estados += 2;
#endif

}





//128kb todos: 128k, +2, +2A etc

void contend_read_128k(z80_int direccion,int time)
{

#ifdef EMULATE_CONTEND

		z80_int segmento;
                segmento=direccion / 16384;
		if (contend_pages_actual[segmento]) {
			t_estados += contend_table[ t_estados ];
		}
#endif

        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_128k(z80_int direccion,int time)
{

#ifdef EMULATE_CONTEND

		z80_int segmento;
                segmento=direccion / 16384;
		if (contend_pages_actual[segmento]) {
			t_estados += contend_table[ t_estados ];
		}
#endif


        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_128k(z80_int direccion,int time)
{

#ifdef EMULATE_CONTEND

		z80_int segmento;
                segmento=direccion / 16384;

		//printf ("direccion: %d segmento: %d contend: %d\n",direccion,segmento,contend_pages_actual[segmento]);
		if (contend_pages_actual[segmento]) {
			t_estados += contend_table[ t_estados ];
		}
#endif


        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_128k( z80_int port )
{
#ifdef EMULATE_CONTEND
z80_int segmento;
                segmento=port / 16384;
                if (contend_pages_actual[segmento]) {
                	t_estados += contend_table_no_mreq[ t_estados ];

			/*
			extern int temp_gigascreen_ajuste;
			int i;
			if (port==32765) {
			for (i=0;i<temp_gigascreen_ajuste;i++) {
				t_estados++;
				//t_estados +=contend_table_no_mreq[ t_estados ];
			}
			}
			*/


	        }
#endif

  t_estados++;
}

void ula_contend_port_late_128k( z80_int port )
{
#ifdef EMULATE_CONTEND
  if( port_from_ula( port ) ) {
    t_estados += contend_table_no_mreq[ t_estados ];
    t_estados += 2;

  }
  else {


z80_int segmento;
     segmento=port / 16384;
    if (contend_pages_actual[segmento]) {
                t_estados += contend_table_no_mreq[ t_estados ]; t_estados++;
                t_estados += contend_table_no_mreq[ t_estados ]; t_estados++;
                t_estados += contend_table_no_mreq[ t_estados ];

        }
        else {
              t_estados += 2;
        }
  }
#else
        t_estados += 2;
#endif

}


//chloe

void contend_read_chloe(z80_int direccion,int time)
{

#ifdef EMULATE_CONTEND

		z80_int segmento;
                segmento=direccion / 16384;
		if (contend_pages_actual[segmento]) {
			t_estados += contend_table[ t_estados ];
		}
#endif

        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_chloe(z80_int direccion,int time)
{

#ifdef EMULATE_CONTEND

		z80_int segmento;
                segmento=direccion / 16384;
		if (contend_pages_actual[segmento]) {
			t_estados += contend_table[ t_estados ];
		}
#endif


        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_chloe(z80_int direccion,int time)
{

#ifdef EMULATE_CONTEND

		z80_int segmento;
                segmento=direccion / 16384;

		//printf ("direccion: %d segmento: %d contend: %d\n",direccion,segmento,contend_pages_actual[segmento]);
		if (contend_pages_actual[segmento]) {
			t_estados += contend_table[ t_estados ];
		}
#endif


        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_chloe( z80_int port )
{
#ifdef EMULATE_CONTEND
z80_int segmento;
                segmento=port / 16384;
                if (contend_pages_actual[segmento]) {
                	t_estados += contend_table_no_mreq[ t_estados ];


	        }
#endif

  t_estados++;
}

void ula_contend_port_late_chloe( z80_int port )
{
#ifdef EMULATE_CONTEND
  if( port_from_ula( port ) ) {
    t_estados += contend_table_no_mreq[ t_estados ];
    t_estados += 2;

  }
  else {


z80_int segmento;
     segmento=port / 16384;
    if (contend_pages_actual[segmento]) {
                t_estados += contend_table_no_mreq[ t_estados ]; t_estados++;
                t_estados += contend_table_no_mreq[ t_estados ]; t_estados++;
                t_estados += contend_table_no_mreq[ t_estados ];

        }
        else {
              t_estados += 2;
        }
  }
#else
        t_estados += 2;
#endif

}


//chrome

void contend_read_chrome(z80_int direccion,int time)
{

#ifdef EMULATE_CONTEND

		z80_int segmento;
                segmento=direccion / 16384;
		if (contend_pages_actual[segmento]) {
			t_estados += contend_table[ t_estados ];
		}
#endif

        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_chrome(z80_int direccion,int time)
{

#ifdef EMULATE_CONTEND

		z80_int segmento;
                segmento=direccion / 16384;
		if (contend_pages_actual[segmento]) {
			t_estados += contend_table[ t_estados ];
		}
#endif


        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_chrome(z80_int direccion,int time)
{

#ifdef EMULATE_CONTEND

		z80_int segmento;
                segmento=direccion / 16384;

		//printf ("direccion: %d segmento: %d contend: %d\n",direccion,segmento,contend_pages_actual[segmento]);
		if (contend_pages_actual[segmento]) {
			t_estados += contend_table[ t_estados ];
		}
#endif


        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_chrome( z80_int port )
{
#ifdef EMULATE_CONTEND
z80_int segmento;
                segmento=port / 16384;
                if (contend_pages_actual[segmento]) {
                	t_estados += contend_table_no_mreq[ t_estados ];


	        }
#endif

  t_estados++;
}

void ula_contend_port_late_chrome( z80_int port )
{
#ifdef EMULATE_CONTEND
  if( port_from_ula( port ) ) {
    t_estados += contend_table_no_mreq[ t_estados ];
    t_estados += 2;

  }
  else {


z80_int segmento;
     segmento=port / 16384;
    if (contend_pages_actual[segmento]) {
                t_estados += contend_table_no_mreq[ t_estados ]; t_estados++;
                t_estados += contend_table_no_mreq[ t_estados ]; t_estados++;
                t_estados += contend_table_no_mreq[ t_estados ];

        }
        else {
              t_estados += 2;
        }
  }
#else
        t_estados += 2;
#endif

}

//baseconf

void contend_read_baseconf(z80_int direccion GCC_UNUSED,int time)
{
        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_baseconf(z80_int direccion GCC_UNUSED,int time)
{

        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_baseconf(z80_int direccion GCC_UNUSED,int time)
{

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_baseconf( z80_int port GCC_UNUSED)
{
 

  t_estados++;
}

void ula_contend_port_late_baseconf( z80_int port GCC_UNUSED)
{


        t_estados += 2;


}





//tsconf

void contend_read_tsconf(z80_int direccion GCC_UNUSED,int time)
{
        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_tsconf(z80_int direccion GCC_UNUSED,int time)
{

        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_tsconf(z80_int direccion GCC_UNUSED,int time)
{

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_tsconf( z80_int port GCC_UNUSED)
{
 

  t_estados++;
}

void ula_contend_port_late_tsconf( z80_int port GCC_UNUSED)
{


        t_estados += 2;


}

//mk14

void contend_read_mk14(z80_int direccion GCC_UNUSED,int time GCC_UNUSED)
{


}

void contend_read_no_mreq_mk14(z80_int direccion GCC_UNUSED,int time GCC_UNUSED)
{


}

void contend_write_no_mreq_mk14(z80_int direccion GCC_UNUSED,int time GCC_UNUSED)
{


}


void ula_contend_port_early_mk14( z80_int port  GCC_UNUSED)
{

}

void ula_contend_port_late_mk14( z80_int port  GCC_UNUSED)
{


}

void contend_read_svi(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND

#endif

	//Y sumamos estados normales
	t_estados += time;

}

void contend_read_no_mreq_svi(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND

#endif

        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_svi(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND
   
#endif

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_svi( z80_int port GCC_UNUSED )
{
#ifdef EMULATE_CONTEND


#endif

	t_estados++;
}

void ula_contend_port_late_svi( z80_int port GCC_UNUSED)
{
#ifdef EMULATE_CONTEND
 
        t_estados += 2;
#else
	t_estados += 2;
#endif

}




void contend_read_msx1(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND

#endif

	//Y sumamos estados normales
	t_estados += time;

}

void contend_read_no_mreq_msx1(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND

#endif

        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_msx1(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND
   
#endif

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_msx1( z80_int port GCC_UNUSED )
{
#ifdef EMULATE_CONTEND


#endif

	t_estados++;
}

void ula_contend_port_late_msx1( z80_int port GCC_UNUSED)
{
#ifdef EMULATE_CONTEND
 
        t_estados += 2;
#else
	t_estados += 2;
#endif

}


void contend_read_coleco(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND

#endif

	//Y sumamos estados normales
	t_estados += time;

}

void contend_read_no_mreq_coleco(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND

#endif

        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_coleco(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND
   
#endif

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_coleco( z80_int port GCC_UNUSED)
{
#ifdef EMULATE_CONTEND


#endif

	t_estados++;
}

void ula_contend_port_late_coleco( z80_int port GCC_UNUSED)
{
#ifdef EMULATE_CONTEND
 
        t_estados += 2;
#else
	t_estados += 2;
#endif

}



void contend_read_sg1000(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND

#endif

	//Y sumamos estados normales
	t_estados += time;

}

void contend_read_no_mreq_sg1000(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND

#endif

        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_sg1000(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND
   
#endif

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_sg1000( z80_int port GCC_UNUSED)
{
#ifdef EMULATE_CONTEND


#endif

	t_estados++;
}

void ula_contend_port_late_sg1000( z80_int port GCC_UNUSED)
{
#ifdef EMULATE_CONTEND
 
        t_estados += 2;
#else
	t_estados += 2;
#endif

}

void contend_read_sms(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND

#endif

	//Y sumamos estados normales
	t_estados += time;

}

void contend_read_no_mreq_sms(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND

#endif

        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_sms(z80_int direccion GCC_UNUSED,int time)
{

#ifdef EMULATE_CONTEND
   
#endif

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_sms( z80_int port GCC_UNUSED)
{
#ifdef EMULATE_CONTEND


#endif

	t_estados++;
}

void ula_contend_port_late_sms( z80_int port GCC_UNUSED)
{
#ifdef EMULATE_CONTEND
 
        t_estados += 2;
#else
	t_estados += 2;
#endif

}



//prism. No tiene memoria contended
void contend_read_prism(z80_int direccion GCC_UNUSED,int time)
{

        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_prism(z80_int direccion GCC_UNUSED,int time)
{


        //Y sumamos estados normales
        t_estados += time;

}

void contend_write_no_mreq_prism(z80_int direccion GCC_UNUSED,int time)
{


        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_prism( z80_int port GCC_UNUSED)
{

  t_estados++;
}
void ula_contend_port_late_prism( z80_int port GCC_UNUSED)
{
        t_estados += 2;

}



int port_from_ula_48k (z80_int puerto)
{
	// All even ports supplied by ULA
	// O puertos ulaplus
	if (ulaplus_presente.v) {
		if (puerto==0xBF3B || puerto==0xFF3B) return 1;
	}


	return !( puerto & 0x0001 );
}

int port_from_ula_p2a (z80_int puerto)
{

        // O puertos ulaplus
        if (ulaplus_presente.v) {
                if (puerto==0xBF3B || puerto==0xFF3B) return 1;
        }


	// No contended ports
	return 0;
}



void contend_read_z88(z80_int direccion GCC_UNUSED,int time)
{

        //Y sumamos estados normales
        t_estados += time;

}

void contend_read_no_mreq_z88(z80_int direccion GCC_UNUSED,int time)
{

        //Y sumamos estados normales
        t_estados += time;

}


void contend_write_no_mreq_z88(z80_int direccion GCC_UNUSED,int time)
{

        //Y sumamos estados normales
        t_estados += time;

}


void ula_contend_port_early_z88( z80_int port GCC_UNUSED)
{

        t_estados++;
}




void ula_contend_port_late_z88( z80_int port GCC_UNUSED)
{
        t_estados += 2;

}




/*

Now for the timings of each line itself: define a screen line to start with 256 screen pixels, then border,
then horizontal retrace, and then border again. All this takes 224 T states. Every half T state a pixel
is written to the CRT, so if the ULA is reading bytes it does so each 4 T states (and then it reads two: a screen and an ATTR byte).
The border is 48 pixels wide at each side. A video screen line is therefore timed as follows:
128 T states of screen, 24 T states of right border, 48 T states of horizontal retrace and 24 T states of left border.

*/


z80_byte retorna_contend_time( int time, int *timings , int offset_time, int offset_pattern)
{
  int linea, t_estados_en_linea;

  //la ula empieza a dibujar en la zona de pixeles, saltar borde izquierdo
  time = time + screen_testados_total_borde_izquierdo;

  time = time + offset_time;

  linea = time / screen_testados_linea;

  t_estados_en_linea=(time % screen_testados_linea);


  //No contention en borde superior o inferior
  if( linea < screen_indice_inicio_pant                   ||
      linea >= screen_indice_fin_pant    ) return 0;

  //Ni en borde izquierdo
  if( t_estados_en_linea < screen_testados_total_borde_izquierdo )
    return 0;

  //Ni en borde derecho o horizontal retrace
  if( t_estados_en_linea >= screen_testados_indice_borde_derecho)
    return 0;

  //Content normal de pantalla
  return timings[ (t_estados_en_linea+offset_pattern) % 8 ];
}


//Inicializa a ceros las tablas para velocidad > X1, que siempre son ceros
void inicializa_tabla_contend_speed_higher(void)
{

        debug_printf (VERBOSE_DEBUG,"Initializing contend-zero tables for cpu speed > 1X");

        memset(contend_table_speed_higher,0,MAX_CONTEND_TABLE);
        memset(contend_table_no_mreq_speed_higher,0,MAX_CONTEND_TABLE);

}


void inicializa_tabla_contend(void)
{


	int i;

	int *timings,offset_time, offset_patron;

	debug_printf (VERBOSE_INFO,"Initializing Contended Memory Table");

/*
//Tablas cuando cpu speed es X1
z80_byte contend_table_speed_one[CONTEND_TABLE_SIZE_ONE_SPEED];
z80_byte contend_table_no_mreq_speed_one[CONTEND_TABLE_SIZE_ONE_SPEED];

//Tablas para cuando cpu speed es > X1 y estan a 0 siempre
z80_byte contend_table_speed_higher[MAX_CONTEND_TABLE];
z80_byte contend_table_no_mreq_speed_higher[MAX_CONTEND_TABLE];


z80_byte *contend_table;
z80_byte *contend_table_no_mreq;
*/        

	

	//no hacer tabla contend si hay velocidad turbo
	if (cpu_turbo_speed!=1) {
                //Punteros a tablas con ceros
                contend_table=contend_table_speed_higher;
                contend_table_no_mreq=contend_table_no_mreq_speed_higher;
                debug_printf (VERBOSE_DEBUG,"Setting contend-zero tables for cpu speed > 1X and not recalculating them");
		return;
	}

        debug_printf (VERBOSE_DEBUG,"Setting contend tables for 1X and recalculating them");

        //Punteros a tablas con cpu speed X1
        contend_table=contend_table_speed_one;
        contend_table_no_mreq=contend_table_no_mreq_speed_one;


	if (MACHINE_IS_SPECTRUM_16_48 && !(MACHINE_IS_INVES)) {
		//48k
		timings=contend_patron_65432100;
		offset_time=1;
		offset_patron=-1;
		//Empieza en 14335 con 65432100 Segun http://www.worldofspectrum.org/faq/reference/48kreference.htm
	}

	if (MACHINE_IS_SPECTRUM_128_P2) {
		//128k
                timings=contend_patron_65432100;

		//Empieza en 14365 con 65432100, esto poniendo offset_time=-1
                //offset_time=-1;

		offset_time=3; //6,5,4,3,2,1,0,0 pattern starts at 14361 segun http://www.worldofspectrum.org/faq/reference/128kreference.htm
                offset_patron=-1;


	}


  if (MACHINE_IS_CHROME) {
    //128k
                timings=contend_patron_65432100;

    //Empieza en 14365 con 65432100, esto poniendo offset_time=-1
                //offset_time=-1;

    offset_time=3; //6,5,4,3,2,1,0,0 pattern starts at 14361 segun http://www.worldofspectrum.org/faq/reference/128kreference.htm
                offset_patron=-1;


  }
 

  if (MACHINE_IS_TSCONF) {
                //no tiene memoria contended
                return;

  }

    if (MACHINE_IS_BASECONF) {
                //no tiene memoria contended
                return;
    }
    
    if (MACHINE_IS_MK14) {
                //no tiene memoria contended
                return;
    }
    
	if (MACHINE_IS_PRISM) {
		//no tiene memoria contended
		return;
        }    

	//TODO cpc464. de momento sin contend y no inicializamos tabla porque sino se sale de testados y se sale de la tabla
	if (MACHINE_IS_CPC_464 || MACHINE_IS_CPC_4128) {
		return;

        }

	//TODO msx, coleco , sg1000 y sms. de momento sin contend y no inicializamos tabla porque sino se sale de testados y se sale de la tabla
	if (MACHINE_IS_MSX || MACHINE_IS_COLECO || MACHINE_IS_SG1000 || MACHINE_IS_SVI || MACHINE_IS_SMS) {

		return;

        }

//TODO sam coupe. de momento sin contend y no inicializamos tabla porque sino se sale de testados y se sale de la tabla
	if (MACHINE_IS_SAM) {

		return;

        }


        //TODO QL. de momento sin contend y no inicializamos tabla porque sino se sale de testados y se sale de la tabla
        if (MACHINE_IS_QL) {

          return;

              }        

        if (MACHINE_IS_CHLOE) {
                //Como 48k
                timings=contend_patron_65432100;
                offset_time=1;
                offset_patron=-1;
                //Empieza en 14335 con 65432100
        }



	if (MACHINE_IS_TBBLUE) {
		//Timings de 48k o 128k
		//z80_byte t=(tbblue_config1 >> 6)&3;
    z80_byte t=(tbblue_registers[3])&3;
		if (t>=2) {
			//128k
			timings=contend_patron_65432100;

                	offset_time=3; //6,5,4,3,2,1,0,0 pattern starts at 14361 segun http://www.worldofspectrum.org/faq/reference/128kreference.htm
	                offset_patron=-1;
        	        //Empieza en 14365 con 65432100
		}
		else {
			//48k
			timings=contend_patron_65432100;
	                offset_time=1;
        	        offset_patron=-1;
                	//Empieza en 14335 con 65432100
		}

        }



	if (MACHINE_IS_SPECTRUM_P2A_P3) {
		//+2A
                timings=contend_patron_76543210;
                offset_time=-1;
                offset_patron=4;
		//Empieza en 14365 con 1076543210  Segun http://www.worldofspectrum.org/faq/reference/128kreference.htm#Plus3

	}

	//ZXUno timing de 128k (128k, no +2a)
        if (MACHINE_IS_ZXUNO && (zxuno_ports[0]&16)) {
                //Mismos timings que 128k
                timings=contend_patron_65432100;

		//Empieza en 14365 con 65432100, esto poniendo offset_time=-1
                //offset_time=-1;

		offset_time=3; //6,5,4,3,2,1,0,0 pattern starts at 14361 segun http://www.worldofspectrum.org/faq/reference/128kreference.htm
                offset_patron=-1;
                //Empieza en 14365 con 65432100

        }

        //ZXUno timing de 48k
        if (MACHINE_IS_ZXUNO && (zxuno_ports[0]&16)==0) {
                //48k
                timings=contend_patron_65432100;
                offset_time=1;
                offset_patron=-1;
                //Empieza en 14335 con 65432100

        }



	//ZX80, ZX81, Inves, Z88, Jupiter Ace sin contend

	if (MACHINE_IS_ZX8081 || MACHINE_IS_INVES || MACHINE_IS_Z88 || MACHINE_IS_ACE) {
		timings=contend_patron_no_contend;
                offset_time=0;
                offset_patron=0;

		//printf ("timings: %d [0]: %d\n",timings,timings[0]);

	}


	//TODO timex. Hacemos como 48k
	 if (MACHINE_IS_TIMEX_TS2068) {
                //Timex
                timings=contend_patron_65432100;
                offset_time=1;
                offset_patron=-1;
                //Empieza en 14335 con 65432100
        }




	int final_tabla=screen_testados_total+100;

	if (final_tabla>=CONTEND_TABLE_SIZE_ONE_SPEED) {
		cpu_panic("Initializing Contend Table exceeds maximum allowed of CONTEND_TABLE_SIZE_ONE_SPEED constant. Fix source code contend.h");
	}


	if (ula_late_timings.v) {
		offset_time--;
		//temporal prueba para demo Scroll 2017
		if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {

			//Con esto, la ula128 se ve perfecta y la scroll2017 muy mal
			//offset_time--;
			//offset_time--;

		}
	}

	//Inicializar tabla
	//Por si acaso generamos unos 100 valores mas, por si al final de pantalla se sale...
	z80_byte contend_value;
        for (i=0;i<final_tabla;i++) {
		contend_value=retorna_contend_time(i,timings,offset_time,offset_patron);

		contend_table[i]=contend_value;

		if (MACHINE_IS_SPECTRUM_P2A_P3) {
			//en +2A contend_delay_no_mreq -> no hay contend
			contend_table_no_mreq[i]=0;
		}

		else contend_table_no_mreq[i]=contend_value;


		//Si contend desactivado
		if (contend_enabled.v==0) {
			contend_table_no_mreq[i]=0;
			contend_table[i]=0;
		}

		if (i>14300 && i<14400) debug_printf (VERBOSE_PARANOID,"Contended table. T-state: %d:  %d . no_mreq: %d",i,contend_table[i],contend_table_no_mreq[i]);
                //printf ("Contended table. T-state: %d:  %d . no_mreq: %d\n",i,contend_table[i],contend_table_no_mreq[i]);
        }
        //printf ("Final tabla: %d\n",final_tabla);



}


//Punto de entrada de cambio de cpu speed, se retornan tablas cacheadas
void inicializa_tabla_contend_cached_change_cpu_speed(void)
{

	//no hacer tabla contend si hay velocidad turbo
	if (cpu_turbo_speed!=1) {
                //Punteros a tablas con ceros
                contend_table=contend_table_speed_higher;
                contend_table_no_mreq=contend_table_no_mreq_speed_higher;
                debug_printf (VERBOSE_DEBUG,"Setting contend-zero tables for cpu speed > 1X and not recalculating them");
		return;
	}

        //Punteros a tablas con cpu speed X1
        contend_table=contend_table_speed_one;
        contend_table_no_mreq=contend_table_no_mreq_speed_one;
        debug_printf (VERBOSE_DEBUG,"Setting contend tables for 1X and not recalculating them");

}

  
        