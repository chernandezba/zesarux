#!/usr/bin/env bash

MAQUINAS=`./zesarux --machinelist`

for i in $MAQUINAS; do

echo Maquina: $i

if [ "$i" == "QL" ] || [ "$i" == "Sam" ] || [ "$i" == "Prism" ] || [ "$i" == "TSConf" ] || [ "$i" == "TBBlue" ] || [ "$i" == "CPC464" ] || [ "$i" == "CPC4128" ] || [ "$i" == "CPC664" ] || [ "$i" == "CPC6128" ] || [ "$i" == "PCW8256" ] || [ "$i" == "PCW8512" ] || [ "$i" == "SAM" ]; then
	ZOOM=1
else
	ZOOM=2
fi

if [ "$i" == "Chloe280" ]; then
	./zesarux --noconfigfile --machine chloe280 --mmc-file extras/media/spectrum/chloe/chloehd.mmc --enable-mmc --enable-divmmc --nowelcomemessage --mmc-no-persistent-writes --quickexit --exit-after 10
else
	./zesarux --noconfigfile --quickexit --machine $i --zoom $ZOOM --exit-after 10
fi

if [ $? != 0 ]; then
	echo "Error running machine $i"
	exit 1
fi

done
