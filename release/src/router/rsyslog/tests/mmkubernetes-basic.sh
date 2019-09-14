#!/bin/bash
# added 2018-04-06 by richm, released under ASL 2.0
#
# Note: on buidbot VMs (where there is no environment cleanup), the
# kubernetes test server may be kept running if the script aborts or
# is aborted (buildbot master failure!) for some reason. As such we
# execute it under "timeout" control, which ensure it always is
# terminated. It's not a 100% great method, but hopefully does the
# trick. -- rgerhards, 2018-07-21
#export RSYSLOG_DEBUG="debug"
USE_VALGRIND=false
. $srcdir/diag.sh init
check_command_available timeout
pwd=$( pwd )
k8s_srv_port=$( get_free_port )
generate_conf
add_conf '
global(workDirectory="'$RSYSLOG_DYNNAME.spool'")
module(load="../plugins/impstats/.libs/impstats" interval="1"
	   log.file="'"$RSYSLOG_DYNNAME.spool"'/mmkubernetes-stats.log" log.syslog="off" format="cee")
module(load="../plugins/imfile/.libs/imfile")
module(load="../plugins/mmjsonparse/.libs/mmjsonparse")
module(load="../contrib/mmkubernetes/.libs/mmkubernetes")

template(name="mmk8s_template" type="list") {
    property(name="$!all-json-plain")
    constant(value="\n")
}

input(type="imfile" file="'$RSYSLOG_DYNNAME.spool'/pod-*.log" tag="kubernetes" addmetadata="on")
action(type="mmjsonparse" cookie="")
action(type="mmkubernetes" busyretryinterval="1" token="dummy" kubernetesurl="http://localhost:'$k8s_srv_port'"
       filenamerules=["rule=:'$pwd/$RSYSLOG_DYNNAME.spool'/%pod_name:char-to:.%.%container_hash:char-to:_%_%namespace_name:char-to:_%_%container_name_and_id:char-to:.%.log",
	                  "rule=:'$pwd/$RSYSLOG_DYNNAME.spool'/%pod_name:char-to:_%_%namespace_name:char-to:_%_%container_name_and_id:char-to:.%.log"]
)
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="mmk8s_template")
'

testsrv=mmk8s-test-server
echo starting kubernetes \"emulator\"
timeout 2m python -u $srcdir/mmkubernetes_test_server.py $k8s_srv_port ${RSYSLOG_DYNNAME}${testsrv}.pid ${RSYSLOG_DYNNAME}${testsrv}.started > ${RSYSLOG_DYNNAME}.spool/mmk8s_srv.log 2>&1 &
BGPROCESS=$!
wait_process_startup ${RSYSLOG_DYNNAME}${testsrv} ${RSYSLOG_DYNNAME}${testsrv}.started
echo background mmkubernetes_test_server.py process id is $BGPROCESS

