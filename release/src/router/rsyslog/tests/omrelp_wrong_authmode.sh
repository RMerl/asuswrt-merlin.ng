#!/bin/bash
# add 2018-09-13 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/omrelp/.libs/omrelp")
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="13514" ruleset="ruleset")

ruleset(name="ruleset") {
	action(type="omrelp" target="127.0.0.1" port="10514" tls="on" tls.authMode="INVALID_AUTH_MODE" tls.caCert="tls-certs/ca.pem" tls.myCert="tls-certs/cert.pem" tls.myPrivKey="tls-certs/key.pem" tls.permittedPeer=["rsyslog-test-root-ca"])
}

action(type="omfile" file="'$RSYSLOG_OUT_LOG'")
'
startup
shutdown_when_empty
wait_shutdown

grep "omrelp.* invalid auth.*mode .*INVALID_AUTH_MODE" $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
        echo "FAIL: expected error message from missing input file not found. $RSYSLOG_OUT_LOG is:"
        cat -n $RSYSLOG_OUT_LOG
        error_exit 1
fi

exit_test
