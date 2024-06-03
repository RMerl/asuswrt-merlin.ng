rm -rf /tmp/wapp_ctrl
killall -15 mapd
killall -15 wapp
killall -15 p1905_managerd
killall -15 bs20
switch vlan clear
switch vlan set 0 1 0000110
switch vlan set 0 2 1111001
switch clear

