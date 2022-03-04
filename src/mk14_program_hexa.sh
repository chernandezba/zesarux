#!/usr/bin/env bash

#./splithexaspaces.sh

#:180F440094EDC400CF01C0BB35C0B93190CEC40F35C41431C40F36C4EA
#123456789012345678901234567890123456789012345678901234567890

while read LINEA; do

BINARIO=`echo -n $LINEA | cut -c 10-57`

DIRECCION=`echo -n $LINEA | cut -c 4-7`

echo -n "wmm ${DIRECCION}H "
./splithexaspaces.sh $BINARIO

echo

done 
