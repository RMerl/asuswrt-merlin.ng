#!/bin/sh

cmd=`pspctl adump wlpreconfig`
if [ ! -z "${cmd##*"not found"*}" ]; then
    echo "wlan driver preconfig..."
    echo "preconfig cmd=$cmd"
    eval $cmd
fi
