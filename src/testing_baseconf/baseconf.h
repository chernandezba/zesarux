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

#ifndef BASECONF_H
#define BASECONF_H

#include "cpu.h"


#define BASECONF_LEFT_BORDER_NO_ZOOM 0
#define BASECONF_TOP_BORDER_NO_ZOOM 0

#define BASECONF_LEFT_BORDER BASECONF_LEFT_BORDER_NO_ZOOM*zoom_x
#define BASECONF_TOP_BORDER BASECONF_TOP_BORDER_NO_ZOOM*zoom_y

#define BASECONF_DISPLAY_WIDTH 720
#define BASECONF_DISPLAY_HEIGHT 576

#define BASECONF_FMAPS_SIZE 1024
#define BASECONF_ROM_PAGES 32
#define BASECONF_RAM_PAGES 256

//#define BASECONF_MAX_SPRITES 85

//extern z80_byte baseconf_last_port_eff7;
//extern z80_byte baseconf_last_port_dff7;
//extern z80_byte baseconf_nvram[];

//extern void baseconf_write_af_port(z80_byte puerto_h,z80_byte value);
//extern z80_byte baseconf_get_af_port(z80_byte index);
extern void baseconf_init_memory_tables(void);
extern void baseconf_set_memory_pages(void);
extern z80_byte *baseconf_memory_paged[];
extern z80_byte *baseconf_rom_mem_table[];
extern z80_byte *baseconf_ram_mem_table[];
//extern z80_byte baseconf_af_ports[];

//extern z80_byte baseconf_fmaps[];

//extern int temp_baseconf_in_system_rom_flag;

extern void baseconf_reset_cpu(void);
extern void baseconf_hard_reset(void);

extern z80_byte baseconf_memory_segments[];
extern z80_byte baseconf_memory_segments_type[];

extern void baseconf_out_port(z80_int puerto,z80_byte valor);

extern int baseconf_sd_enabled;

extern int baseconf_sd_cs;

extern int baseconf_shadow_ports_available(void);
extern z80_byte baseconf_last_port_bf;
extern void lee_byte_evo_aux(z80_int direccion);


#define MEM_SMALL_PAGES 0


enum {
        DIF_NONE = 0,
        DIF_BDI,
        DIF_P3DOS,
        DIF_SMK512,
        DIF_END = -1
};

// mempage type
enum {
	MEM_RAM	= 1,
	MEM_ROM,
	MEM_SLOT,
	MEM_EXT,
	MEM_IO
};

// memory size
#define MEM_256	(1<<8)
#define MEM_512	(1<<9)
#define MEM_1K	(1<<10)
#define MEM_2K	(1<<11)
#define MEM_4K	(1<<12)
#define MEM_8K	(1<<13)
#define MEM_16K	(1<<14)
#define MEM_32K	(1<<15)
#define MEM_64K	(1<<16)
#define MEM_128K	(1<<17)
#define MEM_256K	(1<<18)
#define MEM_512K	(1<<19)
#define MEM_1M	(1<<20)
#define MEM_2M	(1<<21)
#define MEM_4M	(1<<22)




typedef struct {
	int type;			// type of page data
	int num;			// 16K page number
	void* data;			// ptr for rd/wr func
	//extmrd rd;			// external rd (for type MEM_EXT)
	//extmwr wr;			// external wr (for type MEM_EXT)
} MemPage;

typedef struct {
	MemPage map[256];			// 4 x 16K | 256 x 256
	unsigned char ramData[0x400000];	// 4M
	unsigned char romData[0x80000];		// 512K
	int ramSize;
	int ramMask;
	int romSize;
	int romMask;
} Memory;


typedef struct {
	unsigned char flag;
	unsigned char page;
} memEntry;





