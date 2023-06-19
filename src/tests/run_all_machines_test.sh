#!/usr/bin/env bash

MAQUINAS=`./zesarux --machinelist`

MAQUINAS="tbblue prueba zx80"

# comprobar solamente que ZEsarUX no salga con codigo error
for i in $MAQUINAS; do

	echo "Test maquina $i"

	./zesarux --noconfigfile --quickexit --exit-after 1 --machine $i --vo stdout

	RETURNCODE=$?

	if [ "$RETURNCODE" != 0 ]; then
		echo "Error running machine"
		exit 1
	fi
done

exit 0
