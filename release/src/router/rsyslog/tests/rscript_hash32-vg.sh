#!/bin/bash
# added 2018-02-07 by Harshvardhan Shrivastava
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \rscript_hash32.sh\]: test for hash32 and hash64mod script-function
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="%$.hash_no_1% -  %$.hash_no_2%\n")

module(load="../plugins/imtcp/.libs/imtcp")
module(load="../contrib/fmhash/.libs/fmhash")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

set $.hash_no_1 = hash32("0f9a1d07-a8c9-43a7-a6f7-198dca3d932e");
set $.hash_no_2 = hash32mod("0f9a1d07-a8c9-43a7-a6f7-198dca3d932e", 100);

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup_vg
tcpflood -m 20
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown_vg
check_exit_vg
. $srcdir/diag.sh content-pattern-check "^\(746581550 -  50\|3889673532 -  32\)$"
exit_test
