#!/bin/bash
# add 2018-06-29 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../contrib/pmsnare/.libs/pmsnare")
module(load="../plugins/imudp/.libs/imudp")
input(type="imudp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")

global(localHostname="localhost")

template(name="outfmt" type="string" string="%PRI%,%syslogfacility-text%,%syslogseverity-text%,%programname%,%syslogtag%,%msg%\n")

ruleset(name="ruleset1" parser=["rsyslog.snare","rsyslog.rfc5424","rsyslog.rfc3164"]) {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
	       template="outfmt")
}

'
startup
tcpflood -m1 -T "udp" -M "\"<14> 05/21/2017:00:00:00 GMT HOSTNAME 1-ABC-2 : default SSLLOG SSL_HANDSHAKE_SUCCESS 39672436 0 :  SPCBId 6377757 - ClientIP 192.168.0.11 - ClientPort 55073 - VserverServiceIP 192.168.0.11 - VserverServicePort 443 - ClientVersion TLSv1.0 - CipherSuite \\\"AES-256-CBC-SHA TLSv1 Non-Export 256-bit\\\" - Session Reuse    The authenti\""
tcpflood -m1 -T "udp" -M "\"<14>May 21 12:00:01 hostname.domain MSWinEventLog	1	Security	00000000	Sun May 21 12:00:01.123	4624	Microsoft-Windows-Security-Auditing	N/A	N/A	Success Audit	hostname.domain	Logon		An account was successfully logged on.    Subject:   Security ID:  0x000000000   Account Name:  first.last   Account Domain:  domain   Logon ID:  0x000000000    Logon Type:   3    New Logon:   Security ID:  0x000000000   Account Name:  first.last   Account Domain:  domain   Logon ID:  0x000000000   Logon GUID:  0x000000000    Process Information:   Process ID:  0x000000000   Process Name:  first.last    Network Information:   Workstation Name:    Source Network Address: 192.168.0.11   Source Port:  51542    Detailed Authentication Information:   Logon Process:  Kerberos   Authentication Package: Kerberos   Transited Services: -   Package Name (NTLM only): -   Key Length:  0    This event is generated when a logon session is created. It is generated on the computer that was accessed.    The subject fields indicate the account on the local system which requested the logon. This is most commonly a service such as the Server service, or a local process such as Winlogon.exe or Services.exe.    The logon type field indicates the kind of logon that occurred. The most common types are 2 (interactive) and 3 (network).    The New Logon fields indicate the account for whom the new logon was created, i.e. the account that wa................\""
tcpflood -m1 -T "udp" -M "\"hostname.domain	MSWinEventLog	1	Security	00000000	Sun May 21 12:00:01.123	5061	Microsoft-Windows-Security-Auditing	N/A	N/A	Success Audit	hostname.domain	System Integrity		Cryptographic operation.    Subject:   Security ID:  0x000000000   Account Name:  first.last   Account Domain:  domain   Logon ID:  0x000000000    Cryptographic Parameters:   Provider Name: Microsoft Software Key Storage Provider   Algorithm Name: RSA   Key Name: le-c6bdb786-1851-4159-b5ea-5e3966571698   Key Type: Machine key.    Cryptographic Operation:   Operation: Open Key.   Return Code: 0x0	-0000000000\""
shutdown_when_empty
wait_shutdown

echo '14,user,info,,, 05/21/2017:00:00:00 GMT HOSTNAME 1-ABC-2 : default SSLLOG SSL_HANDSHAKE_SUCCESS 39672436 0 :  SPCBId 6377757 - ClientIP 192.168.0.11 - ClientPort 55073 - VserverServiceIP 192.168.0.11 - VserverServicePort 443 - ClientVersion TLSv1.0 - CipherSuite "AES-256-CBC-SHA TLSv1 Non-Export 256-bit" - Session Reuse    The authenti
14,user,info,MSWinEventLog,MSWinEventLog, 1#011Security#01100000000#011Sun May 21 12:00:01.123#0114624#011Microsoft-Windows-Security-Auditing#011N/A#011N/A#011Success Audit#011hostname.domain#011Logon#011#011An account was successfully logged on.    Subject:   Security ID:  0x000000000   Account Name:  first.last   Account Domain:  domain   Logon ID:  0x000000000    Logon Type:   3    New Logon:   Security ID:  0x000000000   Account Name:  first.last   Account Domain:  domain   Logon ID:  0x000000000   Logon GUID:  0x000000000    Process Information:   Process ID:  0x000000000   Process Name:  first.last    Network Information:   Workstation Name:    Source Network Address: 192.168.0.11   Source Port:  51542    Detailed Authentication Information:   Logon Process:  Kerberos   Authentication Package: Kerberos   Transited Services: -   Package Name (NTLM only): -   Key Length:  0    This event is generated when a logon session is created. It is generated on the computer that was accessed.    The subject fields indicate the account on the local system which requested the logon. This is most commonly a service such as the Server service, or a local process such as Winlogon.exe or Services.exe.    The logon type field indicates the kind of logon that occurred. The most common types are 2 (interactive) and 3 (network).    The New Logon fields indicate the account for whom the new logon was created, i.e. the account that wa................
13,user,notice,MSWinEventLog,MSWinEventLog, 1#011Security#01100000000#011Sun May 21 12:00:01.123#0115061#011Microsoft-Windows-Security-Auditing#011N/A#011N/A#011Success Audit#011hostname.domain#011System Integrity#011#011Cryptographic operation.    Subject:   Security ID:  0x000000000   Account Name:  first.last   Account Domain:  domain   Logon ID:  0x000000000    Cryptographic Parameters:   Provider Name: Microsoft Software Key Storage Provider   Algorithm Name: RSA   Key Name: le-c6bdb786-1851-4159-b5ea-5e3966571698   Key Type: Machine key.    Cryptographic Operation:   Operation: Open Key.   Return Code: 0x0#011-0000000000' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
