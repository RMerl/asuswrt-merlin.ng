#!/bin/sh

echo "!!! ATE RUN-IN START !!!"

# CPU + MEMORY
echo "start stress..."
CPU_cores=`grep -c processor /proc/cpuinfo`
count=0
while [ $count -lt $CPU_cores ]; do
	stress cpu -b $count -t 30000 &
	count=`expr $count + 1`
done

# 2G
echo "start wl0 pkteng..."
wl -i wl0 mpc 0
wl -i wl0 down
wl -i wl0 country ALL
wl -i wl0 wsec 0
wl -i wl0 stbc_rx 1
wl -i wl0 scansuppress 1
wl -i wl0 ssid ""
wl -i wl0 txbf 0
wl -i wl0 spect 0
wl -i wl0 mbss 0
wl -i wl0 ampdu 0
wl -i wl0 PM 0
wl -i wl0 stbc_tx 0
wl -i wl0 bi 65535
wl -i wl0 mimo_txbw -1
wl -i wl0 frameburst 1
wl -i wl0 spatial_policy 1
wl -i wl0 txcore -s 1 -c 15
wl -i wl0 band b
wl -i wl0 chanspec 1/20
wl -i wl0 eht features 7
wl -i wl0 up
wl -i wl0 phy_forcecal 1
wl -i wl0 phy_watchdog 0
wl -i wl0 2g_rate -e 11 -s 1 -i 0 --ldpc -b 20
wl -i wl0 txpwr1 -q -o 88
wl -i wl0 phy_forcecal 1
wl -i wl0 pkteng_start 00:11:22:33:44:55 tx 100 179 0 00:22:44:66:88:00

# 5GL
echo "start wl1 pkteng..."
wl -i wl1 mpc 0
wl -i wl1 down
wl -i wl1 country ALL
wl -i wl1 wsec 0
wl -i wl1 stbc_rx 1
wl -i wl1 scansuppress 1
wl -i wl1 ssid ""
wl -i wl1 txbf 0
wl -i wl1 mbss 0
wl -i wl1 ampdu 0
wl -i wl1 PM 0
wl -i wl1 stbc_tx 0
wl -i wl1 bw_cap 5g 7
wl -i wl1 bi 65535
wl -i wl1 mimo_txbw -1
wl -i wl1 frameburst 1
wl -i wl1 spatial_policy 1
wl -i wl1 txcore  -s 1 -c 15
wl -i wl1 band a
wl -i wl1 chanspec 36/80
wl -i wl1 eht features 7
wl -i wl1 up
wl -i wl1 phy_forcecal 1
wl -i wl1 phy_watchdog 0
wl -i wl1 5g_rate -e 11 -s 1 -i 0 --ldpc -b 80
wl -i wl1 txpwr1 -q -o 88
wl -i wl1 phy_forcecal 1
wl -i wl1 pkteng_start 00:11:22:33:44:55 tx 100 907 0 00:22:44:66:88:04

# 6G
echo "start wl2 pkteng..."
wl -i wl2 mpc 0
wl -i wl2 down
wl -i wl2 country ALL
wl -i wl2 wsec 0
wl -i wl2 stbc_rx 1
wl -i wl2 scansuppress 1
wl -i wl2 ssid ""
wl -i wl2 txbf 0
wl -i wl2 mbss 0
wl -i wl2 ampdu 0
wl -i wl2 PM 0
wl -i wl2 stbc_tx 0
wl -i wl2 bw_cap 6g 15
wl -i wl2 bi 65535
wl -i wl2 mimo_txbw -1
wl -i wl2 frameburst 1
wl -i wl2 spatial_policy 1
wl -i wl2 txcore  -s 1 -c 15
wl -i wl2 band 6g
wl -i wl2 chanspec 6g161/20
wl -i wl2 eht features 7
wl -i wl2 up
wl -i wl2 phy_forcecal 1
wl -i wl2 phy_watchdog 0
wl -i wl2 6g_rate -t 11 -s 1 -i 1 --ldpc -b 20
wl -i wl2 txpwr1 -q -o 88
wl -i wl2 phy_forcecal 1
wl -i wl2 pkteng_start 00:11:22:33:44:55 tx 100 907 0 00:22:44:66:88:04

# Start temperature polling
touch "tmp/Ate_temp_rec_start"
