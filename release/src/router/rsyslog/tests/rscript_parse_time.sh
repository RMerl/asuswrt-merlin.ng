#!/bin/bash
# Added 2017-10-28 by Stephen Workman, released under ASL 2.0

# Because this script tests functionality that depends on the current date,
# we cannot use static values for the expected results. They have to be
# calculated. Also, because we cannot depend on the GNU version of the
# 'date' command on all of our test systems (think FreeBSD, and Solaris),
# we need a method of converting given date/time strings to UNIX timestamps.
# For that we use an external Python 2.x script to do the job.

getts="python $srcdir/rscript_parse_time_get-ts.py"

# Run the Python script's self-tests
$getts selftest

if [[ $? -ne 0 ]]; then
  printf "Failed own self-test(s)!\n"
  error_exit 1
fi

# Since the RFC 3164 date/time format does not include a year, we need to
# try to "guess" an appropriate one based on the incoming date and the
# current date. So, we'll use a reasonable spread of RFC 3164 date/time 
# strings to ensure that we test as much of our year "guessing" as
# possible. Since this uses the CURRENT DATE (as in, the date this)
# script was invoked, we need to calculate our expected results to
# compare them with the values returned by the parse_time() RainerScript
# function.
rfc3164_1="Oct  5 01:10:11"
rfc3164_1_r=$($getts "$rfc3164_1")

rfc3164_2="Jan 31 13:00:00"
rfc3164_2_r=$($getts "$rfc3164_2")

rfc3164_3="Feb 28 14:35:00"
rfc3164_3_r=$($getts "$rfc3164_3")

rfc3164_4="Mar  1 14:00:00"
rfc3164_4_r=$($getts "$rfc3164_4")

rfc3164_5="Apr  3 15:00:00"
rfc3164_5_r=$($getts "$rfc3164_5")

rfc3164_6="May  5 16:00:00"
rfc3164_6_r=$($getts "$rfc3164_6")

rfc3164_7="Jun 11 03:00:00"
rfc3164_7_r=$($getts "$rfc3164_7")

rfc3164_8="Jul 15 05:00:00"
rfc3164_8_r=$($getts "$rfc3164_8")

rfc3164_9="Aug 17 08:00:00"
rfc3164_9_r=$($getts "$rfc3164_9")

rfc3164_10="Sep 20 18:00:00"
rfc3164_10_r=$($getts "$rfc3164_10")

rfc3164_11="Nov 23 19:00:00"
rfc3164_11_r=$($getts "$rfc3164_11")

rfc3164_12="Dec 25 20:00:00"
rfc3164_12_r=$($getts "$rfc3164_12")

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/omstdout/.libs/omstdout")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

# $DebugLevel 2

# RFC 3164 Parse Tests (using fixed input values - see above)
set $!datetime!rfc3164_1  = parse_time("'"$rfc3164_1"'");
set $!datetime!rfc3164_2  = parse_time("'"$rfc3164_2"'");
set $!datetime!rfc3164_3  = parse_time("'"$rfc3164_3"'");
set $!datetime!rfc3164_4  = parse_time("'"$rfc3164_4"'");
set $!datetime!rfc3164_5  = parse_time("'"$rfc3164_5"'");
set $!datetime!rfc3164_6  = parse_time("'"$rfc3164_6"'");
set $!datetime!rfc3164_7  = parse_time("'"$rfc3164_7"'");
set $!datetime!rfc3164_8  = parse_time("'"$rfc3164_8"'");
set $!datetime!rfc3164_9  = parse_time("'"$rfc3164_9"'");
set $!datetime!rfc3164_10 = parse_time("'"$rfc3164_10"'");
set $!datetime!rfc3164_11 = parse_time("'"$rfc3164_11"'");
set $!datetime!rfc3164_12 = parse_time("'"$rfc3164_12"'");

# RFC 3339 Parse Tests (these provide their own year)
set $!datetime!rfc3339    = parse_time("2017-10-05T01:10:11Z");
set $!datetime!rfc3339tz1 = parse_time("2017-10-05T01:10:11+04:00");
set $!datetime!rfc3339tz2 = parse_time("2017-10-05T01:10:11+00:00");

# Test invalid date strings, these should return 0
set $!datetime!inval1 = parse_time("not a date/time");
set $!datetime!inval2 = parse_time("2017-10-05T01:10:11");
set $!datetime!inval3 = parse_time("2017-SOMETHING: 42");

template(name="outfmt" type="string" string="%!datetime%\n")
local4.* action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
local4.* :omstdout:;outfmt
'

startup
tcpflood -m1 -y
shutdown_when_empty
wait_shutdown

# Our fixed and calculated expected results
EXPECTED='{ "rfc3164_1": '"$rfc3164_1_r"', "rfc3164_2": '"$rfc3164_2_r"', "rfc3164_3": '"$rfc3164_3_r"', "rfc3164_4": '"$rfc3164_4_r"', "rfc3164_5": '"$rfc3164_5_r"', "rfc3164_6": '"$rfc3164_6_r"', "rfc3164_7": '"$rfc3164_7_r"', "rfc3164_8": '"$rfc3164_8_r"', "rfc3164_9": '"$rfc3164_9_r"', "rfc3164_10": '"$rfc3164_10_r"', "rfc3164_11": '"$rfc3164_11_r"', "rfc3164_12": '"$rfc3164_12_r"', "rfc3339": 1507165811, "rfc3339tz1": 1507151411, "rfc3339tz2": 1507165811, "inval1": 0, "inval2": 0, "inval3": 0 }'

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
