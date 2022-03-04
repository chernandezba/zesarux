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

/*

Note for curious: I'm testing MMU from XSpeccy emulator 
https://github.com/samstyle/Xpeccy
here mixed with my code
If I manage to get it working, I will add the needed License files to the emulator
Meanwhile, here is the XSpeccy License:

Copyright (c) 2009-..., SAM style

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
//segun si bit 4 de 7ffd se cambian 0-3 o 4-7
z80_byte baseconf_memory_segments[8];

//Tipos de bloques de memoria asignados
//0: rom. otra cosa: ram
z80_byte baseconf_memory_segments_type[8];

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
        int inc_memory_card=0;
        if (puerto_32765 & 16) inc_memory_card=4;

        for (i=0;i<4;i++) {
                z80_byte pagina=baseconf_memory_segments[i+inc_memory_card];
                z80_byte pagina_es_ram=baseconf_memory_segments_type[i+inc_memory_card];

                if ((baseconf_shadow_mode_port_77&1)==0) {
                        //A8: if 0, then disable the memory manager. In each window processor is installed the last page of ROM. 0 after reset.
                        pagina=255;
                        pagina_es_ram=0;
                }
                
                else {
                //TODO: esto es mas complejo...
                //en modo pentagon1024 por ejenplo entran mas bits de puerto 32765
                
                if (i==3 && pagina_es_ram) {
                  pagina &= (255-7);
                  pagina |= (puerto_32765 & 7);
                }
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


Computer mybaseconf;

void evoReset(Computer* comp) {
	comp->dos = 1;
	comp->prt2 = 0x03;
	
	//no se por que queremos mapear trdos en reset
	
	//comp->dos = 0;
}




void baseconf_hard_reset(void)
{

  debug_printf(VERBOSE_DEBUG,"BaseConf Hard reset cpu");

  //Asignar bloques memoria
  
  int i;
  
  for (i=0;i<8;i++) {
  
  baseconf_memory_segments[i]=255;
  baseconf_memory_segments_type[i]=0;
  
  }

 
  reset_cpu();
  
  evoReset(&mybaseconf);


	


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






typedef unsigned char(*extmrd)(unsigned short, void*);
typedef void(*extmwr)(unsigned short, unsigned char, void*);


void memSetBank(Memory* mem GCC_UNUSED, int page, int type, int bank, int siz, extmrd rd, extmwr wr, void* data) {


int pagina_es_ram=1;

if (type==MEM_ROM) pagina_es_ram=0;
int i=page >> 6;
int pagina=bank;

printf ("mapeando pagina %d en %d tipo %d pc=%04XH\n",pagina,i,type,reg_pc);


baseconf_memory_segments_type[i]=pagina_es_ram;

if (pagina_es_ram) {
                        baseconf_memory_paged[i]=baseconf_ram_mem_table[pagina];
                        debug_paginas_memoria_mapeadas[i]=pagina;
                }
                else {
                        pagina=pagina & 31;
                        baseconf_memory_paged[i]=baseconf_rom_mem_table[pagina];
                        debug_paginas_memoria_mapeadas[i]=DEBUG_PAGINA_MAP_ES_ROM+pagina;
                }



	}


/*

void evoSetVideoMode(Computer* comp) {
	int mode = (comp->pEFF7 & 0x20) | ((comp->pEFF7 & 0x01) << 1) | (comp->prt2 & 0x07);	// z5.z0.0.b2.b1.b0	b:FF77, z:eff7
	switch (mode) {
		case 0x03: vidSetMode(comp->vid,VID_NORMAL); break;		// common
		case 0x13: vidSetMode(comp->vid,VID_ALCO); break;		// alco 16c
		case 0x23: vidSetMode(comp->vid,VID_HWMC); break;		// zx hardware multicolor
		case 0x02: vidSetMode(comp->vid,VID_ATM_HWM); break;	// atm hardware multicolor
		case 0x00: vidSetMode(comp->vid,VID_ATM_EGA); break;	// atm ega
		case 0x06: vidSetMode(comp->vid,VID_ATM_TEXT); break;	// atm text
		case 0x07: vidSetMode(comp->vid,VID_EVO_TEXT); break;	// pentevo text
		default: vidSetMode(comp->vid,VID_UNKNOWN); break;
	}
}


*/


