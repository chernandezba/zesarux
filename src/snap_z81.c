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

Contains code from EightyOne  - A Windows ZX80/81/clone emulator.
Copyright (C) 2003-2006 Michael D Wynne

*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <string.h>

#include "zx8081.h"
#include "snap_z81.h"
#include "snap.h"
#include "cpu.h"
#include "debug.h"
#include "operaciones.h"

//extern int rowcounter;

void load_snap_cpu(FILE *f);
void load_snap_mem(FILE *f);
void load_snap_zx81(FILE *f);


char *get_token(FILE *f)
{
        static char buffer[256];
        int buflen;
        char c;

        c=fgetc(f);
        while(isspace(c) && !feof(f)) c=fgetc(f);

        //if (feof(f)) return(NULL);

        buflen=0;
        buffer[buflen++]=c;

        c=fgetc(f);
        while(!isspace(c) && !feof(f) && buflen<255)
        {
                buffer[buflen++]=c;
                c=fgetc(f);
        }

        buffer[buflen]='\0';

        //if (!buflen) return(NULL);
        return(buffer);
}

int hex2dec(char *str)
{
        int num;

        num=0;
        while(*str)
        {
                num=num*16;
                if (*str>='0' && *str<='9') num += *str - '0';
                else if (*str>='a' && *str<='f') num += *str +10 - 'a';
                else if (*str>='A' && *str<='F') num += *str +10 - 'A';
                else return(num);
                str++;
        }
        return(num);
}

void load_snap_cpu(FILE *f)
{
        char *tok;
	z80_int registro,nada;

        while(!feof(f))
        {
                tok=get_token(f);
                if (!strcmp(tok,"[MEMORY]"))
                {
                        load_snap_mem(f);
                        return;
                }
                if (!strcmp(tok,"[ZX81]"))
                {
                        load_snap_zx81(f);
                        return;
                }

                if (!strcmp(tok,"PC")) reg_pc = hex2dec(get_token(f));
                if (!strcmp(tok,"SP")) reg_sp = hex2dec(get_token(f));
                if (!strcmp(tok,"HL")) HL = hex2dec(get_token(f));
                if (!strcmp(tok,"DE")) DE = hex2dec(get_token(f));
                if (!strcmp(tok,"BC")) BC = hex2dec(get_token(f));
		

		//Realmente no es importante los registros dado que al final de la carga los establecemos como en basic normal,
		//igual que snapshots .p y .o

                if (!strcmp(tok,"AF")) {
			registro = hex2dec(get_token(f));
			reg_a=value_16_to_8h(registro);
			store_flags(value_16_to_8l(registro));
		}
	
                if (!strcmp(tok,"HL_")) {
			registro = hex2dec(get_token(f));
			reg_l_shadow=value_16_to_8l(registro);
			reg_h_shadow=value_16_to_8h(registro);
		}

                if (!strcmp(tok,"DE_")) {
                        registro = hex2dec(get_token(f));
                        reg_e_shadow=value_16_to_8l(registro);
                        reg_d_shadow=value_16_to_8h(registro);
                }

                if (!strcmp(tok,"BC_")) {
                        registro = hex2dec(get_token(f));
                        reg_c_shadow=value_16_to_8l(registro);
                        reg_b_shadow=value_16_to_8h(registro);
                }

                if (!strcmp(tok,"AF_")) {
                        registro = hex2dec(get_token(f));
                        reg_a_shadow=value_16_to_8h(registro);
                        store_flags_shadow(value_16_to_8l(registro));
                }


                if (!strcmp(tok,"IX")) reg_ix = hex2dec(get_token(f));
                if (!strcmp(tok,"IY")) reg_iy = hex2dec(get_token(f));
                if (!strcmp(tok,"IM")) im_mode = hex2dec(get_token(f));
                if (!strcmp(tok,"IF1")) iff1.v = hex2dec(get_token(f));
                if (!strcmp(tok,"IF2")) iff2.v = hex2dec(get_token(f));
                //if (!strcmp(tok,"HT")) z80.halted = hex2dec(get_token(f));
                if (!strcmp(tok,"HT")) nada = hex2dec(get_token(f));
                if (!strcmp(tok,"IR"))
		
                {
                        int a;

                        a=hex2dec(get_token(f));

                        reg_i = (a>>8) & 255;
                        reg_r = a & 255;
                        reg_r_bit7 = a & 128;

			if (reg_i==0x0E) {

				//Registro I tiene valor tipico de ZX80. Hacemos hotswap a ZX80

                               //ZX80  
                               debug_printf (VERBOSE_INFO,"Register I has tipical ZX80 value 0x0E. Hotswapping to ZX80");
                                current_machine_type=120;
                                set_machine_params();
                                post_set_machine(NULL);

			}


                }
        }

	//Para que el compilador no se queje de variable no usada
	nada++;
}

