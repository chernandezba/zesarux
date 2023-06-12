#!/bin/bash

# just a dummy script to test image generation

echo
echo "Calling Dummy script conversion from text to image, using text: $1"
echo "------"

# just a sleep
sleep 5

# just any image
cp keyboard_48.bmp output_aventure_location_image.bmp 

# Tell ZEsarUX there is a new image
touch output_aventure_location_image.new

