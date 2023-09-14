#!/bin/bash

REMOTESERVER=51.83.33.13


echo "Joining room"
USER_PASS=`( sleep 1 ; echo "zo join 0" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`
echo "User pass: $USER_PASS"


SNAPSHOT_DATA="00112233445566778899AABBCCDDEEFF"

echo "Getting remote snapshot"
RETURNED_SNAPSHOT_DATA=`( sleep 1 ; echo "zo get-snapshot $USER_PASS 0" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`

echo "Putting local snapshot"
( sleep 1 ; echo "put-snapshot $SNAPSHOT_DATA" ; sleep 1 )|telnet localhost 10000


