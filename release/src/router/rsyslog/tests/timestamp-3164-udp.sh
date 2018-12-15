#!/bin/bash
# add 2018-06-25 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imudp/.libs/imudp")
input(type="imudp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="%timestamp:::date-rfc3164%\n")

:syslogtag, contains, "TAG" action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
				   template="outfmt")


'
startup
tcpflood -m1 -T "udp" -M "\"<167>Jan  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Feb  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Mar  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Apr  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>May  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Jun  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Jul  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Aug  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Sep  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Oct  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Nov  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Dec  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Jan  6 16:57:54 172.20.245.8 TAG: MSG\""
tcpflood -m1 -T "udp" -M "\"<167>Jan 16 16:57:54 172.20.245.8 TAG: MSG\""
shutdown_when_empty
wait_shutdown

echo 'Jan  6 16:57:54
Feb  6 16:57:54
Mar  6 16:57:54
Apr  6 16:57:54
May  6 16:57:54
Jun  6 16:57:54
Jul  6 16:57:54
Aug  6 16:57:54
Sep  6 16:57:54
Oct  6 16:57:54
Nov  6 16:57:54
Dec  6 16:57:54
Jan  6 16:57:54
Jan 16 16:57:54' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
