#!/bin/bash
# 
# this shell script provides commands to the common diag system. It enables
# test scripts to wait for certain conditions and initiate certain actions.
# needs support in config file.
# NOTE: this file should be included with ". diag.sh", as it otherwise is
# not always able to convey back states to the upper-level test driver
# begun 2009-05-27 by rgerhards
# This file is part of the rsyslog project, released under GPLv3
#
# This file can be customized to environment specifics via environment
# variables:
# RS_SORTCMD    Sort command to use (must support $RS_SORT_NUMERIC_OPT option). If unset,
#		"sort" is used. E.g. Solaris needs "gsort"
# RS_SORT_NUMERIC_OPT option to use for numerical sort, If unset "-g" is used.
# RS_CMPCMD     cmp command to use. If unset, "cmd" is used.
#               E.g. Solaris needs "gcmp"
# RS_HEADCMD    head command to use. If unset, "head" is used.
#               E.g. Solaris needs "ghead"
#

# environment variables:
# USE_AUTO_DEBUG "on" --> enables automatic debugging, anything else
#                turns it off

# diag system internal environment variables
# these variables are for use by test scripts - they CANNOT be
# overriden by the user
# TCPFLOOD_EXTRA_OPTS   enables to set extra options for tcpflood, usually
#                       used in tests that have a common driver where it
#                       is too hard to set these options otherwise

#valgrind="valgrind --malloc-fill=ff --free-fill=fe --log-fd=1"

# **** use the line below for very hard to find leaks! *****
#valgrind="valgrind --leak-check=full --show-leak-kinds=all --malloc-fill=ff --free-fill=fe --log-fd=1"

#valgrind="valgrind --tool=drd --log-fd=1"
#valgrind="valgrind --tool=helgrind --log-fd=1 --suppressions=$srcdir/linux_localtime_r.supp --gen-suppressions=all"
#valgrind="valgrind --tool=exp-ptrcheck --log-fd=1"
#set -o xtrace
#export RSYSLOG_DEBUG="debug nologfuncflow noprintmutexaction nostdout"
#export RSYSLOG_DEBUGLOG="log"
TB_TIMEOUT_STARTSTOP=400 # timeout for start/stop rsyslogd in tenths (!) of a second 400 => 40 sec
# note that 40sec for the startup should be sufficient even on very slow machines. we changed this from 2min on 2017-12-12
export RSYSLOG_DEBUG_TIMEOUTS_TO_STDERR="on"  # we want to know when we loose messages due to timeouts
if [ "$srcdir" == "" ]; then
	printf "\$srcdir not set, for a manual build, do 'export srcdir=\$(pwd)'\n"
fi
if [ "$TESTTOOL_DIR" == "" ]; then
	export TESTTOOL_DIR="$srcdir"
fi

# newer functionality is preferrably introduced via bash functions
# rgerhards, 2018-07-03
function rsyslog_testbench_test_url_access() {
    local missing_requirements=
    if ! hash curl 2>/dev/null ; then
        missing_requirements="'curl' is missing in PATH; Make sure you have cURL installed! Skipping test ..."
    fi

    if [ -n "${missing_requirements}" ]; then
        echo ${missing_requirements}
        exit 77
    fi

    local http_endpoint="$1"
    if ! curl --fail --max-time 30 "${http_endpoint}" 1>/dev/null 2>&1; then
        echo "HTTP endpoint '${http_endpoint}' is not reachable. Skipping test ..."
        exit 77
    else
        echo "HTTP endpoint '${http_endpoint}' is reachable! Starting test ..."
    fi
}

# function to skip a test on a specific platform
# $1 is what we check in uname, $2 (optioal) is a reason message
function skip_platform() {
	if [ "$(uname)" == "$1" ]; then
		echo "uname $(uname)"
		echo "test does not work under $1"
		if [ "$2" != "" ]; then
			echo "reason: $2"
		fi
		exit 77
	fi

}


function setvar_RS_HOSTNAME() {
	printf "### Obtaining HOSTNAME (prequisite, not actual test) ###\n"
	generate_conf
	add_conf 'module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

$template hostname,"%hostname%"
local0.* ./'${RSYSLOG_DYNNAME}'.HOSTNAME;hostname
'
	rm -f "${RSYSLOG_DYNNAME}.HOSTNAME"
	startup
	tcpflood -m1 -M "\"<128>\""
	shutdown_when_empty # shut down rsyslogd when done processing messages
	wait_shutdown	# we need to wait until rsyslogd is finished!
	export RS_HOSTNAME="$(cat ${RSYSLOG_DYNNAME}.HOSTNAME)"
	rm -f "${RSYSLOG_DYNNAME}.HOSTNAME"
	echo HOSTNAME is: $RS_HOSTNAME
}


# begin a new testconfig
#	2018-09-07:	Incremented inputs.timeout.shutdown to 60000 because kafka tests may not be 
#			finished under stress otherwise
# $1 is the instance id, if given
function generate_conf() {
	export TCPFLOOD_PORT="$(get_free_port)"
	if [ "$1" == "" ]; then
		export TESTCONF_NM="${RSYSLOG_DYNNAME}_" # this basename is also used by instance 2!
		export RSYSLOG_OUT_LOG="${RSYSLOG_DYNNAME}.out.log"
		export RSYSLOG2_OUT_LOG="${RSYSLOG_DYNNAME}_2.out.log"
		export RSYSLOG_PIDBASE="${RSYSLOG_DYNNAME}:" # also used by instance 2!
		mkdir $RSYSLOG_DYNNAME.spool
	fi
	echo 'module(load="../plugins/imdiag/.libs/imdiag")
global(inputs.timeout.shutdown="60000")
$IMDiagListenPortFileName '$RSYSLOG_DYNNAME.imdiag$1.port'
$IMDiagServerRun 0

:syslogtag, contains, "rsyslogd"  ./'${RSYSLOG_DYNNAME}$1'.started
###### end of testbench instrumentation part, test conf follows:' > ${TESTCONF_NM}$1.conf
}

# add more data to config file. Note: generate_conf must have been called
# $1 is config fragment, $2 the instance id, if given
function add_conf() {
	printf "%s" "$1" >> ${TESTCONF_NM}$2.conf
}


function rst_msleep() {
	$TESTTOOL_DIR/msleep $1
}


# compare file to expected exact content
# $1 is file to compare
function cmp_exact() {
	if [ "$1" == "" ]; then
		printf "Testbench ERROR, cmp_exact() needs filename as \$1\n"
		error_exit  1
	fi
	if [ "$EXPECTED" == "" ]; then
		printf "Testbench ERROR, cmp_exact() needs to have env var EXPECTED set!\n"
		error_exit  1
	fi
	printf "%s\n" "$EXPECTED" | cmp - "$1"
	if [ $? -ne 0 ]; then
		echo "invalid response generated"
		echo "################# $1 is:"
		cat -n ${RSYSLOG_OUT_LOG}
		echo "################# EXPECTED was:"
		printf "%s\n" "$EXPECTED" | cat -n -
		printf "\n#################### diff is:\n"
		printf "%s\n" "$EXPECTED" | diff - "$1"
		error_exit  1
	fi;
}

# code common to all startup...() functions
function startup_common() {
	instance=
	if [ "$1" == "2" ]; then
	    CONF_FILE="${TESTCONF_NM}2.conf"
	    instance=2
	elif [ "$1" == "" -o "$1" == "1" ]; then
	    CONF_FILE="${TESTCONF_NM}.conf"
	else
	    CONF_FILE="$srcdir/testsuites/$1"
	    instance=$2
	fi
	# we need to remove the imdiag port file as there are some
	# tests that start multiple times. These may get the old port
	# number if the file still exists AND timing is bad so that
	# imdiag does not genenrate the port file quickly enough on
	# startup.
	rm -f $RSYSLOG_DYNNAME.imdiag$instance.port
	if [ ! -f $CONF_FILE ]; then
	    echo "ERROR: config file '$CONF_FILE' not found!"
	    error_exit 1
	fi
	echo config $CONF_FILE is:
	cat -n $CONF_FILE
}