void evoMRd(Computer* comp, unsigned short adr, int m1) {
	//if (m1 && (comp->dif->type == DIF_BDI)) {
	if (1) {	
		//if (comp->dos && (comp->mem->map[(adr >> 8) & 0xff].type == MEM_RAM) && (comp->prt2 & 0x40)) {
		if (comp->dos && (  baseconf_memory_segments_type[adr/16384]) && (comp->prt2 & 0x40)) {
			comp->dos = 0;
			if (comp->rom) comp->hw->mapMem(comp);
		}
		if (!comp->dos && ((adr & 0xff00) == 0x3d00) && (comp->rom)) {
			comp->dos = 1;
			comp->hw->mapMem(comp);
		}
	}
	//return memRd(comp->mem,adr);
}

void lee_byte_evo_aux(z80_int direccion)
{
	evoMRd(&mybaseconf,direccion,1);
}

void evoMWr(Computer* comp, unsigned short adr, unsigned char val) {
	//if (comp->evo.evoBF & 4) comp->vid->font[adr & 0x7ff] = val;	// PentEvo: write font byte
	//memWr(comp->mem,adr,val);
}



void evoSetBank(Computer* comp, int bank, memEntry me) {
	unsigned char page = me.page ^ 0xff;
	if (me.flag & 0x80) {
		if (me.flag & 0x40) {
			if (comp->pEFF7 & 4) {
				page = (page & 0xf8) | (comp->p7FFD & 7);				// mix with b0..2 (7FFD) - 128K mode
			} else {
				page = (page & 0xc0) | (comp->p7FFD & 7) | ((comp->p7FFD & 0xe0) >> 2);	// mix with b0..2,5..7 (7FFD) - P1024 mode
			}
		} else {
			page = (page & 0x3e) | (comp->dos ? 1 : 0);				// mix with dosen
		}
	}
	memSetBank(comp->mem, bank, (me.flag & 0x40) ? MEM_RAM : MEM_ROM, page, MEM_16K, NULL, NULL, NULL);
}

void evoMapMem(Computer* comp) {
	if (comp->prt2 & 0x20) {		// A8.xx77
		int adr = (comp->rom) ? 4 : 0;
		evoSetBank(comp,0x00,comp->memMap[adr]);
		evoSetBank(comp,0x40,comp->memMap[adr+1]);
		evoSetBank(comp,0x80,comp->memMap[adr+2]);
		evoSetBank(comp,0xc0,comp->memMap[adr+3]);
	} else {
		comp->dos = 1;
		memSetBank(comp->mem,0x00,MEM_ROM,0xff, MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0x40,MEM_ROM,0xff, MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0x80,MEM_ROM,0xff, MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0xc0,MEM_ROM,0xff, MEM_16K, NULL, NULL, NULL);
	}
	if (comp->pEFF7 & 8)	{
		printf ("mapping ram 0 to 0\n");
					// b3.EFF7: ram0 @ 0x0000 : high priority

		memSetBank(comp->mem,0x00,MEM_RAM,0x00, MEM_16K, NULL, NULL, NULL);
	}
}

// in

/*

unsigned char evoIn1F(Computer* comp, unsigned short port) {	// !dos
	return joyInput(comp->joy);
}

unsigned char evoIn57(Computer* comp, unsigned short port) {	// !dos
	return sdcRead(comp->sdc);
}

unsigned char evoIn77(Computer* comp, unsigned short port) {	// !dos
	unsigned char res = 0x02;		// rd only
	if (comp->sdc->image != NULL) res |= 0x01;
	return res;
}

*/

unsigned char evoInBE(Computer* comp, unsigned short port) {
	unsigned char res = 0xff;
	int i;
	if ((port & 0xf800) == 0x0000) {
		res = comp->memMap[(port & 0x0700) >> 8].page;
	} else {
		switch (port & 0xff00) {
			case 0x0800:
				res = 0;
				for (i = 0; i < 8; i++) {
					res = (res >> 1);
					if (comp->memMap[i].flag & 0x40) res |= 0x80;
				}
				break;
			case 0x0900:
				res = 0;
				for (i = 0; i < 8; i++) {
					res = (res >> 1);
					if (comp->memMap[i].flag & 0x80) res |= 0x80;
				}
				break;
			case 0x0a00: res = comp->p7FFD; break;
			case 0x0b00: res = comp->pEFF7; break;
			case 0x0c00: res = comp->prt2 | (comp->dos ? 0x10 : 0x00); break;
			default:
//				printf("PentEvo\tin %.4X.%i\n",port,bdiz);
//				assert(0);
				break;
		}
	}
	return res;
}

