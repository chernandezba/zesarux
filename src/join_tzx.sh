#!/usr/bin/env bash

#join two tzx files on a new one

if [ $# != 2 ]; then
	echo "$0 [file1.tzx] [file2.tzx]"
	echo "Note output file is written on stdout"
	exit 1
fi

TEMPFILEA=`mktemp`
TEMPFILEB=`mktemp`


cp $1 $TEMPFILEA

#skip 10 bytes of header
dd if=$2 of=$TEMPFILEB bs=1 skip=10

cat $TEMPFILEB >> $TEMPFILEA

cat $TEMPFILEA

rm -f $TEMPFILEA
rm -f $TEMPFILEB


