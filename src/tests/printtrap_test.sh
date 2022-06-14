#!/bin/bash

# Script to test print char traps for printing numbers but also rest ascii chars

TEMPFILE=`mktemp`

MAQUINAS="16k 48k 128k P2 P2F P2S P2A40 P2A41 P2AS Inves 48ks 128ks TK90X TK90XS TK95 ZX80 ZX81"

for i in $MAQUINAS; do

	echo "Machine $i"
	if [ "$i" == "ZX81" ]; then
		./zesarux --noconfigfile --chardetectcompatnum --vo stdout tests/printtrap_test.p --exit-after 5 --machine ZX81 --cpuspeed 400 > $TEMPFILE
	elif [ "$i" == "ZX80" ]; then
		./zesarux --noconfigfile --vo stdout tests/printtrap_test.zsf --exit-after 3 --machine ZX80 --cpuspeed 400 > $TEMPFILE
	else
		./zesarux --noconfigfile --chardetectcompatnum --vo stdout tests/printtrap_test.tap --hardware-debug-ports --exit-after 5 --machine $i --fastautoload > $TEMPFILE
	fi

	grep 1234 $TEMPFILE
	if [ $? != 0 ]; then
		echo "ERROR Number"
	else
		echo "OK Number"
	fi

	grep HOLA $TEMPFILE
	if [ $? != 0 ]; then
		echo "ERROR Ascii"
	else
		echo "OK Ascii"
	fi

	echo

done

