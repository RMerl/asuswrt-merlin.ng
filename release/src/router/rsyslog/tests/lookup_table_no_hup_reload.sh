#!/bin/bash
# added 2015-09-30 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[lookup_table_no_hup_reload.sh\]: test for lookup-table with HUP based reloading disabled
. $srcdir/diag.sh init
generate_conf
add_conf '
lookup_table(name="xlate" file="xlate.lkp_tbl" reloadOnHUP="off")

template(name="outfmt" type="string" string="- %msg% %$.lkp%\n")

set $.lkp = lookup("xlate", $msg);

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
cp -f $srcdir/testsuites/xlate.lkp_tbl xlate.lkp_tbl
startup
injectmsg  0 3
wait_queueempty
content_check "msgnum:00000000: foo_old"
content_check "msgnum:00000001: bar_old"
assert_content_missing "baz"
cp -f $srcdir/testsuites/xlate_more.lkp_tbl xlate.lkp_tbl
issue_HUP
await_lookup_table_reload
injectmsg  0 3
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown
assert_content_missing "foo_new"
assert_content_missing "bar_new"
assert_content_missing "baz"
exit_test
