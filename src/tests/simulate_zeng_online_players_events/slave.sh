#!/bin/bash

REMOTESERVER=localhost


echo "Joining room"
USER_PASS=`( sleep 1 ; echo "zo join 0" ; sleep 1 )|telnet $REMOTESERVER 10000|grep "command>"|head -1|awk '{printf $2}'`
echo "User pass: $USER_PASS"

echo "Getting events"
( sleep 1 ; echo "zo get-keys $USER_PASS 0" ; sleep 1000 )|telnet $REMOTESERVER 10000

