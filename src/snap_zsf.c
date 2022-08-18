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
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cpu.h"
#include "debug.h"
#include "operaciones.h"

#include "snap_zsf.h"

#include "zx8081.h"
#include "mem128.h"
#include "ay38912.h"
#include "compileoptions.h"
#include "tape_smp.h"
#include "audio.h"
#include "screen.h"
#include "zxvision.h"
#include "tape.h"
#include "snap.h"
#include "snap_z81.h"
#include "snap_zx8081.h"
#include "utils.h"
#include "ula.h"
#include "joystick.h"
#include "realjoystick.h"
#include "z88.h"
#include "chardetect.h"
#include "jupiterace.h"
#include "cpc.h"
#include "timex.h"
#include "zxuno.h"
#include "ulaplus.h"
#include "chloe.h"
#include "prism.h"
#include "diviface.h"
#include "snap_rzx.h"
#include "divmmc.h"
#include "divide.h"
#include "zxevo.h"
#include "tsconf.h"
#include "baseconf.h"
#include "tbblue.h"
#include "msx.h"
#include "vdp_9918a.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "sn76489an.h"
#include "svi.h"
#include "m68k.h"
#include "ql_zx8302.h"
#include "ql_i8049.h"
#include "vdp_9918a_sms.h"
#include "settings.h"
#include "scmp.h"
#include "mk14.h"
#include "chrome.h"


#include "autoselectoptions.h"

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif




int zsf_force_uncompressed=0; //Si forzar bloques no comprimidos

/*
Format ZSF:
* All numbers are LSB

Every block is defined with a header:

2 bytes - 16 bit: block ID
4 bytes - 32 bit: block Lenght
After these 6 bytes, the data for the block comes.



-Block ID 0: ZSF_NOOP
No operation. Block lenght 0

-Block ID 1: ZSF_MACHINEID
Defines which machine is this snapshot. Normally it should come after any other block, but can appear later
Even it could be absent, so the snapshot will be loaded according to the current machine

Byte fields:
0: Machine ID. Same ID  defined on function set_machine_params from cpu.c

-Block ID 2: ZSF_Z80_REGS
Z80 CPU Registers

-Block ID 3: ZSF_MOTO_REGS
Motorola CPU Registers

-Block ID 4: ZSF_RAMBLOCK
A ram binary block
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where 
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address
3,4: Block lenght (if 0, means 65536. Value 0 only used on Inves)
5 and next bytes: data bytes



-Block ID 5: ZSF_SPEC128_MEMCONF
Byte Fields:
0: Port 32765 contents
1: Port 8189 contents
2: Total memory multiplier: 1 for 128kb ram, 2 for 256 kb ram, 4 for 512 kb ram

-Block ID 6: ZSF_SPEC128_RAMBLOCK
A ram binary block for a spectrum 128, p2 or p2a machine
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id (0..7) for a spectrum 128k for example
6 and next bytes: data bytes


-Block ID 7: ZSF_AYCHIP
Byte fields:
0: AY Chip number, starting at 0. A normal spectrum will be only the 0. A turbosound, 0 and 1, etc
1: Current AY Chip selected (variable ay_chip_selected). Redundant in all ZSF_AYCHIP blocks
2: AY Last Register selection
3-18: AY Chip contents

-Block ID 8: ZSF_ULA
Byte fields:
0: Border color (Last out to port 254 AND 7)



-Block ID 9: ZSF_ULAPLUS
Byte fields:
0: ULAplus mode
1: ULAplus last out to port BF3B
2: ULAplus last out to port FF3B
3-66: ULAplus palette


-Block ID 10: ZSF_ZXUNO_RAMBLOCK
A ram binary block for a zxuno
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes



-Block ID 11: ZSF_ZXUNO_CONF
Ports and internal registers of ZXUNO machine
Byte fields:
0: Last out to port FC3B
1-256: 256 internal ZXUNO registers
257: Flash SPI bus index
258: Flash SPI next read byte   
259: Flash SPI status register
260-262: 24 bit value with last spi write address
263-265: 24 bit value with last spi read address
266-273: 8 byte with spi bus contents


-Block ID 12: ZSF_ZX8081_CONF
Internal configuration and state of ZX80/81 machine
Byte fields:
0: Ram assigned to ZX80/81 (1..16), not counting ram packs on 2000H, 8000H or C000H 
1: Flags1: Bits:

*7-5: Reserved for future use
*4: if 16k RAM block in C000H 
*3: if 16k RAM block in 8000H 
*2: if 8k RAM block in 2000H 
*1: if hsync generator is active 
*0: if nmi generator is active

2: Flags2: Reserved for future use

-Block ID 13: ZSF_ZXEVO_NVRAM
Contents of nvram for ZX-Evolution machines (Baseconf, TSConf)
Byte fields:
0: NVRam access control byte (last value sent to port eff7)
1: Last NVRam selected register (last value sent to port dff7)
2-257: NVRAM contents

-Block ID 14: ZSF_TSCONF_RAMBLOCK
A ram binary block for a tsconf
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes


-Block ID 15: ZSF_TSCONF_CONF
Ports and internal registers of TSCONF machine
Byte fields:

0:255: tsconf_af_ports[256];
256-1279: tsconf_fmaps

-Block ID 16: ZSF_DIVIFACE_CONF
Divmmc/divide common settings (diviface), in case it's enabled
Byte fields:

0: Memory size: Value of 2=32 kb, 3=64 kb, 4=128 kb, 5=256 kb, 6=512 kb
1: Diviface control register
2: Status bits: 
  Bit 0=If entered automatic divmmc paging. 
  Bit 1=If divmmc interface is enabled
  Bit 2=If divmmc ports are enabled
  Bit 3=If divide interface is enabled
  Bit 4=If divide ports are enabled  
  Bits 5-7: unused by now

-Block ID 17: ZSF_DIVIFACE_MEM
A ram binary block for diviface memory
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1: ram block id 
2 and next bytes: data bytes


-Block ID 18: ZSF_CPC_RAMBLOCK
A ram binary block for a cpc
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes


-Block ID 19: ZSF_CPC_CONF
Ports and internal registers of CPC machine
Byte fields:
0-3: Gate registers
4-19: Palette
20-23: PPI ports
24-55: CRTC registers
56: Border color
57: last crtc selected register

-Block ID 20: ZSF_PENTAGON_CONF
Ports and internal registers of Pentagon machine
Byte fields:
0: Port EFF7


-Block ID 21: ZSF_TBBLUE_RAMBLOCK
A ram binary block for a tbblue. We store all the 2048 MB (memoria_spectrum pointer). Total pages: 128
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id (in blocks of 16kb)
6 and next bytes: data bytes



-Block ID 22: ZSF_TBBLUE_CONF
0: tbblue_last_register
1-256: 256 internal TBBLUE registers
257: tbblue_bootrom_flag
258: tbblue_port_123b
259: Same 3 bytes as ZSF_DIVIFACE_CONF:

  0: Memory size: Value of 2=32 kb, 3=64 kb, 4=128 kb, 5=256 kb, 6=512 kb
  1: Diviface control register
  2: Status bits:
    Bit 0=If entered automatic divmmc paging.
    Bit 1=If divmmc interface is enabled
    Bit 2=If divmmc ports are enabled
    Bit 3=If divide interface is enabled
    Bit 4=If divide ports are enabled
    Bits 5-7: unused by now

262: Word: Copper PC
264: Byte: Copper memory (currently 2048 bytes)
2312:
....


-Block ID 23 ZSF_TBBLUE_PALETTES
Colour palettes of TBBLUE machine
Byte fields:
0   -511 z80_int tbblue_palette_ula_first[256];
512 -1023 z80_int tbblue_palette_ula_second[256];
1024-1535 z80_int tbblue_palette_layer2_first[256];
1536-2047 z80_int tbblue_palette_layer2_second[256];
2048-2559 z80_int tbblue_palette_sprite_first[256];
2560-3071 z80_int tbblue_palette_sprite_second[256];
3072-3583 z80_int tbblue_palette_tilemap_first[256];
3584-4095 z80_int tbblue_palette_tilemap_second[256];


-Block ID 24: ZSF_TBBLUE_SPRITES
0: 16KB with the sprite patterns
16384: z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][TBBLUE_SPRITE_ATTRIBUTE_SIZE];



-Block ID 25: ZSF_TIMEX
Byte fields:
0: timex_port_f4
1: timex_port_ff



-Block ID 26: ZSF_MSX_MEMBLOCK
A ram binary block for a msx
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: slot (0,1,2 or 3)
6: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff)
7: type: 0 rom, 1 ram
8 and next bytes: data bytes


-Block ID 27: ZSF_MSX_CONF
Ports and internal registers of MSX machine
Byte fields:
0: msx_ppi_register_a
1: msx_ppi_register_b
2: msx_ppi_register_c




-Block ID 28: ZSF_VDP_9918A_VRAM
VRAM contents for machine with vdp 9918a chip
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght




-Block ID 29: ZSF_GENERIC_LINEAR_MEM
A ram/rom binary block for a coleco, sg1000, spectravideo,or any machine to save memory linear
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff, ...)


-Block ID 30: ZSF_VDP_9918A_CONF
Ports and internal registers of VDP 9918A registers
Byte fields:
0:      vdp_9918a_registers[16];
16:     vdp_9918a_status_register;

17,18:  vdp_9918a_last_command_status_bytes[2];
19:     vdp_9918a_last_command_status_bytes_counter=0;

20,21,22:   vdp_9918a_last_vram_bytes[3];

23,24:  vdp_9918a_last_vram_position;


-Block ID 31: ZSF_SNCHIP
Byte fields:
0-15: SN Chip contents


-Block ID 32: ZSF_SVI_CONF
Ports and internal registers of SVI machine
Byte fields:
0: svi_ppi_register_a
1: svi_ppi_register_b
2: svi_ppi_register_c

-Block ID 33: ZSF_DATETIME
Date and time of ZSF file
0: day
1: month
2,3: year
4: hour
5: minute


-Block ID 34: ZSF_QL_RAMBLOCK
A ram binary block for a QL, of 16kb
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes

-Block ID 35: ZSF_QL_CONF
Ports and internal registers of QL
Byte fields:

0: unsigned char ql_pc_intr;
1: unsigned char ql_mc_stat;

-Block ID 36: ZSF_SMS_ROMBLOCK
A rom binary block for a SMS
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff, ...)

-Block ID 37: ZSF_SMS_RAMBLOCK
A rom binary block for a SMS
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff, ...)

-Block ID 38: ZSF_SMS_CONF
Ports and internal registers of SMS machine
Byte fields:
0 sms_mapper_type=SMS_MAPPER_TYPE_NONE(0), SMS_MAPPER_TYPE_SEGA(1);
1 sms_mapper_FFFC;
2 sms_mapper_FFFD;
3 sms_mapper_FFFE;
4 sms_mapper_FFFF;
5 flags:
bit 0: Si sms_writing_cram
6: index_sms_escritura_cram

-Block ID 39: ZSF_SMS_CRAM
CRAM colour palette
Byte fields:
0...31

-Block ID 40: ZSF_ACE_CONF 
Byte fields:
0 ((ramtop_ace-16383)/1024)+3;


-Block ID 41: ZSF_Z88_MEMBLOCK
A ram binary block for a z88
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: bank

-Block ID 42: ZSF_Z88_CONF
Byte fields:
0: Z88 Internal ROM size (divided by 16384)
1: Z88 Internal RAM size (divided by 16384)
2: Indicates which types of slots are inserted. If some slot doesn't have card, type has no sense. Value is codificated as:
  Bit Description
  6-7 Unused
  4-5 Slot 3 Memory type
  2-3 Slot 2 Memory type
  0-1 Slot 1 Memory type
  Type can be:

  0: RAM
  1: Hybrid RAM+EPROM 
  2: EPROM
  3: FLASH
  Note: EPROM, Hybrid EPROM+RAM or FLASH on slot 3 are not saved on zsf snapshots, 
  but are indicated here to warn user that it must be inserted manually an eprom or flash

3: Z88 Slot 1 size (divided by 16384)
4: Z88 Slot 2 size (divided by 16384)
5: Z88 Slot 3 size (divided by 16384)
6: low 8 bits of blink_pixel_base[0]
7: high 8 bits of blink_pixel_base[0]

8: low 8 bits of blink_pixel_base[1]
9: high 8 bits of blink_pixel_base[1]

10: low 8 bits of blink_pixel_base[2]
11: high 8 bits of blink_pixel_base[2]

12: low 8 bits of blink_pixel_base[3]
13: high 8 bits of blink_pixel_base[3]

14: low 8 bits of blink_sbr
15: high 8 bits of blink_sbr

16: blink_com
17: blink_int

18: blink_sta
19: blink_epr

20: blink_tmk
21: blink_tsta

22: blink_mapped_memory_banks[0]
23: blink_mapped_memory_banks[1]
24: blink_mapped_memory_banks[2]
25: blink_mapped_memory_banks[3]

26: blink_tim[0]
27: blink_tim[1]
28: blink_tim[2]
29: blink_tim[3]
30: blink_tim[4]

31: blink_rxd
32: blink_rxe

33: blink_rxc
34: blink_txd

35: blink_txc
36: blink_umk

37: blink_uit

-Block ID 43: ZSF_Z80_HALT_STATE
Byte fields:
0: z80_halt_signal.v;
  

-Block ID 44: ZSF_TIMEX_DOCK_ROM
A rom dock for timex ts 2068
Byte Fields:
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5 and next bytes: data bytes


-Block ID 45: ZSF_MK14_REGS_ID
Registers of the MK14
Byte fields:
0,1: PC Register
2,3: P1 Register
4,5: P2 Register
6,7: P3 Register
8: AC Register
9: ER Register
10: SR Register

-Block ID 46: ZSF_MK14_MEMBLOCK
Memory RAM/ROM of the MK14
Byte fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1 and next bytes: Dump of the 64kb

-Block ID 47: ZSF_MK14_LEDS
Leds values
Byte fields
0-7: led values



-Block ID 48: ZSF_CHROME_RAMBLOCK
A ram binary block for a chrome
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes

-Block ID 49: ZSF_PRISM_CONF
Ports and internal registers of TSCONF machine
Byte fields:
0: Last out to port 60987
1-16: prism_ula2_registers[16]
17: prism_ula2_palette_control_colour
18: prism_ula2_palette_control_index
19-21: prism_ula2_palette_control_rgb[3]
22: prism_last_ae3b;
23-278: prism_ae3b_registers[256]
279: 12 bit prism_palette_two[256]
791: end


-Block ID 50: ZSF_PRISM_RAMBLOCK
A ram binary block for a prism. blocks of 8kb size
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes

-Block ID 51: ZSF_PRISM_VRAMBLOCK
A vram binary block for a prism. blocks of 8kb size
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes


-Block ID 52: ZSF_CHLOE_HOME_RAMBLOCK
A ram binary block for a Chloe, Home banks. blocks of 16kb size
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes

-Block ID 53: ZSF_CHLOE_EX_RAMBLOCK
A ram binary block for a Chloe, EX banks. blocks of 8kb size
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes


-Block ID 54: ZSF_CHLOE_DOCK_RAMBLOCK
A ram binary block for a Chloe, DOCK banks. blocks of 8kb size
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes

-Como codificar bloques de memoria para Spectrum 128k, zxuno, tbblue, tsconf, etc?
Con un numero de bloque (0...255) pero... que tamaño de bloque? tbblue usa paginas de 8kb, tsconf usa paginas de 16kb
Quizá numero de bloque y parametro que diga tamaño, para tener un block id comun para todos ellos
->NOTA: esto parece que no lo he tenido en cuenta pues ya estoy usando bloques diferentes para tsconf, zxuno, tbblue etc
Por otra parte, tener bloques diferentes ayuda a saber mejor qué tipos de bloques son en el snapshot

*/

//Maxima longitud de los bloques de descripcion
#define MAX_ZSF_BLOCK_ID_NAMELENGTH 30

