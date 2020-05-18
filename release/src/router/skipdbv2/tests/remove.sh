#!/bin/sh 

COUNTER=0

V="your are the best"
K="hello"
while [  $COUNTER -lt 2000 ]; do
    ../build/bin/dbus remove $K$COUNTER
    let COUNTER=COUNTER+1 
done