unsigned char evoInBF(Computer* comp, unsigned short port) {
	return comp->evo.evoBF;
}


/*
unsigned char evoInBDI(Computer* comp, unsigned short port) {
	unsigned char res;
	difIn(comp->dif, port, &res, 1);
	return res;
}
*/

unsigned char evoIn2F(Computer* comp, unsigned short port) {
	return comp->evo.evo2F;
}

unsigned char evoIn4F(Computer* comp, unsigned short port) {
	return comp->evo.evo4F;
}

unsigned char evoIn6F(Computer* comp, unsigned short port) {
	return comp->evo.evo6F;
}

unsigned char evoIn8F(Computer* comp, unsigned short port) {
	return comp->evo.evo8F;
}

/*

unsigned char evoInBEF7(Computer* comp, unsigned short port) {	// dos
	return cmsRd(comp);
}

unsigned char evoInBFF7(Computer* comp, unsigned short port) {	// !dos
	return (comp->pEFF7 & 0x80) ? cmsRd(comp) : 0xff;
}

*/

// out

void evoOutBF(Computer* comp, unsigned short port, unsigned char val) {
	comp->evo.evoBF = val;
}

/*
void evoOutFB(Computer* comp, unsigned short port, unsigned char val) {
	sdrvOut(comp->sdrv,0xfb,val);
}
*/

/*
void evoOutFE(ZXComp* comp, unsigned short port, unsigned char val) {
	comp->vid->nextbrd = (val & 0x07) | (~port & 8);
	if (!comp->vid->border4t) comp->vid->brdcol = comp->vid->nextbrd;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}
*/

void evoOut2F(Computer* comp, unsigned short port, unsigned char val) {
	comp->evo.evo2F = val;
}

void evoOut4F(Computer* comp, unsigned short port, unsigned char val) {
	comp->evo.evo4F = val;
}

void evoOut6F(Computer* comp, unsigned short port, unsigned char val) {
	comp->evo.evo6F = val;
}

void evoOut8F(Computer* comp, unsigned short port, unsigned char val) {
	comp->evo.evo8F = val;
}

/*
void evoOut57(Computer* comp, unsigned short port, unsigned char val) {
	sdcWrite(comp->sdc,val);
}
*/


void evoOut77(Computer* comp, unsigned short port, unsigned char val) {
	//comp->sdc->on = val & 1;
	//comp->sdc->cs = (val & 2) ? 1 : 0;
}



void evoOut77d(Computer* comp, unsigned short port, unsigned char val) {
	comp->prt2 = ((port & 0x4000) >> 7) | ((port & 0x0300) >> 3) | (val & 0x0f);	// a14.a9.a8.0.b3.b2.b1.b0
	//compSetTurbo(comp,(val & 0x08) ? 4 : ((comp->pEFF7 & 0x10) ? 1 : 2));
	//evoSetVideoMode(comp);
	evoMapMem(comp);
}

void evoOutF7(Computer* comp, unsigned short port, unsigned char val) {
	int adr = ((comp->rom) ? 4 : 0) | ((port & 0xc000) >> 14);
	if (port & 0x0800) {
		comp->memMap[adr].flag = val & 0xc0;
		comp->memMap[adr].page = val | 0xc0;
	} else {
		comp->memMap[adr].flag |= 0x40;		// ram
		comp->memMap[adr].page = val;
	}
	evoMapMem(comp);
}

/*

void evoOutBDI(Computer* comp, unsigned short port, unsigned char val) {		// dos
	difOut(comp->dif, port, val, 1);
}
*/

static const unsigned char atm3clev[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};