//Total de nombres sin contar el unknown final
#define MAX_ZSF_BLOCK_ID_NAMES 54
char *zsf_block_id_names[]={
 //123456789012345678901234567890
  "ZSF_NOOP",
  "ZSF_MACHINEID",
  "ZSF_Z80_REGS",
  "ZSF_MOTO_REGS",
  "ZSF_RAMBLOCK",
  "ZSF_SPEC128_MEMCONF",
  "ZSF_SPEC128_RAMBLOCK",
  "ZSF_AYCHIP",
  "ZSF_ULA",
  "ZSF_ULAPLUS",
  "ZSF_ZXUNO_RAMBLOCK",
  "ZSF_ZXUNO_CONF",
  "ZSF_ZX8081_CONF",
  "ZSF_ZXEVO_NVRAM",
  "ZSF_TSCONF_RAMBLOCK",
  "ZSF_TSCONF_CONF",
  "ZSF_DIVIFACE_CONF",
  "ZSF_DIVIFACE_MEM",
  "ZSF_CPC_RAMBLOCK",
  "ZSF_CPC_CONF",
  "ZSF_PENTAGON_CONF",
  "ZSF_TBBLUE_RAMBLOCK",
  "ZSF_TBBLUE_CONF",
  "ZSF_TBBLUE_PALETTES",
  "ZSF_TBBLUE_SPRITES",
  "ZSF_TIMEX",
  "ZSF_MSX_MEMBLOCK",
  "ZSF_MSX_CONF",
  "ZSF_VDP_9918A_VRAM",
  "ZSF_GENERIC_LINEAR_MEM",
  "ZSF_VDP_9918A_CONF",
  "ZSF_SNCHIP",
  "ZSF_SVI_CONF",
  "ZSF_DATETIME",
  "ZSF_QL_RAMBLOCK",
  "ZSF_QL_CONF",
  "ZSF_SMS_ROMBLOCK",
  "ZSF_SMS_RAMBLOCK",
  "ZSF_SMS_CONF",
  "ZSF_SMS_CRAM",
  "ZSF_ACE_CONF",
  "ZSF_Z88_MEMBLOCK",
  "ZSF_Z88_CONF",
  "ZSF_Z80_HALT_STATE",
  "ZSF_TIMEX_DOCK_ROM",
  "ZSF_MK14_REGS_ID",
  "ZSF_MK14_MEMBLOCK",
  "ZSF_MK14_LEDS",
  "ZSF_CHROME_RAMBLOCK",
  "ZSF_PRISM_CONF",
  "ZSF_PRISM_RAMBLOCK",
  "ZSF_PRISM_VRAMBLOCK",
  "ZSF_CHLOE_HOME_RAMBLOCK",
  "ZSF_CHLOE_EX_RAMBLOCK",
  "ZSF_CHLOE_DOCK_RAMBLOCK",

  "Unknown"  //Este siempre al final
};


char zsf_magic_header[]="ZSF ZEsarUX Snapshot File.";

char *zsf_get_block_id_name(int block_id)
{
  if (block_id>=MAX_ZSF_BLOCK_ID_NAMES) return zsf_block_id_names[MAX_ZSF_BLOCK_ID_NAMES];
  else return zsf_block_id_names[block_id];
}

//Si ptr_zsf_file no es NULL, lo guarda en archivo
//Si ptr_zsf_file es NULL, lo guarda en destination_memory
//El **destination_memory es el puntero al puntero donde esta la memoria destino, se modificara al salir con el siguiente byte del final
int zsf_write_block(FILE *ptr_zsf_file, z80_byte **destination_memory, int *longitud_total, z80_byte *source,z80_int block_id, unsigned int lenght)
{
  z80_byte block_header[6];
  block_header[0]=value_16_to_8l(block_id);
  block_header[1]=value_16_to_8h(block_id);

  block_header[2]=(lenght)      & 0xFF;
  block_header[3]=(lenght>>8)   & 0xFF;
  block_header[4]=(lenght>>16)  & 0xFF;
  block_header[5]=(lenght>>24)  & 0xFF;

  z80_byte *puntero_memoria;
  puntero_memoria=*destination_memory;

  //Write header
  if (ptr_zsf_file!=NULL) {
    fwrite(block_header, 1, 6, ptr_zsf_file);
  }
  else {
    memcpy(puntero_memoria,block_header,6);
    puntero_memoria +=6;
  }

  *longitud_total +=6;



  //Write data block
  if (lenght) {
    if (ptr_zsf_file!=NULL) {
      fwrite(source, 1, lenght, ptr_zsf_file);
    }
    else {
      memcpy(puntero_memoria,source,lenght);
      puntero_memoria +=lenght;      
    }
  }

  *longitud_total +=lenght;

  //Modificar puntero
  if (ptr_zsf_file==NULL) *destination_memory=puntero_memoria;

  return 0;

}

void load_zsf_spec128_memconf(z80_byte *header)
{
/*
-Block ID 5: ZSF_SPEC128_MEMCONF
Byte Fields:
0: Port 32765 contents
1: Port 8189 contents
2: Total memory multiplier: 1 for 128kb ram, 2 for 256 kb ram, 4 for 512 kb ram
*/

	puerto_32765=header[0];
	puerto_8189=header[1];
	mem128_multiplicador=header[2];

//Distinguir entre 128/p2 y p2a
	if (MACHINE_IS_SPECTRUM_128_P2) {
		debug_printf(VERBOSE_DEBUG,"Paging 128k according to port 32765: %02XH",puerto_32765);
		mem_page_ram_128k();
		mem_page_rom_128k();
	}

	if (MACHINE_IS_SPECTRUM_P2A_P3) {
		mem_page_ram_p2a();

		if (puerto_8189&1) mem_page_ram_rom();
		else mem_page_rom_p2a();


		//mem_init_memory_tables_p2a();
/*

p2a
32765:
                        //asignar ram
                        mem_page_ram_p2a();

                        //asignar rom
                        mem_page_rom_p2a();


8189:

ram in rom: mem_page_ram_rom();
mem_page_rom_p2a();
*/
	}

    if (MACHINE_IS_CHROME) {
        chrome_set_memory_pages();
    }

    if (MACHINE_IS_CHLOE) {
        chloe_set_memory_pages();
    }


}



void load_zsf_snapshot_z80_regs(z80_byte *header)
{
  reg_c=header[0];
  reg_b=header[1];
  reg_e=header[2];
  reg_d=header[3];
  reg_l=header[4];
  reg_h=header[5];

        store_flags(header[6]);
        reg_a=header[7];

        reg_ix=value_8_to_16(header[9],header[8]);
        reg_iy=value_8_to_16(header[11],header[10]);

        reg_c_shadow=header[12];
        reg_b_shadow=header[13];
        reg_e_shadow=header[14];
        reg_d_shadow=header[15];
        reg_l_shadow=header[16];
        reg_h_shadow=header[17];

        store_flags_shadow(header[18]);
        reg_a_shadow=header[19];

        reg_r=header[20];
        reg_r_bit7=reg_r&128;

        reg_i=header[21];

        reg_sp=value_8_to_16(header[23],header[22]);

        reg_pc=value_8_to_16(header[25],header[24]);

        im_mode=header[26] & 2;
        if (im_mode==1) im_mode=2;

        iff1.v=iff2.v=header[26] &1;
}


void load_zsf_snapshot_z80_halt_state(z80_byte *header)
{
    z80_halt_signal.v=header[0] & 1;
    //printf("Estado halt al cargar snapshot: %d\n",z80_halt_signal.v);

}

void load_zsf_snapshot_moto_regs(z80_byte *header)
{

    int i=0;

    m68k_set_reg(M68K_REG_PC,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_SP,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_USP,util_read_long_value(&header[i])); i+=4;
    m68k_set_reg(M68K_REG_SR,util_read_long_value(&header[i]));  i+=4;

    m68k_set_reg(M68K_REG_A0,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_A1,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_A2,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_A3,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_A4,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_A5,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_A6,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_A7,util_read_long_value(&header[i]));  i+=4;

    m68k_set_reg(M68K_REG_D0,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_D1,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_D2,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_D3,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_D4,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_D5,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_D6,util_read_long_value(&header[i]));  i+=4;
    m68k_set_reg(M68K_REG_D7,util_read_long_value(&header[i]));  i+=4;


}


void load_zsf_snapshot_mk14_regs(z80_byte *header)
{

    scmp_m_PC.w.l=value_8_to_16(header[1],header[0]);
    scmp_m_P1.w.l=value_8_to_16(header[3],header[2]);
    scmp_m_P2.w.l=value_8_to_16(header[5],header[4]);
    scmp_m_P3.w.l=value_8_to_16(header[7],header[6]);

    scmp_m_AC=header[8];
    scmp_m_ER=header[9];
    scmp_m_SR=header[10];

}

void load_zsf_snapshot_mk14_leds(z80_byte *header)
{

    int i;

    for (i=0;i<8;i++) {
        mk14_ledstat[i]=header[i];
    }


}


/*
Cargar bloque de datos en destino indicado
block_length: usado en bloques no comprimidos (lo que dice la cabecera que ocupa)
longitud_original: usado en bloques comprimidos (lo que ocupan los bloques comprimidos)
*/
void load_zsf_snapshot_block_data_addr(z80_byte *block_data,z80_byte *destino,int block_lenght, int longitud_original,int si_comprimido)
{
  
  //printf ("load_zsf_snapshot_block_data_addr block_lenght: %d longitud_original: %d si_comprimido: %d\n",block_lenght,longitud_original,si_comprimido);

  
  if (si_comprimido) {
    //Comprimido
    int longitud_descomprimido=util_uncompress_data_repetitions(block_data,destino,longitud_original,0xDD);

    int longitud_bloque=block_lenght;

    //excepcion
    if (longitud_bloque==0) longitud_bloque=65536;

    if (longitud_descomprimido!=longitud_bloque) {
        debug_printf(VERBOSE_ERR,"Length uncompressed (%d) is not expected length (%d). It should not happen!",longitud_descomprimido,block_lenght);
    }


  }


  else {
    int i=0;
    while (block_lenght) {
      *destino=block_data[i++];
      destino++;
      block_lenght--;
    }
  }
}

void load_zsf_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  if (block_lenght==0) block_lenght=65536;

  debug_printf (VERBOSE_DEBUG,"Block start: %d Length: %d Compressed: %s Length_source: %d",block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);
  //printf ("Block start: %d Length: %d Compressed: %d Length_source: %d\n",block_start,block_lenght,block_flags&1,longitud_original);


  longitud_original -=5;


  load_zsf_snapshot_block_data_addr(&block_data[i],&memoria_spectrum[block_start],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_snapshot_mk14_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 1 bytes

  i++;
  z80_int block_start=0;

  int block_lenght=65536;

  debug_printf (VERBOSE_DEBUG,"Block start: %d Length: %d Compressed: %s Length_source: %d",block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);
  //printf ("Block start: %d Length: %d Compressed: %d Length_source: %d\n",block_start,block_lenght,block_flags&1,longitud_original);


  longitud_original -=1;


  load_zsf_snapshot_block_data_addr(&block_data[i],&memoria_spectrum[block_start],block_lenght,longitud_original,block_flags&1);

}


void load_zsf_spec128_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],ram_mem_table[ram_page],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_zxuno_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],zxuno_sram_mem_table_new[ram_page],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_prism_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],prism_ram_mem_table[ram_page],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_prism_snapshot_block_data_vram(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block vram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],prism_vram_mem_table[ram_page],block_lenght,longitud_original,block_flags&1);

}


void load_zsf_chloe_snapshot_block_data_home(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],chloe_home_ram_mem_table[ram_page],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_chloe_snapshot_block_data_ex(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],chloe_ex_ram_mem_table[ram_page],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_chloe_snapshot_block_data_dock(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],chloe_dock_ram_mem_table[ram_page],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_chrome_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],chrome_ram_mem_table[ram_page],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_timex_dockrom_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;


  debug_printf (VERBOSE_DEBUG,"Block rom_dock start: %d Length: %d Compressed: %s Length_source: %d",block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=5;


  load_zsf_snapshot_block_data_addr(&block_data[i],timex_dock_rom_mem_table[0],block_lenght,longitud_original,block_flags&1);

  timex_cartridge_inserted.v=1;

}

void load_zsf_z88_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte bank=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block bank: %d start: %d Length: %d Compressed: %s Length_source: %d",bank,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],&memoria_spectrum[bank*16384],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_msx_snapshot_block_data(z80_byte *block_data,int longitud_original)
{
/*
-Block ID 26: ZSF_MSX_MEMBLOCK
A ram binary block for a msx
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: slot (0,1,2 or 3)
6: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff)
7: type: 0 rom, 1 ram
8 and next bytes: data bytes
*/


  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 8 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte slot=block_data[i];
  i++;

  z80_byte segment=block_data[i];
  i++;

  z80_byte mem_type=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block slot: %d segment: %d start: %d Length: %d Compressed: %s Length_source: %d",slot,segment,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=8;

  //if (ram_page>1) cpu_panic("Loading more than 32kb ram not implemented yet");


  msx_memory_slots[slot][segment]=mem_type;

  int offset=(slot*4+segment)*16384;


  load_zsf_snapshot_block_data_addr(&block_data[i],&memoria_spectrum[offset],block_lenght,longitud_original,block_flags&1);

}



void load_ZSF_GENERIC_LINEAR_MEM_snapshot_block_data(z80_byte *block_data,int longitud_original)
{
/*
-Block ID 29: ZSF_GENERIC_LINEAR_MEM
A ram/rom binary block for a coleco, sg1000, spectravideo,or any machine to save memory linear
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff, ....)
*/


  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 8 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;


  z80_byte segment=block_data[i];
  i++;


  debug_printf (VERBOSE_DEBUG,"Block segment: %d start: %d Length: %d Compressed: %s Length_source: %d",segment,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;

  //if (ram_page>1) cpu_panic("Loading more than 32kb ram not implemented yet");



  int offset=segment*16384;


  load_zsf_snapshot_block_data_addr(&block_data[i],&memoria_spectrum[offset],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_sms_romblock_snapshot_block_data(z80_byte *block_data,int longitud_original)
{
/*

0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff, ....)
*/


  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 8 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;


  z80_byte segment=block_data[i];
  i++;


  debug_printf (VERBOSE_DEBUG,"Block segment: %d start: %d Length: %d Compressed: %s Length_source: %d",segment,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;

  //if (ram_page>1) cpu_panic("Loading more than 32kb ram not implemented yet");



  int offset=segment*16384;


  load_zsf_snapshot_block_data_addr(&block_data[i],&memoria_spectrum[offset],block_lenght,longitud_original,block_flags&1);

  //Los bloques vendran ordenados, el tamaño del cartucho insertado sera el ultimo bloque * 16384
  sms_cartridge_size=(segment+1)*16384;

  sms_cartridge_inserted.v=1;

    //El tipo de mapper lo guardamos en bloque ZSF_SMS_CONF
    //sms_set_mapper_type_from_size();

    sms_set_mapper_mask_bits();  

}


void load_zsf_sms_ramblock_snapshot_block_data(z80_byte *block_data,int longitud_original)
{
/*

0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff, ....)
*/


  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 8 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;


  z80_byte segment=block_data[i];
  i++;


  debug_printf (VERBOSE_DEBUG,"Block segment: %d start: %d Length: %d Compressed: %s Length_source: %d",segment,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;

  //if (ram_page>1) cpu_panic("Loading more than 32kb ram not implemented yet");


  //int offset=segment*16384;

  //return &memoria_spectrum[SMS_MAX_ROM_SIZE + (direccion & 8191)];


  load_zsf_snapshot_block_data_addr(&block_data[i],&memoria_spectrum[SMS_MAX_ROM_SIZE],block_lenght,longitud_original,block_flags&1);



}


void load_zsf_msx_snapshot_vram_data(z80_byte *block_data,int longitud_original)
{
/*
VRAM contents for machine with vdp 9918a chip
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
*/


  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 8 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;


  debug_printf (VERBOSE_DEBUG,"VRAM start: %d Length: %d Compressed: %s Length_source: %d",block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=5;

  //if (ram_page>1) cpu_panic("Loading more than 32kb ram not implemented yet");

  z80_byte *vram_destination;

  if (MACHINE_IS_COLECO) vram_destination=coleco_vram_memory;
  else if (MACHINE_IS_SG1000) vram_destination=sg1000_vram_memory;
  else if (MACHINE_IS_SMS) vram_destination=sms_vram_memory;
  else if (MACHINE_IS_SVI) vram_destination=svi_vram_memory;
  else vram_destination=msx_vram_memory;



  load_zsf_snapshot_block_data_addr(&block_data[i],vram_destination,block_lenght,longitud_original,block_flags&1);

}


void load_zsf_tbblue_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;

  int offset_memoria=16384*ram_page;


  load_zsf_snapshot_block_data_addr(&block_data[i],&memoria_spectrum[offset_memoria],block_lenght,longitud_original,block_flags&1);


  //Y ajustar memoria total
  //Esto se hace estableciendo el numero de bloques segun la direccion donde se carga el ultimo bloque
  //Dado que se cargan los bloques ordenados, al final se tendrá el valor mas alto siempre para la memoria total
  //Si se cargasen en otro orden, esto fallaria
  tbblue_set_ram_blocks(offset_memoria/1024);

}

void load_zsf_cpc_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],cpc_ram_mem_table[ram_page],block_lenght,longitud_original,block_flags&1);

}


void load_zsf_diviface_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];


  i++;

  z80_byte ram_page=block_data[i];
  i++;

  z80_int block_lenght=16384;

  debug_printf (VERBOSE_DEBUG,"Block diviface ram_page: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=3;

    z80_byte *puntero_origen;
    puntero_origen=&diviface_memory_pointer[16384*ram_page];


  load_zsf_snapshot_block_data_addr(&block_data[i],puntero_origen,block_lenght,longitud_original,block_flags&1);

}


void load_zsf_tsconf_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],tsconf_ram_mem_table[ram_page],block_lenght,longitud_original,block_flags&1);

}


