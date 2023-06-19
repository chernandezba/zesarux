#!/usr/bin/env bash

MAQUINAS=`./zesarux --machinelist`

TEMPFILE=`mktemp`

# comprobar que ZEsarUX no finalice con codigo error, o que de error al cargar rom
for i in $MAQUINAS; do

	echo "Test maquina $i"

	TEXTORETORNO=`./zesarux --noconfigfile --quickexit --exit-after 1 --machine $i --vo null --ao null > $TEMPFILE 2>&1`

	RETURNCODE=$?

	if [ "$RETURNCODE" != 0 ]; then
		echo "Error running machine"
		exit 1
	fi

	grep "Error loading ROM" $TEMPFILE
	A=$?

	grep "Unable to open rom" $TEMPFILE
	B=$?

	if [ $A == 0 ] || [ $B == 0 ]; then
		echo "Error loading rom from machine $i"
		exit 1
	fi
done

exit 0
