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

#ifndef ATOMIC_H
#define ATOMIC_H

#include "compileoptions.h"


#if defined(__APPLE__)
    //En Mac OS X

    //old
    //#include <libkern/OSAtomic.h>
    //typedef uint32_t z_atomic_semaphore;

    #include <stdatomic.h>

    typedef _Atomic int z_atomic_semaphore;


    #else 

    #ifdef MINGW
        //En Windows
        //TODO: how to use semafores on mingw
            typedef int z_atomic_semaphore;

        #else

            //En Linux
            //#include <asm/atomic.h>
            typedef char z_atomic_semaphore;

    #endif

#endif

//Retorna 1 si no se ha establecido el valor
extern int z_atomic_test_and_set(z_atomic_semaphore *s);

extern void z_atomic_reset(z_atomic_semaphore *s);



#endif
