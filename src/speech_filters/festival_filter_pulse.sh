#!/usr/bin/env bash

#Play festival using pulse device

WAVEFILE=`mktemp`
cat | text2wave -o $WAVEFILE
paplay $WAVEFILE >/dev/null 2>&1
rm -f $WAVEFILE

