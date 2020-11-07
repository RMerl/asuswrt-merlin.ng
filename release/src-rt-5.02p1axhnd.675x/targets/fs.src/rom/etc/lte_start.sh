#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage: lte_start.sh  kerenl_version interface " 
  echo " Ex: lte_start.sh 4.1.45 eth5 "
  exit 1
fi


KER_VER=$1
lte_if=$2

lte_ip=192.168.42.100
lte_gw_ip=192.168.42.129
lte_netmask=255.255.255.0
lte_net=192.168.42.0
lte_bcast_ip=192.168.42.255

lan_net=192.168.1.0
lan_netmask=255.255.255.0



# load the iptables & conntrack modules

insmod /lib/modules/$KER_VER/kernel/net/netfilter/nf_conntrack.ko 2>/dev/null
insmod /lib/modules/$KER_VER/kernel/net/ipv4/netfilter/nf_defrag_ipv4.ko 2>/dev/null
insmod /lib/modules/$KER_VER/kernel/net/ipv4/netfilter/nf_conntrack_ipv4.ko 2>/dev/null
#insmod /lib/modules/$KER_VER/kernel/net/ipv4/netfilter/nf_nat.ko 2>/dev/null
insmod /lib/modules/$KER_VER/kernel/net/netfilter/nf_nat.ko 2>/dev/null
insmod /lib/modules/$KER_VER/kernel/net/netfilter/xt_nat.ko 2>/dev/null
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
iptables -w -t nat -A PREROUTING -i $lte_if -p tcp --dport 9999 -j DNAT --to 192.168.1.5:9999
iptables -w -t nat -A PREROUTING -i $lte_if -p udp --dport 9999 -j DNAT --to 192.168.1.5:9999


#echo 1 > /proc/sys/net/ipv4/conf/$lte_if/rp_filter

# set ip to LTE interface
ifconfig $lte_if $lte_ip netmask $lte_netmask broadcast $lte_bcast_ip up
route add -net $lte_net  netmask $lte_netmask metric 1 gw $lte_gw_ip
sendarp -s br0 -d br0


# set default route 
route del default 2>/dev/null
route add default gw $lte_gw_ip dev $lte_if 2>/var/gwerr

#display the lte ip & routes

ifconfig $lte_if

route -n