# wait for appearance of a specific pid file, given as $1
function wait_startup_pid() {
	if [ "$1" == "" ]; then
		echo "FAIL: testbench bug: wait_startup_called without \$1"
		error_exit 100
	fi
	i=0
	while test ! -f $1.pid; do
		$TESTTOOL_DIR/msleep 100 # wait 100 milliseconds
		let "i++"
		if test $i -gt $TB_TIMEOUT_STARTSTOP
		then
		   ps -f
		   echo "ABORT! Timeout waiting on startup (pid file $1.pid)"
		   error_exit 1
		fi
	done
	echo "$1.pid found, pid  `cat $1.pid`"
}

# special version of wait_startup_pid() for rsyslog startup
function wait_rsyslog_startup_pid() {
	wait_startup_pid $RSYSLOG_PIDBASE$1
}

# wait for startup of an arbitrary process
# $1 - pid file name
# $2 - startup file name (optional, only checked if given)
function wait_process_startup() {
	wait_startup_pid $1
	i=0
	if [ "$2" != "" ]; then
		while test ! -f "$2"; do
			$TESTTOOL_DIR/msleep 100 # wait 100 milliseconds
			ps -p `cat $1.pid` &> /dev/null
			if [ $? -ne 0 ]
			then
			   echo "ABORT! pid in $1 no longer active during startup!"
			   error_exit 1
			fi
			let "i++"
			if test $i -gt $TB_TIMEOUT_STARTSTOP
			then
			   echo "ABORT! Timeout waiting on file '$2'"
			   error_exit 1
			fi
		done
		echo "$2 seen, associated pid " `cat $1.pid`
	fi
}

# wait for file $1 to exist AND be non-empty
# $1 : file to wait for
# $2 (optional): error message to show if timeout occurs
function wait_file_exists() {
	i=0
	while true; do
		if [ -f $1 -a "$(cat $1 2> /dev/null)" != "" ]; then
			break
		fi
		$TESTTOOL_DIR/msleep 100 # wait 100 milliseconds
		let "i++"
		if test $i -gt $TB_TIMEOUT_STARTSTOP; then
		   echo "ABORT! Timeout waiting for file $1"
		   ls -l $1
		   if [ "$2" != "" ]; then
			echo "$2"
		   fi
		   error_exit 1
		fi
	done
}

# kafka special wait function: we wait for the output file in order
# to ensure Kafka/Zookeeper is actually ready to go. This is NOT
# a generic check function and must only used with those kafka tests
# that actually need it.
function kafka_wait_group_coordinator() {
echo We are waiting for kafka/zookeper being ready to deliver messages
wait_file_exists $RSYSLOG_OUT_LOG "

Non-existence of $RSYSLOG_OUT_LOG can be caused
by a problem inside zookeeper. If debug output in the receiver is enabled, one
may see this message:

\"GroupCoordinator response error: Broker: Group coordinator not available\"

In this case you may want to do a web search and/or have a look at
    https://github.com/edenhill/librdkafka/issues/799

The question, of course, is if there is nevertheless a problem in imkafka.
Usually, the wait we do inside the testbench is sufficient to handle all
Zookeeper/Kafka startup. So if the issue reoccurs, it is suggested to enable
debug output in the receiver and check for actual problems.
"
}

# wait for rsyslogd startup ($1 is the instance)
function wait_startup() {
	wait_rsyslog_startup_pid $1
	i=0
	while test ! -f ${RSYSLOG_DYNNAME}$1.started; do
		$TESTTOOL_DIR/msleep 100 # wait 100 milliseconds
		ps -p `cat $RSYSLOG_PIDBASE$1.pid` &> /dev/null
		if [ $? -ne 0 ]
		then
		   echo "ABORT! rsyslog pid no longer active during startup!"
		   error_exit 1 stacktrace
		fi
		let "i++"
		if test $i -gt $TB_TIMEOUT_STARTSTOP
		then
		   echo "ABORT! Timeout waiting on startup ('${RSYSLOG_DYNNAME}.started' file)"
		   error_exit 1
		fi
	done
	echo "rsyslogd$1 startup msg seen, pid " `cat $RSYSLOG_PIDBASE$1.pid`
	wait_file_exists $RSYSLOG_DYNNAME.imdiag$1.port
	eval export IMDIAG_PORT$1=$(cat $RSYSLOG_DYNNAME.imdiag$1.port)
	eval PORT=$IMDIAG_PORT$1
	echo "imdiag$1 port: $PORT"
	if [ "$PORT" == "" ]; then
		echo "TESTBENCH ERROR: imdiag port not found!"
		ls -l $RSYSLOG_DYNNAME*
		exit 100
	fi
}


# start rsyslogd with default params. $1 is the config file name to use
# returns only after successful startup, $2 is the instance (blank or 2!)
# RS_REDIR maybe set to redirect rsyslog output
function startup() {
	startup_common "$1" "$2"
	eval LD_PRELOAD=$RSYSLOG_PRELOAD $valgrind ../tools/rsyslogd -C -n -i$RSYSLOG_PIDBASE$instance.pid -M../runtime/.libs:../.libs -f$CONF_FILE $RS_REDIR &
	wait_startup $instance
}


# same as startup_vg, BUT we do NOT wait on the startup message!
function startup_vg_waitpid_only() {
	startup_common "$1" "$2"
	if [ "x$RS_TESTBENCH_LEAK_CHECK" == "x" ]; then
	    RS_TESTBENCH_LEAK_CHECK=full
	fi
	LD_PRELOAD=$RSYSLOG_PRELOAD valgrind $RS_TEST_VALGRIND_EXTRA_OPTS $RS_TESTBENCH_VALGRIND_EXTRA_OPTS --suppressions=$srcdir/known_issues.supp ${EXTRA_VALGRIND_SUPPRESSIONS:-} --gen-suppressions=all --log-fd=1 --error-exitcode=10 --malloc-fill=ff --free-fill=fe --leak-check=$RS_TESTBENCH_LEAK_CHECK ../tools/rsyslogd -C -n -i$RSYSLOG_PIDBASE$2.pid -M../runtime/.libs:../.libs -f$CONF_FILE &
	wait_rsyslog_startup_pid $1
}

# start rsyslogd with default params under valgrind control. $1 is the config file name to use
# returns only after successful startup, $2 is the instance (blank or 2!)
function startup_vg() {
		startup_vg_waitpid_only $1 $2
		wait_startup $2
		echo startup_vg still running
}

# same as startup-vg, except that --leak-check is set to "none". This
# is meant to be used in cases where we have to deal with libraries (and such
# that) we don't can influence and where we cannot provide suppressions as
# they are platform-dependent. In that case, we can't test for leak checks
# (obviously), but we can check for access violations, what still is useful.
function startup_vg_noleak() {
	RS_TESTBENCH_LEAK_CHECK=no
	startup_vg $*
}

# same as startup-vgthread, BUT we do NOT wait on the startup message!
function startup_vgthread_waitpid_only() {
	startup_common "$1" "$2"
	valgrind --tool=helgrind $RS_TEST_VALGRIND_EXTRA_OPTS $RS_TESTBENCH_VALGRIND_EXTRA_OPTS --log-fd=1 --error-exitcode=10 --suppressions=$srcdir/linux_localtime_r.supp ${EXTRA_VALGRIND_SUPPRESSIONS:-} --gen-suppressions=all ../tools/rsyslogd -C -n -i$RSYSLOG_PIDBASE$2.pid -M../runtime/.libs:../.libs -f$CONF_FILE &
	wait_rsyslog_startup_pid $2
}

