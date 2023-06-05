#!/bin/bash

echo "Invocando api con texto: $1"
./text_to_image.py "$1"
echo "Convirtiendo a bmp"
convert created_image.png -colors 256 -compress none created_image.bmp

#cp created_image.bmp ../zesarux/src/keyboard_48.bmp
