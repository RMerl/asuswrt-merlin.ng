#!/bin/bash
# add 2016-11-22 by Pascal Withopf, released under ASL 2.0
echo [imfile-file-not-found-error.sh]
. $srcdir/diag.sh check-inotify
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imfile/.libs/imfile")

input(type="imfile" File="testsuites/NotExistingInputFile" Tag="tag1" fileNotFoundError="off")

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
shutdown_when_empty
wait_shutdown

grep "error*file*NotExistingInputFile*No such file or directory"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -eq 0 ]; then
        echo
        echo "FAIL: error message from missing input file found.  $RSYSLOG_OUT_LOG is:"
        cat $RSYSLOG_OUT_LOG
        error_exit 1
fi

exit_test
