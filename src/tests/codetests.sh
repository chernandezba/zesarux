#!/usr/bin/env bash

#probar a comprimir y descomprimir en memoria unos cuantos diferentes tipos de archivos, a ver si la rutina de ZSF funciona bien
mkdir /tmp/pruebas
cp *.c /tmp/pruebas
cp *.rom /tmp/pruebas

#generar diferentes archivos con bytes dd
echo -n -e '\xdd' > /tmp/pruebas/zzddfile1
echo -n -e '\xdd\xdd' > /tmp/pruebas/zzddfile2
echo -n -e '\xdd\xdd\xdd' > /tmp/pruebas/zzddfile3
echo -n -e '\xdd\xdd\xdd\xdd' > /tmp/pruebas/zzddfile4
echo -n -e '\xdd\xdd\xdd\xdd\xdd' > /tmp/pruebas/zzddfile5
echo -n -e '\xdd\xdd\xdd\xdd\xdd\xdd' > /tmp/pruebas/zzddfile6
echo -n -e '\xdd' > /tmp/pruebas/zzddfile11
echo -n -e '\xdd\xdd' > /tmp/pruebas/zzddfile12
echo -n -e '\xdd\xdd\xdd' > /tmp/pruebas/zzddfile13
echo -n -e '\xdd\xdd\xdd\xdd' > /tmp/pruebas/zzddfile14
echo -n -e '\xdd\xdd\xdd\xdd\xdd' > /tmp/pruebas/zzddfile15
echo -n -e '\xdd\xdd\xdd\xdd\xdd\xdd' > /tmp/pruebas/zzddfile16
echo -n -e '\xdd' > /tmp/pruebas/zzddfile21
echo -n -e '\xdd\xdd' > /tmp/pruebas/zzddfile22
echo -n -e '\xdd\xdd\xdd' > /tmp/pruebas/zzddfile23
echo -n -e '\xdd\xdd\xdd\xdd' > /tmp/pruebas/zzddfile24
echo -n -e '\xdd\xdd\xdd\xdd\xdd' > /tmp/pruebas/zzddfile25
echo -n -e '\xdd\xdd\xdd\xdd\xdd\xdd' > /tmp/pruebas/zzddfile26


dd if=/dev/zero of=/tmp/vacio bs=1 count=300
cat /tmp/vacio >> /tmp/pruebas/zzddfile11
cat /tmp/vacio >> /tmp/pruebas/zzddfile12
cat /tmp/vacio >> /tmp/pruebas/zzddfile13
cat /tmp/vacio >> /tmp/pruebas/zzddfile14
cat /tmp/vacio >> /tmp/pruebas/zzddfile15
cat /tmp/vacio >> /tmp/pruebas/zzddfile16

echo "Pruebas" >> /tmp/pruebas/zzddfile21
echo "Pruebas" >> /tmp/pruebas/zzddfile22
echo "Pruebas" >> /tmp/pruebas/zzddfile23
echo "Pruebas" >> /tmp/pruebas/zzddfile24
echo "Pruebas" >> /tmp/pruebas/zzddfile25
echo "Pruebas" >> /tmp/pruebas/zzddfile26

for i in /tmp/pruebas/*; do
	echo "Running compress/uncompress test for $i"
	./zesarux --codetests $i
	if [ $? != 0 ]; then
		echo "ERROR"
		exit 1
	fi
done