void load_zsf_ql_snapshot_block_data(z80_byte *block_data,int longitud_original)
{



  int i=0;
  z80_byte block_flags=block_data[i];

  //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

  i++;
  z80_int block_start=value_8_to_16(block_data[i+1],block_data[i]);
  i +=2;
  z80_int block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
  i+=2;

  z80_byte ram_page=block_data[i];
  i++;

  debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


  longitud_original -=6;


  load_zsf_snapshot_block_data_addr(&block_data[i],&memoria_ql[0x20000+ram_page*16384],block_lenght,longitud_original,block_flags&1);

}

void load_zsf_aychip(z80_byte *header)
{

  
  ay_chip_present.v=1;

  z80_byte header_aychip_number=header[0];
  z80_byte header_aychip_selected=header[1];

  //MAX_AY_CHIPS
  debug_printf(VERBOSE_DEBUG,"Loading AY Chip number %d contents",header_aychip_number);

  if (header_aychip_number>MAX_AY_CHIPS-1 || header_aychip_selected>MAX_AY_CHIPS-1) {
    debug_printf(VERBOSE_ERR,"Snapshot uses more ay chips than we have (%d), ignoring this ZSF_AYCHIP block",MAX_AY_CHIPS);
    return;
  }

  ay_chip_selected=header_aychip_selected;

  //Si el numero de chip a cargar (0..) es mayor que el numero actual de chips (-1)
  if (header_aychip_number>total_ay_chips-1) {
    total_ay_chips=header_aychip_number+1;
    debug_printf(VERBOSE_DEBUG,"Increasing total ay chips to %d",total_ay_chips);
  }

  ay_3_8912_registro_sel[header_aychip_number]=header[2];


      int j;
      for (j=0;j<16;j++) ay_3_8912_registros[header_aychip_number][j]=header[3+j];


  ay_establece_frecuencias_todos_canales();
  

/*
      
-Block ID 7: ZSF_AYCHIP
Byte fields:
0: AY Chip number, starting at 0. A normal spectrum will be only the 0. A turbosound, 0 and 1, etc
1: Current AY Chip selected (variable ay_chip_selected). Redundant in all ZSF_AYCHIP blocks
2: AY Last Register selection
3-18: AY Chip contents
      */
  /*

*/

}


void load_zsf_snchip(z80_byte *header)
{

  
  sn_chip_present.v=1;


      int j;
      for (j=0;j<16;j++) sn_chip_registers[j]=header[j];
  

/*
      
-Block ID 31: ZSF_SNCHIP
Byte fields:
0-15: AY Chip contents
      */
  /*

*/


  sn_establece_frecuencias_todos_canales();

}

void load_zsf_ula(z80_byte *header)
{
  out_254=header[0] & 7;
  out_254_original_value=out_254;

  //printf ("border: %d\n",out_254);
  modificado_border.v=1;

}


void load_zsf_timex(z80_byte *header)
{
  timex_port_f4=header[0];
  timex_port_ff=header[1];

  if (MACHINE_IS_TIMEX_TS_TC_2068) timex_set_memory_pages();
}



void load_zsf_ulaplus(z80_byte *header)
{
/*
-Block ID 9: ZSF_ULAPLUS
Byte fields:
0: ULAplus mode
1: ULAplus last out to port BF3B
2: ULAplus last out to port FF3B
3-66: ULAplus palette
*/

	ulaplus_presente.v=1;
                        
	ulaplus_mode=header[0];
	if (ulaplus_mode) ulaplus_enabled.v=1;
	else ulaplus_enabled.v=0;

	debug_printf (VERBOSE_DEBUG,"Setting ULAplus mode %d",ulaplus_mode);


        ulaplus_last_send_BF3B=header[1];
        ulaplus_last_send_FF3B=header[2];


        //Leer 64 bytes de paleta ulaplus
        int i;
        for (i=0;i<64;i++) ulaplus_palette_table[i]=header[3+i];
}


//NOTA: esta funcion se usa tanto en bloque ZSF_DIVIFACE_CONF como cargado desde dentro de ZSF_TBBLUE_CONF
void load_zsf_diviface_conf(z80_byte *header)
{
/*
Block ID 16: ZSF_DIVIFACE_CONF
Divmmc/divide common settings (diviface), in case it's enabled
Byte fields:

0: Memory size: Value of 2=32 kb, 3=64 kb, 4=128 kb, 5=256 kb, 6=512 kb
1: Diviface control register
2: Status bits: 
  Bit 0=If entered automatic divmmc paging. 
  Bit 1=If divmmc interface is enabled
  Bit 2=If divmmc ports are enabled
  Bit 3=If divide interface is enabled
  Bit 4=If divide ports are enabled  
  Bits 5-7: unused by now
*/

  //Resetear settings divide/divmmc. Ya los habilitara luego si conviene
  divmmc_diviface_enabled.v=0;
  divide_diviface_enabled.v=0;
  diviface_enabled.v=0;

  diviface_current_ram_memory_bits=header[0];

  //Activar dispositivos segun bits
  //Si divmmc
  if (header[2] & 2) {
    divmmc_diviface_enable();
  }

  //O divide
  if (header[2] & 8) {
    divide_diviface_disable();
  }

  //Y settings de puertos
  if (header[2] & 4) {
    divmmc_mmc_ports_enable();
  }
  else {
    divmmc_mmc_ports_disable();
  }


  if (header[2] & 16) {
    divide_ide_ports_enable();
  }
  else {
    divide_ide_ports_disable();
  }

  //Control register al final, y tambien el automatic paging
  diviface_control_register=header[1];
  diviface_paginacion_automatica_activa.v=header[2]&1;

}


void load_zsf_zx8081_conf(z80_byte *header)
{
/*
0: Ram assigned to ZX80/81 (1..16), not counting ram packs on 2000H, 8000H or C000H 
1: Flags1: Bits:

*7-5: Reserved for future use
*4: if 16k RAM block in C000H 
*3: if 16k RAM block in 8000H 
*2: if 8k RAM block in 2000H 
*1: if hsync generator is active 
*0: if nmi generator is active

2: Flags2: Reserved for future use

*/

  z80_int zx8081ram=header[0];
  set_zx8081_ramtop(zx8081ram);

  ram_in_49152.v=(header[1]>>4)&1;
  ram_in_32768.v=(header[1]>>3)&1;
  ram_in_8192.v=(header[1]>>2)&1;
  hsync_generator_active.v=(header[1]>>1)&1;
  nmi_generator_active.v=header[1]&1;


}

void load_zsf_zxevo_nvram(z80_byte *header)
{

  int i;

  //Control de acceso a celdas nvram
  puerto_eff7=header[0];

  //celda nvram seleccionada
  zxevo_last_port_dff7=header[1];

  for (i=0;i<256;i++) zxevo_nvram[i]=header[i+2];

}

void load_zsf_pentagon_conf(z80_byte *header)
{

  

  //Puerto pentagon interno
  puerto_eff7=header[0];



}


void load_zsf_zxuno_conf(z80_byte *header)
{
/*
-Block ID 11: ZSF_ZXUNO_CONF
Ports and internal registers of ZXUNO machine
Byte fields:
0: Last out to port FC3B
1-256: 256 internal ZXUNO registers
257: Flash SPI bus index
258: Flash SPI next read byte   
259: Flash SPI status register
260-262: 24 bit value with last spi write address
263-265: 24 bit value with last spi read address
266-273: 8 byte with spi bus contents
*/

  last_port_FC3B=header[0];
  int i;
  for (i=0;i<256;i++) zxuno_ports[i]=header[1+i];

  zxuno_spi_bus_index=header[257];
  next_spi_read_byte=header[258];
  zxuno_spi_status_register=header[259];


  last_spi_write_address=(header[260]) + (256 * header[261]) + (65536 * header[262]);
  last_spi_read_address=(header[263]) + (256 * header[264]) + (65536 * header[265]);

  for (i=0;i<8;i++) zxuno_spi_bus[i]=header[266+i];

  zxuno_set_memory_pages();    




  //Sincronizar settings de emulador con los valores de puertos de zxuno
  zxuno_set_emulador_settings();



  ulaplus_set_extended_mode(zxuno_ports[0x40]);
}


void load_zsf_prism_conf(z80_byte *header)
{


/*
-Block ID 49: ZSF_PRISM_CONF
Ports and internal registers of TSCONF machine
Byte fields:
0: Last out to port 60987
1-16: prism_ula2_registers[16]
17: prism_ula2_palette_control_colour
18: prism_ula2_palette_control_index
19-21: prism_ula2_palette_control_rgb[3]
22: prism_last_ae3b;
23-278: prism_ae3b_registers[256]
279: 12 bit prism_palette_two[256]
791: end
*/

    prism_rom_page=header[0];

    int i;
    for (i=0;i<16;i++) prism_ula2_registers[i]=header[1+i];

    prism_ula2_palette_control_colour=header[17];
    prism_ula2_palette_control_index=header[18];

    prism_ula2_palette_control_rgb[0]=header[19];
    prism_ula2_palette_control_rgb[1]=header[20];
    prism_ula2_palette_control_rgb[2]=header[21];

    prism_last_ae3b=header[22];

    for (i=0;i<256;i++) prism_ae3b_registers[i]=header[23+i];

    for (i=0;i<256;i++) {
        z80_int valor;
        valor=value_8_to_16(header[279+i*2+1],header[279+i*2]);
        prism_palette_two[i]=valor;
    }

    //Importante: En el snapshot se ha guardado antes que este bloque uno de ZSF_SPEC128_MEMCONF, que contiene
    //informacion de puertos 32765 y 8189, necesarios para el prism_set_memory_pages
    prism_set_memory_pages();

}




void load_zsf_z88_conf(z80_byte *header)
{


    z88_internal_rom_size=(header[0]*16384)-1;
    debug_printf(VERBOSE_DEBUG,"Setting Z88 Internal ROM Size to %d",z88_internal_rom_size+1);
    z88_internal_ram_size=(header[1]*16384)-1;
    debug_printf(VERBOSE_DEBUG,"Setting Z88 Internal RAM Size to %d",z88_internal_ram_size+1);




    int slot_leido=1;
    z88_memory_slots[slot_leido].type=header[2] & 3;
    //si hay alguna tarjeta de memoria de tipo 1 es hibrida ram+eprom
    if (z88_memory_slots[slot_leido].type==1) z88_memory_slots[slot_leido].type=4;

    debug_printf(VERBOSE_DEBUG,"Setting Z88 Slot 1 Memory type to: %s",z88_memory_types[z88_memory_slots[1].type]);
    slot_leido++;

    z88_memory_slots[slot_leido].type=(header[2]>>2) & 3;
    //si hay alguna tarjeta de memoria de tipo 1 es hibrida ram+eprom
    if (z88_memory_slots[slot_leido].type==1) z88_memory_slots[slot_leido].type=4;

    debug_printf(VERBOSE_DEBUG,"Setting Z88 Slot 2 Memory type to: %s",z88_memory_types[z88_memory_slots[2].type]);
    slot_leido++;

    z88_memory_slots[slot_leido].type=(header[2]>>4) & 3;
    //si hay alguna tarjeta de memoria de tipo 1 es hibrida ram+eprom
    if (z88_memory_slots[slot_leido].type==1) z88_memory_slots[slot_leido].type=4;

    debug_printf(VERBOSE_DEBUG,"Setting Z88 Slot 3 Memory type to: %s",z88_memory_types[z88_memory_slots[3].type]);


    if (header[3]) z88_memory_slots[1].size=(header[3]*16384)-1;
    else z88_memory_slots[1].size=0;

    if (header[4]) z88_memory_slots[2].size=(header[4]*16384)-1;
    else z88_memory_slots[2].size=0;

    if (header[5]) z88_memory_slots[3].size=(header[5]*16384)-1;
    else z88_memory_slots[3].size=0;

    //Si hay EPROM o Flash en slot 3, cambiarlo a RAM y size 0

    int avisarerror=0;
    int i=3;
        if (z88_memory_slots[i].type==2 || z88_memory_slots[i].type==3 || z88_memory_slots[i].type==4) {
            if (z88_memory_slots[i].size!=0) {

                z88_memory_slots[i].size=0;
                avisarerror=1;
            }

            z88_memory_slots[i].type=0;
        }

    if (avisarerror) {
        debug_printf (VERBOSE_ERR,"Snapshot had an EPROM or Flash card on Slot 3. It is NOT loaded. You must insert it manually");
    }


    //Mostrar en debug tamanyo slots
    z80_long_int size;
    int sl;
    for (sl=1;sl<=3;sl++) {
        size=z88_memory_slots[sl].size;
        debug_printf(VERBOSE_DEBUG,"Setting Z88 Slot %d Size to: %d",sl,(size ? size +1 : 0));

    }


    //Leer registros del blink


    blink_pixel_base[0]=value_8_to_16(header[7],header[6]);
    blink_pixel_base[1]=value_8_to_16(header[9],header[8]);
    blink_pixel_base[2]=value_8_to_16(header[11],header[10]);
    blink_pixel_base[3]=value_8_to_16(header[13],header[12]);

    blink_sbr=value_8_to_16(header[15],header[14]);


    blink_com=header[16];
    blink_int=header[17];

    blink_sta=header[18];
    blink_epr=header[19];

    blink_tmk=header[20];
    blink_tsta=header[21];

    blink_mapped_memory_banks[0]=header[22];
    blink_mapped_memory_banks[1]=header[23];
    blink_mapped_memory_banks[2]=header[24];
    blink_mapped_memory_banks[3]=header[25];

    blink_tim[0]=header[26];
    blink_tim[1]=header[27];
    blink_tim[2]=header[28];
    blink_tim[3]=header[29];
    blink_tim[4]=header[30];

    blink_rxd=header[31];
    blink_rxe=header[32];

    blink_rxc=header[33];
    blink_txd=header[34];

    blink_txc=header[35];
    blink_umk=header[36];

    blink_uit=header[37];


}


void load_zsf_msx_conf(z80_byte *header)
{

  /*
-Block ID 27: ZSF_MSX_CONF
Ports and internal registers of MSX machine
Byte fields:
0: msx_ppi_register_a
1: msx_ppi_register_b
2: msx_ppi_register_c
*/

  msx_ppi_register_a=header[0];
  msx_ppi_register_b=header[1];
  msx_ppi_register_c=header[2];




 
}

void load_zsf_sms_conf(z80_byte *header)
{

  /*
-Block ID 38: ZSF_SMS_CONF
Ports and internal registers of SMS machine
Byte fields:
0 sms_mapper_type=SMS_MAPPER_TYPE_NONE(0), SMS_MAPPER_TYPE_SEGA(1);

1 sms_mapper_FFFC;
2 sms_mapper_FFFD;
3 sms_mapper_FFFE;
4 sms_mapper_FFFF;
5 flags:
bit 0: Si sms_writing_cram
6: index_sms_escritura_cram
*/

  sms_mapper_type=header[0];
  sms_mapper_FFFC=header[1];
  sms_mapper_FFFD=header[2];
  sms_mapper_FFFE=header[3];
  sms_mapper_FFFF=header[4];
  sms_writing_cram=header[5]&1;
  index_sms_escritura_cram=header[6];


 
}

void load_zsf_sms_cram(z80_byte *header)
{

    int i;

    for (i=0;i<VDP_9918A_SMS_MODE4_MAPPED_PALETTE_COLOURS;i++) {
        vdp_9918a_sms_cram[i]=header[i];
    }


 
}

void load_zsf_ace_conf(z80_byte *header)
{
    z80_byte aceram=header[0];
    debug_printf(VERBOSE_DEBUG,"Setting Jupiter Ace ram to %d kb",aceram);
    set_ace_ramtop(aceram);

}




void load_zsf_svi_conf(z80_byte *header)
{

  /*
-Block ID 27: ZSF_SVI_CONF
Ports and internal registers of SVI machine
Byte fields:
0: svi_ppi_register_a
1: svi_ppi_register_b
2: svi_ppi_register_c
*/

  svi_ppi_register_a=header[0];
  svi_ppi_register_b=header[1];
  svi_ppi_register_c=header[2];




 
}

void load_zsf_datetime(z80_byte *header)
{


			//Fecha. Solo para informacion. No se usa para nada mas
			char buffer_fecha[64];
			sprintf(buffer_fecha,"Snapshot saved on: %d/%02d/%02d %02d:%02d ",
          value_8_to_16(header[3],header[2]),header[1],header[0],header[4],header[5]);
			debug_printf(VERBOSE_INFO,buffer_fecha);

      //printf("%s\n",buffer_fecha);

 
}


