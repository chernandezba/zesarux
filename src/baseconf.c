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

#include "cpu.h"
#include "baseconf.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "menu.h"
#include "screen.h"
#include "ula.h"
#include "operaciones.h"
#include "zxevo.h"


//Si la sd esta activa o no
int baseconf_sd_enabled=1;

int baseconf_sd_cs=0;

//Direcciones donde estan cada pagina de rom. 32 paginas de 16 kb
z80_byte *baseconf_rom_mem_table[32];

//Direcciones donde estan cada pagina de ram, en paginas de 16 kb
z80_byte *baseconf_ram_mem_table[256];


//Direcciones actuales mapeadas, bloques de 16 kb
z80_byte *baseconf_memory_paged[4];


//Numeros de bloques de memoria asignados
z80_byte baseconf_memory_segments[4];

//Tipos de bloques de memoria asignados
//0: rom. otra cosa: ram
z80_byte baseconf_memory_segments_type[4];

z80_byte baseconf_last_port_77;

z80_byte baseconf_shadow_mode_port_77;

z80_byte baseconf_last_port_bf;

z80_byte baseconf_last_port_eff7;

//ver Xpeccy - http://github.com/samstyle/Xpeccy Baseconf ports and memory maping is in ./src/libxpeccy/hardware/pentevo.c
 
int baseconf_shadow_ports_available(void)
{

        if (baseconf_last_port_bf&1) {
                //0: if 1 then enable shadow ports. 0 after reset.
                return 1;
        }
        if ((baseconf_shadow_mode_port_77&2)==0) {
                //Enable shadow mode ports of the memory manager's permission.
                return 1;
        }

        return 0;
}


void lee_byte_evo_aux(z80_int direccion GCC_UNUSED)
{
        //TODO: funcion que se usa en el core baseconf de testing
}

void baseconf_reset_cpu(void)
{


    //TODO. Que otros puertos de baseconf se ponen a 0 en el reset?




    baseconf_set_memory_pages();
    //baseconf_set_sizes_display();
}

void baseconf_init_memory_tables(void)
{
	debug_printf (VERBOSE_DEBUG,"Initializing BaseConf memory pages");

	z80_byte *puntero;
	puntero=memoria_spectrum;

	int i;
	for (i=0;i<BASECONF_ROM_PAGES;i++) {
		baseconf_rom_mem_table[i]=puntero;
		puntero +=16384;
	}

	for (i=0;i<BASECONF_RAM_PAGES;i++) {
		baseconf_ram_mem_table[i]=puntero;
		puntero +=16384;
	}




}



void baseconf_set_memory_pages(void)
{

        int i=0;

        for (i=0;i<4;i++) {
                z80_byte pagina=baseconf_memory_segments[i];
                z80_byte pagina_es_ram=baseconf_memory_segments_type[i];

                if ((baseconf_shadow_mode_port_77&1)==0) {
                        //A8: if 0, then disable the memory manager. In each window processor is installed the last page of ROM. 0 after reset.
                        pagina=255;
                        pagina_es_ram=0;
                }

                if (baseconf_last_port_eff7&8) {
                        /* 3: When placing a 1 in box # 0000 .. # 3FFF forces the zero page RAM. 
                        This bit has priority over all other ways to switch the memory page in the window.
                        Value after reset - 0.
                        */

                       if (i==0) {
                               pagina=0;
                               pagina_es_ram=1;
                       }
                }

                //TODO: A9: If 0 then "force" the inclusion of TR-DOS and the shadow ports. 0 after reset.

                if (pagina_es_ram) {
                        baseconf_memory_paged[i]=baseconf_ram_mem_table[pagina];
                        debug_paginas_memoria_mapeadas[i]=pagina;
                }
                else {
                        pagina=pagina & 31;
                        baseconf_memory_paged[i]=baseconf_rom_mem_table[pagina];
                        debug_paginas_memoria_mapeadas[i]=DEBUG_PAGINA_MAP_ES_ROM+pagina;
                }

                //printf ("segmento %d pagina %d\n",i,pagina);
        }
	


  //printf ("32765: %02XH rom %d ram1 %d ram2 %d ram3 %d\n",puerto_32765,rom_page,ram_page_40,ram_page_80,ram_page_c0);


}


