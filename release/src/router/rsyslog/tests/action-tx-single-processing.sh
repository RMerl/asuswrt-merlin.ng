#!/bin/bash
. $srcdir/diag.sh init
mysql --user=rsyslog --password=testbench < testsuites/mysql-truncate.sql
generate_conf
add_conf '
$ModLoad ../plugins/ommysql/.libs/ommysql
global(errormessagestostderr.maxnumber="50")

template(type="string" name="tpl" string="insert into SystemEvents (Message, Facility) values (\"%msg%\", %$!facility%)" option.sql="on")
template(type="string" name="tpl2" string="%$.num%|%$!facility%|insert into SystemEvents (Message, Facility) values (\"%msg%\", %$!facility%)\n" option.sql="on")

if($msg contains "msgnum:") then {
	set $.num = field($msg, 58, 2);
	if $.num % 2 == 0 then {
		set $!facility = $syslogfacility;
	} else {
		set $/cntr = 0;
	}
	action(type="omfile" file="tmp" template="tpl2")
	action(type="ommysql" name="mysql_action" server="127.0.0.1" template="tpl"
	       db="Syslog" uid="rsyslog" pwd="testbench")
}
action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG`)
'
startup
injectmsg 0 5000
shutdown_when_empty
wait_shutdown
# note "-s" is requried to suppress the select "field header"
mysql -s --user=rsyslog --password=testbench < testsuites/mysql-select-msg.sql > $RSYSLOG_OUT_LOG
seq_check  0 4999 -i2
exit_test