void load_zsf_vdp_9918a_conf(z80_byte *header)
{

  /*
-Block ID 30: ZSF_VDP_9918A_CONF
Ports and internal registers of VDP 9918A registers
Byte fields:
0:      vdp_9918a_registers[16];
16:     vdp_9918a_status_register;

17,18:  vdp_9918a_last_command_status_bytes[2];
19:     vdp_9918a_last_command_status_bytes_counter=0;

20,21,22:   vdp_9918a_last_vram_bytes[3];

23,24:  vdp_9918a_last_vram_position;
*/


    int i;
    for (i=0;i<VDP_9918A_TOTAL_REGISTERS;i++) vdp_9918a_registers[i]=header[i];


    vdp_9918a_status_register=header[VDP_9918A_TOTAL_REGISTERS];

    vdp_9918a_last_command_status_bytes[0]=header[VDP_9918A_TOTAL_REGISTERS+1];
    vdp_9918a_last_command_status_bytes[1]=header[VDP_9918A_TOTAL_REGISTERS+2];
    vdp_9918a_last_command_status_bytes_counter=header[VDP_9918A_TOTAL_REGISTERS+3];

    vdp_9918a_last_vram_bytes[0]=header[VDP_9918A_TOTAL_REGISTERS+4];
    vdp_9918a_last_vram_bytes[1]=header[VDP_9918A_TOTAL_REGISTERS+5];
    vdp_9918a_last_vram_bytes[2]=header[VDP_9918A_TOTAL_REGISTERS+6];
    vdp_9918a_last_vram_position=value_8_to_16(header[VDP_9918A_TOTAL_REGISTERS+8],header[VDP_9918A_TOTAL_REGISTERS+7]);



}


void load_zsf_tbblue_conf(z80_byte *header)
{
/*
-Block ID 22: ZSF_TBBLUE_CONF
0: tbblue_last_register
1-256: 256 internal TBBLUE registers
257: tbblue_bootrom_flag
258: tbblue_port_123b
259: Same 3 bytes as ZSF_DIVIFACE_CONF:

  0: Memory size: Value of 2=32 kb, 3=64 kb, 4=128 kb, 5=256 kb, 6=512 kb
  1: Diviface control register
  2: Status bits:
    Bit 0=If entered automatic divmmc paging.
    Bit 1=If divmmc interface is enabled
    Bit 2=If divmmc ports are enabled
    Bit 3=If divide interface is enabled
    Bit 4=If divide ports are enabled
    Bits 5-7: unused by now

262: Word: Copper PC
264: Byte: Copper memory (currently 2048 bytes)
2312:
....
*/

  tbblue_last_register=header[0];
  int i;
  for (i=0;i<256;i++) tbblue_registers[i]=header[1+i];

   tbblue_bootrom.v=header[257];
  tbblue_port_123b=header[258];
  tbblue_port_123b_second_byte=header[259];
 
  load_zsf_diviface_conf(&header[260]);

  tbblue_copper_pc=value_8_to_16(header[264],header[263]);
  for (i=0;i<2048;i++) {
    tbblue_copper_memory[i]=header[265+i];
  }
  



  
 
  //Final settings
  tbblue_set_emulator_setting_timing();

   tbblue_set_emulator_setting_reg_8();


  tbblue_set_memory_pages();   

  //turbo despues de tbblue_set_emulator_setting_timing pues cambia timing
  tbblue_set_emulator_setting_turbo();


  tbblue_set_emulator_setting_divmmc();

}



void load_zsf_tbblue_sprites(z80_byte *header)
{
/*
-Block ID 24: ZSF_TBBLUE_SPRITES
0: 16KB with the sprite patterns
16384: z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][TBBLUE_SPRITE_ATTRIBUTE_SIZE];
*/


  //Patterns de sprites
  //z80_byte tbsprite_new_patterns[TBBLUE_SPRITE_ARRAY_PATTERN_SIZE];

  int i;

  for (i=0;i<TBBLUE_SPRITE_ARRAY_PATTERN_SIZE;i++) {
    tbsprite_new_patterns[i]=header[i];
  }

  int spr,attr;
  int indice=i; //Leer desde donde nos hemos quedado antes
  for (spr=0;spr<TBBLUE_MAX_SPRITES;spr++) {
    for (attr=0;attr<TBBLUE_SPRITE_ATTRIBUTE_SIZE;attr++) {

      //tbsprite_sprites[spr][attr]=header[indice];
      tbblue_write_sprite_value(spr,attr,header[indice]);

      indice++;
    }
  }


}



void load_zsf_tbblue_palettes(z80_byte *header)
{
/*
-Block ID 23 ZSF_TBBLUE_PALETTES
Colour palettes of TBBLUE machine
Byte fields:
0   -511 z80_int tbblue_palette_ula_first[256];
512 -1023 z80_int tbblue_palette_ula_second[256];
1024-1535 z80_int tbblue_palette_layer2_first[256];
1536-2047 z80_int tbblue_palette_layer2_second[256];
2048-2559 z80_int tbblue_palette_sprite_first[256];
2560-3071 z80_int tbblue_palette_sprite_second[256];
3072-3583 z80_int tbblue_palette_tilemap_first[256];
3584-4095 z80_int tbblue_palette_tilemap_second[256];
*/

  int i;

  for (i=0;i<256;i++) {

    int offs=i*2;

    tbblue_palette_ula_first[i]=util_get_value_little_endian(&header[offs]);
    tbblue_palette_ula_second[i]=util_get_value_little_endian(&header[512+offs]);

    tbblue_palette_layer2_first[i]=util_get_value_little_endian(&header[1024+offs]);
    tbblue_palette_layer2_second[i]=util_get_value_little_endian(&header[1536+offs]);    

    tbblue_palette_sprite_first[i]=util_get_value_little_endian(&header[2048+offs]);
    tbblue_palette_sprite_second[i]=util_get_value_little_endian(&header[2560+offs]);    

    tbblue_palette_tilemap_first[i]=util_get_value_little_endian(&header[3072+offs]);
    tbblue_palette_tilemap_second[i]=util_get_value_little_endian(&header[3584+offs]);      
          
  }

  

}


void load_zsf_cpc_conf(z80_byte *header)
{
/*
-Block ID 19: ZSF_CPC_CONF
Ports and internal registers of CPC machine
Byte fields:

0-3: Gate registers
4-19: Palette
20-23: PPI ports
24-55: CRTC registers
56: Border color
57: last crtc selected register


  */

  int i;

  for (i=0;i<4;i++)   cpc_gate_registers[i]=header[i];
  for (i=0;i<16;i++)  cpc_palette_table[i]=header[4+i];
  for (i=0;i<4;i++)   cpc_ppi_ports[i]=header[20+i];
  for (i=0;i<32;i++)  cpc_crtc_registers[i]=header[24+i];


  cpc_border_color=header[56];
  cpc_crtc_last_selected_register=header[57];                      

  cpc_set_memory_pages();

  //Si hay que autoactivar real video porque algun registro de tamanyo de pantalla sea fuera de lo normal
  cpc_if_autoenable_realvideo();
}


void load_zsf_tsconf_conf(z80_byte *header)
{
/*
-Block ID 15: ZSF_TSCONF_CONF
Ports and internal registers of TSCONF machine
Byte fields:

0:255: tsconf_af_ports[256];
256-1279: tsconf_fmaps
*/

  last_port_FC3B=header[0];
  int i;
  for (i=0;i<256;i++) tsconf_af_ports[i]=header[i];

  for (i=0;i<1024;i++) tsconf_fmaps[i]=header[i+256];

  
  tsconf_set_memory_pages();    

  //Sincronizar settings de emulador con los valores de puertos de tsconf
  tsconf_set_emulador_settings();

    tsconf_set_sizes_display();
    modificado_border.v=1;



  //ulaplus_set_extended_mode(zxuno_ports[0x40]);
}

void load_zsf_ql_conf(z80_byte *header)
{
/*
-Block ID 15: ZSF_QL_CONF
Ports and internal registers of QL
Byte fields:

0: unsigned char ql_pc_intr;
1: unsigned char ql_mc_stat;

*/

  ql_pc_intr=header[0];
  ql_mc_stat=header[1];



}


int load_zsf_eof(FILE *ptr_zsf_file,int longitud_memoria)
{

  if (ptr_zsf_file!=NULL) {
    return feof(ptr_zsf_file);
  }
  else {
    if (longitud_memoria>0) return 0;
    else return 1; 
  }

}

//Load snapshot de disco o desde memoria
//Si leer de archivo, filename contiene nombre y no se usa origin_memory ni longitud_memoria
//Si leer en memoria, filename es NULL y origin_memory contiene puntero origen memoria y longitud_memoria contiene longitud bloque memoria
//Si modo rapido, no resetea maquina al cargar snapshot
void load_zsf_snapshot_file_mem(char *filename,z80_byte *origin_memory,int longitud_memoria,int load_fast_mode)
{

  FILE *ptr_zsf_file;

  if (filename!=NULL) {
    ptr_zsf_file=fopen(filename,"rb");
    if (!ptr_zsf_file) {
            debug_printf (VERBOSE_ERR,"Error reading snapshot file %s",filename);
            return;
    }
  }

  else {
    ptr_zsf_file=NULL;
  }
 

  //Verificar que la cabecera inicial coincide
  //zsf_magic_header

  char buffer_magic_header[256];

  int longitud_magic=strlen(zsf_magic_header);


  if (filename!=NULL) {
    int leidos=fread(buffer_magic_header,1,longitud_magic,ptr_zsf_file);

    if (leidos!=longitud_magic) {
      debug_printf(VERBOSE_ERR,"Invalid ZSF file, small magic header");
      fclose(ptr_zsf_file);
      return;
    }
  }

  else {
    memcpy(buffer_magic_header,origin_memory,longitud_magic);
    origin_memory +=longitud_magic;
    longitud_memoria -=longitud_magic;
  }


  //Comparar texto
  buffer_magic_header[longitud_magic]=0;

  if (strcmp(buffer_magic_header,zsf_magic_header)) {
    debug_printf(VERBOSE_ERR,"Invalid ZSF file, invalid magic header");
    fclose(ptr_zsf_file);
    return;
  }


  z80_byte block_header[6];

  //Read blocks
  //while (!feof(ptr_zsf_file)) {
  while (!load_zsf_eof(ptr_zsf_file,longitud_memoria)) {
    //Read header block
    unsigned int leidos;

    if (filename!=NULL) {
      leidos=fread(block_header,1,6,ptr_zsf_file);
    }
    else {
      if (longitud_memoria>0) {
        memcpy(block_header,origin_memory,6);
        origin_memory +=6;      
        leidos=6;
        longitud_memoria -=6;
      }
      else leidos=0;
    }

    if (leidos==0) break; //End while

    if (leidos!=6) {
      debug_printf(VERBOSE_ERR,"Error reading snapshot file. Read: %u Expected: 6",leidos);
      return;
    }

    z80_int block_id;
    block_id=value_8_to_16(block_header[1],block_header[0]);
    unsigned int block_lenght=block_header[2]+(block_header[3]*256)+(block_header[4]*65536)+(block_header[5]*16777216);

    debug_printf (VERBOSE_INFO,"Block id: %u (%s) Length: %u",block_id,zsf_get_block_id_name(block_id),block_lenght);

    z80_byte *block_data;

    //Evitar bloques de longitud cero
    //Por si acaso inicializar a algo
    z80_byte buffer_nothing;
    block_data=&buffer_nothing;

    int cambio_maquina=1;

    if (block_lenght) {
      block_data=malloc(block_lenght);

      if (block_data==NULL) {
        debug_printf(VERBOSE_ERR,"Error allocation memory reading ZSF file");
        return;
      }

      


      if (filename!=NULL) {
        //Read block data
        leidos=fread(block_data,1,block_lenght,ptr_zsf_file);
      }
      else {
        if (longitud_memoria>0) {
          memcpy(block_data,origin_memory,block_lenght);
          origin_memory +=block_lenght;      
          leidos=block_lenght;
          longitud_memoria -=block_lenght;
        }
        else leidos=0;
      }



      if (leidos!=block_lenght) {
        debug_printf(VERBOSE_ERR,"Error reading snapshot file. Read: %u Expected: %u",leidos,block_lenght);
        return;
      }
    }

    //switch for every possible block id
    switch(block_id)
    {

      case ZSF_NOOP_ID:
        //no hacer nada
      break;

      case ZSF_MACHINEID:
        //Si modo rapido, no resetea maquina al cargar snapshot, esto se usa en zeng
        if (load_fast_mode) {
          if (current_machine_type==*block_data) {
            cambio_maquina=0;
          }
        }

        if (cambio_maquina) {
          current_machine_type=*block_data;
          set_machine(NULL);
          reset_cpu();
        }

        //Si cambiamos a QL, resetear ipc, para que no se quede el buffer de teclado medio tonto, por ejemplo
        //Lo ideal seria que toda la info de ipc viniera en el snapshot, incluido el sonido, pero 
        //de momento esto corrige el problema del teclado (que se manifiesta sobretodo con ZENG)
        if (MACHINE_IS_QL) {
            ql_ipc_reset();
        }
      break;

      case ZSF_Z80_REGS_ID:
        load_zsf_snapshot_z80_regs(block_data);
      break;

      case ZSF_MOTO_REGS_ID:
        load_zsf_snapshot_moto_regs(block_data);
      break;    

      case ZSF_MK14_REGS_ID:
        load_zsf_snapshot_mk14_regs(block_data);
      break;     

      case ZSF_MK14_LEDS:
        load_zsf_snapshot_mk14_leds(block_data);
      break;             

      case ZSF_RAMBLOCK:
        load_zsf_snapshot_block_data(block_data,block_lenght);
      break;

      case ZSF_SPEC128_MEMCONF:
        load_zsf_spec128_memconf(block_data);
      break;

      case ZSF_SPEC128_RAMBLOCK:
        load_zsf_spec128_snapshot_block_data(block_data,block_lenght);
      break;


      case ZSF_AYCHIP:
        load_zsf_aychip(block_data);
      break;

      case ZSF_ULA:
        load_zsf_ula(block_data);
      break;

      case ZSF_ULAPLUS:
        load_zsf_ulaplus(block_data);
      break;

      case ZSF_ZXUNO_RAMBLOCK:
        load_zsf_zxuno_snapshot_block_data(block_data,block_lenght);
      break;

      case ZSF_ZXUNO_CONF:
        load_zsf_zxuno_conf(block_data);
      break;

      case ZSF_ZX8081_CONF:
        load_zsf_zx8081_conf(block_data);
      break;    

      case ZSF_ZXEVO_NVRAM:
        load_zsf_zxevo_nvram(block_data);
      break;  

      case ZSF_TSCONF_RAMBLOCK:
        load_zsf_tsconf_snapshot_block_data(block_data,block_lenght);
      break;

      case ZSF_TSCONF_CONF:
        load_zsf_tsconf_conf(block_data);
      break;

      case ZSF_DIVIFACE_CONF:
        load_zsf_diviface_conf(block_data);
      break;  

      case ZSF_DIVIFACE_MEM:
        load_zsf_diviface_snapshot_block_data(block_data,block_lenght);
      break;       

      case ZSF_CPC_RAMBLOCK:
        load_zsf_cpc_snapshot_block_data(block_data,block_lenght);
      break;   

      case ZSF_CPC_CONF:
        load_zsf_cpc_conf(block_data);
      break;     

      case ZSF_PENTAGON_CONF:
        load_zsf_pentagon_conf(block_data);
      break;        

      case ZSF_TBBLUE_RAMBLOCK:
        load_zsf_tbblue_snapshot_block_data(block_data,block_lenght);
      break;    

      case ZSF_TBBLUE_CONF:
        load_zsf_tbblue_conf(block_data);
      break;         

      case ZSF_TBBLUE_PALETTES:
        load_zsf_tbblue_palettes(block_data);
      break;      

      case ZSF_TBBLUE_SPRITES:
        load_zsf_tbblue_sprites(block_data);
      break; 
      
      case ZSF_TIMEX:
        load_zsf_timex(block_data);
      break;      

      case ZSF_MSX_MEMBLOCK:
        load_zsf_msx_snapshot_block_data(block_data,block_lenght);
      break;

      case ZSF_MSX_CONF:
        load_zsf_msx_conf(block_data);
      break;   

      case ZSF_VDP_9918A_VRAM:
        load_zsf_msx_snapshot_vram_data(block_data,block_lenght);
      break;  

      case ZSF_GENERIC_LINEAR_MEM:
        load_ZSF_GENERIC_LINEAR_MEM_snapshot_block_data(block_data,block_lenght);
      break;  

      case ZSF_VDP_9918A_CONF:
        load_zsf_vdp_9918a_conf(block_data);
      break;    

      case ZSF_SNCHIP:
        load_zsf_snchip(block_data);
      break;     

      case ZSF_SVI_CONF:
        load_zsf_svi_conf(block_data);
      break;      

      case ZSF_DATETIME:
        load_zsf_datetime(block_data);
      break;        

      case ZSF_QL_RAMBLOCK:
        load_zsf_ql_snapshot_block_data(block_data,block_lenght);
      break;      

      case ZSF_QL_CONF:
        load_zsf_ql_conf(block_data);
      break;  

      case ZSF_SMS_ROMBLOCK:
        load_zsf_sms_romblock_snapshot_block_data(block_data,block_lenght);
      break;         

      case ZSF_SMS_RAMBLOCK:
        load_zsf_sms_ramblock_snapshot_block_data(block_data,block_lenght);
      break;      

      case ZSF_SMS_CONF:
        load_zsf_sms_conf(block_data);
      break;       

      case ZSF_SMS_CRAM:
        load_zsf_sms_cram(block_data);
      break;

      case ZSF_ACE_CONF:
        load_zsf_ace_conf(block_data);
      break;

      case ZSF_Z88_MEMBLOCK:
        load_zsf_z88_snapshot_block_data(block_data,block_lenght);
      break;      

      case ZSF_Z88_CONF:
        load_zsf_z88_conf(block_data);
      break;

      case ZSF_Z80_HALT_STATE:
        load_zsf_snapshot_z80_halt_state(block_data);
      break;

      case ZSF_TIMEX_DOCK_ROM:
        load_zsf_timex_dockrom_block_data(block_data,block_lenght);
      break;

      case ZSF_MK14_MEMBLOCK:
        load_zsf_snapshot_mk14_block_data(block_data,block_lenght);
      break;    

      case ZSF_CHROME_RAMBLOCK:
        load_zsf_chrome_snapshot_block_data(block_data,block_lenght);
      break;

      case ZSF_PRISM_CONF:
        load_zsf_prism_conf(block_data);
      break;

      case ZSF_PRISM_RAMBLOCK:
        load_zsf_prism_snapshot_block_data(block_data,block_lenght);
      break;

      case ZSF_PRISM_VRAMBLOCK:
        load_zsf_prism_snapshot_block_data_vram(block_data,block_lenght);
      break;  

      case ZSF_CHLOE_HOME_RAMBLOCK:
        load_zsf_chloe_snapshot_block_data_home(block_data,block_lenght);
      break;

      case ZSF_CHLOE_EX_RAMBLOCK:
        load_zsf_chloe_snapshot_block_data_ex(block_data,block_lenght);
      break;

      case ZSF_CHLOE_DOCK_RAMBLOCK:
        load_zsf_chloe_snapshot_block_data_dock(block_data,block_lenght);
      break;

      default:
        debug_printf(VERBOSE_ERR,"Unknown ZSF Block ID: %u. Continue anyway",block_id);
      break;

    }


    if (block_lenght) free(block_data);

  }

  if (filename!=NULL) fclose(ptr_zsf_file);

  //Ajustes posteriores a la carga de snapshot
  if (MACHINE_IS_Z88 && sync_clock_to_z88.v) {
    debug_printf(VERBOSE_DEBUG,"Setting system clock to Z88 clock");
    z88_set_system_clock_to_z88();
  }


}


