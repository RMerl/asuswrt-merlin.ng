#!/bin/bash
# This test check what happens if we cannot doe getaddrinfo early
# in rsyslog startup  (this has caused an error in the past). Even more
# importantly, it checks that error messages can be issued very early
# during startup.
# Note that we use the override of the hostname to ensure we do not
# accidentely get an acceptable FQDN-type hostname during testing.
#
# IMPORTANT: We cannot use the regular plumbing here, as our preload
# interferes with socket operations (we cannot bind the port for some
# reason). As we do not necessarily need the full plumbing for this
# simple test, we emulate what we need. It's a bit ugly, but actually
# the simplest way forward.
#
# This is part of the rsyslog testbench, licensed under ASL 2.0
. $srcdir/diag.sh init
skip_platform "AIX" "we cannot preload required dummy lib"
skip_platform "SunOS" "there seems to be an issue with LD_PRELOAD libraries"
skip_platform "FreeBSD" "temporarily disabled until we know what is wrong, \
see https://github.com/rsyslog/rsyslog/issues/2833"

echo 'action(type="omfile" file="'$RSYSLOG_DYNNAME'.out.log")' > ${RSYSLOG_DYNNAME}.conf 
LD_PRELOAD=".libs/liboverride_gethostname_nonfqdn.so:.libs/liboverride_getaddrinfo.so" \
	../tools/rsyslogd -C -n -i$RSYSLOG_DYNNAME.pid -M../runtime/.libs:../.libs -f${RSYSLOG_DYNNAME}.conf &
wait_process_startup $RSYSLOG_DYNNAME
sleep 1 # wait a bit so that rsyslog can do some processing...
kill $(cat $RSYSLOG_DYNNAME.pid )

grep " nonfqdn " < $RSYSLOG_DYNNAME.out.log
if [ ! $? -eq 0 ]; then
  echo "expected hostnaame \"nonfqdn\" not found in logs, $RSYSLOG_DYNNAME.out.log is:"
  cat $RSYSLOG_DYNNAME.out.log
  error_exit 1
fi;

echo EVERYTHING OK - error messages are just as expected!
exit_test
