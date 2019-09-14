#!/bin/bash
# addd 2016-07-11 by RGerhards, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")
template(name="outfmt" type="string" string="%hostname%\n")

# note: we use the default parser chain, which includes RFC5424 and that parser
# should be selected AND detect the hostname with slashes as valid.
local4.debug action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
echo '<167>1 2003-03-01T01:00:00.000Z hostname1/hostname2 tcpflood - tag [tcpflood@32473 MSGNUM="0"] data' > $RSYSLOG_DYNNAME.input
tcpflood -B -I $RSYSLOG_DYNNAME.input
shutdown_when_empty
wait_shutdown
echo "hostname1/hostname2" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid hostname generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;
exit_test
