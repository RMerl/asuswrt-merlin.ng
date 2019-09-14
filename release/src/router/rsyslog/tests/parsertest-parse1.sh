#!/bin/bash
# add 2018-06-27 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
setvar_RS_HOSTNAME
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")

template(name="outfmt" type="string" string="%PRI%,%syslogfacility-text%,%syslogseverity-text%,%timestamp%,%hostname%,%programname%,%syslogtag%,%msg%\n")

ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
	       template="outfmt")
}

'
startup
tcpflood -m1 -M "\"<167>Mar  6 16:57:54 172.20.245.8 %PIX-7-710005: UDP request discarded from SERVER1/2741 to test_app:255.255.255.255/61601\""
tcpflood -m1 -M "\"<167>Mar 27 19:06:53 source_server sshd(pam_unix)[12750]: session opened for user foo by (uid=0)\""
tcpflood -m1 -M "\"<167>Apr  6 15:07:10 lxcvs07 sshd(pam_unix)[31738]: session closed for user cvsadmin\""
tcpflood -m1 -M "\"<167>Jul 31 21:39:21 example-b example-gw[10538]: disconnect host=/192.0.2.1 destination=192.0.2.2/11282 in=3274 out=1448 duration=0\""
tcpflood -m1 -M "\"<167>AUG 10 22:18:24 host tag This msg contains 8-bit European chars: äöü\""
tcpflood -m1 -M "\"<167> Mar  7 19:06:53 example tag: testmessage (only date actually tested)\""
tcpflood -m1 -M "\"<167>Mar 7 2008 19:06:53: example tag: testmessage (only date actually tested)\""
tcpflood -m1 -M "\"<167>Mar 7 2008 19:06:53 example tag: testmessage (only date actually tested)\""
tcpflood -m1 -M "\"<167>Mar 7 19:06:53: example tag: testmessage (only date actually tested)\""
tcpflood -m1 -M "\"<14>Jan  6 2009 15:22:26 localhost\""
tcpflood -m1 -M "\"<167>Oct  8 23:05:06 10.321.1.123 05\\\",result_code=200,b\""
tcpflood -m1 -M "\"<167>Feb 18 16:01:59 serverX -- MARK --\""
tcpflood -m1 -M "\"Feb 18 16:01:59 serverX -- MARK --\""
tcpflood -m1 -M "\"<38>Mar 27 19:06:53 source_server 0123456789012345678901234567890123456789: MSG part\""
tcpflood -m1 -M "\"<29>Oct 16 20:47:24 example-p exam-pl[12345]: connect host= /192.0.2.1\""
tcpflood -m1 -M "\"<34>Oct 11 22:14:15 mymachine su: su root failed for lonvick on /dev/pts/8\""
tcpflood -m1 -M "\"<34>1 2003-10-11T22:14:15.003Z mymachine.example.com su - ID47 - BOMsu root failed for lonvick on /dev/pts/8\""
tcpflood -m1 -M "\"<165>1 2003-08-24T05:14:15.000003-07:00 192.0.2.1 myproc 8710 - - %% Its time to make the do-nuts.\""
tcpflood -m1 -M "\"<165>1 2003-10-11T22:14:15.003Z mymachine.example.com evntslog - ID47 [exampleSDID@32473 iut=\\\"3\\\" eventSource= \\\"Application\\\" eventID=\\\"1011\\\"][examplePriority@32473 class=\\\"high\\\"]\""
tcpflood -m1 -M "\"<165>1 2003-10-11T22:14:15.003Z mymachine.example.com evntslog - ID47 [exampleSDID@32473 iut=\\\"3\\\" eventSource= \\\"Application\\\" eventID=\\\"1011\\\"] BOMAn application event log entry...\""
tcpflood -m1 -M "\"<6>AUG 10 22:18:24 2009  netips-warden2-p [audit] user=[*SMS] src=192.168.11.11 iface=5 access=9 Update State Reset\""
tcpflood -m1 -M "\"<14>Aug 30 23:00:05 X4711 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\""
tcpflood -m1 -M "\"<14>Aug 30 23:00:05 X4711 \""
tcpflood -m1 -M "\"<14>Aug 30 23:00:05 X4711\""
tcpflood -m1 -M "\"<14>Aug 30 23:00:05 \""
tcpflood -m1 -M "\"<14>Aug 30 23:00:05\""
tcpflood -m1 -M "\"<14>2010-08-30T23:00:05Z X4711 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\""
tcpflood -m1 -M "\"<14>2010-08-30T23:00:05Z X4711 \""
tcpflood -m1 -M "\"<14>2010-08-30T23:00:05Z X4711\""
shutdown_when_empty
wait_shutdown

