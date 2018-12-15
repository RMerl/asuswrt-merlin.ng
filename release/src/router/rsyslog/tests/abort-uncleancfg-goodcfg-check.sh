#!/bin/bash
# Copyright 2015-01-29 by Tim Eifler
# This file is part of the rsyslog project, released  under ASL 2.0
# The configuration test should pass because of the good config file.
. $srcdir/diag.sh init
echo "testing a good Configuration verification run"
../tools/rsyslogd  -C -N1 -f$srcdir/testsuites/abort-uncleancfg-goodcfg.conf -M../runtime/.libs:../.libs
if [ $? -ne 0 ]; then
   echo "Error: config check fail"
   exit 1 
fi
exit_test

