#!/bin/sh

echo "!!! ATE RUN-IN START !!!"

# CPU (+ MEMORY ?)
echo "start stress..."
CPU_cores=`grep -c processor /proc/cpuinfo`
count=0
while [ $count -lt $CPU_cores ]; do
	stress cpu -b $count -t 30000 &
	count=`expr $count + 1`
done

# 2G
echo "start 2.4GHz pkteng..."
wl -i eth5 mpc 0
wl -i eth5 down
wl -i eth5 country ALL
wl -i eth5 wsec 0
wl -i eth5 stbc_rx 1
wl -i eth5 scansuppress 1
wl -i eth5 ssid ""
wl -i eth5 txbf 0
wl -i eth5 spect 0
wl -i eth5 mbss 0
wl -i eth5 ampdu 0
wl -i eth5 PM 0
wl -i eth5 stbc_tx 0
wl -i eth5 bw_cap 2g 1
wl -i eth5 bi 65535
wl -i eth5 mimo_txbw -1
wl -i eth5 frameburst 1
wl -i eth5 spatial_policy 1
wl -i eth5 txcore  -s 1 -c 3
wl -i eth5 band b
wl -i eth5 chanspec 1/20
wl -i eth5 he features 3
wl -i eth5 up
wl -i eth5 phy_forcecal 1
wl -i eth5 phy_watchdog 0
wl -i eth5 2g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i eth5 txpwr1 -1
wl -i eth5 phy_forcecal 1
wl -i eth5 pkteng_start 00:11:22:33:44:55 tx 100 400 0 00:22:44:66:88:00

# 5G
echo "start 5GHz pkteng..."
wl -i eth6 mpc 0
wl -i eth6 down
wl -i eth6 country ALL
wl -i eth6 wsec 0
wl -i eth6 stbc_rx 1
wl -i eth6 scansuppress 1
wl -i eth6 ssid ""
wl -i eth6 txbf 0
wl -i eth6 mbss 0
wl -i eth6 ampdu 0
wl -i eth6 PM 0
wl -i eth6 stbc_tx 0
wl -i eth6 bw_cap 5g 1
wl -i eth6 bi 65535
wl -i eth6 mimo_txbw -1
wl -i eth6 frameburst 1
wl -i eth6 spatial_policy 1
wl -i eth6 txcore  -s 1 -c 15
wl -i eth6 band a
wl -i eth6 chanspec 36/20
wl -i eth6 he features 3
wl -i eth6 up
wl -i eth6 phy_forcecal 1
wl -i eth6 phy_watchdog 0
wl -i eth6 5g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i eth6 txpwr1 -1
wl -i eth6 phy_forcecal 1
wl -i eth6 pkteng_start 00:11:22:33:44:55 tx 100 400 0 00:22:44:66:88:04

# Start temperature polling
touch "tmp/Ate_temp_rec_start"
