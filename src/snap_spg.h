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

#ifndef SNAP_SPG_H
#define SNAP_SPG_H

#include "cpu.h"

struct blkSPG1_0{
	z80_byte addr;
	z80_byte size;
	z80_byte page;
};

struct hdrSPG1_0 {
	z80_byte author[32];
	z80_byte sign[12];	// These are common for versions 1.0 and 0.2
	z80_byte ver;			//
	z80_byte day;
	z80_byte month;
	z80_byte year;
	z80_int pc;
	z80_int sp;
	z80_byte win3_pg;
	z80_byte clk;
	z80_int pgmgr_addr;
	z80_int rsd_addr;
	z80_int n_blk;
	z80_byte second;
	z80_byte minute;
	z80_byte hour;
	z80_byte res1[17];
	z80_byte creator[32];
	z80_byte res2[144];
	struct blkSPG1_0 blocks[256];
	z80_byte data;
};

struct blkSPG0_2{
	z80_int addr;
	z80_byte size;
	z80_byte page;
};

struct hdrSPG0_2 {
	z80_byte res1[32];
	z80_byte sign[12];	// These are common for versions 1.0 and 0.2
	z80_byte ver;			//
	z80_int crc;
	z80_int crc_inv;
	z80_int port_xi_addr;
	z80_byte port_xi_val;
	z80_byte res2[64-52];
	z80_int pc;
	z80_byte win3_pg;
	z80_byte flag;
	z80_int pgmgr_addr;
	z80_byte day;
	z80_byte month;
	z80_byte year;
	z80_byte assy_ver;
	z80_int sp;
	z80_int vars_addr;
	z80_int vars_len;
	z80_byte res3[128-80];
	struct blkSPG0_2 blocks[16];
	z80_byte vars[320];
	z80_byte data;
};


extern void load_spg_snapshot(char *filename);

#endif
