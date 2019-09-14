#!/bin/bash
# add 2018-06-29 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imudp/.libs/imudp")
input(type="imudp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")

global(localHostname="localhost")

template(name="outfmt" type="string" string="%PRI%,%syslogfacility-text%,%syslogseverity-text%,%programname%,%syslogtag%,%msg%\n")

ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
	       template="outfmt")
}

'
startup
tcpflood -m1 -T "udp" -M "\"<14> 05/21/2017:00:00:00 GMT HOSTNAME 1-ABC-2 : default SSLLOG SSL_HANDSHAKE_SUCCESS 39672436 0 :  SPCBId 6377757 - ClientIP 192.168.0.11 - ClientPort 55073 - VserverServiceIP 192.168.0.11 - VserverServicePort 443 - ClientVersion TLSv1.0 - CipherSuite \\\"AES-256-CBC-SHA TLSv1 Non-Export 256-bit\\\" - Session Reuse    The authenti\""
tcpflood -m1 -T "udp" -M "\"<14>123456789: HOSTNAME: May 21 12:00:01.123 gmt: %IOSXE-6-PLATFORM: F0: cpp_cp: QFP:0.0 Thread:105 TS:00000000000000 %NAT-6-LOG_TRANSLATION: Created Translation UDP 192.168.0.11:44593 192.168.0.11:21129 192.168.0.11:53 192.168.0.11:53 0................\""
tcpflood -m1 -T "udp" -M "\"<14>May 21 2017 00:00:00: %ASA-4-102030: Deny udp src vlan_12302:192.168.0.11/514 dst vlan_1233:192.168.0.11/514 by access-group \\\"local_in\\\" [0x0, 0x0]\""
tcpflood -m1 -T "udp" -M "\"<14>May 21 2017 00:00:00: %ASA-6-102030: SFR requested ASA to bypass further packet redirection and process TCP flow from vlan_1233:192.168.0.11/10469 to vlan_12323:192.168.0.11/443 locally\""
tcpflood -m1 -T "udp" -M "\"<14>2017-05-21T00:00:01.123Z hostname.domain Hostd: verbose hostd[81480B70] [Originator@6876 sub=Hostsvc.StorageSystem] SendStorageInfoEvent: Notify: StorageSystemMsg{HBAs=[vmhba0, vmhba1, vmhba2, vmhba3, vmhba32, vmhba4, ]};\""
tcpflood -m1 -T "udp" -M "\"<14>2017-05-21T00:00:01.123Z hostname.domain Rhttpproxy: verbose rhttpproxy[479C1B70] [Originator@6876 sub=Proxy Req 69725] Resolved endpoint : [N7Vmacore4Http16LocalServiceSpecE:0x00000000] _serverNamespace = /vpxa _isRedirect = false _port = 0000000000\""
tcpflood -m1 -T "udp" -M "\"<14>May 21 12:00:01 hostname CROND[12393]: pam_unix(crond:session): session closed for user root................\""
tcpflood -m1 -T "udp" -M "\"<14>May 21 12:00:01 hostname MSWinEventLog	1	N/A	113977	Sun May 21 12:00:01.123	N/A	nxlog	N/A	N/A	N/A	hostname	N/A		reconnecting to agent manager in 200 seconds	N/A\""
shutdown_when_empty
wait_shutdown

echo '14,user,info,,, 05/21/2017:00:00:00 GMT HOSTNAME 1-ABC-2 : default SSLLOG SSL_HANDSHAKE_SUCCESS 39672436 0 :  SPCBId 6377757 - ClientIP 192.168.0.11 - ClientPort 55073 - VserverServiceIP 192.168.0.11 - VserverServicePort 443 - ClientVersion TLSv1.0 - CipherSuite "AES-256-CBC-SHA TLSv1 Non-Export 256-bit" - Session Reuse    The authenti
14,user,info,123456789,123456789:, HOSTNAME: May 21 12:00:01.123 gmt: %IOSXE-6-PLATFORM: F0: cpp_cp: QFP:0.0 Thread:105 TS:00000000000000 %NAT-6-LOG_TRANSLATION: Created Translation UDP 192.168.0.11:44593 192.168.0.11:21129 192.168.0.11:53 192.168.0.11:53 0................
14,user,info,%ASA-4-102030,%ASA-4-102030:, Deny udp src vlan_12302:192.168.0.11/514 dst vlan_1233:192.168.0.11/514 by access-group "local_in" [0x0, 0x0]
14,user,info,%ASA-6-102030,%ASA-6-102030:, SFR requested ASA to bypass further packet redirection and process TCP flow from vlan_1233:192.168.0.11/10469 to vlan_12323:192.168.0.11/443 locally
14,user,info,Hostd,Hostd:, verbose hostd[81480B70] [Originator@6876 sub=Hostsvc.StorageSystem] SendStorageInfoEvent: Notify: StorageSystemMsg{HBAs=[vmhba0, vmhba1, vmhba2, vmhba3, vmhba32, vmhba4, ]};
14,user,info,Rhttpproxy,Rhttpproxy:, verbose rhttpproxy[479C1B70] [Originator@6876 sub=Proxy Req 69725] Resolved endpoint : [N7Vmacore4Http16LocalServiceSpecE:0x00000000] _serverNamespace = /vpxa _isRedirect = false _port = 0000000000
14,user,info,CROND,CROND[12393]:, pam_unix(crond:session): session closed for user root................
14,user,info,MSWinEventLog#0111#011N,MSWinEventLog#0111#011N/A#011113977#011Sun, May 21 12:00:01.123#011N/A#011nxlog#011N/A#011N/A#011N/A#011hostname#011N/A#011#011reconnecting to agent manager in 200 seconds#011N/A' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
