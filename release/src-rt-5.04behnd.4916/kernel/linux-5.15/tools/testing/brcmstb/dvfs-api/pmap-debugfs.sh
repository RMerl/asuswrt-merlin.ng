#!/bin/bash

ret=0
cd /sys/kernel/debug/brcm-scmi/pmap_cores || exit 1


for c in $(ls) ; do
    freqs=$(tr '\n' ' ' < $c/all_freqs)
    n=$(cat $c/num_pstates)
    echo "===========" $c "==========="
    echo "num_pstates ...... $n"
    echo "cur_pstate ....... $(cat $c/cur_pstate)"
    echo "cur_freq ......... $(cat $c/cur_freq)"
    echo "all_freqs ........ $freqs"
    cur_ps=$(cat $c/cur_pstate)
    for p in $(seq 0 $(($n - 1))) ; do
	echo $p > $c/cur_pstate
	new_ps=$(cat $c/cur_pstate)
	nf=$(cat $c/cur_freq)
	echo "state P$cur_ps -> P$p ... P$new_ps ($nf)"
	if [ "$p" != "$new_ps" ] ; then
	    echo "   ERROR: FAILED FAILED FAILED FAILED"
	    ret=1
	fi
	cur_ps=$new_ps
    done
    echo
done

exit $ret
