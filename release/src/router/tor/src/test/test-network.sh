#!/bin/sh

# This script calls the equivalent script in chutney/tools

# If we already know CHUTNEY_PATH, don't bother with argument parsing
TEST_NETWORK="$CHUTNEY_PATH/tools/test-network.sh"
# Call the chutney version of this script, if it exists, and we can find it
if [ -d "$CHUTNEY_PATH" ] && [ -x "$TEST_NETWORK" ]; then
    # we can't produce any output, because we might be --quiet
    # this preserves arguments with spaces correctly
    exec "$TEST_NETWORK" "$@"
fi

# We need to go looking for CHUTNEY_PATH

# Do we output anything at all?
ECHO="${ECHO:-echo}"
# Output is prefixed with the name of the script
myname=$(basename "$0")

# We need to find CHUTNEY_PATH, so that we can call the version of this script
# in chutney/tools with the same arguments. We also need to respect --quiet.
CHUTNEY_PATH=$(echo "$@" | awk -F '--chutney-path ' '{sub(" .*","",$2); print $2}')
TOR_DIR=$(echo "$@" | awk -F '--tor-dir ' '{sub(" .*","",$2); print $2}')

if echo "$@" | grep -e "--quiet" > /dev/null; then
  ECHO=true
fi

# optional: $TOR_DIR is the tor build directory
# it's used to find the location of tor binaries
# if it's not set:
#  - set it to $BUILDDIR, or
#  - if $PWD looks like a tor build directory, set it to $PWD, or
#  - unset $TOR_DIR, and let chutney fall back to finding tor binaries in $PATH
if [ ! -d "$TOR_DIR" ]; then
    if [ -d "$BUILDDIR/src/core/or" ] && [ -d "$BUILDDIR/src/tools" ]; then
        # Choose the build directory
        # But only if it looks like one
        $ECHO "$myname: \$TOR_DIR not set, trying \$BUILDDIR"
        TOR_DIR="$BUILDDIR"
    elif [ -d "$PWD/src/core/or" ] && [ -d "$PWD/src/tools" ]; then
        # Guess the tor directory is the current directory
        # But only if it looks like one
        $ECHO "$myname: \$TOR_DIR not set, trying \$PWD"
        TOR_DIR="$PWD"
    else
        $ECHO "$myname: no \$TOR_DIR, chutney will use \$PATH for tor binaries"
        unset TOR_DIR
    fi
fi

# mandatory: $CHUTNEY_PATH is the path to the chutney launch script
# if it's not set:
#  - if $PWD looks like a chutney directory, set it to $PWD, or
#  - set it based on $TOR_DIR, expecting chutney to be next to tor, or
#  - fail and tell the user how to clone the chutney repository
if [ ! -d "$CHUTNEY_PATH" ] || [ ! -x "$CHUTNEY_PATH/chutney" ]; then
    if [ -x "$PWD/chutney" ]; then
        $ECHO "$myname: \$CHUTNEY_PATH not valid, trying \$PWD"
        CHUTNEY_PATH="$PWD"
    elif [ -d "$TOR_DIR" ] && [ -d "$TOR_DIR/../chutney" ] && \
          [ -x "$TOR_DIR/../chutney/chutney" ]; then
        $ECHO "$myname: \$CHUTNEY_PATH not valid, trying \$TOR_DIR/../chutney"
        CHUTNEY_PATH="$TOR_DIR/../chutney"
    else
        $ECHO "$myname: missing 'chutney' in \$CHUTNEY_PATH ($CHUTNEY_PATH)"
        $ECHO "$myname: Get chutney: git clone https://git.torproject.org/\
chutney.git"
        $ECHO "$myname: Set \$CHUTNEY_PATH to a non-standard location: export \
CHUTNEY_PATH=\`pwd\`/chutney"
        unset CHUTNEY_PATH
        exit 1
    fi
fi

TEST_NETWORK="$CHUTNEY_PATH/tools/test-network.sh"
# Call the chutney version of this script, if it exists, and we can find it
if [ -d "$CHUTNEY_PATH" ] && [ -x "$TEST_NETWORK" ]; then
    $ECHO "$myname: Calling newer chutney script $TEST_NETWORK"
    # this may fail if some arguments have spaces in them
    # if so, set CHUTNEY_PATH before calling test-network.sh, and spaces
    # will be handled correctly
    exec "$TEST_NETWORK" "$@"
else
    $ECHO "$myname: Could not find tools/test-network.sh in CHUTNEY_PATH."
    $ECHO "$myname: Please update your chutney using 'git pull'."
    # We have failed to do what the user asked
    exit 1
fi
