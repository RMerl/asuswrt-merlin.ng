#!/bin/sh

SQUID_WATCHDOG_INTERVAL_SEC=$(nvram get squid_watchdog_interval_sec)
ROUTER_IP=$(nvram get lan_ipaddr)
ROUTER_NETMASK=$(nvram get lan_netmask)
PROXY_SERVER=$(nvram get squid_proxy_server_ip)
PROXY_PORT=$(nvram get squid_proxy_server_port)

while sleep $SQUID_WATCHDOG_INTERVAL_SEC
do
  for ip in $PROXY_SERVER
  do
    if ping -c 1 $ip > /tmp/null
    then
      if [ -z "$(nvram get transparent_proxy)" ]; then
        /usr/sbin/iptables -t nat -A PREROUTING -i br0 -s $ROUTER_IP/$ROUTER_NETMASK \
          -d $ROUTER_IP/$ROUTER_NETMASK -p tcp --dport 80 -j ACCEPT
        /usr/sbin/iptables -t nat -A PREROUTING -i br0 -s ! $PROXY_SERVER -p tcp --dport 80 \
          -j DNAT --to $PROXY_SERVER:$PROXY_PORT
        /usr/sbin/iptables -t nat -A POSTROUTING -o br0 -s $ROUTER_IP/$ROUTER_NETMASK -p tcp -d \
          $PROXY_SERVER -j SNAT --to $ROUTER_IP
        /usr/sbin/iptables -t filter -I FORWARD -s $ROUTER_IP/$ROUTER_NETMASK -d $PROXY_SERVER -i br0 \
          -o br0 -p tcp --dport $PROXY_PORT -j ACCEPT
        /usr/sbin/nvram set transparent_proxy="1"
      else
        echo "This script has already run!"
      fi
    else
      if [ "$(/usr/sbin/nvram get transparent_proxy)" = "1" ]; then
        /usr/sbin/iptables -t nat -D PREROUTING -i br0 -s $ROUTER_IP/$ROUTER_NETMASK \
          -d $ROUTER_IP/$ROUTER_NETMASK -p tcp --dport 80 -j ACCEPT
        /usr/sbin/iptables -t nat -D PREROUTING -i br0 -s ! $PROXY_SERVER -p tcp --dport 80 \
          -j DNAT --to $PROXY_SERVER:$PROXY_PORT
        /usr/sbin/iptables -t nat -D POSTROUTING -o br0 -s $ROUTER_IP/$ROUTER_NETMASK -p tcp -d \
          $PROXY_SERVER -j SNAT --to $ROUTER_IP
        /usr/sbin/iptables -t filter -D FORWARD -s $ROUTER_IP/$ROUTER_NETMASK -d $PROXY_SERVER -i br0 \
          -o br0 -p tcp --dport $PROXY_PORT -j ACCEPT
        /usr/sbin/nvram unset transparent_proxy
      else
        echo "This script has not run!"
      fi
    fi
  done
done 2>&1
