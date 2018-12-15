#!/bin/bash
# make sure we do not abort on invalid parameter (we 
# once had this problem)
# addd 2016-03-03 by RGerhards, released under ASL 2.0
echo \[glbl-invld-param\]: 
. $srcdir/diag.sh init
generate_conf
add_conf '
global(invalid="off")
global(debug.unloadModules="invalid")
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
sleep 1
shutdown_when_empty
wait_shutdown
# if we reach this point, we consider this a success.
exit_test
