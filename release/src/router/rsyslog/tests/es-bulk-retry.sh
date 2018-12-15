#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0
export ES_PORT=19200
. $srcdir/diag.sh download-elasticsearch
. $srcdir/diag.sh stop-elasticsearch
. $srcdir/diag.sh prepare-elasticsearch
# change settings to cause bulk rejection errors
cat >> $dep_work_dir/es/config/elasticsearch.yml <<EOF
thread_pool.bulk.queue_size: 1
thread_pool.bulk.size: 1
EOF
. $srcdir/diag.sh start-elasticsearch

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/impstats/.libs/impstats" interval="1"
	   log.file="'$RSYSLOG_DYNNAME'.spool/es-stats.log" log.syslog="off" format="cee")

set $.msgnum = field($msg, 58, 2);
set $.testval = cnum($.msgnum % 2);
if $.testval == 0 then {
	# these should be successful
	set $!msgnum = $.msgnum;
} else {
	# these should cause "hard" errors
	set $!msgnum = "x" & $.msgnum;
}

template(name="tpl" type="string"
    string="{\"msgnum\":\"%$!msgnum%\"}")

module(load="../plugins/omelasticsearch/.libs/omelasticsearch")

template(name="id-template" type="string" string="%$.es_msg_id%")

ruleset(name="error_es") {
	action(type="omfile" template="RSYSLOG_DebugFormat" file="'$RSYSLOG_DYNNAME'.spool/es-bulk-errors.log")
}

ruleset(name="try_es") {
	if strlen($.omes!status) > 0 then {
		# retry case
		if ($.omes!status == 200) or ($.omes!status == 201) or (($.omes!status == 409) and ($.omes!writeoperation == "create")) then {
			stop # successful
		}
		if ($.omes!writeoperation == "unknown") or (strlen($.omes!error!type) == 0) or (strlen($.omes!error!reason) == 0) then {
			call error_es
			stop
		}
		if ($.omes!status == 400) or ($.omes!status < 200) then {
			call error_es
			stop
		}
		# else fall through to retry operation
	}
	if strlen($.omes!_id) > 0 then {
		set $.es_msg_id = $.omes!_id;
	} else {
		# NOTE: in production code, use $uuid - depends on rsyslog being compiled with --enable-uuid
		set $.es_msg_id = $.msgnum;
	}
	action(type="omelasticsearch"
	       server="127.0.0.1"
	       serverport="'${ES_PORT:-19200}'"
	       template="tpl"
	       writeoperation="create"
	       bulkid="id-template"
	       dynbulkid="on"
	       bulkmode="on"
	       retryfailures="on"
	       retryruleset="try_es"
	       searchType="test-type"
	       searchIndex="rsyslog_testbench")
}

if $msg contains "msgnum:" then {
	call try_es
}

action(type="omfile" file="'$RSYSLOG_OUT_LOG'")
'

curl -s -H 'Content-Type: application/json' -XPUT localhost:${ES_PORT:-19200}/rsyslog_testbench/ -d '{
  "mappings": {
    "test-type": {
      "properties": {
        "msgnum": {
          "type": "integer"
        }
      }
    }
  }
}
' | python -mjson.tool
#export RSYSLOG_DEBUG="debug nostdout noprintmutexaction"
#export RSYSLOG_DEBUGLOG="debug.log"
if [ "x${USE_VALGRIND:-false}" == "xtrue" ] ; then
	startup_vg
else
	startup
fi
if [ -n "${USE_GDB:-}" ] ; then
	echo attach gdb here
	sleep 54321 || :
fi
numrecords=100
success=50
badarg=50
injectmsg 0 $numrecords
shutdown_when_empty
if [ "x${USE_VALGRIND:-false}" == "xtrue" ] ; then
	wait_shutdown_vg
	check_exit_vg
else
	wait_shutdown
fi
. $srcdir/diag.sh es-getdata $numrecords $ES_PORT
rc=$?

. $srcdir/diag.sh stop-elasticsearch
. $srcdir/diag.sh cleanup-elasticsearch

if [ -f work ] ; then
	cat work | \
	python -c '
import sys,json
records = int(sys.argv[1])
expectedrecs = {}
rc = 0
for ii in xrange(0, records*2, 2):
	ss = "{:08}".format(ii)
	expectedrecs[ss] = ss
for item in json.load(sys.stdin)["hits"]["hits"]:
	msgnum = item["_source"]["msgnum"]
	if msgnum in expectedrecs:
		del expectedrecs[msgnum]
	else:
		print "Error: found unexpected msgnum {} in record".format(msgnum)
		rc = 1
for item in expectedrecs:
	print "Error: msgnum {} was missing in Elasticsearch".format(item)
	rc = 1
sys.exit(rc)
' $success || { rc=$?; echo error: did not find expected records in Elasticsearch; }
else
	echo error: elasticsearch output file work not found
	rc=1
fi

if [ -f ${RSYSLOG_DYNNAME}.spool/es-stats.log ] ; then
	cat ${RSYSLOG_DYNNAME}.spool/es-stats.log | \
	python -c '
import sys,json
success = int(sys.argv[1])
badarg = int(sys.argv[2])
lasthsh = {}
for line in sys.stdin:
	jstart = line.find("{")
	if jstart >= 0:
		hsh = json.loads(line[jstart:])
		if hsh["name"] == "omelasticsearch":
			lasthsh = hsh
actualsuccess = lasthsh["response.success"]
actualbadarg = lasthsh["response.badargument"]
actualrej = lasthsh["response.bulkrejection"]
actualsubmitted = lasthsh["submitted"]
assert(actualsuccess == success)
assert(actualbadarg == badarg)
assert(actualrej > 0)
assert(actualsuccess + actualbadarg + actualrej == actualsubmitted)
' $success $badarg || { rc=$?; echo error: expected responses not found in ${RSYSLOG_DYNNAME}.spool/es-stats.log; }
else
	echo error: stats file ${RSYSLOG_DYNNAME}.spool/es-stats.log not found
	rc=1
fi

if [ -f ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log ] ; then
	found=0
	for ii in $(seq --format="x%08.f" 1 2 $(expr 2 \* $badarg)) ; do
		if grep -q '^[$][!]:{.*"msgnum": "'$ii'"' ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log ; then
			found=$( expr $found + 1 )
		else
			echo error: missing message $ii in ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log
			rc=1
		fi
	done
	if [ $found -ne $badarg ] ; then
		echo error: found only $found of $badarg messages in ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log
		rc=1
	fi
	if grep -q '^[$][.]:{.*"omes": {' ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log ; then
		:
	else
		echo error: es response info not found in ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log
		rc=1
	fi
	if grep -q '^[$][.]:{.*"status": 400' ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log ; then
		:
	else
		echo error: status 400 not found in ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log
		rc=1
	fi
else
	echo error: bulk error file ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log not found
	rc=1
fi

if [ $rc -eq 0 ] ; then
	echo tests completed successfully
else
	cat $RSYSLOG_OUT_LOG
	if [ -f ${RSYSLOG_DYNNAME}.spool/es-stats.log ] ; then
		cat ${RSYSLOG_DYNNAME}.spool/es-stats.log
	fi
	if [ -f ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log ] ; then
		cat ${RSYSLOG_DYNNAME}.spool/es-bulk-errors.log
	fi
	error_exit 1 stacktrace
fi

exit_test