void load_zsf_snapshot(char *filename)
{
  load_zsf_snapshot_file_mem(filename,NULL,0,0);
}




void save_zsf_snapshot_moto_cpuregs(FILE *ptr,z80_byte **destination_memory,int *longitud_total)
{

    //(4 + 8 + 8)*4=80

    z80_byte header[80];

    int i=0;
 
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_PC));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_SP));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_USP)); i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_SR));  i+=4;

    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_A0));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_A1));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_A2));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_A3));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_A4));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_A5));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_A6));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_A7));  i+=4;

    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_D0));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_D1));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_D2));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_D3));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_D4));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_D5));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_D6));  i+=4;
    util_write_long_value(&header[i],m68k_get_reg(NULL, M68K_REG_D7));  i+=4;



    zsf_write_block(ptr, destination_memory, longitud_total, header,ZSF_MOTO_REGS_ID, 80);

}

void save_zsf_snapshot_mk14_cpuregs(FILE *ptr,z80_byte **destination_memory,int *longitud_total)
{

  z80_byte header[11];


  header[0]=value_16_to_8l(scmp_m_PC.w.l);
  header[1]=value_16_to_8h(scmp_m_PC.w.l);

  header[2]=value_16_to_8l(scmp_m_P1.w.l);
  header[3]=value_16_to_8h(scmp_m_P1.w.l);

  header[4]=value_16_to_8l(scmp_m_P2.w.l);
  header[5]=value_16_to_8h(scmp_m_P2.w.l);

  header[6]=value_16_to_8l(scmp_m_P3.w.l);
  header[7]=value_16_to_8h(scmp_m_P3.w.l);

  header[8]=scmp_m_AC;
  header[9]=scmp_m_ER;
  header[10]=scmp_m_SR;



/*
union SCMP_PAIR    scmp_m_PC;
union SCMP_PAIR    scmp_m_P1;
union SCMP_PAIR    scmp_m_P2;
union SCMP_PAIR    scmp_m_P3;
SCMP_UINT_8   scmp_m_AC;
SCMP_UINT_8   scmp_m_ER;
SCMP_UINT_8   scmp_m_SR;

*/



  zsf_write_block(ptr, destination_memory, longitud_total, header,ZSF_MK14_REGS_ID, 11);




}


void save_zsf_snapshot_z80_cpuregs(FILE *ptr,z80_byte **destination_memory,int *longitud_total)
{

  z80_byte header[27];

  header[0]=reg_c;
  header[1]=reg_b;
  header[2]=reg_e;
  header[3]=reg_d;
  header[4]=reg_l;
  header[5]=reg_h;

  header[6]=get_flags();
  header[7]=reg_a;



  header[8]=value_16_to_8l(reg_ix);
  header[9]=value_16_to_8h(reg_ix);
  header[10]=value_16_to_8l(reg_iy);
  header[11]=value_16_to_8h(reg_iy);

  header[12]=reg_c_shadow;
  header[13]=reg_b_shadow;
  header[14]=reg_e_shadow;
  header[15]=reg_d_shadow;
  header[16]=reg_l_shadow;
  header[17]=reg_h_shadow;



  header[18]=get_flags_shadow();
  header[19]=reg_a_shadow;

  header[20]=(reg_r&127) | (reg_r_bit7&128);

  header[21]=reg_i;

  header[22]=value_16_to_8l(reg_sp);
  header[23]=value_16_to_8h(reg_sp);
  header[24]=value_16_to_8l(reg_pc);
  header[25]=value_16_to_8h(reg_pc);


  z80_byte bits_estado=(iff1.v) | (im_mode==2 ? 2 : 0);
  header[26]=bits_estado;

  zsf_write_block(ptr, destination_memory, longitud_total, header,ZSF_Z80_REGS_ID, 27);

  //Se guarda tambien estado Halt del Z80, en otro tipo de bloque
  z80_byte halt_state=z80_halt_signal.v;
  zsf_write_block(ptr,destination_memory,longitud_total, &halt_state,ZSF_Z80_HALT_STATE, 1);

  //printf("Estado halt al grabar snapshot: %d\n",z80_halt_signal.v);


}




//Guarda en destino el bloque de memoria comprimido, siempre que salga a cuenta comprimirlo. 
//Si ocupa mas que el original, lo guardara comprimido
//Retorna longitud en valor retorno, flag indicando si esta comprimido o no
//Tener en cuenta que el destino sea al menos de tamanyo el doble que origen
int save_zsf_copyblock_compress_uncompres(z80_byte *origen,z80_byte *destino,int tamanyo_orig,int *si_comprimido)
{

  int longitud_comprimido=util_compress_data_repetitions(origen,destino,tamanyo_orig,0xDD);

  if (zsf_force_uncompressed || longitud_comprimido>tamanyo_orig) {
    *si_comprimido=0;
    memcpy(destino,origen,tamanyo_orig);
    return tamanyo_orig;
  }

  else {
    *si_comprimido=1;
    return longitud_comprimido;
  }
}



void snap_aux_save_vdp_9918a_conf(z80_byte *vdpconfblock)
{

/*
-Block ID 30: ZSF_VDP_9918A_CONF
Ports and internal registers of VDP 9918A
Byte fields:
0:      vdp_9918a_registers[16];
16:     vdp_9918a_status_register;

17,18:  vdp_9918a_last_command_status_bytes[2];
19:     vdp_9918a_last_command_status_bytes_counter=0;

20,21,22:   vdp_9918a_last_vram_bytes[3];

23,24:  vdp_9918a_last_vram_position;

*/    


    int i;
    for (i=0;i<VDP_9918A_TOTAL_REGISTERS;i++) vdpconfblock[i]=vdp_9918a_registers[i];

    vdpconfblock[VDP_9918A_TOTAL_REGISTERS]=vdp_9918a_status_register;

    vdpconfblock[VDP_9918A_TOTAL_REGISTERS+1]=vdp_9918a_last_command_status_bytes[0];
    vdpconfblock[VDP_9918A_TOTAL_REGISTERS+2]=vdp_9918a_last_command_status_bytes[1];
    vdpconfblock[VDP_9918A_TOTAL_REGISTERS+3]=vdp_9918a_last_command_status_bytes_counter;

    vdpconfblock[VDP_9918A_TOTAL_REGISTERS+4]=vdp_9918a_last_vram_bytes[0];
    vdpconfblock[VDP_9918A_TOTAL_REGISTERS+5]=vdp_9918a_last_vram_bytes[1];
    vdpconfblock[VDP_9918A_TOTAL_REGISTERS+6]=vdp_9918a_last_vram_bytes[2];
    vdpconfblock[VDP_9918A_TOTAL_REGISTERS+7]=value_16_to_8l(vdp_9918a_last_vram_position);
    vdpconfblock[VDP_9918A_TOTAL_REGISTERS+8]=value_16_to_8h(vdp_9918a_last_vram_position);

 
}


//Guarda snapshot en disco on en memoria destino:
//Si guardar en archivo, filename contiene nombre y no se usa destination_memory
//Si guardar en memoria, filename es NULL y destination_memory contiene puntero destino memoria
void save_zsf_snapshot_file_mem(char *filename,z80_byte *destination_memory,int *longitud_total)
{

  FILE *ptr_zsf_file;

  *longitud_total=0;

  //ZSF File
  if (filename!=NULL) {
    ptr_zsf_file=fopen(filename,"wb");
    if (!ptr_zsf_file) {
            debug_printf (VERBOSE_ERR,"Error writing snapshot file %s",filename);
            return;
    }
  }
  else {
    ptr_zsf_file=NULL;
  }

  int longitud_zsf_magic_header=strlen(zsf_magic_header);

  if (filename!=NULL) {
    //Save header
    fwrite(zsf_magic_header, 1, longitud_zsf_magic_header, ptr_zsf_file);
  }
  else {
    memcpy(destination_memory,zsf_magic_header,longitud_zsf_magic_header );
    destination_memory +=longitud_zsf_magic_header;
    (*longitud_total) +=longitud_zsf_magic_header;
  }
  


  //First save machine ID
  z80_byte save_machine_id=current_machine_type;
  zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, &save_machine_id,ZSF_MACHINEID, 1); 

  //Save date time
  z80_byte datetime_buffer[6];
  		//fecha grabacion
		time_t tiempo = time(NULL);
		struct tm tm = *localtime(&tiempo);

		//printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		datetime_buffer[0]=tm.tm_mday;
		datetime_buffer[1]=tm.tm_mon+1;

		z80_int year;
		year=tm.tm_year + 1900;

		datetime_buffer[2]=value_16_to_8l(year);
		datetime_buffer[3]=value_16_to_8h(year);

		datetime_buffer[4]=tm.tm_hour;
		datetime_buffer[5]=tm.tm_min;

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, datetime_buffer,ZSF_DATETIME, 6); 



  //Save cpu registers. Z80 or Moto or MK14
  if (CPU_IS_MOTOROLA) {
    save_zsf_snapshot_moto_cpuregs(ptr_zsf_file,&destination_memory,longitud_total);
  }
  else if (CPU_IS_SCMP) {
    save_zsf_snapshot_mk14_cpuregs(ptr_zsf_file,&destination_memory,longitud_total);
  }

  else {
    save_zsf_snapshot_z80_cpuregs(ptr_zsf_file,&destination_memory,longitud_total);
  }



 //Ula block y timex. En caso de Spectrum
  if (MACHINE_IS_SPECTRUM) {
    z80_byte ulablock[1];

    ulablock[0]=out_254 & 7;

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, ulablock,ZSF_ULA, 1);
    
    
    
    
    z80_byte timexblock[2];

    timexblock[0]=timex_port_f4;
    timexblock[1]=timex_port_ff;

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, timexblock,ZSF_TIMEX, 2);
    

  }



  //ULAPlus block. Mejor que este esto hacia el principio, asi si por ejemplo se carga zxuno y esta un modo ulaplus extendido,
  //como radastan, no hay problema en que el modo ulaplus estandard esté desactivado
  if (ulaplus_presente.v) {
   z80_byte ulaplusblock[67];
/*
-Block ID 9: ZSF_ULAPLUS
Byte fields:
0: ULAplus mode
1: ULAplus last out to port BF3B
2: ULAplus last out to port FF3B
3-66: ULAplus palette
*/
  ulaplusblock[0]=ulaplus_mode;
  ulaplusblock[1]=ulaplus_last_send_BF3B;
  ulaplusblock[2]=ulaplus_last_send_FF3B;

        int i;
        for (i=0;i<64;i++) ulaplusblock[3+i]=ulaplus_palette_table[i];

     zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, ulaplusblock,ZSF_ULAPLUS, 67);
  }

  //Algunos flags zx80/81
  if (MACHINE_IS_ZX8081) {
    z80_byte zx8081confblock[3];

    z80_byte ram_zx8081=(ramtop_zx8081-16383)/1024;
    zx8081confblock[0]=ram_zx8081;

    z80_byte flags1;
    z80_byte flags2=0;

    flags1=(ram_in_49152.v<<4)|(ram_in_32768.v<<3)|(ram_in_8192.v<<2)|(hsync_generator_active.v<<1)|nmi_generator_active.v;

    zx8081confblock[1]=flags1;
    zx8081confblock[2]=flags2;

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, zx8081confblock,ZSF_ZX8081_CONF, 3);


  }


  //Algunos flags ace
  if (MACHINE_IS_ACE) {
    z80_byte aceconfblock[1];

    z80_byte ram_ace=((ramtop_ace-16383)/1024)+3;
    aceconfblock[0]=ram_ace;

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, aceconfblock,ZSF_ACE_CONF, 1);

  }


  if (MACHINE_IS_SPECTRUM_16_48 || MACHINE_IS_ZX8081 || MACHINE_IS_ACE || MACHINE_IS_TIMEX_TS_TC_2068) {

	  int inicio_ram=16384;
	  int longitud_ram=49152;
	  if (MACHINE_IS_SPECTRUM_16) longitud_ram=16384;

	  if (MACHINE_IS_INVES) {
  		//Grabar tambien la ram oculta de inves (0-16383). Por tanto, grabar todos los 64kb de ram
	  	longitud_ram=65536; //65536
		  inicio_ram=0;
	  }

    
    if (MACHINE_IS_ZX8081) {
      int final_ram=get_ramtop_with_rampacks()+1;
      if (ram_in_8192.v) inicio_ram=8192;
      longitud_ram=final_ram-inicio_ram;
    }

    if (MACHINE_IS_ACE) {
        inicio_ram=8192;
        longitud_ram=65536-inicio_ram;
    }


    //Para el bloque comprimido
    z80_byte *compressed_ramblock=malloc(longitud_ram*2);
    if (compressed_ramblock==NULL) {
      debug_printf (VERBOSE_ERR,"Error allocating memory");
      return;
    } 

    /*
    0: Flags. Currently: bit 0: if compressed
    1,2: Block start
    3,4: Block lenght
    5 and next bytes: data bytes
    */

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(inicio_ram);
    compressed_ramblock[2]=value_16_to_8h(inicio_ram);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[inicio_ram],&compressed_ramblock[5],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    //Store block to file
    debug_printf(VERBOSE_DEBUG,"Saving ZSF_RAMBLOCK start %d length: %d",inicio_ram,longitud_bloque);
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_RAMBLOCK, longitud_bloque+5);

    free(compressed_ramblock);

  }


  if (MACHINE_IS_MK14) {

	  int inicio_ram=0;
	  int longitud_ram=65536;

    //Para el bloque comprimido
    z80_byte *compressed_ramblock=malloc(longitud_ram*2);
    if (compressed_ramblock==NULL) {
      debug_printf (VERBOSE_ERR,"Error allocating memory");
      return;
    } 

    /*
    0: Flags. Currently: bit 0: if compressed
    1 and next bytes: data bytes
    */

    compressed_ramblock[0]=0;


    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[inicio_ram],&compressed_ramblock[1],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    //Store block to file
    debug_printf(VERBOSE_DEBUG,"Saving ZSF_MK14_MEMBLOCK length: %d",longitud_bloque);
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_MK14_MEMBLOCK, longitud_bloque+1);

    free(compressed_ramblock);


    //Guardar los leds
    z80_byte mk14_leds_copia[8];
    int i;

    for (i=0;i<8;i++) {
        mk14_leds_copia[i]=mk14_ledstat[i];
    }


    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, mk14_leds_copia,ZSF_MK14_LEDS, 8);

  }


  //Cartucho timex en DOCK
  if (MACHINE_IS_TIMEX_TS_TC_2068 && timex_cartridge_inserted.v) {

   int longitud_dock_rom=65536;

  
   //Para el bloque comprimido
   z80_byte *compressed_romblock=malloc(longitud_dock_rom*2);
  if (compressed_romblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }      

/*
-Block ID 44: ZSF_TIMEX_DOCK_ROM
A rom dock for timex ts 2068
Byte Fields:
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5 and next bytes: data bytes



  */

    //Nota: grabamos los 64kb del maximo de un cartucho dock, sin importar cuanto realmente hay de ese cartucho
    //al final el bloque está comprimido por lo que lo que no se esté usando no ocupará casi espacio

    compressed_romblock[0]=0;
    compressed_romblock[1]=0;
    compressed_romblock[2]=0;
    compressed_romblock[3]=value_16_to_8l(longitud_dock_rom);
    compressed_romblock[4]=value_16_to_8h(longitud_dock_rom);

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(timex_dock_rom_mem_table[0],&compressed_romblock[5],longitud_dock_rom,&si_comprimido);
    if (si_comprimido) compressed_romblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_TIMEX_DOCK_ROM length: %d",longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_romblock,ZSF_TIMEX_DOCK_ROM, longitud_bloque+5);

    free(compressed_romblock);


  }

