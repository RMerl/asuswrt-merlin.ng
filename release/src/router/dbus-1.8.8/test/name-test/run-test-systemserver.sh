#! /bin/sh
die()
{
    if ! test -z "$DBUS_SESSION_BUS_PID" ; then
        echo "killing message bus "$DBUS_SESSION_BUS_PID >&2
        kill -9 $DBUS_SESSION_BUS_PID
    fi
    echo $SCRIPTNAME: $* >&2

    exit 1
}

SCRIPTNAME=$0
MODE=$1

## so the tests can complain if you fail to use the script to launch them
DBUS_TEST_NAME_RUN_TEST_SCRIPT=1
export DBUS_TEST_NAME_RUN_TEST_SCRIPT

SOURCE_CONFIG_FILE=$DBUS_TOP_SRCDIR/test/name-test/tmp-session-like-system.conf
export SOURCE_CONFIG_FILE
# Rerun ourselves with tmp session bus if we're not already
if test -z "$DBUS_TEST_NAME_IN_SYS_RUN_TEST"; then
  DBUS_TEST_NAME_IN_SYS_RUN_TEST=1
  export DBUS_TEST_NAME_IN_SYS_RUN_TEST
  exec $DBUS_TOP_SRCDIR/tools/run-with-tmp-session-bus.sh $SCRIPTNAME $MODE
fi 

if test -n "$DBUS_TEST_MONITOR"; then
  dbus-monitor --session &
fi

XDG_RUNTIME_DIR="$DBUS_TOP_BUILDDIR"/test/XDG_RUNTIME_DIR
test -d "$XDG_RUNTIME_DIR" || mkdir "$XDG_RUNTIME_DIR"
chmod 0700 "$XDG_RUNTIME_DIR"
export XDG_RUNTIME_DIR

echo "running test-expected-echo-fail"
${DBUS_TOP_BUILDDIR}/libtool --mode=execute $DEBUG $DBUS_TOP_BUILDDIR/tools/dbus-send --print-reply --dest=org.freedesktop.DBus.TestSuiteEchoService /org/freedesktop/TestSuite org.freedesktop.TestSuite.Echo string:hi >echo-error-output.tmp 2>&1
if ! grep -q 'DBus.Error' echo-error-output.tmp; then
  echo "Didn't get expected failure; output was:"
  echo "====="
  cat echo-error-output.tmp
  echo "====="
  exit 1
fi

echo "running test echo signal"
if test "x$PYTHON" = "x:"; then
  echo "Skipped test-echo-signal: Python interpreter not found"
elif ! $PYTHON $DBUS_TOP_SRCDIR/test/name-test/test-wait-for-echo.py; then
  echo "Failed test-wait-for-echo"
  exit 1
fi

exit 0
