#!/bin/sh

. lib/generic.sh

ts_log "[Testing Add BareUDP interface (unicast MPLS)]"
NEW_DEV="$(rand_dev)"

ts_ip "$0" "Add $NEW_DEV BareUDP interface (unicast MPLS)" link add dev $NEW_DEV type bareudp dstport 6635 ethertype mpls_uc

ts_ip "$0" "Show $NEW_DEV BareUDP interface (unicast MPLS)" -d link show dev $NEW_DEV
test_on "$NEW_DEV"
test_on "dstport 6635"
test_on "ethertype mpls_uc"
test_on "nomultiproto"

ts_ip "$0" "Del $NEW_DEV BareUDP interface (unicast MPLS)" link del dev $NEW_DEV


ts_log "[Testing Add BareUDP interface (multicast MPLS)]"
NEW_DEV="$(rand_dev)"

ts_ip "$0" "Add $NEW_DEV BareUDP interface (multicast MPLS)" link add dev $NEW_DEV type bareudp dstport 6635 ethertype mpls_mc

ts_ip "$0" "Show $NEW_DEV BareUDP interface (multicast MPLS)" -d link show dev $NEW_DEV
test_on "$NEW_DEV"
test_on "dstport 6635"
test_on "ethertype mpls_mc"
test_on "nomultiproto"

ts_ip "$0" "Del $NEW_DEV BareUDP interface (multicast MPLS)" link del dev $NEW_DEV


ts_log "[Testing Add BareUDP interface (unicast and multicast MPLS)]"
NEW_DEV="$(rand_dev)"

ts_ip "$0" "Add $NEW_DEV BareUDP interface (unicast and multicast MPLS)" link add dev $NEW_DEV type bareudp dstport 6635 ethertype mpls_uc multiproto

ts_ip "$0" "Show $NEW_DEV BareUDP interface (unicast and multicast MPLS)" -d link show dev $NEW_DEV
test_on "$NEW_DEV"
test_on "dstport 6635"
test_on "ethertype mpls_uc"
test_on "multiproto"

ts_ip "$0" "Del $NEW_DEV BareUDP interface (unicast and multicast MPLS)" link del dev $NEW_DEV


ts_log "[Testing Add BareUDP interface (IPv4)]"
NEW_DEV="$(rand_dev)"

ts_ip "$0" "Add $NEW_DEV BareUDP interface (IPv4)" link add dev $NEW_DEV type bareudp dstport 6635 ethertype ipv4

ts_ip "$0" "Show $NEW_DEV BareUDP interface (IPv4)" -d link show dev $NEW_DEV
test_on "$NEW_DEV"
test_on "dstport 6635"
test_on "ethertype ip"
test_on "nomultiproto"

ts_ip "$0" "Del $NEW_DEV BareUDP interface (IPv4)" link del dev $NEW_DEV


ts_log "[Testing Add BareUDP interface (IPv6)]"
NEW_DEV="$(rand_dev)"

ts_ip "$0" "Add $NEW_DEV BareUDP interface (IPv6)" link add dev $NEW_DEV type bareudp dstport 6635 ethertype ipv6

ts_ip "$0" "Show $NEW_DEV BareUDP interface (IPv6)" -d link show dev $NEW_DEV
test_on "$NEW_DEV"
test_on "dstport 6635"
test_on "ethertype ipv6"
test_on "nomultiproto"

ts_ip "$0" "Del $NEW_DEV BareUDP interface (IPv6)" link del dev $NEW_DEV


ts_log "[Testing Add BareUDP interface (IPv4 and IPv6)]"
NEW_DEV="$(rand_dev)"

ts_ip "$0" "Add $NEW_DEV BareUDP interface (IPv4 and IPv6)" link add dev $NEW_DEV type bareudp dstport 6635 ethertype ipv4 multiproto

ts_ip "$0" "Show $NEW_DEV BareUDP interface (IPv4 and IPv6)" -d link show dev $NEW_DEV
test_on "$NEW_DEV"
test_on "dstport 6635"
test_on "ethertype ip"
test_on "multiproto"

ts_ip "$0" "Del $NEW_DEV BareUDP interface (IPv4 and IPv6)" link del dev $NEW_DEV