/*
  Maquinas Spectrum de 128kb (128, p2, p2a)
  Permitir mas ram segun parametro 
  int mem128_multiplicador;
  Si hay mas de 128kb para maquinas tipo 128k y +2a
  Si vale 1, solo 128k
  Si vale 2, hay 256 kb
  Si vale 4, hay 512 kb
  Otros parametros implicados aqui
  z80_byte *ram_mem_table[32];

  z80_byte puerto_32765;
  z80_byte puerto_8189;

  Hay que crear trama nueva para guardar esos puertos de paginacion, y el valor del multiplicador



*/
  if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 || MACHINE_IS_ZXUNO || MACHINE_IS_CHLOE || MACHINE_IS_PRISM || 
  MACHINE_IS_TBBLUE || MACHINE_IS_PENTAGON || MACHINE_IS_CHROME || MACHINE_IS_ZXEVO) {
/*
-Block ID 5: ZSF_SPEC128_MEMCONF
Byte Fields:
0: Port 32765 contents
1: Port 8189 contents
2: Total memory multiplier: 1 for 128kb ram, 2 for 256 kb ram, 4 for 512 kb ram
*/
	z80_byte memconf[3];
	memconf[0]=puerto_32765;
	memconf[1]=puerto_8189;
	memconf[2]=mem128_multiplicador;

  	zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, memconf,ZSF_SPEC128_MEMCONF, 3);

}

  if (MACHINE_IS_CHROME) {

   int longitud_ram=16384;

  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

  /*

-Block ID 48: ZSF_CHROME_RAMBLOCK
A ram binary block for a chrome
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes
  */

  int paginas=10;
  z80_byte ram_page;

  for (ram_page=0;ram_page<paginas;ram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=ram_page;

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(chrome_ram_mem_table[ram_page],&compressed_ramblock[6],
        longitud_ram,&si_comprimido);


    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_CHROME_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_CHROME_RAMBLOCK, longitud_bloque+6);

  }

  free(compressed_ramblock);




}

if (MACHINE_IS_PRISM) {



   int longitud_ram=8192;

  
   //Para el bloque comprimido y tambien para el bloque de ZSF_PRISM_CONF
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

/*
-Block ID 49: ZSF_PRISM_CONF
Ports and internal registers of TSCONF machine
Byte fields:
0: Last out to port 60987
1-16: prism_ula2_registers[16]
17: prism_ula2_palette_control_colour
18: prism_ula2_palette_control_index
19-21: prism_ula2_palette_control_rgb[3]
22: prism_last_ae3b;
23-278: prism_ae3b_registers[256]
279: 12 bit prism_palette_two[256]
791: end
*/

    compressed_ramblock[0]=prism_rom_page;

    int i;
    for (i=0;i<16;i++) compressed_ramblock[1+i]=prism_ula2_registers[i];

    compressed_ramblock[17]=prism_ula2_palette_control_colour;
    compressed_ramblock[18]=prism_ula2_palette_control_index;

    compressed_ramblock[19]=prism_ula2_palette_control_rgb[0];
    compressed_ramblock[20]=prism_ula2_palette_control_rgb[1];
    compressed_ramblock[21]=prism_ula2_palette_control_rgb[2];

    compressed_ramblock[22]=prism_last_ae3b;

    for (i=0;i<256;i++) compressed_ramblock[23+i]=prism_ae3b_registers[i];

    for (i=0;i<256;i++) {
        compressed_ramblock[279+i*2]=value_16_to_8l(prism_palette_two[i]);
        compressed_ramblock[279+i*2+1]=value_16_to_8h(prism_palette_two[i]);
    }


    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_PRISM_CONF, 791);



  /*

-Block ID 50: ZSF_PRISM_RAMBLOCK
A ram binary block for a prism. blocks of 8kb size
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes
  */

  int paginas=PRISM_SRAM_PAGES;
  z80_byte ram_page;

  for (ram_page=0;ram_page<paginas;ram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=ram_page;

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(prism_ram_mem_table[ram_page],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_PRISM_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_PRISM_RAMBLOCK, longitud_bloque+6);

  }
  
  /*

-Block ID 51: ZSF_PRISM_VRAMBLOCK
A vram binary block for a prism. blocks of 8kb size
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes
  */

  paginas=PRISM_VRAM_PAGES;
  z80_byte vram_page;

  for (vram_page=0;vram_page<paginas;vram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=vram_page;

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(prism_vram_mem_table[vram_page],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_PRISM_VRAMBLOCK ram page: %d length: %d",vram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_PRISM_VRAMBLOCK, longitud_bloque+6);


  }

  free(compressed_ramblock);



}


if (MACHINE_IS_CHLOE) {



   int longitud_ram=16384;

  
   //Para el bloque comprimido 
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

/*
-Block ID 54: ZSF_CHLOE_HOME_RAMBLOCK
A ram binary block for a Chloe, Home banks. blocks of 16kb size
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id
6 and next bytes: data bytes
*/

 

  int paginas=CHLOE_HOME_RAM_TOTAL_PAGES;
  z80_byte ram_page;

  for (ram_page=0;ram_page<paginas;ram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=ram_page;

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(chloe_home_ram_mem_table[ram_page],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_CHLOE_HOME_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_CHLOE_HOME_RAMBLOCK, longitud_bloque+6);

  }
  

/*
-Block ID 55: ZSF_CHLOE_EX_RAMBLOCK
A ram binary block for a Chloe, EX banks. blocks of 8kb size
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id
6 and next bytes: data bytes
*/

  longitud_ram=8192;
  paginas=CHLOE_EX_RAM_TOTAL_PAGES;

  for (ram_page=0;ram_page<paginas;ram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=ram_page;

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(chloe_ex_ram_mem_table[ram_page],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_CHLOE_EX_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_CHLOE_EX_RAMBLOCK, longitud_bloque+6);

  }

/*
-Block ID 54: ZSF_CHLOE_DOCK_RAMBLOCK
A ram binary block for a Chloe, DOCK banks. blocks of 8kb size
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id
6 and next bytes: data bytes
*/


  longitud_ram=8192;
  paginas=CHLOE_DOCK_RAM_TOTAL_PAGES;

  for (ram_page=0;ram_page<paginas;ram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=ram_page;

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(chloe_dock_ram_mem_table[ram_page],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_CHLOE_DOCK_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_CHLOE_DOCK_RAMBLOCK, longitud_bloque+6);

  }


  free(compressed_ramblock);



}



if (MACHINE_IS_ZXEVO) {
  //Grabar nvram
  z80_byte nvramblock[258];

  nvramblock[0]=puerto_eff7;
  nvramblock[1]=zxevo_last_port_dff7;

  int i;
  for (i=0;i<256;i++) nvramblock[i+2]=zxevo_nvram[i];

  zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, nvramblock,ZSF_ZXEVO_NVRAM, 258);


}

if (MACHINE_IS_PENTAGON) {
  //Grabar estado puertos
  z80_byte pentagonconf[1];

  pentagonconf[0]=puerto_eff7;

  zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, pentagonconf,ZSF_PENTAGON_CONF, 1);


}

if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {

   int longitud_ram=16384;

  //Allocate 6+48kb bytes
  /*z80_byte *ramblock=malloc(longitud_ram+6);
  if (ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }*/


  //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

  /*

-Block ID 6: ZSF_SPEC128_RAMBLOCK
A ram binary block for a spectrum 128, p2 or p2a machine
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address
3,4: Block lenght
5: ram block id (0..7) for a spectrum 128k for example
6 and next bytes: data bytes
  */

  int paginas=8*mem128_multiplicador;
  z80_byte ram_page;

  for (ram_page=0;ram_page<paginas;ram_page++) {

	  compressed_ramblock[0]=0;
	  compressed_ramblock[1]=value_16_to_8l(16384);
	  compressed_ramblock[2]=value_16_to_8h(16384);
	  compressed_ramblock[3]=value_16_to_8l(longitud_ram);
	  compressed_ramblock[4]=value_16_to_8h(longitud_ram);
	  compressed_ramblock[5]=ram_page;

	  int si_comprimido;
	  int longitud_bloque=save_zsf_copyblock_compress_uncompres(ram_mem_table[ram_page],&compressed_ramblock[6],longitud_ram,&si_comprimido);
	  if (si_comprimido) compressed_ramblock[0]|=1;

	  debug_printf(VERBOSE_DEBUG,"Saving ZSF_SPEC128_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

	  //Store block to file
	  zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_SPEC128_RAMBLOCK, longitud_bloque+6);

  }

  free(compressed_ramblock);


  }

if (MACHINE_IS_ZXUNO) {

    z80_byte zxunoconfblock[274];

/*
-Block ID 11: ZSF_ZXUNO_CONF
Ports and internal registers of ZXUNO machine
Byte fields:
0: Last out to port FC3B
1-256: 256 internal ZXUNO registers
257: Flash SPI bus index
258: Flash SPI next read byte   
259: Flash SPI status register
260-262: 24 bit value with last spi write address
263-265: 24 bit value with last spi read address
266-273: 8 byte with spi bus contents
*/    

    zxunoconfblock[0]=last_port_FC3B;
    int i;
    for (i=0;i<256;i++) zxunoconfblock[1+i]=zxuno_ports[i];

    zxunoconfblock[257]=zxuno_spi_bus_index;
    zxunoconfblock[258]=next_spi_read_byte;  
    zxunoconfblock[259]=zxuno_spi_status_register;  


    zxunoconfblock[260]=last_spi_write_address & 0xFF;
    zxunoconfblock[261]=(last_spi_write_address>>8) & 0xFF;
    zxunoconfblock[262]=(last_spi_write_address>>16) & 0xFF;

    zxunoconfblock[263]=last_spi_read_address & 0xFF;
    zxunoconfblock[264]=(last_spi_read_address>>8) & 0xFF;
    zxunoconfblock[265]=(last_spi_read_address>>16) & 0xFF;

    for (i=0;i<8;i++) zxunoconfblock[266+i]=zxuno_spi_bus[i];

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, zxunoconfblock,ZSF_ZXUNO_CONF, 274);





   int longitud_ram=16384;

  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

  /*

-Block ID 10: ZSF_ZXUNO_RAMBLOCK
A ram binary block for a zxuno
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes
  */

  int paginas=ZXUNO_SRAM_PAGES;
  z80_byte ram_page;

  for (ram_page=0;ram_page<paginas;ram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=ram_page;

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(zxuno_sram_mem_table_new[ram_page],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_ZXUNO_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_ZXUNO_RAMBLOCK, longitud_bloque+6);

  }

  free(compressed_ramblock);


  }

  if (MACHINE_IS_Z88) {

    z80_byte z88confblock[38];

   
    z88confblock[0]=(z88_internal_rom_size+1)/16384;
    z88confblock[1]=(z88_internal_ram_size+1)/16384;


    //Si hay algun slot tipo hibrido, cambiar numeracion
    z80_byte mem_types[3];
    mem_types[0]=z88_memory_slots[1].type;
    mem_types[1]=z88_memory_slots[2].type;
    mem_types[2]=z88_memory_slots[3].type;

    int i;
    for (i=0;i<3;i++) {
        if (mem_types[i]==4) {
            mem_types[i]=1;
        }
    }

    z88confblock[2]=(mem_types[0]) | (mem_types[1]<<2) | (mem_types[2]<<4);


    if (z88_memory_slots[1].size) z88confblock[3]=(z88_memory_slots[1].size+1)/16384;
    else z88confblock[3]=0;

    if (z88_memory_slots[2].size) z88confblock[4]=(z88_memory_slots[2].size+1)/16384;
    else z88confblock[4]=0;

    if (z88_memory_slots[3].size) z88confblock[5]=(z88_memory_slots[3].size+1)/16384;
    else z88confblock[5]=0;

    //Grabar registros del Blink
    //z80_int blink_pixel_base[4];
    z88confblock[6]=value_16_to_8l(blink_pixel_base[0]);
    z88confblock[7]=value_16_to_8h(blink_pixel_base[0]);

    z88confblock[8]=value_16_to_8l(blink_pixel_base[1]);
    z88confblock[9]=value_16_to_8h(blink_pixel_base[1]);

    z88confblock[10]=value_16_to_8l(blink_pixel_base[2]);
    z88confblock[11]=value_16_to_8h(blink_pixel_base[2]);

    z88confblock[12]=value_16_to_8l(blink_pixel_base[3]);
    z88confblock[13]=value_16_to_8h(blink_pixel_base[3]);

    z88confblock[14]=value_16_to_8l(blink_sbr);
    z88confblock[15]=value_16_to_8h(blink_sbr);

    z88confblock[16]=blink_com;
    z88confblock[17]=blink_int;

    z88confblock[18]=blink_sta;
    z88confblock[19]=blink_epr;

    z88confblock[20]=blink_tmk;
    z88confblock[21]=blink_tsta;

    z88confblock[22]=blink_mapped_memory_banks[0];
    z88confblock[23]=blink_mapped_memory_banks[1];
    z88confblock[24]=blink_mapped_memory_banks[2];
    z88confblock[25]=blink_mapped_memory_banks[3];

    z88confblock[26]=blink_tim[0];
    z88confblock[27]=blink_tim[1];
    z88confblock[28]=blink_tim[2];
    z88confblock[29]=blink_tim[3];
    z88confblock[30]=blink_tim[4];

    z88confblock[31]=blink_rxd;
    z88confblock[32]=blink_rxe;

    z88confblock[33]=blink_rxc;
    z88confblock[34]=blink_txd;

    z88confblock[35]=blink_txc;
    z88confblock[36]=blink_umk;

    z88confblock[37]=blink_uit;
	

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, z88confblock,ZSF_Z88_CONF, 38);



   int longitud_ram=16384;

  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

  /*

-Block ID 41: ZSF_Z88_MEMBLOCK
A ram binary block for a z88
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: bank
  */

    //Grabar ROM interna
    //calculo numero de bancos
    z80_byte bancos_total=(z88_internal_rom_size+1)/16384;

    
    for (i=0;i<bancos_total;i++) {

        z80_byte bank=i;
        compressed_ramblock[0]=0;
        compressed_ramblock[1]=value_16_to_8l(16384);
        compressed_ramblock[2]=value_16_to_8h(16384);
        compressed_ramblock[3]=value_16_to_8l(longitud_ram);
        compressed_ramblock[4]=value_16_to_8h(longitud_ram);
        compressed_ramblock[5]=bank;

        int si_comprimido;
        int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[16384*bank],&compressed_ramblock[6],longitud_ram,&si_comprimido);
        if (si_comprimido) compressed_ramblock[0]|=1;

        debug_printf(VERBOSE_DEBUG,"Saving ZSF_Z88_MEMBLOCK bank: %d length: %d",bank,longitud_bloque);

        //Store block to file
        zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_Z88_MEMBLOCK, longitud_bloque+6);


    }


    //Grabar RAM interna
    //calculo numero de bancos
    bancos_total=(z88_internal_ram_size+1)/16384;

    for (i=0;i<bancos_total;i++) {
        z80_byte bank=0x20+i;

        compressed_ramblock[5]=bank;

        int si_comprimido;
        int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[16384*bank],&compressed_ramblock[6],longitud_ram,&si_comprimido);
        if (si_comprimido) compressed_ramblock[0]|=1;

        debug_printf(VERBOSE_DEBUG,"Saving ZSF_Z88_MEMBLOCK bank: %d length: %d",bank,longitud_bloque);

        //Store block to file
        zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_Z88_MEMBLOCK, longitud_bloque+6);

    }


    //Los 3 slots y siempre que que size!=0 y no haya eprom ni flash en slot 3
    int slot;
    for (slot=1;slot<=3;slot++) {
        if (z88_memory_slots[slot].size!=0) {
            //Hay algo. Si es slot 3, que no sea eprom ni flash
            if (slot==3 && (z88_memory_slots[slot].type==2 || z88_memory_slots[slot].type==3 || z88_memory_slots[slot].type==4) ) {
                debug_printf (VERBOSE_DEBUG,"Do not save eprom/flash on slot 3");
            }

            else {

                //calculo numero de bancos
                bancos_total=(z88_memory_slots[slot].size+1)/16384;
                for (i=0;i<bancos_total;i++) {
                    //save_zx_snapshot_bytes_z88(ptr_zxfile,0x40*slot+i);
                    z80_byte bank=0x40*slot+i;

                    compressed_ramblock[5]=bank;

                    int si_comprimido;
                    int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[16384*bank],&compressed_ramblock[6],longitud_ram,&si_comprimido);
                    if (si_comprimido) compressed_ramblock[0]|=1;

                    debug_printf(VERBOSE_DEBUG,"Saving ZSF_Z88_MEMBLOCK bank: %d length: %d",bank,longitud_bloque);

                    //Store block to file
                    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_Z88_MEMBLOCK, longitud_bloque+6);
                }
            }
        }
    }





  free(compressed_ramblock);


  }


