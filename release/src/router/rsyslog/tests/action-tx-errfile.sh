#!/bin/bash
. $srcdir/diag.sh init
mysql --user=rsyslog --password=testbench < testsuites/mysql-truncate.sql
generate_conf
add_conf '
$ModLoad ../plugins/ommysql/.libs/ommysql
global(errormessagestostderr.maxnumber="5")

template(type="string" name="tpl" string="insert into SystemEvents (Message, Facility) values (\"%msg%\", %$!facility%)" option.sql="on")

if((not($msg contains "error")) and ($msg contains "msgnum:")) then {
	set $.num = field($msg, 58, 2);
	if $.num % 2 == 0 then {
		set $!facility = $syslogfacility;
	} else {
		set $/cntr = 0;
	}
	action(type="ommysql" name="mysql_action" server="127.0.0.1" template="tpl"
	       db="Syslog" uid="rsyslog" pwd="testbench" action.errorfile=`echo $RSYSLOG2_OUT_LOG`)
}
'
startup
injectmsg 0 50
shutdown_when_empty
wait_shutdown
cmp testsuites/action-tx-errfile.result ${RSYSLOG2_OUT_LOG}
if [ ! $? -eq 0 ]; then
  printf "errorfile does not contain excpected result. Expected:\n\n"
  cat testsuites/action-tx-errfile.result 
  printf "\nActual:\n\n"
  cat ${RSYSLOG2_OUT_LOG}
  error_exit 1
fi;
# note "-s" is required to suppress the select "field header"
mysql -s --user=rsyslog --password=testbench < testsuites/mysql-select-msg.sql > $RSYSLOG_OUT_LOG
seq_check  0 49 -i2
exit_test
