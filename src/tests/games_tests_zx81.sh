#!/usr/bin/env bash

OPTIONS="--noconfigfile --quickexit --nowelcomemessage --enable-zxdesktop"

while read LINE; do

echo "Test $LINE"
./zesarux $OPTIONS --machine zx81 "$LINE"

done < tests/lista_zx81

