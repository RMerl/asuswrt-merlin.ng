#!/usr/bin/env sh

WRAPPER=""

if [ -n "$VALGRIND_TESTS" ]; then
    WRAPPER="valgrind --error-exitcode=301 --leak-check=yes --show-reachable=yes --track-origins=yes"
fi

exec $WRAPPER "$@"
