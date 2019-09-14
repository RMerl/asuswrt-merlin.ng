#!/bin/bash 
# This file is part of the rsyslog project, released under ASL 2.0 

# we libmaxmindb, in packaged versions, has a small cosmetic memory leak,
# thus we need a supressions file:
export RS_TESTBENCH_VALGRIND_EXTRA_OPTS="$RS_TESTBENCH_VALGRIND_EXTRA_OPTS --suppressions=$srcdir/libmaxmindb.supp"

. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="%$!iplocation%\n")

module(load="../plugins/mmdblookup/.libs/mmdblookup")
module(load="../plugins/mmnormalize/.libs/mmnormalize")
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'" ruleset="testing")

ruleset(name="testing") {
	action(type="mmnormalize" rulebase=`echo $srcdir/mmdb.rb`)
	action(type="mmdblookup" mmdbfile=`echo $srcdir/test.mmdb` key="$!ip" fields="city" )
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}'
startup_vg
tcpflood -m 100 -j "202.106.0.20\ "
shutdown_when_empty
wait_shutdown_vg
check_exit_vg
content_check '{ "city": "Beijing" }'
exit_test 