cat > ${RSYSLOG_DYNNAME}.spool/pod-error1.log <<EOF
{"log":"not in right format","stream":"stdout","time":"2018-04-06T17:26:34.492083106Z","testid":1}
EOF
cat > ${RSYSLOG_DYNNAME}.spool/pod-error2.log <<EOF
{"message":"not in right format","CONTAINER_NAME":"not in right format","CONTAINER_ID_FULL":"id3","testid":2}
EOF
cat > ${RSYSLOG_DYNNAME}.spool/pod-name1_namespace-name1_container-name1-id1.log <<EOF
{"log":"{\"type\":\"response\",\"@timestamp\":\"2018-04-06T17:26:34Z\",\"tags\":[],\"pid\":75,\"method\":\"head\",\"statusCode\":200,\"req\":{\"url\":\"/\",\"method\":\"head\",\"headers\":{\"user-agent\":\"curl/7.29.0\",\"host\":\"localhost:5601\",\"accept\":\"*/*\"},\"remoteAddress\":\"127.0.0.1\",\"userAgent\":\"127.0.0.1\"},\"res\":{\"statusCode\":200,\"responseTime\":1,\"contentLength\":9},\"message\":\"HEAD1 / 200 1ms - 9.0B\"}\n","stream":"stdout","time":"2018-04-06T17:26:34.492083106Z","testid":3}
EOF
cat > ${RSYSLOG_DYNNAME}.spool/pod-name2.container-hash2_namespace-name2_container-name2-id2.log <<EOF
{"log":"{\"type\":\"response\",\"@timestamp\":\"2018-04-06T17:26:34Z\",\"tags\":[],\"pid\":75,\"method\":\"head\",\"statusCode\":200,\"req\":{\"url\":\"/\",\"method\":\"head\",\"headers\":{\"user-agent\":\"curl/7.29.0\",\"host\":\"localhost:5601\",\"accept\":\"*/*\"},\"remoteAddress\":\"127.0.0.1\",\"userAgent\":\"127.0.0.1\"},\"res\":{\"statusCode\":200,\"responseTime\":1,\"contentLength\":9},\"message\":\"HEAD2 / 200 1ms - 9.0B\"}\n","stream":"stdout","time":"2018-04-06T17:26:34.492083106Z","testid":4}
EOF
cat > ${RSYSLOG_DYNNAME}.spool/pod-name3.log <<EOF
{"message":"a message from container 3","CONTAINER_NAME":"some-prefix_container-name3.container-hash3_pod-name3_namespace-name3_unused3_unused33","CONTAINER_ID_FULL":"id3","testid":5}
EOF
cat > ${RSYSLOG_DYNNAME}.spool/pod-name4.log <<EOF
{"message":"a message from container 4","CONTAINER_NAME":"some-prefix_container-name4_pod-name4_namespace-name4_unused4_unused44","CONTAINER_ID_FULL":"id4","testid":6}
EOF
cat > ${RSYSLOG_DYNNAME}.spool/pod-name5.log <<EOF
{"message":"a message from container 5","CONTAINER_NAME":"some-prefix_container-name5_pod-name5.with.dot.in.pod.name_namespace-name5_unused5_unused55","CONTAINER_ID_FULL":"id5","testid":7}
EOF

if [ "x${USE_VALGRIND:-false}" == "xtrue" ] ; then
	export EXTRA_VALGRIND_SUPPRESSIONS="--suppressions=$srcdir/mmkubernetes.supp"
	startup_vg
else
	startup
fi
# wait for the first batch of tests to complete
wait_queueempty

cat > ${RSYSLOG_DYNNAME}.spool/pod-test-not-found-and-busy.log <<EOF
{"message":"this record should have no namespace metadata","CONTAINER_NAME":"some-prefix_container-name-6_pod-name-6_namespace-name-6-not-found_unused6_unused66","CONTAINER_ID_FULL":"id6","testid":8}
{"message":"this record should have no pod metadata","CONTAINER_NAME":"some-prefix_container-name-7_pod-name-7-not-found_namespace-name-7_unused7_unused77","CONTAINER_ID_FULL":"id7","testid":9}
{"message":"this record should have no namespace or pod metadata and retry","CONTAINER_NAME":"some-prefix_container-name-8_pod-name-8_namespace-name-8-busy_unused8_unused88","CONTAINER_ID_FULL":"id8","testid":10}
EOF

wait_queueempty
sleep 5 # greater than busyretryinterval

cat >> ${RSYSLOG_DYNNAME}.spool/pod-test-not-found-and-busy.log <<EOF
{"message":"this record should have namespace and pod metadata after retry","CONTAINER_NAME":"some-prefix_container-name-8_pod-name-8_namespace-name-8-busy_unused8_unused88","CONTAINER_ID_FULL":"id8","testid":11}
{"message":"this record should have no pod metadata and retry","CONTAINER_NAME":"some-prefix_container-name-9_pod-name-9-busy_namespace-name-9_unused9_unused99","CONTAINER_ID_FULL":"id9","testid":12}
EOF

wait_queueempty
sleep 5 # greater than busyretryinterval

cat >> ${RSYSLOG_DYNNAME}.spool/pod-test-not-found-and-busy.log <<EOF
{"message":"this record should have pod metadata after retry","CONTAINER_NAME":"some-prefix_container-name-9_pod-name-9-busy_namespace-name-9_unused9_unused99","CONTAINER_ID_FULL":"id9","testid":13}
{"message":"this record should process normally","CONTAINER_NAME":"some-prefix_container-name-10_pod-name-10_namespace-name-10_unused10_unused100","CONTAINER_ID_FULL":"id10","testid":14}
EOF

shutdown_when_empty
if [ "x${USE_VALGRIND:-false}" == "xtrue" ] ; then
	wait_shutdown_vg
	check_exit_vg