void load_snap_zx81(FILE *f)
{
        char *tok;
	int nada;

        while(!feof(f))
        {
                tok=get_token(f);
                if (!strcmp(tok,"[MEMORY]"))
                {
                        load_snap_mem(f);
                        return;
                }
                if (!strcmp(tok,"[CPU]"))
                {
                        load_snap_cpu(f);
                        return;
                }


                //if (!strcmp(tok,"NMI")) NMI_generator = hex2dec(get_token(f));
                //if (!strcmp(tok,"HSYNC")) HSYNC_generator = hex2dec(get_token(f));
                //if (!strcmp(tok,"ROW")) rowcounter = hex2dec(get_token(f));
                if (!strcmp(tok,"NMI")) {
			nmi_generator_active.v = hex2dec(get_token(f));
		}
                if (!strcmp(tok,"HSYNC")) hsync_generator_active.v = hex2dec(get_token(f));
                if (!strcmp(tok,"ROW")) nada = hex2dec(get_token(f));
        }

        //Para que el compilador no se queje de variable no usada
        nada++;

}


//Me encuentro con snaps de ZX80 que hacen poke mas alla de la ram asignada. Entonces la sentencia poke 
//mas alla de la RAMTOP lo que haria es pokear en dir & (ramtop), borrando lo que ya se haya escrito en la RAM....
//Por tanto, miramos si el poke quiere ir mas alla y entonces volvemos sin mas
void load_z81_poke (z80_int dir, z80_byte value)
{
	z80_int limite=ramtop_zx8081;
	if (ram_in_32768.v==1) limite=49151;
	if (ram_in_49152.v==1) limite=65535;

	if (dir<=limite) poke_byte_no_time(dir,value);
}

void load_snap_mem(FILE *f)
{
        int Addr, Count, Chr;
        char *tok;

        Addr=16384;

        while(!feof(f))
        {
                tok=get_token(f);

                if (!strcmp(tok,"[CPU]"))
                {
                        load_snap_cpu(f);
                        return;
                }
                else if (!strcmp(tok,"[ZX81]"))
                {
                        load_snap_zx81(f);
                        return;
                }
                else if (!strcmp(tok,"MEMRANGE"))
                {
                        Addr=hex2dec(get_token(f));
                        get_token(f);
                }
                else if (*tok=='*')
                {
                        Count=hex2dec(tok+1);
                        Chr=hex2dec(get_token(f));
                        while(Count--) {
				load_z81_poke(Addr++,Chr);
			}
                }
                else {
			z80_byte valor=hex2dec(tok);
			load_z81_poke(Addr++,valor);
		}
        }
}

