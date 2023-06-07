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

ts_tc "$0" "Add MPLS filter matching first LSE with minimal values" \
	filter add dev $DEV ingress protocol mpls_uc flower         \
	mpls_label 0 mpls_tc 0 mpls_bos 0 mpls_ttl 0                \
	action drop
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "mpls_label 0"
test_on "mpls_tc 0"
test_on "mpls_bos 0"
test_on "mpls_ttl 0"

reset_qdisc
ts_tc "$0" "Add MPLS filter matching first LSE with maximal values" \
	filter add dev $DEV ingress protocol mpls_uc flower         \
	mpls_label 1048575 mpls_tc 7 mpls_bos 1 mpls_ttl 255        \
	action drop
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "mpls_label 1048575"
test_on "mpls_tc 7"
test_on "mpls_bos 1"
test_on "mpls_ttl 255"

reset_qdisc
ts_tc "$0" "Add MPLS filter matching second LSE with minimal values" \
	filter add dev $DEV ingress protocol mpls_uc flower          \
	mpls lse depth 2 label 0 tc 0 bos 0 ttl 0                    \
	action drop
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "mpls"
test_on "lse"
test_on "depth 2"
test_on "label 0"
test_on "tc 0"
test_on "bos 0"
test_on "ttl 0"

reset_qdisc
ts_tc "$0" "Add MPLS filter matching second LSE with maximal values" \
	filter add dev $DEV ingress protocol mpls_uc flower          \
	mpls lse depth 2 label 1048575 tc 7 bos 1 ttl 255            \
	action drop
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "mpls"
test_on "lse"
test_on "depth 2"
test_on "label 1048575"
test_on "tc 7"
test_on "bos 1"
test_on "ttl 255"

reset_qdisc
ts_tc "$0" "Add MPLS filter matching two LSEs"                   \
	filter add dev $DEV ingress protocol mpls_uc flower mpls \
	lse depth 1 label 0 tc 0 bos 0 ttl 0                     \
	lse depth 2 label 1048575 tc 7 bos 1 ttl 255             \
	action drop
ts_tc "$0" "Show ingress filters" filter show dev $DEV ingress
test_on "mpls"
test_on "lse"
test_on "depth 1"
test_on "label 0"
test_on "tc 0"
test_on "bos 0"
test_on "ttl 0"
test_on "depth 2"
test_on "label 1048575"
test_on "tc 7"
test_on "bos 1"
test_on "ttl 255"
