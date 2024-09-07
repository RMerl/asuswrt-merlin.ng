#!/bin/sh

echo "======Version Info======"

echo
echo "###### Board-id ######"
cat /proc/nvram/boardid
echo

echo
echo "######image version######"
cat /etc/image_version

echo
echo "######build profile######"
if [ -e /etc/build_profile ]; then
    ls -al /etc/build_profile|cut -d'>' -f 2
    cat /etc/build_profile
fi

echo
echo "######build kernel config######"
gunzip -c /proc/config.gz

if [ -e /etc/patch.version ]; then
    echo
    echo "######patch version######"
    cat /etc/patch.version
fi

echo
echo "######kernel version######"
cat /proc/version

if [ -e /bin/pwrctl ]; then
    echo
    echo "######Power Management Configuration######"
    /bin/pwrctl show
fi

echo
echo "======bootstate======"

bootstate_list="/proc/bootstate/active_image
		/proc/bootstate/boot_failed_count
		/proc/bootstate/old_reset_reason
		/proc/bootstate/reset_reason
		/proc/bootstate/reset_status"

if [ -e /proc/bootstate ]; then
    for f in $bootstate_list
    do
        if [ -e $f ]; then
            echo
            echo "######${f}######"
            cat $f
            echo
        fi
    done
    echo
fi

echo
echo
echo "======System Info======"

sys_info_list="/proc/uptime
               /proc/cpuinfo
               /proc/brcm/kernel_config
               /proc/interrupts
               /proc/meminfo
               /proc/iomem
               /proc/slabinfo
               /proc/modules
               /proc/timer_list
               /proc/bus/pci/devices
               /proc/sys/kernel/sched_compat_yield
               /proc/sys/kernel/sched_rt_period_us
               /proc/sys/kernel/sched_rt_runtime_us"

# busybox msh does not support passing lists to functions
# so must repeat the function here
for f in $sys_info_list
do
    echo
    echo "######${f}######"
    if [ -e $f ]; then
        cat $f
    else
        echo "$f does not exist on this system."
    fi
done

echo
echo
echo "======free======"
free

# Current Processes Information
echo
echo "###### ps ######"
ps

