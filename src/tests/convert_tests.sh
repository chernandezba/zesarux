#!/usr/bin/env bash

#Convertir tap-tzx-tap y tap-pzx-tap y ver que los tap finales son iguales que los originales

OPTIONS="--noconfigfile"

ORIGFILE=./my_soft/spectrum/vintage/sped52.tap
PZXFILE=`mktemp`.pzx
TZXFILE=`mktemp`.tzx
TAPFILE=`mktemp`.tap

SISTEMA=`uname -o 2>/dev/null`
if [ $? != 0 ]; then
        #para mac os x
        SISTEMA=`uname -s 2>/dev/null`
fi

if [ "$SISTEMA" == "Darwin" ]; then
	MD5TOOL="md5 -q"
else
	MD5TOOL="md5sum"
fi


SUMAORIG=`$MD5TOOL $ORIGFILE|awk '{printf $1}'`


#TAP-TZX-TAP
./zesarux $OPTIONS --convert-tap-tzx $ORIGFILE $TZXFILE
if [ $? != 0 ]; then
	exit 1
fi
./zesarux $OPTIONS --convert-tzx-tap $TZXFILE  $TAPFILE
if [ $? != 0 ]; then
	exit 1
fi
SUMATAP=`$MD5TOOL $TAPFILE|awk '{printf $1}'`

if [ "$SUMAORIG" != "$SUMATAP" ]; then
	echo "Error converting tap-tzx-tap"
	exit 1
else
	echo "## Convert tap-tzx-tap ok"
fi

#TAP-PZX-TAP
./zesarux $OPTIONS --convert-tap-pzx $ORIGFILE $PZXFILE
if [ $? != 0 ]; then
	exit 1
fi
./zesarux $OPTIONS --convert-pzx-tap $PZXFILE  $TAPFILE
if [ $? != 0 ]; then
	exit 1
fi
SUMATAP=`$MD5TOOL $TAPFILE|awk '{printf $1}'`

if [ "$SUMAORIG" != "$SUMATAP" ]; then
	echo "Error converting tap-pzx-tap"
	exit 1
else
	echo "## Convert tap-pzx-tap ok"
fi