# start rsyslogd with default params under valgrind thread debugger control.
# $1 is the config file name to use, $2 is the instance (blank or 2!)
# returns only after successful startup
function startup_vgthread() {
	startup_vgthread_waitpid_only $1 $2
	wait_startup $2
}


# inject messages via our inject interface (imdiag)
function injectmsg() {
	echo injecting $2 messages
	echo injectmsg $1 $2 $3 $4 | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT || error_exit  $?
}

# inject messages in INSTANCE 2 via our inject interface (imdiag)
function injectmsg2() {
	echo injecting $2 messages
	echo injectmsg $1 $2 $3 $4 | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT2 || error_exit  $?
	# TODO: some return state checking? (does it really make sense here?)
}


# show the current main queue size. $1 is the instance.
function get_mainqueuesize() {
	if [ "$1" == "2" ]; then
		echo getmainmsgqueuesize | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT2 || error_exit  $?
	else
		echo getmainmsgqueuesize | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT || error_exit  $?
	fi
}

# grep for (partial) content. $1 is the content to check for
function content_check() {
	cat ${RSYSLOG_OUT_LOG} | grep -qF "$1"
	if [ "$?" -ne "0" ]; then
	    printf "\n============================================================\n"
	    echo FAIL: content_check failed to find "'$1'", content is
	    cat -n ${RSYSLOG_OUT_LOG}
	    error_exit 1
	fi
}


function content_check_with_count() {
	# content check variables for Timeout
	if [ "x$3" == "x" ]; then
		timeoutend=1
	else
		timeoutend=$3
	fi
	timecounter=0

	while [  $timecounter -lt $timeoutend ]; do
		let timecounter=timecounter+1

		count=$(cat ${RSYSLOG_OUT_LOG} | grep -F "$1" | wc -l)

		if [ $count -eq $2 ]; then
			echo content_check_with_count success, \"$1\" occured $2 times
			break
		else
			if [ "x$timecounter" == "x$timeoutend" ]; then
				shutdown_when_empty
				wait_shutdown

				echo content_check_with_count failed, expected \"$1\" to occur $2 times, but found it $count times
				echo file ${RSYSLOG_OUT_LOG} content is:
				sort < ${RSYSLOG_OUT_LOG}
				error_exit 1
			else
				echo content_check_with_count have $count, wait for $2 times $1...
				$TESTTOOL_DIR/msleep 1000
			fi
		fi
	done
}


function custom_content_check() {
	cat $2 | grep -qF "$1"
	if [ "$?" -ne "0" ]; then
	    echo FAIL: custom_content_check failed to find "'$1'" inside "'$2'"
	    echo "file contents:"
	    cat -n $2
	    error_exit 1
	fi
}


# wait for main message queue to be empty. $1 is the instance.
function wait_queueempty() {
	if [ "$1" == "2" ]; then
		echo WaitMainQueueEmpty | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT2 || error_exit  $?
	else
		echo WaitMainQueueEmpty | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT || error_exit  $?
	fi
}


# shut rsyslogd down when main queue is empty. $1 is the instance.
function shutdown_when_empty() {
	if [ "$1" == "2" ]; then
	   echo Shutting down instance 2
	else
	   echo Shutting down instance 1
	fi
	wait_queueempty $1
	if [ "$RSYSLOG_PIDBASE" == "" ]; then
		echo "RSYSLOG_PIDBASE is EMPTY! - bug in test? (instance $1)"
		exit 1
	fi
	cp $RSYSLOG_PIDBASE$1.pid $RSYSLOG_PIDBASE$1.pid.save
	$TESTTOOL_DIR/msleep 500 # wait a bit (think about slow testbench machines!)
	kill `cat $RSYSLOG_PIDBASE$1.pid` # note: we do not wait for the actual termination!
}


# actually, we wait for rsyslog.pid to be deleted.
# $1 is the instance
function wait_shutdown() {
	i=0
	out_pid=`cat $RSYSLOG_PIDBASE$1.pid.save`
	echo wait on shutdown of $out_pid
	if [[ "x$out_pid" == "x" ]]
	then
		terminated=1
	else
		terminated=0
	fi
	while [[ $terminated -eq 0 ]]; do
		ps -p $out_pid &> /dev/null
		if [[ $? != 0 ]]
		then
			terminated=1
		fi
		$TESTTOOL_DIR/msleep 100 # wait 100 milliseconds
		let "i++"
		if test $i -gt $TB_TIMEOUT_STARTSTOP
		then
		   echo "ABORT! Timeout waiting on shutdown"
		   echo "Instance is possibly still running and may need"
		   echo "manual cleanup."
		   exit 1
		fi
	done
	unset terminated
	unset out_pid
	if [ -e core.* ]
	then
	   echo "ABORT! core file exists"
	   error_exit  1
	fi
}


# wait for all pending lookup table reloads to complete $1 is the instance.
function await_lookup_table_reload() {
	if [ "$1" == "2" ]; then
		echo AwaitLookupTableReload | $TESTTOOL_DIR/diagtalker -pIMDIAG_PORT2 || error_exit  $?
	else
		echo AwaitLookupTableReload | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT || error_exit  $?
	fi
}


function assert_content_missing() {
	cat ${RSYSLOG_OUT_LOG} | grep -qF "$1"
	if [ "$?" -eq "0" ]; then
		echo content-missing assertion failed, some line matched pattern "'$1'"
		error_exit 1
	fi
}


function custom_assert_content_missing() {
	cat $2 | grep -qF "$1"
	if [ "$?" -eq "0" ]; then
		echo content-missing assertion failed, some line in "'$2'" matched pattern "'$1'"
		cat -n "$2"
		error_exit 1
	fi
}


# shut rsyslogd down when main queue is empty. $1 is the instance.
function issue_HUP() {
	kill -HUP `cat $RSYSLOG_PIDBASE$1.pid`
	$TESTTOOL_DIR/msleep 1000
}


# actually, we wait for rsyslog.pid to be deleted. $1 is the instance
function wait_shutdown_vg() {
	wait `cat $RSYSLOG_PIDBASE$1.pid`
	export RSYSLOGD_EXIT=$?
	echo rsyslogd run exited with $RSYSLOGD_EXIT
	if [ -e vgcore.* ]; then
	   echo "ABORT! core file exists"
	   error_exit 1
	fi
}


# check exit code for valgrind error
function check_exit_vg(){
	if [ "$RSYSLOGD_EXIT" -eq "10" ]; then
		echo "valgrind run FAILED with exceptions - terminating"
		error_exit 1
	fi
}

