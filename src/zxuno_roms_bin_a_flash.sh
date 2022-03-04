#!/bin/sh

#generar un archivo de 1 MB, metiendo roms_a8000.bin en el offset A8000H y el resto a ceros

#a8000h = 688128

OUTPUTFILE=zxuno.flash

TEMPFILE=`mktemp`

#ceros al principio
dd if=/dev/zero of=$TEMPFILE bs=1 count=688128

#agregar roms
cat ../zxuno/roms_a8000.bin >> $TEMPFILE

#completar mas ceros - 1 MB entero por si acaso

ZEROFILE=`mktemp`
dd if=/dev/zero of=$ZEROFILE bs=1M count=1
cat $ZEROFILE >> $TEMPFILE

#y generar archivo final de 1 MB
dd if=$TEMPFILE of=$OUTPUTFILE bs=1M count=1


