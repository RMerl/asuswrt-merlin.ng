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

module(load="../plugins/omelasticsearch/.libs/omelasticsearch")
:msg, contains, "msgnum:" action(type="omelasticsearch"
				 template="tpl"
				 serverport="19200"
				 searchIndex="rsyslog_testbench"
				 bulkmode="on"
				 maxbytes="1k")
'
startup
injectmsg  0 10000
shutdown_when_empty
wait_shutdown 
. $srcdir/diag.sh es-getdata 10000 19200
seq_check  0 9999
. $srcdir/diag.sh cleanup-elasticsearch
exit_test
