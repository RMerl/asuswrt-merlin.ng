#!/bin/sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:${PATH}

get()
{
	value=`ip neigh | grep REACHABLE`
	proc="/proc/ark/ghost"

	echo "${value}" |
	{
		while read line
		do
			update_arg=`echo $line | awk '{print "update",$1,$5,$3}'`
			[ -f "${proc}" ] && echo "${update_arg}" > "${proc}"
		done
	}
}

get_lookup()
{
	if [ -z "$1" ]; then
		return;
	fi

	local lockf="/tmp/.$1"
	if [ -f "${lockf}" ]; then
			return;
	fi

	touch ${lockf}

	proc="/proc/ark/ss"

	addrs=$(nslookup $1 | sed '1,3d' | grep -i "^Address" | awk '{ print $2; }')
	for addr in ${addrs}
	do
		[ -f "${proc}" ] && echo "add-cache $1 ${addr} 60" > "${proc}"
	done

	rm -f ${lockf}
}

get_ping()
{
	if [ -z "$1" ]; then
		return;
	fi

	local lockf="/tmp/.$1"
	if [ -f "${lockf}" ]; then
			return;
	fi

	touch ${lockf}

	ping $1 -q -c 1
	
	rm -f ${lockf}
}

get_neigh()
{
	set -f
	value=`cat /proc/net/arp`
	proc="/proc/ark/netip"

	echo "${value}" |
	{
		while read line
		do
			update_arg=`echo $line | awk '{if ($3 != "0x0") print "set",$1,$4,$6}'`
			[ -f "${proc}" ] && echo "${update_arg}" > "${proc}"
		done
	}
	set +f
}

get_neigh_v6()
{
    set -f
    value=`ip -6 neighbor show`
	proc="/proc/ark/netip"

    echo "${value}" |
    {
        while read line
        do
            update_arg=`echo ${line} | awk '{ print "set",$1,$5,$3}'`
			[ -f "${proc}" ] && echo "${update_arg}" > "${proc}"
        done
    }

    set +f
}


get_neigh2()
{
	set -f
	value=`cat /proc/net/arp`
	proc="/proc/ark/neighbor_ctrl"

	echo "${value}" |
	{
		while read line
		do
			update_arg=`echo $line | awk '{if ($3 != "0x0") print "add",$1,$4,$6}'`
			[ -f "${proc}" ] && echo "${update_arg}" > "${proc}"
		done
	}
	set +f
}

flush_neigh()
{
	devup=$(cat /proc/ark/malnet_up)
	devdw=$(cat /proc/ark/malnet_dw)
	
	ip neigh flush dev ${devup}
	ip neigh flush dev ${devdw}
	
	get_neigh
}

case $1 in
	get)
		get
		;;
	get-neigh)
		get_neigh
        get_neigh_v6
		;;
	get-neigh2)
		get_neigh2
		;;
	get-lookup)
		get_lookup "$2"
		;;
	get-ping)
		get_ping "$2"
		;;
esac
