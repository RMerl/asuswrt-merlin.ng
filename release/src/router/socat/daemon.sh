#! /bin/sh
# source: daemon.sh
# Copyright Gerhard Rieger and contributors (see file CHANGES)
# Published under the GNU General Public License V.2, see file COPYING

# This script assumes that you create group daemon1 and user daemon1 before.
# they need only the right to exist (no login etc.)

# Note: this pid file mechanism is not robust!

# You will adapt these variables
USER=daemon1
GROUP=daemon1
INIF=fwnonsec.domain.org
OUTIF=fwsec.domain.org
TARGET=w3.intra.domain.org
INPORT=80
DSTPORT=80
#
INOPTS="fork,setgid=$GROUP,setuid=$USER"
OUTOPTS=
PIDFILE=/var/run/socat-$INPORT.pid
OPTS="-d -d -lm"	# notice to stderr, then to syslog
SOCAT=/usr/local/bin/socat

if [ "$1" = "start" -o -z "$1" ]; then

    $SOCAT $OPTS tcp-l:$INPORT,bind=$INIF,$INOPTS tcp:$TARGET:$DSTPORT,bind=$OUTIF,$OUTOPTS </dev/null &
    echo $! >$PIDFILE

elif [ "$1" = "stop" ]; then

    /bin/kill $(/bin/cat $PIDFILE)
fi
