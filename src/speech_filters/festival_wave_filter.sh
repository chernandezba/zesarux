#!/usr/bin/env bash

#Play festival using alsa and also record audio to wav files

WAVEFILE=/tmp/wave_`date +%s``date +%N`.wav
cat | text2wave -o $WAVEFILE
aplay $WAVEFILE >/dev/null 2>&1
