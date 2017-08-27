#!/bin/bash

DIR=$(dirname `readlink -f $0`)
. $DIR/testing.conf
. $DIR/scripts/function.sh

echo "Stopping test environment"

NETWORKS="vnet1 vnet2 vnet3"
KNLTARGET=/var/run/kvm-swan-kernel
HOSTFSTARGET=/var/run/kvm-swan-hostfs

[ `id -u` -eq 0 ] || die "You must be root to run $0"

check_commands virsh

for net in $NETWORKS
do
	log_action "Network $net"
	execute "virsh net-destroy $net"
done

for host in $STRONGSWANHOSTS
do
	log_action "Guest $host"
	execute "virsh shutdown $host"
	rm -f $VIRTIMGSTORE/$host.$IMGEXT
done

log_action "Removing kernel $KERNEL"
execute "rm $KNLTARGET"

log_action "Removing link to hostfs"
execute "rm $HOSTFSTARGET"
