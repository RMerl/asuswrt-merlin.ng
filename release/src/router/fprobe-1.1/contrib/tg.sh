#!/bin/bash

Q=10000

if [[ $# -ne 2 ]]; then
    echo -e "\
Usage: `basename $0` [target] [number]

This is simple traffic generator script for stress tests.
It based on the traceroute: ftp://ftp.ee.lbl.gov/traceroute.tar.gz
(FreeBSD and Debian Linux ships with this version).

Script will send number*$Q UDP packets
and, of course, receive number*$Q ICMP replys.

Note: for stress test you must turn off ICMP rate limit on target machine:
Linux: sysctl -w net/ipv4/icmp_ratelimit=0
FreeBSD: sysctl -w net.inet.icmp.icmplim=0
Solaris: /usr/sbin/ndd -set /dev/ip ip_icmp_err_interval 0
"
    exit
fi

H=$1
C=$2
c=0
s=$(date +%s)

while [[ $((C--)) -gt 0 ]]; do
    traceroute -n -q $Q $H &>/dev/null
    e=$(date +%s)
    l=$(($e-$s))
    c=$(($c+2*$Q))
    p=$(($c*10/($l*10+1)))
    echo -ne "$l sec. $p pkts/s       \r"
done 
echo

# v1.2 by sla@0n.ru
