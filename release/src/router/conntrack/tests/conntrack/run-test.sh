#!/bin/bash

if [ $UID -ne 0 ]
then
	echo "Run this test as root"
	exit 1
fi

gcc test-conntrack.c -o test
#
# XXX: module auto-load not support by nfnetlink_cttimeout yet :-(
#
# any or all of these might be built-ins rather than modules, so don't error
# out on failure from modprobe
modprobe nf_conntrack_ipv4 || true
modprobe nf_conntrack_ipv6 || true
modprobe nf_conntrack_proto_udplite || true
modprobe nf_conntrack_proto_sctp || true
modprobe nf_conntrack_proto_dccp || true
modprobe nf_conntrack_proto_gre || true
./test testcases
