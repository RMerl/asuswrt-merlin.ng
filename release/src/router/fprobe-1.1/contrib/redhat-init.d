#!/bin/sh
#
# fprobe        This shell script takes care of starting and stopping
#               the fprobe daemon.
#
# chkconfig: - 65 35
# description: fprobe netflow generator

# Source function library.
. /etc/rc.d/init.d/functions

# Source networking configuration.
. /etc/sysconfig/network

# Check that networking is up.
[ ${NETWORKING} = "no" ] && exit 0

[ -f /usr//local/bin/fprobe ] || exit 0

RETVAL=0

OPTIONS="-i eth0 -a 192.168.5.1 -l 1 ginger.local.senie.com:2055"

start() {
         # Start daemons.
         echo -n "Starting fprobe: "
         daemon /usr/local/bin/fprobe $OPTIONS
         RETVAL=$?
         echo
         [ $RETVAL -eq 0 ] && touch /var/lock/subsys/fprobe
         return $RETVAL
}


stop() {
         # Stop daemons.
         echo -n "Shutting down fprobe: "
         killproc fprobe
         RETVAL=$?
         echo
         [ $RETVAL -eq 0 ] && rm -f /var/lock/subsys/fprobe
         return $RETVAL
}

# See how we were called.
case "$1" in
   start)
         start
         ;;
   stop)
         stop
         ;;
   restart|reload)
         stop
         start
         RETVAL=$?
         ;;
   *)
         echo "Usage: fprobe.rc {start|stop|restart}"
         exit 1
esac

exit $RETVAL
