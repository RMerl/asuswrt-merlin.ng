#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0
export ES_DOWNLOAD=elasticsearch-6.0.0.tar.gz
. $srcdir/diag.sh download-elasticsearch
. $srcdir/diag.sh stop-elasticsearch
. $srcdir/diag.sh prepare-elasticsearch
. $srcdir/diag.sh start-elasticsearch

. $srcdir/diag.sh init
. $srcdir/diag.sh es-init
generate_conf
add_conf '
template(name="tpl" type="string"
	 string="{\"msgnum\":\"%msg:F,58:2%\"}")


ruleset(name="stats") {
  action(type="omfile" file="'${RSYSLOG_DYNNAME}'.out.stats.log")
}
module(load="../plugins/impstats/.libs/impstats" interval="1" severity="7" resetCounters="off" Ruleset="stats" bracketing="on" format="json")

module(load="../plugins/omelasticsearch/.libs/omelasticsearch")
:msg, contains, "msgnum:" action(type="omelasticsearch"
				 server=["localhost", "http://localhost/", "localhost:9201"]
				 serverport="19200"
				 template="tpl"
				 searchIndex="rsyslog_testbench")
'
startup
injectmsg  0 100
wait_queueempty
. $srcdir/diag.sh wait-for-stats-flush ${RSYSLOG_DYNNAME}.out.stats.log
shutdown_when_empty
wait_shutdown 
. $srcdir/diag.sh es-getdata 100 19200
seq_check  0 99
# The configuration makes every other request from message #3 fail checkConn (N/2-1)
custom_content_check '"failed.checkConn": 49' "${RSYSLOG_DYNNAME}.out.stats.log"
. $srcdir/diag.sh cleanup-elasticsearch
exit_test
