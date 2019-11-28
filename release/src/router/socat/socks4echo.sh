#! /bin/bash
# source: socks4echo.sh

# Copyright Gerhard Rieger and contributors (see file CHANGES)
# Published under the GNU General Public License V.2, see file COPYING

# perform primitive simulation of a socks4 server with echo function via stdio.
# accepts and answers correct SOCKS4 requests, but then just echoes data.
# it is required for test.sh
# for TCP, use this script as:
# socat tcp-l:1080,reuseaddr,crlf system:"socks4echo.sh"

# older bash and ksh do not have -n option to read command; we try dd then
#if echo a |read -n 1 null >/dev/null 2>&1; then
#    HAVE_READ_N=1
#else
    # and newer bash (4.3) has some other problem with read -n
    HAVE_READ_N=
#fi

if type socat >/dev/null 2>&1; then
    SOCAT=socat
else
    SOCAT=./socat
fi

case `uname` in
HP-UX|OSF1)
    CAT="$SOCAT -u stdin stdout"
    ;;
*)
    CAT=cat
    ;;
esac

if   [ $(echo "x\c") = "x" ]; then E=""
elif [ $(echo -e "x\c") = "x" ]; then E="-e"
else
    echo "cannot suppress trailing newline on echo" >&2
    exit 1
fi
ECHO="echo $E"

if [ $($ECHO "\0101") = "A" ]; then
    SOCKSREPLY_FAILED="\0\0133\0\0\0\0\0\0\c"
    SOCKSREPLY_OK="\0\0132\0\0\0\0\0\0\c"
else
    SOCKSREPLY_FAILED="\0\133\0\0\0\0\0\0\c"
    SOCKSREPLY_OK="\0\132\0\0\0\0\0\0\c"
fi

# read and parse SOCKS4 header
if [ "$HAVE_READ_N" ]; then
    read -r -n 1 vn	# bash 2.0.3 does not support -n
else
    vn=$(dd bs=1 count=1 2>/dev/null)
fi
if [ "$vn" != $($ECHO "\04") ]; then
    $ECHO "$SOCKSREPLY_FAILED"
    echo "invalid socks version requested" >&2
    exit
fi

if [ "$HAVE_READ_N" ]; then
    read -r -n 1 cd
else
    cd=$(dd bs=1 count=1 2>/dev/null)
fi
if [ "$cd" != $($ECHO "\01") ]; then
    $ECHO "$SOCKSREPLY_FAILED"
    echo "invalid socks operation requested" >&2
    exit
fi

if [ "$HAVE_READ_N" ]; then
    read -r -n 6 a
else
    a=$(dd bs=1 count=6 2>/dev/null)
fi
if [ "$a" != "$($ECHO "}m bL6")" ]; then
    $ECHO "$SOCKSREPLY_FAILED"
    echo "$0: wrong socks address or port requested" >&2
    echo "$0: expected $($ECHO "}m bL6"|od -t x1), received $($ECHO "$a"|od -t x1)" >&2
    exit
fi

if [ "$HAVE_READ_N" ]; then
    read -r -n 7 u
else
    u=$(dd bs=1 count=7 2>/dev/null)
fi
if [ "$u" != "nobody" ]; then
    $ECHO "$SOCKSREPLY_FAILED"
    echo "wrong socks user requested" >&2
    exit
fi

# send ok status
$ECHO "$SOCKSREPLY_OK"

# perform echo function
$CAT