else
	wait_shutdown
fi
kill $BGPROCESS
. $srcdir/diag.sh wait-pid-termination ${RSYSLOG_DYNNAME}${testsrv}.pid

rc=0
# for each record in mmkubernetes-basic.out.json, see if the matching
# record is found in $RSYSLOG_OUT_LOG
python -c 'import sys,json
k8s_srv_port = sys.argv[3]
expected = {}
for hsh in json.load(open(sys.argv[1])):
	if "testid" in hsh:
		if "kubernetes" in hsh and "master_url" in hsh["kubernetes"]:
			hsh["kubernetes"]["master_url"] = hsh["kubernetes"]["master_url"].format(k8s_srv_port=k8s_srv_port)
		expected[hsh["testid"]] = hsh
rc = 0
actual = {}
for line in open(sys.argv[2]):
	hsh = json.loads(line)
	if "testid" in hsh:
		actual[hsh["testid"]] = hsh
for testid,hsh in expected.items():
	if not testid in actual:
		print("Error: record for testid {0} not found in output".format(testid))
		rc = 1
	else:
		for kk,vv in hsh.items():
			if not kk in actual[testid]:
				print("Error: key {0} in record for testid {1} not found in output".format(kk, testid))
				rc = 1
			elif not vv == actual[testid][kk]:
				print("Error: value {0} for key {1} in record for testid {2} does not match the expected value {3}".format(str(actual[testid][kk]), kk, testid, str(vv)))
				rc = 1
sys.exit(rc)
' $srcdir/mmkubernetes-basic.out.json $RSYSLOG_OUT_LOG $k8s_srv_port || rc=$?
grep -q 'mmkubernetes: Not Found: the resource does not exist at url .*/namespaces\\\/namespace-name-6-not-found' $RSYSLOG_OUT_LOG || { echo fail1; rc=1; }
grep -q 'mmkubernetes: Not Found: the resource does not exist at url .*/pods\\\/pod-name-7-not-found' $RSYSLOG_OUT_LOG || { echo fail2; rc=1; }
grep -q 'mmkubernetes: Too Many Requests: the server is too heavily loaded to provide the data for the requested url .*/namespaces\\\/namespace-name-8-busy' $RSYSLOG_OUT_LOG || { echo fail3; rc=1; }
grep -q 'mmkubernetes: Too Many Requests: the server is too heavily loaded to provide the data for the requested url .*/pods\\\/pod-name-9-busy' $RSYSLOG_OUT_LOG || { echo fail4; rc=1; }

if [ -f ${RSYSLOG_DYNNAME}.spool/mmkubernetes-stats.log ] ; then
	cat ${RSYSLOG_DYNNAME}.spool/mmkubernetes-stats.log | \
	python -c '
import sys,json
k8s_srv_port = sys.argv[1]
expected = {"name": "mmkubernetes(http://localhost:{0})".format(k8s_srv_port),
    "origin": "mmkubernetes", "recordseen": 12, "namespacemetadatasuccess": 9,
	"namespacemetadatanotfound": 1, "namespacemetadatabusy": 1, "namespacemetadataerror": 0,
	"podmetadatasuccess": 9, "podmetadatanotfound": 1, "podmetadatabusy": 2, "podmetadataerror": 0 }
actual = {}
for line in sys.stdin:
	jstart = line.find("{")
	if jstart >= 0:
		hsh = json.loads(line[jstart:])
		if hsh["origin"] == "mmkubernetes":
			actual = hsh
assert(expected == actual)
' $k8s_srv_port || { rc=$?; echo error: expected stats not found in ${RSYSLOG_DYNNAME}.spool/mmkubernetes-stats.log; }
else
	echo error: stats file ${RSYSLOG_DYNNAME}.spool/mmkubernetes-stats.log not found
	rc=1
fi

if [ ${rc:-0} -ne 0 ]; then
	echo
	echo "FAIL: expected data not found.  $RSYSLOG_OUT_LOG is:"
	cat ${RSYSLOG_DYNNAME}.spool/mmk8s_srv.log
	cat $RSYSLOG_OUT_LOG
	cat ${RSYSLOG_DYNNAME}.spool/mmkubernetes-stats.log
	error_exit 1
fi

exit_test
