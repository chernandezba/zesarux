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
   Main and initial ZEsarUX functions
*/


#include <stdlib.h>
#include <stdio.h>
#ifndef MINGW
	#include <unistd.h>
#endif
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include <signal.h>

#ifdef MINGW
	//Para llamar a FreeConsole
	#include <windows.h>
#endif


#if defined(__APPLE__)
	//Para _NSGetExecutablePath
	#include <mach-o/dyld.h>
#endif


#include "cpu.h"
