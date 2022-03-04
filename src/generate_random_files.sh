#!/usr/bin/env bash

#    ZEsarUX  ZX Second-Emulator And Released for UniX
#    Copyright (C) 2013 Cesar Hernandez Bano
#
#    This file is part of ZEsarUX.
#
#    ZEsarUX is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Script to generate random files


while true; do


	NUMBER1=$RANDOM
	NUMBER2=$RANDOM

	SIZE=${NUMBER1}${NUMBER2}

	for LONGITUD in 4 5 6; do

		TAMANYO=`echo $SIZE|cut -b 1-$LONGITUD`

		for EXTENSION in tap tzx pzx trd dsk scr sna sp z80 p zsf; do

	
			TEMPNAME=random_$TAMANYO.$EXTENSION

			echo $TEMPNAME

			dd if=/dev/urandom of=$TEMPNAME bs=1 count=$TAMANYO
		done

	done

	sleep 0.1

done

