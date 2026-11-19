#!/usr/bin/env bash

#tool to run some different tests to ZEsarUX
#tests are Speccy programs, in tap format, which should return text "RESULT: OK" or "RESULT: ERROR", and then exit the emulator by doing a exit emulator action,
#by using ZEsarUX ZXI hardware debug ports

run_test()
{
	TEMPFILE=`mktemp`
	echo Running $@
	$@ > $TEMPFILE
	RESULTADO=`cat $TEMPFILE|grep RESULT|grep OK`
	if [ $? == 0 ]; then
		echo "Result OK"
		rm -f $TEMPFILE
	else
		echo "!!!!!!!!!!!!!!!!!!!!"
		echo "!!!!!!!!!!!!!!!!!!!!"
		echo "ERROR"
		echo "!!!!!!!!!!!!!!!!!!!!"
		echo "!!!!!!!!!!!!!!!!!!!!"
		echo
		sleep 5
		echo "Output:"
		echo
		cat $TEMPFILE
		rm -f $TEMPFILE
		exit 1
	fi

}

echo "Didaktik Gama MMU test."
#no uso --hardware-debug-ports para salir del programa basic porque la rom del didaktik gama no envia sentencias OUT reales, 
#su rutina de BASIC OUT pone el valor enviado en el byte alto del puerto. Así, OUT 53051,3 acaba escribiendo en 033Bh, y OUT 57147,1 en 013Bh.
#el test tarda 25 segundos, con 30 de maximo tiene suficiente

run_test ./zesarux --noconfigfile --ao null --machine Didaktik --exit-after 30 --vo stdout tests/testramdidaktik.tap




