#!/bin/bash
# Copyright 2015-01-29 by Tim Eifler
# This file is part of the rsyslog project, released  under ASL 2.0
# The configuration test should fail because of the invalid config file.
. $srcdir/diag.sh init
../tools/rsyslogd  -C -N1 -f$srcdir/testsuites/abort-uncleancfg-badcfg.conf -M../runtime/.libs:../.libs
if [ $? == 0 ]; then
   echo "Error: config check should fail"
   error_exit 1
fi
printf "unclean config lead to exit, as expected - OK\n"
exit_test