export EXPECTED="167,local4,debug,Mar  6 16:57:54,172.20.245.8,%PIX-7-710005,%PIX-7-710005:, UDP request discarded from SERVER1/2741 to test_app:255.255.255.255/61601
167,local4,debug,Mar 27 19:06:53,source_server,sshd(pam_unix),sshd(pam_unix)[12750]:, session opened for user foo by (uid=0)
167,local4,debug,Apr  6 15:07:10,lxcvs07,sshd(pam_unix),sshd(pam_unix)[31738]:, session closed for user cvsadmin
167,local4,debug,Jul 31 21:39:21,example-b,example-gw,example-gw[10538]:, disconnect host=/192.0.2.1 destination=192.0.2.2/11282 in=3274 out=1448 duration=0
167,local4,debug,Aug 10 22:18:24,host,tag,tag, This msg contains 8-bit European chars: äöü
167,local4,debug,Mar  7 19:06:53,example,tag,tag:, testmessage (only date actually tested)
167,local4,debug,Mar  7 19:06:53,example,tag,tag:, testmessage (only date actually tested)
167,local4,debug,Mar  7 19:06:53,example,tag,tag:, testmessage (only date actually tested)
167,local4,debug,Mar  7 19:06:53,example,tag,tag:, testmessage (only date actually tested)
14,user,info,Jan  6 15:22:26,localhost,,,
167,local4,debug,Oct  8 23:05:06,10.321.1.123,05\",result_code=200,b,05\",result_code=200,b,
167,local4,debug,Feb 18 16:01:59,serverX,--,--, MARK --
13,user,notice,Feb 18 16:01:59,serverX,--,--, MARK --
38,auth,info,Mar 27 19:06:53,source_server,0123456789012345678901234567890123456789,0123456789012345678901234567890123456789:, MSG part
29,daemon,notice,Oct 16 20:47:24,example-p,exam-pl,exam-pl[12345]:, connect host= /192.0.2.1
34,auth,crit,Oct 11 22:14:15,mymachine,su,su:, su root failed for lonvick on /dev/pts/8
34,auth,crit,Oct 11 22:14:15,mymachine.example.com,su,su,BOMsu root failed for lonvick on /dev/pts/8
165,local4,notice,Aug 24 05:14:15,192.0.2.1,myproc,myproc[8710],%% Its time to make the do-nuts.
165,local4,notice,Oct 11 22:14:15,mymachine.example.com,evntslog,evntslog,
165,local4,notice,Oct 11 22:14:15,mymachine.example.com,evntslog,evntslog,BOMAn application event log entry...
6,kern,info,Aug 10 22:18:24,2009,,, netips-warden2-p [audit] user=[*SMS] src=192.168.11.11 iface=5 access=9 Update State Reset
14,user,info,Aug 30 23:00:05,X4711,AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA,AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA,
14,user,info,Aug 30 23:00:05,X4711,,,
14,user,info,Aug 30 23:00:05,X4711,,,
14,user,info,Aug 30 23:00:05,$RS_HOSTNAME,,,
14,user,info,Aug 30 23:00:05,$RS_HOSTNAME,,,
14,user,info,Aug 30 23:00:05,X4711,AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA,AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA,
14,user,info,Aug 30 23:00:05,X4711,,,
14,user,info,Aug 30 23:00:05,X4711,,,"
cmp_exact $RSYSLOG_OUT_LOG

exit_test
