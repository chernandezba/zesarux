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

#ifndef ZESARUX_H
#define ZESARUX_H


//
//Por el uso de PATH_MAX en diferentes sistemas operativos
//

//Para Linux
#include <dirent.h>

//Para Mac
#if defined(__APPLE__)
    #include <sys/syslimits.h>
#endif


//Para Windows
#ifdef MINGW
#include <stdlib.h>
#define PATH_MAX MAX_PATH
#define NAME_MAX MAX_PATH
#endif

//FreeBSD, Haiku (y quizá otros Unix) utilizan este
#include <limits.h>


//
//Fin uso de PATH_MAX en diferentes sistemas operativos
//



#endif
