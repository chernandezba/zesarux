#!/bin/bash

./zesarux --codetests
if [ $? != 0 ]; then
	echo "Error codetests"
	exit 1
fi

tests/codetests.sh
if [ $? != 0 ]; then
	echo "Error codetests 2"
	exit 1
fi

tests/convert_tests.sh
if [ $? != 0 ]; then
        echo "Error convert tests"
        exit 1
fi

tests/run_all_machines_test.sh
if [ $? != 0 ]; then
        echo "Error running run all machines tests"
        exit 1
fi

tests/tbblue_mmu.sh
if [ $? != 0 ]; then
        echo "Error tbblue mmu tests"
        exit 1
fi

tests/printtrap_test.sh
if [ $? != 0 ]; then
        echo "Error printtrap tests"
        exit 1
fi

tests/load_test.sh
if [ $? != 0 ]; then
        echo "Error printtrap tests"
        exit 1
fi


echo "All tests succeeded"
exit 0
