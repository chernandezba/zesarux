#!/usr/bin/env bash

# Script to test load but also print char traps for printing ascii

TEMPFILE=`mktemp`

MAQUINAS="ZX80 TK80 TK82 ZX81 TS1000 TS1500 TK82C TK83 TK85 16k 48k 48kp 128k P2 P2F P2S P2A40 P2A41 P2AS TC2048 TC2068 TS2068 Inves 48ks 128ks TK90X TK90XS TK95 TK95S Pentagon ZXUNO CZ1000 CZ1500 CZ1000p CZ1500p CZ2000 CZSPEC CZSPECp"

for i in $MAQUINAS; do

	echo "Machine $i"
	if [ "$i" == "ZX81" ] || [ "$i" == "TS1000" ] || [ "$i" == "TS1500" ] || [ "$i" == "TK82C" ] || [ "$i" == "TK83" ] || [ "$i" == "TK85" ] || [ "$i" == "CZ1000" ] || [ "$i" == "CZ1500" ] || [ "$i" == "CZ1000p" ] || [ "$i" == "CZ1500p" ]; then
		./zesarux --noconfigfile --ao null --vo stdout tests/printtrap_test.p --exit-after 5 --machine $i --cpuspeed 400 > $TEMPFILE
	elif [ "$i" == "ZX80" ] || [ "$i" == "TK80" ] || [ "$i" == "TK82" ]; then
		./zesarux --noconfigfile --ao null --vo stdout tests/printtrap_test.zsf --exit-after 3 --machine $i --cpuspeed 400 > $TEMPFILE
	else
		./zesarux --noconfigfile --ao null --vo stdout tests/printtrap_test.tap --hardware-debug-ports --exit-after 10 --machine $i --fastautoload > $TEMPFILE
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


# Also test loading +3 Disk
echo "Test Load +3 Disk"
./zesarux --noconfigfile --ao null --vo stdout --machine p340 tests/testdsk.dsk --exit-after 10 --fastautoload > $TEMPFILE

grep "RUN PROGRAM OK" $TEMPFILE

if [ $? != 0 ]; then
	echo "ERROR Load +3 Disk"
	exit 1
else
	echo "OK Load +3 Disk"
fi

echo


# Also test loading microdrive
echo "Test Load Standard Microdrive MDR"
./zesarux --noconfigfile --ao null --vo stdout --tape tests/testmicrodrive.tap --zx-mdv-file 1 tests/testmicrodrive.mdr --zx-mdv-enable 1 --zx-mdv-no-persistent-writes 1 --enable-interface1 --exit-after 10 --fastautoload > $TEMPFILE

grep "RUN PROGRAM OK" $TEMPFILE

if [ $? != 0 ]; then
        echo "ERROR Load Standard Microdrive MDR"
        exit 1
else
        echo "OK Load Standard Microdrive MDR"
fi

echo

echo "Test Load RAW Microdrive RMD"
./zesarux --noconfigfile --ao null --vo stdout --tape tests/testmicrodrive.tap --zx-mdv-file 1 tests/testmicrodrive.rmd --zx-mdv-enable 1 --zx-mdv-no-persistent-writes 1 --enable-interface1 --exit-after 10 --fastautoload > $TEMPFILE

grep "RUN PROGRAM OK" $TEMPFILE

if [ $? != 0 ]; then
        echo "ERROR Load RAW Microdrive RMD"
        exit 1
else
        echo "OK Load RAW Microdrive RMD"
fi
