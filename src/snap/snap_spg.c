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
#include <ctype.h>

#include <string.h>

#include "snap_spg.h"
#include "snap.h"
#include "cpu.h"
#include "debug.h"
#include "operaciones.h"
#include "mem128.h"
#include "tsconf.h"

extern void demlz(z80_byte *dst, z80_byte *src, int size);
extern z80_int dehrust(z80_byte* dst, z80_byte* src, int size);


z80_byte *snbuf;

#define MAX_SPG_SNAPSHOT (4*1024*1024)

void load_spg_snapshot_free(void)
{
  free(snbuf);
}


/*void demlz(z80_byte *zxram, z80_byte *data, int size)
{
  int leidos=mlz_decompress_simple(
	zxram,
	16384, // = limit
	data,
	size
);

  debug_printf (VERBOSE_DEBUG,"Uncompressed mlz size: %d",leidos);

}*/

void load_spg_snapshot(char *filename)
{
  //Asignamos 4 MB para snapshot.
  snbuf=malloc(MAX_SPG_SNAPSHOT);
  if (snbuf==NULL) {
    cpu_panic("Can not allocate memory for spg snapshot load");
  }

  FILE *ff = fopen(filename, "rb");
  if (!ff) return;
  int snapsize = fread(snbuf, 1, MAX_SPG_SNAPSHOT, ff);
  fclose(ff);

  debug_printf(VERBOSE_DEBUG,"Read %d bytes of snapshot %s",snapsize,filename);

  if (snapsize==MAX_SPG_SNAPSHOT) {
    debug_printf(VERBOSE_ERR,"Snapshot too big");
    load_spg_snapshot_free();
    return;
  }

  struct hdrSPG1_0 *hdr10 = (struct hdrSPG1_0 *)snbuf;
  //struct hdrSPG0_2 *hdr02 = (struct hdrSPG0_2 *)snbuf;




  if (memcmp(&hdr10->sign, "SpectrumProg", 12)) {
    debug_printf(VERBOSE_ERR,"Unknown snapshot signature");
    load_spg_snapshot_free();
    return;
  }




  	z80_byte snap_type = hdr10->ver;
  debug_printf (VERBOSE_DEBUG,"Snapshot type: %02XH",snap_type);

  	if ((snap_type != 0) && (snap_type != 1) && (snap_type != 2) && (snap_type != 0x10)) {
      debug_printf(VERBOSE_ERR,"Unknown snapshot type: %02XH",snap_type);
      load_spg_snapshot_free();
      return;
    }

    current_machine_type=MACHINE_ID_TSCONF;
        set_machine(NULL);
        //reset_cpu();
      //tsconf_hard_reset();
	cold_start_cpu_registers(); //cold start ya hace hard reset e inicializa paleta de colores basicos



  	reg_iy=0x5C3A;
    reg_l_shadow=0x58;
    reg_h_shadow=0x27;
  	reg_i=63;
    im_mode=1;
  	puerto_32765 = 16;

  	/* SPG ver.1.0 */

  	if (snap_type == 0x10)
  	{
  		reg_sp = hdr10->sp;
  		reg_pc = hdr10->pc;
      debug_printf(VERBOSE_DEBUG,"Register PC set to %04XH",reg_pc);
      iff1.v = (hdr10->clk & 4) ? 1 : 0;
      iff2.v = iff1.v;
	debug_printf(VERBOSE_DEBUG,"Setting iff1/2 to %d", iff2.v);
  		//comp.ts.zclk = hdr10->clk & 3;
      tsconf_af_ports[0x13]= hdr10->win3_pg;
      debug_printf(VERBOSE_DEBUG,"Paging RAM %02XH to C000H",hdr10->win3_pg);
			tsconf_set_memory_pages();



  		z80_byte *data = &hdr10->data;
      z80_byte i;
  		for (i = 0; i < hdr10->n_blk; i++)
  		{
  			z80_int size = ((hdr10->blocks[i].size & 0x1F) + 1) * 512;
  			z80_byte page = hdr10->blocks[i].page;
  			z80_int offs = (hdr10->blocks[i].addr & 0x1F) * 512;
  			//z80_byte *zxram = page_ram(page) + offs;
        z80_byte *zxram = tsconf_ram_mem_table[page] + offs;
  			switch ((hdr10->blocks[i].size & 0xC0) >> 6)
  			{
  				case 0x00:
            debug_printf(VERBOSE_DEBUG,"Copying block type 0 (uncompressed). Size: %d Page: %d Offset: %d",size,page,offs);
  					memcpy(zxram, data, size);
  					break;

  				case 0x01:
            debug_printf(VERBOSE_DEBUG,"Uncompressing block type 1 (mlz). Size: %d Page: %d Offset: %d",size,page,offs);
  					demlz(zxram, data, size);
  					break;

  				case 0x02:
            debug_printf(VERBOSE_DEBUG,"Uncompressing block type 2 (hrust). Size: %d Page: %d Offset: %d",size,page,offs);
            //printf("Uncompressing block type 2 (hrust). Size: %d Page: %d Offset: %d\n",size,page,offs);
  					dehrust(zxram, data, size);
  					break;
  			}

  			data += size;

  			if (hdr10->blocks[i].addr & 0x80)
  				break;
  		}
  	}


  	/* SPG ver.0.x */

  	else
  	{
      debug_printf(VERBOSE_ERR,"Unsupported SPG file ver.0.x. TO-DO!");

  		/*cpu.sp = hdr02->sp;
  		cpu.pc = hdr02->pc;
  		cpu.iff1 = 0;
  		comp.ts.zclk = 0;
  		comp.ts.page[3] = hdr02->win3_pg & ((type == 2) ? 15 : 7);

  		if (hdr02->vars_len)
  		{
  			z80_int addr;

  			if (!hdr02->vars_addr)
  				addr = 0x5B00;
  			else
  				addr = hdr02->vars_addr;
  			addr -= 0x4000;		// fucking crotch for only loading to the lower memory

  			memcpy(page_ram(5) + addr, hdr02->vars, hdr02->vars_len);
  		}

  		z80_byte *data = &hdr02->data;
  		for (z80_byte i = 0; i < 15; i++)
  		{
  			z80_int addr = hdr02->blocks[i].addr;
  			if (addr < 0x9A00)
  				break;
  			int size = hdr02->blocks[i].size * 2048;
  			if (!i)
  				size += 1536;
  			z80_byte page = hdr02->blocks[i].page & ((type == 2) ? 15 : 7);
  			int offs = (addr < 0xC000) ? (addr - 0x8000) : (addr - 0xC000);

  			if (addr < 0xC000)
  			{
  				int sz = ((size + offs) > 0x4000) ? (0x4000 - offs) : size;
  				memcpy(page_ram(2) + offs, data, sz);
  				offs = 0x0000;
  				size -= sz;
  				data += sz;
  			}

  			if (size)
  			{
  				memcpy(page_ram(page) + offs, data, size);
  				data += size;
  			}

  			if (type != 2)		// for ver. 0.0 and 0.1 there are 4 dummy bytes in every block descriptor
  				i++;
  		}

  		if (hdr02->pgmgr_addr)
  		{
  			// const z80_byte mgr[] = {0xC5, 0x01, 0xAF, 0x13, 0xED, 0x79, 0xC1, 0xC9};	// TS-conf
  			const z80_byte mgr[] = {0xC5, 0x4F, 0xE6, 0xF8, 0x79, 0x28, 0x04, 0xE6, 0x07, 0xF6, 0x40, 0xF6, 0x10, 0x01, 0xFD, 0x7F, 0xED, 0x79, 0xC1, 0xC9};	// Pentagon
  			memcpy(page_ram(5) + hdr02->pgmgr_addr - 0x4000, mgr, sizeof(mgr));
  		}
      */
  	}

  	//set_clk();
  	//set_banks();

  	//snbuf[0x20] = 0;	// to avoid garbage in header
  	//SetWindowText(wnd, (char*)snbuf);
  	//return 1;


    //sleep(2);

  load_spg_snapshot_free();
}
