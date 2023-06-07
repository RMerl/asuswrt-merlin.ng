#!/bin/sh

echo "!!! ATE RUN-IN START !!!"

# CPU + MEMORY
echo "start stress..."
stress cpu -b 0 -t 30000 &
stress cpu -b 1 -t 30000 &
stress cpu -b 2 -t 30000 &
stress cpu -b 3 -t 30000 &

# 2G
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
wl -i wl0 bw_cap 2g 1
wl -i wl0 bi 65535
wl -i wl0 mimo_txbw -1
wl -i wl0 frameburst 1
wl -i wl0 spatial_policy 1
wl -i wl0 txcore  -s 1 -c 3
wl -i wl0 band b
wl -i wl0 chanspec 1/20
wl -i wl0 he features 3
wl -i wl0 up
wl -i wl0 phy_forcecal 1
wl -i wl0 phy_watchdog 0
wl -i wl0 2g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i wl0 txpwr1 -1
wl -i wl0 phy_forcecal 1
wl -i wl0 pkteng_start 00:11:22:33:44:55 tx 100 400 0 00:22:44:66:88:00


#5G1
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
wl -i wl1 bw_cap 5g 1
wl -i wl1 bi 65535
wl -i wl1 mimo_txbw -1
wl -i wl1 frameburst 1
wl -i wl1 spatial_policy 1
wl -i wl1 txcore  -s 1 -c 3
wl -i wl1 band a
wl -i wl1 chanspec 36/20
wl -i wl1 he features 3
wl -i wl1 up
wl -i wl1 phy_forcecal 1
wl -i wl1 phy_watchdog 0
wl -i wl1 5g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i wl1 txpwr1 -1
wl -i wl1 phy_forcecal 1
wl -i wl1 pkteng_start 00:11:22:33:44:55 tx 100 400 0 00:22:44:66:88:04

# Start temperature polling
touch "tmp/Ate_temp_rec_start"
