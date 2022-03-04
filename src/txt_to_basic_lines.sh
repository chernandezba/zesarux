#!/usr/bin/env bash

#tipos: 
#1. for 48k and zx81, rem written as token (key e)
#2. for zx80, rem written as token (key y)
#3. for 128k editor or z88, rem written as "REM"

#Agregar numero linea. parametro 3
#"si"

#Agregar texto "REM". parametro 4
#"si"

#Si pausa en cada linea, parametro 5
#"si"

if [ $# != 5 ]; then
	echo "$0 archivo tipo si_numlinea si_rem si_delayline"
	echo " tipo puede ser: 1: 48k, zx81. 2: zx80. 3: 128k, z88, o cualquier otro sistema que use 'REM' con caracteres y no tokens"
	exit 1
fi

ARCHIVO=$1
TIPO=$2
SI_NUMLINEA=$3
SI_REM=$4
SI_DELAYLINE=$5


#Initial enters 
echo 
echo

NUM=1

  LINE=""

  while [ 1 ]
  do
	#generate some delay before every line
	#disable it on Z88
    if [ "$SI_DELAYLINE" == "si" ]; then 
	echo -n '\\\\\\\\\\' 
    fi


    read LINE || break

   if [ "$SI_NUMLINEA" == "si" ]; then
	echo -n "${NUM} "
   fi

   if [ "$SI_REM" == "si" ]; then
	case $TIPO in
		1)
			echo -n "e"
		;;

		2)
			echo -n "y"
		;;

		3)
			echo -n "rem "
		;;
	esac
   fi

   echo "${LINE}"

    NUM=$((NUM+1))
  done < $ARCHIVO


