#!/bin/bash

usage() {

	echo "##### Usage #####"
	echo "Usage: lte_start.sh  kerenl_version lte_dev lte_ip lte_net lte_gw_ip lan_net lan_pc vserver_port"
	echo " Ex: lte_start.sh 4.19.134 eth5 192.168.42.100 192.168.42.0 192.168.42.129 192.168.1.0 192.168.1.5 9999"
	exit
}


config() {
	KER_VER=$1
	lte_if=$2

	lte_ip=$3
	lte_net=$4
	lte_gw_ip=$5

	lte_netmask=255.255.255.0

	lan_net=$6
	lan_netmask=255.255.255.0

	lan_pc=$7
	vserv_port=$8
}



setup() {

# load the iptables & conntrack modules
	insmod /lib/modules/$KER_VER/kernel/net/ipv4/netfilter/nf_defrag_ipv4.ko 2>/dev/null
	insmod /lib/modules/$KER_VER/kernel/net/ipv6/netfilter/nf_defrag_ipv6.ko 2>/dev/null
	insmod /lib/modules/$KER_VER/kernel/net/netfilter/nf_conntrack.ko 2>/dev/null
	insmod /lib/modules/$KER_VER/kernel/net/netfilter/nf_nat.ko 2>/dev/null
	insmod /lib/modules/$KER_VER/kernel/net/netfilter/xt_nat.ko 2>/dev/null

	insmod /lib/modules/$KER_VER/kernel/net/ipv4/netfilter/nf_conntrack_ipv4.ko 2>/dev/null
	insmod /lib/modules/$KER_VER/kernel/net/ipv4/netfilter/nf_nat_ipv4.ko 2>/dev/null
	insmod /lib/modules/$KER_VER/kernel/net/ipv4/netfilter/nf_nat_masquerade_ipv4.ko 2>/dev/null
	insmod /lib/modules/$KER_VER/kernel/net/ipv4/netfilter/ip_tables.ko 2>/dev/null
	insmod /lib/modules/$KER_VER/kernel/net/ipv4/netfilter/iptable_nat.ko 2>/dev/null
	insmod /lib/modules/$KER_VER/kernel/net/ipv4/netfilter/ipt_MASQUERADE.ko 2>/dev/null



#Configure iptables rules to perform NAT
	iptables -w -t nat -F

	iptables -w -t nat -D POSTROUTING -o $lte_if -s $lan_net/$lan_netmask -j MASQUERADE 2>/dev/null
	iptables -w -t nat -D POSTROUTING -o $lte_if -s $lan_net/$lan_netmask -j MASQUERADE --mode fullcone 2>/dev/null

	iptables -w -t nat -A POSTROUTING -o $lte_if -s $lan_net/$lan_netmask -j MASQUERADE 

#port forwarding rules
	iptables -w -t nat -A PREROUTING -i $lte_if -p tcp --dport $vserv_port -j DNAT --to $lan_pc:$vserv_port
	iptables -w -t nat -A PREROUTING -i $lte_if -p udp --dport $vserv_port -j DNAT --to $lan_pc:$vserv_port


#echo 1 > /proc/sys/net/ipv4/conf/$lte_if/rp_filter

# set ip to LTE interface
	ifconfig $lte_if $lte_ip netmask $lte_netmask up
	route add -net $lte_net  netmask $lte_netmask metric 1 gw $lte_gw_ip
	sendarp -s br0 -d br0


# set default route 
	route del default 2>/dev/null
	route add default gw $lte_gw_ip dev $lte_if 2>/dev/null

#display the lte ip & routes

	ifconfig $lte_if

	route -n
}

if [ $# -ne 8 ]; then
usage
fi
config $@
setup
