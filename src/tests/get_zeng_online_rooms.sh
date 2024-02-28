#!/bin/bash

REMOTESERVER=localhost


echo "Getting rooms"
( sleep 1 ; echo "zo list-rooms" ; sleep 2 )|telnet $REMOTESERVER 10000

