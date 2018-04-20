#!/bin/bash

FILE=$1
TMP=/tmp/$2.wcnf

trap 'rm -f "$TMP $TMP.result"' EXIT

./pbo2maxsat $FILE > $TMP
./msuncore-20130422-linux64 $TMP > $TMP.result

C=$(head -n1 $TMP | awk '{print $4}')

if [ "$C" == "offset" ]; then
        OFFSET=$(head -n1 $TMP | awk '{print $5}')
        O=$(grep "^o " $TMP.result | tail -n1 | awk '{print $2}')
        if [ "$O" == "" ]; then
                cat $TMP.result
        else
                echo o $((O+OFFSET))
                grep "^v " $TMP.result
        fi
else
        cat $TMP.result
fi

rm -f $TMP
