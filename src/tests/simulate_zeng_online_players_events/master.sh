#!/bin/bash

REMOTESERVER=localhost

CREATEROOM=1


if [ $# == 1 ]; then
	if [ "$1" == "nocreate" ]; then
		CREATEROOM=0
	fi
fi


if [ $CREATEROOM -eq 1 ]; then
	echo "Activating"
	( sleep 1 ; echo "zo enable" ; sleep 1 )|telnet $REMOTESERVER 10000
	echo "Creating room"
	CREATOR_PASS=`( sleep 1 ; echo "zo create-room 0 pruebas" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`
	echo "Creator pass: $CREATOR_PASS"
fi

echo "Joining room"
USER_PASS=`( sleep 1 ; echo "zo join 0" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`
echo "User pass: $USER_PASS"

TECLA=1
while true; do
	echo "Putting event. Tecla $TECLA"
	# enviamos pulsacion y liberacion al momento
	( sleep 1 ; echo "zo send-keys $USER_PASS 0 CUALQUIERUUID $TECLA 1 0" ; sleep 0.2 ; echo "zo send-keys $USER_PASS 0 CUALQUIERUUID $TECLA 0 0" ; sleep 0.2 )|telnet $REMOTESERVER 10000

	TECLA=$(($TECLA+1))
done


