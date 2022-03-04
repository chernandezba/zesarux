#!/usr/bin/env bash

#Play festival using alsa device

WAVEFILE=`mktemp`
cat | text2wave -o $WAVEFILE
aplay $WAVEFILE >/dev/null 2>&1
rm -f $WAVEFILE
