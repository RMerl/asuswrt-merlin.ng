#!/bin/bash
# Copyright 2015 Red Hat, Inc.
# This file is part of the rsyslog project, released  under ASL 2.0
# The configuration test should pass because we now support leading
# underscores in variable names.
. $srcdir/diag.sh init
../tools/rsyslogd  -C -N1 -f$srcdir/testsuites/variable_leading_underscore.conf -M../runtime/.libs:../.libs
if [ $? -ne 0 ]; then
   echo "Error: config check fail"
   exit 1 
fi
exit_test

