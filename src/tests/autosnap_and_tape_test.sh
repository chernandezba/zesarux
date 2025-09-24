#!/usr/bin/env bash

# Script to test autoloading autosnap and inserting tape using --tape

TEMPFILE=`mktemp`


TEMPSNAP=`mktemp`
HAYSNAP=0

if [ -e zesarux_autosave.zsf ]; then
	HAYSNAP=1
fi

if [ $HAYSNAP == 1 ]; then
	# hago backup del anterior snap
	cp zesarux_autosave.zsf $TEMPSNAP  
fi

#meto mi autosnap preparado
cp tests/blank48.zsf zesarux_autosave.zsf

./zesarux --noconfigfile --ao null --autoloadsnap --vo stdout --hardware-debug-ports --exit-after 5 --machine 48k --fastautoload --tape tests/autosnap_and_tape_test.tap > $TEMPFILE


rm -f zesarux_autosave.zsf

if [ $HAYSNAP == 1 ]; then
	cp $TEMPSNAP zesarux_autosave.zsf
fi

grep HOLA $TEMPFILE
if [ $? != 0 ]; then
	echo "ERROR"
	exit 1
else
	echo "OK"
fi

echo


