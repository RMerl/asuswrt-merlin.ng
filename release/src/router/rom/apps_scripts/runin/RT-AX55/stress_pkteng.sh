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
wl -i eth2 mpc 0
wl -i eth2 down
wl -i eth2 country ALL
wl -i eth2 wsec 0
wl -i eth2 stbc_rx 1
wl -i eth2 scansuppress 1
wl -i eth2 ssid "AP0"
wl -i eth2 txbf 0
wl -i eth2 spect 0
wl -i eth2 mbss 0
wl -i eth2 ampdu 0
wl -i eth2 PM 0
wl -i eth2 stbc_tx 0
wl -i eth2 bw_cap 2g 1
wl -i eth2 bi 65535
wl -i eth2 mimo_txbw -1
wl -i eth2 frameburst 1
wl -i eth2 spatial_policy 1
wl -i eth2 txcore  -s 1 -c 3
wl -i eth2 band b
wl -i eth2 chanspec 1/20
wl -i eth2 he features 3
wl -i eth2 up
wl -i eth2 phy_forcecal 1
wl -i eth2 phy_watchdog 0
wl -i eth2 2g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i eth2 txpwr1 -1
wl -i eth2 phy_forcecal 1
wl -i eth2 pkteng_start wl -i eth2 mpc 0
wl -i eth2 down
wl -i eth2 country ALL
wl -i eth2 wsec 0
wl -i eth2 stbc_rx 1
wl -i eth2 scansuppress 1
wl -i eth2 ssid "AP0"
wl -i eth2 txbf 0
wl -i eth2 spect 0
wl -i eth2 mbss 0
wl -i eth2 ampdu 0
wl -i eth2 PM 0
wl -i eth2 stbc_tx 0
wl -i eth2 bw_cap 2g 1
wl -i eth2 bi 65535
wl -i eth2 mimo_txbw -1
wl -i eth2 frameburst 1
wl -i eth2 spatial_policy 1
wl -i eth2 txcore  -s 1 -c 3
wl -i eth2 band b
wl -i eth2 chanspec 1/20
wl -i eth2 he features 3
wl -i eth2 up
wl -i eth2 phy_forcecal 1
wl -i eth2 phy_watchdog 0
wl -i eth2 2g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i eth2 txpwr1 -1
wl -i eth2 phy_forcecal 1
wl -i eth2 pkteng_stop rx
wl -i eth2 pkteng_stop tx
wl -i eth2 mpc 0
wl -i eth2 isup
wl -i eth2 interference
wl -i eth2 interference_override 0
wl -i eth2 pkteng_maxlen
wl -i eth2 ap
wl -i eth2 ssid ""
wl -i eth2 pkteng_start 00:11:22:33:44:55 tx 30 1563 0 00:90:4C:32:B0:00

# 5G
echo "start 5GHz pkteng..."
wl -i eth3 mpc 0
wl -i eth3 down
wl -i eth3 country ALL
wl -i eth3 wsec 0
wl -i eth3 stbc_rx 1
wl -i eth3 scansuppress 1
wl -i eth3 ssid "AP1"
wl -i eth3 txbf 0
wl -i eth3 mbss 0
wl -i eth3 ampdu 0
wl -i eth3 PM 0
wl -i eth3 stbc_tx 0
wl -i eth3 bw_cap 5g 1
wl -i eth3 bi 65535
wl -i eth3 mimo_txbw -1
wl -i eth3 frameburst 1
wl -i eth3 spatial_policy 1
wl -i eth3 txcore  -s 1 -c 3
wl -i eth3 band a
wl -i eth3 chanspec 36/20
wl -i eth3 he features 3
wl -i eth3 up
wl -i eth3 phy_forcecal 1
wl -i eth3 phy_watchdog 0
wl -i eth3 5g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i eth3 txpwr1 -1
wl -i eth3 phy_forcecal 1
wl -i eth3 pkteng_stop rx
wl -i eth3 pkteng_stop tx
wl -i eth3 mpc 0
wl -i eth3 isup
wl -i eth3 interference
wl -i eth3 interference_override 0
wl -i eth3 pkteng_maxlen
wl -i eth3 ap
wl -i eth3 ssid ""
wl -i eth3 pkteng_start 00:11:22:33:44:55 tx 30 1563 0 30:17:AB:AB:A9:14

# Start temperature polling
touch "tmp/Ate_temp_rec_start"
