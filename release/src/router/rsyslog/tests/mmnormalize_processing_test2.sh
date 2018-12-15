#!/bin/bash
# add 2016-11-22 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
. $srcdir/faketime_common.sh

export TZ=TEST+02:00

generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/mmnormalize/.libs/mmnormalize")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")

template(name="t_file_record" type="string" string="%timestamp:::date-rfc3339% %timestamp:::date-rfc3339% %hostname% %$!v_tag% %$!v_msg%\n")
template(name="t_file_path" type="string" string="/sb/logs/incoming/%$year%/%$month%/%$day%/svc_%$!v_svc%/ret_%$!v_ret%/os_%$!v_os%/%fromhost-ip%/r_relay1/%$!v_file:::lowercase%.gz\n")

template(name="t_fromhost-ip" type="string" string="%fromhost-ip%")
template(name="t_analytics_msg_default" type="string" string="%$!v_analytics_prefix%%rawmsg-after-pri%")
template(name="t_analytics_tag_prefix" type="string" string="%$!v_tag%: ")
template(name="t_analytics_msg_normalized" type="string" string="%timereported% %$!v_hostname% %$!v_analytics_prefix%%$!v_msg%")
template(name="t_analytics_msg_normalized_vc" type="string" string="%timereported:1:6% %$year% %timereported:8:$% %$!v_hostname% %$!v_analytics_prefix%%$!v_msg%")
template(name="t_analytics" type="string" string="[][][%$!v_fromhost-ip%][%timestamp:::date-unixtimestamp%][] %$!v_analytics_msg%\n")

ruleset(name="ruleset1") {
	action(type="mmnormalize" rulebase=`echo $srcdir/testsuites/mmnormalize_processing_tests.rulebase` useRawMsg="on")
	if ($!v_file == "") then {
		set $!v_file=$!v_tag;
	}
	action(type="omfile" File=`echo $RSYSLOG_OUT_LOG` template="t_file_record")
	action(type="omfile" File=`echo $RSYSLOG_OUT_LOG` template="t_file_path")

	set $!v_forward="PCI";

	if ($!v_forward contains "PCI") then {
		if ($!v_fromhost-ip == "") then {
			set $!v_fromhost-ip=exec_template("t_fromhost-ip");
		}
		if ($!v_msg == "" or $!v_tag == "") then {
			set $!v_analytics_msg=exec_template("t_analytics_msg_default");
		} else {
			if ($!v_analytics_prefix == "") then {
				set $!v_analytics_prefix=exec_template("t_analytics_tag_prefix");
			}
			if ($!v_hostname == "") then { # needed for vCenter logs with custom hostname
				set $!v_hostname=exec_template("t_fromhost-ip");
			}
			if ($!v_exception == "VC") then {
				set $!v_analytics_msg=exec_template("t_analytics_msg_normalized_vc");
			} else {
				set $!v_analytics_msg=exec_template("t_analytics_msg_normalized");
			}
		}
		action(type="omfile" File=`echo $RSYSLOG_OUT_LOG` template="t_analytics")
	}	
}
'
FAKETIME='2017-03-08 12:18:47' startup
tcpflood -m1 -M "\"<166>2017-03-08T12:18:47.165Z Host2.domain.com Process1: [FFB87B70 verbose Process1HalCnxHostagent opID=WFU-abfbbece] [WaitForUpdatesDone] Completed callback\""
shutdown_when_empty
wait_shutdown
echo '2017-03-08T12:18:47.165Z 2017-03-08T12:18:47.165Z Host2.domain.com Process1 [FFB87B70 verbose Process1HalCnxHostagent opID=WFU-abfbbece] [WaitForUpdatesDone] Completed callback
/sb/logs/incoming/2017/03/08/svc_SER2/ret_Y01/os_ESX/127.0.0.1/r_relay1/esx.gz
[][][127.0.0.1][1488975527][] Mar  8 12:18:47 127.0.0.1 Process1: [FFB87B70 verbose Process1HalCnxHostagent opID=WFU-abfbbece] [WaitForUpdatesDone] Completed callback' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
