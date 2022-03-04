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

Contains code from PlayTZX for Linux

Version v0.12c by Philip Kendall 2005-05-16
Based very heavily on original PlayTZX v0.56b by Tomaz Kac 1998

*/


#include "audio.h"
//#include "au-file.h"

/* .au file things for TZX-stuff
 * 
 * Created 27.10.1997 by Tero Turtiainen
 *
 * I was too lazy to make this little/big-endian independant. I will do it
 * later
 */

typedef struct 
{
	int magic;               /* magic number SND_MAGIC */
	int dataLocation;        /* offset or pointer to the data */
	int dataSize;            /* number of bytes of data */
	int dataFormat;          /* the data format code */
	int samplingRate;        /* the sampling rate */
	int channelCount;        /* the number of channels */
	char info[4];            /* optional text information */
} SNDSoundStruct;

#define SND_MAGIC 0x2e736e64

#define SND_FORMAT_LINEAR_8 2


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"
#include "utils.h"

#define HTYPES  0x10
#define HIDS    0x1E

char htype[HTYPES][256]=                {
	
	"Computer","External Storage","ROM/RAM Type Add-On","Sound Device",
	"Joystick","Mouse","Other Controller","Serial Port","Parallel Port",
	"Printer","Modem","Digitiser","Network Adapter","Keyboard or Keypad",
	"AD/DA Converter","EPROM Programmer"
};
char hid[HTYPES][HIDS][256]={
	{
		"ZX Spectrum 16k", "ZX Spectrum 48k, plus","ZX Spectrum 48k Issue 1",
		"ZX Spectrum 128k (Sinclair)","ZX Spectrum 128k +2 (Grey case)",
		"ZX Spectrum 128k +2A, +3","Timex Sinclair TC-2048",
		"Timex Sinclair TS-2068","Pentagon 128","Sam Coupe","Didaktik M",
		"Didaktik Gama","ZX-81 with 1k RAM","ZX-81 with 16k RAM or more",
		"ZX Spectrum 128k, Spanish version","ZX Spectrum, Arabic version",
		"TK 90-X","TK 95","Byte","Elwro","ZS Scorpion",
		"Amstrad CPC 464","Amstrad CPC 664","Amstrad CPC 6128",
		"Amstrad CPC 464+","Amstrad CPC 6128+","Jupiter ACE",
		"Enterprise", "Commodore 64", "Commodore 128"
	},
	{
		"Microdrive","Opus Discovery","Disciple","Plus-D",
		"Rotronics Wafadrive","TR-DOS (BetaDisk)","Byte Drive","Watsford",
		"FIZ","Radofin","Didaktik disk drives","BS-DOS (MB-02)",
		"ZX Spectrum +3 disk drive","JLO (Oliger) disk interface",
		"FDD3000","Zebra disk drive","Ramex Millenia","Larken"
	},
	{
		"Sam Ram","Multiface","Multiface 128k","Multiface +3","MultiPrint",
		"MB-02 ROM/RAM expansion"
	},
	{
		"Classic AY hardware (compatible with 128k ZXs)",
		"Fuller Box AY sound hardware","Currah microSpeech","SpecDrum",
		"AY ACB stereo; Melodik","AY ABC stereo"
	},
	{ 
		"Kempston","Cursor, Protek, AGF","Sinclair 2 Left",
		"Sinclair 1 Right","Fuller"
	},
	{ "AMX Mouse","Kempston mouse" },
	{ "Trickstick","ZX Light Gun","Zebra Graphics Tablet" },
	{ "ZX Interface 1","ZX Spectrum 128k" },
	{
		"Kempston S","Kempston E","ZX Spectrum +3","Tasman","DK'Tronics",
		"Hilderbay","INES Printerface","ZX LPrint Interface 3",
		"MultiPrint","Opus Discovery","Standard 8255 chip"
	},
	{
		"ZX Printer, Alphacom 32 & Compatibles","Generic Printer",
		"EPSON Compatible"
	},
	{ "VTX 5000","T/S 2050 or Westridge 2050" },
	{ "RD Digital Tracer","DK'Tronics Light Pen","British MicroGraph Pad" },
	{ "ZX interface 1" },
	{ "Keypad for ZX Spectrum 128k" },
	{ "Harley Systems ADC 8.2","Blackboard Electronics" },
	{ "Orme Electronics" }
};


/* Major revision of the format this program supports */
#define MAJREV 1

/* Minor revision of the format this program supports */
#define MINREV 13

/* C64 Loader defines ...*/

#define ROM_S_HALF      616	/* ROM Loader SHORT  Half Wave */
#define ROM_M_HALF      896	/* ROM Loader MEDIUM Half Wave */
#define ROM_L_HALF      1176	/* ROM Loader LONG   Half Wave */

#define STT_0_HALF      426	/* Standard Turbo Tape BIT 0 Half Wave */
#define STT_1_HALF      596	/* Standard Turbo Tape BIT 1 Half Wave */

int playtzx_Error(char *errstr)
{
	/* exits with an error message *errstr */
	
	debug_printf (VERBOSE_ERR,"Error: %s", errstr);
	return 1;
	
}



int n, m;
int num;
char *d;
int line = 3;
//int fh;				/* Input File Handle */

FILE *ptr_archivo_entrada;

int ofh;			/* Output File Handle */
int flen;			/* File Length */

unsigned char *mem;		/* File in Memory */
unsigned char *memorig;		/* Valor de mem justo al asignar memoria */

int pos;			/* Position in File */
int curr;			/* Current block that is playing */
int numblocks;			/* Total Num. of blocks */
int *block = NULL;		/* Array of Block starts */
/*size_t*/ int blocks_allocated = 0;	/* The current size of the block[] array */
double cycle;			/* Freq/3500000 */

char *sbbuf[2];			/* SB Buffers */
int sbbuflen = 1024;		/* Length of buffers */
int freq = 44100;		/* Sample Freq.  (SHOULD THIS BE 45454 ????) NOT !!! */
int sbcurrpage = 1;		/* Buffer that is currently available for writing */
int sbpos = 0;			/* Position in the buffer */
int amp;			/* Amplitude of the current signal */
int prvi;

int cpc = 0;			/* Amstrad CPC tape ? */
int sam = 0;			/* SAM Coupe tape ? */

int id;				/* Current Block ID */
int pilot;			/* Len of Pilot signal (in hp's) */
int sb_pilot;			/* Pilot pulse */
int sb_sync1;			/* Sync first half-period (hp) */
int sb_sync2;			/* Sync second */
int sb_bit0;			/* Bit-0 */
int sb_bit1;			/* Bit-1 */
int sb_pulse;			/* Pulse in Sequence of pulses and direct recording block */
int lastbyte;			/* How many bits are in last byte of data ? */
int tzx_pause;			/* Pause after current block (in 1s/10) */
unsigned char *data;			/* Data to be played */
int datalen;			/* Len of ^^^ */
int datapos;			/* Position in ^^^ */
int bitcount;			/* How many bits to play in current byte ? */
int sb_bit;			/* should we play bit 0 or 1 ? */
char databyte;			/* Current Byte to be replayed of the data */
signed short jump;		/* Relative Jump */
int not_rec;			/* Some blocks were not recognised ?? */
int files = 0;			/* Number of Files on the command line */
char finp[255];			/* Input File  (First Command Line Option) */
char fout[255];			/* Output File (Second Command Line Option or First with .VOC) */
char errstr[255];		/* Error String */
int starting = 1;		/* starting block */
int ending = 0;			/* ending block */

int voc = 0;			/* Are we making a .VOC file ? */
int au = 0;			/* Are we making a .au file? */
int tzx_info = 0;			/* if info=1 then show EXTENSIVE information */
/* info=2 then show ONE LINE of Info per block */
int pages = 0;			/* Waiting after each page of the info ? */
int expand = 0;			/* Expand Groups ? */
int draw = 1;			/* Local flag for outputing a line when in a
group */
int mode128 = 0;		/* Are we working in 128k mode ? (for Stop in 48k block) */

