#!/usr/bin/env bash


for DRIVER in coreaudio pulse alsa sdl dsp null; do 

echo "Driver $DRIVER"
sleep 5

./zesarux --noconfigfile --enable-silencedetector --verbose 3 --machine 128k extras/media/spectrum/music.tap --ao $DRIVER
./zesarux --noconfigfile --enable-silencedetector --verbose 3 --machine 128k extras/media/spectrum/beeper/Digi_Pop.tap --ao $DRIVER

echo "Testear silencio. Primero en 128k, beeper silence"
echo "Luego pasar a 48k, saltaran los dos silence (porque no habra AY Chip)"
sleep 3

./zesarux --noconfigfile --enable-silencedetector --verbose 3 --machine 128k --ao $DRIVER
./zesarux --noconfigfile --enable-silencedetector --verbose 3 extras/media/zx81/aydemo.p --ao $DRIVER
./zesarux --noconfigfile --enable-silencedetector --verbose 3 extras/media/zx81/ORQUESTA.P --ao $DRIVER
./zesarux --noconfigfile --enable-silencedetector --verbose 3 extras/media/z88/lem.epr --ao $DRIVER
./zesarux --noconfigfile --enable-silencedetector --verbose 3 extras/snap_tests/test3200z88.zx --ao $DRIVER

done
