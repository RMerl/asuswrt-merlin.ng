#!/bin/sh
# use STUN to find the external IP.

servers="default stun.l.google.com:19302 stun.iptel.org stun.stunprotocol.org stun.xten.com"
prefixes="wan0_ wan1_"

which ministun >/dev/null || exit 1

if [ "$(nvram get wans_mode)" = "lb" ] ; then
	primary="0"
	for prefix in $prefixes; do
		state=$(nvram get ${prefix}state_t)
		sbstate=$(nvram get ${prefix}sbstate_t)
		auxstate=$(nvram get ${prefix}auxstate_t)

		# is_wan_connect()
		[ "$state" = "2" ] || continue
		[ "$sbstate" = "0" ] || continue
		[ "$auxstate" = "0" -o "$auxstate" = "2" ] || continue

		# get_wan_ifname()
		proto=$(nvram get ${prefix}proto)
		if [ "$proto" = "pppoe" -o "$proto" = "pptp" -o "$proto" = "l2tp" ] ; then
			ifname=$(nvram get ${prefix}pppoe_ifname)
		else
			ifname=$(nvram get ${prefix}ifname)
		fi

		for server in $servers; do
			[ "$server" = "default" ] && server=
			result=$(ministun -t 1000 -c 1 -i $ifname $server 2>/dev/null)
			[ $? -eq 0 ] && break
			result=
		done
		[ -z "$result" ] && state=1 || state=2
		nvram set ${prefix}realip_state=$state
		nvram set ${prefix}realip_ip=$result

		wan=`echo $prefix|sed -e "s,_,,"`
		[ -z "$result" ] && echo "$wan failed." || echo "$wan external IP is $result."
	done
else
	for prefix in $prefixes; do
		primary=$(nvram get ${prefix}primary)
		[ "$primary" = "1" ] && break
	done

	[ "$primary" = "1" ] || exit 1

	# get_wan_ifname()
	proto=$(nvram get ${prefix}proto)
	if [ "$proto" = "pppoe" -o "$proto" = "pptp" -o "$proto" = "l2tp" ] ; then
		ifname=$(nvram get ${prefix}pppoe_ifname)
	else
		ifname=$(nvram get ${prefix}ifname)
	fi

	for server in $servers; do
		[ "$server" = "default" ] && server=
		result=$(ministun -t 1000 -c 1 -i $ifname $server 2>/dev/null)
		[ $? -eq 0 ] && break
		result=
	done
	[ -z "$result" ] && state=1 || state=2
	nvram set ${prefix}realip_state=$state
	nvram set ${prefix}realip_ip=$result

	[ -z "$result" ] && echo "Failed." || echo "External IP is $result."
fi
