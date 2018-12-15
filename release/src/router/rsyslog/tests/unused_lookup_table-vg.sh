#!/bin/bash
# added 2015-09-30 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[unused_lookup_table.sh\]: test for ensuring clean destruction of lookup-table even when it is never used
. $srcdir/diag.sh init
generate_conf
add_conf '
lookup_table(name="xlate" file="xlate.lkp_tbl")

template(name="outfmt" type="string" string="- %msg%\n")

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
cp -f $srcdir/testsuites/xlate.lkp_tbl xlate.lkp_tbl
startup_vg
injectmsg  0 1
shutdown_when_empty
wait_shutdown_vg
check_exit_vg
content_check "msgnum:00000000:"
exit_test

# the test actually expects clean destruction of lookup_table
# when lookup_table is loaded, it can either be:
# - used (clean destruct covered by another test)
# - not-used (this test ensures its destroyed cleanly)
