#!/usr/bin/env bash


for DRIVER in stdout simpletext curses cursesw aa caca cocoa xwindows sdl fbdev null; do 

echo "Driver $DRIVER"
sleep 2

./zesarux --noconfigfile --verbose 3 --vo $DRIVER


done