if (MACHINE_IS_MSX) {

    z80_byte msxconfblock[11];

/*
-Block ID 27: ZSF_MSX_CONF
Ports and internal registers of MSX machine
Byte fields:
0: msx_ppi_register_a
1: msx_ppi_register_b
2: msx_ppi_register_c
*/    

    msxconfblock[0]=msx_ppi_register_a;
    msxconfblock[1]=msx_ppi_register_b;
    msxconfblock[2]=msx_ppi_register_c;


    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, msxconfblock,ZSF_MSX_CONF, 3);




    z80_byte vdpconfblock[VDP_9918A_TOTAL_REGISTERS+9];

    snap_aux_save_vdp_9918a_conf(vdpconfblock);

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, vdpconfblock,ZSF_VDP_9918A_CONF, VDP_9918A_TOTAL_REGISTERS+9);  
    




   
int longitud_ram=16384;
  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

/*
-Block ID 28: ZSF_VDP_9918A_VRAM
VRAM contents for machine with vdp 9918a chip
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
*/

        compressed_ramblock[0]=0;
        compressed_ramblock[1]=value_16_to_8l(16384);
        compressed_ramblock[2]=value_16_to_8h(16384);
        compressed_ramblock[3]=value_16_to_8l(longitud_ram); //"Casualidad" que la vram tambien ocupa 16kb
        compressed_ramblock[4]=value_16_to_8h(longitud_ram);


        int si_comprimido;
        int longitud_bloque=save_zsf_copyblock_compress_uncompres(msx_vram_memory,&compressed_ramblock[5],longitud_ram,&si_comprimido);
        if (si_comprimido) compressed_ramblock[0]|=1;

        debug_printf(VERBOSE_DEBUG,"Saving ZSF_VDP_9918A_VRAM length: %d",longitud_bloque);

        
        zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_VDP_9918A_VRAM, longitud_bloque+5);



  /*



-Block ID 26: ZSF_MSX_MEMBLOCK
A ram binary block for a msx
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: slot (0,1,2 or 3)
6: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff)
7: type: 0 rom, 1 ram
8 and next bytes: data bytes
  */


  int slot,segment;

  for (slot=0;slot<4;slot++) {

    for (segment=0;segment<4;segment++) {

      //Store block to file, if block present

      //Y no grabar la rom interna (slot 0, segmento 0 y 1)
      //Aunque luego realmente la rutina de carga si que permite cargar esto, pero dado que será siempre la misma rom de msx,
      //no tiene sentido grabarla y ocupar espacio
      if (msx_memory_slots[slot][segment]!=MSX_SLOT_MEMORY_TYPE_EMPTY) {

        int grabar=1;

        if (slot==0 && (segment==0 || segment==1)) grabar=0;

        if (grabar) {
          compressed_ramblock[0]=0;
          compressed_ramblock[1]=value_16_to_8l(16384);
          compressed_ramblock[2]=value_16_to_8h(16384);
          compressed_ramblock[3]=value_16_to_8l(longitud_ram);
          compressed_ramblock[4]=value_16_to_8h(longitud_ram);
          compressed_ramblock[5]=slot;
          compressed_ramblock[6]=segment;
          compressed_ramblock[7]=msx_memory_slots[slot][segment];

          int offset=(slot*4+segment)*16384;

          int si_comprimido;
          int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[offset],&compressed_ramblock[8],longitud_ram,&si_comprimido);
          if (si_comprimido) compressed_ramblock[0]|=1;

          debug_printf(VERBOSE_DEBUG,"Saving ZSF_MSX_MEMBLOCK slot: %d segment: %d length: %d",slot,segment,longitud_bloque);

          
          zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_MSX_MEMBLOCK, longitud_bloque+8);
        }
      } 

    }
  }

  free(compressed_ramblock);


  }


if (MACHINE_IS_SVI) {


    z80_byte sviconfblock[11];

/*
-Block ID 27: ZSF_SVI_CONF
Ports and internal registers of SVI machine
Byte fields:
0: svi_ppi_register_a
1: svi_ppi_register_b
2: msx_ppi_register_c
*/    

    sviconfblock[0]=svi_ppi_register_a;
    sviconfblock[1]=svi_ppi_register_b;
    sviconfblock[2]=svi_ppi_register_c;


    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, sviconfblock,ZSF_SVI_CONF, 3);



    z80_byte vdpconfblock[VDP_9918A_TOTAL_REGISTERS+9];

    snap_aux_save_vdp_9918a_conf(vdpconfblock);

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, vdpconfblock,ZSF_VDP_9918A_CONF, VDP_9918A_TOTAL_REGISTERS+9);  





   
int longitud_ram=16384;
  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }


/*
-Block ID 28: ZSF_VDP_9918A_VRAM
VRAM contents for machine with vdp 9918a chip
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
*/

        compressed_ramblock[0]=0;
        compressed_ramblock[1]=value_16_to_8l(16384);
        compressed_ramblock[2]=value_16_to_8h(16384);
        compressed_ramblock[3]=value_16_to_8l(longitud_ram); //"Casualidad" que la vram tambien ocupa 16kb
        compressed_ramblock[4]=value_16_to_8h(longitud_ram);

  z80_byte *vram;

  vram=svi_vram_memory;


        int si_comprimido;
        int longitud_bloque=save_zsf_copyblock_compress_uncompres(vram,&compressed_ramblock[5],longitud_ram,&si_comprimido);
        if (si_comprimido) compressed_ramblock[0]|=1;

        debug_printf(VERBOSE_DEBUG,"Saving ZSF_VDP_9918A_VRAM length: %d",longitud_bloque);




        
        zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_VDP_9918A_VRAM, longitud_bloque+5);



  
/*

-Block ID 29: ZSF_GENERIC_LINEAR_MEM
A ram/rom binary block for a coleco, sg1000, spectravideo,or any machine to save memory linear
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff, ...)
  */


  int segment;

  
  //No me complico mucho la vida. Guardo toda la memoria asignada de spectravideo en bloques de 16kb
    for (segment=0;segment<16;segment++) {

      //Store block to file

        compressed_ramblock[0]=0;
        compressed_ramblock[1]=value_16_to_8l(16384);
        compressed_ramblock[2]=value_16_to_8h(16384);
        compressed_ramblock[3]=value_16_to_8l(longitud_ram);
        compressed_ramblock[4]=value_16_to_8h(longitud_ram);
        compressed_ramblock[5]=segment;


        int offset=segment*16384;

        int si_comprimido;
        int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[offset],&compressed_ramblock[6],longitud_ram,&si_comprimido);
        if (si_comprimido) compressed_ramblock[0]|=1;

        debug_printf(VERBOSE_DEBUG,"Saving ZSF_GENERIC_LINEAR_MEM segment: %d length: %d offset: %d",segment,longitud_bloque,offset);

        //printf("Saving ZSF_GENERIC_LINEAR_MEM segment: %d length: %d offset: %d\n",segment,longitud_bloque,offset);
        
        zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_GENERIC_LINEAR_MEM, longitud_bloque+6);
        
    }



 
  
  free(compressed_ramblock);


  }    


if (MACHINE_IS_SG1000 || MACHINE_IS_COLECO) {



    z80_byte vdpconfblock[VDP_9918A_TOTAL_REGISTERS+9];

    snap_aux_save_vdp_9918a_conf(vdpconfblock);

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, vdpconfblock,ZSF_VDP_9918A_CONF, VDP_9918A_TOTAL_REGISTERS+9);  





   
int longitud_ram=16384;
  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }


/*
-Block ID 28: ZSF_VDP_9918A_VRAM
VRAM contents for machine with vdp 9918a chip
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
*/

        compressed_ramblock[0]=0;
        compressed_ramblock[1]=value_16_to_8l(16384);
        compressed_ramblock[2]=value_16_to_8h(16384);
        compressed_ramblock[3]=value_16_to_8l(longitud_ram); //"Casualidad" que la vram tambien ocupa 16kb
        compressed_ramblock[4]=value_16_to_8h(longitud_ram);

  z80_byte *vram;

  if (MACHINE_IS_COLECO) vram=coleco_vram_memory;
  else vram=sg1000_vram_memory;


        int si_comprimido;
        int longitud_bloque=save_zsf_copyblock_compress_uncompres(vram,&compressed_ramblock[5],longitud_ram,&si_comprimido);
        if (si_comprimido) compressed_ramblock[0]|=1;

        debug_printf(VERBOSE_DEBUG,"Saving ZSF_VDP_9918A_VRAM length: %d",longitud_bloque);




        
        zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_VDP_9918A_VRAM, longitud_bloque+5);



  /*



-Block ID 29: ZSF_GENERIC_LINEAR_MEM
A ram/rom binary block for a coleco, sg1000,spectravideo,or anything that has only 64kb
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff, ...)
  */


  int segment;

  

    for (segment=0;segment<4;segment++) {

      //Store block to file

        compressed_ramblock[0]=0;
        compressed_ramblock[1]=value_16_to_8l(16384);
        compressed_ramblock[2]=value_16_to_8h(16384);
        compressed_ramblock[3]=value_16_to_8l(longitud_ram);
        compressed_ramblock[4]=value_16_to_8h(longitud_ram);
        compressed_ramblock[5]=segment;


        int offset=segment*16384;

        int si_comprimido;
        int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[offset],&compressed_ramblock[6],longitud_ram,&si_comprimido);
        if (si_comprimido) compressed_ramblock[0]|=1;

        debug_printf(VERBOSE_DEBUG,"Saving ZSF_GENERIC_LINEAR_MEM segment: %d length: %d",segment,longitud_bloque);

        
        zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_GENERIC_LINEAR_MEM, longitud_bloque+6);
        
      

    }
  
  free(compressed_ramblock);


  }  


if (MACHINE_IS_SMS) {

    z80_byte smsconfblock[7];

/*
-Block ID 38: ZSF_SMS_CONF
Ports and internal registers of SMS machine
Byte fields:
0 sms_mapper_type=SMS_MAPPER_TYPE_NONE(0), SMS_MAPPER_TYPE_SEGA(1);
1 sms_mapper_FFFC;
2 sms_mapper_FFFD;
3 sms_mapper_FFFE;
4 sms_mapper_FFFF;
5 flags:
bit 0: Si sms_writing_cram
6: index_sms_escritura_cram
*/    

    smsconfblock[0]=sms_mapper_type;
    smsconfblock[1]=sms_mapper_FFFC;
    smsconfblock[2]=sms_mapper_FFFD;
    smsconfblock[3]=sms_mapper_FFFE;
    smsconfblock[4]=sms_mapper_FFFF;
    smsconfblock[5]=sms_writing_cram&1;
    smsconfblock[6]=index_sms_escritura_cram;


    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, smsconfblock,ZSF_SMS_CONF, 7);

    //Paleta. La podemos pillar de aqui directamente sin buffer intermedio
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, vdp_9918a_sms_cram,ZSF_SMS_CRAM, VDP_9918A_SMS_MODE4_MAPPED_PALETTE_COLOURS);



    z80_byte vdpconfblock[VDP_9918A_TOTAL_REGISTERS+9];

    snap_aux_save_vdp_9918a_conf(vdpconfblock);

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, vdpconfblock,ZSF_VDP_9918A_CONF, VDP_9918A_TOTAL_REGISTERS+9);   





   
int longitud_ram=16384;
  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }


/*
-Block ID 28: ZSF_VDP_9918A_VRAM
VRAM contents for machine with vdp 9918a chip
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
*/

        compressed_ramblock[0]=0;
        compressed_ramblock[1]=value_16_to_8l(16384);
        compressed_ramblock[2]=value_16_to_8h(16384);
        compressed_ramblock[3]=value_16_to_8l(longitud_ram); //"Casualidad" que la vram tambien ocupa 16kb
        compressed_ramblock[4]=value_16_to_8h(longitud_ram);

  z80_byte *vram;

  vram=sms_vram_memory;


        int si_comprimido;
        int longitud_bloque=save_zsf_copyblock_compress_uncompres(vram,&compressed_ramblock[5],longitud_ram,&si_comprimido);
        if (si_comprimido) compressed_ramblock[0]|=1;

        debug_printf(VERBOSE_DEBUG,"Saving ZSF_VDP_9918A_VRAM length: %d",longitud_bloque);




        
        zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_VDP_9918A_VRAM, longitud_bloque+5);



  /*


0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: memory segment(0=0000-3fff, 1=4000-7fff, 2=8000-bfff, 3=c000-ffff, ...)
  */


  int segment;

    int total_segmentos=sms_cartridge_size/16384;

    for (segment=0;segment<total_segmentos;segment++) {

      //Store block to file

        compressed_ramblock[0]=0;
        compressed_ramblock[1]=value_16_to_8l(16384);
        compressed_ramblock[2]=value_16_to_8h(16384);
        compressed_ramblock[3]=value_16_to_8l(longitud_ram);
        compressed_ramblock[4]=value_16_to_8h(longitud_ram);
        compressed_ramblock[5]=segment;


        int offset=segment*16384;

        int si_comprimido;
        int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[offset],&compressed_ramblock[6],longitud_ram,&si_comprimido);
        if (si_comprimido) compressed_ramblock[0]|=1;

        debug_printf(VERBOSE_DEBUG,"Saving ZSF_SMS_ROMBLOCK segment: %d length: %d",segment,longitud_bloque);

        
        zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_SMS_ROMBLOCK, longitud_bloque+6);
        
      

    }

    //Y bloque RAM

    longitud_ram=8192;
    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    //Segmento en principio no usado en RAM
    compressed_ramblock[5]=0;


    longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[SMS_MAX_ROM_SIZE],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_SMS_RAMBLOCK segment: %d length: %d",segment,longitud_bloque);

    
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_SMS_RAMBLOCK, longitud_bloque+6);
        
          

  
  free(compressed_ramblock);


  }  


