#!/usr/bin/env bash


MAQUINAS="MK14 ZX80 ZX81 16k 48k 48kp TC2048 128k QL P2 P2F P2S P2A40 P2A41 P2AS P340 P341 P3S TS2068 Inves 48ks 128ks TK90X TK90XS TK95 Z88 Sam Pentagon Chloe140 Chloe280 Chrome Prism ZXUNO BaseConf TSConf TBBlue ACE CPC464 CPC4128"

for i in $MAQUINAS; do

echo Maquina: $i

./zesarux --noconfigfile --exit-after 10 --machine $i

done
