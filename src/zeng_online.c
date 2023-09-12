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

Online Network Play (using a central server) related code

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>



#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "network.h"
#include "compileoptions.h"
#include "zeng_online.h"
#include "remote.h"
#include "snap_zsf.h"
#include "autoselectoptions.h"



#ifdef USE_PTHREADS



#endif

//Variables, estructuras etc que se pueden compilar aun sin soporte de pthreads




//Funciones que usan pthreads
#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>

pthread_t xxxxxxx;




#else

//Funciones sin pthreads. ZENG_ONLINE no se llama nunca cuando no hay pthreads, pero hay que crear estas funciones vacias
//para evitar errores de compilacion cuando no hay pthreads

void zeng_online_xxxxx(void)
{
}


#endif