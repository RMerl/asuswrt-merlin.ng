#!/bin/bash
# addd 2017-03-06 by RGerhards, released under ASL 2.0

# Note: we need to inject a somewhat larger nubmer of messages in order
# to ensure that we receive some messages in the actual output file,
# as batching can (validly) cause a larger loss in the non-writable
# file

. $srcdir/diag.sh init
generate_conf
add_conf '
global(umask="0077")

template(name="outfmt" type="string" string="%msg:F,58:2%\n")
:msg, contains, "msgnum:" {
	action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
}
'
startup
injectmsg 0 1
shutdown_when_empty
wait_shutdown

if [ `ls -l $RSYSLOG_OUT_LOG|$RS_HEADCMD -c 10 ` != "-rw-------" ]; then
  echo "invalid file permission (umask),  $RSYSLOG_OUT_LOG has:"
  ls -l $RSYSLOG_OUT_LOG
  error_exit 1
fi;
exit_test
