#!/bin/bash
# add 2018-06-29 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")

input(type="imtcp" name="12514" port="12514" ruleset="ruleset1")
input(type="imtcp" name="12515" port="12515" ruleset="ruleset1")
input(type="imtcp" name="12516" port="12516" ruleset="ruleset1")

template(name="outfmt" type="string" string="%inputname%\n")

ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
	       template="outfmt")
}

'
startup
tcpflood -p12514 -m1 -M "\"<167>Mar  6 16:57:54 172.20.245.8 %PIX-7-710005: MSG\""
tcpflood -p12515 -m1 -M "\"<167>Mar  6 16:57:54 172.20.245.8 %PIX-7-710005: MSG\""
tcpflood -p12516 -m1 -M "\"<167>Mar  6 16:57:54 172.20.245.8 %PIX-7-710005: MSG\""
shutdown_when_empty
wait_shutdown

echo '12514
12515
12516' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