# this is called if we had an error and need to abort. Here, we
# try to gather as much information as possible. That's most important
# for systems like Travis-CI where we cannot debug on the machine itself.
# our $1 is the to-be-used exit code. if $2 is "stacktrace", call gdb.
function error_exit() {
	#env
	if [ -e core* ]
	then
		echo trying to obtain crash location info
		echo note: this may not be the correct file, check it
		CORE=`ls core*`
		echo "bt" >> gdb.in
		echo "q" >> gdb.in
		gdb ../tools/rsyslogd $CORE -batch -x gdb.in
		CORE=
		rm gdb.in
	fi
	if [[ "$2" == 'stacktrace' || ( ! -e IN_AUTO_DEBUG &&  "$USE_AUTO_DEBUG" == 'on' ) ]]; then
		if [ -e core* ]
		then
			echo trying to analyze core for main rsyslogd binary
			echo note: this may not be the correct file, check it
			CORE=`ls core*`
			#echo "set pagination off" >gdb.in
			#echo "core $CORE" >>gdb.in
			echo "bt" >> gdb.in
			echo "echo === THREAD INFO ===" >> gdb.in
			echo "info thread" >> gdb.in
			echo "echo === thread apply all bt full ===" >> gdb.in
			echo "thread apply all bt full" >> gdb.in
			echo "q" >> gdb.in
			gdb ../tools/rsyslogd $CORE -batch -x gdb.in
			CORE=
			rm gdb.in
		fi
	fi
	if [[ ! -e IN_AUTO_DEBUG &&  "$USE_AUTO_DEBUG" == 'on' ]]; then
		touch IN_AUTO_DEBUG
		# OK, we have the testname and can re-run under valgrind
		echo re-running under valgrind control
		current_test="./$(basename $0)" # this path is probably wrong -- theinric
		$current_test
		# wait a little bit so that valgrind can finish
		$TESTTOOL_DIR/msleep 4000
		# next let's try us to get a debug log
		RSYSLOG_DEBUG_SAVE=$RSYSLOG_DEBUG
		export RSYSLOG_DEBUG="debug nologfuncflow noprintmutexaction"
		$current_test
		$TESTTOOL_DIR/msleep 4000
		RSYSLOG_DEBUG=$RSYSLOG_DEBUG_SAVE
		rm IN_AUTO_DEBUG
	fi
	# Extended debug output for dependencies started by testbench
	if [[ "$EXTRA_EXITCHECK" == 'dumpkafkalogs' ]]; then
		# Dump Zookeeper log
		. $srcdir/diag.sh dump-zookeeper-serverlog
		# Dump Kafka log
		. $srcdir/diag.sh dump-kafka-serverlog
	fi
	# output listening ports as a temporay debug measure (2018-09-08 rgerhards)
	if [ $(uname) == "Linux" ]; then
		netstat -tlp
	else
		netstat
	fi
	# we need to do some minimal cleanup so that "make distcheck" does not
	# complain too much
	exit $1
}


# do the usual sequence check to see if everything was properly received.
# $4... are just to have the abilit to pass in more options...
# add -v to chkseq if you need more verbose output
function seq_check() {
	$RS_SORTCMD $RS_SORT_NUMERIC_OPT < ${RSYSLOG_OUT_LOG} | ./chkseq -s$1 -e$2 $3 $4 $5 $6 $7
	if [ "$?" -ne "0" ]; then
		echo "sequence error detected in $RSYSLOG_OUT_LOG"
		echo "number of lines in file: $(wc -l $RSYSLOG_OUT_LOG)"
		echo "sorted data has been placed in error.log"
		$RS_SORTCMD $RS_SORT_NUMERIC_OPT < ${RSYSLOG_OUT_LOG} > error.log
		error_exit 1 
	fi
}


# do the usual sequence check to see if everything was properly received. This is
# a duplicateof seq-check, but we could not change its calling conventions without
# breaking a lot of exitings test cases, so we preferred to duplicate the code here.
# $4... are just to have the abilit to pass in more options...
# add -v to chkseq if you need more verbose output
function seq_check2() {
	$RS_SORTCMD $RS_SORT_NUMERIC_OPT < ${RSYSLOG2_OUT_LOG}  | ./chkseq -s$1 -e$2 $3 $4 $5 $6 $7
	if [ "$?" -ne "0" ]; then
		echo "sequence error detected"
		error_exit 1
	fi
}


# do the usual sequence check, but for gzip files
# $4... are just to have the abilit to pass in more options...
function gzip_seq_check() {
	ls -l ${RSYSLOG_OUT_LOG}
	gunzip < ${RSYSLOG_OUT_LOG} | $RS_SORTCMD $RS_SORT_NUMERIC_OPT | ./chkseq -v -s$1 -e$2 $3 $4 $5 $6 $7
	if [ "$?" -ne "0" ]; then
		echo "sequence error detected"
		error_exit 1
	fi
}


# do a tcpflood run and check if it worked params are passed to tcpflood
function tcpflood() {
	eval ./tcpflood -p$TCPFLOOD_PORT "$@" $TCPFLOOD_EXTRA_OPTS
	if [ "$?" -ne "0" ]; then
		echo "error during tcpflood on port ${TCPFLOOD_PORT}! see ${RSYSLOG_OUT_LOG}.save for what was written"
		cp ${RSYSLOG_OUT_LOG} ${RSYSLOG_OUT_LOG}.save
		error_exit 1 stacktrace
	fi
}


# cleanup
# detect any left-over hanging instance
function exit_test() {
	nhanging=0
	#for pid in $(ps -eo pid,args|grep '/tools/[r]syslogd ' |sed -e 's/\( *\)\([0-9]*\).*/\2/');
	#do
		#echo "ERROR: left-over instance $pid, killing it"
	#	ps -fp $pid
	#	pwd
	#	printf "we do NOT kill the instance as this does not work with multiple\n"
	#	printf "builds per machine - this message is now informational to show prob exists!\n"
	#	#kill -9 $pid
	#	let "nhanging++"
	#done
	if test $nhanging -ne 0
	then
	   echo "ABORT! hanging instances left at exit"
	   #error_exit 1
	   #exit 77 # for now, just skip - TODO: reconsider when supporting -j
	fi
	# now real cleanup
	rm -f rsyslog.action.*.include
	rm -f work rsyslog.out.* xlate*.lkp_tbl
	rm -rf test-logdir stat-file1
	rm -f rsyslog.conf.tlscert stat-file1 rsyslog.empty imfile-state*
	rm -rf rsyslog-link.*.log targets
	rm -f ${TESTCONF_NM}.conf
	rm -f tmp.qi nocert
	rm -fr $RSYSLOG_DYNNAME*  # delete all of our dynamic files
	unset TCPFLOOD_EXTRA_OPTS
	printf "Test SUCCESFUL\n"
	echo  -------------------------------------------------------------------------------
}

# finds a free port that we can bind a listener to
# Obviously, any solution is race as another process could start
# just after us and grab the same port. However, in practice it seems
# to work pretty well. In any case, we should probably call this as
# late as possible before the usage of the port.
function get_free_port() {
python -c 'import socket; s=socket.socket(); s.bind(("", 0)); print(s.getsockname()[1]); s.close()'
}

# check if command $1 is available - will exit 77 when not OK
function check_command_available() {
	command -v $1
	if [ $? -ne 0 ] ; then
		echo Testbench requires unavailable command: $1
		exit 77
	fi
}


# sort the output file just like we normally do it, but do not call
# seqchk. This is needed for some operations where we need the sort
# result for some preprocessing. Note that a later seqchk will sort
# again, but that's not an issue.
function presort() {
	rm -f $RSYSLOG_DYNNAME.presort
	$RS_SORTCMD $RS_SORT_NUMERIC_OPT < ${RSYSLOG_OUT_LOG} > $RSYSLOG_DYNNAME.presort
}


#START: ext kafka config
#dep_cache_dir=$(readlink -f .dep_cache)
dep_cache_dir=$(pwd)/.dep_cache
dep_zk_url=http://www-us.apache.org/dist/zookeeper/zookeeper-3.4.13/zookeeper-3.4.13.tar.gz
dep_zk_cached_file=$dep_cache_dir/zookeeper-3.4.13.tar.gz

