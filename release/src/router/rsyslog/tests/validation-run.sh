#!/bin/bash
# check if the configuration test run detects invalid config files.
#
# Part of the testbench for rsyslog.
#
# Copyright 2009-2018 Rainer Gerhards and Adiscon GmbH.
#
# This file is part of rsyslog.
# Released under ASL 2.0
echo \[validation-run.sh\]: testing configuraton validation
echo "testing a failed configuration verification run"
../tools/rsyslogd  -u2 -N1 -f$srcdir/testsuites/invalid.conf -M../runtime/.libs:../.libs
if [ $? -ne 1 ]; then
   echo "after test 1: return code ne 1"
   exit 1
fi
echo testing a valid config verification run
../tools/rsyslogd -u2 -N1 -f$srcdir/testsuites/valid.conf -M../runtime/.libs:../.libs
if [ $? -ne 0 ]; then
   echo "after test 2: return code ne 0"
   exit 1
fi
echo testing empty config file
../tools/rsyslogd -u2 -N1 -f/dev/null -M../runtime/.libs:../.libs
if [ $? -ne 1 ]; then
   echo "after test 3: return code ne 1"
   exit 1
fi
echo SUCCESS: validation run tests
