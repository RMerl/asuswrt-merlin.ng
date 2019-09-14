#!/bin/bash
# add 2018-06-29 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imudp/.libs/imudp")
input(type="imudp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")

template(name="outfmt" type="string" string="%msg:F,32:2%\n")

ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
	       template="outfmt")
}

'
startup
tcpflood -m1 -T "udp" -M "\"<167>Mar  6 16:57:54 172.20.245.8 %PIX-7-710005: DROP_url_www.sina.com.cn:IN=eth1 OUT=eth0 SRC=192.168.10.78 DST=61.172.201.194 LEN=1182 TOS=0x00 PREC=0x00 TTL=63 ID=14368 DF PROTO=TCP SPT=33343 DPT=80 WINDOW=92 RES=0x00 ACK PSH URGP=0\""
shutdown_when_empty
wait_shutdown

echo 'DROP_url_www.sina.com.cn:IN=eth1' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
