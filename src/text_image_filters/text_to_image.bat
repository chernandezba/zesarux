echo off
rem requires previous:
rem set OPENAI_API_KEY=sk-......

echo "Invocando api con texto: " 
echo %1

rem dado que la ruta esta en subcarpeta, obtener con % ~dp0
python %~dp0\aux_text_to_image.py %1
echo "Convirtiendo a bmp"
magick output_aventure_location_image.png -colors 256 -compress none output_aventure_location_image.bmp

rem para avisar que hay nueva imagen
echo "" > output_aventure_location_image.new