typedef struct {
	unsigned brk:1;			// breakpoint
	unsigned debug:1;		// dont' do breakpoints
	unsigned maping:1;		// map memory during execution
	unsigned frmStrobe:1;		// new frame started
	unsigned intStrobe:1;		// int front
	unsigned nmiRequest:1;		// Magic button pressed
	unsigned firstRun:1;
	unsigned ddpal:1;

	unsigned rom:1;			// b4,7ffd
	unsigned dos:1;			// BDI dos
	unsigned cpm:1;

	unsigned evenM1:1;		// scorpion wait mode
	unsigned contMem:1;		// contended mem
	unsigned contIO:1;		// contended IO

	unsigned irq:1;
	unsigned brkirq:1;		// break on irq

	double fps;
	double cpuFrq;
	double frqMul;
	unsigned char intVector;

	char* msg;			// message ptr for displaying outside

	//DiskIF* dif;
	Memory* mem;

	struct HardWare *hw;
	
	int tickCount;
	int frmtCount;
	int nsPerTick;

	unsigned char p7FFD;		// stored port out
	unsigned char p1FFD;
	unsigned char pEFF7;
	unsigned char pDFFD;
	unsigned char p77hi;
	unsigned char prt2;		// scorpion ProfROM layer (0..3)
	unsigned char reg[256];		// internal registers
	unsigned char iomap[0x10000];
	unsigned short wdata;
	
	memEntry memMap[16];
	
	unsigned char brkRamMap[0x400000];	// ram brk/type : b0..3:brk flags, b4..7:type
	unsigned char brkRomMap[0x80000];	// rom brk/type : b0..3:brk flags, b4..7:type
	unsigned char brkAdrMap[0x10000];	// adr brk
	unsigned char brkIOMap[0x10000];	// io brk

	unsigned short padr;
	unsigned char pval;

/*
	struct {
		int flags;
		int cnt;	// ns counter
		int bper;	// base
		int per;	// current
		int ival;	// initial value
		int val;	// ticks counter
	} timer;
*/
	struct {
		unsigned char evoBF;		// PentEvo rw ports
		unsigned char evo2F;
		unsigned char evo4F;
		unsigned char evo6F;
		unsigned char evo8F;
		unsigned char blVer[16];	// bootloader info
		unsigned char bcVer[16];	// baseconf info
	} evo;
	
	struct {
		int flag;
		unsigned short tsMapAdr;	// adr for altera mapping
		unsigned char Page0;
//		unsigned char p00af;		// ports to be updated from next line
		unsigned char p01af;
		unsigned char p02af;
		unsigned char p03af;
		unsigned char p04af;
		unsigned char p05af;
//		unsigned char p07af;
		unsigned char p21af;
		unsigned char pwr_up;		// 1 on 1st run, 0 after reading 00AF
		unsigned char vdos;
	} tsconf;
	struct {
		unsigned char p7E;		// color num (if trig7E=0)
	} profi;
	struct {
		unsigned char keyLine;		// selected keyboard line
		unsigned char pA8;		// port A8
		unsigned char pAA;		// port AA
		unsigned char mFFFF;		// mem FFFF : mapper secondary slot
		unsigned char pF5;
		unsigned char pslot[4];
		unsigned char sslot[4];
		unsigned char memMap[4];	// RAM pages (ports FC..FF)
		struct {
			unsigned char regA;
			unsigned char regB;
			unsigned char regC;
		} ppi;
	} msx;
	struct {
		int type;			// DENDY | NTSC | PAL
		int priPadState;		// b0..7 = A,B,sel,start,up,down,left,right,0,0,0,0,....
		int secPadState;
		int priJoy;			// shift registers (reading 4016/17 returns bit 0 & shift right)
		int secJoy;			//		write b0=1 to 4016 restore status
	} nes;
	struct {
		unsigned boot:1;	// boot rom on
		unsigned inpint:1;	// button pressed: request interrupt
		int buttons;
		struct {
			struct {
				long per;
				long cnt;
			} div;		// divider (16KHz, inc FF04)
			struct {
				unsigned on:1;
				unsigned intrq:1;
				long per;
				long cnt;
			} t;		// manual timer
		} timer;

		int vbank;		// video bank (0,1)
		int wbank;		// ram bank (d000..dfff)
		int rbank;		// slot ram bank (a000..bfff)
		unsigned bgpal[0x3f];
		unsigned sppal[0x3f];
		unsigned char iram[256];	// internal ram (FF80..FFFE)
		unsigned short iomap[128];
	} gb;
	
//	int romsize;
	
	int resbank;			// rompart active after reset
//	int tapCount;
} Computer;



// reset
typedef void(*cbHwRes)(Computer*);
// map memory
typedef void(*cbHwMap)(Computer*);
// sync: calls after every CPU command
typedef void(*cbHwSnc)(Computer*, int);
// memory read
typedef unsigned char(*cbHwMrd)(Computer*, unsigned short, int);
// memory write
typedef void(*cbHwMwr)(Computer*, unsigned short, unsigned char);
// io read; TODO:remove last argument (bdi activity)
typedef unsigned char(*cbHwIrd)(Computer*, unsigned short, int);
// io write
typedef void(*cbHwIwr)(Computer*, unsigned short, unsigned char, int);
// key press/release
//typedef void(*cbHwKey)(Computer*, keyEntry);
// get volume
//typedef sndPair(*cbHwVol)(Computer*, sndVolume*);



struct HardWare {
        int id;                 // id
        int grp;
        const char* name;       // name used for conf file
        const char* optName;    // name used for setup window
        int base;
        int mask;               // mem size bits (see memory.h)
        cbHwMap mapMem;
        cbHwIwr out;            // io wr
        cbHwIrd in;             // io rd
        cbHwMrd mrd;            // mem rd
        cbHwMwr mwr;            // mem wr
        cbHwRes reset;          // reset
        cbHwSnc sync;           // sync time
        //cbHwKey keyp;           // key press
        //cbHwKey keyr;           // key release
        //cbHwVol vol;            // read volume
};

#endif
