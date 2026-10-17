#!/bin/sh
source /etc/profile

case "$ACTION" in
    "add")
        echo "mdev_tc: detected PLUG of $1"
        echo "$1 len ${#1}"
        if [ `expr "$1" : 'eth[0-9]\+'` = ${#1} -a -x "/sys/devices/virtual/net/$1" ]; then
            tc qdisc add dev $1 clsact
            lans=`ethswctl -c getlanall | awk -F ":" '{print $2}' | tr "," "\n"`
            # tc chain id 1~4 are reserved for tc rules of IC,
            # Keep in sync with bcmdrivers/opensource/include/bcm963xx/bcm_tc.h
            if [ -z "`echo $lans|grep $1`" ]; then
                echo "Add GBE tc Ingress Filter rules for $1"
                tc filter add dev $1 ingress protocol ip chain 1 prio 1 flower ip_proto udp dst_port 67 skip_sw action trap
                tc filter add dev $1 ingress protocol 0x8863 chain 1 prio 9 flower skip_sw action trap
                tc filter add dev $1 ingress protocol 0x8864 prio 10 chain 1 flower skip_sw action trap
            else
                echo "Add LAN tc Ingress Filter rules for $1"
                tc filter add dev $1 ingress protocol ip chain 1 prio 1 flower ip_proto udp dst_port 67 skip_sw action trap
                tc filter add dev $1 ingress protocol ip chain 1 prio 2 flower ip_proto 2 skip_sw action trap
                tc filter add dev $1 ingress protocol ipv6 chain 1 prio 3 flower ip_proto icmpv6 type 130 skip_sw action trap
                tc filter add dev $1 ingress protocol 0x8864 prio 10 chain 1 flower skip_sw action trap
                tc filter add dev $1 ingress protocol arp chain 1 prio 11 flower skip_sw action trap
                tc filter add dev $1 ingress protocol 0x8902 chain 1 prio 13 flower skip_sw action trap
                tc filter add dev $1 ingress chain 1 prio 19 flower dst_mac 11 skip_sw action trap
                tc filter add dev $1 ingress chain 1 prio 20 flower dst_mac FF:FF:FF:FF:FF:FF skip_sw action trap
                tc filter add dev $1 ingress protocol ip chain 1 prio 24 flower dst_ip 224.0.0.0 skip_sw action trap
                cat /proc/config.gz | gunzip | grep -q CONFIG_BCM_PTP_1588
                rc=$?
                if [ ${rc} -eq 0 ]; then
                    # Add PTP 1588 rules
                    tc filter add dev $1 ingress protocol 0x88f7 chain 1 prio 14 flower skip_sw action trap
                    tc filter add dev $1 ingress protocol ip chain 1 prio 15 flower ip_proto udp dst_port 320 skip_sw action trap
                fi
            fi
        fi
        if [ `expr match "$1" 'gpondef'` = ${#1} -o `expr "$1" : 'epon'` -ne 0 ]; then
            echo "Add WAN tc Ingress Filter rules for $1"
            tc qdisc add dev $1 clsact
            sleep 1
            tc filter add dev $1 ingress protocol 0x8864 prio 10 chain 1 flower skip_sw action trap
            tc filter add dev $1 ingress protocol arp chain 1 prio 11 flower skip_sw action trap
            tc filter add dev $1 ingress chain 1 prio 20 flower dst_mac FF:FF:FF:FF:FF:FF skip_sw action trap
        fi
        if [ `expr match "$1" 'wl[0-9]\+'` = ${#1} ]; then
            echo "Add WLAN tc Ingress Filter rules for $1"
            tc qdisc add dev $1 clsact
            tc filter add dev $1 ingress protocol ip chain 1 prio 1 flower ip_proto udp dst_port 67 skip_sw action trap
            tc filter add dev $1 ingress protocol ip chain 1 prio 2 flower ip_proto 2 skip_sw action trap
            tc filter add dev $1 ingress protocol ipv6 chain 1 prio 3 flower ip_proto icmpv6 type 130 skip_sw action trap
            tc filter add dev $1 ingress protocol 0x8864 prio 10 chain 1 flower skip_sw action trap
            tc filter add dev $1 ingress protocol arp chain 1 prio 11 flower skip_sw action trap
            tc filter add dev $1 ingress protocol 0x8902 chain 1 prio 13 flower skip_sw action trap
            tc filter add dev $1 ingress chain 1 prio 19 flower dst_mac 11 skip_sw action trap
            tc filter add dev $1 ingress chain 1 prio 20 flower dst_mac FF:FF:FF:FF:FF:FF skip_sw action trap
            tc filter add dev $1 ingress protocol ip chain 1 prio 24flower dst_ip 224.0.0.0 skip_sw action trap
        fi
        ;;
    "remove")
        echo "mdev_tc: detected UNPLUG of $1"
        ;;
esac

