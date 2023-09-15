#!/bin/bash

#this local ZEsarUX on port 10001
#./zesarux --noconfigfile --enable-remoteprotocol --remoteprotocol-port 10001


REMOTESERVER=51.83.33.13
PAUSA=0.5

echo "Joining room"
USER_PASS=`( sleep 1 ; echo "zo join 0" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`
echo "User pass: $USER_PASS"


while true; do
	echo "Getting remote snapshot"
	RETURNED_SNAPSHOT_DATA=`( sleep $PAUSA ; echo "zo get-snapshot $USER_PASS 0" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`

	echo "Received snapshot: $RETURNED_SNAPSHOT_DATA"

	echo "Putting local snapshot"
	( sleep $PAUSA ; echo "put-snapshot $RETURNED_SNAPSHOT_DATA" ; sleep 1 )|telnet localhost 10001

done
