#!/bin/sh
. lib/generic.sh

DEV="$(rand_dev)"
ts_ip "$0" "Add $DEV dummy interface" link add dev $DEV type dummy
ts_ip "$0" "Enable $DEV" link set $DEV up
ts_tc "$0" "Add ingress qdisc" qdisc add dev $DEV clsact

TMP="$(mktemp)"
echo filt add dev $DEV ingress pref 1000 matchall action pass >> "$TMP"
echo filt add dev $DEV ingress pref 1000 matchall action pass >> "$TMP"

"$TC" -b "$TMP" 2> $STD_ERR > $STD_OUT
if [ $? -eq 0 ]; then
	ts_err "$0: batch passed when it should have failed"
elif [ ! -s $STD_ERR ]; then
	ts_err "$0: batch produced no error message"
else
	echo "$0: batch failed, as expected"
fi

rm "$TMP"
ts_ip "$0" "Del $DEV dummy interface" link del dev $DEV
