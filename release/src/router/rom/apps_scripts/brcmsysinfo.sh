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
if [ -e /bin/vlanctl ]; then
   echo
   echo "###### dump all vlanctl rules ######"
   vlanctl --rule-dump-all
fi

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
dump_br_showmacs()
{
    i=0

    while [ "$i" -le 255 ]
    do
        br="br$i"
        path="/sys/devices/virtual/net/$br"

        if [ -d "$path" ]; then
            echo
            echo "###### brctl showmacs $br ######"
            brctl showmacs "$br"
        fi

        i=$((i + 1))
    done
}
dump_br_showmacs

echo
echo "######route -n######"
route -n

echo
echo "######route -A inet6######"
route -A inet6

parse_tcp_udp()
{
    echo
    echo "###### RAW: /proc/net/tcp ######"
    if [ -e /proc/net/tcp ]; then
        cat /proc/net/tcp
    else
        echo "/proc/net/tcp does not exist on this system."
    fi

    echo
    echo "###### RAW: /proc/net/udp ######"
    if [ -e /proc/net/udp ]; then
        cat /proc/net/udp
    else
        echo "/proc/net/udp does not exist on this system."
    fi

    echo
    echo "###### PARSED: /proc/net/tcp /proc/net/udp ######"

    awk '
    function h2d(h, i,c,n) {
      n=0
      for (i=1;i<=length(h);i++) {
        c=substr(h,i,1)
        if (c>="0" && c<="9") n=n*16+c
        else if (c>="A" && c<="F") n=n*16+index("ABCDEF",c)+9
        else if (c>="a" && c<="f") n=n*16+index("abcdef",c)+9
      }
      return n
    }

    function ip4(h) {
      return h2d(substr(h,7,2)) "." h2d(substr(h,5,2)) "." h2d(substr(h,3,2)) "." h2d(substr(h,1,2))
    }

    BEGIN {
      st["01"]="ESTABLISHED"; st["02"]="SYN_SENT"; st["03"]="SYN_RECV"
      st["04"]="FIN_WAIT1";   st["05"]="FIN_WAIT2"; st["06"]="TIME_WAIT"
      st["07"]="CLOSE";       st["08"]="CLOSE_WAIT";st["09"]="LAST_ACK"
      st["0A"]="LISTEN";      st["0B"]="CLOSING"
    }

    FNR==1 {
      proto=(FILENAME ~ /udp$/) ? "UDP" : "TCP"
      next
    }

    {
      split($2,l,":")
      split($3,r,":")
      s = ($4 in st) ? st[$4] : $4

      printf "%-3s Local: %-22s Remote: %-22s State: %s\n", \
        proto, ip4(l[1]) ":" h2d(l[2]), ip4(r[1]) ":" h2d(r[2]), s
    }
    ' /proc/net/tcp /proc/net/udp
}

parse_tcp_udp

echo
echo "######ip -f inet neigh######"
ip -f inet neigh

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

dump_raw()
{
    f="$1"
    echo
    echo "###### RAW: $f ######"
    if [ -e "$f" ]; then
        cat "$f"
    else
        echo "$f does not exist on this system."
    fi
}

