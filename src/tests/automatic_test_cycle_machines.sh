#!/usr/bin/env bash

MAQUINAS=`./zesarux --machinelist`

for i in $MAQUINAS; do

echo Maquina: $i

./zesarux --noconfigfile --exit-after 10 --quickexit --machine $i

done
