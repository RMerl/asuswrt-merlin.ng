#!/bin/bash
# Added 2017-12-16 by Stephen Workman, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/omstdout/.libs/omstdout")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

# $DebugLevel 2

set $!result!date_auto_1 = is_time("Oct  5 01:10:11");
set $!result!errno_date_auto_1 = script_error();
set $!result!date_auto_2 = is_time("2017-10-05T01:10:11Z");
set $!result!errno_date_auto_2 = script_error();
set $!result!date_auto_3 = is_time("2017-10-05T01:10:11-03:00");
set $!result!errno_date_auto_3 = script_error();
set $!result!date_auto_4 = is_time("90210");
set $!result!errno_date_auto_4 = script_error();

set $!result!date_explicit_1 = is_time("Oct  5 01:10:11", "date-rfc3164");
set $!result!errno_date_explicit_1 = script_error();
set $!result!date_explicit_2 = is_time("2017-10-05T01:10:11Z", "date-rfc3339");
set $!result!errno_date_explicit_2 = script_error();
set $!result!date_explicit_3 = is_time("2017-10-05T01:10:11+04:00", "date-rfc3339");
set $!result!errno_date_explicit_3 = script_error();
set $!result!date_explicit_4 = is_time(90210, "date-unix");
set $!result!errno_date_explicit_4 = script_error();
set $!result!date_explicit_5 = is_time(-88, "date-unix");
set $!result!errno_date_explicit_5 = script_error();
set $!result!date_explicit_6 = is_time(0, "date-unix");
set $!result!errno_date_explicit_6 = script_error();
set $!result!date_explicit_7 = is_time("90210", "date-unix");
set $!result!errno_date_explicit_7 = script_error();
set $!result!date_explicit_8 = is_time("-88", "date-unix");
set $!result!errno_date_explicit_8 = script_error();

# Bad dates
set $!result!date_fail_1 = is_time("Oct 88 01:10:11");
set $!result!errno_date_fail_1 = script_error();
set $!result!date_fail_2 = is_time("not at all a date");
set $!result!errno_date_fail_2 = script_error();

# Wrong format
set $!result!date_fail_3 = is_time("Oct  5 01:10:11", "date-rfc3339");
set $!result!errno_date_fail_3 = script_error();
set $!result!date_fail_4 = is_time("2017-10-05T01:10:11Z", "date-rfc3164");
set $!result!errno_date_fail_4 = script_error();
set $!result!date_fail_5 = is_time("Oct  5 01:10:11", "date-unix");
set $!result!errno_date_fail_5 = script_error();

# Invalid format
set $!result!date_fail_6 = is_time("90210", "date-spoonix");
set $!result!errno_date_fail_6 = script_error();

template(name="outfmt" type="string" string="%!result%\n")
local4.* action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
local4.* :omstdout:;outfmt
'

startup
tcpflood -m1 -y
shutdown_when_empty
wait_shutdown

# Our fixed and calculated expected results
EXPECTED='{ "date_auto_1": 1, "errno_date_auto_1": 0, "date_auto_2": 1, "errno_date_auto_2": 0, "date_auto_3": 1, "errno_date_auto_3": 0, "date_auto_4": 1, "errno_date_auto_4": 0, "date_explicit_1": 1, "errno_date_explicit_1": 0, "date_explicit_2": 1, "errno_date_explicit_2": 0, "date_explicit_3": 1, "errno_date_explicit_3": 0, "date_explicit_4": 1, "errno_date_explicit_4": 0, "date_explicit_5": 1, "errno_date_explicit_5": 0, "date_explicit_6": 1, "errno_date_explicit_6": 0, "date_explicit_7": 1, "errno_date_explicit_7": 0, "date_explicit_8": 1, "errno_date_explicit_8": 0, "date_fail_1": 0, "errno_date_fail_1": 1, "date_fail_2": 0, "errno_date_fail_2": 1, "date_fail_3": 0, "errno_date_fail_3": 1, "date_fail_4": 0, "errno_date_fail_4": 1, "date_fail_5": 0, "errno_date_fail_5": 1, "date_fail_6": 0, "errno_date_fail_6": 1 }'

# FreeBSD's cmp does not support reading from STDIN
cmp <(echo "$EXPECTED") $RSYSLOG_OUT_LOG

if [[ $? -ne 0 ]]; then
  printf "Invalid function output detected!\n"
  printf "Expected: $EXPECTED\n"
  printf "Got:      "
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;

exit_test
