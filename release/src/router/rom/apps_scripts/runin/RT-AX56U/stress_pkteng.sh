#!/bin/sh

echo "!!! ATE RUN-IN START !!!"

# CPU (+ MEMORY ?)
echo "start stress..."
stress cpu -b 0 -t 30000 &
stress cpu -b 1 -t 30000 &
stress cpu -b 2 -t 30000 &
stress cpu -b 3 -t 30000 &

# 2G
echo "start 2.4GHz pkteng..."
wl -i eth5 mpc 0
wl -i eth5 down
wl -i eth5 country ALL
wl -i eth5 wsec 0
wl -i eth5 stbc_rx 1
wl -i eth5 scansuppress 1
wl -i eth5 ssid "AP0"
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
wl -i eth5 pkteng_start wl -i eth5 mpc 0
wl -i eth5 down
wl -i eth5 country ALL
wl -i eth5 wsec 0
wl -i eth5 stbc_rx 1
wl -i eth5 scansuppress 1
wl -i eth5 ssid "AP0"
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
wl -i eth5 pkteng_stop rx
wl -i eth5 pkteng_stop tx
wl -i eth5 mpc 0
wl -i eth5 isup
wl -i eth5 interference
wl -i eth5 interference_override 0
wl -i eth5 pkteng_maxlen
wl -i eth5 ap
wl -i eth5 ssid ""
wl -i eth5 pkteng_start 00:11:22:33:44:55 tx 30 1563 0 00:90:4C:32:B0:00

# 5G
echo "start 5GHz pkteng..."
wl -i eth6 mpc 0
wl -i eth6 down
wl -i eth6 country ALL
wl -i eth6 wsec 0
wl -i eth6 stbc_rx 1
wl -i eth6 scansuppress 1
wl -i eth6 ssid "AP1"
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
wl -i eth6 pkteng_stop rx
wl -i eth6 pkteng_stop tx
wl -i eth6 mpc 0
wl -i eth6 isup
wl -i eth6 interference
wl -i eth6 interference_override 0
wl -i eth6 pkteng_maxlen
wl -i eth6 ap
wl -i eth6 ssid ""
wl -i eth6 pkteng_start 00:11:22:33:44:55 tx 30 1563 0 30:17:AB:AB:A9:14

# Start temperature polling
touch "tmp/Ate_temp_rec_start"
