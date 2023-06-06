
echo "Invocando api con texto: %1"
./text_to_image.py %1
echo "Convirtiendo a bmp"
convert output_aventure_location_image.png -colors 256 -compress none output_aventure_location_image.bmp

#para avisar que hay nueva imagen
echo "" > output_aventure_location_image.new

