#!/usr/bin/env bash

#probar a comprimir y descomprimir en memoria unos cuantos diferentes tipos de archivos, a ver si la rutina de ZSF funciona bien
mkdir /tmp/pruebas
cp *.c /tmp/pruebas
cp *.rom /tmp/pruebas

for i in /tmp/pruebas/*; do
	echo "Running compress/uncompress test for $i"
	./zesarux --codetests $i
	if [ $? != 0 ]; then
		echo "ERROR"
		exit 1
	fi
done
