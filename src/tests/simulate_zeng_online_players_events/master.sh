#!/bin/bash

REMOTESERVER=localhost


echo "Activating"
( sleep 1 ; echo "zo enable" ; sleep 1 )|telnet $REMOTESERVER 10000
echo "Creating room"
CREATOR_PASS=`( sleep 1 ; echo "zo create-room 0 pruebas" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`
echo "Creator pass: $CREATOR_PASS"

echo "Joining room"
USER_PASS=`( sleep 1 ; echo "zo join 0" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`
echo "User pass: $USER_PASS"

TECLA=1
while true; do
	echo "Putting event"
	( sleep 1 ; echo "zo send-keys $USER_PASS 0 CUALQUIERUUID $TECLA 1 0" ; sleep 1 )|telnet $REMOTESERVER 10000

	TECLA=$(($TECLA+1))
done


