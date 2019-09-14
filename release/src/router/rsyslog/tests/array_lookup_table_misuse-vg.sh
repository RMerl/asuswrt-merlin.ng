#!/bin/bash
# added 2015-10-30 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[array_lookup_table-vg.sh\]: test cleanup for array lookup-table and HUP based reloading of it
. $srcdir/diag.sh init
generate_conf
add_conf '
lookup_table(name="xlate" file="xlate_array.lkp_tbl")

template(name="outfmt" type="string" string="%msg% %$.lkp%\n")

set $.num = field($msg, 58, 2);

set $.lkp = lookup("xlate", $.num);

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
cp -f $srcdir/testsuites/xlate_array_misuse.lkp_tbl xlate_array.lkp_tbl
startup_vg
injectmsg  0 3
wait_queueempty
assert_content_missing "foo"
assert_content_missing "bar"
assert_content_missing "baz"
cp -f $srcdir/testsuites/xlate_array_more_misuse.lkp_tbl xlate_array.lkp_tbl
issue_HUP
await_lookup_table_reload
injectmsg  0 3
wait_queueempty
assert_content_missing "foo"
assert_content_missing "bar"
assert_content_missing "baz"
cp -f $srcdir/testsuites/xlate_array_more.lkp_tbl xlate_array.lkp_tbl
issue_HUP
await_lookup_table_reload
injectmsg  0 3
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown_vg
check_exit_vg
content_check "msgnum:00000000: foo_new"
content_check "msgnum:00000001: bar_new"
content_check "msgnum:00000002: baz"
exit_test
