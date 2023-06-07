#!/bin/sh

. lib/generic.sh

ts_log "[Testing vlan show]"

BR_DEV="$(rand_dev)"
VX0_DEV="$(rand_dev)"
VX1_DEV="$(rand_dev)"

ts_ip "$0" "Add $BR_DEV bridge interface" link add $BR_DEV type bridge

ts_ip "$0" "Add $VX0_DEV vxlan interface" \
	link add $VX0_DEV type vxlan dstport 4789 external
ts_ip "$0" "Enslave $VX0_DEV under $BR_DEV" \
	link set dev $VX0_DEV master $BR_DEV
ts_bridge "$0" "Delete default vlan from $VX0_DEV" \
	vlan del dev $VX0_DEV vid 1
ts_ip "$0" "Add $VX1_DEV vxlan interface" \
	link add $VX1_DEV type vxlan dstport 4790 external
ts_ip "$0" "Enslave $VX1_DEV under $BR_DEV" \
	link set dev $VX1_DEV master $BR_DEV

# Test that bridge ports without vlans do not appear in the output
ts_bridge "$0" "Show vlan" vlan
test_on_not "$VX0_DEV"

# Test that bridge ports without tunnels do not appear in the output
ts_bridge "$0" "Show vlan tunnel info" vlan tunnelshow
test_lines_count 1 # header only
