#!/usr/bin/env bash

#formato nombre
#sprite-6-33.bin
#entre guiones el ancho y el alto

#parametros:
#archivo numero_sprite nombre_sprite
ARCHIVO=$1
NUMERO_SPRITE=$2
NOMBRE_SPRITE=$3
ANCHO=`echo -n $ARCHIVO|cut -d '-' -f2`
ALTO=`echo -n $ARCHIVO|cut -d '-' -f3|cut -d '.' -f1`

#echo $ANCHO $ALTO

echo "//Sprite $NOMBRE_SPRITE"
echo "z80_byte easter_egg_sprite_$NUMERO_SPRITE[]={"

./bin_sprite_to_c $ARCHIVO $ANCHO $ALTO

echo "};"
