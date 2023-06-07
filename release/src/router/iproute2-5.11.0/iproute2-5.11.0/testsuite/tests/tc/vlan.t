#!/bin/sh

. lib/generic.sh

DEV="$(rand_dev)"
ts_ip "$0" "Add $DEV dummy interface" link add dev $DEV up type dummy
ts_tc "$0" "Add ingress qdisc" qdisc add dev $DEV ingress

reset_qdisc()
{
	ts_tc "$0" "Remove ingress qdisc" qdisc del dev $DEV ingress
	ts_tc "$0" "Add ingress qdisc" qdisc add dev $DEV ingress
}

ts_tc "$0" "Add vlan action pop" \
	filter add dev $DEV ingress matchall action vlan pop
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "vlan"
test_on "pop"
test_on "pipe"

reset_qdisc
ts_tc "$0" "Add vlan action push (default parameters)" \
	filter add dev $DEV ingress matchall action vlan push id 5
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "vlan"
test_on "push"
test_on "id 5"
test_on "protocol 802.1Q"
test_on "priority 0"
test_on "pipe"

reset_qdisc
ts_tc "$0" "Add vlan action push (explicit parameters)" \
	filter add dev $DEV ingress matchall            \
	action vlan push id 5 protocol 802.1ad priority 2
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "vlan"
test_on "push"
test_on "id 5"
test_on "protocol 802.1ad"
test_on "priority 2"
test_on "pipe"

reset_qdisc
ts_tc "$0" "Add vlan action modify (default parameters)" \
	filter add dev $DEV ingress matchall action vlan modify id 5
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "vlan"
test_on "modify"
test_on "id 5"
test_on "protocol 802.1Q"
test_on "priority 0"
test_on "pipe"

reset_qdisc
ts_tc "$0" "Add vlan action modify (explicit parameters)" \
	filter add dev $DEV ingress matchall              \
	action vlan modify id 5 protocol 802.1ad priority 2
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "vlan"
test_on "modify"
test_on "id 5"
test_on "protocol 802.1ad"
test_on "priority 2"
test_on "pipe"

reset_qdisc
ts_tc "$0" "Add vlan action pop_eth" \
	filter add dev $DEV ingress matchall action vlan pop_eth
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "vlan"
test_on "pop_eth"
test_on "pipe"

reset_qdisc
ts_tc "$0" "Add vlan action push_eth"                  \
	filter add dev $DEV ingress matchall           \
	action vlan push_eth dst_mac 02:00:00:00:00:02 \
	src_mac 02:00:00:00:00:01
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "vlan"
test_on "push_eth"
test_on "dst_mac 02:00:00:00:00:02"
test_on "src_mac 02:00:00:00:00:01"
test_on "pipe"
