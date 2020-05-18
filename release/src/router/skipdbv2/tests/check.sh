#!/bin/sh 

COUNTER=0
V="your are the best"
K="hello"
eval `../build/bin/dbus export hello`
while [  $COUNTER -lt 10000 ]; do
    eval "KV=$"$K$COUNTER
    if [ "$KV" != "$V$COUNTER" ]; then
        echo "$KV $V$COUNTER"
    fi
    let COUNTER=COUNTER+1 
done