/*
void evoOutFF(Computer* comp, unsigned short port, unsigned char val) {		// dos
	difOut(comp->dif, 0xff, val, 1);
	if (comp->prt2 & 0x80) return;
	val ^= 0xff;	// inverse colors
	int adr = comp->vid->brdcol & 0x0f;
#if 1			// ddp extend palete
	port ^= 0xff00;
	if (!comp->ddpal) port = (port & 0xff) | ((val << 8) & 0xff00);
	comp->vid->pal[adr].b = atm3clev[((val & 0x01) << 3) | ((val & 0x20) >> 3) | ((port & 0x0100) >> 7) | ((port & 0x2000) >> 13)];
	comp->vid->pal[adr].r = atm3clev[((val & 0x02) << 2) | ((val & 0x40) >> 4) | ((port & 0x0200) >> 8) | ((port & 0x4000) >> 14)];
	comp->vid->pal[adr].g = atm3clev[((val & 0x10) >> 1) | ((val & 0x80) >> 5) | ((port & 0x1000) >> 11)| ((port & 0x8000) >> 15)];
#else
	comp->vid->pal[adr].b = ((val & 0x01) ? 0xaa : 0x00) + ((val & 0x20) ? 0x55 : 0x00);
	comp->vid->pal[adr].r = ((val & 0x02) ? 0xaa : 0x00) + ((val & 0x40) ? 0x55 : 0x00);
	comp->vid->pal[adr].g = ((val & 0x10) ? 0xaa : 0x00) + ((val & 0x80) ? 0x55 : 0x00);
#endif
}

*/

void evoOut7FFD(Computer* comp, unsigned short port, unsigned char val) {
	if ((comp->pEFF7 & 4) && (comp->p7FFD & 0x20)) return;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val;
	//comp->vid->curscr = (val & 0x08) ? 7 : 5;
	evoMapMem(comp);
}

/*

void evoOutBEF7(Computer* comp, unsigned short port, unsigned char val) {	// dos
	cmsWr(comp,val);
}

*/

/*

void evoOutDEF7(Computer* comp, unsigned short port, unsigned char val) {	// dos
	comp->cmos.adr = val;
}

*/

/*

void evoOutBFF7(Computer* comp, unsigned short port, unsigned char val) {	// !dos
	if (comp->pEFF7 & 0x80) cmsWr(comp,val);
}


*/

/*
void evoOutDFF7(Computer* comp, unsigned short port, unsigned char val) {	// !dos
	if (comp->pEFF7 & 0x80) comp->cmos.adr = val;
}
*/

void evoOutEFF7(Computer* comp, unsigned short port, unsigned char val) {	// !dos
	comp->pEFF7 = val;
	//compSetTurbo(comp,(comp->prt2 & 0x08) ? 4 : (val & 0x08) ? 2 : 1);
	//evoSetVideoMode(comp);
	evoMapMem(comp);
}

/*
static xPort evoPortMap[] = {
	{0x00f7,0x00fe,2,2,2,xInFE,	xOutFE},	// A3 = border bright
//	{0x00ff,0x00fb,2,2,2,NULL,	evoOutFB},	// covox
	{0x00ff,0x00be,2,2,2,evoInBE,	NULL},
	{0x00ff,0x00bf,2,2,2,evoInBF,	evoOutBF},
	{0xffff,0x7ffd,2,2,2,NULL,	evoOut7FFD},
	{0xffff,0xfadf,2,2,2,xInFADF,	NULL},		// k-mouse (fadf,fbdf,ffdf)
	{0xffff,0xfbdf,2,2,2,xInFBDF,	NULL},
	{0xffff,0xffdf,2,2,2,xInFFDF,	NULL},
	{0xfeff,0xbffd,2,2,2,NULL,	xOutBFFD},	// ay/ym; bffd, fffd
	{0xfeff,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	// dos only
	{0x009f,0x001f,1,2,2,evoInBDI,	evoOutBDI},	// bdi 1f,3f,5f,7f
	{0x00ff,0x00ff,1,2,2,evoInBDI,	evoOutFF},	// bdi ff + set palette
	{0x00ff,0x002f,1,2,2,evoIn2F,	evoOut2F},	// extend bdi ports
	{0x00ff,0x004f,1,2,2,evoIn4F,	evoOut4F},
	{0x00ff,0x006f,1,2,2,evoIn6F,	evoOut6F},
	{0x00ff,0x008f,1,2,2,evoIn8F,	evoOut8F},
	{0x00ff,0x0057,1,2,2,NULL,	evoOut77},	// 55,77 : spi. 55 dos = 77 !dos


	{0x00ff,0x0077,1,2,2,NULL,	evoOut77d},


	{0xffff,0xbef7,1,2,2,evoInBEF7,	evoOutBEF7},	// nvram
	{0xffff,0xdef7,1,2,2,NULL,	evoOutDEF7},
	{0x07ff,0x07f7,1,2,2,NULL,	evoOutF7},	// x7f7
	// !dos only
	{0x00ff,0x001f,0,2,2,evoIn1F,	NULL},		// k-joy
	{0x00ff,0x0057,0,2,2,evoIn57,	evoOut57},	// 57,77 : spi

	{0x00ff,0x0077,0,2,2,evoIn77,	evoOut77},

	{0xffff,0xbff7,0,2,2,evoInBFF7,	evoOutBFF7},	// nvram
	{0xffff,0xdff7,0,2,2,NULL,	evoOutDFF7},
	{0xffff,0xeff7,0,2,2,NULL,	evoOutEFF7},

	{0x0000,0x0000,2,2,2,NULL,	NULL}
};

*/