void baseconf_hard_reset(void)
{

  debug_printf(VERBOSE_DEBUG,"BaseConf Hard reset cpu");

  //Asignar bloques memoria
  baseconf_memory_segments[0]=baseconf_memory_segments[1]=baseconf_memory_segments[2]=baseconf_memory_segments[3]=255;
  baseconf_memory_segments_type[0]=baseconf_memory_segments_type[1]=baseconf_memory_segments_type[2]=baseconf_memory_segments_type[3]=0;

 
  reset_cpu();


	int i;


       //Borrar toda memoria ram
        int d;
        z80_byte *puntero;
        
        for (i=0;i<BASECONF_RAM_PAGES;i++) {
                puntero=baseconf_ram_mem_table[i];
                for (d=0;d<16384;d++,puntero++) {
                        *puntero=0;
                }
        }
baseconf_last_port_77=0;
baseconf_shadow_mode_port_77=0;
baseconf_last_port_bf=0;
baseconf_last_port_eff7=0;

        baseconf_set_memory_pages();

}

//Cambia el valor de entrada de numero de pagina ram segun :
/*
for RAM - in the window there is a substitution under 3 or 6 bits (depending on the mode of ZX Spectrum 128k or pentagon 1024k) 
page numbers are not inverse bits from port # 7FFD.
*/
z80_byte baseconf_change_ram_page_7ffd(z80_byte value)
{
        
/*
baseconf_last_port_eff7;
2: off for a 1 - mode ZX Spectrum 128k, otherwise - mode pentagon 1024k.
Value after reset - 0.
*/
        //printf ("adjusting ram to bits port 7ffdh\n");

        if (baseconf_last_port_eff7&4) {
                //paginacion 128k
                value=value&(255-7);
                value=value | (puerto_32765&7);
        }
        else {
                //paginacion pentagon 1024k. 6 bits
                z80_byte ram_entra=(puerto_32765&7) | ((puerto_32765>>2)&(8+16+32));
                value=value&(255-63);
                value=value|ram_entra;
        }

        return value;
}

//Cambia el valor de entrada de numero de pagina rom segun:
/*
For ROM - there is a substitution LSB page numbers signal the inclusion of TR-DOS (1 if the TR-DOS included).
In addition, there is the inclusion of the shadow of ports and TR-DOS («log in TR-DOS »), if in this box will code execution with the offset # 3Dxx.
*/

z80_byte baseconf_change_rom_page_trdos(z80_byte value)
{
        value=value&254;
        if ((baseconf_shadow_mode_port_77&2)==0) {
                //printf ("TODO: If 0 then -force- the inclusion of TR-DOS and the shadow ports. 0 after reset\n");
                /* 
                A9: If 0 then "force" the inclusion of TR-DOS and the shadow ports. 0 after reset.
                */

               value=value|1; // no estoy seguro de esto
        }
        return value;
}