void load_snap_ace(FILE *f)
{
	printf ("TODO\n");

	//de momento volver sin mas
	//y que el compilador no se queje de variable no usada
	f=NULL;	
	f++;
	return;
/*
        int puntero_memoria=0x2000;
        unsigned char c;
        int len, eof;

        eof=0;

        while(!eof)
        {
                c=fgetc(f);

                if (c!=0xED) memory[puntero_memoria++]=c;
                else
                {
                        len=fgetc(f);

                        if (!len) eof=1;
                        else
                        {
                                c=fgetc(f);
                                while(len--) memory[puntero_memoria++]=c;
                        }
                }

                if (feof(f)) eof=1;
        }

        zx81.RAMTOP = (memory[0x2081]*256)-1;
        if (zx81.RAMTOP == -1) zx81.RAMTOP=65535;

        puntero_memoria=0x2100;

        z80.af.b.l = memory[puntero_memoria]; z80.af.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.bc.b.l = memory[puntero_memoria]; z80.bc.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.de.b.l = memory[puntero_memoria]; z80.de.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.hl.b.l = memory[puntero_memoria]; z80.hl.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.ix.b.l = memory[puntero_memoria]; z80.ix.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.iy.b.l = memory[puntero_memoria]; z80.iy.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.sp.b.l = memory[puntero_memoria]; z80.sp.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.pc.b.l = memory[puntero_memoria]; z80.pc.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.af_.b.l = memory[puntero_memoria]; z80.af_.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.bc_.b.l = memory[puntero_memoria]; z80.bc.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.de_.b.l = memory[puntero_memoria]; z80.de_.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;
        z80.hl_.b.l = memory[puntero_memoria]; z80.hl_.b.h = memory[puntero_memoria+1]; puntero_memoria+=4;

        z80.im = memory[puntero_memoria]; puntero_memoria+=4;
        z80.iff1 = memory[puntero_memoria]; puntero_memoria+=4;
        z80.iff2 = memory[puntero_memoria]; puntero_memoria+=4;
        z80.i = memory[puntero_memoria]; puntero_memoria+=4;
        z80.r = memory[puntero_memoria];
	*/
}


void load_z81_snapshot(char *archivo)
{
                debug_printf (VERBOSE_INFO,"Loading ZX80/ZX81 .Z81 Snapshot %s",archivo);
                snap_load_z81(archivo);


}

//Rutina comun para cargar snapshot, y para LOAD desde ROM en zx80/zx81
void snap_load_z81(char *archivo)
{
	//los registros ya nos vienen indicados en el snapshot .z81
        snap_load_zx80_zx81_load_z81_file(archivo);

}


void snap_load_zx80_zx81_load_z81_file(char *filename)
{
        //char *p;
        FILE *f;

	/*
        p=filename+strlen(filename)-4;

        if (strcmp(p,".Z81") && strcmp(p,".z81")
                && strcmp(p,".ace") && strcmp(p,".ACE") ) return;


        if (!strcmp(p,".ace") || !strcmp(p,".ACE"))
        {
                f=fopen(filename,"rb");
                if (!f) return;
                load_snap_ace(f);
        }
        else
        {
	*/
                f=fopen(filename,"rt");
                if (!f) return;

                while(!feof(f))
                {
                        if (!strcmp(get_token(f),"[CPU]")) load_snap_cpu(f);
                        if (!strcmp(get_token(f),"[MEMORY]")) load_snap_mem(f);
                        if (!strcmp(get_token(f),"[ZX81]")) load_snap_zx81(f);
                }
        //}

        fclose(f);
        return;
}


