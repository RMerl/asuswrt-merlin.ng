#!/bin/sh

EXITCODE=0

# #ifdef instead of #if
grep '#ifdef DROPBEAR' -I -- *.c *.h && EXITCODE=1

exit $EXITCODE
