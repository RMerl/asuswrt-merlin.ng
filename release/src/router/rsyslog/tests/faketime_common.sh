#!/bin/bash
# addd 2016-03-11 by Thomas D., released under ASL 2.0
# Several tests make use of faketime. They all need to know when
# faketime is missing or the system isn't year-2038 complaint.
# This script can be sourced to prevent duplicated code.

rsyslog_testbench_preload_libfaketime() {
    local missing_requirements=
    if ! hash find 2>/dev/null ; then
        missing_requirements="'find' is missing in PATH; Make sure you have findutils/coreutils installed! Skipping test ..."
    fi

    if ! hash $RS_SORTCMD 2>/dev/null ; then
        missing_requirements="'sort' is missing in PATH; Make sure you have coreutils installed! Skipping test ..."
    fi

    if ! hash $RS_HEADCMD 2>/dev/null ; then
        missing_requirements="'head' is missing in PATH; Make sure you have coreutils installed! Skipping test ..."
    fi

    if [ -n "${missing_requirements}" ]; then
        echo ${missing_requirements}
        exit 77
    fi

    RSYSLOG_LIBFAKETIME=$(find /usr -name 'libfaketime.so*' -type f | $RS_SORTCMD --reverse | $RS_HEADCMD --lines 1)
    if [ -z "${RSYSLOG_LIBFAKETIME}" ]; then
        echo "Could not determine libfaketime library, skipping test!"
        exit 77
    fi

    echo "Testing '${RSYSLOG_LIBFAKETIME}' library ..."
    local faketime_testtime=$(LD_PRELOAD="${RSYSLOG_LIBFAKETIME}" FAKETIME="1991-08-25 20:57:08" TZ=GMT date +%s 2>/dev/null)
    if [ ${faketime_testtime} -ne 683153828 ] ; then
        echo "'${RSYSLOG_LIBFAKETIME}' failed sanity check, skipping test!"
        exit 77
    else
        echo "Test passed! Will use '${RSYSLOG_LIBFAKETIME}' library!"
        export RSYSLOG_PRELOAD="${RSYSLOG_LIBFAKETIME}"
    fi

    # GMT-1 (POSIX TIME) is GMT+1 in "Human Time"
    faketime_testtime=$(LD_PRELOAD="${RSYSLOG_LIBFAKETIME}" FAKETIME="2040-01-01 16:00:00" TZ=GMT-1 date +%s 2>/dev/null)
    if [ ${faketime_testtime} -eq -1 ]; then
        echo "Note: System is not year-2038 compliant"
        RSYSLOG_TESTBENCH_Y2K38_INCOMPATIBLE="yes"
    else
        echo "Note: System is year-2038 compliant"
    fi
}

rsyslog_testbench_require_y2k38_support() {
    if [ -n "${RSYSLOG_TESTBENCH_Y2K38_INCOMPATIBLE}" ]; then
        echo "Skipping further tests because system doesn't support year 2038 ..."
        exit_test
        exit 0
    fi
}

rsyslog_testbench_preload_libfaketime
