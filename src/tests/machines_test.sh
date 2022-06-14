#!/usr/bin/env bash

MAQUINAS="MK14 ZX80 ZX81 16k 48k 128k QL P2 P2F P2S P2A40 P2A41 P2AS P340 P341 P3S TS2068 Inves 48ks 128ks TK90X TK90XS TK95 Z88 Sam Pentagon Chloe140 Chloe280 Chrome Prism ZXUNO BaseConf TSConf TBBlue ACE CPC464 CPC4128 MSX1 Coleco SG1000 SMS SVI318 SVI328"

for i in $MAQUINAS; do

	echo "Test maquina $i"

	if [ "$i" == "QL" ] || [ "$i" == "Sam" ] || [ "$i" == "Prism" ] || [ "$i" == "TSConf" ] || [ "$i" == "TBBlue" ] || [ "$i" == "CPC464" ] || [ "$i" == "CPC4128" ]; then
		ZOOM=1
	else
		ZOOM=2
	fi

	if [ "$i" == "Chloe280" ]; then
		./zesarux --noconfigfile --machine chloe280 --mmc-file extras/media/spectrum/chloe/chloehd.mmc --enable-mmc --enable-divmmc --nowelcomemessage --mmc-no-persistent-writes --quickexit
	else

		./zesarux --noconfigfile --quickexit --machine $i --zoom $ZOOM
	fi
done
