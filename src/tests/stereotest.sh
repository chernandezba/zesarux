#!/usr/bin/env bash


for DRIVER in pulse alsa sdl dsp null; do 

echo "Driver $DRIVER"
sleep 5

./zesarux --noconfigfile extras/snap_tests/teststereo.tap --verbose 2 --ao $DRIVER --zoom 1 --machine 128k --audiovolume 50


done
