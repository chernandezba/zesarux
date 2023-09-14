#!/bin/bash

REMOTESERVER=51.83.33.13


echo "Activating"
( sleep 1 ; echo "zo enable" ; sleep 1 )|telnet $REMOTESERVER 10000
echo "Creating room"
CREATOR_PASS=`( sleep 1 ; echo "zo create-room 0 pruebas" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`
echo "Creator pass: $CREATOR_PASS"

echo "Getting snapshot from this ZEsarUX"
SNAPSHOT_DATA=`( sleep 1 ; echo "zo get-snapshot $USER_PASS 0" ; sleep 1 )|telnet localhost 10000|grep "command>"|head -1|awk '{printf $2}'`

echo "Putting snapshot to remote"
( sleep 1 ; echo "zo put-snapshot $CREATOR_PASS 0 $SNAPSHOT_DATA" ; sleep 1 )|telnet $REMOTESERVER 10000