# byANDRE: We stay with kafka 0.10.x for now. Newer Kafka Versions have changes that
#	makes creating testbench with single kafka instances difficult.
# old version -> dep_kafka_url=http://www-us.apache.org/dist/kafka/0.10.2.2/kafka_2.12-0.10.2.2.tgz
# old version -> dep_kafka_cached_file=$dep_cache_dir/kafka_2.12-0.10.2.2.tgz
dep_kafka_url=http://www-us.apache.org/dist/kafka/2.0.0/kafka_2.12-2.0.0.tgz
dep_kafka_cached_file=$dep_cache_dir/kafka_2.12-2.0.0.tgz

if [ -z "$ES_DOWNLOAD" ]; then
	export ES_DOWNLOAD=elasticsearch-5.6.9.tar.gz
fi
dep_es_cached_file="$dep_cache_dir/$ES_DOWNLOAD"

# kafaka (including Zookeeper)
dep_kafka_dir_xform_pattern='s#^[^/]\+#kafka#g'
dep_zk_dir_xform_pattern='s#^[^/]\+#zk#g'
dep_es_dir_xform_pattern='s#^[^/]\+#es#g'
#dep_kafka_log_dump=$(readlink -f rsyslog.out.kafka.log)

#	TODO Make dynamic work dir for multiple instances
#dep_work_dir=$(readlink -f .dep_wrk)
dep_work_dir=$(pwd)/.dep_wrk
#dep_kafka_work_dir=$dep_work_dir/kafka
#dep_zk_work_dir=$dep_work_dir/zk

#END: ext kafka config

case $1 in
   'init')	$srcdir/killrsyslog.sh # kill rsyslogd if it runs for some reason
		# for (solaris) load debugging, uncomment next 2 lines:
		#export LD_DEBUG=all
		#ldd ../tools/rsyslogd

		# environment debug
		#find / -name "librelp.so*"
		#ps -ef |grep syslog
		#netstat -a | grep LISTEN

		# cleanup of hanging instances from previous runs
		# practice has shown this is pretty useful!
		#for pid in $(ps -eo pid,args|grep '/tools/[r]syslogd ' |sed -e 's/\( *\)\([0-9]*\).*/\2/');
		#do
			#echo "ERROR: left-over previous instance $pid, killing it"
			#ps -fp $pid
			#pwd
			#kill -9 $pid
		#done
		# cleanup hanging uxsockrcvr processes
		for pid in $(ps -eo pid,args|grep 'uxsockrcvr' |grep -v grep |sed -e 's/\( *\)\([0-9]*\).*/\2/');
		do
			echo "ERROR: left-over previous uxsockrcvr instance $pid, killing it"
			ps -fp $pid
			pwd
			kill -9 $pid
		done
		# end cleanup

		# some default names (later to be set in other parts, once we support fully
		# parallel tests)
		export RSYSLOG_DFLT_LOG_INTERNAL=1 # testbench needs internal messages logged internally!
		if [ "$SYSLOG_DYNNAME" != "" ]; then
			echo "FAIL: \$RSYSLOG_DYNNAME already set in init"
			echo "hint: was init accidently called twice?"
			exit 2
		fi
		export RSYSLOG_DYNNAME="rstb_$(./test_id $(basename $0))"
		export RSYSLOG2_OUT_LOG=rsyslog2.out.log # TODO: remove
		export RSYSLOG_OUT_LOG=rsyslog.out.log # TODO: remove
		export RSYSLOG_PIDBASE="rsyslog" # TODO: remove
		export IMDIAG_PORT=13500
		export IMDIAG_PORT2=13501
		export TCPFLOOD_PORT=13514

		# we create one file with the test name, so that we know what was
		# left over if "make distcheck" complains
		touch $RSYSLOG_DYNNAME-$(basename $0).test_id

		if [ -z $RS_SORTCMD ]; then
			RS_SORTCMD=sort
		fi
		if [ -z $RS_SORT_NUMERIC_OPT ]; then
			if [ "$(uname)" == "AIX" ]; then
				RS_SORT_NUMERIC_OPT=-n
			else
				RS_SORT_NUMERIC_OPT=-g
			fi
		fi
		if [ -z $RS_CMPCMD ]; then
			RS_CMPCMD=cmp
		fi
		if [ -z $RS_HEADCMD ]; then
			RS_HEADCMD=head
		fi
		ulimit -c unlimited  &> /dev/null # at least try to get core dumps
		echo "------------------------------------------------------------"
		echo "Test: $0"
		echo "------------------------------------------------------------"
		# we assume TZ is set, else most test will fail. So let's ensure
		# this really is the case
		if [ -z $TZ ]; then
			echo "testbench: TZ env var not set, setting it to UTC"
			export TZ=UTC
		fi
		rm -f xlate*.lkp_tbl
		rm -f log log* # RSyslog debug output 
		rm -f work 
		rm -rf test-logdir stat-file1
		rm -f rsyslog.empty imfile-state* omkafka-failed.data
		rm -rf rsyslog-link.*.log targets
		rm -f tmp.qi nocert
		rm -f core.* vgcore.* core*
		# Note: rsyslog.action.*.include must NOT be deleted, as it
		# is used to setup some parameters BEFORE calling init. This
		# happens in chained test scripts. Delete on exit is fine,
		# though.
		# note: TCPFLOOD_EXTRA_OPTS MUST NOT be unset in init, because
		# some tests need to set it BEFORE calling init to accomodate
		# their generic test drivers.
		if [ "$TCPFLOOD_EXTRA_OPTS" != '' ] ; then
		        echo TCPFLOOD_EXTRA_OPTS set: $TCPFLOOD_EXTRA_OPTS
                fi
		if [ "$USE_AUTO_DEBUG" != 'on' ] ; then
			rm -f IN_AUTO_DEBUG
                fi
		if [ -e IN_AUTO_DEBUG ]; then
			export valgrind="valgrind --malloc-fill=ff --free-fill=fe --suppressions=$srcdir/known_issues.supp ${EXTRA_VALGRIND_SUPPRESSIONS:-} --log-fd=1"
		fi
		;;

   'check-ipv6-available')   # check if IPv6  - will exit 77 when not OK
		ifconfig -a |grep ::1
		if [ $? -ne 0 ] ; then
			echo this test requires an active IPv6 stack, which we do not have here
			exit 77
		fi
		;;
   'es-init')   # initialize local Elasticsearch *testbench* instance for the next
                # test. NOTE: do NOT put anything useful on that instance!
		curl --silent -XDELETE localhost:${ES_PORT:-9200}/rsyslog_testbench
		;;
   'es-getdata') # read data from ES to a local file so that we can process
		if [ "x$3" == "x" ]; then
			es_get_port=9200
		else
			es_get_port=$3
		fi

   		# it with out regular tooling.
		# Note: param 2 MUST be number of records to read (ES does
		# not return the full set unless you tell it explicitely).
		curl --silent localhost:$es_get_port/rsyslog_testbench/_search?size=$2 > work
		python $srcdir/es_response_get_msgnum.py > ${RSYSLOG_OUT_LOG}
		;;
   'getpid')
		pid=$(cat $RSYSLOG_PIDBASE$2.pid)
		;;
   'wait-pid-termination')  # wait for the pid in pid $2 to terminate, abort on timeout
		i=0
		out_pid=$2
		if [[ "x$out_pid" == "x" ]]
		then
			terminated=1
		else
			terminated=0
		fi
		while [[ $terminated -eq 0 ]]; do
			ps -p $out_pid &> /dev/null
			if [[ $? != 0 ]]
			then
				terminated=1
			fi
			$TESTTOOL_DIR/msleep 100 # wait 100 milliseconds
			let "i++"
			if test $i -gt $TB_TIMEOUT_STARTSTOP
			then
			   echo "ABORT! Timeout waiting on shutdown"
			   echo "on PID file $2"
			   echo "Instance is possibly still running and may need"
			   echo "manual cleanup."
			   exit 1
			fi
		done
		unset terminated
		unset out_pid
		;;
   'shutdown-immediate') # shut rsyslogd down without emptying the queue. $2 is the instance.
		cp $RSYSLOG_PIDBASE$2.pid $RSYSLOG_PIDBASE$2.pid.save
		kill `cat $RSYSLOG_PIDBASE$2.pid`
		# note: we do not wait for the actual termination!
		;;
   'kill-immediate') # kill rsyslog unconditionally
		kill -9 `cat $RSYSLOG_PIDBASE.pid`
		# note: we do not wait for the actual termination!
		;;
    'injectmsg-litteral') # inject litteral-payload  via our inject interface (imdiag)
		echo injecting msg payload from: $2
    cat $2 | sed -e 's/^/injectmsg litteral /g' | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT || error_exit  $?
		# TODO: some return state checking? (does it really make sense here?)
		;;
   'check-mainq-spool') # check if mainqueue spool files exist, if not abort (we just check .qi).
		echo There must exist some files now:
		ls -l $RSYSLOG_DYNNAME.spool
		echo .qi file:
		cat $RSYSLOG_DYNNAME.spool/mainq.qi
		if test ! -f $RSYSLOG_DYNNAME.spool/mainq.qi; then
		  echo "error: mainq.qi does not exist where expected to do so!"
		  error_exit 1
		fi
		;;
   'assert-equal')
		if [ "x$4" == "x" ]; then
			tolerance=0
		else
			tolerance=$4
		fi
		result=$(echo $2 $3 $tolerance | awk 'function abs(v) { return v > 0 ? v : -v } { print (abs($1 - $2) <= $3) ? "PASSED" : "FAILED" }')
		if [ $result != 'PASSED' ]; then
				echo "Value '$2' != '$3' (within tolerance of $tolerance)"
		  error_exit 1
		fi
		;;
   'ensure-no-process-exists')
    ps -ef | grep -v grep | grep -qF "$2"
    if [ "x$?" == "x0" ]; then
      echo "assertion failed: process with name-fragment matching '$2' found"
		  error_exit 1
    fi
    ;;
   'grep-check') # grep for "$EXPECTED" present in rsyslog.log - env var must be set
		 # before this method is called
		grep "$EXPECTED" ${RSYSLOG_OUT_LOG} > /dev/null
		if [ $? -eq 1 ]; then
		  echo "GREP FAIL: ${RSYSLOG_OUT_LOG} content:"
		  cat ${RSYSLOG_OUT_LOG}
		  echo "GREP FAIL: expected text not found:"
		  echo "$EXPECTED"
		error_exit 1
		fi;
		;;
   'wait-file-lines') 
		# $2 filename, $3 expected nbr of lines, $4 nbr of tries
		if [ "$4" == "" ]; then
			timeoutend=1
		else
			timeoutend=$4
		fi
		timecounter=0

		while [  $timecounter -lt $timeoutend ]; do
			let timecounter=timecounter+1

			count=$(wc -l < ${RSYSLOG_OUT_LOG})
			if [ $count -eq $3 ]; then
				echo wait-file-lines success, have $3 lines
				break
			else
				if [ "x$timecounter" == "x$timeoutend" ]; then
					echo wait-file-lines failed, expected $3 got $count
					shutdown_when_empty
					wait_shutdown
					error_exit 1
				else
					echo wait-file-lines not yet there, currently $count lines
					$TESTTOOL_DIR/msleep 200
				fi
			fi
		done
		unset count
		;;
	 'block-stats-flush')
		echo blocking stats flush
		echo "blockStatsReporting" | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT || error_exit  $?
		;;
	 'await-stats-flush-after-block')
		echo unblocking stats flush and waiting for it
		echo "awaitStatsReport" | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT || error_exit  $?
		;;
	 'allow-single-stats-flush-after-block-and-wait-for-it')
		echo blocking stats flush
		echo "awaitStatsReport block_again" | $TESTTOOL_DIR/diagtalker -p$IMDIAG_PORT || error_exit  $?
		;;
	 'wait-for-stats-flush')
		echo "will wait for stats push"
		while [[ ! -f $2 ]]; do
				echo waiting for stats file "'$2'" to be created
				$TESTTOOL_DIR/msleep 100
		done
		prev_count=$(cat $2 | grep 'BEGIN$' | wc -l)
		new_count=$prev_count
		while [[ "x$prev_count" == "x$new_count" ]]; do
				new_count=$(cat $2 | grep 'BEGIN$' | wc -l) # busy spin, because it allows as close timing-coordination in actual test run as possible
		done
		echo "stats push registered"
		;;
	 'wait-for-dyn-stats-reset')
		echo "will wait for dyn-stats-reset"
		while [[ ! -f $2 ]]; do
				echo waiting for stats file "'$2'" to be created
				$TESTTOOL_DIR/msleep 100
		done
		prev_purged=$(cat $2 | grep -F 'origin=dynstats' | grep -F "${3}.purge_triggered=" | sed -e 's/.\+.purge_triggered=//g' | awk '{s+=$1} END {print s}')
		new_purged=$prev_purged
		while [[ "x$prev_purged" == "x$new_purged" ]]; do
				new_purged=$(cat $2 | grep -F 'origin=dynstats' | grep -F "${3}.purge_triggered=" | sed -e 's/.\+\.purge_triggered=//g' | awk '{s+=$1} END {print s}') # busy spin, because it allows as close timing-coordination in actual test run as possible
				$TESTTOOL_DIR/msleep 10
		done
		echo "dyn-stats reset for bucket ${3} registered"
		;;