void baseconf_out_port(z80_int puerto,z80_byte valor)
{

        z80_byte puerto_h=puerto>>8;

      

        //xxBFH
        //Enable shadow mode ports write permission in ROM.
        if ( (puerto&0x00FF)==0xBF ) {
               baseconf_last_port_bf=valor; 

               baseconf_set_memory_pages();
        }        

        //xx77H
        else if ( (puerto&0x00FF)==0x77 && baseconf_shadow_ports_available() ) {
                baseconf_shadow_mode_port_77=puerto_h;
               baseconf_last_port_77=valor; 

               baseconf_set_memory_pages();
        }

        else if (puerto==0xEFF7) {
                //printf ("setting port EFF7 value\n");
                baseconf_last_port_eff7=valor;
                baseconf_set_memory_pages();
        }


        //xFF7H
        //The memory manager pages.
        else if ( (puerto&0x0FFF)==0xFF7 && baseconf_shadow_ports_available() ) {
               

                
                  z80_byte pagina=valor ^ 255;
                         z80_byte es_ram=valor & 64;



                      z80_byte segmento=puerto_h>>6;
                     if (es_ram==0) {
                           pagina=pagina&31;
                         if (valor&128) pagina=baseconf_change_rom_page_trdos(pagina);
                  }

                  else {
                                pagina=pagina&63;
                                if (valor&128) pagina=baseconf_change_ram_page_7ffd(pagina);
                 }

                 baseconf_memory_segments[segmento]=pagina;  
                 baseconf_memory_segments_type[segmento]=es_ram;
                

               baseconf_set_memory_pages();
        }
        /*Out port baseconf port FFF7H value 40H. PC=03AAH
segmento 0 pagina 24
segmento 1 pagina 64
segmento 2 pagina 255
segmento 3 pagina 63
Out port baseconf port F7F7H value BFH. PC=03AFH  -> BF=10 111111 -> pagina invertida=64... o sea que solo hay que pillar bits inferiores?
segmento 0 pagina 24
segmento 1 pagina 64
segmento 2 pagina 255
segmento 3 pagina 64
Out port baseconf port 3FF7H value 06H. PC=84C0H
segmento 0 pagina 25
segmento 1 pagina 64
segmento 2 pagina 255
segmento 3 pagina 64
Out port baseconf port DEF7H value EFH. PC=31BCH
Baseconf reading port BEF7H
baseconf reading nvram register EFH
Out port baseconf port 3FF7H value 3FH. PC=84D7H
segmento 0 pagina 0

*/

        //x7F7H
        //The memory manager pages. All ram access. Port not in ATM2
        else if ( (puerto&0x0FFF)==0x7F7 && baseconf_shadow_ports_available() ) {
                z80_byte pagina=valor ^ 255;
                z80_byte es_ram=1;

                z80_byte segmento=puerto_h>>6;


                if (valor&128) pagina=baseconf_change_ram_page_7ffd(pagina);

                baseconf_memory_segments[segmento]=pagina;  
                baseconf_memory_segments_type[segmento]=es_ram;      


               baseconf_set_memory_pages();
        }        

        else if (puerto==0x7ffd) {
                //mapeamos ram y rom , pero sin habilitando memory manager
                //baseconf_shadow_ports |=1;

                //ram
                baseconf_memory_segments[3]=valor&7;
                baseconf_memory_segments_type[3]=1;

                //rom
                baseconf_memory_segments[0] &=254;
                if (valor&16) baseconf_memory_segments[0] |= 1;
                baseconf_memory_segments_type[0]=0;       


                puerto_32765=valor;

                baseconf_set_memory_pages();

                //printf ("mapping segun puerto 32765\n");
        }

        //Puertos NVRAM. 
	else if (puerto==0xeff7 && !baseconf_shadow_ports_available() ) puerto_eff7=valor;
	else if (puerto==0xdff7 && !baseconf_shadow_ports_available() ) zxevo_last_port_dff7=valor;
        else if (puerto==0xdef7 && baseconf_shadow_ports_available() ) zxevo_last_port_dff7=valor;


	else if (puerto==0xbff7 && !baseconf_shadow_ports_available() ) {
						//Si esta permitida la escritura
						if (puerto_eff7&128) zxevo_nvram[zxevo_last_port_dff7]=valor;
	}

        else if (puerto==0xbef7 && baseconf_shadow_ports_available() ) {
                        //Note: In the shadow mode port # BEF7 available regardless of bit 7 port # EFF7.
		 zxevo_nvram[zxevo_last_port_dff7]=valor;
					}
        else if ( (puerto&0x00FF)==0x77 ) {
                printf ("Record: control signal CS to SD-card unemulated\n");
        }

        else {
                printf ("unhandled out port %04XH value %02XH\n",puerto,valor);
                //sleep(1);
        }
}


void screen_baseconf_refresca_pantalla(void)
{

	/*
	//Como spectrum clasico

	//modo clasico. sin rainbow
	if (rainbow_enabled.v==0) {
        screen_baseconf_refresca_border();
        z80_byte modo_video=baseconf_get_video_mode_display();


        //printf ("modo video: %d\n",modo_video );
        if (modo_video==0) scr_baseconf_refresca_pantalla_zxmode_no_rainbow();
        if (modo_video==1) scr_baseconf_refresca_pantalla_16c_256c_no_rainbow(1);
        if (modo_video==2) scr_baseconf_refresca_pantalla_16c_256c_no_rainbow(2);
        if (modo_video==3) screen_baseconf_refresca_text_mode();

	}

	else {
	//modo rainbow - real video
        if (baseconf_si_render_spritetile_rapido.v) baseconf_fast_tilesprite_render();

        screen_baseconf_refresca_rainbow();
	}
*/
}





