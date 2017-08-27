#!/bin/sh
USER=$(/usr/sbin/nvram get pptpd_client_srvuser)
PASS=$(/usr/sbin/nvram get pptpd_client_srvpass)
SEC=$(/usr/sbin/nvram get pptpd_client_srvsec)
MTU=$(/usr/sbin/nvram get pptpd_client_srvmtu)
MRU=$(/usr/sbin/nvram get pptpd_client_srvmru)
sleep 30
mkdir /tmp/pptpd_client
mkdir /tmp/ppp
cp /etc/config/pptpd_client.options /tmp/pptpd_client/options.vpn
echo "$SEC"  >> /tmp/pptpd_client/options.vpn
echo -n "mtu " >> /tmp/pptpd_client/options.vpn
echo "$MTU"  >> /tmp/pptpd_client/options.vpn
echo -n "mru " >> /tmp/pptpd_client/options.vpn
echo "$MRU"  >> /tmp/pptpd_client/options.vpn
echo -n "name "  >> /tmp/pptpd_client/options.vpn
echo "$USER" >> /tmp/pptpd_client/options.vpn
echo -n "password "  >> /tmp/pptpd_client/options.vpn
echo "$PASS" >> /tmp/pptpd_client/options.vpn
cp /etc/config/pptpd_client.vpn /tmp/pptpd_client/vpn
chmod +x /tmp/pptpd_client/vpn
cp /etc/config/pptpd_client.ip-up /tmp/pptpd_client/ip-up
chmod +x /tmp/pptpd_client/ip-up
cp /etc/config/pptpd_client.ip-down /tmp/pptpd_client/ip-down
chmod +x /tmp/pptpd_client/ip-down
pidof vpn || /tmp/pptpd_client/vpn start
