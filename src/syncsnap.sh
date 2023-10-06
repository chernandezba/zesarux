#!/usr/bin/env bash
#simple test to show snapshot copy from one instance of ZEsarUX to another

SOURCEHOST=localhost
DESTHOST=localhost

TEMPFILE=`mktemp`
( sleep 1 ; echo "get-snapshot " ; sleep 1 ) | telnet $SOURCEHOST 10000 > $TEMPFILE

PARSEDFILE=`mktemp`

cat $TEMPFILE|tail -2|head -1|sed 's/command> //' > $PARSEDFILE

( sleep 1 ; echo -n "put-snapshot " ; cat $PARSEDFILE ; echo ; sleep 1 ) | telnet $DESTHOST 10000

rm -f $TEMPFILE
rm -f $PARSEDFILE