parse_igmp()
{
    f="$1"
    echo
    echo "###### PARSED: $f ######"

    awk '
    function h2d(h, i,c,n) {
      n=0
      for (i=1;i<=length(h);i++) {
        c=substr(h,i,1)
        if (c>="0" && c<="9") n=n*16+c
        else if (c>="A" && c<="F") n=n*16+index("ABCDEF",c)+9
        else if (c>="a" && c<="f") n=n*16+index("abcdef",c)+9
      }
      return n
    }
    function ip4le(h) {
      return h2d(substr(h,7,2)) "." h2d(substr(h,5,2)) "." h2d(substr(h,3,2)) "." h2d(substr(h,1,2))
    }
    BEGIN {
      name["224.0.0.1"]="all-hosts"
      name["224.0.0.2"]="all-routers"
      name["224.0.0.22"]="IGMPv3"
      name["224.0.0.251"]="mDNS"
      name["224.0.0.252"]="LLMNR"
      name["239.255.255.250"]="SSDP/UPnP"

      printf "%-8s %-15s %-18s %-7s %-14s %-8s\n", "Device", "Group", "Name", "Users", "Timer", "Reporter"
    }
    $0 ~ /^[0-9]+[ \t]+/ {
      dev=$2
      sub(/:$/,"",dev)
      next
    }
    $1 ~ /^[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]$/ {
      ip=ip4le($1)
      printf "%-8s %-15s %-18s %-7s %-14s %-8s\n", dev, ip, ((ip in name)?name[ip]:"-"), $2, $3, $4
    }
    ' "$f"
}

parse_ip_mr_cache()
{
    f="$1"
    echo
    echo "###### PARSED: $f ######"

    awk '
    function h2d(h, i,c,n) {
      n=0
      for (i=1;i<=length(h);i++) {
        c=substr(h,i,1)
        if (c>="0" && c<="9") n=n*16+c
        else if (c>="A" && c<="F") n=n*16+index("ABCDEF",c)+9
        else if (c>="a" && c<="f") n=n*16+index("abcdef",c)+9
      }
      return n
    }
    function ip4le(h) {
      return h2d(substr(h,7,2)) "." h2d(substr(h,5,2)) "." h2d(substr(h,3,2)) "." h2d(substr(h,1,2))
    }
    NR==1 {
      printf "%-15s %-15s %4s %8s %10s %8s %s\n", $1, $2, $3, $4, $5, $6, "Oifs"
      next
    }
    {
      printf "%-15s %-15s %4s %8s %10s %8s", ip4le($1), ip4le($2), $3, $4, $5, $6
      for (i=7;i<=NF;i++) printf " %s", $i
      printf "\n"
    }
    ' "$f"
}

parse_ip_mr_vif()
{
    f="$1"
    echo
    echo "###### PARSED: $f ######"

    awk '
    function h2d(h, i,c,n) {
      n=0
      for (i=1;i<=length(h);i++) {
        c=substr(h,i,1)
        if (c>="0" && c<="9") n=n*16+c
        else if (c>="A" && c<="F") n=n*16+index("ABCDEF",c)+9
        else if (c>="a" && c<="f") n=n*16+index("abcdef",c)+9
      }
      return n
    }
    function ip4le(h) {
      return h2d(substr(h,7,2)) "." h2d(substr(h,5,2)) "." h2d(substr(h,3,2)) "." h2d(substr(h,1,2))
    }
    NR==1 {
      printf "%3s %-10s %10s %8s %10s %8s %-7s %-15s %-15s\n", \
        "Vif", $1, $2, $3, $4, $5, $6, $7, $8
      next
    }
    {
      printf "%3s %-10s %10s %8s %10s %8s %-7s %-15s %-15s\n", \
        $1, $2, $3, $4, $5, $6, $7, ip4le($8), ip4le($9)
    }
    ' "$f"
}

parse_dev_mcast()
{
    f="$1"
    echo
    echo "###### PARSED: $f ######"

    awk '
    function macfmt(s, i,out) {
      out=""
      for (i=1;i<=length(s);i+=2) {
        if (out!="") out=out ":"
        out=out substr(s,i,2)
      }
      return out
    }
    BEGIN {
      name["01:00:5e:00:00:01"]="IPv4 all-hosts 224.0.0.1"
      name["01:00:5e:00:00:02"]="IPv4 all-routers 224.0.0.2"
      name["01:00:5e:00:00:16"]="IPv4 IGMPv3 224.0.0.22"
      name["01:00:5e:00:00:fb"]="IPv4 mDNS 224.0.0.251"
      name["01:00:5e:00:00:fc"]="IPv4 LLMNR 224.0.0.252"
      name["01:00:5e:7f:ff:fa"]="IPv4 SSDP 239.255.255.250"

      name["33:33:00:00:00:01"]="IPv6 all-nodes ff02::1"
      name["33:33:00:00:00:02"]="IPv6 all-routers ff02::2"
      name["33:33:00:00:00:fb"]="IPv6 mDNS ff02::fb"
    }
    NR==1 {
      printf "%-5s %-8s %-8s %-8s %-17s %s\n", "Idx", "Device", "Users", "Global", "Address", "Name"
      next
    }
    {
      mac=macfmt($5)
      printf "%-5s %-8s %-8s %-8s %-17s %s\n", $1, $2, $3, $4, mac, ((mac in name)?name[mac]:"-")
    }
    ' "$f"
}

