#!/bin/sh
INSTANCE=$1
PROTO=$2

servers="stun.l.google.com:19302 stun.stunprotocol.org"

if [ "$PROTO" == "wireguard" ] ; then
	IFACE="wgc$INSTANCE"
else
	IFACE="tun1$INSTANCE"
fi


for server in $servers; do
	result=$(/usr/sbin/ministun -t 5000 -c 1 -i $IFACE $server 2>/dev/null)
	[ $? -eq 0 ] && break
	result="unknown"
done

if [ "$PROTO" == "wireguard" ] ; then
	nvram set "wgc${INSTANCE}_rip"=$result
else
	nvram set "vpn_client${INSTANCE}_rip"=$result
fi