/*
int save_snap_z81(char *filename)
{
        FILE *f;
        char *p;
        int Addr, Count, Chr, puntero_memoria;

        p=filename+strlen(filename)-4;

        if (strcmp(p,".Z81") && strcmp(p,".z81")
                && strcmp(p,".ace") && strcmp(p,".ACE") ) return(0);


        if (!strcmp(p,".ace") || !strcmp(p,".ACE"))
        {
                f=fopen(filename,"wb");
                if (!f) return(0);

                puntero_memoria=0x2000;
                memory[puntero_memoria]=0x01; memory[puntero_memoria+1]=0x80;
                memory[puntero_memoria+2]=0x00; memory[puntero_memoria+3]=0x00;

                puntero_memoria=0x2080;
                memory[puntero_memoria]=0x00; memory[puntero_memoria+1]=(zx81.RAMTOP+1)/256;
                memory[puntero_memoria+2]=0x00; memory[puntero_memoria+3]=0x00;

                puntero_memoria=0x0284;
                memory[puntero_memoria]=0x00; memory[puntero_memoria+1]=0x00;
                memory[puntero_memoria+2]=0x00; memory[puntero_memoria+3]=0x00;

                puntero_memoria=0x0288;
                memory[puntero_memoria]=0x00; memory[puntero_memoria+1]=0x00;
                memory[puntero_memoria+2]=0x00; memory[puntero_memoria+3]=0x00;

                puntero_memoria=0x028c;
                memory[puntero_memoria]=0x03; memory[puntero_memoria+1]=0x00;
                memory[puntero_memoria+2]=0x00; memory[puntero_memoria+3]=0x00;

                puntero_memoria=0x0290;
                memory[puntero_memoria]=0x03; memory[puntero_memoria+1]=0x00;
                memory[puntero_memoria+2]=0x00; memory[puntero_memoria+3]=0x00;

                puntero_memoria=0x0294;
                memory[puntero_memoria]=0xfd; memory[puntero_memoria+1]=0xfd;
                memory[puntero_memoria+2]=0x00; memory[puntero_memoria+3]=0x00;

                puntero_memoria+=0x0298;
                memory[puntero_memoria]=0x01; memory[puntero_memoria+1]=0x00;
                memory[puntero_memoria+2]=0x00; memory[puntero_memoria+3]=0x00;

                puntero_memoria+=0x029c;
                memory[puntero_memoria]=0x00; memory[puntero_memoria+1]=0x00;
                memory[puntero_memoria+2]=0x00; memory[puntero_memoria+3]=0x00;

                puntero_memoria=0x2100;

                memory[puntero_memoria] = z80.af.b.l; memory[puntero_memoria+1] = z80.af.b.h; puntero_memoria+=4;
                memory[puntero_memoria] = z80.bc.b.l; memory[puntero_memoria+1] = z80.bc.b.h; puntero_memoria+=4;
                memory[puntero_memoria] = z80.de.b.l; memory[puntero_memoria+1] = z80.de.b.h; puntero_memoria+=4;
                memory[puntero_memoria] = z80.hl.b.l; memory[puntero_memoria+1] = z80.hl.b.h; puntero_memoria+=4;
                memory[puntero_memoria] = z80.ix.b.l; memory[puntero_memoria+1] = z80.ix.b.h; puntero_memoria+=4;
                memory[puntero_memoria] = z80.iy.b.l; memory[puntero_memoria+1] = z80.iy.b.h; puntero_memoria+=4;
                memory[puntero_memoria] = z80.sp.b.l; memory[puntero_memoria+1] = z80.sp.b.h; puntero_memoria+=4;
                memory[puntero_memoria] = z80.pc.b.l; memory[puntero_memoria+1] = z80.pc.b.h; puntero_memoria+=4;
                memory[puntero_memoria] = z80.af_.b.l; memory[puntero_memoria+1] = z80.af_.b.h; puntero_memoria+=4;
                memory[puntero_memoria] = z80.bc_.b.l; memory[puntero_memoria+1] = z80.bc.b.h ; puntero_memoria+=4;
                memory[puntero_memoria] = z80.de_.b.l; memory[puntero_memoria+1] = z80.de_.b.h; puntero_memoria+=4;
                memory[puntero_memoria] = z80.hl_.b.l; memory[puntero_memoria+1] = z80.hl_.b.h; puntero_memoria+=4;

                memory[puntero_memoria] = z80.im ; puntero_memoria+=4;
                memory[puntero_memoria] = z80.iff1; puntero_memoria+=4;
                memory[puntero_memoria] = z80.iff2; puntero_memoria+=4;
                memory[puntero_memoria] = z80.i; puntero_memoria+=4;
                memory[puntero_memoria] = z80.r;
                
                Addr=0x2000;

                while(Addr<32768)
                {
                        Chr=memory[Addr];
                        Count=1;

                        while((memory[Addr+Count]==Chr) && ((Addr+Count)<=32768))
                                Count++;

                        if (Count>240) Count=240;

                        if (Count>3 || Chr==0xed)
                        {
                                fputc(0xed,f);
                                fputc(Count,f);
                        }
                        else    Count=1;

                        fputc(Chr,f);
                        Addr+=Count;
                }

                fputc(0xed,f);
                fputc(0x00,f);
        }
        else
        {
                f=fopen(filename,"wt");
                if (!f) return(1);

                fprintf(f,"[CPU]\n");
                fprintf(f,"PC %04X    SP  %04X\n", z80.pc.w,z80.sp.w);
                fprintf(f,"HL %04X    HL_ %04X\n", z80.hl.w,z80.hl_.w);
                fprintf(f,"DE %04X    DE_ %04X\n", z80.de.w,z80.de_.w);
                fprintf(f,"BC %04X    BC_ %04X\n", z80.bc.w,z80.bc_.w);
                fprintf(f,"AF %04X    AF_ %04X\n", z80.af.w,z80.af_.w);
                fprintf(f,"IX %04X    IY  %04X\n", z80.ix.w,z80.iy.w);
                fprintf(f,"IR %04X\n", (z80.i<<8) | (z80.r7 & 128) | ((z80.r) & 127));

                fprintf(f,"IM %02X      IF1 %02X\n", z80.im, z80.iff1);
                fprintf(f,"HT %02X      IF2 %02X\n", z80.halted, z80.iff2);

                fprintf(f,"\n[ZX81]\n");
                fprintf(f,"NMI %02X     HSYNC %02X\n",
                                NMI_generator, HSYNC_generator);
                fprintf(f,"ROW %03X\n", rowcounter);

                fprintf(f,"\n[MEMORY]\n");

                fprintf(f,"MEMRANGE %04X %04X\n", zx81.ROMTOP+1, zx81.RAMTOP);

                Addr=zx81.ROMTOP+1;

                while(Addr<=zx81.RAMTOP)
                {
                        Chr=memory[Addr];
                        Count=1;

                        while((memory[Addr+Count]==Chr) && ((Addr+Count)<=zx81.RAMTOP))
                                Count++;

                        if (Count>1) fprintf(f,"*%04X %02X ",Count, Chr);
                        else fprintf(f,"%02X ",Chr);

                        Addr += Count;
                }
                fprintf(f,"\n\n[EOF]\n");
        }
        fclose(f);
        return(0);
}

*/

/*
int memory_load(char *filename, int address, int length)
{
        int fptr;
        char file[256];
        int len;


        if (strchr(filename, '\\') || strchr(filename, '/'))
        {
                strcpy(file, filename);
        }
        else
        {
                strcpy(file, zx81.cwd);
                if (file[strlen(file)-1]!='\\') strcat(file,"\\");
                strcat(file,"ROM\\");
                strcat(file,filename);
        }

        fptr=open(file, O_RDONLY | O_BINARY);
        if (fptr<1) return(errno);

        if ((len=read(fptr, memory+address, length))==-1)
        {
                int err;

                err=errno;
                close(fptr);
                return(err);
        }

        close(fptr);

        return(len);
}
*/

/*
int font_load(char *filename, char *address, int length)
{
        int fptr;
        char file[256];
        int len;

        strcpy(file, zx81.cwd);
        if (file[strlen(file)-1]!='\\') strcat(file,"\\");
        strcat(file,"ROM\\");
        strcat(file,filename);

        fptr=open(file, O_RDONLY | O_BINARY);
        if (fptr<1) return(errno);

        if ((len=read(fptr, address, length))==-1)
        {
                int err;

                err=errno;
                close(fptr);
                return(err);
        }

        close(fptr);

        return(len);
}
*/
