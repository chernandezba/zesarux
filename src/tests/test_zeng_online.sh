#!/bin/bash

# put y get snapshot

./zesarux --noconfigfile --enable-remoteprotocol --vo stdout --ao null --exit-after 30 &
sleep 5

echo "Activating"
( sleep 1 ; echo "zo enable" ; sleep 1 )|telnet localhost 10000
echo "Creating room"
CREATOR_PASS=`( sleep 1 ; echo "zo create-room 0 pruebas" ; sleep 1 )|telnet localhost 10000|grep "command>"|head -1|awk '{printf $2}'`
echo "Creator pass: $CREATOR_PASS"

echo "Joining room"
USER_PASS=`( sleep 1 ; echo "zo join 0" ; sleep 1 )|telnet localhost 10000|grep "command>"|head -1|awk '{printf $2}'`
echo "User pass: $USER_PASS"


SNAPSHOT_DATA="00112233445566778899AABBCCDDEEFF"

echo "Putting snapshot"
( sleep 1 ; echo "zo put-snapshot $CREATOR_PASS 0 $SNAPSHOT_DATA" ; sleep 1 )|telnet localhost 10000

echo "Getting snapshot"
RETURNED_SNAPSHOT_DATA=`( sleep 1 ; echo "zo get-snapshot $USER_PASS 0" ; sleep 1 )|telnet localhost 10000|grep "command>"|head -1|awk '{printf $2}'`

echo "Original snapshot data: $SNAPSHOT_DATA"
echo "Returned snapshot data: $RETURNED_SNAPSHOT_DATA"
if [ "$SNAPSHOT_DATA" != "$RETURNED_SNAPSHOT_DATA" ]; then
	echo "They are different. Error!"
	exit 1
else
	echo "They are the same. OK"
	exit 0
fi

