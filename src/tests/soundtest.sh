#!/usr/bin/env bash

ZESARUXCMD="./zesarux --noconfigfile --enable-silencedetector --verbose 3 --advancedmenus"

AUDIODRIVERS="coreaudio pulse alsa sdl dsp null"

for DRIVER in $AUDIODRIVERS; do 

echo "Driver $DRIVER"
sleep 5

$ZESARUXCMD --machine 128k extras/media/spectrum/music.tap --ao $DRIVER
$ZESARUXCMD --machine 128k extras/media/spectrum/beeper/Digi_Pop.tap --ao $DRIVER

echo "Testear silencio. Primero en 128k, beeper silence"
echo "Luego pasar a 48k, saltaran los dos silence (porque no habra AY Chip)"
sleep 3

$ZESARUXCMD --machine 128k --ao $DRIVER
$ZESARUXCMD extras/media/zx81/aydemo.p --ao $DRIVER
$ZESARUXCMD extras/media/zx81/ORQUESTA.P --ao $DRIVER
$ZESARUXCMD extras/media/z88/lem.epr --ao $DRIVER
$ZESARUXCMD extras/snap_tests/test3200z88.zx --ao $DRIVER

done
