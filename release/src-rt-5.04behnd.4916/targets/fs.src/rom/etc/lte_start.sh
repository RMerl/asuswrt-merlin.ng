#!/bin/bash

usage() {

	echo "##### Usage #####"
	echo "Usage: lte_start.sh [kerenl_version] lte_dev lte_ip lte_net lte_gw_ip lan_net lan_pc vserver_port"
	echo " Ex: lte_start.sh 5.15.149 eth5 192.168.42.100 192.168.42.0 192.168.42.129 192.168.1.0 192.168.1.5 9999"
	echo "  or lte_start.sh eth5 192.168.42.100 192.168.42.0 192.168.42.129 192.168.1.0 192.168.1.5 9999"
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
	lan_netmask_len=24

	lan_pc=$7
	vserv_port=$8

	# echo "DEBUG: KER_VER=$KER_VER, lte_if=$lte_if, lte_ip=$lte_ip, lte_net=$lte_net, lte_gw_ip=$lte_gw_ip lan_net=$lan_net, lan_pc=$lan_pc vserv_port=$vserv_port"
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


	if [ -x "/bin/nft" ]; then 
		#Configure nftable rules to perform NAT
		nft flush table ip ip_nat
		nft add rule ip ip_nat Postrouting oifname $lte_if ip saddr $lan_net/$lan_netmask_len counter masquerade comment "Lte_NatMasq"

		nft add rule ip ip_nat Prerouting iifname $lte_if tcp dport $vserv_port counter dnat to $lan_pc:$vserv_port comment "Lte_DNAT_tcp"
		nft add rule ip ip_nat Prerouting iifname $lte_if udp dport $vserv_port counter dnat to $lan_pc:$vserv_port comment "Lte_DNAT_udp"
	else
		#Configure iptables rules to perform NAT
		iptables -w -t nat -F

		iptables -w -t nat -D POSTROUTING -o $lte_if -s $lan_net/$lan_netmask -j MASQUERADE 2>/dev/null
		iptables -w -t nat -D POSTROUTING -o $lte_if -s $lan_net/$lan_netmask -j MASQUERADE --mode fullcone 2>/dev/null

		iptables -w -t nat -A POSTROUTING -o $lte_if -s $lan_net/$lan_netmask -j MASQUERADE 

		#port forwarding rules
		iptables -w -t nat -A PREROUTING -i $lte_if -p tcp --dport $vserv_port -j DNAT --to $lan_pc:$vserv_port
		iptables -w -t nat -A PREROUTING -i $lte_if -p udp --dport $vserv_port -j DNAT --to $lan_pc:$vserv_port
	fi

#echo 1 > /proc/sys/net/ipv4/conf/$lte_if/rp_filter

# delete the interface from br0
	in_br0=`brctl show br0 | grep $lte_if`
	if [ "$in_br0" != "" ] ; then
		brctl delif br0 $lte_if
	fi

# get the mac address of the real interface
	real_if=`echo $lte_if | cut -d"." -f0`
	lte_if_mac=`cat /sys/class/net/$real_if/address`
	ifconfig $lte_if down
	ifconfig $lte_if hw ether $lte_if_mac
	ifconfig $lte_if up

# set ip to LTE interface
	ifconfig $lte_if $lte_ip netmask $lte_netmask up
	route add -net $lte_net  netmask $lte_netmask metric 1 gw $lte_gw_ip
	bcm_sendarp -s br0 -d br0


# set default route 
	route del default 2>/dev/null
	route add default gw $lte_gw_ip dev $lte_if 2>/dev/null

#display the lte ip & routes

	ifconfig $lte_if

	route -n
}

if [ $# -eq 8 ]; then
	config $@
	setup
elif [ $# -eq 7 ]; then
	kver=`uname -r`
	config $kver $@
	setup
else
	usage
fi
