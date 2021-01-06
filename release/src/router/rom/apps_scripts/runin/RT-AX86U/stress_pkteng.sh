#!/bin/sh

echo "!!! ATE RUN-IN START !!!"

intf_2G=eth6
intf_5G=eth7

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
wl -i $intf_2G mpc 0
wl -i $intf_2G down
wl -i $intf_2G country ALL
wl -i $intf_2G wsec 0
wl -i $intf_2G stbc_rx 1
wl -i $intf_2G scansuppress 1
wl -i $intf_2G ssid ""
wl -i $intf_2G txbf 0
wl -i $intf_2G spect 0
wl -i $intf_2G mbss 0
wl -i $intf_2G ampdu 0
wl -i $intf_2G PM 0
wl -i $intf_2G stbc_tx 0
wl -i $intf_2G bw_cap 2g 1
wl -i $intf_2G bi 65535
wl -i $intf_2G mimo_txbw -1
wl -i $intf_2G frameburst 1
wl -i $intf_2G spatial_policy 1
wl -i $intf_2G txcore  -s 1 -c 3
wl -i $intf_2G band b
wl -i $intf_2G chanspec 1/20
wl -i $intf_2G he features 3
wl -i $intf_2G up
wl -i $intf_2G phy_forcecal 1
wl -i $intf_2G phy_watchdog 0
wl -i $intf_2G 2g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i $intf_2G txpwr1 -1
wl -i $intf_2G phy_forcecal 1
wl -i $intf_2G pkteng_start 00:11:22:33:44:55 tx 100 400 0 00:22:44:66:88:00

# 5G
echo "start 5GHz pkteng..."
wl -i $intf_5G mpc 0
wl -i $intf_5G down
wl -i $intf_5G country ALL
wl -i $intf_5G wsec 0
wl -i $intf_5G stbc_rx 1
wl -i $intf_5G scansuppress 1
wl -i $intf_5G ssid ""
wl -i $intf_5G txbf 0
wl -i $intf_5G mbss 0
wl -i $intf_5G ampdu 0
wl -i $intf_5G PM 0
wl -i $intf_5G stbc_tx 0
wl -i $intf_5G bw_cap 5g 1
wl -i $intf_5G bi 65535
wl -i $intf_5G mimo_txbw -1
wl -i $intf_5G frameburst 1
wl -i $intf_5G spatial_policy 1
wl -i $intf_5G txcore  -s 1 -c 15
wl -i $intf_5G band a
wl -i $intf_5G chanspec 36/20
wl -i $intf_5G he features 3
wl -i $intf_5G up
wl -i $intf_5G phy_forcecal 1
wl -i $intf_5G phy_watchdog 0
wl -i $intf_5G 5g_rate -e 7 -s 1 -i 1 --ldpc -b 20
wl -i $intf_5G txpwr1 -1
wl -i $intf_5G phy_forcecal 1
wl -i $intf_5G pkteng_start 00:11:22:33:44:55 tx 100 400 0 00:22:44:66:88:04

# Start temperature polling
touch "tmp/Ate_temp_rec_start"
