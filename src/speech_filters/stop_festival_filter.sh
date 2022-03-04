#!/usr/bin/env bash

#echo "Deteniendo proceso speech"


killall festival > /dev/null 2>&1
killall /usr/lib/festival/audsp > /dev/null 2>&1
killall aplay > /dev/null 2>&1
#sleep 1
#ps aux
