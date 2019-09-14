#!/bin/bash
# add 2017-09-21 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '

module(load="../plugins/omrelp/.libs/omrelp")
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="ruleset")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

ruleset(name="ruleset") {
	action(type="omrelp" target="127.0.0.1" port="10514" tls="on" tls.authMode="name" tls.caCert="tls-certs/ca.pem" tls.myCert="tls-certs/fake-cert.pem" tls.myPrivKey="tls-certs/fake-key.pem" tls.permittedPeer=["rsyslog-test-root-ca"])
}

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
shutdown_when_empty
wait_shutdown

grep "certificate file tls-certs/fake-cert.pem.*No such file"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
        echo
        echo "FAIL: expected error message from missing input file not found.  $RSYSLOG_OUT_LOG is:"
        cat $RSYSLOG_OUT_LOG
        error_exit 1
fi

exit_test