parse_igmp6()
{
    f="$1"
    echo
    echo "###### PARSED: $f ######"

    awk '
    function trim0(s) {
      sub(/^0+/, "", s)
      return s == "" ? "0" : s
    }

    function ipv6_short(h, i,g,n,best_s,best_l,cur_s,cur_l,out,part) {
      n=0
      for (i=1;i<=32;i+=4) {
        n++
        g[n]=substr(h,i,4)
      }

      best_s=0; best_l=0; cur_s=0; cur_l=0
      for (i=1;i<=8;i++) {
        if (g[i]=="0000") {
          if (cur_l==0) cur_s=i
          cur_l++
          if (cur_l>best_l) {
            best_s=cur_s
            best_l=cur_l
          }
        } else {
          cur_l=0
        }
      }

      if (best_l < 2) best_s=0

      out=""
      for (i=1;i<=8;i++) {
        if (best_s && i==best_s) {
          if (out=="") out="::"
          else out=out ":"
          i += best_l - 1
          continue
        }

        part=trim0(g[i])
        if (out!="" && out!="::") out=out ":"
        out=out part
      }

      if (out=="") out="::"
      return out
    }

    function mgroup(h, ip) {
      ip=ipv6_short(h)

      if (ip=="ff01::1")  return "All IPv6 nodes within the node-local scope"
      if (ip=="ff01::2")  return "All IPv6 routers within the node-local scope"
      if (ip=="ff02::1")  return "All IPv6 nodes within the link-local scope"
      if (ip=="ff02::2")  return "All IPv6 routers within the link-local scope"
      if (ip=="ff02::5")  return "All OSPFv3 routers within the link-local scope"
      if (ip=="ff02::6")  return "All OSPFv3 designated routers within the link-local scope"
      if (ip=="ff02::9")  return "All RIPng routers within the link-local scope"
      if (ip=="ff02::a")  return "All EIGRP routers within the link-local scope"
      if (ip=="ff02::d")  return "All PIM routers within the link-local scope"
      if (ip=="ff02::16") return "All MLDv2-capable routers"
      if (ip=="ff02::1:2") return "All DHCPv6 agents within the link-local scope"
      if (ip=="ff05::2")  return "All IPv6 routers within the site-local scope"

      if (substr(h,1,26)=="ff0200000000000000000001ff")
        return "IPv6 solicited-node multicast address within the link-local scope"

      return "-"
    }

    BEGIN {
      printf "%-5s %-8s %-25s %-45s %-8s %-10s %s\n", \
        "Idx", "Device", "Multicast Address", "Multicast Group", "Users", "Flags", "Timer"
    }

    NF >= 6 {
      addr=ipv6_short($3)
      printf "%-5s %-8s %-25s %-45s %-8s %-10s %s\n", \
        $1, $2, addr, mgroup($3), $4, $5, $6
    }
    ' "$f"
}

