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


#include "audionull.h"
#include "cpu.h"
#include "debug.h"
#include "audio.h"
#include "settings.h"



int audionull_init(void)
{

	//audio_driver_accepts_stereo.v=1;

	debug_printf (VERBOSE_INFO,"Init Null Audio Driver");

	audio_set_driver_name("null");
	return 0;
}


void audionull_send_frame(char *buffer)
{

        //Para evitar warnings al compilar de unused parameter ‘buffer’ [-Wunused-parameter]
        buffer=0;
        buffer++;


}

int audionull_thread_finish(void)
{
return 0;
}


void audionull_end(void)
{
        debug_printf (VERBOSE_INFO,"Ending null audio driver");
        audionull_thread_finish();
	audio_playing.v=0;

}


void audionull_get_buffer_info (int *buffer_size,int *current_size)
{
  *buffer_size=0;
  *current_size=0;
}


int audionull_can_record_input(void)
{
    return 0;
}