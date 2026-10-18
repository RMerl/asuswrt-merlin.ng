#!/bin/bash

function recover_num_pstates {
	local X=$(dmesg | egrep 'NUM_PSTATES:' | tail -1)
	X=$(echo $X | grep PASS | sed -e 's/.*PASS NUM_PSTATES: //')
	if [ "$X" = "" ] ; then echo 0 ; else echo $X ; fi
}


cores=(v3d hvd raaga vice xpt m2mc mipmap tsx sc)


for c in ${cores[@]}; do
    echo
    echo "==== $c ===="
    insmod num_pstates.ko core=$c 2>/dev/null
    X=$(recover_num_pstates)
    X=$(($X - 1))
    Y=$(($X - 1))
    sleep 1;
    for i in $(seq $Y -1 0) $(seq 1 $X) ; do
	insmod set_pstate.ko core=$c target=$i 2>/dev/null
    done
done
