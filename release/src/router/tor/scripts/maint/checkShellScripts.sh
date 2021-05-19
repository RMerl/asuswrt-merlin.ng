#!/usr/bin/env bash
#
# Copyright (c) 2019 The Tor Project, Inc.
# See LICENSE for license information
#
# checkShellScripts.sh
# --------------------
# If shellcheck is installed, check all the shell scripts that we can fix.

set -e

# Only run this script if shellcheck is installed
# command echoes the path to shellcheck, which is a useful diagnostic log
if ! command -v shellcheck; then
    printf "%s: Install shellcheck to check shell scripts.\\n" "$0"
    exit 0
fi

# Some platforms don't have realpath
if command -v realpath ; then
    HERE=$(dirname "$(realpath "$0")")
else
    HERE=$(dirname "$0")
    if [ ! -d "$HERE" ] || [ "$HERE" = "." ]; then
        HERE=$(dirname "$PWD/$0")
    fi
fi
TOPLEVEL=$(dirname "$(dirname "$HERE")")

# Check we actually have a tor/src directory
if [ ! -d "$TOPLEVEL/src" ]; then
    printf "Error: Couldn't find src directory in expected location: %s\\n" \
        "$TOPLEVEL/src"
    exit 1
fi

# Remove obsolete scripts generated from older versions of Tor
rm -f "$TOPLEVEL/contrib/dist/suse/tor.sh" "$TOPLEVEL/contrib/dist/tor.sh"

# Check *.sh scripts, but ignore the ones that we can't fix
find "$TOPLEVEL/contrib" "$TOPLEVEL/doc" "$TOPLEVEL/scripts" "$TOPLEVEL/src" \
    -name "*.sh" \
    -not -path "$TOPLEVEL/src/ext/*" \
    -not -path "$TOPLEVEL/src/rust/registry/*" \
    -exec shellcheck {} +

# Check scripts that aren't named *.sh
if [ -d "$TOPLEVEL/scripts/test" ]; then
    shellcheck \
        "$TOPLEVEL/scripts/test/cov-diff" \
        "$TOPLEVEL/scripts/test/coverage"
fi
if [ -e \
    "$TOPLEVEL/contrib/dirauth-tools/nagios-check-tor-authority-cert" \
   ]; then
    shellcheck \
        "$TOPLEVEL/contrib/dirauth-tools/nagios-check-tor-authority-cert"
fi
if [ -e "$TOPLEVEL/contrib/client-tools/torify" ]; then
    shellcheck "$TOPLEVEL/contrib/client-tools/torify"
fi
if [ -d "$TOPLEVEL/scripts/git" ]; then
    shellcheck "$TOPLEVEL/scripts/git/"*.git-hook
fi
