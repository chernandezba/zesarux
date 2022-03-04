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

echo "TBBlue MMU test."
#echo ' Write j"" +ENTER   after a few seconds'
#esto es debido a que en el modo --tbblue-fast-boot-mode  no hace autoload de la cinta

#run_test ./zesarux --noconfigfile --hardware-debug-ports --exit-after 60 --machine tbblue --vo stdout --mmc-file tbblue.mmc --enable-mmc --enable-divmmc-ports extras/media/spectrum/tbblue/testmmu.tap 
run_test ./zesarux --noconfigfile --tbblue-fast-boot-mode --hardware-debug-ports --exit-after 60 --machine tbblue --vo stdout extras/media/spectrum/tbblue/testmmu.tap 


echo "TBBlue MMU test 2 (rom space)"
#echo ' Write j"" +ENTER   after a few seconds'
#run_test ./zesarux --noconfigfile --hardware-debug-ports --exit-after 60 --machine tbblue --vo stdout --mmc-file tbblue.mmc --enable-mmc --enable-divmmc-ports extras/media/spectrum/tbblue/test_mmu_low/testing-mmu-low.tap
run_test ./zesarux --noconfigfile --tbblue-fast-boot-mode --hardware-debug-ports --exit-after 60 --machine tbblue --vo stdout extras/media/spectrum/tbblue/test_mmu_low/testing-mmu-low.tap

#RESULT: OK


