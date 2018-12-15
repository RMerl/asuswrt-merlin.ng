#!/bin/bash
# add 2016-11-22 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
. $srcdir/faketime_common.sh

export TZ=TEST-02:00

generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/mmnormalize/.libs/mmnormalize")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")

template(name="t_file_record" type="string" string="%timestamp:::date-rfc3339% %timestamp:::date-rfc3339% %hostname% %$!v_tag% %$!v_msg%\n")
template(name="t_file_path" type="string" string="/sb/logs/incoming/%$year%/%$month%/%$day%/svc_%$!v_svc%/ret_%$!v_ret%/os_%$!v_os%/%fromhost-ip%/r_relay1/%$!v_file:::lowercase%.gz\n")

ruleset(name="ruleset1") {
	action(type="mmnormalize" rulebase=`echo $srcdir/testsuites/mmnormalize_processing_tests.rulebase` useRawMsg="on")
	if ($!v_file == "") then {
		set $!v_file=$!v_tag;
	}
	action(type="omfile" File=`echo $RSYSLOG_OUT_LOG` template="t_file_record")
	action(type="omfile" File=`echo $RSYSLOG_OUT_LOG` template="t_file_path")

}
'
FAKETIME='2017-03-08 14:56:37' startup
tcpflood -m1 -M "\"<187>Mar  8 14:56:37 host4 Process2: {SER4.local7 Y01 LNX [SRCH ALRT DASH REPT ANOM]} (/sb/env/logs/dir1/dir2/log_20170308.log) in 1: X/c79RgpDtrva5we84XHTg== (String)\""
shutdown_when_empty
wait_shutdown
echo '2017-03-08T14:56:37+02:00 2017-03-08T14:56:37+02:00 host4 Process2 in 1: X/c79RgpDtrva5we84XHTg== (String)
/sb/logs/incoming/2017/03/08/svc_SER4/ret_Y01/os_LNX/127.0.0.1/r_relay1/sb/env/logs/dir1/dir2/log_20170308.log.gz' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
