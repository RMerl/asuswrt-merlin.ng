# Common functions for shell testcases

PASS () {
    echo "PASS"
    exit 0
}

FAIL () {
    echo "FAIL" "$@"
    exit 2
}

FAIL_IF_SIGNAL () {
    ret="$1"
    if [ "$ret" -gt 127 ]; then
	signame=$(kill -l $((ret - 128)))
	FAIL "Killed by SIG$signame"
    fi
}

if [ -z "$TEST_BINDIR" ]; then
    TEST_BINDIR=..
fi

DTC=${TEST_BINDIR}/dtc
DTGET=${TEST_BINDIR}/fdtget
DTPUT=${TEST_BINDIR}/fdtput
FDTDUMP=${TEST_BINDIR}/fdtdump
FDTOVERLAY=${TEST_BINDIR}/fdtoverlay

verbose_run () {
    if [ -z "$QUIET_TEST" ]; then
	"$@"
    else
	"$@" > /dev/null 2> /dev/null
    fi
}

verbose_run_check () {
    verbose_run "$@"
    ret="$?"
    FAIL_IF_SIGNAL $ret
    if [ $ret != 0 ]; then
	FAIL "Returned error code $ret"
    fi
}

verbose_run_log () {
    LOG="$1"
    shift
    "$@" > "$LOG" 2>&1
    ret=$?
    if [ -z "$QUIET_TEST" ]; then
	cat "$LOG" >&2
    fi
    return $ret
}

verbose_run_log_check () {
    verbose_run_log "$@"
    ret="$?"
    FAIL_IF_SIGNAL $ret
    if [ $ret != 0 ]; then
	FAIL "Returned error code $ret"
    fi
}

