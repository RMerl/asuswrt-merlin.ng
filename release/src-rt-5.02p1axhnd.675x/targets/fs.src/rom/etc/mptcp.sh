#!/bin/sh

config() {

LAN_DEV=$2
PROXY_SERVER_ADDR=$3
PROXY_SERVER_PORT=$4

WAN1_DEV=$5
WAN1_ADDR=$6
WAN1_GW=$7
WAN1_NET=$8

WAN2_DEV=$9
WAN2_ADDR=${10}
WAN2_GW=${11}
WAN2_NET=${12}

echo "LAN_DEV=$LAN_DEV PROXY_SERVER_ADDR=$PROXY_SERVER_ADDR PROXY_SERVER_PORT=$PROXY_SERVER_PORT"
echo "WAN1_DEV=$WAN1_DEV WAN1_ADDR=$WAN1_ADDR WAN1_GW=$WAN1_GW WAN1_NET=$WAN1_NET"
echo "WAN2_DEV=$WAN2_DEV WAN2_ADDR=$WAN2_ADDR WAN2_GW=$WAN2_GW WAN2_NET=$WAN2_NET"

}

usage() {

	echo "##### Usage #####"
	echo "mptcp.sh stop"
	echo " OR"
	echo "mptcp.sh start landev proxt_server_ip porxy_server_port wan1_dev wan1_addr wan1_gw wan1_net wan2_dev wan2_addr wan2_gw wan2_net"
	echo "ex: mptcp.sh start br0 4.4.4.4 1080 ptm0.1 3.3.3.1 3.3.3.3 3.3.3.0 eth0.1 2.2.2.1 2.2.2.2 2.2.2.0"

	exit
}

cleanup() {
	ip rule del table 1 2> /dev/null
	ip rule del table 2 2> /dev/null
	ip route flush table 1 2> /dev/null
	ip route flush table 2 2> /dev/null

	iptables -w -t nat -D PREROUTING -i $LAN_DEV -p tcp -j REDSOCKS 2> /dev/null

	iptables -w -t nat -F REDSOCKS 2> /dev/null
	iptables -w -t nat -X REDSOCKS 2> /dev/null
}

case $1 in
	start)

		if [ $# -ne 12 ]; then
			usage
		fi

		config $@
		cleanup

		
		#disable mptcp on LAN
		[ ! -z "$LAN_DEV" ] && ip link set dev $LAN_DEV multipath off

####### Setup Routing ########

		echo "configuring routes"

		# add policy routing rules for WAN1
		[ ! -z "$WAN1_ADDR" ] && ip rule add from $WAN1_ADDR table 1
		# add routes on WAN1
		[ ! -z "$WAN1_GW" ] && {
			ip route add $WAN1_NET dev $WAN1_DEV scope link table 1
			ip route add default via $WAN1_GW dev $WAN1_DEV table 1
		}

		# add policy routing rules for WAN2
		[ ! -z "$WAN2_ADDR" ] && ip rule add from $WAN2_ADDR table 2
		# add routes on WAN2
		[ ! -z "$WAN2_GW" ] && {
			ip route add $WAN2_NET dev $WAN2_DEV scope link table 2
			ip route add default via $WAN2_GW dev $WAN2_DEV table 2
		}

		#add routes to default route table
		#ip route add $WAN1_NET dev $WAN1_DEV src $WAN1_ADDR
		#ip route add $WAN2_NET dev $WAN2_DEV src $WAN2_ADDR
		#ip route add default via $WAN1_ADDR


####### Configure Iptables to redirect tcp traffic from LAN to redsocks ########

		echo "configuring iptables rules"
		# add a new REDSOCKS chain
		iptables -w -t nat -N REDSOCKS

		# igonre traffic we are not intrested in sending to proxy
		# adjust as per test setup
		iptables -w -t nat -I REDSOCKS -o lo -j RETURN
		iptables -w -t nat -I REDSOCKS -p tcp -d $PROXY_SERVER_ADDR --dport $PROXY_SERVER_PORT -j RETURN
		iptables -w -t nat -A REDSOCKS -d 127.0.0.0/8 -j RETURN                                                                                     
		iptables -w -t nat -A REDSOCKS -d 192.168.0.0/16 -j RETURN
		iptables -w -t nat -A REDSOCKS -d 10.0.0.0/8 -j RETURN                                                                                      
		iptables -w -t nat -A REDSOCKS -d 169.254.0.0/16 -j RETURN                                                                                  
		iptables -w -t nat -A REDSOCKS -d 172.16.0.0/12 -j RETURN                                                                                   
		iptables -w -t nat -A REDSOCKS -d 224.0.0.0/4 -j RETURN
		iptables -w -t nat -A REDSOCKS -d 240.0.0.0/4 -j RETURN

		#redirect the tcp packets filtered in REDSOCKS chain to port 12345
		iptables -w -t nat -A REDSOCKS -p tcp -j REDIRECT --to-ports 12345

		# add a rule to tap all incoming tcp traffic on br0 into redsocks CHAIN at PREROUTING
		iptables -w -t nat -I PREROUTING -i $LAN_DEV -p tcp -j REDSOCKS

		# add rules to tap locally generated tcp traffic to REDSOCKS
		#[ ! -z "$WAN1_DEV" ] && iptables -w -t nat -A OUTPUT -o $WAN1_DEV -p tcp -j REDSOCKS
		#[ ! -z "$WAN2_DEV" ] && iptables -w -t nat -A OUTPUT -o $WAN2_DEV -p tcp -j REDSOCKS

	;;

	stop)
		cleanup
	;;

	*)
		usage

esac
