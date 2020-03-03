#!/bin/sh
# This validates that the kernel will fall back to using the user helper
# to load firmware it can't find on disk itself. We must request a firmware
# that the kernel won't find, and any installed helper (e.g. udev) also
# won't find so that we can do the load ourself manually.
set -e

modprobe test_firmware

DIR=/sys/devices/virtual/misc/test_firmware

OLD_TIMEOUT=$(cat /sys/class/firmware/timeout)

FWPATH=$(mktemp -d)
FW="$FWPATH/test-firmware.bin"

test_finish()
{
	echo "$OLD_TIMEOUT" >/sys/class/firmware/timeout
	rm -f "$FW"
	rmdir "$FWPATH"
}

load_fw()
{
	local name="$1"
	local file="$2"

	# This will block until our load (below) has finished.
	echo -n "$name" >"$DIR"/trigger_request &

	# Give kernel a chance to react.
	local timeout=10
	while [ ! -e "$DIR"/"$name"/loading ]; do
		sleep 0.1
		timeout=$(( $timeout - 1 ))
		if [ "$timeout" -eq 0 ]; then
			echo "$0: firmware interface never appeared" >&2
			exit 1
		fi
	done

	echo 1 >"$DIR"/"$name"/loading
	cat "$file" >"$DIR"/"$name"/data
	echo 0 >"$DIR"/"$name"/loading

	# Wait for request to finish.
	wait
}

trap "test_finish" EXIT

# This is an unlikely real-world firmware content. :)
echo "ABCD0123" >"$FW"
NAME=$(basename "$FW")

DEVPATH="$DIR"/"nope-$NAME"/loading

# Test failure when doing nothing (timeout works).
echo -n 2 >/sys/class/firmware/timeout
echo -n "nope-$NAME" >"$DIR"/trigger_request 2>/dev/null &

# Give the kernel some time to load the loading file, must be less
# than the timeout above.
sleep 1
if [ ! -f $DEVPATH ]; then
	echo "$0: fallback mechanism immediately cancelled"
	echo ""
	echo "The file never appeared: $DEVPATH"
	echo ""
	echo "This might be a distribution udev rule setup by your distribution"
	echo "to immediately cancel all fallback requests, this must be"
	echo "removed before running these tests. To confirm look for"
	echo "a firmware rule like /lib/udev/rules.d/50-firmware.rules"
	echo "and see if you have something like this:"
	echo ""
	echo "SUBSYSTEM==\"firmware\", ACTION==\"add\", ATTR{loading}=\"-1\""
	echo ""
	echo "If you do remove this file or comment out this line before"
	echo "proceeding with these tests."
	exit 1
fi

if diff -q "$FW" /dev/test_firmware >/dev/null ; then
	echo "$0: firmware was not expected to match" >&2
	exit 1
else
	echo "$0: timeout works"
fi

# Put timeout high enough for us to do work but not so long that failures
# slow down this test too much.
echo 4 >/sys/class/firmware/timeout

# Load this script instead of the desired firmware.
load_fw "$NAME" "$0"
if diff -q "$FW" /dev/test_firmware >/dev/null ; then
	echo "$0: firmware was not expected to match" >&2
	exit 1
else
	echo "$0: firmware comparison works"
fi

# Do a proper load, which should work correctly.
load_fw "$NAME" "$FW"
if ! diff -q "$FW" /dev/test_firmware >/dev/null ; then
	echo "$0: firmware was not loaded" >&2
	exit 1
else
	echo "$0: user helper firmware loading works"
fi

exit 0