# Log all processes's cpu affinity and prority.
if [ -e /usr/bin/taskset ] && [ -e /usr/bin/chrt ]; then
echo
echo "###### All processes's cpu affinity/priority ######"
for entry in /proc/*
do
    #echo "$entry"
    if [ -f $entry/comm ] ; then
        [ $entry = "/proc/self" ] && continue
        [ $entry = "/proc/thread-self" ] && continue

        comm=`cat "$entry"/comm`
        pid=${entry:6}
        echo "### Process: name=$comm pid=$pid ###"
        taskset -p $pid
        chrt -p $pid
    fi
done
fi

# Log all processes's open files count
if [ -e /usr/bin/lsof ]; then
    echo
    echo "###### All processes's open files count ######"
    lsof | awk '{ print $1 " " $2; }' | sort -rn | uniq -c | sort -rn
fi

if [ -e /bin/wlaffinity ]; then
   echo
   echo "###### WLAN affinity ######"
   /bin/wlaffinity show
fi


echo
echo

echo "======Networking Info======"

#Networking Information
echo
echo "######ifconfig -a######"
ifconfig -a

#virtual interface info
echo
echo "###### dump all vlanctl rules ######"
vlanctl --rule-dump-all

#TC rules
echo
echo "###### dump all interfaces ######"
sleep 1
ip -d link
echo
echo

echo
echo "###### dump all TC rules ######"
sleep 1
for i in $(ip -br a | awk '{print$1}'); do echo "### $i qdisc ###"; tc qdisc show dev $i 2>/dev/null; echo "### $i class ###"; tc class show dev $i 2>/dev/null; echo "### $i filter ###"; tc -s filter show dev $i 2>/dev/null; done;

echo
echo "######brctl show######"
brctl show

echo
echo "######route -n######"
route -n

echo
echo "######route -A inet6######"
route -A inet6

echo
echo "######ip -f inet6 neigh######"
ip -f inet6 neigh

echo
echo "###### iptables -w -t nat -L ######"
iptables -t nat -L -v -n

echo
echo "###### iptables -w -t filter -L ######"
iptables -t filter -L -v -n

echo
echo "###### ip6tables -w -t filter -L ######"
ip6tables -t filter -L -v -n

echo
echo "###### iptables -w -t mangle -L ######"
iptables -t mangle -L -v -n

echo
echo "###### ebtables -t filter -L --Lc --Ln ######"
ebtables -t filter -L --Lc --Ln

echo
echo "###### ebtables -t broute -L --Lc --Ln ######"
ebtables -t broute -L --Lc --Ln

echo
echo "###### ebtables -t nat -L --Lc --Ln ######"
ebtables -t nat -L --Lc --Ln

net_info_list="/proc/net/arp
               /proc/sys/net/core/netdev_budget
               /proc/sys/net/core/netdev_max_backlog
               /proc/sys/net/core/dev_weight
               /proc/net/ip_tables_names
               /proc/net/ip_tables_matches
               /proc/net/ip_tables_targets
               /proc/net/netfilter/nf_log
               /proc/net/netfilter/nf_queue
               /proc/net/netfilter/nfnetlink_queue
               /proc/net/stat/nf_conntrack
               /proc/net/nf_conntrack_expect"

# busybox msh does not support passing lists to functions
# so must repeat the function here
for f in $net_info_list
do
    echo
    echo "######${f}######"
    if [ -e $f ]; then
        cat $f
    else
        echo "$f does not exist on this system."
    fi
done

net_info_list="nf_conntrack_acct
               nf_conntrack_checksum
               nf_conntrack_generic_timeout
               nf_conntrack_icmp_timeout
               nf_conntrack_log_invalid
               nf_conntrack_tcp_be_liberal
               nf_conntrack_tcp_loose
               nf_conntrack_tcp_max_retrans
               nf_conntrack_tcp_timeout_close
               nf_conntrack_tcp_timeout_close_wait
               nf_conntrack_tcp_timeout_established
               nf_conntrack_tcp_timeout_fin_wait
               nf_conntrack_tcp_timeout_last_ack
               nf_conntrack_tcp_timeout_max_retrans
               nf_conntrack_tcp_timeout_syn_recv
               nf_conntrack_tcp_timeout_syn_sent
               nf_conntrack_tcp_timeout_time_wait
               nf_conntrack_tcp_timeout_unacknowledged
               nf_conntrack_udp_timeout
               nf_conntrack_udp_timeout_stream
               nf_conntrack_buckets
               nf_conntrack_max
               nf_conntrack_count
               nf_conntrack_expect_max"

# busybox msh does not support passing lists to functions
# so must repeat the function here.  Also it does not like long
# lists, so the netfilter list is broken into two parts
for f in $net_info_list
do
    lf=/proc/sys/net/netfilter/$f
    echo
    echo "######${lf}######"
    if [ -e $lf ]; then
        cat $lf
    else
        echo "$lf does not exist on this system."
    fi
done

# compress /proc/net/nf_conntrack
cp /proc/net/nf_conntrack /tmp
cd /tmp && tar zcf nf_conntrack.tgz nf_conntrack
rm -f /tmp/nf_conntrack
mkdir -p /tmp/asusfbsvcs/duplicate_log
mv /tmp/nf_conntrack.tgz /tmp/asusfbsvcs/duplicate_log

echo
echo
echo "======Multicast Info======"
echo "###### /var/mcpd.conf ######"
cat /var/mcpd.conf
if [ -e /bin/mcpctl ]; then
   echo "###### mcpctl allinfo ######"
   mcpctl allinfo
fi

mcast_info_list="/proc/net/igmp
                 /proc/net/igmp_snooping
                 /proc/net/ip_mr_cache
                 /proc/net/ip_mr_vif
                 /proc/net/dev_mcast
                 /proc/net/igmp6
                 /proc/net/mld_snooping
                 /proc/net/ip6_mr_cache
                 /proc/net/ip6_mr_vif"

# busybox msh does not support passing lists to functions
# so must repeat the function here
for f in $mcast_info_list
do
    echo
    echo "######${f}######"
    if [ -e $f ]; then
        cat $f
    else
        echo "$f does not exist on this system."
    fi
done

echo
echo
echo "======Accelerators======"

if [ -e /proc/fcache ]; then
    if [ -e /bin/fcctl ]; then
       echo
       echo "###### fcctl status ######"
       fcctl status
    fi
    echo
    echo "###### /proc/fcache/* ######"
    cat /proc/fcache/*
    echo
    echo "###### /proc/fcache/misc/* ######"
    cat /proc/fcache/misc/*
    echo
    echo "###### /proc/fcache/stats/* ######"
    cat /proc/fcache/stats/*
fi

redirect_dmesg () {
    rm -f /tmp/dmesg_tmp
    dmesg -c >> /tmp/dmesg_tmp
    cat /tmp/dmesg_tmp
}

mkdir -p /tmp/asusfbsvcs/duplicate_log
dmesg -c >> /tmp/asusfbsvcs/duplicate_log/dmesg.txt

# Archer
if [ -e /bin/archerctl ]; then
    echo "###### archerctl status ######"
    archerctl status
    redirect_dmesg
    echo
    echo "###### archerctl stats ######"
    archerctl stats
    redirect_dmesg
    echo
    echo "###### archerctl sysport_tm stats ######"
    archerctl sysport_tm stats
    redirect_dmesg
fi

if [ -e /bin/ethctl ]; then
    echo
    echo "###### ethctl phy-map ######"
    ethctl phy-map
fi

if [ -e /proc/driver/phy/cmd ]; then
    echo
    echo "###### list > /proc/driver/phy/cmd ######"
    echo list > /proc/driver/phy/cmd
    redirect_dmesg
fi

mibdump=`nvram get eth_mibdump`
if [ "$mibdump" == "1" ]; then
if [ -e /bin/ethswctl ]; then
    echo
    echo "###### ethswctl -c mibdump -a ######"

    for i in 1 2 3
    do
        echo
        echo "###### $i ######"
        ethswctl -c mibdump -a
        redirect_dmesg
        if [ "$i" != "3" ]; then
        sleep 5
        fi
    done
fi
fi

if [ -e /bin/spuctl ]; then
    echo
    echo "###### spuctl showstats ######"
    spuctl showstats
    redirect_dmesg
fi

if [ -e /bin/iqctl ]; then
    echo
    echo "###### iqctl status ######"
    iqctl status
    redirect_dmesg
fi

if [ -e /bin/bpmctl ]; then
    echo
    echo "###### bpm status ######"
    bpmctl status
    redirect_dmesg
    echo
    echo "###### bpm thresh ######"
    bpmctl thresh
    redirect_dmesg
fi

if [ -e /bin/blogctl ]; then
    echo
    echo "###### blogctl stats ######"
    blogctl stats
    redirect_dmesg
fi

# RDPA configuration
if [ -e /bin/bs ]; then
    echo
    echo "######RDPA configuration######"
    /bin/bs /bdmf/examine system children:yes class:config max_prints:-1
    redirect_dmesg
fi

# ACSD
if [ -e /usr/sbin/acs_cli2 ]; then
    for ifname in `nvram get acs_ifnames`; do
        echo ""
        echo "###### ACSD $ifname information ######"
        echo ""
        acs_cli2 -i $ifname --report
    done
fi
