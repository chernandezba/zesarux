#!/usr/bin/env bash


CONTADOR=0

while [ $CONTADOR -lt 16 ]; do
	INDICE=$(($CONTADOR*2))
	INDICE=$(($INDICE+1))

	SIGUIENTE=$(($INDICE+1))
	HEXA=`echo -n "$1"|cut -c ${INDICE}-${SIGUIENTE}`
	echo -n "${HEXA}H "

	CONTADOR=$(($CONTADOR+1))

done

echo

#00 00 00 c4 0d 35 c4 00 31 c4 01 c8 f4 c4 10 c8 a9
