#!/bin/bash
# Test for queue data persisting at shutdown. We use the actual driver
# to carry out multiple tests with different queue modes
# added 2009-05-27 by Rgerhards
# This file is part of the rsyslog project, released  under GPLv3
echo ===============================================================================
echo \[daqueue-persist.sh\]: test data persisting at shutdown
echo TEST is currently DISABLE because it is unstable
exit 77
echo mode linkedList
$srcdir/daqueue-persist-drvr.sh LinkedList
echo mode fixedArray
$srcdir/daqueue-persist-drvr.sh FixedArray
# the disk test should not fail, however, the config is extreme and using
# it more or less is a config error
echo Disk
$srcdir/daqueue-persist-drvr.sh Disk
# we do not test Direct mode because this absolute can not work in direct mode
# (maybe we should do a fail-type of test?)
