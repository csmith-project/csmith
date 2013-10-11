#!/bin/bash

res=$(grep -c 'compiler error' out.txt)
if [ $res -gt 0 ]
then
    res=$(grep -c 'definition in block' out.txt)
    if [ $res -eq 0 ]
    then
	FN=`hexdump -n 16 -v -e '/1 "%02X"' /dev/urandom`
	cp small.c /mnt/local/randomtest/tmp/$FN
    fi
fi

exit 0
