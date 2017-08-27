#!/bin/sh
#
# $Id: test_arp.sh,v 1.2 2006/03/31 12:36:08 tjaqua Exp $
#
export PATH=/tmp/sbin:/tmp/bin:/bin:/usr/bin:/sbin:/usr/sbin

IP=$1
#CMD=`ip neighbor show $IP`

arping -q -c 1 -I `nvram get lan_ifname` $IP
echo $?

#case "$CMD" in
#'') echo 0;;
#?*) echo 1;;
#esac