void evoOut(Computer* comp, unsigned short port, unsigned char val, int dos) {
	if (comp->evo.evoBF & 0x01) dos = 1;	// force open ports
	//zx_dev_wr(comp, port, val, dos);
	//hwOut(evoPortMap, comp, port, val, dos);
}

/*

unsigned char evoIn(Computer* comp, unsigned short port, int dos) {
	unsigned char res = 0xff;
	if (comp->evo.evoBF & 1) dos = 1;
	if (zx_dev_rd(comp, port, &res, dos)) return res;
	res = hwIn(evoPortMap, comp, port, dos);
	return res;
}
*/





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
        
        int inc_memory_card=0;
        if (puerto_32765 & 16) inc_memory_card=4;

      

        //xxBFH
        //Enable shadow mode ports write permission in ROM.
        if ( (puerto&0x00FF)==0xBF ) {
               baseconf_last_port_bf=valor; 

               //baseconf_set_memory_pages();
               evoOutBF(&mybaseconf,puerto,valor);
        }        

        //xx77H
        else if ( (puerto&0x00FF)==0x77 /*&& baseconf_shadow_ports_available() */) {
                baseconf_shadow_mode_port_77=puerto_h;
               baseconf_last_port_77=valor; 
               
			   if (mybaseconf.dos) evoOut77d(&mybaseconf,puerto,valor);
			   else evoOut77(&mybaseconf,puerto,valor);
               //baseconf_set_memory_pages();
        }

        else if (puerto==0xEFF7) {
                printf ("setting port EFF7 value\n");
                baseconf_last_port_eff7=valor;
                //baseconf_set_memory_pages();
                evoOutEFF7(&mybaseconf,puerto,valor);
                
        }


        //xFF7H
        //The memory manager pages.
        else if ( (puerto&0x0FFF)==0xFF7 && baseconf_shadow_ports_available() ) {
               

                
                  z80_byte pagina=valor ^ 255;
                         z80_byte es_ram=valor & 64;



                      z80_byte segmento=puerto_h>>6;
                      
                      segmento += inc_memory_card;
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
                 
                 
                evoOutF7(&mybaseconf,puerto,valor);


               //baseconf_set_memory_pages();
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

   segmento += inc_memory_card;

                if (valor&128) pagina=baseconf_change_ram_page_7ffd(pagina);

                baseconf_memory_segments[segmento]=pagina;  
                baseconf_memory_segments_type[segmento]=es_ram;      


     evoOutF7(&mybaseconf,puerto,valor);

               //baseconf_set_memory_pages();
        }        

        else if (puerto==0x7ffd) {
                //mapeamos ram y rom , pero sin habilitando memory manager
                //baseconf_shadow_ports |=1;

                //ram
                
                /*
                baseconf_memory_segments[3+inc_memory_card]=valor&7;
                baseconf_memory_segments_type[3+inc_memory_card]=1;
                
                */

                //rom
                /*
                baseconf_memory_segments[0] &=254;
                if (valor&16) baseconf_memory_segments[0] |= 1;
                baseconf_memory_segments_type[0]=0;       
*/

                puerto_32765=valor;
                
                evoOut7FFD(&mybaseconf,puerto,valor);

                //baseconf_set_memory_pages();

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





