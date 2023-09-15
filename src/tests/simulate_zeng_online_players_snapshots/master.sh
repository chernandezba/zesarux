#!/bin/bash

REMOTESERVER=51.83.33.13
PAUSA=0.5

echo "Activating"
( sleep 1 ; echo "zo enable" ; sleep 1 )|telnet $REMOTESERVER 10000
echo "Creating room"
CREATOR_PASS=`( sleep 1 ; echo "zo create-room 0 pruebas" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`
echo "Creator pass: $CREATOR_PASS"

while true; do
	echo "Getting snapshot from this ZEsarUX"
	SNAPSHOT_DATA=`( sleep $PAUSA ; echo "get-snapshot" ; sleep 1 )|telnet localhost 10000|grep "command>"|head -1|awk '{printf $2}'`

	echo "Snapshot data: $SNAPSHOT_DATA"

	echo "Putting snapshot to remote"
	( sleep $PAUSA ; echo "zo put-snapshot $CREATOR_PASS 0 $SNAPSHOT_DATA" ; sleep 1 )|telnet $REMOTESERVER 10000
	#( sleep $PAUSA ; echo "zo put-snapshot $CREATOR_PASS 0" ; sleep 1 )|telnet $REMOTESERVER 10000
	#( sleep $PAUSA ; echo "zo put-snapshot" ; sleep 1 )|telnet $REMOTESERVER 10000
done


