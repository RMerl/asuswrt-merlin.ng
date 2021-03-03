#!/bin/sh

. lib/generic.sh

TUNNEL_NAME="tunnel_test_ip"
KMODS="ip6_gre ip6_tunnel ip_gre ip_tunnel gre"

# unload kernel modules to remove dummy interfaces only if they were not in use beforehand
kmods_remove=
# note that checkbashism reports command -v, but dash supports it and it's POSIX 2008 compliant
if command -v lsmod >/dev/null 2>&1 && command -v rmmod >/dev/null 2>&1; then
    for i in $KMODS; do
        lsmod | grep -q "^$i" || kmods_remove="$kmods_remove $i";
    done
fi

ts_log "[Testing add/del tunnels]"

ts_ip "$0" "Add GRE tunnel over IPv4" tunnel add name $TUNNEL_NAME mode gre local 1.1.1.1 remote 2.2.2.2
ts_ip "$0" "Del GRE tunnel over IPv4" tunnel del $TUNNEL_NAME

ts_ip "$0" "Add GRE tunnel over IPv6" tunnel add name $TUNNEL_NAME mode ip6gre local dead:beef::1 remote dead:beef::2
ts_ip "$0" "Del GRE tunnel over IPv6" tunnel del $TUNNEL_NAME

for mod in $kmods_remove; do
    sudo rmmod "$mod"
done
