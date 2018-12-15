#!/bin/bash
# Copyright 2015-01-29 by Tim Eifler
# This file is part of the rsyslog project, released  under ASL 2.0
# The configuration test should fail because of the invalid config file.
echo ===============================================================================
echo \[abort-uncleancfg-badcfg_1.sh\]: testing abort on unclean configuration
echo "testing a bad Configuration verification run"
. $srcdir/diag.sh init
../tools/rsyslogd  -C -N1 -f$srcdir/testsuites/abort-uncleancfg-badcfg_1.conf -M../runtime/.libs:../.libs
if [ $? == 0 ]; then
   echo "Error: config check should fail"
   exit 1 
fi
exit_test

