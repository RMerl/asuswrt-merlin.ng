#!/bin/sh

. lib/generic.sh

ts_log "[Testing add default route]"

DEV=dummy0

ts_ip "$0" "Add new interface $DEV" link add $DEV type dummy
ts_ip "$0" "Set $DEV into UP state" link set up dev $DEV

ts_ip "$0" "Add 1.1.1.1/24 addr on $DEV" addr add 1.1.1.1/24 dev $DEV
ts_ip "$0" "Add default route via 1.1.1.2" route add default via 1.1.1.2

ts_ip "$0" "Show IPv4 default route" -4 route show default
test_on "default via 1.1.1.2 dev $DEV"
test_lines_count 1

ts_ip "$0" "Add another IPv4 route dst 2.2.2.0/24" -4 route add 2.2.2.0/24 dev $DEV
ts_ip "$0" "Show IPv4 default route" -4 route show default
test_on "default via 1.1.1.2 dev $DEV"
test_lines_count 1

ts_ip "$0" "Add dead:beef::1/64 addr on $DEV" -6 addr add dead:beef::1/64 dev $DEV
ts_ip "$0" "Add default route via dead:beef::2" route add default via dead:beef::2
ts_ip "$0" "Show IPv6 default route" -6 route show default
test_on "default via dead:beef::2 dev $DEV"
test_lines_count 1

ts_ip "$0" "Add another IPv6 route dst cafe:babe::/64" -6 route add cafe:babe::/64 dev $DEV
ts_ip "$0" "Show IPv6 default route" -6 route show default
test_on "default via dead:beef::2 dev $DEV"
test_lines_count 1

ts_ip "$0" "Del $DEV dummy interface"  link del dev $DEV
