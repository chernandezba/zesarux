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

#include "tape.h"
#include "tape_tap.h"
#include "cpu.h"
#include "operaciones.h"
#include "debug.h"


/*
void (*tape_block_open)(void);
void (*tape_block_read)(void *dir,int longitud);
void (*tape_block_seek)(int longitud,int direccion);
*/

FILE *ptr_mycinta;
FILE *ptr_mycinta_out;


int tape_block_tap_open(void)
{

		ptr_mycinta=fopen(tapefile,"rb");

                if (!ptr_mycinta)
                {
                        debug_printf(VERBOSE_ERR,"Unable to open input file %s",tapefile);
                        //temp prueba texto largo
			//debug_printf(VERBOSE_ERR,"This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.");


			tapefile=0;
                        return 1;
                }

		return 0;

}




int tape_block_tap_read(void *dir,int longitud)
{

if (ptr_mycinta) return fread(dir,1,longitud,ptr_mycinta);
else {
	debug_printf (VERBOSE_ERR,"Tape uninitialized");
	return 0;
}

}

int tape_block_tap_readlength(void)
{
z80_byte buffer[2];

if (ptr_mycinta) {
  if (fread(buffer,1,2,ptr_mycinta)==0) return 0;
  return value_8_to_16(buffer[1],buffer[0]);
}

else {
  debug_printf (VERBOSE_ERR,"Tape uninitialized"); 
  return 0;
}
}

int tape_block_tap_feof(void)
{
    return feof(ptr_mycinta);
}

void tape_block_tap_rewindbegin(void)
{
    fseek(ptr_mycinta,0, SEEK_SET);
}

int tape_block_tap_seek(int longitud,int direccion)
{

if (ptr_mycinta) return fseek(ptr_mycinta,longitud,direccion);
else {
	debug_printf (VERBOSE_ERR,"Tape uninitialized");
	return -1;
}

}




int tape_out_block_tap_open(void)
{

                ptr_mycinta_out=fopen(tape_out_file,"ab");

                if (!ptr_mycinta_out)
                {
                        debug_printf(VERBOSE_ERR,"Unable to open output file %s",tape_out_file);
                        tape_out_file=0;
                        return 1;
                }

                return 0;

}

int tape_out_block_tap_close(void)
{
	if (ptr_mycinta_out) fclose(ptr_mycinta_out);
	else debug_printf (VERBOSE_ERR,"Tape uninitialized");
	return 0;
}




int tape_block_tap_save(void *dir,int longitud)
{
	if (ptr_mycinta_out) return fwrite(dir, 1, longitud, ptr_mycinta_out);
	else {
	  debug_printf (VERBOSE_ERR,"Tape uninitialized");
	  return -1;
	}
}


void tape_block_tap_begin_save(int longitud GCC_UNUSED,z80_byte flag GCC_UNUSED)
{
}

