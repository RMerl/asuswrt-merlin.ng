#!/bin/bash
# added 2016-01-20 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[multiple_lookup_table.sh\]: test for multiple lookup-table and HUP based reloading of it
. $srcdir/diag.sh init
generate_conf
add_conf '
lookup_table(name="xlate_0" file="xlate.lkp_tbl")
lookup_table(name="xlate_1" file="xlate_1.lkp_tbl")

template(name="outfmt" type="string" string="- %msg% 0_%$.lkp_0% 1_%$.lkp_1%\n")

set $.lkp_0 = lookup("xlate_0", $msg);
set $.lkp_1 = lookup("xlate_1", $msg);

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
cp -f $srcdir/testsuites/xlate.lkp_tbl xlate.lkp_tbl
cp -f $srcdir/testsuites/xlate.lkp_tbl xlate_1.lkp_tbl
startup
injectmsg  0 3
wait_queueempty
content_check "msgnum:00000000: 0_foo_old 1_foo_old"
content_check "msgnum:00000001: 0_bar_old 1_bar_old"
assert_content_missing "baz"
cp -f $srcdir/testsuites/xlate_more.lkp_tbl xlate.lkp_tbl
issue_HUP
await_lookup_table_reload
injectmsg  0 3
wait_queueempty
content_check "msgnum:00000000: 0_foo_new 1_foo_old"
content_check "msgnum:00000001: 0_bar_new 1_bar_old"
content_check "msgnum:00000002: 0_baz"
assert_content_missing "1_baz"
cp -f $srcdir/testsuites/xlate_more.lkp_tbl xlate_1.lkp_tbl
issue_HUP
await_lookup_table_reload
injectmsg  0 3
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown
content_check "msgnum:00000000: 0_foo_new 1_foo_new"
content_check "msgnum:00000001: 0_bar_new 1_bar_new"
content_check "msgnum:00000002: 0_baz 1_baz"
exit_test
