#!/bin/sh
#
# simple testing for cttimeout infrastructure using one single computer
#

WAIT_BETWEEN_TESTS=10

# flush cttimeout table
nfct flush timeout

# flush the conntrack table
conntrack -F

#
# No.1: test generic timeout policy
#

echo "---- test no. 1 ----"

conntrack -E -p 13 &

nfct add timeout test-generic inet generic timeout 100
iptables -I OUTPUT -t raw -p all -j CT --timeout test-generic
hping3 -c 1 -V -I eth0 -0 8.8.8.8 -H 13

killall -15 conntrack

echo "---- end test no. 1 ----"

sleep $WAIT_BETWEEN_TESTS

iptables -D OUTPUT -t raw -p all -j CT --timeout test-generic
nfct del timeout test-generic

#
# No.2: test TCP timeout policy
#

echo "---- test no. 2 ----"

conntrack -E -p tcp &

nfct add timeout test-tcp inet tcp syn_sent 100
iptables -I OUTPUT -t raw -p tcp -j CT --timeout test-tcp
hping3 -V -S -p 80 -s 5050 8.8.8.8 -c 1

sleep $WAIT_BETWEEN_TESTS

iptables -D OUTPUT -t raw -p tcp -j CT --timeout test-tcp
nfct del timeout test-tcp

killall -15 conntrack

echo "---- end test no. 2 ----"

#
# No. 3: test ICMP timeout policy
#

echo "---- test no. 3 ----"

conntrack -E -p icmp &

nfct add timeout test-icmp inet icmp timeout 50
iptables -I OUTPUT -t raw -p icmp -j CT --timeout test-icmp
hping3 -1 8.8.8.8 -c 2

iptables -D OUTPUT -t raw -p icmp -j CT --timeout test-icmp
nfct del timeout test-icmp

killall -15 conntrack

echo "---- end test no. 3 ----"
