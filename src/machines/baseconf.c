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
#include "zxvision.h"
#include "screen.h"
#include "ula.h"
#include "operaciones.h"
#include "zxevo.h"
#include "mmc.h"


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

/*
 * BaseConf has two sets of four memory-manager registers.  Bit 4 of
 * #7FFD selects the set that is visible to the CPU.  Keep the register
 * contents apart from baseconf_memory_segments[], which describes the
 * four pages currently resolved and is used by the memory/debug cores.
 *
 * Bit 7 in the flags enables substitution of the page bits from #7FFD;
 * bit 6 selects RAM.  Page numbers below are stored non-inverted.
 */
static z80_byte baseconf_mmu_pages[8];
static z80_byte baseconf_mmu_flags[8];
static z80_byte baseconf_text_font[2048];
static int baseconf_dos_signal;
/* PentEvo virtual Beta Disk state.  The firmware stores the RAM-disk
   service in RAM page FE and selects it instead of the physical FDC. */
static z80_byte baseconf_beta_drive_virtual;
static z80_byte baseconf_beta_drive_selected;
static z80_byte baseconf_extended_dos_ports[4];
static z80_int baseconf_palette[16];
static z80_byte baseconf_border_colour;
static z80_int baseconf_nmi_breakpoint;
static int baseconf_nmi_active;
static int baseconf_nmi_exit_countdown;

static z80_byte baseconf_change_ram_page_7ffd(z80_byte value);
static z80_byte baseconf_change_rom_page_trdos(z80_byte value);
extern z80_byte baseconf_shadow_mode_port_77;
extern z80_byte baseconf_last_port_77;
extern z80_byte baseconf_last_port_eff7;

void baseconf_pre_opcode_fetch(z80_int direccion)
{
        int mapa=(puerto_32765&16) ? 4 : 0;
        int segmento=direccion>>14;

        /* The EVO tape emulator installs a hardware breakpoint (normally
           #0556, the 48K ROM loader entry).  Its NMI service is resident in
           RAM page FF, so it does not use ZEsarUX ROM tape traps. */
        if (!baseconf_nmi_active && (baseconf_last_port_bf&0x10) &&
            direccion==baseconf_nmi_breakpoint) {
                printf("BaseConf hardware breakpoint hit at %04XH: entering RAM FF NMI service\n",
                       direccion);
                baseconf_nmi_active=1;
                baseconf_nmi_exit_countdown=0;
                baseconf_set_memory_pages();
                generate_nmi();
        }

        if (baseconf_nmi_active && baseconf_nmi_exit_countdown) {
                baseconf_nmi_exit_countdown--;
                if (!baseconf_nmi_exit_countdown) {
                        baseconf_nmi_active=0;
                        baseconf_set_memory_pages();
                }
        }

        /* A9 allows DOS to leave when execution has moved to RAM. */
        if (baseconf_dos_signal && direccion>=0x4000 &&
            (baseconf_shadow_mode_port_77&2)) {
                baseconf_dos_signal=0;
                baseconf_set_memory_pages();
        }

        /* An M1 in #3Dxx re-enters DOS when substitution is enabled for
           the current MMU window. */
        if (!baseconf_dos_signal && (direccion&0x3f00)==0x3d00 &&
            (baseconf_mmu_flags[mapa+segmento]&128)) {
                baseconf_dos_signal=1;
                baseconf_set_memory_pages();
        }
}

int baseconf_memory_write_allowed(z80_int direccion)
{
        int mapa=(puerto_32765&16) ? 4 : 0;
        return (baseconf_mmu_flags[mapa+(direccion>>14)]&32)==0;
}

z80_byte baseconf_read_config_port(z80_byte puerto_h)
{
        int i;
        z80_byte value=0;

        if (!baseconf_shadow_ports_available()) return 0xff;

        if ((puerto_h&0xf8)==0) {
                return baseconf_mmu_pages[puerto_h&7]^255;
        }

        switch (puerto_h) {
        case 0x08:
                for (i=7;i>=0;i--) value=(value<<1) | ((baseconf_mmu_flags[i]>>6)&1);
                return value;
        case 0x09:
                for (i=7;i>=0;i--) value=(value<<1) | ((baseconf_mmu_flags[i]>>7)&1);
                return value;
        case 0x0a:
                return puerto_32765;
        case 0x0b:
                return baseconf_last_port_eff7;
        case 0x0c:
                /* a14.a9.a8.0.b3..b0, with bit 4 reporting DOS. */
                return ((baseconf_shadow_mode_port_77&0x40)<<1) |
                       ((baseconf_shadow_mode_port_77&3)<<5) |
                       (baseconf_dos_signal ? 0x10 : 0) |
                       (baseconf_last_port_77&0x0f);
        case 0x13:
                return baseconf_beta_drive_virtual;
        case 0x10:
                return baseconf_nmi_breakpoint&0xff;
        case 0x11:
                return baseconf_nmi_breakpoint>>8;
        default:
                return 0xff;
        }
}

