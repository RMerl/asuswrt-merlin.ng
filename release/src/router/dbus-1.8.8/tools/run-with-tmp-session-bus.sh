#! /bin/sh

SCRIPTNAME="$0"
WRAPPED_SCRIPT="$1"
shift

CONFIG_FILE=./tmp-session-bus.$$.conf

die ()
{
    echo "$SCRIPTNAME: $*" >&2
    rm -f "$CONFIG_FILE"
    exit 1
}

if test -z "$DBUS_TOP_BUILDDIR" ; then
    die "Must set DBUS_TOP_BUILDDIR"
fi

SERVICE_DIR="$DBUS_TOP_BUILDDIR/test/data/valid-service-files"
ESCAPED_SERVICE_DIR=`echo $SERVICE_DIR | sed -e 's/\//\\\\\\//g'`
echo "escaped service dir is: $ESCAPED_SERVICE_DIR" >&2

if test -z "$SOURCE_CONFIG_FILE"; then
    SOURCE_CONFIG_FILE="$DBUS_TOP_BUILDDIR/bus/session.conf";
fi
## create a configuration file based on the standard session.conf
cat $SOURCE_CONFIG_FILE |  \
    sed -e 's/<standard_session_servicedirs.*$/<servicedir>'$ESCAPED_SERVICE_DIR'<\/servicedir>/g' |  \
    sed -e 's/<include.*$//g'                \
  > $CONFIG_FILE

echo "Created configuration file $CONFIG_FILE" >&2

if ! test -e "$DBUS_TOP_BUILDDIR"/bus/dbus-daemon ; then
    die "$DBUS_TOP_BUILDDIR/bus/dbus-daemon does not exist"
fi

PATH="$DBUS_TOP_BUILDDIR"/bus:$PATH
export PATH

## the libtool script found by the path search should already do this, but
LD_LIBRARY_PATH=$DBUS_TOP_BUILDDIR/dbus/.libs:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
unset DBUS_SESSION_BUS_ADDRESS
unset DBUS_SESSION_BUS_PID

# this does not actually affect dbus-run-session any more, but could be
# significant for dbus-launch as used by the autolaunch test
DBUS_USE_TEST_BINARY=1
export DBUS_USE_TEST_BINARY

$DBUS_TOP_BUILDDIR/tools/dbus-run-session \
    --config-file="$CONFIG_FILE" \
    --dbus-daemon="$DBUS_TOP_BUILDDIR/bus/dbus-daemon" \
    -- \
    "$WRAPPED_SCRIPT" "$@"
error=$?

# clean up
rm -f "$CONFIG_FILE"
exit $error
