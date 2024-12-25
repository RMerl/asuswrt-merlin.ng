#!/bin/sh
INSTANCE=$1
PROTO=$2
METHOD=$3

http_servers="api.ipify.org ident.me"
stun_servers="stun.l.google.com:19302 stun.cloudflare.com:3478"

if [ "$PROTO" == "wireguard" ] ; then
	IFACE="wgc$INSTANCE"
else
	IFACE="tun1$INSTANCE"
fi


if [ "$METHOD" == "stun" ] ; then
	for server in $stun_servers; do
		result=$(/usr/sbin/ministun -t 5000 -c 1 -i $IFACE $server 2>/dev/null)
		[ $? -eq 0 ] && break
		result="unknown"
	done
else
	for server in $http_servers; do
		result=$(/usr/sbin/curl -4 -m 5 -sf --interface $IFACE $server 2>/dev/null)
		[ $? -eq 0 ] && break
		result="unknown"
	done
fi


if [ "$PROTO" == "wireguard" ] ; then
	nvram set "wgc${INSTANCE}_rip"=$result
else
	nvram set "vpn_client${INSTANCE}_rip"=$result
fi