z80_byte baseconf_read_extended_dos_port(z80_byte puerto_l)
{
        switch (puerto_l) {
        case 0x2f: return baseconf_extended_dos_ports[0];
        case 0x4f: return baseconf_extended_dos_ports[1];
        case 0x6f: return baseconf_extended_dos_ports[2];
        case 0x8f: return baseconf_extended_dos_ports[3];
        default: return 0xff;
        }
}

z80_byte baseconf_last_port_77;

z80_byte baseconf_shadow_mode_port_77;

z80_byte baseconf_last_port_bf;

z80_byte baseconf_last_port_eff7;

//ver Xpeccy - http://github.com/samstyle/Xpeccy Baseconf ports and memory maping is in ./src/libxpeccy/hardware/pentevo.c

int baseconf_shadow_ports_available(void)
{

        /* An M1 fetch in #3Dxx asserts DOS, which exposes both the
           TR-DOS ROM and the shadow configuration ports. */
        if (baseconf_dos_signal) return 1;

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

void baseconf_write_memory_aux(z80_int direccion,z80_byte valor)
{
        /* BF.bit2 redirects every CPU memory write to the 2 KB text
           font RAM as well as to the normally mapped memory. */
        if (baseconf_last_port_bf&4) {
                baseconf_text_font[direccion&2047]=valor;
        }
}

z80_byte baseconf_get_video_mode(void)
{
        return (baseconf_last_port_eff7&0x20) |
               ((baseconf_last_port_eff7&1)<<1) |
               (baseconf_last_port_77&7);
}

int baseconf_text_mode_active(void)
{
        z80_byte mode=baseconf_get_video_mode();
        return mode==6 || mode==7;
}

static z80_int baseconf_get_palette_colour(z80_byte colour)
{
        return baseconf_palette[colour&15];
}

/* All BaseConf modes are rendered into a 640x400 active area.  Keep the
   scaling integral: 256x192 -> 512x384, 320x200 -> 640x400 and
   640x200 -> 640x400.  The unused area becomes additional border. */
static void baseconf_putpixel_scaled(int x,int y,z80_int colour,
                                    int scale_x,int scale_y,
                                    int offset_x,int offset_y)
{
        int dx,dy;
        int output_x=offset_x+x*scale_x;
        int output_y=offset_y+y*scale_y;

        for (dy=0;dy<scale_y;dy++)
                for (dx=0;dx<scale_x;dx++)
                        scr_putpixel_zoom(output_x+dx,output_y+dy,colour);
}

static void baseconf_get_mode_geometry(int *offset_x,int *offset_y,
                                       int *width,int *height)
{
        z80_byte mode=baseconf_get_video_mode();

        if (mode==0x13 || mode==0x23 || mode==3) {
                *width=512;
                *height=384;
                *offset_x=(640-*width)/2;
                *offset_y=(400-*height)/2;
        }
        else {
                *width=640;
                *height=400;
                *offset_x=0;
                *offset_y=0;
        }
}

static void baseconf_putpixel_absolute(int x,int y,z80_int colour)
{
        int dx,dy;
        int px=x*zoom_x;
        int py=y*zoom_y;

        for (dy=0;dy<zoom_y;dy++)
                for (dx=0;dx<zoom_x;dx++)
                        scr_putpixel(px+dx,py+dy,colour);
}

static void screen_baseconf_refresca_border(void)
{
        int x,y,offset_x,offset_y,width,height;
        int left,top,right,bottom;
        z80_int colour=baseconf_get_palette_colour(baseconf_border_colour);

        if (!border_enabled.v) return;

        baseconf_get_mode_geometry(&offset_x,&offset_y,&width,&height);
        left=BASECONF_LEFT_BORDER_NO_ZOOM+offset_x;
        top=BASECONF_TOP_BORDER_NO_ZOOM+offset_y;
        right=left+width;
        bottom=top+height;

        for (y=0;y<top;y++)
                for (x=0;x<BASECONF_DISPLAY_WIDTH;x++)
                        baseconf_putpixel_absolute(x,y,colour);

        for (y=bottom;y<BASECONF_DISPLAY_HEIGHT;y++)
                for (x=0;x<BASECONF_DISPLAY_WIDTH;x++)
                        baseconf_putpixel_absolute(x,y,colour);

        for (y=top;y<bottom;y++) {
                for (x=0;x<left;x++)
                        baseconf_putpixel_absolute(x,y,colour);
                for (x=right;x<BASECONF_DISPLAY_WIDTH;x++)
                        baseconf_putpixel_absolute(x,y,colour);
        }
}

void baseconf_set_border_colour(z80_int puerto,z80_byte value)
{
        /* A3 is inverted and supplies the fourth border colour bit. */
        baseconf_border_colour=(value&7) | ((~puerto)&8);
}

/* ALCO: 256x192, one 4-bit colour per pixel.  Four interleaved byte
   streams occupy the two adjacent Spectrum screen pages. */
void screen_baseconf_refresca_alco_mode(void)
{
        int x,y;
        int vpage=(puerto_32765&8) ? 7 : 5;

        for (y=0;y<192;y++) {
                int line=((y&0xc0)<<5) | ((y&7)<<8) | ((y&0x38)<<2);
                for (x=0;x<256;x++) {
                        int adr=line+(x>>3);
                        int page=vpage;
                        z80_byte value;

                        if ((x&6)==0 || (x&6)==4) page^=1;
                        if (x&4) adr+=0x2000;
                        value=baseconf_ram_mem_table[page][adr];
                        if (x&1) value=((value&0x38)>>3) | ((value&0x80)>>4);
                        else value=(value&7) | ((value&0x40)>>3);
                        baseconf_putpixel_scaled(x,y,baseconf_get_palette_colour(value),
                                                2,2,64,8);
                }
        }
}

void screen_baseconf_refresca_ega_mode(void)
{
        int x,y;
        int vpage=(puerto_32765&8) ? 7 : 5;

        /* ATM EGA is a 320x200 packed display.  Each byte describes
           the colours of two adjacent pixels; the four byte streams
           alternate between video pages vpage and vpage^4. */
        for (y=0;y<200;y++) {
                for (x=0;x<320;x++) {
                        int sx=x;
                        int pair=sx&~1;
                        int adr=y*40+(sx>>3);
                        int page=vpage;
                        z80_byte value;

                        switch (pair&7) {
                        case 0:
                                page=vpage^4;
                                break;
                        case 2:
                                break;
                        case 4:
                                page=vpage^4;
                                adr+=0x2000;
                                break;
                        default: /* pair 6 */
                                adr+=0x2000;
                                break;
                        }

                        value=baseconf_ram_mem_table[page][adr];
                        if (sx&1) value=((value&0x38)>>3) | ((value&0x80)>>4);
                        else value=(value&7) | ((value&0x40)>>3);
                        baseconf_putpixel_scaled(x,y,baseconf_get_palette_colour(value),
                                                2,2,0,0);
                }
        }
}

/* ATM hardware multicolor is a 640x200 bitmap with one attribute byte
   per group of eight high-resolution pixels. */
void screen_baseconf_refresca_atm_multicolor_mode(void)
{
        int x,y;
        int vpage=(puerto_32765&8) ? 7 : 5;

        for (y=0;y<200;y++) {
                for (x=0;x<640;x++) {
                        int sx=x;
                        int adr=y*40+(sx>>4);
                        int half=(sx&8) ? 0x2000 : 0;
                        z80_byte pixels=baseconf_ram_mem_table[vpage][adr+half];
                        z80_byte attr=baseconf_ram_mem_table[vpage^4][adr+half];
                        z80_byte ink=(attr&7) | ((attr&0x40)>>3);
                        z80_byte paper=((attr&0x38)>>3) | ((attr&0x80)>>4);
                        baseconf_putpixel_scaled(x,y,baseconf_get_palette_colour(
                                (pixels&(0x80>>(sx&7))) ? ink : paper),1,2,0,0);
                }
        }
}

void screen_baseconf_refresca_atm_text_mode(void)
{
        int x,y;
        int vpage=(puerto_32765&8) ? 7 : 5;

        for (y=0;y<200;y++) {
                int row=y>>3;
                int font_line=y&7;
                for (x=0;x<640;x++) {
                        int sx=x;
                        int column=sx>>3;
                        int adr=0x1c0+row*64+(column>>1);
                        z80_byte caracter,attr;

                        if (column&1) {
                                caracter=baseconf_ram_mem_table[vpage][adr+0x2000];
                                attr=baseconf_ram_mem_table[vpage^4][adr+1];
                        }
                        else {
                                caracter=baseconf_ram_mem_table[vpage][adr];
                                attr=baseconf_ram_mem_table[vpage^4][adr^0x2000];
                        }
                        z80_byte font=baseconf_text_font[caracter*8+font_line];
                        z80_byte ink=(attr&7) | ((attr&0x40)>>3);
                        z80_byte paper=((attr&0x38)>>3) | ((attr&0x80)>>4);
                        baseconf_putpixel_scaled(x,y,baseconf_get_palette_colour(
                                (font&(0x80>>(sx&7))) ? ink : paper),1,2,0,0);
                }
        }
}

void screen_baseconf_refresca_hw_multicolor_mode(void)
{
        int x,y;
        int vpage=(puerto_32765&8) ? 7 : 5;
        z80_byte *screen=baseconf_ram_mem_table[vpage];

        /* In hardware multicolor mode every bitmap byte also supplies
           the attribute for that same 8-pixel group. */
        for (y=0;y<192;y++) {
                int adr_line=((y&0xc0)<<5) | ((y&7)<<8) | ((y&0x38)<<2);
                for (x=0;x<256;x++) {
                        z80_byte value=screen[adr_line+(x>>3)];
                        z80_byte ink=(value&7) | ((value&0x40)>>3);
                        z80_byte paper=(value&0x78)>>3;
                        //printf("%d %d\n",ink,paper);
                        baseconf_putpixel_scaled(x,y,baseconf_get_palette_colour(
                                (value&(0x80>>(x&7))) ? ink : paper),2,2,64,8);
                }
        }
}

void screen_baseconf_refresca_text_mode(void)
{
        int x,y;
        int vpage=(puerto_32765&8) ? 7 : 5;
        z80_byte *text=baseconf_ram_mem_table[vpage+3];

        /* PentEvo text is 80x25 characters, hence 640x200 pixels. */
        for (y=0;y<200;y++) {
                int row=y>>3;
                int font_line=y&7;
                for (x=0;x<640;x++) {
                        int half_pixel=x;
                        int column=half_pixel>>3;
                        int font_x=half_pixel&7;
                        int adr=0x1c0+row*64+(column>>1);
                        z80_byte caracter,atributo;

                        if (column&1) {
                                caracter=text[adr+0x1000];
                                atributo=text[adr+0x2001];
                        }
                        else {
                                caracter=text[adr];
                                atributo=text[adr+0x3000];
                        }

                        z80_byte font=baseconf_text_font[caracter*8+font_line];
                        z80_byte ink=(atributo&7)+((atributo&0x40) ? 8 : 0);
                        z80_byte paper=((atributo>>3)&7)+((atributo&0x80) ? 8 : 0);
                        baseconf_putpixel_scaled(x,y,baseconf_get_palette_colour(
                                (font&(0x80>>font_x)) ? ink : paper),1,2,0,0);
                }
        }
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
        int mapa=(puerto_32765&16) ? 4 : 0;

        for (i=0;i<4;i++) {
                z80_byte flags=baseconf_mmu_flags[mapa+i];
                z80_byte pagina=baseconf_mmu_pages[mapa+i];
                z80_byte pagina_es_ram=flags&64;

                /* #xFF7 bit 7 does not mean page 7.  It makes the
                   selected low page bits follow #7FFD dynamically. */
                if (flags&128) {
                        if (pagina_es_ram) pagina=baseconf_change_ram_page_7ffd(pagina);
                        else pagina=baseconf_change_rom_page_trdos(pagina);
                }

                if ((baseconf_shadow_mode_port_77&1)==0) {
                        //A8: if 0, then disable the memory manager. In each window processor is installed the last page of ROM. 0 after reset.
                        pagina=255;
                        pagina_es_ram=0;
                }

                /* EFF7.bit3 has priority over the MMU for the first
                   16K window and exposes RAM page 0 there. */
                if ((baseconf_last_port_eff7&8) && i==0) {
                        pagina=0;
                        pagina_es_ram=1;
                }
                /* A selected virtual Beta Disk replaces the physical FDC.
                   Its service code/data live in the last-but-one RAM page. */
                else if (i==0 && baseconf_nmi_active) {
                        pagina=0xff;
                        pagina_es_ram=1;
                }
                else if (i==0 && baseconf_beta_drive_selected &&
                         baseconf_beta_drive_virtual==baseconf_beta_drive_selected) {
                        pagina=0xfe;
                        pagina_es_ram=1;
                }

                //TODO: A9: If 0 then "force" the inclusion of TR-DOS and the shadow ports. 0 after reset.

                if (pagina_es_ram) {
                        baseconf_memory_paged[i]=baseconf_ram_mem_table[pagina];
                        debug_paginas_memoria_mapeadas[i]=pagina;
                }
                else {
                        pagina=pagina & 31;
                        /* Temporary compatibility for flash images whose
                           low logical ROM slots are stored eight slots later. */
                        if (pagina<8 && baseconf_rom_mem_table[pagina][0]==0xff &&
                           baseconf_rom_mem_table[pagina+8][0]!=0xff) {
                                baseconf_memory_paged[i]=baseconf_rom_mem_table[pagina+8];
                        }
                        else baseconf_memory_paged[i]=baseconf_rom_mem_table[pagina];
                        debug_paginas_memoria_mapeadas[i]=DEBUG_PAGINA_MAP_ES_ROM+pagina;
                }

                baseconf_memory_segments[i]=pagina;
                baseconf_memory_segments_type[i]=pagina_es_ram;

                //printf ("segmento %d pagina %d\n",i,pagina);
        }



  //printf ("32765: %02XH rom %d ram1 %d ram2 %d ram3 %d\n",puerto_32765,rom_page,ram_page_40,ram_page_80,ram_page_c0);


}


void baseconf_hard_reset(void)
{

  debug_printf(VERBOSE_DEBUG,"BaseConf Hard reset cpu");

  //Asignar bloques memoria
  int i;
  for (i=0;i<8;i++) {
          baseconf_mmu_pages[i]=255;
          baseconf_mmu_flags[i]=0;
  }
  for (i=0;i<2048;i++) baseconf_text_font[i]=0;
  baseconf_dos_signal=1;
  baseconf_beta_drive_virtual=0;
  baseconf_beta_drive_selected=0;
  for (i=0;i<4;i++) baseconf_extended_dos_ports[i]=0;
  for (i=0;i<16;i++) baseconf_palette[i]=i;
  baseconf_border_colour=0;
  baseconf_nmi_breakpoint=0;
  baseconf_nmi_active=0;
  baseconf_nmi_exit_countdown=0;


  reset_cpu();


       //Borrar toda memoria ram
        int d;
        z80_byte *puntero;

        for (i=0;i<BASECONF_RAM_PAGES;i++) {
                puntero=baseconf_ram_mem_table[i];
                for (d=0;d<16384;d++,puntero++) {
                        *puntero=0;
                }
        }
/* Hardware reset value is encoded as 0x83 internally: A14=1 and
   data bits 1:0=11.  In ZEsarUX both parts are kept separately.  This
   selects the ordinary ZX video mode while leaving A8/A9 cleared. */
baseconf_last_port_77=3;
baseconf_shadow_mode_port_77=0x40;
baseconf_last_port_bf=0;
baseconf_last_port_eff7=0;
baseconf_sd_enabled=1;
baseconf_sd_cs=1;

        baseconf_set_memory_pages();

}

//Cambia el valor de entrada de numero de pagina ram segun :
/*
for RAM - in the window there is a substitution under 3 or 6 bits (depending on the mode of ZX Spectrum 128k or pentagon 1024k)
page numbers are not inverse bits from port # 7FFD.
*/
static z80_byte baseconf_change_ram_page_7ffd(z80_byte value)
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

static z80_byte baseconf_change_rom_page_trdos(z80_byte value)
{
        value=value&254;
        if (baseconf_dos_signal) value|=1;
        return value;
}

void baseconf_out_port(z80_int puerto,z80_byte valor)
{

        z80_byte puerto_h=puerto>>8;
        z80_byte puerto_l=puerto&0xff;



        /* Newer EVO firmware writes the additional configuration registers
           through xxBD.  13BD marks drives A-D that are RAM disks. */
        if ((puerto&0x00ff)==0xbd && baseconf_shadow_ports_available() &&
            (((puerto_h&0xfc)==0x10) || puerto_h<=1)) {
                int extended_register=((puerto_h&0xfc)==0x10) ?
                                      (puerto_h&3) : puerto_h;
                if (extended_register==0) {
                        baseconf_nmi_breakpoint=(baseconf_nmi_breakpoint&0xff00)|valor;
                        printf("BaseConf NMI breakpoint low=%02XH, address=%04XH\n",
                               valor,baseconf_nmi_breakpoint);
                }
                else if (extended_register==1) {
                        baseconf_nmi_breakpoint=(baseconf_nmi_breakpoint&0x00ff)|(valor<<8);
                        printf("BaseConf NMI breakpoint high=%02XH, address=%04XH\n",
                               valor,baseconf_nmi_breakpoint);
                }
                else if (extended_register==3) {
                        baseconf_beta_drive_virtual=valor&0x0f;
                        baseconf_set_memory_pages();
                }
        }

        /* OUT #xxBE from the resident service restores the previous mapping
           after the two M1 cycles of RETN. */
        else if ((puerto&0x00ff)==0xbe && baseconf_nmi_active) {
                printf("BaseConf NMI service exit through %04XH\n",puerto);
                baseconf_nmi_exit_countdown=2;
        }

        /* The Beta Disk system register contains the selected drive.  It is
           still decoded for a virtual drive although the WD1793 ports are not. */
        else if ((puerto&0x00ff)==0xff && baseconf_shadow_ports_available()) {
                /* A14 of the last #xx77 access selects the meaning of #xxFF.
                   With A14 low it programs the palette; with A14 high it is
                   the Beta Disk system register.  BaseConf software such as
                   Hypnotoad does not require BF.5 for palette writes. */
                if (!(baseconf_shadow_mode_port_77&0x40)) {
                        z80_int inverted_value=(~valor)&0xff;
                        z80_int inverted_port=(~puerto)&0xffff;
                        int r=((inverted_value&0x02)<<2) |
                              ((inverted_value&0x40)>>4) |
                              ((inverted_port&0x0200)>>8) |
                              ((inverted_port&0x4000)>>14);
                        int g=((inverted_value&0x10)>>1) |
                              ((inverted_value&0x80)>>5) |
                              ((inverted_port&0x1000)>>11) |
                              ((inverted_port&0x8000)>>15);
                        int b=((inverted_value&0x01)<<3) |
                              ((inverted_value&0x20)>>3) |
                              ((inverted_port&0x0100)>>7) |
                              ((inverted_port&0x2000)>>13);
                        baseconf_palette[baseconf_border_colour]=
                                PRISM_INDEX_FIRST_COLOR+(r<<8)+(g<<4)+b;
                }
                else {
                        baseconf_beta_drive_selected=valor;
                        baseconf_set_memory_pages();
                }
        }

        else if (baseconf_shadow_ports_available() &&
                 (puerto_l==0x2f || puerto_l==0x4f ||
                  puerto_l==0x6f || puerto_l==0x8f)) {
                baseconf_extended_dos_ports[(puerto_l-0x2f)>>5]=valor;
        }

        //xxBFH
        //Enable shadow mode ports write permission in ROM.
        else if ( (puerto&0x00FF)==0xBF ) {
               baseconf_last_port_bf=valor;

               if ((valor&0x10) || baseconf_nmi_breakpoint)
                       printf("BaseConf BF=%02XH: hardware breakpoint %s at %04XH\n",
                              valor,(valor&0x10) ? "enabled" : "disabled",
                              baseconf_nmi_breakpoint);

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
                 z80_byte es_ram=valor&64;
                 z80_byte pagina=(valor^255)&(es_ram ? 63 : 31);
                 z80_byte segmento=(puerto_h>>6)+((puerto_32765&16) ? 4 : 0);

                 baseconf_mmu_pages[segmento]=pagina;
                 baseconf_mmu_flags[segmento]=valor&0xC0;

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
                z80_byte pagina=valor^255;
                z80_byte segmento=(puerto_h>>6)+((puerto_32765&16) ? 4 : 0);

                baseconf_mmu_pages[segmento]=pagina;
                /* #x7F7 supplies all eight inverted page bits, but does not
                   alter the substitution flag previously set by #xFF7. */
                baseconf_mmu_flags[segmento] |=64;

               baseconf_set_memory_pages();
        }

        //xBF7H: write protection for the selected MMU window
        else if ( (puerto&0x0FFF)==0xBF7 && baseconf_shadow_ports_available() ) {
                z80_byte segmento=(puerto_h>>6)+((puerto_32765&16) ? 4 : 0);
                baseconf_mmu_flags[segmento] &=~32;
                if (valor&1) baseconf_mmu_flags[segmento] |=32;
        }

        else if (puerto==0x7ffd) {
                /* In 128K paging mode bit 5 locks subsequent #7FFD
                   writes until reset.  MMU writes through #xFF7/#x7F7
                   remain available. */
                if ((baseconf_last_port_eff7&4) && (puerto_32765&32)) return;

                puerto_32765=valor;

                baseconf_set_memory_pages();

                //printf ("mapping segun puerto 32765\n");
        }

        //Puertos NVRAM.
	else if (puerto==0xeff7 && !baseconf_shadow_ports_available() ) puerto_eff7=valor;
	else if (puerto==0xdff7 && !baseconf_shadow_ports_available() ) {
		zxevo_last_port_dff7=valor;
		if (valor==0xed) printf("BaseConf CMOS selected EDH through DFF7H\n");
	}
        else if (puerto==0xdef7 && baseconf_shadow_ports_available() ) {
                zxevo_last_port_dff7=valor;
                if (valor==0xed) printf("BaseConf CMOS selected EDH\n");
        }


	else if (puerto==0xbff7 && !baseconf_shadow_ports_available() ) {
		/* On BaseConf #EFF7 is the configuration register kept in
		   baseconf_last_port_eff7.  Using the generic puerto_eff7 here
		   left non-shadow CMOS access permanently disabled. */
		if (baseconf_last_port_eff7&128) {
			zxevo_nvram[zxevo_last_port_dff7]=valor;
			if (zxevo_last_port_dff7==0xed)
				printf("BaseConf CMOS EDH written %02XH through BFF7H: Emu tape=%d Autostart=%d\n",
				       valor,(valor&0x40) ? 1 : 0,(valor&4) ? 1 : 0);
		}
	}

        else if (puerto==0xbef7 && baseconf_shadow_ports_available() ) {
                        //Note: In the shadow mode port # BEF7 available regardless of bit 7 port # EFF7.
		 zxevo_nvram[zxevo_last_port_dff7]=valor;
		 if (zxevo_last_port_dff7==0xed)
                         printf("BaseConf CMOS EDH written %02XH: Emu tape=%d Autostart=%d\n",
                                valor,(valor&0x40) ? 1 : 0,(valor&4) ? 1 : 0);
					}
        else if ( (puerto&0x00FF)==0x77 ) {
                baseconf_sd_enabled=valor&1;
                baseconf_sd_cs=(valor&2) ? 1 : 0;
                mmc_cs(baseconf_sd_cs ? 0xff : 0xfe);
        }

        else if ( (puerto&0x00FF)==0x57 ) {
                if (baseconf_sd_enabled && !baseconf_sd_cs) mmc_write(valor);
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


//Refresco pantalla sin rainbow
void baseconf_refresca_pantalla_no_rainbow_standard_48k(void)
{
    int x,y,bit;
    z80_int direccion,dir_atributo;
    z80_byte byte_leido;
    int color=0;
    int fila;
    //int zx,zy;

    z80_byte attribute,ink,paper,bright,flash,aux;



    if (simulate_screen_zx8081.v==1) {
        //simular modo video zx80/81
        scr_simular_video_zx8081();
        return;
    }


    z80_byte *screen=get_base_mem_pantalla();

    //printf ("dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
    z80_byte x_hi;

    for (y=0;y<192;y++) {
        //direccion=16384 | devuelve_direccion_pantalla(0,y);

        //direccion=16384 | screen_addr_table[(y<<5)];
        direccion=screen_addr_table[(y<<5)];


        fila=y/8;
        dir_atributo=6144+(fila*32);
        for (x=0,x_hi=0;x<32;x++,x_hi +=8) {



            byte_leido=screen[direccion];
            attribute=screen[dir_atributo];

            //Prueba de un modo de video inventado en que el color de la tinta sale de los 4 bits de la zona de pixeles
            //int ink1,ink2;

            if (scr_refresca_sin_colores.v) {
                attribute=56;
                //ink1=(byte_leido >>4)&0xF;
                //ink2=(byte_leido    )&0xF;

            }

            else {
                if (scr_refresca_show_attribute_grid.v) {
                    int cuad=(fila+x)%2;
                    if (cuad) attribute=56+64;
                    else attribute=56;
                }
            }


            ink=attribute &7;
            paper=(attribute>>3) &7;
            bright=(attribute) &64;
            flash=(attribute)&128;
            if (flash) {
                    //intercambiar si conviene
                    if (estado_parpadeo.v) {
                            aux=paper;
                            paper=ink;
                            ink=aux;
                    }
            }

            if (bright) {
                ink +=8;
                paper +=8;
            }

            for (bit=0;bit<8;bit++) {

                color= ( byte_leido & 128 ? ink : paper );
                //if (scr_refresca_sin_colores.v) {
                //	if (bit<=3) color= ( byte_leido & 128 ? ink1 : paper );
                //	else color= ( byte_leido & 128 ? ink2 : paper );
                //}

                /*

                Prueba cutre de visualizar la imagen en un plano 3D
                int xfinal,yfinal;
                //191-y porque el 0,0 lo tenemos arriba del todo pero la funcion de 3D lo asume abajo del todo
                zxvision_widgets_draw_particles_3d_convert(x_hi+bit,191-y,0,&xfinal,&yfinal);
                scr_putpixel_zoom(xfinal,191-yfinal,color);
                if (xfinal<0) printf("X %d\n",xfinal);

                */

                baseconf_putpixel_scaled(x_hi+bit,y,color,2,2,64,8);

                byte_leido=byte_leido<<1;
            }


            //temp
            //else {
            //	printf ("no refrescamos zona x %d fila %d\n",x,fila);
            //}


            direccion++;
            dir_atributo++;
        }

    }

}







//Refresco pantalla sin rainbow
void baseconf_refresca_pantalla_no_rainbow(void)
{


        z80_byte baseconf_mode=baseconf_get_video_mode();
        if (baseconf_mode==0x13) {
            printf("BaseConf video mode 13H: ALCO 16 colour, pixels 256x192 -> 512x384, border L/R=104 T/B=96\n");
            screen_baseconf_refresca_alco_mode();
            return;
        }
        if (baseconf_mode==0) {
            printf("BaseConf video mode 00H: ATM EGA, pixels 320x200 -> 640x400, border L/R=40 T/B=88\n");
            screen_baseconf_refresca_ega_mode();
            return;
        }
        if (baseconf_mode==0x23) {
            printf("BaseConf video mode 23H: ZX hardware multicolor, pixels 256x192 -> 512x384, border L/R=104 T/B=96\n");
            screen_baseconf_refresca_hw_multicolor_mode();
            return;
        }
        if (baseconf_mode==2) {
            printf("BaseConf video mode 02H: ATM hardware multicolor, pixels 640x200 -> 640x400, border L/R=40 T/B=88\n");
            screen_baseconf_refresca_atm_multicolor_mode();
            return;
        }
        if (baseconf_mode==6) {
            printf("BaseConf video mode 06H: ATM text, pixels 640x200 -> 640x400, border L/R=40 T/B=88\n");
            screen_baseconf_refresca_atm_text_mode();
            return;
        }
        if (baseconf_mode==7) {
            printf("BaseConf video mode 07H: EVO text, pixels 640x200 -> 640x400, border L/R=40 T/B=88\n");
            screen_baseconf_refresca_text_mode();
            return;
        }
        if (baseconf_mode==3)
            printf("BaseConf video mode 03H: standard ZX, pixels 256x192 -> 512x384, border L/R=104 T/B=96\n");
        else
            printf("BaseConf video mode %02XH: unknown, using standard ZX renderer, pixels 256x192 -> 512x384, border L/R=104 T/B=96\n",
                   baseconf_mode);
        baseconf_refresca_pantalla_no_rainbow_standard_48k();
}


void baseconf_refresca_pantalla(void)
{
        /* Real-video timing is not implemented yet, so both paths use the
           complete frame renderer. */
        baseconf_refresca_pantalla_no_rainbow();
        screen_baseconf_refresca_border();
        modificado_border.v=0;
}