if (MACHINE_IS_TBBLUE) {

  #define TBBLUECONFBLOCKSIZE (1+256+1+1+1+3+2+2048)
    //z80_byte tbblueconfblock[TBBLUECONFBLOCKSIZE];

    z80_byte *tbblueconfblock;

    tbblueconfblock=malloc(TBBLUECONFBLOCKSIZE);

    if (tbblueconfblock==NULL) {
      cpu_panic("Cannot allocate memory for tbblue zsf saving");
    }

/*
-Block ID 22: ZSF_TBBLUE_CONF
0: tbblue_last_register
1-256: 256 internal TBBLUE registers
257: tbblue_bootrom_flag
258: tbblue_port_123b
259: tbblue_port_123b_2
260: Same 3 bytes as ZSF_DIVIFACE_CONF:

  0: Memory size: Value of 2=32 kb, 3=64 kb, 4=128 kb, 5=256 kb, 6=512 kb
  1: Diviface control register
  2: Status bits:
    Bit 0=If entered automatic divmmc paging.
    Bit 1=If divmmc interface is enabled
    Bit 2=If divmmc ports are enabled
    Bit 3=If divide interface is enabled
    Bit 4=If divide ports are enabled
    Bits 5-7: unused by now

263: Word: Copper PC
265: Byte: Copper memory (currently 2048 bytes)
2313:
....


....
*/    

  tbblueconfblock[0]=tbblue_last_register;
  int i;
  for (i=0;i<256;i++) tbblueconfblock[1+i]=tbblue_registers[i];

  tbblueconfblock[257]=tbblue_bootrom.v;
  tbblueconfblock[258]=tbblue_port_123b;
  tbblueconfblock[259]=tbblue_port_123b_second_byte;

  tbblueconfblock[260+0]=diviface_current_ram_memory_bits;
  tbblueconfblock[260+1]=diviface_control_register;
  tbblueconfblock[260+2]=diviface_paginacion_automatica_activa.v | (divmmc_diviface_enabled.v<<1) | (divmmc_mmc_ports_enabled.v<<2) | (divide_diviface_enabled.v<<3) | (divide_ide_ports_enabled.v<<4); 

  tbblueconfblock[263]=value_16_to_8l(tbblue_copper_pc);
  tbblueconfblock[263+1]=value_16_to_8h(tbblue_copper_pc);

  for (i=0;i<2048;i++) {
    tbblueconfblock[265+i]=tbblue_copper_memory[i];
  }

  zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, tbblueconfblock,ZSF_TBBLUE_CONF, TBBLUECONFBLOCKSIZE);
  free(tbblueconfblock);


  //
  // Sprites
  //

  /*
  ZSF_TBBLUE_SPRITES
  */



/*
-Block ID 24: ZSF_TBBLUE_SPRITES
0: 16KB with the sprite patterns
16384: z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][TBBLUE_SPRITE_ATTRIBUTE_SIZE];
*/


 #define TBBLUESPRITEBLOCKSIZE (TBBLUE_SPRITE_ARRAY_PATTERN_SIZE+TBBLUE_MAX_SPRITES*TBBLUE_SPRITE_ATTRIBUTE_SIZE)

    z80_byte *tbbluespriteblock;

    tbbluespriteblock=malloc(TBBLUESPRITEBLOCKSIZE);

    if (tbbluespriteblock==NULL) {
      cpu_panic("Cannot allocate memory for tbblue zsf saving");
    }


  //Patterns de sprites
  //z80_byte tbsprite_new_patterns[TBBLUE_SPRITE_ARRAY_PATTERN_SIZE];



  for (i=0;i<TBBLUE_SPRITE_ARRAY_PATTERN_SIZE;i++) {
    tbbluespriteblock[i]=tbsprite_new_patterns[i];
  }


  int spr,attr;
  int indice=i; //desde donde ha acabado antes
  for (spr=0;spr<TBBLUE_MAX_SPRITES;spr++) {
    for (attr=0;attr<TBBLUE_SPRITE_ATTRIBUTE_SIZE;attr++) {
      tbbluespriteblock[indice]=tbsprite_new_sprites[spr][attr];

      indice++;
    }
  }



  zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, tbbluespriteblock,ZSF_TBBLUE_SPRITES, TBBLUESPRITEBLOCKSIZE);
  free(tbbluespriteblock);








  //
  //Paletas de color
  //
/*
-Block ID 23 ZSF_TBBLUE_PALETTES
Colour palettes of TBBLUE machine
Byte fields:
0   -511 z80_int tbblue_palette_ula_first[256];
512 -1023 z80_int tbblue_palette_ula_second[256];

1024-1535 z80_int tbblue_palette_layer2_first[256];
1536-2047 z80_int tbblue_palette_layer2_second[256];

2048-2559 z80_int tbblue_palette_sprite_first[256];
2560-3071 z80_int tbblue_palette_sprite_second[256];

3072-3583 z80_int tbblue_palette_tilemap_first[256];
3584-4095 z80_int tbblue_palette_tilemap_second[256];
*/
  #define TBBLUEPALETTESBLOCKSIZE 4096

    z80_byte *tbbluepalettesblock;

    tbbluepalettesblock=malloc(TBBLUEPALETTESBLOCKSIZE);

    if (tbbluepalettesblock==NULL) {
      cpu_panic("Cannot allocate memory for tbblue zsf saving");
    }

  for (i=0;i<256;i++) {

    int offs=i*2;
    util_store_value_little_endian(&tbbluepalettesblock[offs],tbblue_palette_ula_first[i]);
    util_store_value_little_endian(&tbbluepalettesblock[512+offs],tbblue_palette_ula_second[i]);

    util_store_value_little_endian(&tbbluepalettesblock[1024+offs],tbblue_palette_layer2_first[i]);
    util_store_value_little_endian(&tbbluepalettesblock[1536+offs],tbblue_palette_layer2_second[i]);    

    util_store_value_little_endian(&tbbluepalettesblock[2048+offs],tbblue_palette_sprite_first[i]);
    util_store_value_little_endian(&tbbluepalettesblock[2560+offs],tbblue_palette_sprite_second[i]);       

    util_store_value_little_endian(&tbbluepalettesblock[3072+offs],tbblue_palette_tilemap_first[i]);
    util_store_value_little_endian(&tbbluepalettesblock[3584+offs],tbblue_palette_tilemap_second[i]);        
  }


  zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, tbbluepalettesblock,ZSF_TBBLUE_PALETTES, TBBLUEPALETTESBLOCKSIZE);
  free(tbbluepalettesblock);




  //
  //Bloques de RAM
  //

   int longitud_ram=16384;

   //Grabamos 

  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

  /*

-Block ID 21: ZSF_TBBLUE_RAMBLOCK
A ram binary block for a tbblue. We store all the 2048 MB  (memoria_spectrum pointer). Total pages: 128
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id (in blocks of 16kb)
6 and next bytes: data bytes
  */

  int paginas=tbblue_get_current_ram()/16; //paginas de 16kb 
  z80_byte ram_page;

  for (ram_page=0;ram_page<paginas;ram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=ram_page;

    int si_comprimido;
    int offset_pagina=16384*ram_page;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_spectrum[offset_pagina],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_TBBLUE_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_TBBLUE_RAMBLOCK, longitud_bloque+6);

  }

  free(compressed_ramblock);


  }  

if (MACHINE_IS_CPC) {
  //Configuracion

  z80_byte cpcconfblock[58];

/*
-Block ID 19: ZSF_CPC_CONF
Ports and internal registers of CPC machine
Byte fields:

0-3: Gate registers
4-19: Palette
20-23: PPI ports
24-55: CRTC registers
56: Border color
57: last crtc selected register

*/    


  int i;
  for (i=0;i<4;i++)   cpcconfblock[i]=cpc_gate_registers[i];
  for (i=0;i<16;i++)  cpcconfblock[4+i]=cpc_palette_table[i];
  for (i=0;i<4;i++)   cpcconfblock[20+i]=cpc_ppi_ports[i];
  for (i=0;i<32;i++)  cpcconfblock[24+i]=cpc_crtc_registers[i];


  cpcconfblock[56]=cpc_border_color;
  cpcconfblock[57]=cpc_crtc_last_selected_register;


  zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, cpcconfblock,ZSF_CPC_CONF, 58);



  //Paginas de memoria
  int longitud_ram=16384;

  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

  /*

-Block ID 10: ZSF_CPC_RAMBLOCK
A ram binary block for a cpc
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes
  */

  int paginas=4;
  if (MACHINE_IS_CPC_4128) paginas=8;
  z80_byte ram_page;

  for (ram_page=0;ram_page<paginas;ram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=ram_page;

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(cpc_ram_mem_table[ram_page],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_CPC_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_CPC_RAMBLOCK, longitud_bloque+6);

  }

  free(compressed_ramblock);

  
}

if (MACHINE_IS_TSCONF) {

    z80_byte tsconfconfblock[256+1024];

/*
-Block ID 15: ZSF_TSCONF_CONF
Ports and internal registers of TSCONF machine
Byte fields:

0:255: tsconf_af_ports[256];
256-1279: tsconf_fmaps

*/    

    int i;
    for (i=0;i<256;i++) tsconfconfblock[i]=tsconf_af_ports[i];
    for (i=0;i<1024;i++) tsconfconfblock[i+256]=tsconf_fmaps[i];

    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, tsconfconfblock,ZSF_TSCONF_CONF, 256+1024);



   int longitud_ram=16384;

  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

  /*

-ZSF_TSCONF_RAMBLOCK
A ram binary block for a tsconf
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes
  */

  int paginas=TSCONF_RAM_PAGES;
  z80_int ram_page; //porque pagina puede ir de 0 a 255, y cuando llega a 256 acabamos el bucle for

  for (ram_page=0;ram_page<paginas;ram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=ram_page;

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(tsconf_ram_mem_table[ram_page],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_TSCONF_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_TSCONF_RAMBLOCK, longitud_bloque+6);

  }

  free(compressed_ramblock);


  }


  //Registros chip AY
  if (ay_chip_present.v) {
    int i;
    for (i=0;i<total_ay_chips;i++) {
      z80_byte aycontents[19];

      /*

-Block ID 7: ZSF_AYCHIP
Byte fields:
0: AY Chip number, starting at 0. A normal spectrum will be only the 0. A turbosound, 0 and 1, etc
1: Current AY Chip selected (variable ay_chip_selected). Redundant in all ZSF_AYCHIP blocks
2: AY Last Register selection
3-18: AY Chip contents
      */
      aycontents[0]=i;
      aycontents[1]=ay_chip_selected;
      aycontents[2]=ay_3_8912_registro_sel[i];

      int j;
      for (j=0;j<16;j++) aycontents[3+j]=ay_3_8912_registros[i][j];

      zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, aycontents,ZSF_AYCHIP, 19);
    }
  }

  //Registros chip SN
  if (sn_chip_present.v) {


      z80_byte sncontents[16];

      /*

-Block ID 31: ZSF_SNCHIP
Byte fields:
0-15: SN Chip contents
      */

      int j;
      for (j=0;j<16;j++) sncontents[j]=sn_chip_registers[j];

      zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, sncontents,ZSF_SNCHIP, 16);
    }
  

 //DIVMMC/DIVIDE config
 //Solo si diviface esta habilitado 
 //Esta parte tiene que estar despues de definir y cargar memoria de maquinas, sobre el final del archivo ZSF

 //TODO: no estoy seguro que esto no haga falta para tbblue . probablemente tendra que ver con la manera peculiar de paginar tbblue
 //o esto o bien poner este bloque se se cargue antes de ZSF_TBBLUE_CONF (quiza insertarlo ahi mismo dentro de ZSF_TBBLUE_CONF)
 if (diviface_enabled.v==1 && !MACHINE_IS_TBBLUE) {

/*-Block ID 16: ZSF_DIVIFACE_CONF
Divmmc/divide common settings (diviface), in case it's enabled
Byte fields:

0: Memory size: Value of 2=32 kb, 3=64 kb, 4=128 kb, 5=256 kb, 6=512 kb
1: Diviface control register
2: Status bits: 
  Bit 0=If entered automatic divmmc paging. 
  Bit 1=If divmmc interface is enabled
  Bit 2=If divmmc ports are enabled
  Bit 3=If divide interface is enabled
  Bit 4=If divide ports are enabled  
  Bits 5-7: unused by now
*/

  z80_byte divifaceblock[3];

  divifaceblock[0]=diviface_current_ram_memory_bits;
  divifaceblock[1]=diviface_control_register;
  divifaceblock[2]=diviface_paginacion_automatica_activa.v | (divmmc_diviface_enabled.v<<1) | (divmmc_mmc_ports_enabled.v<<2) | (divide_diviface_enabled.v<<3) | (divide_ide_ports_enabled.v<<4); 

  zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, divifaceblock,ZSF_DIVIFACE_CONF, 3);

  //memoria divmmc solo en el caso que no sea ni zxuno, ni tbblue ni prism 
  if (!MACHINE_IS_ZXUNO && !MACHINE_IS_TBBLUE && !MACHINE_IS_PRISM) {
  /*

-Block ID 17: ZSF_DIVIFACE_MEM
A ram binary block for diviface memory
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1: ram block id 
2 and next bytes: data bytes
  */

  //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(32768);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

  int paginas=(DIVIFACE_FIRMWARE_ALLOCATED_KB+DIVIFACE_RAM_ALLOCATED_KB)/16;
  z80_byte ram_page;

  for (ram_page=0;ram_page<paginas;ram_page++) {

    int longitud_ram=16384;

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=ram_page;

    int si_comprimido;

    z80_byte *puntero_origen;
    puntero_origen=&diviface_memory_pointer[16384*ram_page];

    int longitud_bloque=save_zsf_copyblock_compress_uncompres(puntero_origen,&compressed_ramblock[2],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_DIVIFACE_MEM ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_DIVIFACE_MEM, longitud_bloque+3);

  }

  free(compressed_ramblock);
  }


 }
 
 //DIVMMC/DIVIDE memoria. En caso de maquinas que no son zxuno o tbblue o prism (dado que estas paginan memoria divmmc en su espacio de ram)


  if (MACHINE_IS_QL) {

   z80_byte qlconfblock[2];

/*
-Block ID 15: ZSF_QL_CONF
Ports and internal registers of QL machine
Byte fields:

0: unsigned char ql_pc_intr;
1: unsigned char ql_mc_stat;

*/    
    qlconfblock[0]=ql_pc_intr;
    qlconfblock[1]=ql_mc_stat;



    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, qlconfblock,ZSF_QL_CONF, 2);





   int longitud_ram=16384;

  
   //Para el bloque comprimido
   z80_byte *compressed_ramblock=malloc(longitud_ram*2);
  if (compressed_ramblock==NULL) {
    debug_printf (VERBOSE_ERR,"Error allocating memory");
    return;
  }

  /*

-ZSF_QL_RAMBLOCK
A ram binary block for a ql
Byte Fields:
0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
1,2: Block start address (currently unused)
3,4: Block lenght
5: ram block id 
6 and next bytes: data bytes
  */

    //128kb , desde 20000-3FFFF)
  int paginas=128/16;
  z80_int ram_page; //porque pagina puede ir de 0 a 255, y cuando llega a 256 acabamos el bucle for

  for (ram_page=0;ram_page<paginas;ram_page++) {

    compressed_ramblock[0]=0;
    compressed_ramblock[1]=value_16_to_8l(16384);
    compressed_ramblock[2]=value_16_to_8h(16384);
    compressed_ramblock[3]=value_16_to_8l(longitud_ram);
    compressed_ramblock[4]=value_16_to_8h(longitud_ram);
    compressed_ramblock[5]=ram_page;

    int si_comprimido;
    int longitud_bloque=save_zsf_copyblock_compress_uncompres(&memoria_ql[0x20000+ram_page*16384],&compressed_ramblock[6],longitud_ram,&si_comprimido);
    if (si_comprimido) compressed_ramblock[0]|=1;

    debug_printf(VERBOSE_DEBUG,"Saving ZSF_QL_RAMBLOCK ram page: %d length: %d",ram_page,longitud_bloque);

    //Store block to file
    zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, compressed_ramblock,ZSF_QL_RAMBLOCK, longitud_bloque+6);

  }

  free(compressed_ramblock);

  }



  //test meter un NOOP
  zsf_write_block(ptr_zsf_file,&destination_memory,longitud_total, NULL,0, 0);

  if (filename!=NULL) {
    fclose(ptr_zsf_file);
  }

}



void save_zsf_snapshot(char *filename)
{
  //z80_byte *puntero;
  //puntero=NULL;
  int longitud;
  //Realmente el NULL del puntero a memoria no seria necesario, ya que como el filename no es NULL, se usa el archivo y no se usa el puntero a memoria
  //Y la longitud no la usamos
  save_zsf_snapshot_file_mem(filename,NULL,&longitud);
}

z80_byte *pending_zrcp_put_snapshot_buffer_destino=NULL;
int pending_zrcp_put_snapshot_longitud;

void check_pending_zrcp_put_snapshot(void)
{
  //Aplicar un snapshot que se habia leido por ZRCP. Aqui se llama al final de cada frame de pantalla

  if (pending_zrcp_put_snapshot_buffer_destino!=NULL) {
    debug_printf (VERBOSE_DEBUG,"Putting snapshot coming from ZRCP");

    load_zsf_snapshot_file_mem(NULL,pending_zrcp_put_snapshot_buffer_destino,pending_zrcp_put_snapshot_longitud,1);

    free(pending_zrcp_put_snapshot_buffer_destino);
    pending_zrcp_put_snapshot_buffer_destino=NULL;
  }

}

