#!/usr/bin/env bash

# Script to test print char traps for printing numbers but also rest ascii chars

TEMPFILE=`mktemp`

MAQUINAS="ZX80 TK80 TK82 ZX81 TS1000 TS1500 TK82C TK83 TK85 16k 48k 48kp 128k P2 P2F P2S P2A40 P2A41 P2AS TC2048 Inves 48ks 128ks TK90X TK90XS TK95 TK95S Pentagon"


for i in $MAQUINAS; do

	echo "Machine $i"
	if [ "$i" == "ZX81" ] || [ "$i" == "TS1000" ] || [ "$i" == "TS1500" ] || [ "$i" == "TK82C" ] || [ "$i" == "TK83" ] || [ "$i" == "TK85" ]; then
		./zesarux --noconfigfile --chardetectcompatnum --vo stdout tests/printtrap_test.p --exit-after 5 --machine $i --cpuspeed 400 > $TEMPFILE
	elif [ "$i" == "ZX80" ] || [ "$i" == "TK80" ] || [ "$i" == "TK82" ]; then
		./zesarux --noconfigfile --vo stdout tests/printtrap_test.zsf --exit-after 3 --machine $i --cpuspeed 400 > $TEMPFILE
	else
		./zesarux --noconfigfile --chardetectcompatnum --vo stdout tests/printtrap_test.tap --hardware-debug-ports --exit-after 5 --machine $i --fastautoload > $TEMPFILE
	fi

	grep 1234 $TEMPFILE
	if [ $? != 0 ]; then
		echo "ERROR Number"
		exit 1
	else
		echo "OK Number"
	fi

	grep HOLA $TEMPFILE
	if [ $? != 0 ]; then
		echo "ERROR Ascii"
		exit 1
	else
		echo "OK Ascii"
	fi

	echo

done

