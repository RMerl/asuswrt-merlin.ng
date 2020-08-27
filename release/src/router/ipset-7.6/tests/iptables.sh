#!/bin/sh

# set -x
set -e

ipset=${IPSET_BIN:-../src/ipset}

# We play with the following networks:
# inet: 10.255.255.0/24
# 	10.255.255.0-31 in ip1
# 	10.255.255.32-63 in ip2
# 	rest in ipport
# inet6: 1002:1002:1002:1002::/64
#	1002:1002:1002:1002::1 in ip1
#	1002:1002:1002:1002::32 in ip2
#	rest in ipport

case "$1" in
inet)
	cmd=iptables
	family=
	NET=10.255.255.0/24
	IP1=10.255.255.1
	IP2=10.255.255.32
	;;
inet6)
	cmd=ip6tables
	family="family inet6"
	NET=1002:1002:1002:1002::/64
	IP1=1002:1002:1002:1002::1
	IP2=1002:1002:1002:1002::32
	;;
*)
	echo "Usage: $0 inet|inet6 start|stop"
	exit 1
	;;
esac

case "$2" in
start)
	$ipset n ip1 hash:ip $family 2>/dev/null
	$ipset a ip1 $IP1 2>/dev/null
	$ipset n ip2 hash:ip $family 2>/dev/null
	$ipset a ip2 $IP2 2>/dev/null
	$ipset n ipport hash:ip,port $family 2>/dev/null
	$ipset n list list:set 2>/dev/null
	$ipset a list ipport 2>/dev/null
	$ipset a list ip1 2>/dev/null
	$cmd -A INPUT ! -s $NET -j ACCEPT
	$cmd -A INPUT -m set ! --match-set ip1 src \
		      -m set ! --match-set ip2 src \
		      -j SET --add-set ipport src,src
	$cmd -A INPUT -m set --match-set ip1 src \
		      -j LOG --log-prefix "in set ip1: "
	$cmd -A INPUT -m set --match-set ip2 src \
		      -j LOG --log-prefix "in set ip2: "
	$cmd -A INPUT -m set --match-set ipport src,src \
		      -j LOG --log-prefix "in set ipport: "
	$cmd -A INPUT -m set --match-set list src,src \
		      -j LOG --log-prefix "in set list: "
	$cmd -A OUTPUT -d $NET -j DROP
	cat /dev/null > .foo.err
	cat /dev/null > /var/log/kern.log
	;;
start_flags)
	$ipset n test hash:net $family 2>/dev/null
	$ipset a test 10.0.0.0/16 2>/dev/null
	$ipset a test 10.0.0.0/24 nomatch 2>/dev/null
	$ipset a test 10.0.0.1 2>/dev/null
	$cmd -A INPUT ! -s 10.0.0.0/16 -j ACCEPT
	$cmd -A INPUT -m set --match-set test src \
		      -j LOG --log-prefix "in set test: "
	$cmd -A INPUT -m set --match-set test src --return-nomatch \
		      -j LOG --log-prefix "in set test-nomatch: "
	$cmd -A INPUT -s 10.0.0.0/16 -j DROP
	cat /dev/null > .foo.err
	cat /dev/null > /var/log/kern.log
	;;
start_flags_reversed)
	$ipset n test hash:net $family 2>/dev/null
	$ipset a test 10.0.0.0/16 2>/dev/null
	$ipset a test 10.0.0.0/24 nomatch 2>/dev/null
	$ipset a test 10.0.0.1 2>/dev/null
	$cmd -A INPUT ! -s 10.0.0.0/16 -j ACCEPT
	$cmd -A INPUT -m set --match-set test src --return-nomatch \
		      -j LOG --log-prefix "in set test-nomatch: "
	$cmd -A INPUT -m set --match-set test src \
		      -j LOG --log-prefix "in set test: "
	$cmd -A INPUT -s 10.0.0.0/16 -j DROP
	cat /dev/null > .foo.err
	cat /dev/null > /var/log/kern.log
	;;
del)
	$cmd -F INPUT
	$cmd -A INPUT -s $NET -j SET --del-set ipport src,src
	;;
add)
	$ipset n test hash:net $family 2>/dev/null
	$cmd -F INPUT
	$cmd -A INPUT -s $NET -j SET --add-set test src
	;;
timeout)
	$ipset n test hash:ip,port timeout 2
	$cmd -A INPUT -s $NET -j SET --add-set test src,src --timeout 10 --exist
	;;
mangle)
	$ipset n test hash:net $family skbinfo 2>/dev/null
	$ipset a test 10.255.0.0/16 skbmark 0x1234 2>/dev/null
	$cmd -t mangle -A INPUT -j SET --map-set test src --map-mark
	$cmd -t mangle -A INPUT -m mark --mark 0x1234 -j LOG --log-prefix "in set mark: "
	$cmd -t mangle -A INPUT -s 10.255.0.0/16 -j DROP
	;;
netiface)
	$ipset n test hash:net,iface
	$ipset a test 0.0.0.0/0,eth0
	$cmd -A OUTPUT -m set --match-set test dst,dst -j LOG --log-prefix "in set netiface: "
	$cmd -A OUTPUT -d 10.255.255.254 -j DROP
	;;
counter)
	$ipset n test hash:ip counters
	$ipset a test 10.255.255.64
	$cmd -A OUTPUT -m set --match-set test src --packets-gt 1 ! --update-counters -j DROP
	$cmd -A OUTPUT -m set --match-set test src -j DROP
	./sendip.sh -p ipv4 -id 10.255.255.254 -is 10.255.255.64 -p udp -ud 80 -us 1025 10.255.255.254 >/dev/null 2>&1
	./sendip.sh -p ipv4 -id 10.255.255.254 -is 10.255.255.64 -p udp -ud 80 -us 1025 10.255.255.254 >/dev/null 2>&1
	./sendip.sh -p ipv4 -id 10.255.255.254 -is 10.255.255.64 -p udp -ud 80 -us 1025 10.255.255.254 >/dev/null 2>&1
	;;
resize)
	$ipset n test hash:ip hashsize 4096 maxelem 655360 2>/dev/null
	$cmd -t raw -A OUTPUT -j SET --add-set test src
	$cmd -t raw -A OUTPUT -s 10.255.0.0/16 -j DROP
	$cmd -t raw -A OUTPUT -s 10.254.0.0/16 -j DROP
	./resize_sendip.sh &
	$ipset restore < resize_target.set
	;;
stop)
	$cmd -F
	$cmd -X
	$cmd -F -t mangle
	$cmd -X -t mangle
	$cmd -F -t raw
	$cmd -X -t raw
	$ipset -F 2>/dev/null
	$ipset -X 2>/dev/null
	;;
*)
	echo "Usage: $0 start|stop"
	exit 1
	;;
esac
