#!/bin/sh
INSTANCE=$1

servers="stun.l.google.com:19302 stun.stunprotocol.org"

for server in $servers; do
	result=$(/usr/sbin/ministun -t 5000 -c 1 -i tun1$INSTANCE $server 2>/dev/null)
	[ $? -eq 0 ] && break
	result="unknown"
done

nvram set "vpn_client${INSTANCE}_rip"=$result
