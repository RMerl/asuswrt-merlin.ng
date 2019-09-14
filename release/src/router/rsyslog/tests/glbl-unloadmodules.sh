#!/bin/bash
# we only test if the parameter is accepted - we cannot
# reliably deduce from the outside if it really worked.
# addd 2016-03-03 by RGerhards, released under ASL 2.0
echo \[glbl-unloadmodules\]: 
. $srcdir/diag.sh init
generate_conf
add_conf '
global(debug.unloadModules="off")

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
sleep 1
shutdown_when_empty
wait_shutdown
# to check for support, we check if an error message has
# been recorded, which would bear the name of our option.
# if it is not recorded, we assume all is well. Not perfect,
# but works good enough.
grep -i "unloadModules" < $RSYSLOG_OUT_LOG
if [ ! $? -eq 1 ]; then
  echo "parameter name in output, assuming error message:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;
exit_test
