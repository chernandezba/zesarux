#!/bin/bash

#Requires imagemagick, python. See file dalle_aux_text_to_image.py for python dependencies

echo
echo "Calling Dall-E api using text: $1"

if [ "$OPENAI_API_KEY" == "" ]; then
	echo "Error: OpenAI API Key is not defined. You must export that setting before running ZEsarUX"
	exit 1
fi

# Run the python script which is on the same path as this one
MYDIR=`dirname $0`

$MYDIR/dalle_aux_text_to_image.py "$1"

#echo "Convert to bmp, 256 indexed colors, uncompressed"
convert output_aventure_location_image.png -colors 256 -compress none output_aventure_location_image.bmp

# Tell ZEsarUX there is a new image
touch output_aventure_location_image.new

