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
echo "start eth10 pkteng..."
wl -i eth10 mpc 0
wl -i eth10 down
wl -i eth10 country ALL
wl -i eth10 wsec 0
wl -i eth10 stbc_rx 1
wl -i eth10 scansuppress 1
wl -i eth10 ssid ""
wl -i eth10 txbf 0
wl -i eth10 spect 0
wl -i eth10 mbss 0
wl -i eth10 ampdu 0
wl -i eth10 PM 0
wl -i eth10 stbc_tx 0
wl -i eth10 bi 65535
wl -i eth10 mimo_txbw -1
wl -i eth10 frameburst 1
wl -i eth10 spatial_policy 1
wl -i eth10 txcore  -s 1 -c 15
wl -i eth10 band b
wl -i eth10 chanspec 1/20
wl -i eth10 he features 3
wl -i eth10 up
wl -i eth10 phy_forcecal 1
wl -i eth10 phy_watchdog 0
wl -i eth10 2g_rate -e 11 -s 1 -i 0 --ldpc -b 20
wl -i eth10 txpwr1 -q -o 88
wl -i eth10 phy_forcecal 1
wl -i eth10 pkteng_start 00:11:22:33:44:55 tx 100 179 0 00:22:44:66:88:00

# 5GL
echo "start eth7 pkteng..."
wl -i eth7 mpc 0
wl -i eth7 down
wl -i eth7 country ALL
wl -i eth7 wsec 0
wl -i eth7 stbc_rx 1
wl -i eth7 scansuppress 1
wl -i eth7 ssid ""
wl -i eth7 txbf 0
wl -i eth7 mbss 0
wl -i eth7 ampdu 0
wl -i eth7 PM 0
wl -i eth7 stbc_tx 0
wl -i eth7 bw_cap 5g 7
wl -i eth7 bi 65535
wl -i eth7 mimo_txbw -1
wl -i eth7 frameburst 1
wl -i eth7 spatial_policy 1
wl -i eth7 txcore  -s 1 -c 15
wl -i eth7 band a
wl -i eth7 chanspec 36/80
wl -i eth7 he features 3
wl -i eth7 up
wl -i eth7 phy_forcecal 1
wl -i eth7 phy_watchdog 0
wl -i eth7 5g_rate -e 11 -s 1 -i 0 --ldpc -b 80
wl -i eth7 txpwr1 -q -o 88
wl -i eth7 phy_forcecal 1
wl -i eth7 pkteng_start 00:11:22:33:44:55 tx 100 907 0 00:22:44:66:88:04

# 5GH
echo "start eth8 pkteng..."
wl -i eth8 mpc 0
wl -i eth8 down
wl -i eth8 country ALL
wl -i eth8 wsec 0
wl -i eth8 stbc_rx 1
wl -i eth8 scansuppress 1
wl -i eth8 ssid ""
wl -i eth8 txbf 0
wl -i eth8 mbss 0
wl -i eth8 ampdu 0
wl -i eth8 PM 0
wl -i eth8 stbc_tx 0
wl -i eth8 bw_cap 5g 7
wl -i eth8 bi 65535
wl -i eth8 mimo_txbw -1
wl -i eth8 frameburst 1
wl -i eth8 spatial_policy 1
wl -i eth8 txcore  -s 1 -c 15
wl -i eth8 band a
wl -i eth8 chanspec 149/80
wl -i eth8 he features 3
wl -i eth8 up
wl -i eth8 phy_forcecal 1
wl -i eth8 phy_watchdog 0
wl -i eth8 5g_rate -e 11 -s 1 -i 0 --ldpc -b 80
wl -i eth8 txpwr1 -q -o 88
wl -i eth8 phy_forcecal 1
wl -i eth8 pkteng_start 00:11:22:33:44:55 tx 100 907 0 00:22:44:66:88:04

# 6GH
echo "start eth9 pkteng..."
wl -i eth9 mpc 0
wl -i eth9 down
wl -i eth9 country ALL
wl -i eth9 wsec 0
wl -i eth9 stbc_rx 1
wl -i eth9 scansuppress 1
wl -i eth9 ssid ""
wl -i eth9 txbf 0
wl -i eth9 mbss 0
wl -i eth9 ampdu 0
wl -i eth9 PM 0
wl -i eth9 stbc_tx 0
wl -i eth9 bw_cap 6g 7
wl -i eth9 bi 65535
wl -i eth9 mimo_txbw -1
wl -i eth9 frameburst 1
wl -i eth9 spatial_policy 1
wl -i eth9 txcore  -s 1 -c 15
wl -i eth9 chanspec 6g37/80
wl -i eth9 he features 3
wl -i eth9 up
wl -i eth9 phy_forcecal 1
wl -i eth9 phy_watchdog 0
wl -i eth9 6g_rate -e 11 -s 1 -i 0 --ldpc -b 80
wl -i eth9 txpwr1 -q -o 88
wl -i eth9 phy_forcecal 1
wl -i eth9 pkteng_start 00:11:22:33:44:55 tx 100 907 0 00:22:44:66:88:04

# Start temperature polling
touch "tmp/Ate_temp_rec_start"
