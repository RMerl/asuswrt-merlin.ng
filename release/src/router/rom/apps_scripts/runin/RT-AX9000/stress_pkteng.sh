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
wl -i eth8 mpc 0
wl -i eth8 down
wl -i eth8 country ALL
wl -i eth8 wsec 0
wl -i eth8 stbc_rx 1
wl -i eth8 scansuppress 1
wl -i eth8 ssid ""
wl -i eth8 txbf 0
wl -i eth8 spect 0
wl -i eth8 mbss 0
wl -i eth8 ampdu 0
wl -i eth8 PM 0
wl -i eth8 stbc_tx 0
wl -i eth8 bw_cap 2g 1
wl -i eth8 bi 65535
wl -i eth8 mimo_txbw -1
wl -i eth8 frameburst 1
wl -i eth8 spatial_policy 1
wl -i eth8 txcore  -s 1 -c 3
wl -i eth8 band b
wl -i eth8 chanspec 1/20
wl -i eth8 he features 3
wl -i eth8 up
wl -i eth8 phy_forcecal 1
wl -i eth8 phy_watchdog 0
wl -i eth8 2g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i eth8 txpwr1 -1
wl -i eth8 phy_forcecal 1
wl -i eth8 pkteng_start 00:11:22:33:44:55 tx 100 400 0 00:22:44:66:88:00


#5G1
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
wl -i eth6 txcore  -s 1 -c 3
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


# 5G2
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
wl -i eth7 bw_cap 5g 1
wl -i eth7 bi 65535
wl -i eth7 mimo_txbw -1
wl -i eth7 frameburst 1
wl -i eth7 spatial_policy 1
wl -i eth7 txcore  -s 1 -c 15
wl -i eth7 band a
wl -i eth7 chanspec 149/20
wl -i eth7 he features 3
wl -i eth7 up
wl -i eth7 phy_forcecal 1
wl -i eth7 phy_watchdog 0
wl -i eth7 5g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i eth7 txpwr1 -1
wl -i eth7 phy_forcecal 1
wl -i eth7 pkteng_start 00:11:22:33:44:55 tx 100 400 0 00:22:44:66:88:04


# Start temperature polling
touch "tmp/Ate_temp_rec_start"
