#!/bin/sh

. lib/generic.sh

ts_log "[Testing tunnelshow]"

BR_DEV="$(rand_dev)"
VX_DEV="$(rand_dev)"

ts_ip "$0" "Add $BR_DEV bridge interface" link add $BR_DEV type bridge

ts_ip "$0" "Add $VX_DEV vxlan interface" \
	link add $VX_DEV type vxlan dstport 4789 external
ts_ip "$0" "Enslave $VX_DEV under $BR_DEV" \
	link set dev $VX_DEV master $BR_DEV
ts_ip "$0" "Set vlan_tunnel on $VX_DEV" \
	link set dev $VX_DEV type bridge_slave vlan_tunnel on

ts_bridge "$0" "Add single vlan" vlan add dev $VX_DEV vid 1000
ts_bridge "$0" "Add single tunnel" \
	vlan add dev $VX_DEV vid 1000 tunnel_info id 1000
ts_bridge "$0" "Add vlan range" vlan add dev $VX_DEV vid 1010-1020
ts_bridge "$0" "Add tunnel range" \
	vlan add dev $VX_DEV vid 1010-1020 tunnel_info id 1010-1020
ts_bridge "$0" "Add single vlan" vlan add dev $VX_DEV vid 1030
ts_bridge "$0" "Add tunnel with vni > 16k" \
	vlan add dev $VX_DEV vid 1030 tunnel_info id 65556

ts_bridge "$0" "Show tunnel info" vlan tunnelshow dev $VX_DEV
test_on "1030\s+65556"
test_lines_count 4

ts_bridge "$0" "Dump tunnel info" -j vlan tunnelshow dev $VX_DEV
