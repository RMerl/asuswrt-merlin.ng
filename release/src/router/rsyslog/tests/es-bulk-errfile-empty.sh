#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0
export ES_DOWNLOAD=elasticsearch-6.0.0.tar.gz
export ES_PORT=19200
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

module(load="../plugins/omelasticsearch/.libs/omelasticsearch")
:msg, contains, "msgnum:" action(type="omelasticsearch"
				 template="tpl"
				 serverport=`echo $ES_PORT`
				 searchIndex="rsyslog_testbench"
				 bulkmode="on"
				 errorFile="./'${RSYSLOG_DYNNAME}'.errorfile")
'
startup
injectmsg  0 10000
shutdown_when_empty
wait_shutdown 
. $srcdir/diag.sh es-getdata 10000 $ES_PORT
if [ -f ${RSYSLOG_DYNNAME}.errorfile ]
then
    echo "error: error file exists!"
    cat ${RSYSLOG_DYNNAME}.errorfile
    error_exit 1
fi
seq_check  0 9999 19200
. $srcdir/diag.sh stop-elasticsearch
. $srcdir/diag.sh cleanup-elasticsearch
exit_test