# note needed at the moment, but let's keep it in a little, 2018-09-03 rgerhards
#   'content-check-regex')
#		# this does a content check which permits regex
#		grep "$2" $3 -q
#		if [ "$?" -ne "0" ]; then
#		    echo "----------------------------------------------------------------------"
#		    echo FAIL: content-check-regex failed to find "'$2'" inside "'$3'"
#		    echo "file contents:"
#		    cat $3
#		    error_exit 1
#		fi
#		;;
   'first-column-sum-check') 
		sum=$(cat $4 | grep $3 | sed -e $2 | awk '{s+=$1} END {print s}')
		if [ "x${sum}" != "x$5" ]; then
		    printf "\n============================================================\n"
		    echo FAIL: sum of first column with edit-expr "'$2'" run over lines from file "'$4'" matched by "'$3'" equals "'$sum'" which is NOT equal to EXPECTED value of "'$5'"
		    echo "file contents:"
		    cat $4
		    error_exit 1
		fi
		;;
   'assert-first-column-sum-greater-than') 
		sum=$(cat $4 | grep $3 | sed -e $2 | awk '{s+=$1} END {print s}')
		if [ ! $sum -gt $5 ]; then
		    echo sum of first column with edit-expr "'$2'" run over lines from file "'$4'" matched by "'$3'" equals "'$sum'" which is smaller than expected lower-limit of "'$5'"
		    echo "file contents:"
		    cat $4
		    error_exit 1
		fi
		;;
   'content-pattern-check') 
		cat ${RSYSLOG_OUT_LOG} | grep -q "$2"
		if [ "$?" -ne "0" ]; then
		    echo content-check failed, not every line matched pattern "'$2'"
		    echo "file contents:"
		    cat -n $4
		    error_exit 1
		fi
		;;
   'require-journalctl')   # check if journalctl exists on the system
		if ! hash journalctl 2>/dev/null ; then
		    echo "journalctl command missing, skipping test"
		    exit 77
		fi
		;;
   'download-kafka')
		if [ ! -d $dep_cache_dir ]; then
			echo "Creating dependency cache dir $dep_cache_dir"
			mkdir $dep_cache_dir
		fi
		if [ ! -f $dep_zk_cached_file ]; then
			echo "Downloading zookeeper"
			wget -q $dep_zk_url -O $dep_zk_cached_file
			if [ $? -ne 0 ]
			then
				echo error during wget, retry:
				wget $dep_zk_url -O $dep_zk_cached_file
				if [ $? -ne 0 ]
				then
					error_exit 1
				fi
			fi
		fi
		if [ ! -f $dep_kafka_cached_file ]; then
			echo "Downloading kafka"
			wget -q $dep_kafka_url -O $dep_kafka_cached_file
			if [ $? -ne 0 ]
			then
				echo error during wget, retry:
				wget $dep_kafka_url -O $dep_kafka_cached_file
				if [ $? -ne 0 ]
				then
					error_exit 1
				fi
			fi
		fi
		;;
	 'download-elasticsearch')
		if [ ! -d $dep_cache_dir ]; then
				echo "Creating dependency cache dir $dep_cache_dir"
				mkdir $dep_cache_dir
		fi
		if [ ! -f $dep_es_cached_file ]; then
				if [ -f /local_dep_cache/$ES_DOWNLOAD ]; then
					printf "ElasticSearch: satisfying dependency %s from system cache.\n" "$ES_DOWNLOAD"
					cp /local_dep_cache/$ES_DOWNLOAD $dep_es_cached_file
				else
					dep_es_url="https://artifacts.elastic.co/downloads/elasticsearch/$ES_DOWNLOAD"
					printf "ElasticSearch: satisfying dependency %s from %s\n" "$ES_DOWNLOAD" "$dep_es_url"
					wget -q $dep_es_url -O $dep_es_cached_file
				fi
		fi
		;;
	 'start-zookeeper')
		if [ "x$2" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
			dep_work_tk_config="zoo.cfg"
		else
			dep_work_dir=$(readlink -f $srcdir/$2)
			dep_work_tk_config="zoo$2.cfg"
		fi

		if [ ! -f $dep_zk_cached_file ]; then
				echo "Dependency-cache does not have zookeeper package, did you download dependencies?"
				exit 77
		fi
		if [ ! -d $dep_work_dir ]; then
				echo "Creating dependency working directory"
				mkdir -p $dep_work_dir
		fi
		if [ -d $dep_work_dir/zk ]; then
				(cd $dep_work_dir/zk && ./bin/zkServer.sh stop)
				$TESTTOOL_DIR/msleep 2000
		fi
		rm -rf $dep_work_dir/zk
		(cd $dep_work_dir && tar -zxvf $dep_zk_cached_file --xform $dep_zk_dir_xform_pattern --show-transformed-names) > /dev/null
		cp -f $srcdir/testsuites/$dep_work_tk_config $dep_work_dir/zk/conf/zoo.cfg
		echo "Starting Zookeeper instance $2"
		(cd $dep_work_dir/zk && ./bin/zkServer.sh start)
		$TESTTOOL_DIR/msleep 2000
		;;
	 'start-kafka')
		# Force IPv4 usage of Kafka!
		export KAFKA_OPTS="-Djava.net.preferIPv4Stack=True"
		if [ "x$2" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
			dep_work_kafka_config="kafka-server.properties"
		else
			dep_work_dir=$(readlink -f $2)
			dep_work_kafka_config="kafka-server$2.properties"
		fi

		if [ ! -f $dep_kafka_cached_file ]; then
				echo "Dependency-cache does not have kafka package, did you download dependencies?"
				exit 77
		fi
		if [ ! -d $dep_work_dir ]; then
				echo "Creating dependency working directory"
				mkdir -p $dep_work_dir
		fi
		rm -rf $dep_work_dir/kafka
		(cd $dep_work_dir && tar -zxvf $dep_kafka_cached_file --xform $dep_kafka_dir_xform_pattern --show-transformed-names) > /dev/null
		cp -f $srcdir/testsuites/$dep_work_kafka_config $dep_work_dir/kafka/config/
		echo "Starting Kafka instance $dep_work_kafka_config"
		(cd $dep_work_dir/kafka && ./bin/kafka-server-start.sh -daemon ./config/$dep_work_kafka_config)
		$TESTTOOL_DIR/msleep 4000

		# Check if kafka instance came up!
		kafkapid=`ps aux | grep -i $dep_work_kafka_config | grep java | grep -v grep | awk '{print $2}'`
		if [[ "" !=  "$kafkapid" ]];
		then
			echo "Kafka instance $dep_work_kafka_config started with PID $kafkapid"
		else
			echo "Starting Kafka instance $dep_work_kafka_config, SECOND ATTEMPT!"
			(cd $dep_work_dir/kafka && ./bin/kafka-server-start.sh -daemon ./config/$dep_work_kafka_config)
			$TESTTOOL_DIR/msleep 4000

			kafkapid=`ps aux | grep -i $dep_work_kafka_config | grep java | grep -v grep | awk '{print $2}'`
			if [[ "" !=  "$kafkapid" ]];
			then
				echo "Kafka instance $dep_work_kafka_config started with PID $kafkapid"
			else
				echo "Failed to start Kafka instance for $dep_work_kafka_config"
				error_exit 77
			fi
		fi
		;;
	 'prepare-elasticsearch') # $2, if set, is the number of additional ES instances
		# Heap Size (limit to 128MB for testbench! defaults is way to HIGH)
		export ES_JAVA_OPTS="-Xms128m -Xmx128m"

		if [ "x$2" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
			dep_work_es_config="es.yml"
			dep_work_es_pidfile="es.pid"
		else
			dep_work_dir=$(readlink -f $srcdir/$2)
			dep_work_es_config="es$2.yml"
			dep_work_es_pidfile="es$2.pid"
		fi

		if [ ! -f $dep_es_cached_file ]; then
				echo "Dependency-cache does not have elasticsearch package, did "
				echo "you download dependencies?"
		                error_exit 1
		fi
		if [ ! -d $dep_work_dir ]; then
				echo "Creating dependency working directory"
				mkdir -p $dep_work_dir
		fi
		if [ -d $dep_work_dir/es ]; then
			if [ -e $dep_work_es_pidfile ]; then
				es_pid = $(cat $dep_work_es_pidfile)
				kill -SIGTERM $es_pid
				. $srcdir/diag.sh wait-pid-termination $es_pid
			fi
		fi
		rm -rf $dep_work_dir/es
		echo TEST USES ELASTICSEARCH BINARY $dep_es_cached_file
		(cd $dep_work_dir && tar -zxf $dep_es_cached_file --xform $dep_es_dir_xform_pattern --show-transformed-names) > /dev/null
		if [ -n "${ES_PORT:-}" ] ; then
			rm -f $dep_work_dir/es/config/elasticsearch.yml
			sed "s/^http.port:.*\$/http.port: ${ES_PORT}/" $srcdir/testsuites/$dep_work_es_config > $dep_work_dir/es/config/elasticsearch.yml
		else
			cp -f $srcdir/testsuites/$dep_work_es_config $dep_work_dir/es/config/elasticsearch.yml
		fi

		if [ ! -d $dep_work_dir/es/data ]; then
				echo "Creating elastic search directories"
				mkdir -p $dep_work_dir/es/data
				mkdir -p $dep_work_dir/es/logs
				mkdir -p $dep_work_dir/es/tmp
		fi
		echo ElasticSearch prepared for use in test.
		;;
	 'start-elasticsearch') # $2, if set, is the number of additional ES instances
		# Heap Size (limit to 128MB for testbench! defaults is way to HIGH)
		export ES_JAVA_OPTS="-Xms128m -Xmx128m"

		pwd
		if [ "x$2" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
			dep_work_es_config="es.yml"
			dep_work_es_pidfile="$(pwd)/es.pid"
		else
			dep_work_dir=$(readlink -f $srcdir/$2)
			dep_work_es_config="es$2.yml"
			dep_work_es_pidfile="es$2.pid"
		fi
		echo "Starting ElasticSearch instance $2"
		# THIS IS THE ACTUAL START of ES
		$dep_work_dir/es/bin/elasticsearch -p $dep_work_es_pidfile -d
		$TESTTOOL_DIR/msleep 2000
		echo "Starting instance $2 started with PID" `cat $dep_work_es_pidfile`

		# Wait for startup with hardcoded timeout
		timeoutend=60
		timeseconds=0
		# Loop until elasticsearch port is reachable or until
		# timeout is reached!
		until [ "`curl --silent --show-error --connect-timeout 1 http://localhost:${ES_PORT:-19200} | grep 'rsyslog-testbench'`" != "" ]; do
			echo "--- waiting for ES startup: $timeseconds seconds"
			$TESTTOOL_DIR/msleep 1000
			let "timeseconds = $timeseconds + 1"

			if [ "$timeseconds" -gt "$timeoutend" ]; then 
				echo "--- TIMEOUT ( $timeseconds ) reached!!!"
		                error_exit 1
			fi
		done
		$TESTTOOL_DIR/msleep 2000
		echo ES startup succeeded
		;;
	 'dump-kafka-serverlog')
		if [ "x$2" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
		else
			dep_work_dir=$(readlink -f $srcdir/$2)
		fi
		if [ ! -d $dep_work_dir/kafka ]; then
			echo "Kafka work-dir $dep_work_dir/kafka does not exist, no kafka debuglog"
		else
			echo "Dumping server.log from Kafka instance $2"
			echo "========================================="
			cat $dep_work_dir/kafka/logs/server.log
			echo "========================================="
		fi
		;;
		
	 'dump-zookeeper-serverlog')
		if [ "x$2" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
		else
			dep_work_dir=$(readlink -f $srcdir/$2)
		fi
		echo "Dumping zookeeper.out from Zookeeper instance $2"
		echo "========================================="
		cat $dep_work_dir/zk/zookeeper.out
		echo "========================================="
		;;
	 'stop-kafka')
		if [ "x$2" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
		else
			dep_work_dir=$(readlink -f $srcdir/$2)
		fi
		if [ ! -d $dep_work_dir/kafka ]; then
			echo "Kafka work-dir $dep_work_dir/kafka does not exist, no action needed"
		else
			echo "Stopping Kafka instance $2"
			(cd $dep_work_dir/kafka && ./bin/kafka-server-stop.sh)
			$TESTTOOL_DIR/msleep 2000
			rm -rf $dep_work_dir/kafka
		fi
		;;
	 'stop-zookeeper')
		if [ "x$2" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
		else
			dep_work_dir=$(readlink -f $srcdir/$2)
		fi
		(cd $dep_work_dir/zk &> /dev/null && ./bin/zkServer.sh stop)
		$TESTTOOL_DIR/msleep 2000
		rm -rf $dep_work_dir/zk
		;;
	 'stop-elasticsearch')
		if [ "x$2" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
			dep_work_es_pidfile="es.pid"
		else
			dep_work_dir=$(readlink -f $srcdir/$2)
			dep_work_es_pidfile="es$2.pid"
		fi
		if [ -e $dep_work_es_pidfile ]; then
			es_pid=$(cat $dep_work_es_pidfile)
			printf "stopping ES with pid %d\n" $es_pid
			kill -SIGTERM $es_pid
			. $srcdir/diag.sh wait-pid-termination $es_pid
		fi
		;;
	 'cleanup-elasticsearch')
		dep_work_dir=$(readlink -f .dep_wrk)
		dep_work_es_pidfile="es.pid"
		. $srcdir/diag.sh stop-elasticsearch
		rm -f $dep_work_es_pidfile
		rm -rf $dep_work_dir/es
		;;
	 'create-kafka-topic')
		if [ "x$3" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
		else
			dep_work_dir=$(readlink -f $3)
		fi
		if [ "x$4" == "x" ]; then
			dep_work_port='2181'
		else
			dep_work_port=$4
		fi
		if [ ! -d $dep_work_dir/kafka ]; then
				echo "Kafka work-dir $dep_work_dir/kafka does not exist, did you start kafka?"
				exit 1
		fi
		if [ "x$2" == "x" ]; then
				echo "Topic-name not provided."
				exit 1
		fi
		(cd $dep_work_dir/kafka && ./bin/kafka-topics.sh --zookeeper localhost:$dep_work_port/kafka --create --topic $2 --replication-factor 1 --partitions 2 )
		(cd $dep_work_dir/kafka && ./bin/kafka-topics.sh --zookeeper localhost:$dep_work_port/kafka --alter --topic $2 --delete-config retention.ms)
		(cd $dep_work_dir/kafka && ./bin/kafka-topics.sh --zookeeper localhost:$dep_work_port/kafka --alter --topic $2 --delete-config retention.bytes)
		;;
	 'delete-kafka-topic')
		if [ "x$3" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
		else
			dep_work_dir=$(readlink -f $srcdir/$3)
		fi
		if [ "x$4" == "x" ]; then
			dep_work_port='2181'
		else
			dep_work_port=$4
		fi

		echo "deleting kafka-topic $2"
		(cd $dep_work_dir/kafka && ./bin/kafka-topics.sh --delete --zookeeper localhost:$dep_work_port/kafka --topic $2)
		;;
	 'dump-kafka-topic')
		if [ "x$3" == "x" ]; then
			dep_work_dir=$(readlink -f .dep_wrk)
			dep_kafka_log_dump=$(readlink -f rsyslog.out.kafka.log)
		else
			dep_work_dir=$(readlink -f $srcdir/$3)
			dep_kafka_log_dump=$(readlink -f rsyslog.out.kafka$3.log)
		fi
		if [ "x$4" == "x" ]; then
			dep_work_port='2181'
		else
			dep_work_port=$4
		fi

		echo "dumping kafka-topic $2"
		if [ ! -d $dep_work_dir/kafka ]; then
				echo "Kafka work-dir does not exist, did you start kafka?"
				exit 1
		fi
		if [ "x$2" == "x" ]; then
				echo "Topic-name not provided."
				exit 1
		fi

		(cd $dep_work_dir/kafka && ./bin/kafka-console-consumer.sh --timeout-ms 2000 --from-beginning --zookeeper localhost:$dep_work_port/kafka --topic $2 > $dep_kafka_log_dump)
		;;
	'check-inotify') # Check for inotify/fen support 
		if [ -n "$(find /usr/include -name 'inotify.h' -print -quit)" ]; then
			echo [inotify mode]
		elif [ -n "$(find /usr/include/sys/ -name 'port.h' -print -quit)" ]; then
			cat /usr/include/sys/port.h | grep -qF "PORT_SOURCE_FILE" 
			if [ "$?" -ne "0" ]; then
				echo [port.h found but FEN API not implemented , skipping...]
				exit 77 # FEN API not available, skip this test
			fi
			echo [fen mode]
		else
			echo [inotify/fen not supported, skipping...]
			exit 77 # no inotify available, skip this test
		fi
		;;
	'check-inotify-only') # Check for ONLY inotify support 
		if [ -n "$(find /usr/include -name 'inotify.h' -print -quit)" ]; then
			echo [inotify mode]
		else
			echo [inotify not supported, skipping...]
			exit 77 # no inotify available, skip this test
		fi
		;;
   *)		echo "TESTBENCH error: invalid argument" $1
		exit 100
esac
