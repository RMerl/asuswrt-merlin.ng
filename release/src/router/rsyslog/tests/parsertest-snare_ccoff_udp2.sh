#!/bin/bash
# add 2018-06-27 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
setvar_RS_HOSTNAME
generate_conf
add_conf '
module(load="../plugins/imudp/.libs/imudp")
input(type="imudp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")

$EscapeControlCharactersOnReceive off

template(name="outfmt" type="string" string="insert into windows (Message, Facility,FromHost, Priority, DeviceReportedTime, ReceivedAt, InfoUnitID, SysLogTag) values (%msg:::space-cc%, %syslogfacility%, %HOSTNAME%,%syslogpriority%, 20100321185328, 20100321185328, %iut%, %syslogtag:::space-cc%)\n")

ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
	       template="outfmt")
}

'
startup
tcpflood -m1 -T "udp" -M "\"test\""
tcpflood -m1 -T "udp" -M "\"UX=Abcd-efg-hij-klmno; XXXXX=1111111111, Z123=192.12.231.245:11111, S1234=123456789, XXXXXX=111111111\""
tcpflood -m1 -T "udp" -M "\"windowsserver	MSWinEventLog	1	Security	1167	Fri Mar 19 15:33:30 2010	540	Security	SYSTEM	User	Success Audit	WINDOWSSERVER	Logon/Logoff		Successful Network Logon:     User Name: WINDOWSSERVER$     Domain: DOMX     Logon ID: (0x0,0xF88396)     Logon Type: 3     Logon Process: Kerberos     Authentication Package: Kerberos     Workstation Name:      Logon GUID: {79b6eb79-7bcc-8a2e-7dad-953c51dc00fd}     Caller User Name: -     Caller Domain: -     Caller Logon ID: -     Caller Process ID: -     Transited Services: -     Source Network Address: 10.11.11.3     Source Port: 3306    	733\\\n\""
shutdown_when_empty
wait_shutdown

export EXPECTED="insert into windows (Message, Facility,FromHost, Priority, DeviceReportedTime, ReceivedAt, InfoUnitID, SysLogTag) values (, 1, test,5, 20100321185328, 20100321185328, 1, )
insert into windows (Message, Facility,FromHost, Priority, DeviceReportedTime, ReceivedAt, InfoUnitID, SysLogTag) values ( XXXXX=1111111111, Z123=192.12.231.245:11111, S1234=123456789, XXXXXX=111111111, 1, $RS_HOSTNAME,5, 20100321185328, 20100321185328, 1, UX=Abcd-efg-hij-klmno;)
insert into windows (Message, Facility,FromHost, Priority, DeviceReportedTime, ReceivedAt, InfoUnitID, SysLogTag) values ( Mar 19 15:33:30 2010 540 Security SYSTEM User Success Audit WINDOWSSERVER Logon/Logoff  Successful Network Logon:     User Name: WINDOWSSERVER$     Domain: DOMX     Logon ID: (0x0,0xF88396)     Logon Type: 3     Logon Process: Kerberos     Authentication Package: Kerberos     Workstation Name:      Logon GUID: {79b6eb79-7bcc-8a2e-7dad-953c51dc00fd}     Caller User Name: -     Caller Domain: -     Caller Logon ID: -     Caller Process ID: -     Transited Services: -     Source Network Address: 10.11.11.3     Source Port: 3306     733\n, 1, $RS_HOSTNAME,5, 20100321185328, 20100321185328, 1, windowsserver MSWinEventLog 1 Security 1167 Fri)"
cmp_exact $RSYSLOG_OUT_LOG

exit_test