char vochead[0x20] = {'C', 'r', 'e', 'a', 't', 'i', 'v', 'e', ' ', 'V', 'o', 'i', 'c', 'e', ' ', 'F', 'i', 'l', 'e',
	0x1A, 0x1A, 0x00, 0x0A, 0x01, 0x29, 0x11};
	char *vocbuf;			/* Buffer for .VOC block */
	int vocbuflen = 0xFFFF;		/* Length of .VOC block (and buffer) */
	char vocstart[4] = {0x02, 0xFF, 0xFF, 0x00};
	int vocpos;			/* Length of current .VOC block */
	
	int nfreq = 0;			/* Did we choose new frequency with /freq switch ? */
	char k;
	int speed;
	int x, last, lastlen;
	
	int loop_start = 0;		/* Position of the last Loop Start block */
	int loop_count = 0;		/* Counter of the Loop */
	int call_pos = 0;		/* Position of the last Call Sequence block */
	int call_num = 0;		/* Number of Calls in the last Call Sequence block */
	int call_cur = 0;		/* Current Call to be made */
	int num_sel;			/* Number of Selections in the Select block */
	int jumparray[256];		/* Array of all possible jumps in Select block */
	
	int sb_bit0_f, sb_bit0_s, sb_bit1_f, sb_bit1_s, xortype, sb_finishbyte_f, sb_finishbyte_s,
	sb_finishdata_f, sb_finishdata_s, num_lead_in, xorvalue;
	int trailing, sb_trailing;
	char lead_in_byte;
	int endian;
	char add_bit;
	
	#define LOAMP   0x10		/* Low Level Amplitude */
	#define HIAMP   0xF0		/* High Level Amplitude */
	
	char tstr[255];
	char tstr2[255];
	char tstr3[255];
	char tstr4[255];
	char spdstr[255];
	char pstr[255];
	
	SNDSoundStruct auhead;  /* .au-file header */
	
	int numt, nump, t2;
	
	/* Conversion functions to get 2,3 and 4 byte words ...*/
	int 
	Get2(unsigned char *mem)
	{
		return (mem[0] + (mem[1] * 256));
	}
	
	int 
	Get3(unsigned char *mem)
	{
		return (mem[0] + (mem[1] * 256) + (mem[2] * 256 * 256));
	}
	
	int 
	Get4(unsigned char *mem)
	{
		return (mem[0] + (mem[1] * 256) + (mem[2] * 256 * 256) + (mem[3] * 256 * 256 * 256));
	}
	
	/* This will convert a sampling value in Z80 T-States to samples for SoundBlaster/VOC output */
	int 
	ConvSB(int n)
	{
		return ((int) (0.5 + (cycle * (double) n)));
	}
	
	//#define GetCh() getchar()
	
	#define GetCh() 0
	
	#define PLAYTZX_AUDIO_BUFFER_SIZE 4096
	unsigned char playtzx_audio_buffer[PLAYTZX_AUDIO_BUFFER_SIZE];
	int buf_index = 0;
	int audiofd;
	
	int 
	FileLength(int fh)
	{
		int curpos, size;
		
		curpos = lseek(fh, 0, SEEK_CUR);
		size = lseek(fh, 0, SEEK_END);
		lseek(fh, curpos, SEEK_SET);
		return (size);
	}
	
	int 
	kbhit()
	{
		return (1);
	}
	
	
	
	int
	lsb_write(int fh, int value)
	{
		char buffer[4];
		
		buffer[3] = value & 0xff;
		value >>= 8;
		
		buffer[2] = value & 0xff;
		value >>= 8;
		
		buffer[1] = value & 0xff;
		value >>= 8;
		
		buffer[0] = value & 0xff;
		
		return (write(fh, buffer, 4));
	}
	
	void 
	PlayAU(char amp, int len)
	{
		
		/* Puts amplitude amp to .VOC file buffer for len time ... This .VOC file
		 *buffer is also used for .au files */
		
		auhead.dataSize += len;
		
		while (len) {
			vocbuf[vocpos] = amp;
			
			len--;
			vocpos++;
			
			if (vocpos == vocbuflen) {	/* Do we need to write the buffer to the .VOC file ? */
				/* write(ofh, vocstart, 4); */
				
				write(ofh, vocbuf, vocbuflen);
				
				vocpos = 0;
			}
		}
		
	}
	
	void 
	PlaySB(char amp, int len)
	{
		
		/* Puts amplitude amp to SoundBlaster buffer for len time (or calls PlayVOC if converting) ...*/
		/* This would be fun to do in C++ :)*/
		
		if (voc) {
			//PlayVOC(amp, len);
			return;
		} 
		else if (au) {
			PlayAU(amp, len);
			return;
		}
		while (len) {
			playtzx_audio_buffer[buf_index++] = amp;
			
			len--;
			
			//if (buf_index == PLAYTZX_AUDIO_BUFFER_SIZE) {	/* Is the buffer full ? */
			//  flushSBBuffer();
			//}
		}
		
	}
	
	
	void 
	PauseSB(char amp, int tzx_pause)
	{
		
		/* Waits for tzx_pause milliseconds*/
		int p;
		
		
		p = (int) ((((float) tzx_pause) * freq) / 1000.0);
		
		PlaySB(amp, p);
		
	}
	
	
	
	void 
	InitAU()
	{
		
		/* Prepares AU file to be written to ... */
		auhead.magic = SND_MAGIC;
		auhead.dataLocation = 32;
		auhead.dataSize = 0;		/* will be filled when the file is written */
		auhead.dataFormat = SND_FORMAT_LINEAR_8;
		auhead.samplingRate = freq;
		auhead.channelCount = 1;
		/* head.info = '    '; */
		
		vocbuf = (char *) malloc(vocbuflen + 256);
		
		if (vocbuf == NULL) {
			free(memorig);
			playtzx_Error("Not enough memory to set up .VOC file buffer!");
			//return 1;
			return;
		}
		
		
		ofh = open(fout, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		
		/* No meter cabecera AU 
		 * write(ofh, &auhead, 0x1A);	// this is propably rubbish now... :) 
		 * 
		 * // then go to position 32 from the beginning of the file 
		 * lseek(ofh, 32, SEEK_SET);
		 */
		
		vocpos = 0;
	}
	
	void
	StopAU()
	{
		/* Finishes the au recording (writes the last buffer and puts the header to
		 *the beginning of the file */
		
		if (vocpos) {
			write(ofh, vocbuf, vocpos);
		}
		
		
		/* Desactivar escritura cabecera AU
		 * lseek(ofh, 0, SEEK_SET);
		 * 
		 * // now write the header info in LSB-format 
		 * 
		 * lsb_write(ofh, auhead.magic);
		 * lsb_write(ofh, auhead.dataLocation);
		 * lsb_write(ofh, auhead.dataSize);
		 * lsb_write(ofh, auhead.dataFormat);
		 * lsb_write(ofh, auhead.samplingRate);
		 * lsb_write(ofh, auhead.channelCount);
		 */
		
		free(vocbuf);
		
		close(ofh);
	}
	
	void 
	ToggleAmp()
	{
		/* Toggles the amplitude of the output*/
		
		if (amp == LOAMP)
			amp = HIAMP;
		else
			amp = LOAMP;
	}
	
	void 
	PlayC64SB(int len)
	{
		PlaySB(amp, len);
		ToggleAmp();
		PlaySB(amp, len);
		ToggleAmp();
	}
	
	void 
	GetC64ROMName(char *name, unsigned char *data)
	{
		char d;
		
		for (n = 0; n < 16; n++) {
			d = data[14 + n];
			if (d < 32 || d > 125)
				name[n] = ' ';
			else
				name[n] = d;
		}
		name[n] = 0;
	}
	
	char 
	MirrorByte(char s)
	{
		return ((s << 7) + ((s << 5) & 64) + ((s << 3) & 32) + ((s << 1) & 16) + ((s >> 1) & 8) + ((s >> 3) & 4) + ((s >> 5) & 2) + (s >> 7));
	}
	
	void 
	GetC64StandardTurboTapeName(char *name, unsigned char *data)
	{
		char d;
		
		for (n = 0; n < 16; n++) {
			d = data[15 + n];
			if (d < 32 || d > 125)
				name[n] = ' ';
			else
				name[n] = d;
		}
		name[n] = 0;
	}
	
	void 
	IdentifyC64ROM(int pos, unsigned char *data, int type)
	{
		char name[255];
		
		/* Determine Loader type */
		if (sb_pilot == ROM_S_HALF && sb_sync1 == ROM_L_HALF && sb_sync2 == ROM_M_HALF &&
			sb_bit0_f == ROM_S_HALF && sb_bit0_s == ROM_M_HALF && sb_bit1_f == ROM_M_HALF &&
			sb_bit1_s == ROM_S_HALF && xortype == 0x01) {
			/* ROM Loader */
			if (data[0] == 0x89 && data[1] == 0x88 && data[2] == 0x87 && data[3] == 0x86 &&
				data[4] == 0x85 && data[5] == 0x84 && data[6] == 0x83 && data[7] == 0x82 &&
				data[8] == 0x81) {
				if (pos == 202) {
					if (!type) {
						strcpy(name, "Header: ");
						GetC64ROMName(name + 8, data);
					} else {
						strcpy(name, "ROM Header: ");
						GetC64ROMName(name + 12, data);
					}
				} else {
					if (!type) {
						strcpy(name, "Data Block              ");
					} else {
						strcpy(name, "ROM: Data Block");
					}
				}
				} else {
					if (!type)
						strcpy(name, "------------------------");
					else
						strcpy(name, "ROM: Last Block Repeated");
				}
				strcpy(tstr, name);
				strcpy(spdstr, "C64 ROM Data ");
				return;
			}
			if (!type)
				strcpy(tstr, "------------------------");
			else
				strcpy(tstr, "Unknown");
			strcpy(spdstr, "C64 Data     ");
	}
	
	void 
	IdentifyC64Turbo(int pos, unsigned char *data, int type)
	{
		char name[255];
		
		/* Determine Loader type */
		if (sb_bit0 == STT_0_HALF && sb_bit1 == STT_1_HALF && lead_in_byte == 0x02) {
			/* Standard Turbo Tape Loader */
			if (data[0] == 0x09 && data[1] == 0x08 && data[2] == 0x07 && data[3] == 0x06 &&
				data[4] == 0x05 && data[5] == 0x04 && data[6] == 0x03 && data[7] == 0x02 &&
				data[8] == 0x01) {
				if (pos == 32 && data[9] != 0x00) {
					if (!type) {
						strcpy(name, "Header: ");
						GetC64StandardTurboTapeName(name + 8, data);
					} else {
						strcpy(name, "TurboTape Header: ");
						GetC64StandardTurboTapeName(name + 18, data);
					}
				} else {
					if (!type) {
						strcpy(name, "------------------------");
					} else {
						strcpy(name, "TurboTape Data Block");
					}
				}
				} else {
					if (!type)
						strcpy(name, "------------------------");
					else
						strcpy(name, "TurboTape Unknown");
				}
				strcpy(tstr, name);
				strcpy(spdstr, "C64 Turbo    ");
				return;
		}
		if (!type)
			strcpy(tstr, "------------------------");
		else
			strcpy(tstr, "Unknown");
		strcpy(spdstr, "C64 Data     ");
	}
	
	void 
	Identify(int len, unsigned char *temp, int type)
	{
		int n, s;
		
		if (cpc) {
			if (temp[0] == 44) {
				if (!type)
					s = 4;
				else
					s = 0;
				
				strcpy(tstr, "    ");
				
				for (n = 0; n < 16; n++) {
					if (temp[n + 1])
						tstr[n + s] = temp[n + 1];
					else
						tstr[n + s] = ' ';
				}
				
				for (n = 0; n < 4; n++)
					tstr[n + s + 16] = ' ';
				
				tstr[n + s + 16] = 0;
				
			} else {
				if (!type)
					strcpy(tstr, "    ------------------  ");
				else
					strcpy(tstr, "Headerless");
			}
			
			return;
		}
		if (sam) {
			if (temp[0] == 1 && (len > 80 && len < 84) && (temp[1] >= 0x10 && temp[1] <= 0x13)) {
				if (!type) {
					s = 14;
					
					switch (temp[1]) {
						case 0x10:
							strcpy(tstr, "    Program : ");
							break;
							
						case 0x11:
							strcpy(tstr, " Num. Array : ");
							break;
							
						case 0x12:
							strcpy(tstr, "Char. Array : ");
							break;
							
						case 0x13:
							strcpy(tstr, "      Bytes : ");
							break;
							
					}
				} else {
					switch (temp[1]) {
						case 0x10:
							strcpy(tstr, "Program : ");
							s = 10;
							break;
							
						case 0x11:
							strcpy(tstr, "Num. Array : ");
							s = 13;
							break;
							
						case 0x12:
							strcpy(tstr, "Char. Array : ");
							s = 14;
							break;
							
						case 0x13:
							strcpy(tstr, "Bytes : ");
							s = 8;
							break;
							
					}
				}
				
				for (n = 0; n < 10; n++) {
					if (temp[n + 2] > 31 && temp[n + 2] < 127)
						tstr[n + s] = temp[n + 2];
					else
						tstr[n + s] = 32;
				}
				
				tstr[n + s] = 0;
			} else {
				if (!type)
					strcpy(tstr, "    --------------------");	/* Not Header */
					else
						strcpy(tstr, "Headerless");
			}
			return;
		}
		if (temp[0] == 0 && (len == 19 || len == 20) && temp[1] < 4) {
			if (!type) {
				s = 14;
				
				switch (temp[1]) {
					case 0x00:
						strcpy(tstr, "    Program : ");
						break;
						
					case 0x01:
						strcpy(tstr, " Num. Array : ");
						break;
						
					case 0x02:
						strcpy(tstr, "Char. Array : ");
						break;
						
					case 0x03:
						strcpy(tstr, "      Bytes : ");
						break;
						
				}
			} else {
				switch (temp[1]) {
					case 0x00:
						strcpy(tstr, "Program : ");
						s = 10;
						break;
						
					case 0x01:
						strcpy(tstr, "Num. Array : ");
						s = 13;
						break;
						
					case 0x02:
						strcpy(tstr, "Char. Array : ");
						s = 14;
						break;
						
					case 0x03:
						strcpy(tstr, "Bytes : ");
						s = 8;
						break;
				}
			}
			
			for (n = 0; n < 10; n++) {
				if (temp[n + 2] > 31 && temp[n + 2] < 127)
					tstr[n + s] = temp[n + 2];
				else
					tstr[n + s] = 32;
				
			}
			
			tstr[n + s] = 0;
		} else {
			if (!type)
				strcpy(tstr, "    --------------------");	/* Not Header */
				else
					strcpy(tstr, "Headerless");
		}
		
	}
	
	
	int 
	getnumber(char *s)
	{
		/* Returns the INT number contained in string *s */
		
		int i;
		
		sscanf(s, "%d", &i);
		return (i);
	}
	
	
	void 
	ChangeFileExtension(char *str, char *ext)
	{
		/* Changes the File Extension of String *str to *ext */
		int n;
		
		n = strlen(str);
		while (str[n] != '.')
			n--;
		
		n++;
		str[n] = 0;
		strcat(str, ext);
	}
	
	
	void 
	invalidoption(char *s)
	{
		/* Prints the Invalid Option error */
		
		sprintf(errstr, "Invalid Option %s !", s);
		playtzx_Error(errstr);
		//return 1;
		return;
	}
	
	char *GetCheckSum(unsigned char *data, int len)
	{
		/* Calculates a XOR checksum for a block and returns a STRING containing the result*/
		unsigned char c = 0;
		int n;
		
		for (n = 0; n < len - 1; n++)
			c ^= data[n];
		
		if (c == data[len - 1])
			return ("OK");
		else {
			sprintf(pstr, "Wrong, should be %3d ($%02X)", c, c);
			return (pstr);
		}
	}
	
	
	void 
	CopyString(char *dest, unsigned char *sour, int len)
	{
		/* Could just use strpy ... */
		int n;
		
		for (n = 0; n < len; n++)
			dest[n] = sour[n];
		dest[n] = 0;
	}
	
	
	void 
	writeout(char *s)
	{
		/* Simple and not too accurate method of waiting after pages ...*/
		//char k;
		
		if (pages) {
			line++;
		}
		debug_printf(VERBOSE_INFO,s);
	}
	
	
	int 
	MultiLine(char *s, int spaces, char *d)
	{
		/* This will convert a text which has lines separated by a single LF (13 dec)*/
		/* character to the text that can be outputed by MSDOS (LF NL) ... it will also*/
		/* Add a number of spaces to the beginning of each line (EXCEPT the first one -*/
		/* so you can use the Description: Text stuff ;) )*/
		/* NOTE: Some UNIX system like LINUX can cope with just LF char (13), some*/
		/*       other systems will need just CR char (10) ... experiment :)*/
		
		int n = 0;
		int m = 0;
		int i;
		int l = 0;
		
		while (s[n]) {
			if (s[n] == 13) {
				d[m] = 13;
				d[m + 1] = 10;		/* Here is the MS-DOS output for line-end */
				m += 2;
				for (i = 0; i < spaces; i++) {
					d[m] = ' ';
					m++;
				}
				
				l++;
			} else {
				d[m] = s[n];
				m++;
			}
			n++;
			
		}
		
		d[m] = 0;
		
		return (l);
	}
	
	void 
	MakeFixedString(char *s, int i)
	{
		/* This will create a fixed length string from null-terminated one...*/
		
		int n = 0;
		int k = 0;
		
		while (i) {
			if (!s[n])
				k = 1;
			if (k)
				s[n] = ' ';
			n++;
			i--;
		}
		s[n] = 0;
	}
	
	void 
	PlayC64ROMByte(char byte, int finish)
	{
		xorvalue = xortype;
		while (bitcount) {
			if (!endian)
				sb_bit = byte & 0x01;
			else
				sb_bit = byte & 0x80;
			if (sb_bit) {
				if (sb_bit1_f)
					PlayC64SB(sb_bit1_f);
				if (sb_bit1_s)
					PlayC64SB(sb_bit1_s);
				xorvalue ^= sb_bit;
			} else {
				if (sb_bit0_f)
					PlayC64SB(sb_bit0_f);
				if (sb_bit0_s)
					PlayC64SB(sb_bit0_s);
				xorvalue ^= sb_bit;
			}
			if (!endian)
				byte >>= 1;
			else
				byte <<= 1;
			bitcount--;
		}
		if (xortype != 0xFF) {
			if (xorvalue) {
				if (sb_bit1_f)
					PlayC64SB(sb_bit1_f);
				if (sb_bit1_s)
					PlayC64SB(sb_bit1_s);
			} else {
				if (sb_bit0_f)
					PlayC64SB(sb_bit0_f);
				if (sb_bit0_s)
					PlayC64SB(sb_bit0_s);
			}
		}
		if (!finish) {
			if (sb_finishbyte_f)
				PlayC64SB(sb_finishbyte_f);
			if (sb_finishbyte_s)
				PlayC64SB(sb_finishbyte_s);
		} else {
			if (sb_finishdata_f)
				PlayC64SB(sb_finishdata_f);
			if (sb_finishdata_s)
				PlayC64SB(sb_finishdata_s);
		}
	}
	
	void 
	PlayC64TurboByte(char byte)
	{
		int add_num;
		
		add_num = add_bit & 3;
		if (add_num && !(add_bit & 4)) {
			while (add_num) {
				if (add_bit & 8)
					PlayC64SB(sb_bit1);
				else
					PlayC64SB(sb_bit0);
				add_num--;
			}
		}
		while (bitcount) {
			if (!endian)
				sb_bit = byte & 0x01;
			else
				sb_bit = byte & 0x80;
			if (sb_bit)
				PlayC64SB(sb_bit1);
			else
				PlayC64SB(sb_bit0);
			if (!endian)
				byte >>= 1;
			else
				byte <<= 1;
			bitcount--;
		}
		if (add_num && (add_bit & 4)) {
			while (add_num) {
				if (add_bit & 8)
					PlayC64SB(sb_bit1);
				else
					PlayC64SB(sb_bit0);
				add_num--;
			}
		}
	}
	
	int main_playtzx(int argc, char *argv[])
	{
		files=0;
		//debug_printf(VERBOSE_INFO,"ZXTape Utilities - Play TZX , TZX to VOC Converter and TZX Info v0.12c for Linux");
		if (argc < 2) {
			debug_printf(VERBOSE_INFO,"Usage: playtzx [switches] file.tzx [output.voc|output.au]");
			debug_printf(VERBOSE_INFO,"       Switches:  -voc      Create a .VOC file instead of audio output");
			debug_printf(VERBOSE_INFO,"                  -au       Create a .au file instead of audio output");
			debug_printf(VERBOSE_INFO,"                  -freq n   Set sampling frequency to n Hz");
			debug_printf(VERBOSE_INFO,"                  -info     Show extensive Info on TZX file");
			debug_printf(VERBOSE_INFO,"                  -one      Show One line of Info per block (condensed form)");
			debug_printf(VERBOSE_INFO,"                  -x        eXpand the Groups in one line mode");
			debug_printf(VERBOSE_INFO,"                  -b    n   Start replay/conversion  at block n");
			debug_printf(VERBOSE_INFO,"                  -e    n   End replay/conversion after block n");
			debug_printf(VERBOSE_INFO,"                  -p        Wait after each page of Info");
			debug_printf(VERBOSE_INFO,"                  -128      Work in 128k mode");
			exit(0);
		}
		/* Check for command line options */
		/* I should change this to use getopt-things */
		for (n = 1; n < argc; n++) {
			if (argv[n][0] == '-') {
				switch (argv[n][1]) {
					case 'a':
						if (strcmp(argv[n], "-au"))
							invalidoption(argv[n]);
						
						au = 1;
						break;
						
					case 'v':
						if (strcmp(argv[n], "-voc"))
							invalidoption(argv[n]);
						
						voc = 1;
						freq = 30303;
						break;
						
					case 'i':
						if (strcmp(argv[n], "-info"))
							invalidoption(argv[n]);
						
						tzx_info = 1;
						break;
						
					case 'o':
						if (strcmp(argv[n], "-one"))
							invalidoption(argv[n]);
						
						tzx_info = 2;
						break;
						
					case 'p':
						if (strcmp(argv[n], "-p"))
							invalidoption(argv[n]);
						
						pages = 1;
						break;
						
					case 'b':
						if (strcmp(argv[n], "-b"))
							invalidoption(argv[n]);
						
						starting = getnumber(argv[n + 1]);
						n++;
						break;
						
					case 'e':
						if (strcmp(argv[n], "-e"))
							invalidoption(argv[n]);
						
						ending = getnumber(argv[n + 1]);
						n++;
						break;
						
					case 'f':
						if (strcmp(argv[n], "-freq"))
							invalidoption(argv[n]);
						
						nfreq = getnumber(argv[n + 1]);
						n++;
						break;
						
					case 'x':
						if (strcmp(argv[n], "-x"))
							invalidoption(argv[n]);
						
						expand = 1;
						break;
						
					case '1':
						if (strcmp(argv[n], "-128"))
							invalidoption(argv[n]);
						
						mode128 = 1;
						break;
						
					default:
						invalidoption(argv[n]);
				}
			} else {
				files++;
				
				switch (files) {
					case 1:
						strcpy(finp, argv[n]);
						break;
						
					case 2:
						strcpy(fout, argv[n]);
						break;
						
					default:
						playtzx_Error("Too Many files on command line!");
						return 1;
						break;
				}
			}
		}
		
		if (voc && tzx_info) {
			playtzx_Error("Both -voc AND -info (or -one) switches selected !");
			return 1;
		}
		
		if (files == 0) {
			playtzx_Error("No Files specified !");
			return 1;
		}
		
		if (files == 1) {
			if (voc) {
				strcpy(fout, finp);
				ChangeFileExtension(fout, "VOC");
			} else if (au) {
				strcpy(fout, finp);
				ChangeFileExtension(fout, "au");
			}
		}
		if (nfreq)
			freq = nfreq;
		
		/**/
		
		/*if ((fh = open(finp, O_RDONLY)) == -1) {
			playtzx_Error("File not found");
			return 1;
		}
		*/
		
		ptr_archivo_entrada=fopen(finp,"rb");
		if (ptr_archivo_entrada==NULL) {
			playtzx_Error("File not found");
			return 1;
		}
		
		//flen = FileLength(fh);
		flen=get_file_size(finp);
		debug_printf (VERBOSE_DEBUG,"File length: %d",flen);
		
		mem = malloc(flen);
		
		if (mem == NULL) {
			playtzx_Error("Not enough memory to load the file!");
			return 1;
		}
		
		memorig=mem;
		
		/* Start reading file...*/
		//read(fh, mem, 10);
		
		
		/*Leemos todo el archivo de golpe
		 * Esto antes hacia dos read, uno de 10 bytes y luego el restante, pero en windows la segunda operacion
		 * read no leia nada. De hecho segun el manual de read:
		 *
		 *        On  files that support seeking, the read operation commences at the current file offset, 
		 * and the file offset is incremented by the number of
		 *  bytes read.  If the current file offset is at or past the end of file, no bytes are read, and read() returns zero.
		 * 
		 * Pues debe ser que en mingw no soporta seek
		*/
		//int leidos=read(fh, mem, flen);
		int leidos=fread(mem,1,flen,ptr_archivo_entrada);
		debug_printf (VERBOSE_INFO,"Read %d bytes",leidos);
		if (leidos!=flen) {
			debug_printf(VERBOSE_ERR,"Error: %s",strerror(errno));
			return 1;
		}
		
		
		mem[7] = 0;
		
		
		//temp debug
		/*
		int indice;
		for (indice=0;indice<10;indice++) {
			unsigned char caracter=mem[indice];
			printf ("%d: %d ",indice,caracter);
			if (caracter>=32 && caracter<=127) printf ("%c",caracter);
			else printf (".");
			printf ("\n");
		}
		*/
		//fin temp debug
		
		
		if (strcmp((const char *) mem, "ZXTape!")) {
			free(memorig);
			playtzx_Error("File is not in ZXTape format!");
			return 1;
		}
		debug_printf(VERBOSE_INFO,"ZXTape file revision %d.%02d", mem[8], mem[9]);
		
		if (!mem[8]) {
			playtzx_Error("Development versions of ZXTape format are not supported!");
			return 1;
		}
		
		if (mem[8] > MAJREV) {
			debug_printf(VERBOSE_INFO,"-- Warning: Some blocks may not be recognised and used!");
			line += 2;
		}
		if (mem[8] == MAJREV && mem[9] > MINREV) {
			debug_printf(VERBOSE_INFO,"-- Warning: Some of the data might not be properly recognised!");
			line += 2;
		}
		/*int leidos=read(fh, mem, flen - 10);
		debug_printf (VERBOSE_INFO,"Read %d additional bytes",leidos);
		if (leidos==0) {
			debug_printf(VERBOSE_INFO,"Error: %s",strerror(errno));
		}
		*/
		
		
		//temp debug
		
		/*int indice;
		for (indice=0;indice<10;indice++) {
			unsigned char caracter=mem[indice];
			printf ("%d: %d ",indice,caracter);
			if (caracter>=32 && caracter<=127) printf ("%c",caracter);
			else printf (".");
			printf ("\n");
		}
		*/
		
		//fin temp debug
		//Saltamos los primeros 10 bytes de cabecera
		mem +=10;
		
		numblocks = 0;
		pos = 0;
		
		not_rec = 0;
		/* Go through the file and record block starts ... (not necessary, could just go right through it)*/
		while (pos < flen - 10) {
			
			if( numblocks >= blocks_allocated ) {
				
				size_t new_size = (blocks_allocated >= 8 ? 2 * blocks_allocated : 16);
				debug_printf (VERBOSE_DEBUG,"Reallocating memory to %d bytes",new_size * sizeof( int ) );
				
				int *ptr = realloc( block, new_size * sizeof( int ) );
				
				
				if( !ptr ) {
					playtzx_Error( "out of memory allocating space for blocks[]" );
					return 1;
				}
				
				block = ptr;
				blocks_allocated = new_size;
			}
			
			block[numblocks] = pos;
			
			pos++;
			
			debug_printf (VERBOSE_DEBUG,"Tag id 0x%02X on position %d",mem[pos - 1],pos-1);
			
			switch (mem[pos - 1]) {
				case 0x10:
					pos += Get2(&mem[pos + 0x02]) + 0x04;
					break;
					
				case 0x11:
					pos += Get3(&mem[pos + 0x0F]) + 0x12;
					break;
					
				case 0x12:
					pos += 0x04;
					break;
					
				case 0x13:
					pos += (mem[pos + 0x00] * 0x02) + 0x01;
					break;
					
				case 0x14:
					pos += Get3(&mem[pos + 0x07]) + 0x0A;
					break;
					
				case 0x15:
					pos += Get3(&mem[pos + 0x05]) + 0x08;
					break;
					
				case 0x16:
					pos += Get4(&mem[pos + 0x00]) + 0x04;
					break;
					
				case 0x17:
					pos += Get4(&mem[pos + 0x00]) + 0x04;
					break;
					
				case 0x20:
					pos += 0x02;
					break;
					
				case 0x21:
					pos += mem[pos + 0x00] + 0x01;
					break;
					
				case 0x22:
					break;
					
				case 0x23:
					pos += 0x02;
					break;
					
				case 0x24:
					pos += 0x02;
					break;
					
				case 0x25:
					break;
					
				case 0x26:
					pos += Get2(&mem[pos + 0x00]) * 0x02 + 0x02;
					break;
					
				case 0x27:
					break;
					
				case 0x28:
					pos += Get2(&mem[pos + 0x00]) + 0x02;
					break;
					
				case 0x2A:
					pos += 0x04;
					break;
					
				case 0x30:
					pos += mem[pos + 0x00] + 0x01;
					break;
					
				case 0x31:
					pos += mem[pos + 0x01] + 0x02;
					break;
					
				case 0x32:
					pos += Get2(&mem[pos + 0x00]) + 0x02;
					break;
					
				case 0x33:
					pos += (mem[pos + 0x00] * 0x03) + 0x01;
					break;
					
				case 0x34:
					pos += 0x08;
					break;
					
				case 0x35:
					pos += Get4(&mem[pos + 0x10]) + 0x14;
					break;
					
					
				case 0x40:
					pos += Get3(&mem[pos + 0x01]) + 0x04;
					break;
					
					
				case 0x5A:
					pos += 0x09;
					break;
					
					
				default:
					debug_printf (VERBOSE_DEBUG,"Tag id 0x%02X not recognized",mem[pos - 1]);
					pos += Get4(&mem[pos + 0x00]) + 0x04;
					not_rec = 1;
					
			}
			
			numblocks++;
			
		}
		
		debug_printf(VERBOSE_INFO,"Number of Blocks: %d", numblocks);
		
		if (not_rec) {
			debug_printf(VERBOSE_INFO,"-- Warning: Some blocks were *NOT* recognised!");
			line += 2;
		}
		curr = 0;
		
		if (starting > 1) {
			if (starting > numblocks) {
				free(memorig);
				playtzx_Error("Invalid Starting Block");
				return 1;
			}
			curr = starting - 1;
		}
		if (ending > 0) {
			if (ending > numblocks || ending < starting) {
				free(memorig);
				playtzx_Error("Invalid Ending Block");
				return 1;
			}
			numblocks = ending;
		}
		if (!tzx_info) {
			if (voc)
				debug_printf(VERBOSE_INFO,"Creating .VOC file using %d Hz frequency.", freq);
			else
				debug_printf(VERBOSE_INFO,"Starting playback using %d Hz frequency.", freq);
		} 
		
		
		
		if (!tzx_info) {
			if (voc) {
				//InitVOC();
			}
			else if (au)
				InitAU();
			//else
			//  InitSB();
		}
		amp = LOAMP;
		cycle = (double) freq / 3500000.0;	/* This is for the conversion later ... */
		
		if (tzx_info== 2)
			line++;
		
		
		/* Start replay of blocks ...*/
		while (curr < numblocks) {
			if (!tzx_info) {
				if (draw)
					debug_printf(VERBOSE_INFO,"Block %3d:", curr + 1);
			} else {
				if (tzx_info== 2 && draw) {
					if (pages) {
						line++;
					}
					debug_printf(VERBOSE_INFO,"%3d-%5X:", curr + 1, block[curr] + 10);
				}
			}
			
			id = mem[block[curr]];
			data = &mem[block[curr] + 1];
			
			switch (id) {
				/* Standard Loading Data block */
				case 0x10:
					tzx_pause = Get2(&data[0]);
					datalen = Get2(&data[2]);
					data += 4;
					
					if (data[0] == 0x00)
						pilot = 8064;
					else
						pilot = 3220;
					
					sb_pilot = ConvSB(2168);
					sb_sync1 = ConvSB(667);
					sb_sync2 = ConvSB(735);
					sb_bit0 = ConvSB(885);
					
					sb_bit1 = ConvSB(1710);
					
					lastbyte = 8;
					
					if (tzx_info== 1) {
						Identify(datalen, data, 1);
						
						sprintf(pstr, "Block %3d (%5X):  10 - Standard Loading Data - %s", curr + 1,
							block[curr] + 10, tstr);
						writeout(pstr);
						
						sprintf(tstr, "                Length: %5d bytes", datalen);
						writeout(tstr);
						
						sprintf(tstr, "                  Flag: %5d ($%02X)", data[0], data[0]);
						writeout(tstr);
						
						sprintf(tstr, "              CheckSum: %5d ($%02X) - %s",
							data[datalen - 1], data[datalen - 1], GetCheckSum(data, datalen));
						writeout(tstr);
						
						sprintf(tstr, "     Pause after block: %5d milliseconds", tzx_pause);
						line++;
						writeout(tstr);
					}
					break;
					
					/* Custom Loading Data block */
					case 0x11:
						sb_pilot = ConvSB(Get2(&data[0]));
						sb_sync1 = ConvSB(Get2(&data[2]));
						sb_sync2 = ConvSB(Get2(&data[4]));
						sb_bit0 = ConvSB(Get2(&data[6]));
						sb_bit1 = ConvSB(Get2(&data[8]));
						
						speed = (int) ((1710.0 / (double) Get2(&data[8])) * 100.0);
						pilot = Get2(&data[10]);
						lastbyte = (int) data[12];
						tzx_pause = Get2(&data[13]);
						datalen = Get3(&data[15]);
						
						data += 18;
						
						if (tzx_info== 1) {
							Identify(datalen, data, 1);
							
							sprintf(pstr, "Block %3d (%5X):  11 - Custom Loading Data - %s", curr + 1,
								block[curr] + 10, tstr);
							writeout(pstr);
							
							sprintf(tstr, "                Length: %5d bytes", datalen);
							writeout(tstr);
							
							sprintf(tstr, "                  Flag: %5d ($%02X)", data[0], data[0]);
							writeout(tstr);
							
							if (!cpc) {
								sprintf(tstr, "              CheckSum: %5d ($%02X) - %s",
									data[datalen - 1], data[datalen - 1], GetCheckSum(data, datalen));
								writeout(tstr);
							}
							sprintf(tstr, "           Pilot pulse: %5d T-States", Get2(data - 18));
							writeout(tstr);
							
							sprintf(tstr, "          Pilot length: %5d pulses", pilot);
							writeout(tstr);
							
							sprintf(tstr, "      Sync first pulse: %5d T-States", Get2(data - 16));
							writeout(tstr);
							
							sprintf(tstr, "     Sync second pulse: %5d T-States", Get2(data - 14));
							writeout(tstr);
							
							sprintf(tstr, "           Bit-0 pulse: %5d T-States", Get2(data - 12));
							writeout(tstr);
							
							sprintf(tstr, "           Bit-1 pulse: %5d T-States", Get2(data - 10));
							writeout(tstr);
							
							sprintf(tstr, "        Last byte used: %5d bits", lastbyte);
							writeout(tstr);
							
							sprintf(tstr, "     Pause after block: %5d milliseconds", tzx_pause);
							line++;
							writeout(tstr);
						}
						break;
						
						/* Pure Tone */
						case 0x12:
							sb_pilot = ConvSB(Get2(&data[0]));
							pilot = Get2(&data[2]);
							
							if (tzx_info!= 1) {
								if (draw)
									debug_printf(VERBOSE_INFO,"    Pure Tone             Length: %5d", pilot);
								
								if (tzx_info!= 2) {
									while (pilot) {
										PlaySB(amp, sb_pilot);
										ToggleAmp();
										pilot--;
									}
								}
							} else {
								sprintf(tstr, "Block %3d (%5X):  12 - Pure Tone", curr + 1, block[curr] + 10);
								writeout(tstr);
								
								sprintf(tstr, "          Pulse length: %5d T-States", Get2(data));
								writeout(tstr);
								
								sprintf(tstr, "           Tone length: %5d pulses", pilot);
								line++;
								writeout(tstr);
							}
							break;
							
							/* Sequence of Pulses */
							case 0x13:
								pilot = (int) data[0];
								data++;
								
								if (tzx_info!= 1) {
									if (draw)
										debug_printf(VERBOSE_INFO,"    Sequence of Pulses    Length: %5d", pilot);
									
									if (tzx_info!= 2) {
										while (pilot) {
											sb_pulse = ConvSB(Get2(&data[0]));
											
											PlaySB(amp, sb_pulse);
											ToggleAmp();
											pilot--;
											data += 2;
										}
									}
								} else {
									sprintf(tstr, "Block %3d (%5X):  13 - Sequence of Pulses", curr + 1,
										block[curr] + 10);
									writeout(tstr);
									
									sprintf(tstr, "      Number of Pulses: %5d", pilot);
									line++;
									writeout(tstr);
								}
								break;
								
								/* Pure Data */
								case 0x14:
									sb_pilot = pilot = sb_sync1 = sb_sync2 = 0;
									sb_bit0 = ConvSB(Get2(&data[0]));
									sb_bit1 = ConvSB(Get2(&data[2]));
									
									speed = (int) ((1710.0 / (double) Get2(&data[2])) * 100.0);
									
									lastbyte = (int) data[4];
									tzx_pause = Get2(&data[5]);
									datalen = Get3(&data[7]);
									data += 10;
									
									if (tzx_info== 1) {
										sprintf(tstr, "Block %3d (%5X):  14 - Pure Data", curr + 1, block[curr] + 10);
										writeout(tstr);
										
										sprintf(tstr, "                Length: %5d bytes", datalen);
										writeout(tstr);
										
										sprintf(tstr, "                  Flag: %5d ($%02X)", data[0], data[0]);
										writeout(tstr);
										
										sprintf(tstr, "              CheckSum: %5d ($%02X) - %s",
											data[datalen - 1], data[datalen - 1], GetCheckSum(data, datalen));
										writeout(tstr);
										
										sprintf(tstr, "           Bit-0 pulse: %5d T-States", Get2(data - 10));
										writeout(tstr);
										
										sprintf(tstr, "           Bit-1 pulse: %5d T-States", Get2(data - 8));
										writeout(tstr);
										
										sprintf(tstr, "        Last byte used: %5d bits", lastbyte);
										writeout(tstr);
										
										sprintf(tstr, "     Pause after block: %5d milliseconds", tzx_pause);
										line++;
										writeout(tstr);
									}
									break;
									
									/* Direct Recording */
									case 0x15:
										sb_pulse = ConvSB(Get2(&data[0]));
										
										/* For now the BEST way is to use the sample frequency for replay that is */
										/* exactly the SAME as the Original Freq. used when sampling this block ! */
										/* i.e. NO downsampling is handled YET ... use TAPER when you need it !  ;-) */
										if (!sb_pulse)
											sb_pulse = 1;		/* In case sample frequency > 44100 */
											
										tzx_pause = Get2(&data[2]);	/* (Should work for frequencies upto 48000) */
										lastbyte = (int) data[4];
										datalen = Get3(&data[5]);
										
										if (tzx_info!= 1) {
											if (draw) {
												debug_printf
												(VERBOSE_INFO,"    Direct Recording      Length:%6d  Original Freq.: %5d Hz",
												 datalen, (int) (0.5 + (3500000.0 / (double) Get2(&data[0]))));

											}
												
												if (tzx_info!= 2) {
													data = &data[8];
													datapos = 0;
													
													/* Replay Direct Recording block ... */
													while (datalen) {
														if (datalen != 1)
															bitcount = 8;
														else
															bitcount = lastbyte;
														
														databyte = data[datapos];
														
														while (bitcount) {
															if (databyte & 0x80)
																amp = HIAMP;
															else
																amp = LOAMP;
															
															PlaySB(amp, sb_pulse);
															
															databyte <<= 1;
															
															bitcount--;
														}
														
														datalen--;
														datapos++;
													}
													amp = LOAMP;
													
													if (tzx_pause)
														PauseSB(amp, tzx_pause);
												}
										} else {
											sprintf(tstr, "Block %3d (%5X):  15 - Direct Recording", curr + 1,
												block[curr] + 10);
											writeout(tstr);
											
											sprintf(tstr, "                Length:%6d bytes", datalen);
											writeout(tstr);
											
											sprintf(tstr,
												"    Original Frequency: %5d T-States/Sample (%5d Hz)",
												Get2(data), (int) (0.5 + (3500000.0 / (double) Get2(data))));
											writeout(tstr);
											
											sprintf(tstr, "        Last byte used: %5d samples", lastbyte);
											writeout(tstr);
											
											sprintf(tstr, "     Pause after block: %5d milliseconds", tzx_pause);
											line++;
											writeout(tstr);
										}
										break;
										
										/* C64 ROM Type Data Block */
										case 0x16:
											data += 4;
											sb_pilot = Get2(&data[0]);
											pilot = Get2(&data[2]);
											sb_sync1 = Get2(&data[4]);
											sb_sync2 = Get2(&data[6]);
											sb_bit0_f = Get2(&data[8]);
											sb_bit0_s = Get2(&data[10]);
											sb_bit1_f = Get2(&data[12]);
											sb_bit1_s = Get2(&data[14]);
											xortype = (int) (data[16]);
											sb_finishbyte_f = Get2(&data[17]);
											sb_finishbyte_s = Get2(&data[19]);
											sb_finishdata_f = Get2(&data[21]);
											sb_finishdata_s = Get2(&data[23]);
											sb_trailing = Get2(&data[25]);
											trailing = Get2(&data[27]);
											lastbyte = (int) (data[29]);
											endian = data[30];
											tzx_pause = Get2(&data[31]);
											datalen = Get3(&data[33]);
											data += 36;
											IdentifyC64ROM(datalen, data, 1);
											if (tzx_info== 1) {
												sprintf(pstr, "Block %3d (%5X):  16 - C64 ROM Type Data - %s", curr + 1, block[curr] + 10, tstr);
												writeout(pstr);
												sprintf(tstr, "                Length: %5d bytes", datalen);
												writeout(tstr);
												sprintf(tstr, "                  Flag: %5d ($%02X)", data[0], data[0]);
												writeout(tstr);
												if (pilot) {
													sprintf(tstr, "           Pilot pulse: %5d T-States, length: %5d pulses", sb_pilot, pilot);
													writeout(tstr);
												} else {
													sprintf(tstr, "           Pilot pulse:  None");
													writeout(tstr);
												}
												sprintf(tstr, "           Sync pulses: %5d & %5d T-States", sb_sync1, sb_sync2);
												writeout(tstr);
												sprintf(tstr, "          Bit-0 pulses: %5d & %5d T-States", sb_bit0_f, sb_bit0_s);
												writeout(tstr);
												sprintf(tstr, "          Bit-1 pulses: %5d & %5d T-States", sb_bit1_f, sb_bit1_s);
												writeout(tstr);
												sprintf(tstr, "        Last byte used: %5d bits", lastbyte);
												writeout(tstr);
												if (xortype != 0xFF) {
													sprintf(tstr, "         Byte XOR Type: %d XOR all bits", xortype);
													writeout(tstr);
												} else {
													sprintf(tstr, "         Byte XOR Type:  None");
													writeout(tstr);
												}
												sprintf(tstr, "    Finish Byte pulses: %5d & %5d T-States", sb_finishbyte_f, sb_finishbyte_s);
												writeout(tstr);
												sprintf(tstr, "    Finish Data pulses: %5d & %5d T-States", sb_finishdata_f, sb_finishdata_s);
												writeout(tstr);
												if (trailing) {
													sprintf(tstr, "   Trailing Tone pulse: %5d T-States, length: %5d pulses", sb_trailing, trailing);
													writeout(tstr);
												} else {
													sprintf(tstr, "   Trailing Tone pulse:  None");
													writeout(tstr);
												}
												if (endian)
													strcpy(pstr, "MSb");
												else
													strcpy(pstr, "LSb");
												sprintf(tstr, "             Endianess:   %s", pstr);
												writeout(tstr);
												sprintf(tstr, "     Pause after block: %5d milliseconds", tzx_pause);
												line++;
												writeout(tstr);
												
												sprintf(tstr, "     First: %02X , Last: %02X, Len: %d", data[0], data[datalen - 1], datalen);
												line++;
												writeout(tstr);
											}
											break;
											/* C64 Turbo Tape Data Block */
											case 0x17:
												data += 4;
												sb_bit0 = Get2(&data[0]);
												sb_bit1 = Get2(&data[2]);
												add_bit = data[4];
												num_lead_in = Get2(&data[5]);
												lead_in_byte = data[7];
												lastbyte = (int) data[8];
												endian = data[9];
												trailing = Get2(&data[10]);
												sb_trailing = data[12];
												tzx_pause = Get2(&data[13]);
												datalen = Get3(&data[15]);
												data += 18;
												IdentifyC64Turbo(datalen, data, 1);
												if (tzx_info== 1) {
													sprintf(pstr, "Block %3d (%5X):  17 - C64 Turbo Tape Type Data - %s", curr + 1, block[curr] + 10, tstr);
													writeout(pstr);
													sprintf(tstr, "                Length: %5d bytes", datalen);
													writeout(tstr);
													sprintf(tstr, "                  Flag: %5d ($%02X)", data[0], data[0]);
													writeout(tstr);
													if (num_lead_in) {
														sprintf(tstr, "         Lead In Bytes: %5d, Value: %3d ($%02X)", num_lead_in, lead_in_byte, lead_in_byte);
														writeout(tstr);
													} else {
														sprintf(tstr, "         Lead In Bytes:  None");
														writeout(tstr);
													}
													sprintf(tstr, "           Bit-0 pulse: %5d T-States", sb_bit0);
													writeout(tstr);
													sprintf(tstr, "           Bit-1 pulse: %5d T-States", sb_bit1);
													writeout(tstr);
													if (add_bit & 3) {
														if (add_bit & 4)
															strcpy(pstr, "After");
														else
															strcpy(pstr, "Before");
														sprintf(tstr, "       Additional Bits: %5d %s Byte, Value %1d", add_bit & 3, pstr, (add_bit >> 3) & 1);
														writeout(tstr);
													} else {
														sprintf(tstr, "       Additional Bits:  None");
														writeout(tstr);
													}
													sprintf(tstr, "        Last byte used: %5d bits", lastbyte);
													writeout(tstr);
													if (endian)
														strcpy(pstr, "MSb");
													else
														strcpy(pstr, "LSb");
													sprintf(tstr, "             Endianess:   %s", pstr);
													writeout(tstr);
													if (trailing) {
														sprintf(tstr, "        Trailing Bytes: %5d, Value: %3d ($%02X)", trailing, sb_trailing, sb_trailing);
														writeout(tstr);
													} else {
														sprintf(tstr, "        Trailing Bytes:  None");
														writeout(tstr);
													}
													sprintf(tstr, "     Pause after block: %5d milliseconds", tzx_pause);
													line++;
													writeout(tstr);
												}
												break;
												
												/* Pause or Stop the Tape command */
												case 0x20:
													tzx_pause = Get2(&data[0]);
													
													amp = LOAMP;
													
													if (tzx_pause) {
														if (tzx_info!= 1) {
															if (draw) {
																debug_printf(VERBOSE_INFO,"    Pause                 Length: %2.3fs",
																	     ((float) tzx_pause) / 1000.0);
															}
																
																if (tzx_info!= 2) {
																	PauseSB(amp, tzx_pause);
																	amp = LOAMP;
																}
														} else {
															sprintf(tstr, "Block %3d (%5X):  20 - Pause (Silence)", curr + 1,
																block[curr] + 10);
															writeout(tstr);
															
															sprintf(tstr, "              Duration: %5d milliseconds", tzx_pause);
															line++;
															writeout(tstr);
														}
													} else {
														if (tzx_info!= 1) {
															if (!voc) {
																if (tzx_info!= 2) {
																	//if (draw)
																	//  debug_printf(VERBOSE_INFO,"    Stop the tape command - Press any key to continue!");
																	
																	PlaySB(amp, sbbuflen << 1);	/* finish last block ... */
																	
																	//StopSB();
																	
																	
																	//InitSB();
																} 
																else {
																	if (draw) debug_printf(VERBOSE_INFO,"    Stop the tape command!");
																}
															} 
															else {
																if (draw) debug_printf(VERBOSE_INFO,"    Stop the tape command!");
																
																if (tzx_info!= 2) {
																	PauseSB(amp, 5000);
																	amp = LOAMP;
																}
															}
														} else {
															sprintf(tstr, "Block %3d (%5X):  20 - Stop the Tape Command", curr + 1,
																block[curr] + 10);
															line++;
															writeout(tstr);
														}
													}
													break;
													
													/* Group Start */
													case 0x21:
														CopyString(pstr, &data[1], data[0]);
														
														if (tzx_info!= 1) {
															if (draw)
																debug_printf(VERBOSE_INFO,"    Group: %s", pstr);
														} else {
															sprintf(tstr, "Block %3d (%5X):  21 - Group: %s", curr + 1, block[curr] + 10,
																pstr);
															line++;
															writeout(tstr);
														}
														
														if (!expand)
															draw = 0;
														
														break;
														
														/* Group End */
														case 0x22:
															if (tzx_info!= 1) {
																if (draw)
																	debug_printf(VERBOSE_INFO,"    Group End");
															} else {
																sprintf(tstr, "Block %3d (%5X):  22 - Group End", curr + 1, block[curr] + 10);
																line++;
																writeout(tstr);
															}
															
															draw = 1;
															break;
															
															/* Jump To Relative */
															case 0x23:
																jump = (signed short) (data[0] + data[1] * 256);
																
																if (tzx_info!= 1) {
																	if (draw)
																		debug_printf(VERBOSE_INFO,"    Jump Relative: %d (To Block %d)", jump, curr + jump + 1);
																	
																	if (!tzx_info) {
																		curr += jump;
																		curr--;
																	}
																} else {
																	sprintf(tstr,
																		"Block %3d (%5X):  23 - Jump Relative: %d (To Block %d)",
																		curr + 1, block[curr] + 10, jump, curr + jump + 1);
																	line++;
																	writeout(tstr);
																}
																break;
																
																/* Loop Start */
																case 0x24:
																	loop_start = curr;
																	loop_count = Get2(&data[0]);
																	
																	if (tzx_info!= 1) {
																		if (draw)
																			debug_printf(VERBOSE_INFO,"    Loop Start, Counter: %d", loop_count);
																	} else {
																		sprintf(tstr, "Block %3d (%5X):  24 - Loop Start, Counter: %d", curr + 1,
																			block[curr] + 10, loop_count - 1);
																		line++;
																		writeout(tstr);
																	}
																	break;
																	
																	/* Loop End */
																	case 0x25:
																		if (tzx_info!= 1) {
																			if (tzx_info!= 2) {
																				loop_count--;
																				
																				if (loop_count > 0) {
																					if (draw)
																						debug_printf(VERBOSE_INFO,"    Loop End, Still To Go %d Time(s)!", loop_count);
																					
																					curr = loop_start;
																				} else {
																					if (draw)
																						debug_printf(VERBOSE_INFO,"    Loop End, Finished");
																				}
																			} else {
																				if (draw)
																					debug_printf(VERBOSE_INFO,"    Loop End");
																			}
																		} else {
																			sprintf(tstr, "Block %3d (%5X):  25 - Loop End", curr + 1, block[curr] + 10);
																			line++;
																			writeout(tstr);
																		}
																		break;
																		
																		/* Call Sequence */
																		case 0x26:
																			call_pos = curr;
																			call_num = Get2(&data[0]);
																			call_cur = 0;
																			
																			if (tzx_info== 1) {
																				sprintf(tstr,
																					"Block %3d (%5X):  26 - Call Sequence, Number of Calls : %d", curr + 1,
																					block[curr] + 10, call_num);
																				line++;
																				writeout(tstr);
																			} else {
																				if (tzx_info== 2) {
																					if (draw)
																						debug_printf(VERBOSE_INFO,"    Call Sequence, Number of Calls : %d", call_num);
																				} else {
																					jump = (signed short) (data[2] + data[3] * 256);
																					
																					if (draw) {
																						debug_printf(VERBOSE_INFO,"    Call Sequence, Number of Calls : %d, First: %d (To Block %d)",
																							     call_num, jump, curr + jump + 1);

																					}
																						
																						curr += jump;
																					curr--;
																				}
																			}
																			break;
																			
																			/* Return from Sequence */
																			case 0x27:
																				call_cur++;
																				
																				if (tzx_info== 1) {
																					sprintf(tstr, "Block %3d (%5X):  27 - Return from Call", curr + 1,
																						block[curr] + 10);
																					line++;
																					writeout(tstr);
																				} else {
																					if (tzx_info== 2) {
																						if (draw)
																							debug_printf(VERBOSE_INFO,"    Return from Call");
																					} else {
																						if (call_cur == call_num) {
																							if (draw)
																								debug_printf(VERBOSE_INFO,"    Return from Call, Last Call Finished");
																							
																							curr = call_pos;
																						} else {
																							curr = call_pos;
																							data = &mem[block[curr] + 1];
																							jump = (signed short) (data[call_cur * 2 + 2] + data[call_cur * 2 + 3] * 256);
																							
																							if (draw) {
																								debug_printf
																								(VERBOSE_INFO,"    Return from Call, Calls Left: %d, Next: %d (To Block %d)",
																								 call_num - call_cur, jump, curr + jump + 1);
																							}
																								
																								curr += jump;
																							curr--;
																						}
																					}
																				}
																				break;
																				
																				/* Select Block */
																				case 0x28:
																					num_sel = data[2];
																					
																					if (tzx_info== 2) {
																						if (draw) {
																							sprintf(tstr, "    Select block");
																							MakeFixedString(tstr, 69);
																							strcpy(tstr + 52, " (-info for more)");
																							debug_printf(VERBOSE_INFO,"%s", tstr);
																						}
																					} else {
																						if (tzx_info== 1) {
																							sprintf(tstr, "Block %3d (%5X):  28 - Select Block", curr + 1,
																								block[curr] + 10);
																							writeout(tstr);
																							
																							data += 3;
																							
																							for (n = 0; n < num_sel; n++) {
																								jump = (signed short) (data[0] + data[1] * 256);
																								
																								CopyString(spdstr, &data[3], data[2]);
																								
																								sprintf(tstr, "%5d - Jump: %3d (To Block %4d) : %s", n + 1, jump,
																									curr + jump + 1, spdstr);
																								writeout(tstr);
																								
																								data += 3 + data[2];
																							}
																							
																							//sprintf(tstr, "\n");
																							//writeout(tstr);
																						} else {
																							debug_printf(VERBOSE_INFO,"    Select :");
																							
																							data += 3;
																							
																							for (n = 0; n < num_sel; n++) {
																								jump = (signed short) (data[0] + data[1] * 256);
																								
																								jumparray[n] = jump;
																								
																								CopyString(spdstr, &data[3], data[2]);
																								
																								debug_printf(VERBOSE_INFO,"%5d : %s", n + 1, spdstr);
																								
																								data += 3 + data[2];
																							}
																							
																							//debug_printf(VERBOSE_INFO,">> Press the number!");
																							
																							if (!voc) {
																								PlaySB(amp, sbbuflen << 1);	/* finish last block ... */
																								//StopSB();
																							} else {
																								PauseSB(amp, 5000);
																								amp = LOAMP;
																							}
																							
																							//k = GetCh();
																							debug_printf (VERBOSE_ERR,"TZX tag 0x28 not supported");
																							return 1;
																							
																							if (k == 27) {
																								free(memorig);
																								//close(fh);
																								fclose(ptr_archivo_entrada);
																								playtzx_Error("ESCAPE key pressed!");
																								return 1;
																							}
																							//if (!voc)
																							//InitSB();
																							
																							k -= 48;
																							
																							if (k < 1 || k > num_sel)
																								debug_printf(VERBOSE_INFO,"Illegal Selection... Continuing...");
																							else {
																								curr += jumparray[k - 1];
																								curr--;
																							}
																						}
																					}
																					break;
																					
																					/* Stop the tape if in 48k mode */
																					case 0x2A:
																						if (tzx_info== 1) {
																							sprintf(tstr, "Block %3d (%5X):  2A - Stop the tape if in 48k mode", curr + 1, block[curr] + 10);
																							line++;
																							writeout(tstr);
																							break;
																						}
																						if (tzx_info== 2) {
																							if (draw)
																								debug_printf(VERBOSE_INFO,"    Stop the tape if in 48k mode!");
																							break;
																						}
																						if (mode128) {
																							if (draw)
																								debug_printf(VERBOSE_INFO,"    Stop the tape only in 48k mode!");
																						} else {
																							if (voc || au) {
																								if (draw)
																									debug_printf(VERBOSE_INFO,"    Stop the tape in 48k mode!");
																								PauseSB(amp, 5000);
																								amp = LOAMP;
																							} else {
																								if (draw)
																									debug_printf(VERBOSE_INFO,"    Stop the tape in 48k mode - Press any key to continue!");
																								PlaySB(amp, sbbuflen << 1);	/* finish last block ... */
																								//StopSB();
																								//InitSB();
																							}
																						}
																						break;
																						
																						/* Description */
																						case 0x30:
																							CopyString(pstr, &data[1], data[0]);
																							
																							if (tzx_info!= 1) {
																								if (draw)
																									debug_printf(VERBOSE_INFO,"    Description: %s", pstr);
																							} else {
																								sprintf(tstr, "Block %3d (%5X):  30 - Description: %s", curr + 1,
																									block[curr] + 10, pstr);
																								line++;
																								writeout(tstr);
																							}
																							
																							break;
																							
																							/* Message */
																							case 0x31:
																								CopyString(pstr, &data[2], data[1]);
																								
																								if (tzx_info!= 1) {
																									if (draw)
																										debug_printf(VERBOSE_INFO,"    Message: %s", pstr);
																								}
																								/* Pause in Message block is ignored ... */ 
																								else {
																									line += MultiLine(pstr, 34, spdstr);
																									
																									sprintf(tstr, "Block %3d (%5X):  31 - Message: %s", curr + 1, block[curr] + 10,
																										spdstr);
																									writeout(tstr);
																									
																									sprintf(tstr, "               Duration: %d seconds", data[0]);
																									line++;
																									writeout(tstr);
																								}
																								break;
																								
																								/* Archive Info */
																								case 0x32:
																									if (tzx_info!= 1) {
																										if (draw) {
																											if (data[3] == 0) {
																												CopyString(spdstr, &data[5], data[4]);
																												sprintf(tstr, "    Title: %s", spdstr);
																												MakeFixedString(tstr, 69);
																												strcpy(tstr + 52, " (-info for more)");
																												debug_printf(VERBOSE_INFO,"%s", tstr);
																											} else {
																												sprintf(tstr, "    Archive Info");
																												MakeFixedString(tstr, 69);
																												strcpy(tstr + 52, " (-info for more)");
																												debug_printf(VERBOSE_INFO,"%s", tstr);
																											}
																										}
																									} else {
																										num = data[2];
																										data += 3;
																										
																										sprintf(tstr, "Block %3d (%5X):  32 - Archive Info:", curr + 1, block[curr] + 10);
																										writeout(tstr);
																										
																										while (num) {
																											switch (data[0]) {
																												case 0x00:
																													sprintf(pstr, "         Title:");
																													break;
																													
																												case 0x01:
																													sprintf(pstr, "     Publisher:");
																													break;
																													
																												case 0x02:
																													sprintf(pstr, "     Author(s):");
																													break;
																													
																												case 0x03:
																													sprintf(pstr, "  Release Date:");
																													break;
																													
																												case 0x04:
																													sprintf(pstr, "      Language:");
																													break;
																													
																												case 0x05:
																													sprintf(pstr, "     Game Type:");
																													break;
																													
																												case 0x06:
																													sprintf(pstr, "         Price:");
																													break;
																													
																												case 0x07:
																													sprintf(pstr, "        Loader:");
																													break;
																													
																												case 0x08:
																													sprintf(pstr, "        Origin:");
																													break;
																													
																												default:
																													sprintf(pstr, "      Comments:");
																													break;
																											}
																											
																											CopyString(spdstr, &data[2], data[1]);
																											
																											line += MultiLine(spdstr, 16, tstr);
																											
																											sprintf(spdstr, "%s %s", pstr, tstr);
																											writeout(spdstr);
																											
																											data += data[1] + 2;
																											num--;
																										}
																										
																										//sprintf(tstr, "\n");
																										//writeout(tstr);
																									}
																									break;
																									
																									/* Hardware Info */
																									case 0x33:
																										if (data[1] == 0 && data[2] > 0x14 && data[2] < 0x1a && data[3] == 1)
																											cpc = 1;
																										
																										if (data[1] == 0 && data[2] == 0x09 && data[3] == 1)
																											sam = 1;
																										
																										if (tzx_info!= 1) {
																											if (draw) {
																												if (data[1] != 0 || data[3] != 1) {
																													sprintf(tstr, "    Hardware Type");
																													MakeFixedString(tstr, 69);
																													strcpy(tstr + 52, " (-info for more)");
																													debug_printf(VERBOSE_INFO,"%s", tstr);
																												} else {
																													debug_printf(VERBOSE_INFO,"    This tape is made for %s !", hid[0][data[2]]);
																												}
																											}
																										} else {
																											num = data[0];
																											
																											data += 1;
																											
																											sprintf(tstr, "Block %3d (%5X):  33 - Hardware Info:", curr + 1, block[curr] + 10);
																											writeout(tstr);
																											
																											for (n = 0; n < 4; n++) {
																												prvi = 1;
																												
																												d = (char *) data;
																												
																												for (m = 0; m < num; m++) {
																													if (d[2] == n) {
																														if (prvi) {
																															prvi = 0;
																															
																															switch (n) {
																																case 0:
																																sprintf(pstr, "  Runs on the following hardware:");
																																writeout(pstr);
																																break;
																																
																																case 1:
																																sprintf(pstr, "  Uses the following hardware:");
																																writeout(pstr);
																																break;
																																
																																case 2:
																																sprintf(pstr,
																																"  Runs, but it doesn't use the following hardware:");
																																writeout(pstr);
																																break;
																																
																																case 3:
																																sprintf(pstr, "  Doesn't run on the following hardware:");
																																writeout(pstr);
																																break;
																															}
																														}
																														if (!prvi && last == d[0]) {
																															for (x = 0; x < lastlen; x++)
																																spdstr[x] = ' ';
																															spdstr[x] = 0;
																															//sprintf(pstr, "      %s  %s", spdstr, hid[d[0]][d[1]]);
																															int jj=d[0];
																															int ii=d[1];
																															
																															sprintf(pstr, "      %s  %s", spdstr, hid[jj] [ii] );
																															writeout(pstr);
																														} else {
																															//sprintf(pstr, "      %s: %s", htype[d[0]], hid[d[0]][d[1]]);
																															int jj=d[0];
																															int ii=d[1];
																															sprintf(pstr, "      %s: %s", htype[jj], hid[jj][ii]);
																															writeout(pstr);
																														}
																														int jj=d[0];
																														//lastlen = strlen(htype[ d[0] ] );
																														lastlen = strlen(htype[ jj ] );
																														last = d[0];
																													}
																													d += 3;
																												}
																											}
																											//sprintf(tstr, "\n");
																											//writeout(tstr);
																										}
																										break;
																										
																										/* Emulation info */
																										case 0x34:
																											if (tzx_info!= 1) {
																												if (draw)
																													debug_printf(VERBOSE_INFO,"    Information for emulators.");
																											} else {
																												sprintf(tstr, "Block %3d (%5X):  34 - Emulation Info", curr + 1,
																													block[curr] + 10);
																												line++;
																												writeout(tstr);
																											}
																											break;
																											
																											/* Custom Info */
																											case 0x35:
																												CopyString(pstr, data, 16);
																												
																												if (tzx_info!= 1) {
																													if (draw) {
																														if (strcmp(pstr, "POKEs           "))
																															debug_printf(VERBOSE_INFO,"    Custom Info: %s", pstr);	/* Only Name of Custom
																															info except POKEs is
																															used ... */
																															else {
																																sprintf(tstr, "    Custom Info: %s", pstr);
																																MakeFixedString(tstr, 69);
																																strcpy(tstr + 52, " (-info for more)");
																																debug_printf(VERBOSE_INFO,"%s", tstr);
																															}
																													}
																												} else {
																													sprintf(tstr, "Block %3d (%5X):  35 - Custom Info: %s", curr + 1, block[curr] + 10, pstr);
																													writeout(tstr);
																													if (!strcmp(pstr, "POKEs           ")) {
																														data += 20;
																														if (data[0]) {
																															sprintf(pstr, "  Description:");
																															CopyString(spdstr, &data[1], data[0]);
																															line += MultiLine(spdstr, 15, tstr) + 1;
																															sprintf(spdstr, "%s %s", pstr, tstr);
																															writeout(spdstr);
																														}
																														data += data[0] + 1;
																														numt = data[0];
																														data++;
																														sprintf(pstr, "          Trainer Description                       Poke Val Org Page");
																														writeout(pstr);
																														sprintf(pstr, "         -------------------------------------------------------------");
																														writeout(pstr);
																														while (numt) {
																															CopyString(pstr, &data[1], data[0]);
																															data += data[0] + 1;
																															nump = data[0];
																															data++;
																															for (n = 0; n < nump; n++) {
																																sprintf(spdstr, "          %s", pstr);
																																MakeFixedString(spdstr, 48);
																																if (data[0] & 8)
																																strcpy(tstr2, "   -");
																																else
																																sprintf(tstr2, "%4d", data[0] & 7);
																																if (data[0] & 32)
																																strcpy(tstr, "  -");
																																else
																																sprintf(tstr, "%3d", data[4]);
																																if (data[0] & 16)
																																strcpy(tstr3, "  -");
																																else
																																sprintf(tstr3, "%3d", data[3]);
																																if (n > 0)
																																strcpy(tstr4, "+");
																																else
																																strcpy(tstr4, " ");
																																sprintf(pstr, "%s %s %5d %s %s %s", spdstr, tstr4, Get2(&data[1]), tstr3, tstr, tstr2);
																																writeout(pstr);
																																data += 5;
																																pstr[0] = 0;
																															}
																															numt--;
																														}
																													}
																													//sprintf(tstr, "\n");
																													//writeout(tstr);
																												}
																												break;
																												
																												/* Snapshot */
																												case 0x40:
																													if (tzx_info!= 1) {
																														if (draw)
																															debug_printf(VERBOSE_INFO,"    Snapshot               (Not Supported yet)");
																													} else {
																														sprintf(tstr, "Block %3d (%5X):  40 - Snapshot", curr + 1, block[curr] + 10);
																														line++;
																														writeout(tstr);
																														
																														switch (data[0]) {
																															case 0:
																																sprintf(pstr, "Type: Z80");
																																break;
																																
																															case 1:
																																sprintf(pstr, "Type: SNA");
																																break;
																																
																															default:
																																sprintf(pstr, "Unknown Type");
																																break;
																														}
																														sprintf(tstr, "                      %s", pstr);
																														line++;
																														writeout(tstr);
																													}
																													
																													break;
																													
																													/* ZXTape!xx */
																													case 0x5A:
																														if (tzx_info!= 1) {
																															if (draw)
																																debug_printf(VERBOSE_INFO,"    Start of the new tape  (Merged Tapes)");
																														} else {
																															sprintf(tstr, "Block %3d (%5X):  5A - Merget Tapes", curr + 1, block[curr] + 10);
																															line++;
																															writeout(tstr);
																														}
																														break;
																														
																														/* Other (unknown) blocks */
																														default:
																															if (tzx_info!= 1) {
																																if (draw)
																																debug_printf(VERBOSE_INFO,"    Unknown block %02X !", id);
																															} else {
																																sprintf(tstr, "Block %3d (%5X):  %02X Unknown Block ", curr + 1,
																																block[curr] + 10, id);
																																line++;
																																writeout(tstr);
																															}
																															break;
			}
			
			if (tzx_info!= 1 && (id == 0x10 || id == 0x11 || id == 0x14)) {	/* One of the data
				blocks ... */
				if (id != 0x14)
					Identify(datalen, data, 0);
				else
					strcpy(tstr, "    Pure Data           ");
				
				if (id == 0x10)
					sprintf(spdstr, "Normal Speed");
				else
					sprintf(spdstr, " Speed: %3d%%", speed);
				
				if (curr == numblocks - 1)
					//sprintf(pstr, "\0");
					pstr[0]=0;
				
				else
					sprintf(pstr, ",Pause: %2.3fs", ((float) tzx_pause) / 1000.0);
				
				if (draw)
					debug_printf(VERBOSE_INFO,"%s  Length:%6d  %s %s", tstr, datalen, spdstr, pstr);
				
				if (tzx_info!= 2) {
					while (pilot) {
						PlaySB(amp, sb_pilot);
						ToggleAmp();
						pilot--;
					}			/* Play PILOT TONE */
					
					if (sb_sync1) {
						PlaySB(amp, sb_sync1);
						ToggleAmp();
					}			/* Play SYNC PULSES */
					if (sb_sync2) {
						PlaySB(amp, sb_sync2);
						ToggleAmp();
					}
					datapos = 0;
					
					while (datalen) {	/* Play actual DATA */
						if (datalen != 1)
							bitcount = 8;
						else
							bitcount = lastbyte;
						
						databyte = data[datapos];
						
						while (bitcount) {
							if (databyte & 0x80)
								sb_bit = sb_bit1;
							else
								sb_bit = sb_bit0;
							
							PlaySB(amp, sb_bit);
							ToggleAmp();
							
							PlaySB(amp, sb_bit);
							ToggleAmp();
							
							databyte <<= 1;
							
							bitcount--;
						}
						
						datalen--;
						datapos++;
					}
					
					/* If there is pause after block present then make first millisecond
					 *	the oposite */
					/* pulse of last pulse played and the rest in LOAMP ... otherwise
					 *	don't do ANY pause */
					
					
					if (tzx_pause) {
						PauseSB(amp, 1);
						amp = LOAMP;
						if (tzx_pause > 1)
							PauseSB(amp, tzx_pause - 1);
					}
				}
			}
			if (tzx_info!= 1 && id == 0x16) {	/* C64 ROM data block ... */
				IdentifyC64ROM(datalen, data, 0);
				
				if (curr == numblocks - 1)
					//sprintf(pstr, "\0");
					pstr[0]=0;
				else
					sprintf(pstr, ",Pause: %2.3fs", ((float) tzx_pause) / 1000.0);
				
				if (draw)
					debug_printf(VERBOSE_INFO," %s Length:%6d  %s%s", tstr, datalen, spdstr, pstr);
				if (tzx_info!= 2) {
					sb_pilot = ConvSB(sb_pilot);
					sb_sync1 = ConvSB(sb_sync1);
					sb_sync2 = ConvSB(sb_sync2);
					sb_bit1_f = ConvSB(sb_bit1_f);
					sb_bit1_s = ConvSB(sb_bit1_s);
					sb_bit0_f = ConvSB(sb_bit0_f);
					sb_bit0_s = ConvSB(sb_bit0_s);
					sb_finishbyte_f = ConvSB(sb_finishbyte_f);
					sb_finishbyte_s = ConvSB(sb_finishbyte_s);
					sb_finishdata_f = ConvSB(sb_finishdata_f);
					sb_finishdata_s = ConvSB(sb_finishdata_s);
					sb_trailing = ConvSB(sb_trailing);
					num_lead_in = 0;
					amp = LOAMP;		/* This might be just opposite !!!! */
					while (pilot) {
						PlayC64SB(sb_pilot);
						pilot--;
					}			/* Play PILOT TONE */
					if (sb_sync1)
						PlayC64SB(sb_sync1);	/* Play SYNC PULSES */


					if (sb_sync2)
							PlayC64SB(sb_sync2);
						datapos = 0;
					while (datalen) {	/* Play actual DATA */
						if (datalen != 1) {
							bitcount = 8;
							PlayC64ROMByte(data[datapos], 0);
						} else {
							bitcount = lastbyte;
							PlayC64ROMByte(data[datapos], 1);
						}
						databyte = data[datapos];
						datalen--;
						datapos++;
					}
					while (trailing) {
						PlayC64SB(sb_trailing);
						trailing--;
					}			/* Play TRAILING TONE */
					/* If there is pause after block present then make first millisecond
					 *	the oposite */
					/* pulse of last pulse played and the rest in LOAMP ... otherwise
					 *	don't do ANY tzx_pause */
					
					
					/*            if (tzx_pause) { PauseSB(amp,1); amp=LOAMP; if (tzx_pause>1) PauseSB(amp,tzx_pause-1); }*/
					
					
					if (tzx_pause) {
						PauseSB(amp, tzx_pause / 2);
						ToggleAmp();
						PauseSB(amp, (tzx_pause / 2) + (tzx_pause % 2));
						ToggleAmp();
					}
				}
			}
			if (tzx_info!= 1 && id == 0x17) {	/* C64 Turbo Tape data block ... */
				IdentifyC64Turbo(datalen, data, 0);
				
				if (curr == numblocks - 1)
					//sprintf(pstr, "\0");
					pstr[0]=0;
				else
					sprintf(pstr, ",Pause: %2.3fs", ((float) tzx_pause) / 1000.0);
				
				if (draw)
					debug_printf(VERBOSE_INFO," %s Length:%6d  %s%s", tstr, datalen, spdstr, pstr);
				if (tzx_info!= 2) {
					sb_bit1 = ConvSB(sb_bit1);
					sb_bit0 = ConvSB(sb_bit0);
					amp = LOAMP;		/* This might be just opposite !!!! */
					while (num_lead_in) {	/* Lead In bytes ! */
						bitcount = 8;
						PlayC64TurboByte(lead_in_byte);
						num_lead_in--;
					}
					datapos = 0;
					while (datalen) {	/* Play actual DATA */
						if (datalen != 1)
							bitcount = 8;
						else
							bitcount = lastbyte;
						PlayC64TurboByte(data[datapos]);
						databyte = data[datapos];
						datalen--;
						datapos++;
					}
					while (trailing) {	/* Trailing bytes ! */
						bitcount = 8;
						PlayC64TurboByte((unsigned char) sb_trailing);
						trailing--;
					}
					/* If there is pause after block present then make first millisecond
					 *	the oposite */
					/* pulse of last pulse played and the rest in LOAMP ... otherwise
					 *	don't do ANY tzx_pause */
					
					
					/*            if (tzx_pause) { PauseSB(amp,1); amp=LOAMP; if (tzx_pause>1) PauseSB(amp,tzx_pause-1); }*/
					
					
					if (tzx_pause) {
						PauseSB(amp, tzx_pause / 2);
						ToggleAmp();
						PauseSB(amp, (tzx_pause / 2) + (tzx_pause % 2));
						ToggleAmp();
					}
				}
			}
			curr++;
		}
		
		if (!tzx_info) {
			PauseSB(amp, 1000);		/* Finish the loading with 1s pause ... */
			
			if (voc) {
				//StopVOC();
			}
			else if (au)
				StopAU();
			//else
			//StopSB();
		}
		//flushSBBuffer();		/* play the final bits in the audio buffer ?? */
		/**/
		free(memorig);
		
		//close(fh);
		fclose(ptr_archivo_entrada);
		
		return 0;
	}
	