parse_ip6_mr_cache()
{
    f="$1"
    echo
    echo "###### PARSED: $f ######"

    awk '
    function ipv6(h, i,out) {
      out=""
      for (i=1;i<=32;i+=4) {
        if (out!="") out=out ":"
        out=out substr(h,i,4)
      }
      return out
    }
    NR==1 {
      printf "%-39s %-39s %-5s %-10s %-10s %s\n", $1, $2, $3, $4, $5, "Wrong Oifs"
      next
    }
    {
      printf "%-39s %-39s %-5s %-10s %-10s", ipv6($1), ipv6($2), $3, $4, $5
      for (i=6;i<=NF;i++) printf " %s", $i
      printf "\n"
    }
    ' "$f"
}

parse_ip6_mr_vif()
{
    f="$1"
    echo
    echo "###### PARSED: $f ######"

    awk '
    NR==1 {
      printf "%-5s %-8s %-15s %-15s %-8s %-8s %-10s %-10s %-8s\n", \
        $1, $2, $3, $4, $5, $6, $7, $8, $9
      next
    }
    {
      printf "%-5s %-8s %-15s %-15s %-8s %-8s %-10s %-10s %-8s\n", \
        $1, $2, $3, $4, $5, $6, $7, $8, $9
    }
    ' "$f"
}

parse_file()
{
    f="$1"

    if [ ! -e "$f" ]; then
        return
    fi

    case "$f" in
        /proc/net/igmp)
            parse_igmp "$f"
            ;;
        /proc/net/ip_mr_cache)
            parse_ip_mr_cache "$f"
            ;;
        /proc/net/ip_mr_vif)
            parse_ip_mr_vif "$f"
            ;;
        /proc/net/dev_mcast)
            parse_dev_mcast "$f"
            ;;
        /proc/net/igmp6)
            parse_igmp6 "$f"
            ;;
        /proc/net/ip6_mr_cache)
            parse_ip6_mr_cache "$f"
            ;;
        /proc/net/ip6_mr_vif)
            parse_ip6_mr_vif "$f"
            ;;
    esac
}

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
    echo "###### archerctl flows --all ######"
    archerctl flows --all
    redirect_dmesg
    echo
    echo "###### archerctl host ######"
    archerctl host
    redirect_dmesg
    echo
    echo "###### archerctl stats ######"
    archerctl stats
    redirect_dmesg
    echo
    echo "###### archerctl sysport_tm stats ######"
    archerctl sysport_tm stats
    redirect_dmesg
    echo
    echo "###### archerctl sysport --reg ######"
    archerctl sysport --reg
    redirect_dmesg
    echo
    echo "###### archerctl sysport --port ######"
    archerctl sysport --port
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

nomibdump=`nvram get nomibdump`
if [ "$nomibdump" != "1" ]; then
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

if [ -e /bin/tmctl ]; then
    echo "###### tmctl queue cfg & stats ######"
    intf_list=$(ifconfig -a | grep -E "^(eth|atm|ptm)" | cut -d" " -f1 | grep -v "\.")
    for intf in $intf_list; do
        echo "###### $intf ######"
        if [[ $(expr match "$intf" 'eth*') != 0 ]]; then
            dev_type=0
            max_qid=7
        elif [[ $(expr match "$intf" 'ptm*') != 0 ]]; then
            dev_type=3
            max_qid=7
        elif [[ $(expr match "$intf" 'atm*') != 0 ]]; then
            dev_type=3
            max_qid=15
        else
            echo "Unsupported interface..."
            continue
        fi

        if [ -e /sys/class/net/$intf/device/uevent ]; then
            if grep -q -i "usb_interface" /sys/class/net/$intf/device/uevent; then
                echo "tmctl currently does not support USB Ethernet interface $intf."
                continue
            fi
        fi

        tmctl getporttmparms --devtype $dev_type --if $intf
        tmctl getportshaper  --devtype $dev_type --if $intf
        for i in $(seq 0 $max_qid); do
            echo "###### $intf Q$i ######"
            tmctl getqcfg     --devtype $dev_type --if $intf --qid $i
            tmctl getqdropalg --devtype $dev_type --if $intf --qid $i
            tmctl getqstats   --devtype $dev_type --if $intf --qid $i
        done
    done
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
