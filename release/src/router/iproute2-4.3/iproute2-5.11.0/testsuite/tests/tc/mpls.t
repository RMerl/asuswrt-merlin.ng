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

ts_tc "$0" "Add mpls action pop"                              \
	filter add dev $DEV ingress protocol mpls_uc matchall \
	action mpls pop protocol ip
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "mpls"
test_on "pop protocol ip pipe"

reset_qdisc
ts_tc "$0" "Add mpls action push"                        \
	filter add dev $DEV ingress protocol ip matchall \
	action mpls push protocol mpls_uc label 20 tc 3 bos 1 ttl 64
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "mpls"
test_on "push"
test_on "protocol mpls_uc"
test_on "label 20"
test_on "tc 3"
test_on "bos 1"
test_on "ttl 64"
test_on "pipe"

reset_qdisc
ts_tc "$0" "Add mpls action mac_push"        \
	filter add dev $DEV ingress matchall \
	action mpls mac_push protocol mpls_uc label 20 tc 3 bos 1 ttl 64
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "mpls"
test_on "mac_push"
test_on "protocol mpls_uc"
test_on "label 20"
test_on "tc 3"
test_on "bos 1"
test_on "ttl 64"
test_on "pipe"

reset_qdisc
ts_tc "$0" "Add mpls action modify"                           \
	filter add dev $DEV ingress protocol mpls_uc matchall \
	action mpls modify label 20 tc 3 ttl 64
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "mpls"
test_on "modify"
test_on "label 20"
test_on "tc 3"
test_on "ttl 64"
test_on "pipe"

reset_qdisc
ts_tc "$0" "Add mpls action dec_ttl"                          \
	filter add dev $DEV ingress protocol mpls_uc matchall \
	action mpls dec_ttl
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "mpls"
test_on "dec_ttl"
test_on "pipe"
